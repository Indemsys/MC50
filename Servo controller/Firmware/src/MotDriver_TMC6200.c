// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-07-13
// 12:07:19
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

T_TMC6200_cbl      tmc6200_cbl;
uint32_t           tmc6200_comm_err_cnt;
T_TMC6200_counters tmc6200_cnts;
uint32_t           tmc6200_u_gstat;


#define TMC6200_TASK_STACK_SIZE 1024
static uint8_t tmc6200_stacks[TMC6200_TASK_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.tmc6200_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static TX_THREAD       tmc6200_thread;


static IRQn_Type tmc6200_fault_int_num;
static void  TMC6200_fault_IRQ0_isr(void);
SSP_VECTOR_DEFINE(TMC6200_fault_IRQ0_isr, ICU, IRQ0);


#define   TMC6200_RESET_FAULT            BIT(0)
#define   TMC6200_RESET_FAULT_DONE       BIT(1)
TX_EVENT_FLAGS_GROUP   tmc6200_flags;

/*-----------------------------------------------------------------------------------------------------


  \param regaddr
  \param word

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t TMC6200_read(uint8_t regaddr, uint32_t *word)
{
  uint32_t res;
  uint8_t  buf[5];
  uint32_t w = 0;

  buf[0] = regaddr;
  buf[1] =(*word >> 24) & 0xFF;
  buf[2] =(*word >> 16) & 0xFF;
  buf[3] =(*word >>  8) & 0xFF;
  buf[4] =(*word >>  0) & 0xFF;

  res =  SPI0_send_recv_buf_to_TMS6200(buf,5);
  if (res == RES_OK)
  {
    w =(buf[1] << 24) | (buf[2] << 16) | (buf[3] << 8) | (buf[4] << 0);
  }
  *word = w;
  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param regaddr
  \param word

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t TMC6200_write(uint8_t regaddr, uint32_t word)
{
  uint32_t res;
  uint8_t  buf[5];

  buf[0] = regaddr | 0x80;
  buf[1] =(word >> 24) & 0xFF;
  buf[2] =(word >> 16) & 0xFF;
  buf[3] =(word >>  8) & 0xFF;
  buf[4] =(word >>  0) & 0xFF;

  res =  SPI0_send_recv_buf_to_TMS6200(buf,5);
  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static uint32_t  TMC6200_register_diagnostic_reading(uint8_t addr, uint32_t *p_reg, const char *name)
{
  if (TMC6200_read(addr,p_reg) == RES_OK)
  {
    APPLOG("TMC6200 %s =  %08X.",name,*p_reg);
    return RES_OK;
  }
  else
  {
    APPLOG("TMC6200 %s read error", name);
    return RES_ERROR;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void  TMC6200_fault_IRQ0_isr(void)
{
  SF_CONTEXT_SAVE;

  R_ICU->IELSRn_b[tmc6200_fault_int_num].IR = 0; // Сбрасываем IR флаг в ICU

  MC_set_driver_fault();

  SF_CONTEXT_RESTORE;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void TMC6200_init_fault_interrut(void)
{
  tmc6200_fault_int_num = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_ICU_IRQ0);
  R_ICU->IRQCRn_b[0].IRQMD = 1;                   // Прерывание по нарастанию на IRQ0
  R_ICU->IELSRn_b[tmc6200_fault_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  NVIC_SetPriority(tmc6200_fault_int_num, TMC6200_FAULT_IRQ0_INT_PRIO);
  NVIC_ClearPendingIRQ(tmc6200_fault_int_num);
  NVIC_EnableIRQ(tmc6200_fault_int_num);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void TMC6200_set_in_work_mode(void)
{

  TMC6200_read(TMC6200_GCONF_ADDR,&tmc6200_cbl.u_gconf);
  tmc6200_cbl.gconf.disable           = 1; // 1: Disable driver
  tmc6200_cbl.gconf.singleline        = 0; // 0: Individual signals L+H
  tmc6200_cbl.gconf.faultdirect       = 1;
  tmc6200_cbl.gconf.amplification     = TMC6200_AMPLIFICATION_20; //
  tmc6200_cbl.gconf.amplifier_off     = 0; // 0: Current sense amplifiers on
  tmc6200_cbl.gconf.test_mode         = 0; // 0: Normal operation
  TMC6200_write(TMC6200_GCONF_ADDR, tmc6200_cbl.u_gconf);

  Wait_ms(2); //

  // Выключаем полностью механизм обнаружения и защиты от коротких замыканий
  TMC6200_read(TMC6200_SHORT_CONF,&tmc6200_cbl.u_short_conf);
  tmc6200_cbl.short_conf.disable_S2G  = wvar.enable_short_to_gnd_prot;
  tmc6200_cbl.short_conf.disable_S2VS = wvar.enable_short_to_vs_prot;
  tmc6200_cbl.short_conf.S2VS_LEVEL   = wvar.short_vs_det_level;
  tmc6200_cbl.short_conf.S2G_LEVEL    = wvar.short_gnd_det_level;
  tmc6200_cbl.short_conf.SHORTFILTER  = wvar.short_det_spike_filter;
  tmc6200_cbl.short_conf.shortdelay   = wvar.short_det_delay_param;
  TMC6200_write(TMC6200_SHORT_CONF,tmc6200_cbl.u_short_conf);


  TMC6200_read(TMC6200_DRVCF_ADDR,&tmc6200_cbl.u_drv_conf);
  tmc6200_cbl.drv_conf.drvstrength    = wvar.gate_driver_current_param;
  TMC6200_write(TMC6200_DRVCF_ADDR,tmc6200_cbl.u_drv_conf);


  // Сбрасываем установленные флаги в регисте флагов, чтобы обнаруживать появление новых
  TMC6200_read(TMC6200_GSTAT_ADDR,&tmc6200_cbl.u_gstat);
  TMC6200_write(TMC6200_GSTAT_ADDR, tmc6200_cbl.u_gstat);

  // Разрешаем работы выходных драйверов
  tmc6200_cbl.gconf.disable        = 0; // 0: Enable driver
  TMC6200_write(TMC6200_GCONF_ADDR, tmc6200_cbl.u_gconf);

  // Активируем работу выходных драйверов аппаратным сигналом MDRV_EN
  MDRV_EN = 1;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void TMC6200_log_registers_state(void)
{
  TMC6200_register_diagnostic_reading(TMC6200_GCONF_ADDR ,&tmc6200_cbl.u_gconf      , "GCONF");
  TMC6200_register_diagnostic_reading(TMC6200_GSTAT_ADDR ,&tmc6200_cbl.u_gstat      , "GSTAT");
  TMC6200_register_diagnostic_reading(TMC6200_INPUT_ADDR ,&tmc6200_cbl.u_inputs     , "INPUT");
  TMC6200_register_diagnostic_reading(TMC6200_SHORT_CONF ,&tmc6200_cbl.u_short_conf , "SHORT");
  TMC6200_register_diagnostic_reading(TMC6200_DRVCF_ADDR ,&tmc6200_cbl.u_drv_conf   , "DRVCF");
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
void TMC6200_init(void)
{
  TMC6200_read(TMC6200_INPUT_ADDR,&tmc6200_cbl.u_inputs);

  if (tmc6200_cbl.u_inputs == 0)
  {
    // В битах 31...24 должен находиться ненулевой номер версии чипа. VERSION: 0x10=first version of the IC Identical numbers mean full digital compatibility.
    // Нулевое содержимое говорит об отсутсвии чипа или об отсутствии связи с ним
    App_set_error_flags(APP_ERR_NO_TMC6200);
    APPLOG("TMC6200: No communication");
    return; // Возврат если регистр прочитан неправильно
  }

  TMC6200_set_in_work_mode();
  TMC6200_log_registers_state();
  return;
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t TMC6200_set_clearing_fault(void)
{
  return  tx_event_flags_set(&tmc6200_flags, TMC6200_RESET_FAULT, TX_OR);
}


/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void TMC6200_faults_statistic(void)
{
  if (tmc6200_cbl.u_gstat != tmc6200_u_gstat)
  {
    T_TMC6200_GSTAT gs;
    uint32_t        gstat_rise =(~tmc6200_u_gstat) & tmc6200_cbl.u_gstat;

    memcpy(&gs,&gstat_rise, sizeof(gs));
    if (gs.reset)
    {
      tmc6200_cnts.reset++;
      APPLOG("TMC6200: Reset flag set detected");
    }
    if (gs.drv_otpw)
    {
      tmc6200_cnts.drv_otpw++;
      APPLOG("TMC6200: drv_otpw. Driver temperature has exceeded overtemperature prewarning-level");
    }
    if (gs.drv_ot)
    {
      tmc6200_cnts.drv_ot++;
      APPLOG("TMC6200: drv_ot. Overtemperature. Driver has been shut down");
    }
    if (gs.uv_cp)
    {
      tmc6200_cnts.uv_cp++;
      APPLOG("TMC6200: uv_cp. Undervoltage on the charge pump. Driver has been shut down");
    }
    if (gs.shortdet_u)
    {
      tmc6200_cnts.shortdet_u++;
      APPLOG("TMC6200: shortdet_u. U short counter has triggered at least once");
    }
    if (gs.s2gu)
    {
      tmc6200_cnts.s2gu++;
      APPLOG("TMC6200: s2gu. Short to GND detected on phase U");
    }
    if (gs.s2vsu)
    {
      tmc6200_cnts.s2vsu++;
      APPLOG("TMC6200: s2vsu. Short to VS detected on phase U");
    }
    if (gs.shortdet_v)
    {
      tmc6200_cnts.shortdet_v++;
      APPLOG("TMC6200: shortdet_v. V short counter has triggered at least once");
    }
    if (gs.s2gv)
    {
      tmc6200_cnts.s2gv++;
      APPLOG("TMC6200: s2gv. Short to GND detected on phase V");
    }
    if (gs.s2vsv)
    {
      tmc6200_cnts.s2vsv++;
      APPLOG("TMC6200: s2vsv. Short to VS detected on phase V");
    }
    if (gs.shortdet_w)
    {
      tmc6200_cnts.shortdet_w++;
      APPLOG("TMC6200: shortdet_w. W short counter has triggered at least once");
    }
    if (gs.s2gw)
    {
      tmc6200_cnts.s2gw++;
      APPLOG("TMC6200: s2gw. Short to GND detected on phase W");
    }
    if (gs.s2vsw)
    {
      tmc6200_cnts.s2vsw++;
      APPLOG("TMC6200: s2vsw. Short to VS detected on phase w");
    }

    tmc6200_u_gstat = tmc6200_cbl.u_gstat;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t TMC6200_check_fault(void)
{
  if (tmc6200_cbl.gstat.drv_ot)
  {
    // Драйвер отключился из-за перегрева
    return 1;
  }

  if (tmc6200_cbl.gstat.uv_cp)
  {
    // Драйвер отключился из-за недостатка напряжения
    return 1;
  }

  if (tmc6200_cbl.gstat.s2gu)
  {
    // Драйвер отключился из-за КЗ на землю в фазе U
    return 1;
  }

  if (tmc6200_cbl.gstat.s2vsu)
  {
    // Драйвер отключился из-за КЗ на шину пмитания в фазе U
    return 1;
  }

  if (tmc6200_cbl.gstat.s2gv)
  {
    // Драйвер отключился из-за КЗ на землю в фазе V
    return 1;
  }

  if (tmc6200_cbl.gstat.s2vsv)
  {
    // Драйвер отключился из-за КЗ на шину пмитания в фазе V
    return 1;
  }

  if (tmc6200_cbl.gstat.s2gw)
  {
    // Драйвер отключился из-за КЗ на землю в фазе W
    return 1;
  }

  if (tmc6200_cbl.gstat.s2vsw)
  {
    // Драйвер отключился из-за КЗ на шину пмитания в фазе W
    return 1;
  }

  return 0;
}

/*-----------------------------------------------------------------------------------------------------
  Сбрасываем флаг ошибки.
  Сброс производится обратной записью единицы в установленный в 1 флаг

  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
void TMC6200_reset_fault_state(void)
{
  TMC6200_read(TMC6200_GSTAT_ADDR,&tmc6200_cbl.u_gstat);      // Читаем чтобы получить текущее состояние флагов ошибки
  TMC6200_faults_statistic();

  if (TMC6200_check_fault())
  {
    MC_freewheeling(); // Чтобы штатно сбросить некоторые флаги ошибки драйвера нужно снять активный сигнал со всех транзистров
    Wait_ms(10);
    TMC6200_write(TMC6200_GSTAT_ADDR, tmc6200_cbl.u_gstat);   // Сбрасываем

    Wait_ms(5000);

    TMC6200_read(TMC6200_GSTAT_ADDR,&tmc6200_cbl.u_gstat);      // Читаем чтобы получить текущее состояние флагов ошибки

    if (!TMC6200_check_fault())
    {
      // Если состояние драйвера восстановлено, то снять флаг аварии драйвера
      App_clear_error_flags(APP_ERR_TMC6200_FAULT);
      MC_brake_on();
      APPLOG("TMC6200: Driver fault was reset successfully");
    }
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param arg
-----------------------------------------------------------------------------------------------------*/
static void TMC6200_task(ULONG arg)
{
  uint32_t   res;
  ULONG      actual_events;

  do
  {
    if (App_get_error_flags() & APP_ERR_NO_TMC6200)
    {
      Wait_ms(10);
      // Если нет связи с TMC6200, то постоянно проверяем связь и при появленнии связи производим реинициализацию
      TMC6200_read(TMC6200_INPUT_ADDR,&tmc6200_cbl.u_inputs);

      if (tmc6200_cbl.inputs.ver >= 0x10)
      {
        TMC6200_set_in_work_mode();
        TMC6200_log_registers_state();
        App_clear_error_flags(APP_ERR_NO_TMC6200);
      }
    }
    else
    {
      TMC6200_read(TMC6200_INPUT_ADDR,&tmc6200_cbl.u_inputs); // Читаем состояния входом микросхемы драйвера. Используем для проверки связи с драйвером
      if (tmc6200_cbl.inputs.ver == 0x00)
      {
        // Деактивируем работу выходных драйверов аппаратным сигналом MDRV_EN
        MDRV_EN = 0;
        App_set_error_flags(APP_ERR_NO_TMC6200);
        continue;
      }

      TMC6200_read(TMC6200_GSTAT_ADDR,&tmc6200_cbl.u_gstat);
      TMC6200_faults_statistic();

      if (TMC6200_check_fault())
      {
        // В случае ошибки мотора передаем флаг ошибки в задачу мотора.
        // Задача мотора остановливает дивижение, если оно было, и передает флаг необходимости сброса ошибки драйвера TMC6200_RESET_FAULT обратно в эту задачу.
        MC_set_driver_fault();
      }
      else
      {
        App_clear_error_flags(APP_ERR_TMC6200_FAULT);
      }


      res = tx_event_flags_get(&tmc6200_flags, TMC6200_RESET_FAULT, TX_OR_CLEAR,&actual_events, MS_TO_TICKS(10));
      if (res == TX_SUCCESS)
      {
        TMC6200_reset_fault_state();
      }
    }
  } while (1);
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Thread_TMC6200_create(void)
{
  UINT              err;

  tx_event_flags_create(&tmc6200_flags, "TMC6200_flags");
  err = tx_thread_create(
                         &tmc6200_thread,
                         (CHAR *)"TMC6200",
                         TMC6200_task,
                         0,
                         tmc6200_stacks,
                         TMC6200_TASK_STACK_SIZE,
                         TMC6200_TASK_PRIO,
                         TMC6200_TASK_PRIO,
                         1,
                         TX_AUTO_START
                        );

  if (err == TX_SUCCESS)
  {
    APPLOG("TMC6200 task created.");
    return RES_OK;
  }
  else
  {
    APPLOG("TMC6200 creating error %d.", err);
    return RES_ERROR;
  }
}


