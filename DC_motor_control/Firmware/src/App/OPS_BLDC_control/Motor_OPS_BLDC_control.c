// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-10-13
// 12:02:55
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

const uint8_t forward_rotation_tbl[8] = {0, 5, 3, 1, 6, 4, 2, 0};
const uint8_t reverse_rotation_tbl[8] = {0, 2, 4, 6, 1, 3, 5, 0};

// Таблицы направления токов в фазах драйвера мотора в зависимости от маски состояния сигналов с датчиков холла
// в соответсвтии с внутренней прошитой таблицей дектодирования в микроконтроллере
// Значение 1 соответствует току втекающему в обмотку, -1 - обозначает ток вытекающий из обмотки
//
//   W V U                        000----001----010----011----100----101----110----111
//const int32_t ph_u_dec_tbl[8] = {1,     1,    -1,     0,     0,     1,    -1,     1 };
//const int32_t ph_v_dec_tbl[8] = {1,     0,     1,     1,    -1,    -1,     0,     1 };
//const int32_t ph_w_dec_tbl[8] = {1,    -1,     0,    -1,     1,     0,     1,     1 };

T_OPSCR        ops_cr;


/*-----------------------------------------------------------------------------------------------------
  Подготавливаем модуль OPS (Output Phase Switching Control)
  микроконтроллера к использованию для управления коммутацией драйвера мотора

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t OPS_init_module(void)
{
  R_MSTP->MSTPCRD_b.MSTPD5  = 0; // GPT ch7-ch0 Module clock enable
  ops_cr.EN     = 0; // Enable-Phase Output Control. 0: Do not output (Hi-Z on external pin), 1: Output.
  ops_cr.FB     = 1; // External Feedback Signal Enable. 0: Select the external input. 1: Select the software settings (OPSCR.UF, VF, WF).
  ops_cr.P      = 1; // Positive-Phase Output (P) Control.  0: Output level signal. 1: Output PWM signal (PWM of GPT32EH0)
  ops_cr.N      = 1; // Negative-Phase Output (N) Control.  0: Output level signal. 1: Output PWM signal (PWM of GPT32EH0)
  ops_cr.ALIGN  = 0; // Input Phase Alignment.              0: Align input phase to PCLKD. 1: Align input phase to PWM.
  ops_cr.NFEN   = 1; // External Input Noise Filter Enable. 1: Use a noise filter on the external input.
  ops_cr.NFCS   = 2; // External Input Noise Filter Clock Selection. 2: PCLKD/16
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
   Переключение функций пинов на модуль GPIO
   Торможение включением всех нижних транзисторов и замыканием обмоток мотора между собой

  \param void
-----------------------------------------------------------------------------------------------------*/
void OPS_hard_stop(void)
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
void OPS_ret_pins_to_periph_mode(void)
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
-----------------------------------------------------------------------------------------------------*/
void OPS_freewheeling(void)
{

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
  Управление коммутацией силовых транзистров

  \param void
-----------------------------------------------------------------------------------------------------*/
void OPS_BLDC_do_6_step_commutation(void)
{

  // Здесь процедура управления коммутацией
  if (drv_cbl.state != 0)
  {
    if (drv_cbl.pwm_val == 0)
    {
      // Мотор не крутим если напряжение модуляции равно 0
      ops_cr.EN = 0;
      R_GPT_OPS->OPSCR = ops_cr.w;
      if (drv_cbl.prev_pwm_val != 0) OPS_hard_stop();
    }
    else
    {
      // На предельных уровнях мощности не используем модуляцию
      if (drv_cbl.pwm_val == 100)
      {
        ops_cr.P = 0;
        ops_cr.N = 0;
      }
      else
      {
        ops_cr.P = 1;
        if (wvar.en_soft_commutation == 0)
        {
          ops_cr.N = 1;
        }
        else
        {
          ops_cr.N = 0; // При мягкой коммутации нижний транзистор всегда открыт
        }
      }

      if ((drv_cbl.direction ^ wvar.opposit_sensor_mnt) == OPENING_CLOCKWISE)
      {
        R_GPT_OPS->OPSCR = ops_cr.w | forward_rotation_tbl[h3.bitmask];  // Переключаем состояние коммутации для вращения в прямом направлении. В направлении открытия
      }
      else
      {
        R_GPT_OPS->OPSCR = ops_cr.w | reverse_rotation_tbl[h3.bitmask]; // Переключаем состояние коммутации для вращения в обратном направлении. В направлении закрытия
      }

      // Первое включение сигнала разрешения EN.
      // Первое его включение должно быть после того как будут установлены другие биты в регистре.
      if (ops_cr.EN == 0)
      {
        ops_cr.EN      = 1;
        R_GPT_OPS->OPSCR_b.EN = 1;
      }

      if (drv_cbl.prev_pwm_val == 0) OPS_ret_pins_to_periph_mode();
    }
  }
  else
  {
    // Здесь если мотор крутить не надо
    ops_cr.EN = 0;
    R_GPT_OPS->OPSCR = ops_cr.w;
  }

  drv_cbl.prev_pwm_val = drv_cbl.pwm_val;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void OPS_BLDC_ISR_handler(void)
{
  Hall_3PH_capture_bitmask();
  Measure_instant_phases_current();          // 2.30 us
  Measure_overall_BLDC_motor_current();
  Measure_DC_bus_voltage_current();
  Measure_servo_sensor_speed();              // 1.46 us
  Hall_3PH_measure_speed_and_direction();    // 0.72 us

  if (drv_cbl.state != 0)
  {
    if (drv_cbl.en_ADRC_control != 0)
    {
      drv_cbl.pwm_val = ADRC_loop_step(adc.shaft_abs_speed_rt, drv_cbl.target_rotation_speed);
    }
    else
    {
      if (drv_cbl.en_PID_current_control != 0)
      {
        if (drv_cbl.en_PID_speed_control != 0)
        {
          if (drv_cbl.speed_loop_divider == 0)
          {
            Speed_PID_step();                    //
          }
        }
        drv_cbl.pwm_val = Current_PID_step();    // 2.64 us
      }
      else
      {
        drv_cbl.direction = drv_cbl.mot_rotation_target_dir;
      }
    }

    GPT0_update_comare_reg(drv_cbl.pwm_val);


    // Current control tick generator
    //..............................................................
    drv_cbl.current_loop_divider++;
    if (drv_cbl.current_loop_divider >= (wvar.pwm_frequency / CURRENT_LOOP_FREQ_HZ))
    {
      drv_cbl.current_loop_divider = 0;
    }
    //..............................................................



    // Speed control tick generator
    //..............................................................
    drv_cbl.speed_loop_divider++;
    if (drv_cbl.speed_loop_divider >= (wvar.pwm_frequency / SPEED_LOOP_FREQ_HZ))
    {
      drv_cbl.speed_loop_divider = 0;
      App_set_flags(MC_SPEED_LOOP_TICK);
    }
    //..............................................................


    OPS_BLDC_do_6_step_commutation();                 // 0.80 us
  }
}

