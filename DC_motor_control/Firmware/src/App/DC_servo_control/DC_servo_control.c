// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-10-13
// 18:40:24
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

#define  TRAJ_LIN_ACCELR      1
#define  TRAJ_LINEAR          2
#define  TRAJ_LIN_DECELR      3
#define  TRAJ_FINISH          4


T_sys_timestump        cooler_t;

/*-----------------------------------------------------------------------------------------------------
  Управление работой DC двигателя

  \param void
-----------------------------------------------------------------------------------------------------*/
static void _DC_motor_control(void)
{
  float pwm;


  mov_cbl.stage_time += 1000.0f / (float)(wvar.pwm_frequency);

  if (mov_cbl.traj_stage   == TRAJ_LIN_ACCELR)
  {
    if (mov_cbl.stage_time < wvar.dc_acceleration_time)
    {
      pwm = wvar.dc_motor_max_pwm * (1 -(wvar.dc_acceleration_time - mov_cbl.stage_time) / wvar.dc_acceleration_time);
    }
    else
    {
      pwm = wvar.dc_motor_max_pwm;
      mov_cbl.traj_stage = TRAJ_LINEAR;
    }
    drv_cbl.pwm_val = (uint32_t)pwm;
    drv_cbl.prev_pwm_val = (uint32_t)pwm;
  }

  if (mov_cbl.traj_stage   == TRAJ_LIN_DECELR)
  {
    if (mov_cbl.stage_time < wvar.dc_deceleration_time)
    {
      pwm = drv_cbl.prev_pwm_val * ((wvar.dc_deceleration_time - mov_cbl.stage_time) / wvar.dc_deceleration_time);
    }
    else
    {
      pwm = wvar.dc_motor_max_pwm;
      mov_cbl.traj_stage = TRAJ_LINEAR;
      App_set_flags(MOTOR_HARD_STOP);
    }
    drv_cbl.pwm_val      = (uint32_t)pwm;
  }

  if (drv_cbl.pwm_val < 100)
  {
    if (drv_cbl.direction == 0)
    {
      Phase_U_mode(PHASE_PULL_DOWN);
      Phase_V_mode(drv_cbl.pwm_val);
    }
    else
    {
      Phase_U_mode(drv_cbl.pwm_val);
      Phase_V_mode(PHASE_PULL_DOWN);
    }
  }
  else
  {
    if (drv_cbl.direction == 0)
    {
      Phase_U_mode(PHASE_PULL_DOWN);
      Phase_V_mode(PHASE_PULL_UP);
    }
    else
    {
      Phase_U_mode(PHASE_PULL_UP);
      Phase_V_mode(PHASE_PULL_DOWN);
    }
  }

}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void  DC_motor_ISR_handler(void)
{
  Measure_instant_phases_current();
  Measure_DC_motors_current();
  Measure_DC_bus_voltage_current();
  Measure_servo_sensor_speed();
  QEnc_measure_speed_and_direction();

  if (drv_cbl.state != 0)
  {
    _DC_motor_control();
  }
  Set_output_U(gpt0_out_mode);
  Set_output_V(gpt1_out_mode);
  Set_output_W(gpt2_out_mode);

}

