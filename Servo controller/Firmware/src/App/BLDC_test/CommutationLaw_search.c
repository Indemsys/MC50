// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-10-19
// 17:16:09
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

#define CURR_INC_STEPS  100

//--------------------------------------------------------------------------------------------------------------
//  Результаты полученные при испытаниях мотора PB30950491
//--------------------------------------------------------------------------------------------------------------
//                                                             Текущее         Вращение           Вращение
//                                                            состояние      против часовой       по часовой
//                                                            U   V   W        U   V   W          U   V   W
//                                                            A   B   C        A   B   C          A   B   C
//
// Step 1: Hall sensors state: HU = 0  HV = 1  HW = 0 (2)     1   0   0        1   0  -1          1  -1   0
// Step 2: Hall sensors state: HU = 0  HV = 1  HW = 1 (3)     1   1   0        0   1  -1          1   0  -1
// Step 3: Hall sensors state: HU = 0  HV = 0  HW = 1 (1)     0   1   0       -1   1   0          0   1  -1
// Step 4: Hall sensors state: HU = 1  HV = 0  HW = 1 (5)     0   1   1       -1   0   1         -1   1   0
// Step 5: Hall sensors state: HU = 1  HV = 0  HW = 0 (4)     0   0   1        0  -1   1         -1   0   1
// Step 6: Hall sensors state: HU = 1  HV = 1  HW = 0 (6)     1   0   1        1  -1   0          0  -1   1
//--------------------------------------------------------------------------------------------------------------
// Соответствие сигналов в модели MATLAB и сигналов датчиков Холла в моторе PB30950491
// Hall_C = inv HV
// Hall_B = inv HU
// Hall_A - inv HW


/*-----------------------------------------------------------------------------------------------------
  Переносим записи в таблицы коммутаций для вращения в прямом и обратном направлении

  \param step
  \param mask

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t _Write_6step_tbl(uint8_t step, uint8_t mask)
{
  if ((step > 6) || (step < 1)) return RES_ERROR; // Ошибка некорректного шага
  if ((mask > 6) || (mask < 1)) return RES_ERROR; // Ошибка некорректной маски

  step = step - 1;
  mask = mask - 1;

  if (g_check_arr[mask] != 0) return RES_ERROR; // Ошибка, поскольку данная маска уже использовалась

  g_check_arr[mask] = 1;
  g_comm_tbl_arr[step] = mask+1;

  r3ph.rv_comm[mask].U = reference_comm_table[step].rv_U;
  r3ph.rv_comm[mask].V = reference_comm_table[step].rv_V;
  r3ph.rv_comm[mask].W = reference_comm_table[step].rv_W;

  r3ph.fw_comm[mask].U = reference_comm_table[step].fw_U;
  r3ph.fw_comm[mask].V = reference_comm_table[step].fw_V;
  r3ph.fw_comm[mask].W = reference_comm_table[step].fw_W;

  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Store_6step_tbl_to_wvar(void)
{
  wvar.bldc_comm_law = g_comm_tbl_arr[0]*100000+g_comm_tbl_arr[1]*10000+g_comm_tbl_arr[2]*1000+g_comm_tbl_arr[3]*100+g_comm_tbl_arr[4]*10+g_comm_tbl_arr[5];
}


/*-----------------------------------------------------------------------------------------------------


  \param p_pwm

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_test_pwm_val(float *p_pwm_fp, int32_t *p_pwm_val)
{
  float     pwm_step;
  float     fp_pwm;
  int32_t   pwm;
  uint32_t  app_err;
  uint32_t  res = RES_ERROR;

  pwm_step = (float)PWM_100 / (float)CURR_INC_STEPS;
  fp_pwm   = pwm_step;
  pwm      = lroundf(fp_pwm);

  for (uint32_t i=0; i < CURR_INC_STEPS; i++)
  {
    PWM_send(pwm, PWM_0 , PWM_0);
    Wait_ms(2);
    Real_values_calculation();
    app_err = App_get_error_flags();
    if (app_err != 0) goto exit_;
    if ((i == 0) && (adc.i_pwr > wvar.bldc_sett_max_current)) goto exit_;
    if (adc.i_pwr > wvar.bldc_sett_max_current) break;
    fp_pwm += pwm_step;
    pwm = lroundf(fp_pwm);
  }

  *p_pwm_fp  = fp_pwm;
  *p_pwm_val = pwm;

  res = RES_OK;

exit_:

  BLDC_rotor_hard_stop();
  Wait_ms(100);
  return res;

}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return float
-----------------------------------------------------------------------------------------------------*/
float Get_test_pwm_percents(void)
{
  return (g_test_fp_pwm * 100.0f / (float)PWM_100);
}

/*-----------------------------------------------------------------------------------------------------

  Последовательность коммутаций

            U             V             W
           (A)           (B)           (C)

     1      pwm           0%            0%

     2     100%          100%         100%-pwm

     3      0%           pwm            0%

     4   100%-pwm        100%          100%

     5      0%            0%            pwm

     6     100%        100%-pwm        100%

  \param void
-----------------------------------------------------------------------------------------------------*/

