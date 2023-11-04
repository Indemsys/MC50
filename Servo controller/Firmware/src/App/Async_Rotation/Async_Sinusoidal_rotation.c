// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-10-28
// 13:45:54
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


extern uint32_t             pwm_indx_to_comp[PWM_STEP_COUNT];
T_sinusoidal_rotation       sr;

static inline void Set_gpt_pwm_val(R_GPTA0_Type *R_GPT, uint32_t top_comp_value, uint32_t pwm_lev);


static uint8_t              en_sinusoidal_rotation;

/*-----------------------------------------------------------------------------------------------------
  Функция вызываемая в обработчике прерываний по окончании выборок АЦП
  Вызывается синхронно с ШИМ с частотой ШИМ

  Длительность прерывания 9.16 us если нет генерации синусоиды
  16...21 us если включена генерация синусоиды
  Общая загрузка процессора с включенной синусоидой = 68%
  С опцией компилятора полной оптимизации по скорости загрузка процессора с включенной синусоидой = 53%
  С опцией компилятора полной оптимизации по скорости без вставок ITM_EVENT8 загрузка процессора и с включенной синусоидой = 48%

  \param void
-----------------------------------------------------------------------------------------------------*/
static void BLDC_SIN_ROT_ISR_handler(void)
{
  //ITM_EVENT8(1,0);
  Hall_3PH_capture_bitmask();                // 0.9 us
  //ITM_EVENT8(1,1);
  Measure_instant_phases_current();          // 0.38 us
  //ITM_EVENT8(1,2);
  Measure_overall_BLDC_motor_current();      // 1.09 us
  //ITM_EVENT8(1,3);
  Measure_DC_bus_voltage_current();          // 0.94 us
  //ITM_EVENT8(1,4);
  Measure_servo_sensor_speed();              // 2.35 us
  //ITM_EVENT8(1,5);
  Hall_3PH_measure_speed_and_direction();    // 0.76 us
  //ITM_EVENT8(1,6);


  if (en_sinusoidal_rotation)
  {
    r3ph.norm_angle = SinGen_get_3ph_sin(r3ph.norm_angle, r3ph.angle_delta,&r3ph.angle_3ph);     // 0.66 us Получаем значения углов трех синусоид
    //ITM_EVENT8(1,7);
    PWM_modulator_integer(&r3ph.angle_3ph, r3ph.sin_pwm_type ,&r3ph.pwm_3ph_norm, NULL);     // 5.0 us Получаем значения нормированных коэфициентов ШИМ
    //ITM_EVENT8(1,8);
    PWM_converter(&r3ph.pwm_3ph_norm, HALF_PWM_STEP_COUNT , r3ph.ampl_scale, &r3ph.pwm_3ph);  // 0.81 us Конвертивруем нормированные коэфициенты ШИМ в реальные
    //ITM_EVENT8(1,9);
    // 0.2 us
    r3ph.gpt_U_pwm_val = r3ph.pwm_3ph.pa;
    r3ph.gpt_V_pwm_val = r3ph.pwm_3ph.pb;
    r3ph.gpt_W_pwm_val = r3ph.pwm_3ph.pc;
    //ITM_EVENT8(1,10);

  }

  switch (r3ph.out_U_state_req)
  {
  case OUT_TO_ENABLE:
    R_GPTA0->GTIOR = R_GPTA0->GTIOR | (BIT(8) | BIT(24));// Установка битов GTIOCA Pin Output Enable и GTIOCB Pin Output Enable
    r3ph.out_U_state_req = OUT_NO_ACTION;
    break;
  case OUT_TO_DISABLE:
    R_GPTA0->GTIOR = R_GPTA0->GTIOR &~(BIT(8) | BIT(24));// Сброс битов GTIOCA Pin Output Enable и GTIOCB Pin Output Enable
    r3ph.out_U_state_req = OUT_NO_ACTION;
    break;
  }
  switch (r3ph.out_V_state_req)
  {
  case OUT_TO_ENABLE:
    R_GPTA1->GTIOR = R_GPTA1->GTIOR | (BIT(8) | BIT(24));// Установка битов GTIOCA Pin Output Enable и GTIOCB Pin Output Enable
    r3ph.out_V_state_req = OUT_NO_ACTION;
    break;
  case OUT_TO_DISABLE:
    R_GPTA1->GTIOR = R_GPTA1->GTIOR &~(BIT(8) | BIT(24));// Сброс битов GTIOCA Pin Output Enable и GTIOCB Pin Output Enable
    r3ph.out_V_state_req = OUT_NO_ACTION;
    break;
  }
  switch (r3ph.out_W_state_req)
  {
  case OUT_TO_ENABLE:
    R_GPTA2->GTIOR = R_GPTA2->GTIOR | (BIT(8) | BIT(24));// Установка битов GTIOCA Pin Output Enable и GTIOCB Pin Output Enable
    r3ph.out_W_state_req = OUT_NO_ACTION;
    break;
  case OUT_TO_DISABLE:
    R_GPTA2->GTIOR = R_GPTA2->GTIOR &~(BIT(8) | BIT(24));// Сброс битов GTIOCA Pin Output Enable и GTIOCB Pin Output Enable
    r3ph.out_W_state_req = OUT_NO_ACTION;
    break;
  }

  // Установка всех 3 фаз длится 0.98 us
  Set_gpt_pwm_val(R_GPTA0, r3ph.gpt_top_val, r3ph.gpt_U_pwm_val);                    // Обновление регистра копаратора фазы U
  Set_gpt_pwm_val(R_GPTA1, r3ph.gpt_top_val, r3ph.gpt_V_pwm_val);                    // Обновление регистра копаратора фазы V
  Set_gpt_pwm_val(R_GPTA2, r3ph.gpt_top_val, r3ph.gpt_W_pwm_val);                    // Обновление регистра копаратора фазы W
  //ITM_EVENT8(1,11);

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
static void Sinusoidal_rotation_start(void)
{
  Currents_offset_calibration();
  r3ph.rotation_freq     = sr.rot_freq * wvar.bldc_mot_pole_num / 2;
  r3ph.ampl_scale             = sr.scale;
  r3ph.norm_angle            = 0;
  r3ph.sin_pwm_type          = sr.pwm_mode;
  r3ph.angle_delta       = SinGen_get_delta(r3ph.rotation_freq, wvar.pwm_frequency);
  en_sinusoidal_rotation = 1;
  BLDC_set_outputs_state(OUT_TO_ENABLE);

}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Sinusoidal_rotation_hard_stop(void)
{
  en_sinusoidal_rotation = 0;
  BLDC_set_outputs_state(OUT_TO_ENABLE);
  PWM_send(PWM_0, PWM_0, PWM_0);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Sinusoidal_rotation_freewheeling(void)
{
  en_sinusoidal_rotation = 0;
  BLDC_set_outputs_state(OUT_TO_DISABLE);
  PWM_send(PWM_0, PWM_0, PWM_0);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Async_Sinusoidal_rotation_control(void)
{
  if (sr.set_rot_freq == 0)
  {
    sr.set_rot_freq  = DEF_SIN_ROT_FREQ;
    sr.rot_freq      = sr.set_rot_freq;
    r3ph.angle_delta = SinGen_get_delta(sr.rot_freq * wvar.bldc_mot_pole_num / 2 , wvar.pwm_frequency);
  }
  else if (sr.set_rot_freq != sr.rot_freq)
  {
    if (sr.set_rot_freq > wvar.bldc_max_rot_speed)
    {
      sr.set_rot_freq = wvar.bldc_max_rot_speed;
    }
    else if (sr.set_rot_freq < MIR_ROT_SPEED)
    {
      sr.set_rot_freq = MIR_ROT_SPEED;
    }
    sr.rot_freq      = sr.set_rot_freq;
    r3ph.angle_delta = SinGen_get_delta(sr.rot_freq * wvar.bldc_mot_pole_num / 2 , wvar.pwm_frequency);
  }

  if (sr.set_rot_ampl == 0)
  {
    sr.set_rot_ampl = DEF_SIN_ROT_AMP;
    sr.rot_ampl     = sr.set_rot_ampl;
    sr.scale        = lroundf(((sr.rot_ampl / 100.0f) * (float)0x7FFFFFFF));
    r3ph.ampl_scale      = sr.scale;
  }
  else if (sr.set_rot_ampl != sr.rot_ampl)
  {
    if (sr.set_rot_ampl > 100.0f)
    {
      sr.set_rot_ampl = 100.0f;
    }
    else if (sr.set_rot_ampl < 1.0f)
    {
      sr.set_rot_ampl = 1.0f;
    }
    sr.rot_ampl = sr.set_rot_ampl;
    sr.scale    = lroundf(((sr.rot_ampl / 100.0f) * (float)0x7FFFFFFF));
    r3ph.ampl_scale  = sr.scale;
  }

  if (sr.set_pwm_mode == 0)
  {
    sr.set_pwm_mode = MDL_CSVPWM;
    sr.pwm_mode     = sr.set_pwm_mode;
    r3ph.sin_pwm_type   = sr.set_pwm_mode;
  }
  else if (sr.set_pwm_mode != sr.pwm_mode)
  {
    if (sr.set_pwm_mode < MDL_CSVPWM)
    {
      sr.set_pwm_mode = MDL_CSVPWM;
    }
    else if (sr.set_pwm_mode > MDL_DPWM_120_BOT)
    {
      sr.set_pwm_mode = MDL_DPWM_120_BOT;
    }
    sr.pwm_mode     = sr.set_pwm_mode;
    r3ph.sin_pwm_type   = sr.set_pwm_mode;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Asinc_Sinusoidal_BLDC_rotation_thread(void)
{
  uint32_t    actual_events;
  uint32_t    res;

  Filters_init();

  Hall_3PH_reinit();
  Rot_Encoders_capturing_init(RPM_MEASURE_OVERFL_TIME);  // Инициализируем измерение скрости вращения двигателя по сигналам с датчиков Холла

  if (PWM_3ph_triangle_2buffered_init(wvar.pwm_frequency) != RES_OK) return;
  PWM_send(PWM_0, PWM_0, PWM_0);
  PWM_3ph_start();

  ADC_init(BLDC_SIN_ROT_ISR_handler);                   // Инициализируем ADC работающий синхронно с PWM мотора


  MC50_3Ph_PWM_pins_init();

  TMC6200_init();
  TMC6200_init_fault_interrut();
  Thread_TMC6200_create();
  Wait_ms(20); // Ожидаем пока запустится задача обслуживания IC драйвера

  Currents_offset_calibration();
  Calculating_scaling_factors();

  Async_Sinusoidal_rotation_control();
  do
  {
    res = App_wait_flags(SINUSOIDAL_ROT_ON |  MOTOR_HARD_STOP | MOTOR_FREEWHEELING,&actual_events, 10);
    if (res == TX_SUCCESS)
    {
      if (actual_events & SINUSOIDAL_ROT_ON)
      {
        Sinusoidal_rotation_start();
      }
      if (actual_events & MOTOR_HARD_STOP)
      {
        Sinusoidal_rotation_hard_stop();
      }
      if (actual_events & MOTOR_FREEWHEELING)
      {
        Sinusoidal_rotation_freewheeling();
      }

    }
    else
    {
      Async_Sinusoidal_rotation_control();
      Real_values_calculation();
      Temperatures_calculation();
    }

  } while (1);

}