/*-----------------------------------------------------------------------------------------------------


  \param v
-----------------------------------------------------------------------------------------------------*/
static void _Set_cooler_state(uint8_t v)
{
  if (v)
  {
    Phase_W_mode(PHASE_PULL_UP);
    Get_hw_timestump(&cooler_t);
  }
  else
  {
    Phase_W_mode(PHASE_PULL_DOWN);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Exec_hard_stop(void)
{
  drv_cbl.state  = 0;
  Phase_U_mode(PHASE_PULL_DOWN);
  Phase_V_mode(PHASE_PULL_DOWN);
  _Set_cooler_state(1);

  APPLOG("Motor stopped");
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Exec_freewheeling(void)
{
  drv_cbl.state = 0;
  Phase_U_mode(PHASE_Z_STATE);
  Phase_V_mode(PHASE_Z_STATE);
  _Set_cooler_state(1);

  APPLOG("Motor freewheeling");
}

/*-----------------------------------------------------------------------------------------------------
  Сарт вращения DC мотора 1
  Подключен между фазами  V - U

  rot_dir = 0 означает вращение по часовой стрелке.     Фаза V притягивается к земле   без ШИМ
  rot_dir = 1 означает вращение против часовой стрелке. Фаза V притягивается к питанию без ШИМ

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Exec_start_motor(uint8_t rot_dir)
{

  if (drv_cbl.state != 0)
  {
    App_set_flags(DC_ROT_DECELER);
    return RES_ERROR;
  }

  _Set_cooler_state(1);

  drv_cbl.direction = rot_dir;

  Currents_offset_calibration();

  // Начинаем с нулевого уровня модуляции для плавного запуска мотора
  drv_cbl.pwm_val      = 0;
  mov_cbl.traj_stage   = TRAJ_LIN_ACCELR;
  mov_cbl.stage_time   = 0;
  drv_cbl.state        = 1;

  APPLOG("Motor started");
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param rot_dir

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Exec_start_motor_no_accel(uint8_t rot_dir)
{

  if (drv_cbl.state != 0)
  {
    return RES_ERROR;
  }

  _Set_cooler_state(1);

  drv_cbl.direction = rot_dir;

  Currents_offset_calibration();

  // Начинаем с нулевого уровня модуляции для плавного запуска мотора
  mov_cbl.traj_stage   = TRAJ_LINEAR;
  mov_cbl.stage_time   = 0;
  drv_cbl.state   = 1;

  APPLOG("Motor started");
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------


  \param rot_dir

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Exec_motor_deceleration(void)
{
  if (mov_cbl.traj_stage  != TRAJ_LIN_DECELR)
  {
    mov_cbl.traj_stage        = TRAJ_LIN_DECELR;
    mov_cbl.stage_time        = 0;
  }
  APPLOG("Motor deceleration");
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Reset_control(void)
{
  drv_cbl.state                 = 0;
  drv_cbl.pwm_val               = 0;
  drv_cbl.prev_pwm_val          = 0;

  APPLOG("Motor control reset");
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void DC_motor_thread(void)
{
  uint32_t    actual_events;
  uint32_t    res;

  Filters_init();

  QEnc_reinit();
  GPT_11_12_Capture_init(RPM_MEASURE_OVERFL_TIME);                  // Инициализируем измерение скрости вращения двигателя по сигналам с датчиков Холла

  GPT_0_1_2_PWM_init(wvar.pwm_frequency);
  Phase_U_mode(PHASE_PULL_DOWN);
  Phase_V_mode(PHASE_PULL_DOWN);
  Phase_W_mode(PHASE_PULL_DOWN);
  GPT_0_1_2_PWM_start();

  ADC_init(DC_motor_ISR_handler);            // Инициализируем ADC работающий синхронно с PWM мотора

  _Reset_control();

  drv_cbl.pwm_freq                = wvar.pwm_frequency;
  drv_cbl.mot_rotation_target_dir = drv_cbl.opening_direction;
  mov_cbl.moving_pulse_acc_ms     = 10;
  mov_cbl.moving_pulse_ms         = 100;
  mov_cbl.moving_pulse_dec_ms     = 10;


  GPT0_update_comare_reg(drv_cbl.pwm_val);
  MC50_3Ph_PWM_pins_init();

  _Exec_hard_stop();


  TMC6200_init();
  TMC6200_init_fault_interrut();
  Thread_TMC6200_create();
  Wait_ms(20); // Ожидаем пока запустится задача обслуживания IC драйвера

  Currents_offset_calibration();
  Calculating_scaling_factors();
  Get_hw_timestump(&cooler_t);

  do
  {
    res = App_wait_flags(0x7FFFFFFF,&actual_events, 10);
    if (res == TX_SUCCESS)
    {

      if (actual_events & MOTOR_HARD_STOP)
      {
        _Exec_hard_stop();
      }

      if (actual_events & MOTOR_FREEWHEELING)
      {
        _Exec_freewheeling();
      }

      if (actual_events & DC_ROT_CW)
      {
        APPLOG("Execution rotation CW");
        _Exec_start_motor(0);
      }

      if (actual_events & DC_ROT_CCW)
      {
        APPLOG("Execution rotation CCW");
        _Exec_start_motor(1);
      }

      if (actual_events & DC_ROT_CW_NO_ACCEL)
      {
        APPLOG("Execution rotation CW without acceleration");
        _Exec_start_motor_no_accel(0);
      }

      if (actual_events & DC_ROT_CCW_NO_ACCEL)
      {
        APPLOG("Execution rotation CCW without acceleration");
        _Exec_start_motor_no_accel(1);
      }


      if (actual_events & DC_ROT_DECELER)
      {
        APPLOG("Execution rotation ending");
        _Exec_motor_deceleration();
      }

    }
    else
    {
      Real_values_calculation();
      Temperatures_calculation();

      if (drv_cbl.state == 0)
      {
        if (Time_elapsed_sec(&cooler_t) > 60)
        {
          _Set_cooler_state(0);
        }
      }

    }

  } while (1);


}

