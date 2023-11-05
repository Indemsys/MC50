// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-10-19
// 17:50:02
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


float   g_test_fp_pwm;
int32_t g_test_pwm_val;

static uint8_t enable_rotation;
static uint8_t reverse_rotation;


static inline void Set_gpt_pwm_val(R_GPTA0_Type *R_GPT, uint32_t top_comp_value, uint32_t pwm_lev);
static void        BLDC_test_6step_commutation(void);
/*-----------------------------------------------------------------------------------------------------
  Функция вызываемая в обработчике прерываний по окончании выборок АЦП
  Вызывается синхронно с ШИМ с частотой ШИМ


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _BLDC_test_ISR_handler(void)
{
  Hall_3PH_capture_bitmask();                // 0.9 us
  Measure_instant_phases_current();          // 0.38 us
  Measure_overall_BLDC_motor_current();      // 1.09 us
  Measure_DC_bus_voltage_current();          // 0.94 us
  Measure_servo_sensor_speed();              // 2.35 us
  Hall_3PH_measure_speed_and_direction();    // 0.76 us

  if (enable_rotation)
  {
    BLDC_test_6step_commutation();
  }

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


  \param void
-----------------------------------------------------------------------------------------------------*/
static void BLDC_test_6step_commutation(void)
{
  T_6step_rec  *tbl;
  uint8_t hall_mask = h3.bitmask;

  if (r3ph.bitmask == hall_mask) return;
  r3ph.bitmask = hall_mask;

  if ((hall_mask < 1) || (hall_mask > 6)) return;
  hall_mask--;

  if (reverse_rotation)
  {
    tbl = r3ph.rv_comm;
  }
  else
  {
    tbl = r3ph.fw_comm;
  }

  if (tbl[hall_mask].U == 1)
  {
    r3ph.gpt_U_pwm_val = g_test_pwm_val;
    r3ph.out_U_state_req = OUT_TO_ENABLE;
  }
  else if (tbl[hall_mask].U == -1)
  {
    r3ph.gpt_U_pwm_val = 0;
    r3ph.out_U_state_req = OUT_TO_ENABLE;
  }
  else
  {
    r3ph.out_U_state_req = OUT_TO_DISABLE;
    // Заранее переключаем уровень ШИМ, чтобы не появилось нежелательных импульсов при выходе из Z состояния
    if (r3ph.gpt_U_pwm_val == 0)
    {
      r3ph.gpt_U_pwm_val = g_test_pwm_val;
    }
    else
    {
      r3ph.gpt_U_pwm_val = 0;
    }
  }

  if (tbl[hall_mask].V == 1)
  {
    r3ph.gpt_V_pwm_val = g_test_pwm_val;
    r3ph.out_V_state_req = OUT_TO_ENABLE;
  }
  else if (tbl[hall_mask].V == -1)
  {
    r3ph.gpt_V_pwm_val = 0;
    r3ph.out_V_state_req = OUT_TO_ENABLE;
  }
  else
  {
    r3ph.out_V_state_req = OUT_TO_DISABLE;
    // Заранее переключаем уровень ШИМ, чтобы не появилось нежелательных импульсов при выходе из Z состояния
    if (r3ph.gpt_V_pwm_val == 0)
    {
      r3ph.gpt_V_pwm_val = g_test_pwm_val;
    }
    else
    {
      r3ph.gpt_V_pwm_val = 0;
    }
  }

  if (tbl[hall_mask].W == 1)
  {
    r3ph.gpt_W_pwm_val = g_test_pwm_val;
    r3ph.out_W_state_req = OUT_TO_ENABLE;
  }
  else if (tbl[hall_mask].W == -1)
  {
    r3ph.gpt_W_pwm_val = 0;
    r3ph.out_W_state_req = OUT_TO_ENABLE;
  }
  else
  {
    r3ph.out_W_state_req = OUT_TO_DISABLE;
    // Заранее переключаем уровень ШИМ, чтобы не появилось нежелательных импульсов при выходе из Z состояния
    if (r3ph.gpt_W_pwm_val == 0)
    {
      r3ph.gpt_W_pwm_val = g_test_pwm_val;
    }
    else
    {
      r3ph.gpt_W_pwm_val = 0;
    }
  }


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
static void _Reset_control(void)
{
  drv_cbl.state                 = 0;
  drv_cbl.current_loop_e        = 0;
  drv_cbl.speed_loop_e          = 0;
  drv_cbl.target_rotation_speed = 0.0f;
  drv_cbl.en_ADRC_control       = 0;
  drv_cbl.pwm_val               = 0;
  drv_cbl.prev_pwm_val          = 0;
  drv_cbl.speed_loop_divider    = 0;
  drv_cbl.current_loop_divider  = 0;
  drv_cbl.target_current        = 0;
  sadrc_U.speed                 = 0;
  sadrc_U.ref_speed             = 0;

  Current_PID_clear();
  Speed_PID_clear();

  APPLOG("Motor control reset");
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _BLDC_rotor_hard_stop(void)
{
  enable_rotation = 0;
  BLDC_set_outputs_state(OUT_TO_ENABLE);
  PWM_send(PWM_0, PWM_0, PWM_0);
  APPLOG("Test:  Motor hard stop");
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _BLDC_rotor_freewheeling(void)
{

  enable_rotation = 0;
  BLDC_set_outputs_state(OUT_TO_DISABLE);
  PWM_send(PWM_0, PWM_0, PWM_0);
  APPLOG("Test:  Motor freewheeling");

}

/*-----------------------------------------------------------------------------------------------------


  \param dir
-----------------------------------------------------------------------------------------------------*/
static void _BLDC_start_rotation(uint8_t dir)
{
  if (enable_rotation == 0)
  {
    APPLOG("Test:  Motor rotation start. Direction = %d", dir);
    if (g_test_pwm_val == 0)
    {
      if (Get_test_pwm_val(&g_test_fp_pwm, &g_test_pwm_val)!= RES_OK)
      {
        APPLOG("Test:  Сommand was canceled due to an error during pwm setting");
        return;
      }
    }

    r3ph.bitmask     = 0; // Обнуляем битовую маску датчиков холла, чтобы инициализировать начало процесса коммутации
    reverse_rotation = dir;
    enable_rotation  = 1;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void BLDC_mot_test_thread(void)
{
  uint32_t    actual_events;
  uint32_t    res;

  BLDC_prepare_6step_tbls();

  Filters_init();

  Hall_3PH_reinit();
  Rot_Encoders_capturing_init(RPM_MEASURE_OVERFL_TIME);  // Инициализируем измерение скрости вращения двигателя по сигналам с датчиков Холла

  if (PWM_3ph_triangle_2buffered_init(wvar.pwm_frequency) != RES_OK) return;
  PWM_send(PWM_0, PWM_0, PWM_0);
  PWM_3ph_start();

  ADC_init(_BLDC_test_ISR_handler);                   // Инициализируем ADC работающий синхронно с PWM мотора


  _Reset_control();

  drv_cbl.pwm_freq                = wvar.pwm_frequency;
  drv_cbl.mot_rotation_target_dir = drv_cbl.opening_direction;


  MC50_3Ph_PWM_pins_init();


  TMC6200_init();
  TMC6200_init_fault_interrut();
  Thread_TMC6200_create();
  Wait_ms(20); // Ожидаем пока запустится задача обслуживания IC драйвера

  Currents_offset_calibration();
  Calculating_scaling_factors();

  APPLOG("Test:  Task start");

  do
  {
    res = App_wait_flags(COMMUTATION_LAW_SEARCH | BLDC_ROT_CW | BLDC_ROT_CCW | MOTOR_HARD_STOP | MOTOR_FREEWHEELING  ,&actual_events, 10);
    if (res == TX_SUCCESS)
    {
      if (actual_events & COMMUTATION_LAW_SEARCH)
      {
        if (enable_rotation == 0)
        {
          if (Commutation_Law_search()==RES_OK)
          {
            Store_6step_tbl_to_wvar();
            BLDC_prepare_6step_tbls();
          }
        }
      }
      if (actual_events & BLDC_ROT_CW)
      {
        _BLDC_start_rotation(0);
      }
      if (actual_events & BLDC_ROT_CCW)
      {
        _BLDC_start_rotation(1);
      }
      if (actual_events & MOTOR_HARD_STOP)
      {
        _BLDC_rotor_hard_stop();
      }
      if (actual_events & MOTOR_FREEWHEELING)
      {
        _BLDC_rotor_freewheeling();
      }
    }
    else
    {
      Real_values_calculation();
      Temperatures_calculation();
    }

  } while (1);


}



