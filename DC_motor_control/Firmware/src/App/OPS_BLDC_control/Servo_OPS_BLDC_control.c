// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-10-14
// 14:51:00
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void _Determine_opening_direction(void)
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

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Start_control(void)
{

  _Reset_control();
  Currents_offset_calibration();
  OPS_ret_pins_to_periph_mode();
  drv_cbl.state      = 1;

  APPLOG("Motor BLDC started");
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Wait_movement_end(uint32_t timeout_ms)
{
  uint32_t         actual_events;
  uint32_t         res;
  T_sys_timestump  tst;

  Get_hw_timestump(&tst);

  while (adc.shaft_abs_speed_smoothly > MAX_ALLOWABLE_SHAFT_SPEED_BEFOR_MOVEMENT)
  {
    res = App_wait_flags(BLDC_DRIVER_FAULT | MOTOR_HARD_STOP | MOTOR_FREEWHEELING | BLDC_SERVO_OPEN | BLDC_SERVO_CLOSE | MC_SPEED_LOOP_TICK, &actual_events, 10);
    if (res == TX_SUCCESS)
    {
      if (actual_events & BLDC_DRIVER_FAULT)
      {
        _Reset_control();
        OPS_freewheeling();
      }
      return RES_ERROR;
    }
    if (Time_elapsed_msec(&tst) > timeout_ms) return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _IsMoving_possible(uint8_t mov_dir)
{
  uint32_t app_err = 0;

  app_err = App_get_error_flags();
  if (app_err != 0)
  {
    mov_cbl.order = NO_ORDER;
    APPLOG("Execution aborted due to an system error %08X", app_err);
    return RES_ERROR;
  }


  if (mov_dir == NEED_TO_OPEN)
  {
    if (drv_cbl.opening_direction == OPENING_CLOCKWISE)
    {
      if (adc.smpl_POS_SENS >= wvar.open_position)
      {
        APPLOG("Execution aborted due to open position reached (%d >= %d)", adc.smpl_POS_SENS, wvar.open_position);
        mov_cbl.order = NO_ORDER;
        return RES_ERROR;
      }
    }
    else
    {
      if (adc.smpl_POS_SENS <= wvar.open_position)
      {
        APPLOG("Execution aborted due to open position reached (%d <= %d)", adc.smpl_POS_SENS, wvar.open_position);
        mov_cbl.order = NO_ORDER;
        return RES_ERROR;
      }
    }
    mov_cbl.order = ORDER_TO_OPEN;
  }
  else if (mov_dir == NEED_TO_CLOSE)
  {
    if (drv_cbl.opening_direction == OPENING_CLOCKWISE)
    {
      if (adc.smpl_POS_SENS <= wvar.close_position)
      {
        APPLOG("Execution aborted due to close position reached (%d <= %d)", adc.smpl_POS_SENS, wvar.close_position);
        mov_cbl.order = NO_ORDER;
        return RES_ERROR;
      }
    }
    else
    {
      if (adc.smpl_POS_SENS >= wvar.close_position)
      {
        APPLOG("Execution aborted due to close  position reached (%d >= %d)", adc.smpl_POS_SENS, wvar.close_position);
        mov_cbl.order = NO_ORDER;
        return RES_ERROR;
      }
    }
    mov_cbl.order = ORDER_TO_CLOSE;
  }

  if (adc.shaft_abs_speed_smoothly > MAX_ALLOWABLE_SHAFT_SPEED_BEFOR_MOVEMENT)
  {
    APPLOG("Execution aborted due to door movement (%0.1f degr/s)", (double)adc.shaft_abs_speed_smoothly);
    if (_Wait_movement_end(1000) != RES_OK) return RES_ERROR;
    return RES_ERROR;
  }

  return RES_OK;
}



/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _Exec_const_pwm_pulse_movement(uint8_t en_brake)
{
  T_sys_timestump  tst;
  uint32_t         elapsed_ms;
  uint32_t         res;
  uint32_t         actual_events;
  uint32_t         max_pwm = drv_cbl.pwm_val;
  uint32_t         cur_pwm = max_pwm;

  uint32_t app_err = App_get_error_flags();
  if (app_err != 0)
  {
    APPLOG("Execution aborted due to an driver error %08X", app_err);
    return;
  }

  drv_cbl.en_PID_speed_control       = 0;
  drv_cbl.en_PID_current_control     = 0;
  drv_cbl.en_ADRC_control            = 0;
  _Start_control();

  if (mov_cbl.moving_pulse_acc_ms != 0) cur_pwm  = 0;
  drv_cbl.pwm_val = cur_pwm;

  GPT0_update_comare_reg(drv_cbl.pwm_val);

  Get_hw_timestump(&tst);

  do
  {
    elapsed_ms = Time_elapsed_msec(&tst);

    if (elapsed_ms < mov_cbl.moving_pulse_acc_ms)
    {
      cur_pwm =(elapsed_ms * max_pwm) / mov_cbl.moving_pulse_acc_ms;
    }
    else if (elapsed_ms > (mov_cbl.moving_pulse_acc_ms + mov_cbl.moving_pulse_ms))
    {
      cur_pwm = max_pwm -((elapsed_ms-(mov_cbl.moving_pulse_acc_ms + mov_cbl.moving_pulse_ms)) * max_pwm) / mov_cbl.moving_pulse_dec_ms;
    }
    else
    {
      cur_pwm = max_pwm;
    }

    drv_cbl.pwm_val = cur_pwm;

    GPT0_update_comare_reg(drv_cbl.pwm_val);

    res = App_wait_flags(BLDC_DRIVER_FAULT | MOTOR_HARD_STOP, &actual_events, 1);
    if (res == TX_SUCCESS)
    {
      if (actual_events & BLDC_DRIVER_FAULT)
      {
        _Reset_control();
        OPS_freewheeling();
      }
      break;
    }

    Real_values_calculation();
  }while (elapsed_ms < (mov_cbl.moving_pulse_ms + mov_cbl.moving_pulse_acc_ms + mov_cbl.moving_pulse_dec_ms));

  if (en_brake)
  {
    _Reset_control();
    OPS_hard_stop();
  }
  else
  {
    _Reset_control();
    OPS_freewheeling();
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Exec_const_current_pulse_movement(uint8_t mov_dir)
{
  uint32_t   res;
  uint32_t   actual_events;
  T_sys_timestump  tst;

  uint32_t app_err = App_get_error_flags();
  if (app_err != 0)
  {
    APPLOG("Execution aborted due to an driver error %08X", app_err);
    return;
  }

  drv_cbl.en_PID_speed_control       = 0;
  drv_cbl.en_PID_current_control     = 1;
  drv_cbl.en_ADRC_control            = 0;
  _Start_control();

  Get_hw_timestump(&tst);
  drv_cbl.mot_rotation_target_dir = mov_dir ^ drv_cbl.opening_direction;
  drv_cbl.target_current          = CURRENT_PULSE_VALUE;

  Set_output_on(OUTP_LED_GREEN);

  do
  {
    res = App_wait_flags(BLDC_DRIVER_FAULT | MOTOR_HARD_STOP | MOTOR_FREEWHEELING, &actual_events, 2);
    if (res == TX_SUCCESS)
    {
      if (actual_events & BLDC_DRIVER_FAULT)
      {
        _Reset_control();
        OPS_freewheeling();
        break;
      }
      if (actual_events & MOTOR_FREEWHEELING)
      {
        _Reset_control();
        OPS_freewheeling();
        break;
      }
    }

    Real_values_calculation();

    if (Time_elapsed_msec(&tst) > CURRENT_PULSE_LEN_MS)
    {
      _Reset_control();
      OPS_hard_stop();
      break;
    }

  }while (1);

  Set_output_off(OUTP_LED_GREEN);
  drv_cbl.en_PID_speed_control = 1;
}

/*-----------------------------------------------------------------------------------------------------
   Выполняем движение

   Перед стартом проверяем условия готовносии к движению

   Необходимо чтобы:
   - не было перегревов
   - напряжения были в норме
   - не было слишком большого тока потребления
   - не было недопустимой позиции
   - не было вращения


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Exec_movement(uint8_t mov_dir)
{
  uint32_t         res;
  uint32_t         actual_events;


  if (_IsMoving_possible(mov_dir) != RES_OK) return;

  if (wvar.en_adrc)
  {
    ADRC_init(wvar.adrc_cbw, wvar.adrc_b0, wvar.adrc_pwm_lo_lim);
    drv_cbl.en_ADRC_control = 1;
  }
  else
  {
    drv_cbl.en_PID_speed_control   = 1;
    drv_cbl.en_PID_current_control = 1;
    Current_PID_init();
    Speed_PID_init();
  }
  Speed_Scurve_init(mov_dir);

  drv_cbl.mot_rotation_target_dir = mov_dir ^ drv_cbl.opening_direction;
  drv_cbl.direction               = drv_cbl.mot_rotation_target_dir;

  Get_hw_timestump(&mov_cbl.start_time);

  // Стартуем движение
  _Start_control();

  Set_output_on(OUTP_LED_GREEN);

  mov_cbl.order = NO_ORDER;

  do
  {
    // Ожидаем флага цикла расчета траектории скорости. Флаг поступает с частотой цикла управления скоростью
    res = App_wait_flags(BLDC_DRIVER_FAULT | MOTOR_HARD_STOP | MOTOR_FREEWHEELING | BLDC_SERVO_OPEN | BLDC_SERVO_CLOSE | MC_SPEED_LOOP_TICK, &actual_events, 10);

    Real_values_calculation();

    if (res == TX_SUCCESS)
    {
      if (actual_events & BLDC_DRIVER_FAULT)
      {
        _Reset_control();
        OPS_freewheeling();
        goto _exit;
      }

      if (actual_events & MC_SPEED_LOOP_TICK)
      {
        // Поступил сигнал цикла выполнения задачи
        // На каждом тике обновляем значение целевой сорости
        drv_cbl.target_rotation_speed = Speed_Scurve_step();
      }

      if ((actual_events & BLDC_SERVO_CLOSE) && (mov_dir == NEED_TO_OPEN))
      {
        // Прекращаем процесс поскольку поступила команда на закрытие во время открытия
        mov_cbl.order = ORDER_TO_CLOSE;
        _Reset_control();
        OPS_hard_stop();
        goto _exit;
      }

      if ((actual_events & BLDC_SERVO_OPEN) && (mov_dir == NEED_TO_CLOSE))
      {
        // Прекращаем процесс поскольку поступила команда на открытие во время закрытия
        mov_cbl.order = ORDER_TO_OPEN;
        _Reset_control();
        OPS_hard_stop();
        goto _exit;
      }


      if (actual_events & MOTOR_HARD_STOP)
      {
        _Reset_control();
        OPS_hard_stop();
        // Прекращаем процесс поскольку поступила команда остановки
        goto _exit;
      }

      if (actual_events & MOTOR_FREEWHEELING)
      {
        _Reset_control();
        OPS_freewheeling();
        // Прекращаем процесс поскольку поступила команда на свободное вращение
        goto _exit;
      }
    }

    // В процессе движения останавливаемся как только пересекли точку финиша
    if (mov_cbl.reverse_movement == 0)
    {
      if (adc.smpl_POS_SENS >= mov_cbl.stop_position)
      {
        // Прекращаем процесс поскольку механизм пересек точку финиша при прямом движении
        _Reset_control();
        OPS_hard_stop();
        goto _exit;
      }
    }
    else
    {
      if (adc.smpl_POS_SENS <= mov_cbl.stop_position)
      {
        // Прекращаем процесс поскольку механизм пересек точку финиша при реверсивном движении
        _Reset_control();
        OPS_hard_stop();
        goto _exit;
      }
    }

    if (Time_elapsed_sec(&mov_cbl.start_time) > wvar.movement_max_duration)
    {
      // Прекращаем процесс поскольку истекло время
      _Reset_control();
      OPS_hard_stop();
      break;
    }

  }while (1);


_exit:

  Set_output_off(OUTP_LED_GREEN);
  return;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Servo_OPS_BLDC_thread(void)
{
  uint32_t    actual_events;
  uint32_t    res;

  Filters_init();

  Hall_3PH_reinit();
  GPT_11_12_Capture_init(RPM_MEASURE_OVERFL_TIME);  // Инициализируем измерение скрости вращения двигателя по сигналам с датчиков Холла

  GPT0_PWM_w_OPS_init(wvar.pwm_frequency);          // Инициализируем PWM предназначенный для управления мотором
  OPS_init_module();

  ADC_init(OPS_BLDC_ISR_handler);                   // Инициализируем ADC работающий синхронно с PWM мотора

  _Determine_opening_direction();

  _Reset_control();

  drv_cbl.pwm_freq                = wvar.pwm_frequency;
  drv_cbl.mot_rotation_target_dir = drv_cbl.opening_direction;
  mov_cbl.moving_pulse_acc_ms     = 10;
  mov_cbl.moving_pulse_ms         = 100;
  mov_cbl.moving_pulse_dec_ms     = 10;


  GPT0_update_comare_reg(drv_cbl.pwm_val);
  MC50_OPS_PWM_pins_init();
  OPS_hard_stop();

  TMC6200_init();
  TMC6200_init_fault_interrut();
  Thread_TMC6200_create();
  Wait_ms(20); // Ожидаем пока запустится задача обслуживания IC драйвера

  Currents_offset_calibration();
  Calculating_scaling_factors();


  do
  {
    res = App_wait_flags(0x7FFFFFFF, &actual_events, 10);
    if (res == TX_SUCCESS)
    {

      if (actual_events & MOTOR_HARD_STOP)
      {
        GPT0_update_comare_reg(drv_cbl.pwm_val);
        _Reset_control();
        OPS_hard_stop();
      }

      if (actual_events & BLDC_MOVING_PULSE_HSTOP)
      {
        _Exec_const_pwm_pulse_movement(1);
      }

      if (actual_events & BLDC_MOVING_PULSE_FREE)
      {
        _Exec_const_pwm_pulse_movement(0);
      }

      if (actual_events & BLDC_SERVO_CLOSING_PULSE)
      {
        APPLOG("Execution of pulse to close direction");
        _Exec_const_current_pulse_movement(NEED_TO_CLOSE);
      }

      if (actual_events & BLDC_SERVO_OPENING_PULSE)
      {
        APPLOG("Execution of pulse to open direction");
        _Exec_const_current_pulse_movement(NEED_TO_OPEN);
      }

      if (actual_events & BLDC_SERVO_OPEN)
      {
        APPLOG("Execution movement to open direction");
        _Exec_movement(NEED_TO_OPEN);
      }

      if (actual_events & BLDC_SERVO_CLOSE)
      {
        APPLOG("Execution movement to close direction");
        _Exec_movement(NEED_TO_CLOSE);
      }

      if (actual_events & MOTOR_FREEWHEELING)
      {
        APPLOG("Execution of freewheeling");
        _Reset_control();
        OPS_freewheeling();
      }

      if (actual_events & BLDC_SERVO_FIX_CLOSE_POS)
      {
        APPLOG("Calibration of close position");
        wvar.close_position = adc.smpl_POS_SENS;
        _Determine_opening_direction();
        Request_save_app_settings();
      }

      if (actual_events & BLDC_SERVO_FIX_OPEN_POS)
      {
        APPLOG("Calibration of open position");
        wvar.open_position = adc.smpl_POS_SENS;
        _Determine_opening_direction();
        Request_save_app_settings();
      }

    }
    else
    {
      if (mov_cbl.order == ORDER_TO_CLOSE)
      {
        APPLOG("Execution movement to close direction");
        _Exec_movement(NEED_TO_CLOSE);
      }
      else if (mov_cbl.order == ORDER_TO_OPEN)
      {
        APPLOG("Execution movement to open direction");
        _Exec_movement(NEED_TO_OPEN);
      }

      Real_values_calculation();
      Temperatures_calculation();

    }

  } while (1);


}


