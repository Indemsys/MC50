// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-10-19
// 17:47:49
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


extern uint32_t             pwm_indx_to_comp[PWM_STEP_COUNT];
T_rotation_3ph              r3ph;
uint8_t                     g_check_arr[6];
uint8_t                     g_comm_tbl_arr[6];

// Референсная таблица мягкой коммутаций для шагов 1...6 при вращении в обратном и прямом направлении
//  0 - означает выключенное состояние фазы
//  1 - означает подачу ШИМ на фазу
// -1 - означает подачу 0 на фазу
const T_comm_rec  reference_comm_table[6] =
{
//  |   FORWARD     |   REVERSE     |
//  |  U    V    W  |  U    V    W  |
  {   0,   1,  -1,     0,  -1,   1    },
  {  -1,   1,   0,     1,  -1,   0    },
  {  -1,   0,   1,     1,   0,  -1    },
  {   0,  -1,   1,     0,   1,  -1    },
  {   1,  -1,   0,    -1,   1,   0    },
  {   1,   0,  -1,    -1,   0,   1    }
};


static inline void Set_gpt_pwm_val(R_GPTA0_Type *R_GPT, uint32_t top_comp_value, uint32_t pwm_lev);

/*-----------------------------------------------------------------------------------------------------
  Функция вызываемая в обработчике прерываний по окончании выборок АЦП
  Вызывается синхронно с ШИМ с частотой ШИМ


  \param void
-----------------------------------------------------------------------------------------------------*/
void BLDC_ISR_handler(void)
{
  Hall_3PH_capture_bitmask();                // 0.9 us
  Measure_instant_phases_current();          // 0.38 us
  Measure_overall_BLDC_motor_current();      // 1.09 us
  Measure_DC_bus_voltage_current();          // 0.94 us
  Measure_servo_sensor_speed();              // 2.35 us
  Hall_3PH_measure_speed_and_direction();    // 0.76 us


  switch (r3ph.out_U_state_req)
  {
  case OUT_TO_ENABLE:
    R_GPTA0->GTIOR = R_GPTA0->GTIOR | (BIT(8) | BIT(24)); // Установка битов GTIOCA Pin Output Enable и GTIOCB Pin Output Enable
    r3ph.out_U_state_req = OUT_NO_ACTION;
    break;
  case OUT_TO_DISABLE:
    R_GPTA0->GTIOR = R_GPTA0->GTIOR & ~(BIT(8) | BIT(24)); // Сброс битов GTIOCA Pin Output Enable и GTIOCB Pin Output Enable
    r3ph.out_U_state_req = OUT_NO_ACTION;
    break;
  }
  switch (r3ph.out_V_state_req)
  {
  case OUT_TO_ENABLE:
    R_GPTA1->GTIOR = R_GPTA1->GTIOR | (BIT(8) | BIT(24)); // Установка битов GTIOCA Pin Output Enable и GTIOCB Pin Output Enable
    r3ph.out_V_state_req = OUT_NO_ACTION;
    break;
  case OUT_TO_DISABLE:
    R_GPTA1->GTIOR = R_GPTA1->GTIOR & ~(BIT(8) | BIT(24)); // Сброс битов GTIOCA Pin Output Enable и GTIOCB Pin Output Enable
    r3ph.out_V_state_req = OUT_NO_ACTION;
    break;
  }
  switch (r3ph.out_W_state_req)
  {
  case OUT_TO_ENABLE:
    R_GPTA2->GTIOR = R_GPTA2->GTIOR | (BIT(8) | BIT(24)); // Установка битов GTIOCA Pin Output Enable и GTIOCB Pin Output Enable
    r3ph.out_W_state_req = OUT_NO_ACTION;
    break;
  case OUT_TO_DISABLE:
    R_GPTA2->GTIOR = R_GPTA2->GTIOR & ~(BIT(8) | BIT(24)); // Сброс битов GTIOCA Pin Output Enable и GTIOCB Pin Output Enable
    r3ph.out_W_state_req = OUT_NO_ACTION;
    break;
  }


  // Установка всех 3 фаз длится 0.98 us
  Set_gpt_pwm_val(R_GPTA0, r3ph.gpt_top_val, r3ph.gpt_U_pwm_val);                    // Обновление регистра копаратора фазы U
  Set_gpt_pwm_val(R_GPTA1, r3ph.gpt_top_val, r3ph.gpt_V_pwm_val);                    // Обновление регистра копаратора фазы V
  Set_gpt_pwm_val(R_GPTA2, r3ph.gpt_top_val, r3ph.gpt_W_pwm_val);                    // Обновление регистра копаратора фазы W

}


