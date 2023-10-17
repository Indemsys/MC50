// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-10-14
// 13:20:13
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


PIDParams       current_PID_cbl;


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Current_PID_init(void)
{
  current_PID_cbl.Kp          = wvar.i_k_prop;
  current_PID_cbl.Ki          = wvar.i_k_integr;
  current_PID_cbl.Kd          = wvar.i_k_diff;
  current_PID_cbl.Kn          = wvar.i_k_filter;
  current_PID_cbl.OutUpLim    = wvar.i_max_out;
  current_PID_cbl.OutLoLim    = wvar.i_min_out;
  current_PID_cbl.OutRateLim  = wvar.i_max_slew_rate;
  current_PID_cbl.smpl_time   = 1.0f / (float)wvar.pwm_frequency;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Current_PID_clear(void)
{
  current_PID_cbl.PID_state.Del2_DSTATE    = 0;
  current_PID_cbl.PID_state.Integr1_states = 0;
  current_PID_cbl.PID_state.Integr2_states = 0;
}

/*-----------------------------------------------------------------------------------------------------
  Выполнение функции петли управления током мотора
  Функция вызывается из прерывания АЦП с частотой PWM_FREQ_HZ

  \return float
-----------------------------------------------------------------------------------------------------*/
uint32_t Current_PID_step(void)
{
  float pid_output;

  #ifdef USE_FILTERED_CURRENT_IN_CURRENT_LOOP
  drv_cbl.current_loop_e = drv_cbl.target_current - adc.filtered_bldc_motor_current;
  #else
  drv_cbl.current_loop_e = drv_cbl.target_current -(adc.instant_bldc_motor_current * adc.current_shunt_scale);
  #endif

  PIDController_custom(drv_cbl.current_loop_e, current_PID_cbl.smpl_time,&current_PID_cbl,&pid_output);

  // Это условие обязательно. Без него начинаются сильные осцилляции направления вращения
  if (pid_output <= 0.0f)
  {
    pid_output= 0.0f;
  }

  if (pid_output < 0)
  {
    drv_cbl.direction = drv_cbl.mot_rotation_target_dir ^ 1;
  }
  else
  {
    drv_cbl.direction = drv_cbl.mot_rotation_target_dir;
  }
  return lrintf(pid_output);
}



