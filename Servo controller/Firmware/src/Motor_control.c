// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-06-25
// 15:10:22
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


T_mot_control_cbl  mot_cbl;


#define   MC_START_MOTOR             BIT(0)
#define   MC_STOP_MOTOR              BIT(1)
#define   MC_CHANGE_PWM              BIT(2)  // Флаг посылаемый при ручном изменении скважности ШИМ
#define   MC_MOVING_PULSE            BIT(3)
#define   MC_DRIVER_FAULT            BIT(4)

#define   MC_OPEN                    BIT(6)
#define   MC_CLOSE                   BIT(7)

#define   MC_EXEC_CLOSING_PULSE      BIT(8)
#define   MC_EXEC_OPENING_PULSE      BIT(9)
#define   MC_FIX_CLOSE_POSITION      BIT(10)
#define   MC_FIX_OPEN_POSITION       BIT(11)
#define   MC_FREEWHEELING            BIT(12)


#define   MC_SPEED_LOOP_TICK         BIT(31)

TX_EVENT_FLAGS_GROUP   mc_flags;


//uint8_t forward_rotation_tbl[8] =  {0, 1, 2, 3, 4, 5, 6, 0}; // Эта таблица тоже вращает мотор но с большей частотой и большим в три раза потреблением тока
const uint8_t forward_rotation_tbl[8] = {0, 5, 3, 1, 6, 4, 2, 0};
const uint8_t reverse_rotation_tbl[8] = {0, 2, 4, 6, 1, 3, 5, 0};

// Таблицы направления токов в фазах драйвера мотора в зависимости от маски состояния сигналов с датчиков холла
// в соответсвтии с внутренней прошитой таблицей дектодирования в микроконтроллере
// Значение 1 соответствует току втекающему в обмотку, -1 - обозначает ток вытекающий из обмотки
//
//  W V U                        000----001----010----011----100----101----110----111
const int32_t ph_u_dec_tbl[8] = {1,     1,    -1,     0,     0,     1,    -1,     1 };
const int32_t ph_v_dec_tbl[8] = {1,     0,     1,     1,    -1,    -1,     0,     1 };
const int32_t ph_w_dec_tbl[8] = {1,    -1,     0,    -1,     1,     0,     1,     1 };

uint32_t        moving_pulse_ms = 100;

PIDParams       pid_i_loop;
PIDParams       pid_s_loop;
uint32_t        speed_loop_divider   = 0;
float           speed_loop_e;
uint32_t        current_loop_divider = 0;
float           current_loop_e;

T_movement_cbl    mc;
/*-----------------------------------------------------------------------------------------------------
  Подготавливаем модуль OPS (Output Phase Switching Control)
  микроконтроллера к использованию для управления коммутацией драйвера мотора

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Init_OPS_module(void)
{
  R_MSTP->MSTPCRD_b.MSTPD5  = 0; // GPT ch7-ch0 Module clock enable
  mot_cbl.opscr.EN     = 0; // Enable-Phase Output Control. 0: Do not output (Hi-Z on external pin), 1: Output.
  mot_cbl.opscr.FB     = 1; // External Feedback Signal Enable. 0: Select the external input. 1: Select the software settings (OPSCR.UF, VF, WF).
  mot_cbl.opscr.P      = 1; // Positive-Phase Output (P) Control.  0: Output level signal. 1: Output PWM signal (PWM of GPT32EH0)
  mot_cbl.opscr.N      = 1; // Negative-Phase Output (N) Control.  0: Output level signal. 1: Output PWM signal (PWM of GPT32EH0)
  mot_cbl.opscr.ALIGN  = 0; // Input Phase Alignment.              0: Align input phase to PCLKD. 1: Align input phase to PWM.
  mot_cbl.opscr.NFEN   = 1; // External Input Noise Filter Enable. 1: Use a noise filter on the external input.
  mot_cbl.opscr.NFCS   = 2; // External Input Noise Filter Clock Selection. 2: PCLKD/16
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Determine_opening_direction(void)
{
  if (wvar.close_position < wvar.open_position)
  {
    mot_cbl.opening_direction        = OPENING_CLOCKWISE;
    mot_cbl.speed_sign_on_open_dir   = OPENING_CLOCKWISE_SPEED_SIGN;
  }
  else
  {
    mot_cbl.opening_direction        = OPENING_COUNTERCLOCKWISE;
    mot_cbl.speed_sign_on_open_dir   = OPENING_COUNTERCLOCKWISE_SPEED_SIGN;
  }

}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Motor_init_PID_params(void)
{
  pid_i_loop.Kp          = wvar.i_k_prop;
  pid_i_loop.Ki          = wvar.i_k_integr;
  pid_i_loop.Kd          = wvar.i_k_diff;
  pid_i_loop.Kn          = wvar.i_k_filter;
  pid_i_loop.OutUpLim    = wvar.i_max_out;
  pid_i_loop.OutLoLim    = wvar.i_min_out;
  pid_i_loop.OutRateLim  = wvar.i_max_slew_rate;
  pid_i_loop.smpl_time   = 1.0f / (float)PWM_FREQ_HZ;

  pid_s_loop.Kp          = wvar.s_k_prop;
  pid_s_loop.Ki          = wvar.s_k_integr;
  pid_s_loop.Kd          = wvar.s_k_diff;
  pid_s_loop.Kn          = wvar.s_k_filter;
  pid_s_loop.OutUpLim    = wvar.s_max_out;
  pid_s_loop.OutLoLim    = wvar.s_min_out;
  pid_s_loop.OutRateLim  = wvar.s_max_slew_rate;
  pid_s_loop.smpl_time   = 1.0f / (float)SPEED_LOOP_FREQ_HZ;

  mot_cbl.en_current_loop = 1;
  mot_cbl.en_speed_loop   = 1;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_pid_pars
-----------------------------------------------------------------------------------------------------*/
static void MOT_PID_clear(PIDParams *p_pid_pars)
{
  p_pid_pars->PID_state.Del2_DSTATE    = 0;
  p_pid_pars->PID_state.Integr1_states = 0;
  p_pid_pars->PID_state.Integr2_states = 0;
}