/*-----------------------------------------------------------------------------------------------------
  Функция вызываемая из обработчика прерывания синхронно с треугольным ШИМ на подъеме или спаде


  \param R_GPT          - указатель на структуру таймера
  \param top_comp_value - верхнее значение компаратора таймера
  \param pwm_lev        - коэффициент заполнения ШИМ от 0 до PWM_STEP_COUNT
-----------------------------------------------------------------------------------------------------*/
static inline void Set_gpt_pwm_val(R_GPTA0_Type *R_GPT, uint32_t top_comp_value, uint32_t pwm_lev)
{
  uint32_t comp_now  = R_GPT->GTCCRA;
  uint32_t count_dir =(R_GPT->GTST) & BIT(15); // Если счет идет вниз, то значение будет равно 0

  if (pwm_lev == LEVEL_0)
  {
    if (comp_now != 0)
    {
      //============================
      // pwm -> 0
      //============================
      if (count_dir == GPT_COUNT_DIR_DOWN)
      {
        R_GPT->GTCCRC = top_comp_value;
      }
    }
    else
    {
      //============================
      // 1 -> 0
      //============================
      if (count_dir != GPT_COUNT_DIR_DOWN)
      {
        R_GPT->GTCCRC = top_comp_value >> 1;
      }
    }
  }
  else if (pwm_lev == LEVEL_1)
  {
    if (comp_now != top_comp_value)
    {
      //============================
      // pwm -> 1
      //============================
      if (count_dir == GPT_COUNT_DIR_DOWN)
      {
        R_GPT->GTCCRC = 0;
      }
    }
    else
    {
      //============================
      // 0 -> 1
      //============================
      if (count_dir != GPT_COUNT_DIR_DOWN)
      {
        R_GPT->GTCCRC = top_comp_value >> 1;
      }
    }
  }
  else
  {
    uint32_t comp_val = r3ph.pwm_indx_to_comp[pwm_lev];
    if (comp_now == top_comp_value)
    {
      //============================
      // 0 -> pwm
      //============================
      if (count_dir == GPT_COUNT_DIR_DOWN)
      {
        R_GPT->GTCCRC = comp_val;
      }
    }
    else if  (comp_now == 0)
    {
      //============================
      // 1 -> pwm
      //============================
      if (count_dir != GPT_COUNT_DIR_DOWN)
      {
        R_GPT->GTCCRC = comp_val;
      }
    }
    else
    {
      //============================
      // pwm -> pwm
      //============================
      R_GPT->GTCCRC = comp_val;
    }
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void BLDC_rotor_hard_stop(void)
{
  BLDC_set_outputs_state(OUT_TO_ENABLE);
  PWM_send(PWM_0, PWM_0, PWM_0);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void BLDC_rotor_freewheeling(void)
{
  BLDC_set_outputs_state(OUT_TO_DISABLE);
  PWM_send(PWM_0, PWM_0, PWM_0);
}

/*-----------------------------------------------------------------------------------------------------


  \param state
-----------------------------------------------------------------------------------------------------*/
void BLDC_set_outputs_state(uint8_t state)
{
  __disable_interrupt();
  r3ph.out_U_state_req  = state;
  r3ph.out_V_state_req  = state;
  r3ph.out_W_state_req  = state;
  __enable_interrupt();
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static void  _Prepare_6step_tbls(void)
{
  uint8_t mask;

  for (uint8_t step=0; step < 6; step++)
  {
    mask = r3ph.comm_law_arr[step] - 1;

    r3ph.rv_comm[mask].U = reference_comm_table[step].rv_U;
    r3ph.rv_comm[mask].V = reference_comm_table[step].rv_V;
    r3ph.rv_comm[mask].W = reference_comm_table[step].rv_W;

    r3ph.fw_comm[mask].U = reference_comm_table[step].fw_U;
    r3ph.fw_comm[mask].V = reference_comm_table[step].fw_V;
    r3ph.fw_comm[mask].W = reference_comm_table[step].fw_W;
  }

  return;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t BLDC_prepare_6step_tbls(void)
{
  uint8_t  b;
  uint32_t bldc_comm_law = wvar.bldc_comm_law;

  memset(g_check_arr, 0, sizeof(g_check_arr));

  for (uint32_t i=0; i < 6; i++)
  {
    b = bldc_comm_law % 10;
    if ((b < 1) || (b > 6)) goto err_;
    if (g_check_arr[b-1] != 0) goto err_;
    g_check_arr[b-1] = 1;
    r3ph.comm_law_arr[5-i] = b;
    bldc_comm_law = bldc_comm_law / 10;
  }
  _Prepare_6step_tbls();

  APPLOG("6-step commutation table accepted successfully: %d %d %d %d %d %d ",
         r3ph.comm_law_arr[0],
         r3ph.comm_law_arr[1],
         r3ph.comm_law_arr[2],
         r3ph.comm_law_arr[3],
         r3ph.comm_law_arr[4],
         r3ph.comm_law_arr[5]
        );
  return RES_OK;

err_:

  r3ph.comm_law_arr[0] = 2;
  r3ph.comm_law_arr[1] = 6;
  r3ph.comm_law_arr[2] = 4;
  r3ph.comm_law_arr[3] = 5;
  r3ph.comm_law_arr[4] = 1;
  r3ph.comm_law_arr[5] = 3;
  _Prepare_6step_tbls();

  APPLOG("6-step commutation table reading ERROR!");
  APPLOG("6-step default commutation table : %d %d %d %d %d %d ",
         r3ph.comm_law_arr[0],
         r3ph.comm_law_arr[1],
         r3ph.comm_law_arr[2],
         r3ph.comm_law_arr[3],
         r3ph.comm_law_arr[4],
         r3ph.comm_law_arr[5]
        );

  return RES_ERROR;
}