uint32_t  Commutation_Law_search(void)
{

  uint32_t  app_err;
  uint32_t  actual_events;
  uint32_t  res;
  uint32_t  ret = RES_ERROR;

  APPLOG("CLSrch: Commutation table creating START");

  app_err = App_get_error_flags();
  if (app_err != 0)
  {
    APPLOG("CLSrch: Execution was broken due to an system error %08X", app_err);
    goto exit_;
  }

  BLDC_rotor_hard_stop();
  Wait_ms(10);
  Currents_offset_calibration();

  memset(g_check_arr, 0 , sizeof(g_check_arr));
  memset(g_comm_tbl_arr, 0 , sizeof(g_comm_tbl_arr));

  if (Get_test_pwm_val(&g_test_fp_pwm,&g_test_pwm_val) != RES_OK)
  {
    APPLOG("CLSrch:  Execution was broken due to an error during pwm setting");
    goto exit_;
  }
  APPLOG("CLSrch: The specified current level has been reached = %0.1f A. PWM level = %d (%0.1f%%)", (double)adc.i_pwr, lroundf(g_test_fp_pwm), (double)Get_test_pwm_percents());

  APPLOG("CLSrch: Step 1: Hall sensors state: HW = %d  HV = %d  HU = %d", h3.W_bit , h3.V_bit, h3.U_bit);

  if (_Write_6step_tbl(1, h3.bitmask) != RES_OK)
  {
    APPLOG("CLSrch: Data error when writing to the commutation table");
    goto exit_;
  }

  PWM_send(PWM_100 , PWM_100, PWM_100-g_test_pwm_val);
  res = App_wait_flags(BLDC_DRIVER_FAULT  ,&actual_events, 100);
  if (res == TX_SUCCESS)
  {
    APPLOG("CLSrch: Execution was broken due an driver error");
    goto exit_;
  }
  APPLOG("CLSrch: Step 2: Hall sensors state: HW = %d  HV = %d  HU = %d", h3.W_bit , h3.V_bit, h3.U_bit);
  BLDC_rotor_hard_stop();
  Wait_ms(100);
  if (_Write_6step_tbl(2, h3.bitmask) != RES_OK)
  {
    APPLOG("CLSrch: Data error when writing to the commutation table");
    goto exit_;
  }

  PWM_send(PWM_0 , g_test_pwm_val, PWM_0);
  res = App_wait_flags(BLDC_DRIVER_FAULT  ,&actual_events, 100);
  if (res == TX_SUCCESS)
  {
    APPLOG("CLSrch: Execution was broken due an driver error");
    goto exit_;
  }
  APPLOG("CLSrch: Step 3: Hall sensors state: HW = %d  HV = %d  HU = %d", h3.W_bit , h3.V_bit, h3.U_bit);
  BLDC_rotor_hard_stop();
  Wait_ms(100);
  if (_Write_6step_tbl(3, h3.bitmask) != RES_OK)
  {
    APPLOG("CLSrch: Data error when writing to the commutation table");
    goto exit_;
  }

  PWM_send(PWM_100-g_test_pwm_val, PWM_100, PWM_100);
  res = App_wait_flags(BLDC_DRIVER_FAULT  ,&actual_events, 100);
  if (res == TX_SUCCESS)
  {
    APPLOG("CLSrch: Execution was broken due an driver error");
    goto exit_;
  }
  APPLOG("CLSrch: Step 4: Hall sensors state: HW = %d  HV = %d  HU = %d", h3.W_bit , h3.V_bit, h3.U_bit);
  BLDC_rotor_hard_stop();
  Wait_ms(100);
  if (_Write_6step_tbl(4, h3.bitmask) != RES_OK)
  {
    APPLOG("CLSrch: Data error when writing to the commutation table");
    goto exit_;
  }

  PWM_send(PWM_0 , PWM_0, g_test_pwm_val);
  res = App_wait_flags(BLDC_DRIVER_FAULT  ,&actual_events, 100);
  if (res == TX_SUCCESS)
  {
    APPLOG("CLSrch: Execution was broken due an driver error");
    goto exit_;
  }
  APPLOG("CLSrch: Step 5: Hall sensors state: HW = %d  HV = %d  HU = %d", h3.W_bit , h3.V_bit, h3.U_bit);
  BLDC_rotor_hard_stop();
  Wait_ms(100);
  if (_Write_6step_tbl(5, h3.bitmask) != RES_OK)
  {
    APPLOG("CLSrch: Data error when writing to the commutation table");
    goto exit_;
  }

  PWM_send(PWM_100 , PWM_100-g_test_pwm_val, PWM_100);
  res = App_wait_flags(BLDC_DRIVER_FAULT  ,&actual_events, 100);
  if (res == TX_SUCCESS)
  {
    APPLOG("CLSrch: Execution was broken due an driver error");
    goto exit_;
  }
  APPLOG("CLSrch: Step 6: Hall sensors state: HW = %d  HV = %d  HU = %d", h3.W_bit , h3.V_bit, h3.U_bit);
  BLDC_rotor_hard_stop();
  Wait_ms(100);
  if (_Write_6step_tbl(6, h3.bitmask) != RES_OK)
  {
    APPLOG("CLSrch: Data error when writing to the commutation table");
    goto exit_;
  }

  APPLOG("CLSrch: Commutation table created successfully: %d %d %d %d %d %d", g_comm_tbl_arr[0],g_comm_tbl_arr[1],g_comm_tbl_arr[2],g_comm_tbl_arr[3],g_comm_tbl_arr[4],g_comm_tbl_arr[5]);

  ret = RES_OK;

exit_:
  APPLOG("CLSrch: Commutation table creating END");
  BLDC_rotor_hard_stop();
  return ret;
}