/*-----------------------------------------------------------------------------------------------------
  Управление коммутацией силовых транзистров

  \param void
-----------------------------------------------------------------------------------------------------*/
void MC_do_6_step_commutation(void)
{

  // Здесь процедура управления коммутацией
  if (mot_cbl.state != 0)
  {
    if (mot_cbl.pwm_val == 0)
    {
      // Мотор не крутим если напряжение модуляции равно 0
      mot_cbl.opscr.EN = 0;
      R_GPT_OPS->OPSCR = mot_cbl.opscr.w;
      if (mot_cbl.prev_pwm_val != 0) MC_brake_on();
    }
    else
    {
      // На предельных уровнях мощности не используем модуляцию
      if (mot_cbl.pwm_val == 100)
      {
        mot_cbl.opscr.P = 0;
        mot_cbl.opscr.N = 0;
      }
      else
      {
        mot_cbl.opscr.P = 1;
        if (wvar.en_soft_commutation == 0)
        {
          mot_cbl.opscr.N = 1;
        }
        else
        {
          mot_cbl.opscr.N = 0; // При мягкой коммутации нижний транзистор всегда открыт
        }
      }

      if (mot_cbl.direction == OPENING_CLOCKWISE)
      {
        R_GPT_OPS->OPSCR = mot_cbl.opscr.w | forward_rotation_tbl[current_hall_state];  // Переключаем состояние коммутации для вращения в прямом направлении. В направлении открытия
      }
      else
      {
        R_GPT_OPS->OPSCR = mot_cbl.opscr.w | reverse_rotation_tbl[current_hall_state]; // Переключаем состояние коммутации для вращения в обратном направлении. В направлении закрытия
      }

      // Первое включение сигнала разрешения EN.
      // Первое его включение должно быть после того как будут установлены другие биты в регистре.
      if (mot_cbl.opscr.EN == 0)
      {
        mot_cbl.opscr.EN      = 1;
        R_GPT_OPS->OPSCR_b.EN = 1;
      }

      if (mot_cbl.prev_pwm_val == 0) MC_brake_off();
    }
  }
  else
  {
    // Здесь если мотор крутить не надо
    mot_cbl.opscr.EN = 0;
    R_GPT_OPS->OPSCR = mot_cbl.opscr.w;
  }

  mot_cbl.prev_pwm_val = mot_cbl.pwm_val;
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void MC_create_sync_obj(void)
{
  tx_event_flags_create(&mc_flags, "motor_ctrl_flags");
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return T_mot_control_cbl*
-----------------------------------------------------------------------------------------------------*/
T_mot_control_cbl* MC_get_cbl(void)
{
  return &mot_cbl;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MC_set_driver_fault(void)
{
  return  tx_event_flags_set(&mc_flags, MC_DRIVER_FAULT, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param flag

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MC_set_start_motor(void)
{
  return  tx_event_flags_set(&mc_flags, MC_START_MOTOR, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MC_set_stop_motor(void)
{
  return  tx_event_flags_set(&mc_flags, MC_STOP_MOTOR, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MC_set_moving_pulse(void)
{
  return  tx_event_flags_set(&mc_flags, MC_MOVING_PULSE, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MC_set_open(void)
{
  return  tx_event_flags_set(&mc_flags, MC_OPEN, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MC_set_close(void)
{
  return  tx_event_flags_set(&mc_flags, MC_CLOSE, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MC_set_closing_pulse(void)
{
  return  tx_event_flags_set(&mc_flags, MC_EXEC_CLOSING_PULSE, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MC_set_opening_pulse(void)
{
  return  tx_event_flags_set(&mc_flags, MC_EXEC_OPENING_PULSE, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MC_set_close_position(void)
{
  return  tx_event_flags_set(&mc_flags, MC_FIX_CLOSE_POSITION, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MC_set_open_position(void)
{
  return  tx_event_flags_set(&mc_flags, MC_FIX_OPEN_POSITION, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MC_set_freewheeling(void)
{
  return  tx_event_flags_set(&mc_flags, MC_FREEWHEELING, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MC_set_reset_tmc6200(void)
{
  return  tx_event_flags_set(&mc_flags, MC_DRIVER_FAULT, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MC_set_change_pwm(void)
{
  return  tx_event_flags_set(&mc_flags, MC_CHANGE_PWM, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t MC_set_speed_loop_tick(void)
{
  return  tx_event_flags_set(&mc_flags, MC_SPEED_LOOP_TICK, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void MC_freewheeling(void)
{
  mot_cbl.state   = 0;
  current_loop_e  = 0;
  speed_loop_e    = 0;

  __disable_interrupt();
  R_PFS->P301PFS_b.PODR = 0;         // UL
  R_PFS->P302PFS_b.PODR = 0;         // UH
  R_PFS->P208PFS_b.PODR = 0;         // VL
  R_PFS->P209PFS_b.PODR = 0;         // VH
  R_PFS->P408PFS_b.PODR = 0;         // WL
  R_PFS->P409PFS_b.PODR = 0;         // WH
  R_PFS->P301PFS_b.PMR  = PORT__IO;
  R_PFS->P302PFS_b.PMR  = PORT__IO;
  R_PFS->P208PFS_b.PMR  = PORT__IO;
  R_PFS->P209PFS_b.PMR  = PORT__IO;
  R_PFS->P408PFS_b.PMR  = PORT__IO;
  R_PFS->P409PFS_b.PMR  = PORT__IO;
  R_PFS->P301PFS_b.PSEL = PSEL_00;   // UL
  R_PFS->P302PFS_b.PSEL = PSEL_00;   // UH
  R_PFS->P208PFS_b.PSEL = PSEL_00;   // VL
  R_PFS->P209PFS_b.PSEL = PSEL_00;   // VH
  R_PFS->P408PFS_b.PSEL = PSEL_00;   // WL
  R_PFS->P409PFS_b.PSEL = PSEL_00;   // WH
  __enable_interrupt();
}

/*-----------------------------------------------------------------------------------------------------
   Переключение функций пинов на модуль GPIO
   Торможение включением всех нижних транзисторов и замыканием обмоток мотора между собой

  \param void
-----------------------------------------------------------------------------------------------------*/
void MC_brake_on(void)
{
  __disable_interrupt();
  R_PFS->P301PFS_b.PODR = 1;         // UL
  R_PFS->P302PFS_b.PODR = 0;         // UH
  R_PFS->P208PFS_b.PODR = 1;         // VL
  R_PFS->P209PFS_b.PODR = 0;         // VH
  R_PFS->P408PFS_b.PODR = 1;         // WL
  R_PFS->P409PFS_b.PODR = 0;         // WH
  R_PFS->P301PFS_b.PMR  = PORT__IO;
  R_PFS->P302PFS_b.PMR  = PORT__IO;
  R_PFS->P208PFS_b.PMR  = PORT__IO;
  R_PFS->P209PFS_b.PMR  = PORT__IO;
  R_PFS->P408PFS_b.PMR  = PORT__IO;
  R_PFS->P409PFS_b.PMR  = PORT__IO;
  R_PFS->P301PFS_b.PSEL = PSEL_00;   // UL
  R_PFS->P302PFS_b.PSEL = PSEL_00;   // UH
  R_PFS->P208PFS_b.PSEL = PSEL_00;   // VL
  R_PFS->P209PFS_b.PSEL = PSEL_00;   // VH
  R_PFS->P408PFS_b.PSEL = PSEL_00;   // WL
  R_PFS->P409PFS_b.PSEL = PSEL_00;   // WH
  __enable_interrupt();
}


/*-----------------------------------------------------------------------------------------------------
  Переключение функций пинов на модуль OPS (Output Phase Switching Control)

  \param void
-----------------------------------------------------------------------------------------------------*/
void MC_brake_off(void)
{
  __disable_interrupt();
  R_PFS->P301PFS_b.PSEL = PSEL_02;   // UL
  R_PFS->P302PFS_b.PSEL = PSEL_02;   // UH
  R_PFS->P208PFS_b.PSEL = PSEL_02;   // VL
  R_PFS->P209PFS_b.PSEL = PSEL_02;   // VH
  R_PFS->P408PFS_b.PSEL = PSEL_02;   // WL
  R_PFS->P409PFS_b.PSEL = PSEL_02;   // WH
  R_PFS->P301PFS_b.PMR  = PORT_PER;
  R_PFS->P302PFS_b.PMR  = PORT_PER;
  R_PFS->P208PFS_b.PMR  = PORT_PER;
  R_PFS->P209PFS_b.PMR  = PORT_PER;
  R_PFS->P408PFS_b.PMR  = PORT_PER;
  R_PFS->P409PFS_b.PMR  = PORT_PER;
  __enable_interrupt();
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t MC_do_start_motor(void)
{
  APPLOG("Motor started");

  speed_loop_divider            = 0;
  current_loop_divider          = 0;
  MOT_PID_clear(&pid_i_loop);
  MOT_PID_clear(&pid_s_loop);
  Currents_offset_calibration();
  MC_brake_off();
  mot_cbl.state      = 1;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t MC_do_stop_motor(void)
{
  mot_cbl.state  = 0;
  MC_brake_on();
  current_loop_e                = 0.0f;
  speed_loop_e                  = 0.0f;
  mot_cbl.pwm_val               = 0.0f;
  mot_cbl.target_rotation_speed = 0.0f;

  APPLOG("Motor stopped");

  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Выполнение функции петли управления током мотора
  Функция вызывается из прерывания АЦП с частотой PWM_FREQ_HZ

  \return float
-----------------------------------------------------------------------------------------------------*/
void MC_current_loop_PID_step(void)
{
  float pid_output;


  if (mot_cbl.state == 0) return;
  if (mot_cbl.en_current_loop == 0)
  {
    mot_cbl.direction = mot_cbl.mot_rotation_target_dir;
    return;
  }

  if (current_loop_divider == 0)
  {
    #ifdef USE_FILTERED_CURRENT_IN_CURRENT_LOOP
    current_loop_e = mot_cbl.target_current - adc.filtered_motor_current;
    #else
    current_loop_e = mot_cbl.target_current -(adc.instant_motor_current * adc.current_shunt_scale);
    #endif

    PIDController_custom(current_loop_e, pid_i_loop.smpl_time,&pid_i_loop,&pid_output);

    if (pid_output < 0)
    {
      mot_cbl.direction = mot_cbl.mot_rotation_target_dir ^ 1;
    }
    else
    {
      mot_cbl.direction = mot_cbl.mot_rotation_target_dir;
    }

    // Это условие обязательно. Без него начинаются сильные осцилляции направления вращения
    if (pid_output <= 0.0f)
    {
      pid_output= 0.0f;
    }

    mot_cbl.pwm_val = lrintf(pid_output);

    GPT0_update_PWM_value();

  }
  // Снижаем частоту выполнение ругулятора до SPEED_LOOP_FREQ_HZ
  current_loop_divider++;
  if (current_loop_divider >= (PWM_FREQ_HZ / CURRENT_LOOP_FREQ_HZ)) current_loop_divider = 0;

}

/*-----------------------------------------------------------------------------------------------------
  Выполнение функции петли управления скростью вращения
  Функция вызывается из прерывания АЦП с частотой PWM_FREQ_HZ

  \param void
-----------------------------------------------------------------------------------------------------*/
void MC_speed_loop_PID_step(void)
{
  float           pid_output;
  float           current_speed;

  if (mot_cbl.state == 0)        return;
  if (mot_cbl.en_current_loop == 0) return;
  if (mot_cbl.en_speed_loop == 0)   return;

  if (speed_loop_divider == 0)
  {
    // Направление вращения задается переменной mot_cbl.target_direction
    // Приводим скорость к положительной величине
    if (mot_cbl.mot_rotation_target_dir == mot_cbl.opening_direction)
    {
      current_speed = adc.shaft_speed * mot_cbl.speed_sign_on_open_dir;
    }
    else
    {
      current_speed = adc.shaft_speed * mot_cbl.speed_sign_on_open_dir * -1.0f;
    }
    speed_loop_e =(mot_cbl.target_rotation_speed - current_speed);
    PIDController_custom(speed_loop_e, pid_s_loop.smpl_time,&pid_s_loop,&pid_output);

    mot_cbl.target_current = pid_output;

    MC_set_speed_loop_tick();
  }
  // Снижаем частоту выполнение ругулятора до SPEED_LOOP_FREQ_HZ
  speed_loop_divider++;
  if (speed_loop_divider >= (PWM_FREQ_HZ / SPEED_LOOP_FREQ_HZ)) speed_loop_divider = 0;
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void Do_const_pwm_pulse_movement(void)
{
  uint32_t res;
  ULONG    actual_events;

  uint32_t app_err = App_get_error_flags();
  if (app_err != 0)
  {
    APPLOG("Execution aborted due to an driver error %08X", app_err);
    return;
  }

  T_sys_timestump  tst;
  Get_hw_timestump(&tst);
  if (mot_cbl.en_current_loop != 0) mot_cbl.pwm_val = 0;
  if (mot_cbl.en_speed_loop != 0)   mot_cbl.target_current = 0;
  GPT0_update_PWM_value();
  MC_do_start_motor();
  do
  {
    res = tx_event_flags_get(&mc_flags, MC_DRIVER_FAULT | MC_STOP_MOTOR, TX_OR_CLEAR,&actual_events, MS_TO_TICKS(2));
    if (res == TX_SUCCESS) break;
    Real_values_calculation();
  }while (Time_elapsed_msec(&tst) < moving_pulse_ms);

  MC_do_stop_motor();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Init_speed_trajectory_calculation(uint8_t mov_dir)
{

  mc.start_position = adc.smpl_POS_SENS;

  if (mov_dir == NEED_TO_CLOSE)
  {
    // Определяем желательную позицию остановки движения
    mc.finish_position = wvar.close_position;

    // Определяем позицию немедленной остановки c учетом запаса по углу на закрытие
    if (mot_cbl.opening_direction == OPENING_CLOCKWISE)
    {
      mc.stop_position = mc.finish_position - lroundf(CLOSING_MARGINE * adc.grad_to_samples_scale);
    }
    else
    {
      mc.stop_position = mc.finish_position + lroundf(CLOSING_MARGINE * adc.grad_to_samples_scale);
    }

  }
  else if (mov_dir == NEED_TO_OPEN)
  {
    // Определяем желательную позицию остановки движения
    mc.finish_position = wvar.open_position;

    // Определяем позицию немедленной остановки c учетом запаса по углу на открытие
    if (mot_cbl.opening_direction == OPENING_CLOCKWISE)
    {
      mc.stop_position = mc.finish_position + lroundf(OPENING_MARGINE * adc.grad_to_samples_scale);
    }
    else
    {
      mc.stop_position = mc.finish_position - lroundf(OPENING_MARGINE * adc.grad_to_samples_scale);
    }
  }

  mc.moving_distance = abs(mc.start_position - mc.finish_position);

  if (mc.start_position > mc.finish_position)
  {
    mc.current_speed = -adc.shaft_speed;
    mc.reverse_movement  = 1;
  }
  else
  {
    mc.current_speed = adc.shaft_speed;
    mc.reverse_movement  = 0;
  }

  // Если движение ещё идет в обратном направлении, то не учитывать его, поскольку не можем создать вращающий момент в противоположном направлении
  if (mc.current_speed < 0.0f) mc.current_speed = 0.0f;

  // .....................................................................................................................................
  // Планирование траектории
  // .....................................................................................................................................
  // Ускорение и замедление проводим по S-кривой
  // Свойство S-кривой таково что максимальное ускорение на ней равно удвоенному линейному ускорению между начальной и конечной скоростью
  // Формула вогнутой (concave) части S-кривой записывается так:  V0 + (As/T)*t^2
  // Формулы выпуклой (convex)  части S-кривой записывается так:  Vh + As*t - (As/T)*t^2
  // Где: T  - время нарастания от начальной скорости к конечной
  //      V0 - начальная скорость, V1 - конеченая скорость
  //      Vh - средняя скорсоть = (V1-V0)/2
  //      As - максимальное ускорение на S-кривой. Оно равно 2*(V1-V0)/T,
  //      t  - текущее время


  // Расчет времени ускорения в секундах с учетом текущей скорости
  mc.accel_time =((wvar.rotation_speed - mc.current_speed) / wvar.rotation_speed) * wvar.acceleration_time_ms / 1000.0f;
  // Расчет дистанции ускорения в отсчетах АЦП
  mc.accel_dist =  mc.accel_time * ((wvar.rotation_speed - mc.current_speed) / 2.0f + mc.current_speed) * adc.grad_to_samples_scale;

  // Расчет времени замедления в секундах с учетом текущей скорости
  mc.decel_time = wvar.deceleration_time_ms / 1000.0f;
  // Расчет дистанции замедления в отсчетах АЦП
  mc.decel_dist =  mc.decel_time * ((wvar.rotation_speed - wvar.min_rotation_speed) / 2.0f + wvar.min_rotation_speed) * adc.grad_to_samples_scale;

  mc.lin_time   = 0; // Продолжительность линейного движения

  float acel_decel_dist = mc.accel_dist + mc.decel_dist;

  float rot_accel_speed_delta = wvar.rotation_speed - mc.current_speed;
  float rot_decel_speed_delta = wvar.rotation_speed - wvar.min_rotation_speed;
  float scale = 1.0f;

  if (acel_decel_dist >= mc.moving_distance)
  {
    // На требуемой дистанции не удастся развить максимальную скорость
    // Уменьшаем время разгона и торможения пропорционально сокращению дистанции
    scale                  = mc.moving_distance / acel_decel_dist;
    mc.accel_time          = mc.accel_time * scale;
    mc.decel_time          = mc.decel_time * scale;
    rot_accel_speed_delta  = rot_accel_speed_delta * scale;
    rot_decel_speed_delta  = rot_decel_speed_delta * scale;
  }
  else
  {
    // Расчет времени линейного движеня в секундах
    mc.lin_time =((mc.moving_distance - acel_decel_dist) * adc.samples_to_grad_scale) / wvar.rotation_speed;
  }

  // Находим коэфициенты необходимые для расчета s-кривой на этапе ускорения
  mc.accel_v0    = mc.current_speed;
  mc.accel_vh    = rot_accel_speed_delta / 2.0f;
  mc.accel_As    = 2 * rot_accel_speed_delta / mc.accel_time;
  mc.accel_C2    = mc.accel_As / mc.accel_time;
  mc.accel_halfT = mc.accel_time / 2.0f;

  // Находим коэфициенты необходимые для расчета s-кривой на этапе замедления
  mc.decel_v0    = wvar.min_rotation_speed * scale;
  mc.decel_vh    = rot_decel_speed_delta / 2.0f + wvar.min_rotation_speed * scale;
  mc.decel_As    = 2 * rot_decel_speed_delta / mc.decel_time;
  mc.decel_C2    = mc.decel_As / mc.decel_time;
  mc.decel_halfT = mc.decel_time / 2.0f;
  float       Jm = 2.0f * mc.decel_As / mc.decel_time;
  mc.decel_concave_dist =(((wvar.min_rotation_speed + powf(mc.decel_As, 2) / (6.0f * Jm)) * mc.decel_As) / Jm) * adc.grad_to_samples_scale; // Находим путь пройденный в течении вогнутой части S-кривой на стадии замедления

  mc.traj_stage = TRAJ_ACCELR_CONCAVE;
  mc.stage_time = 0;
  mc.estim_pos_0= mc.start_position;
}

/*-----------------------------------------------------------------------------------------------------
  Получение значения целевой скорости в градусах в сек в текущий момент

  \param start_pos
  \param finish_pos

  \return float
-----------------------------------------------------------------------------------------------------*/
static float Speed_trajectory_calculation(void)
{
  float t;
  float rotation_speed = 0.0f;

  // Движение безотносительно направления движения и стороны подвеса  начинается с нуля и заканчивается в точке  mc.distance
  // Здесь нормализуем в соотвествии с этим условием значения позиции и скорости
  if (mc.reverse_movement)
  {
    mc.current_pos = mc.start_position - adc.smpl_POS_SENS;
    mc.current_speed = -adc.shaft_speed;
  }
  else
  {
    mc.current_pos = adc.smpl_POS_SENS - mc.start_position;
    mc.current_speed = adc.shaft_speed;
  }

  switch (mc.traj_stage)
  {
  case TRAJ_ACCELR_CONCAVE:
    t = mc.stage_time;
    rotation_speed = mc.accel_v0 + mc.accel_C2 * powf(t,2);
    if (mc.reverse_movement)
    {
      mc.estim_pos   = -(mc.accel_v0 * t + mc.accel_C2 * powf(t,3) / 3.0f) * adc.grad_to_samples_scale + mc.estim_pos_0;
    }
    else
    {
      mc.estim_pos   =(mc.accel_v0 * t + mc.accel_C2 * powf(t,3) / 3.0f) * adc.grad_to_samples_scale + mc.estim_pos_0;
    }
    mc.stage_time += 1.0f / (float)(SPEED_LOOP_FREQ_HZ);
    if (mc.stage_time >= mc.accel_halfT)
    {
      mc.estim_pos_0 = adc.smpl_POS_SENS;
      mc.traj_stage++;
      mc.stage_time = 0.0f;
    }
    break;
  case TRAJ_ACCELR_CONVEX :
    t = mc.stage_time;
    rotation_speed = mc.accel_vh +  mc.accel_As * t -  mc.accel_C2 * powf(t,2);
    if (mc.reverse_movement)
    {
      mc.estim_pos   = -(mc.accel_vh * t +  mc.accel_As * powf(t,2) / 2.0f -  mc.accel_C2 * powf(t,3) / 3.0f) * adc.grad_to_samples_scale + mc.estim_pos_0;
    }
    else
    {
      mc.estim_pos   =(mc.accel_vh * t +  mc.accel_As * powf(t,2) / 2.0f -  mc.accel_C2 * powf(t,3) / 3.0f) * adc.grad_to_samples_scale + mc.estim_pos_0;
    }
    mc.stage_time += 1.0f / (float)(SPEED_LOOP_FREQ_HZ);
    if (mc.stage_time >= mc.accel_halfT)
    {
      mc.estim_pos_0 = adc.smpl_POS_SENS;
      mc.traj_stage++;
      mc.stage_time = 0.0f;
      if (mc.lin_time == 0) mc.traj_stage++; // Пропускаем этап линейной скорости если его продолжительность нулевая
    }
    break;
  case TRAJ_LINEAR        :
    t = mc.stage_time;
    rotation_speed = wvar.rotation_speed;
    mc.stage_time += 1.0f / (float)(SPEED_LOOP_FREQ_HZ);

    if (mc.reverse_movement)
    {
      mc.estim_pos   = -(wvar.rotation_speed * t) * adc.grad_to_samples_scale + mc.estim_pos_0;
    }
    else
    {
      mc.estim_pos   =(wvar.rotation_speed * t) * adc.grad_to_samples_scale + mc.estim_pos_0;
    }

    // Переходим к замедлению если пересекли точку замедления
    if (mc.reverse_movement)
    {
      if (adc.smpl_POS_SENS <= (mc.finish_position + mc.decel_dist))
      {
        mc.estim_pos_0 = adc.smpl_POS_SENS;
        mc.traj_stage++;
        mc.stage_time = 0.0f;
      }
    }
    else
    {
      if (adc.smpl_POS_SENS >= (mc.finish_position - mc.decel_dist))
      {
        mc.estim_pos_0 = adc.smpl_POS_SENS;
        mc.traj_stage++;
        mc.stage_time = 0.0f;
      }
    }

    break;
  case TRAJ_DECELR_CONVEX :
    t = mc.decel_halfT - mc.stage_time;
    rotation_speed = mc.decel_vh +  mc.decel_As * t -  mc.decel_C2 * (powf(t,2));
    if (mc.reverse_movement)
    {
      mc.estim_pos   = -((mc.decel_dist-mc.decel_concave_dist)-(mc.decel_vh * t +  mc.decel_As * powf(t,2) / 2.0f -  mc.decel_C2 * powf(t,3) / 3.0f) * adc.grad_to_samples_scale)+ mc.estim_pos_0;
    }
    else
    {
      mc.estim_pos   =(mc.decel_dist-mc.decel_concave_dist)-(mc.decel_vh * t +  mc.decel_As * powf(t,2) / 2.0f -  mc.decel_C2 * powf(t,3) / 3.0f) * adc.grad_to_samples_scale + mc.estim_pos_0;
    }
    mc.stage_time += 1.0f / (float)(SPEED_LOOP_FREQ_HZ);
    if (mc.stage_time >= mc.decel_halfT)
    {
      mc.estim_pos_0 = adc.smpl_POS_SENS;
      mc.traj_stage++;
      mc.stage_time = 0.0f;
    }
    break;
  case TRAJ_DECELR_CONCAVE :
    t = mc.decel_halfT - mc.stage_time;
    rotation_speed = mc.decel_v0 + mc.decel_C2 * (powf(t,2));
    if (mc.reverse_movement)
    {
      mc.estim_pos   = -(mc.decel_concave_dist -(mc.decel_v0 * t + mc.decel_C2 * powf(t,3) / 3.0f) * adc.grad_to_samples_scale)+ mc.estim_pos_0;
    }
    else
    {
      mc.estim_pos   = mc.decel_concave_dist -(mc.decel_v0 * t + mc.decel_C2 * powf(t,3) / 3.0f) * adc.grad_to_samples_scale + mc.estim_pos_0;
    }
    mc.stage_time += 1.0f / (float)(SPEED_LOOP_FREQ_HZ);
    if (mc.stage_time >= mc.decel_halfT)
    {
      mc.estim_pos_0 = adc.smpl_POS_SENS;
      mc.traj_stage++;
      mc.stage_time = 0.0f;
    }
    break;
  case TRAJ_FINISH:
    rotation_speed = wvar.min_rotation_speed;
    break;
  }

  return rotation_speed;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Do_const_current_pulse_movement(uint8_t mov_dir)
{
  uint32_t res;
  ULONG    actual_events;
  T_sys_timestump  tst;

  uint32_t app_err = App_get_error_flags();
  if (app_err != 0)
  {
    APPLOG("Execution aborted due to an driver error %08X", app_err);
    return;
  }

  Get_hw_timestump(&tst);
  mot_cbl.en_speed_loop = 0;
  mot_cbl.en_current_loop = 1;

  mot_cbl.mot_rotation_target_dir = mov_dir ^ mot_cbl.opening_direction;

  mot_cbl.target_current = CURRENT_PULSE_VALUE;

  MC_do_start_motor();
  Set_output_on(OUTP_LED_GREEN);

  do
  {
    res = tx_event_flags_get(&mc_flags, MC_DRIVER_FAULT | MC_STOP_MOTOR | MC_FREEWHEELING, TX_OR_CLEAR,&actual_events, MS_TO_TICKS(2));
    if (res == TX_SUCCESS)
    {
      if (actual_events & MC_DRIVER_FAULT)
      {
        MC_do_stop_motor();
        MC_set_driver_fault();
        break;
      }
      if (actual_events & MC_FREEWHEELING)
      {
        MC_freewheeling();
        break;
      }
    }

    Real_values_calculation();

    if (Time_elapsed_msec(&tst) > CURRENT_PULSE_LEN_MS)
    {
      MC_do_stop_motor();
      break;
    }

  }while (1);

  Set_output_off(OUTP_LED_GREEN);
  mot_cbl.en_speed_loop = 1;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Waiting_movement_end(uint32_t timeout_ms)
{
  ULONG            actual_events;
  uint32_t         res;
  T_sys_timestump  tst;

  Get_hw_timestump(&tst);

  while (adc.shaft_abs_speed > MAX_ALLOWABLE_SHAFT_SPEED_BEFOR_MOVEMENT)
  {
    res = tx_event_flags_get(&mc_flags, MC_DRIVER_FAULT | MC_STOP_MOTOR | MC_FREEWHEELING | MC_OPEN | MC_CLOSE | MC_SPEED_LOOP_TICK, TX_OR,&actual_events, MS_TO_TICKS(10));
    if (res == TX_SUCCESS) return RES_ERROR;
    if (Time_elapsed_msec(&tst) > timeout_ms) return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Movement_possibility_checking(uint8_t mov_dir)
{
  uint32_t app_err = 0;

  if ((mot_cbl.en_speed_loop == 0) || (mot_cbl.en_current_loop == 0)) return RES_ERROR;

  app_err = App_get_error_flags();
  if (app_err != 0)
  {
    mc.order = NO_ORDER;
    APPLOG("Execution aborted due to an system error %08X", app_err);
    return RES_ERROR;
  }


  if (mov_dir == NEED_TO_OPEN)
  {
    if (mot_cbl.opening_direction == OPENING_CLOCKWISE)
    {
      if (adc.smpl_POS_SENS >= wvar.open_position)
      {
        APPLOG("Execution aborted due to open position reached (%d >= %d)", adc.smpl_POS_SENS, wvar.open_position);
        mc.order = NO_ORDER;
        return RES_ERROR;
      }
    }
    else
    {
      if (adc.smpl_POS_SENS <= wvar.open_position)
      {
        APPLOG("Execution aborted due to open position reached (%d <= %d)", adc.smpl_POS_SENS, wvar.open_position);
        mc.order = NO_ORDER;
        return RES_ERROR;
      }
    }
    mc.order = ORDER_TO_OPEN;
  }
  else if (mov_dir == NEED_TO_CLOSE)
  {
    if (mot_cbl.opening_direction == OPENING_CLOCKWISE)
    {
      if (adc.smpl_POS_SENS <= wvar.close_position)
      {
        APPLOG("Execution aborted due to close position reached (%d <= %d)", adc.smpl_POS_SENS, wvar.close_position);
        mc.order = NO_ORDER;
        return RES_ERROR;
      }
    }
    else
    {
      if (adc.smpl_POS_SENS >= wvar.close_position)
      {
        APPLOG("Execution aborted due to close  position reached (%d >= %d)", adc.smpl_POS_SENS, wvar.close_position);
        mc.order = NO_ORDER;
        return RES_ERROR;
      }
    }
    mc.order = ORDER_TO_CLOSE;
  }

  if (adc.shaft_abs_speed > MAX_ALLOWABLE_SHAFT_SPEED_BEFOR_MOVEMENT)
  {
    APPLOG("Execution aborted due to door movement (%0.1f degr/s)", (double)adc.shaft_abs_speed);
    if (Waiting_movement_end(1000) != RES_OK) return RES_ERROR;
    return RES_ERROR;
  }

  return RES_OK;
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
static void Do_movement(uint8_t mov_dir)
{
  uint32_t         res;
  ULONG            actual_events;

  if (Movement_possibility_checking(mov_dir) != RES_OK) return;

  Init_speed_trajectory_calculation(mov_dir);

  mot_cbl.mot_rotation_target_dir = mov_dir ^ mot_cbl.opening_direction;

  Get_hw_timestump(&mc.start_time);

  // Стартуем движение
  mot_cbl.target_current = 0;
  mot_cbl.pwm_val        = 0;
  mot_cbl.prev_pwm_val   = 0;
  MC_do_start_motor();

  Set_output_on(OUTP_LED_GREEN);

  mc.order = NO_ORDER;

  do
  {
    // Ожидаем флага цикла расчета траектории скорости. Флаг поступает с частотой цикла управления скоростью
    res = tx_event_flags_get(&mc_flags, MC_DRIVER_FAULT | MC_STOP_MOTOR | MC_FREEWHEELING | MC_OPEN | MC_CLOSE | MC_SPEED_LOOP_TICK, TX_OR_CLEAR,&actual_events, MS_TO_TICKS(10));

    Real_values_calculation();

    if (res == TX_SUCCESS)
    {
      if (actual_events & MC_SPEED_LOOP_TICK)
      {
        // Поступил сигнал цикла выполнения задачи
        // На каждом тике обновляем значение целевой сорости
        mot_cbl.target_rotation_speed = Speed_trajectory_calculation();
      }

      if ((actual_events & MC_CLOSE) && (mov_dir == NEED_TO_OPEN))
      {
        // Прекращаем процесс поскольку поступила команда на закрытие во время открытия
        mc.order = ORDER_TO_CLOSE;
        MC_do_stop_motor();
        goto _exit;
      }

      if ((actual_events & MC_OPEN) && (mov_dir == NEED_TO_CLOSE))
      {
        // Прекращаем процесс поскольку поступила команда на открытие во время закрытия
        mc.order = ORDER_TO_OPEN;
        MC_do_stop_motor();
        goto _exit;
      }

      if (actual_events & MC_DRIVER_FAULT)
      {
        MC_set_driver_fault();
        MC_do_stop_motor();
        // Прекращаем процесс поскольку обнаружена ошибка микросхемы драйвера.
        // Флаг ошибки был установлен в обработчике прерывания от драйвера
        goto _exit;
      }

      if (actual_events & MC_STOP_MOTOR)
      {
        MC_do_stop_motor();
        // Прекращаем процесс поскольку поступила команда остановки
        goto _exit;
      }

      if (actual_events & MC_FREEWHEELING)
      {
        MC_freewheeling();
        // Прекращаем процесс поскольку поступила команда на свободное вращение
        goto _exit;
      }
    }

    // В процессе движения останавливаемся как только пересекли точку финиша
    if (mc.reverse_movement == 0)
    {
      if (adc.smpl_POS_SENS >= mc.stop_position)
      {
        // Прекращаем процесс поскольку механизм пересек точку финиша при прямом движении
        MC_do_stop_motor();
        goto _exit;
      }
    }
    else
    {
      if (adc.smpl_POS_SENS <= mc.stop_position)
      {
        // Прекращаем процесс поскольку механизм пересек точку финиша при реверсивном движении
        MC_do_stop_motor();
        goto _exit;
      }
    }

    if (Time_elapsed_sec(&mc.start_time) > wvar.movement_max_duration)
    {
      // Прекращаем процесс поскольку истекло время
      MC_do_stop_motor();
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
void Motor_control_thread(void)
{
  ULONG     actual_events;
  uint32_t  res;

  Motor_init_PID_params();
  Determine_opening_direction();

  mot_cbl.state            = 0;
  mot_cbl.pwm_val          = 0;
  mot_cbl.prev_pwm_val     = 0;
  mot_cbl.pwm_freq         = PWM_FREQ_HZ;
  mot_cbl.mot_rotation_target_dir = mot_cbl.opening_direction;
  mot_cbl.target_current   = 0.0f;
  GPT0_update_PWM_value();
  MC_brake_on();

  TMC6200_init();
  TMC6200_init_fault_interrut();
  Thread_TMC6200_create();

  Init_OPS_module();
  Currents_offset_calibration();
  Calculating_scaling_factors();


  do
  {
    res = tx_event_flags_get(&mc_flags, 0x7FFFFFFF, TX_OR_CLEAR,&actual_events, MS_TO_TICKS(10));
    if (res == TX_SUCCESS)
    {

      if ((actual_events & MC_CHANGE_PWM) && (mot_cbl.en_speed_loop == 0) && (mot_cbl.en_current_loop == 0))
      {
        GPT0_update_PWM_value();
      }

      if (actual_events & MC_STOP_MOTOR)
      {
        GPT0_update_PWM_value();
        MC_do_stop_motor();
      }

      if ((actual_events & MC_START_MOTOR) && (mot_cbl.en_speed_loop == 0))
      {
        APPLOG("Update PWM value");
        GPT0_update_PWM_value();
        MC_do_start_motor();
      }

      if (actual_events & MC_MOVING_PULSE)
      {
        Do_const_pwm_pulse_movement();
      }

      if (actual_events & MC_EXEC_CLOSING_PULSE)
      {
        APPLOG("Execution of pulse to close direction");
        Do_const_current_pulse_movement(NEED_TO_CLOSE);
      }

      if (actual_events & MC_EXEC_OPENING_PULSE)
      {
        APPLOG("Execution of pulse to open direction");
        Do_const_current_pulse_movement(NEED_TO_OPEN);
      }

      if (actual_events & MC_OPEN)
      {
        APPLOG("Execution movement to open direction");
        Do_movement(NEED_TO_OPEN);
      }

      if (actual_events & MC_CLOSE)
      {
        APPLOG("Execution movement to close direction");
        Do_movement(NEED_TO_CLOSE);
      }

      if (actual_events & MC_FREEWHEELING)
      {
        APPLOG("Execution of freewheeling");
        MC_freewheeling();
      }

      if (actual_events & MC_FIX_CLOSE_POSITION)
      {
        APPLOG("Calibration of close position");
        wvar.close_position = adc.smpl_POS_SENS;
        Determine_opening_direction();
        Request_save_app_settings();
      }

      if (actual_events & MC_FIX_OPEN_POSITION)
      {
        APPLOG("Calibration of open position");
        wvar.open_position = adc.smpl_POS_SENS;
        Determine_opening_direction();
        Request_save_app_settings();
      }

      if (actual_events & MC_DRIVER_FAULT)
      {
        App_set_error_flags(APP_ERR_TMC6200_FAULT);
        TMC6200_set_clearing_fault();
      }
    }
    else
    {
      if (mc.order == ORDER_TO_CLOSE)
      {
        APPLOG("Execution movement to close direction");
        Do_movement(NEED_TO_CLOSE);
      }
      else if (mc.order == ORDER_TO_OPEN)
      {
        APPLOG("Execution movement to open direction");
        Do_movement(NEED_TO_OPEN);
      }

      Real_values_calculation();
      Temperatures_calculation();

    }

  } while (1);


}


