// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-10-14
// 13:20:05
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

PIDParams       speed_PID_cbl;




/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Speed_PID_init(void)
{
  speed_PID_cbl.Kp          = wvar.s_k_prop;
  speed_PID_cbl.Ki          = wvar.s_k_integr;
  speed_PID_cbl.Kd          = wvar.s_k_diff;
  speed_PID_cbl.Kn          = wvar.s_k_filter;
  speed_PID_cbl.OutUpLim    = wvar.s_max_out;
  speed_PID_cbl.OutLoLim    = wvar.s_min_out;
  speed_PID_cbl.OutRateLim  = wvar.s_max_slew_rate;
  speed_PID_cbl.smpl_time   = 1.0f / (float)SPEED_LOOP_FREQ_HZ;
}


/*-----------------------------------------------------------------------------------------------------


  \param p_pid_pars
-----------------------------------------------------------------------------------------------------*/
void Speed_PID_clear(void)
{
  speed_PID_cbl.PID_state.Del2_DSTATE    = 0;
  speed_PID_cbl.PID_state.Integr1_states = 0;
  speed_PID_cbl.PID_state.Integr2_states = 0;
}


/*-----------------------------------------------------------------------------------------------------
  Выполнение функции петли управления скростью вращения
  Функция вызывается из прерывания АЦП с частотой PWM_FREQ_HZ

  \param void
-----------------------------------------------------------------------------------------------------*/
void Speed_PID_step(void)
{
  float           pid_output;
  float           current_speed;

  // Направление вращения задается переменной mot_cbl.target_direction
  // Приводим скорость к положительной величине
  if (drv_cbl.mot_rotation_target_dir == drv_cbl.opening_direction)
  {
    current_speed = adc.shaft_speed_rt * drv_cbl.speed_sign_on_open_dir;
  }
  else
  {
    current_speed = adc.shaft_speed_rt * drv_cbl.speed_sign_on_open_dir * -1.0f;
  }
  drv_cbl.speed_loop_e =(drv_cbl.target_rotation_speed - current_speed);
  PIDController_custom(drv_cbl.speed_loop_e, speed_PID_cbl.smpl_time,&speed_PID_cbl,&pid_output);

  drv_cbl.target_current = pid_output;

}

