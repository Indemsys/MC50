// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-10-19
// 17:50:02
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Determine_opening_direction(void)
{
  if (wvar.close_position < wvar.open_position)
  {
    drv_cbl.opening_direction        = OPENING_CLOCKWISE;
    drv_cbl.speed_sign_on_open_dir   = OPENING_CLOCKWISE_SPEED_SIGN;
  }
  else
  {
    drv_cbl.opening_direction        = OPENING_COUNTERCLOCKWISE;
    drv_cbl.speed_sign_on_open_dir   = OPENING_COUNTERCLOCKWISE_SPEED_SIGN;
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
void Servo_BLDC_thread(void)
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

  ADC_init(BLDC_ISR_handler);                   // Инициализируем ADC работающий синхронно с PWM мотора

  _Determine_opening_direction();

  _Reset_control();

  drv_cbl.pwm_freq                = wvar.pwm_frequency;
  drv_cbl.mot_rotation_target_dir = drv_cbl.opening_direction;
  mov_cbl.moving_pulse_acc_ms     = 10;
  mov_cbl.moving_pulse_ms         = 100;
  mov_cbl.moving_pulse_dec_ms     = 10;


  MC50_3Ph_PWM_pins_init();


  TMC6200_init();
  TMC6200_init_fault_interrut();
  Thread_TMC6200_create();
  Wait_ms(20); // Ожидаем пока запустится задача обслуживания IC драйвера

  Currents_offset_calibration();
  Calculating_scaling_factors();


  do
  {
    res = App_wait_flags(COMMUTATION_LAW_SEARCH  ,&actual_events, 10);
    if (res == TX_SUCCESS)
    {


    }
    else
    {
      Real_values_calculation();
      Temperatures_calculation();
    }

  } while (1);


}



