#include "MC50.h"
#include "stdatomic.h"

char                   g_cpu_id_str[CPU_ID_STR_LEN];
uint8_t                g_cpu_id[CPU_ID_LEN];
atomic_uint            g_app_errors;
uint8_t                g_sys_ready;
TX_EVENT_FLAGS_GROUP   app_flags;


extern ssp_err_t       sce_initialize(void);

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void App_create_sync_obj(void)
{
  tx_event_flags_create(&app_flags, "app_flags");
}

/*-----------------------------------------------------------------------------------------------------


  \param flags

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t App_set_flags(uint32_t flags)
{
  return  tx_event_flags_set(&app_flags, flags, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param flags

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t App_wait_flags(uint32_t flags, uint32_t *actual_events, uint32_t timeout_ms)
{
  return tx_event_flags_get(&app_flags, flags, TX_OR_CLEAR, (ULONG*)actual_events, MS_TO_TICK(timeout_ms));
}

/*-----------------------------------------------------------------------------------------------------


  \param flag
-----------------------------------------------------------------------------------------------------*/
inline void App_set_error_flags(uint32_t flag)
{
  atomic_fetch_or(&g_app_errors, flag);
  //  __disable_interrupt();
  //  g_app_errors |= flag;
  //  __enable_interrupt();
}

/*-----------------------------------------------------------------------------------------------------


  \param flag
-----------------------------------------------------------------------------------------------------*/
inline void App_clear_error_flags(uint32_t flag)
{
  atomic_fetch_and(&g_app_errors, ~flag);
  //  __disable_interrupt();
  //  g_app_errors &= ~flag;
  //  __enable_interrupt();
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
inline uint32_t App_get_error_flags(void)
{
  return atomic_load(&g_app_errors);
}

/*-----------------------------------------------------------------------------------------------------


  \param app_err
-----------------------------------------------------------------------------------------------------*/
void Log_app_errors(int32_t app_err)
{
  APPLOG("App errors bitmask: %08X", app_err);
  if (app_err & APP_ERR_NO_TMC6200)          APPLOG("E: No comm. with driver IC");
  if (app_err & APP_ERR_TMC6200_FAULT)       APPLOG("E: Driver IC fault");
  if (app_err & APP_ERR_HALL_STATE_FAULT)    APPLOG("E: Hall sens.err.");
  if (app_err & APP_ERR_LOW_PWR_SRC_VOLTAGE) APPLOG("E: Low pwr.src. voltage");
  if (app_err & APP_ERR_CPU_OVERHEAT)        APPLOG("E: CPU overheat");
  if (app_err & APP_ERR_DRV_OVERHEAT)        APPLOG("E: PCB overheat");
  if (app_err & APP_ERR_MOT_OVERHEAT)        APPLOG("E: Motor overheat");
}

/*-----------------------------------------------------------------------------------------------------


  \param arg
-----------------------------------------------------------------------------------------------------*/
void Main_thread(ULONG arg)
{
  // Разрешаем запись в регистры пинов, проскольку ранее она была запрещена драйвером HAL
  R_PMISC->PWPR_b.BOWI  = 0;
  R_PMISC->PWPR_b.PFSWE = 1;

  Get_reference_time();         // Получаем референсное время до того как будут созданны остальные задачи

  IWDT_refreshing_start();      // Организуема периодический сброс вотчдога.
                                // Включение вотчдога выполняется в настройках в файле bsp_mcu_family_cfg.h !!!

  App_create_sync_obj();
  Create_Backgroung_task();


  g_fmi.p_api->init();
  Get_CPU_UID(g_cpu_id_str, g_cpu_id, CPU_ID_STR_LEN); // Получаем уникальный идентификатор чипа
  Init_RTC();                           // Инициализируем энергонезависимые часы реального времени
  sce_initialize();                     // Инициализация движка шифрования и генерации случайных чисел. Применяется неявно в mDNS
  Init_Flash_driver();                  // Инициализируем драйвер управления записью во внутреннюю Flash и EEPROM
  Init_SD_card_file_system();
  Restore_settings(&wvar_inst);

  Init_logger();
  Write_system_start_report();

  Thread_file_logger_create();
  Thread_CAN_create();

  Init_AGT0_ticks_man_enc(AGT_CLOCK_FREQ / MAN_ENC_SAMLE_RATE);

  Set_usb_mode();
  if (wvar.usb_mode != USB_MODE_NONE)
  {
    Init_USB_stack();
    if (Get_usb_1_mode() == USB1_INTF_VIRTUAL_COM_PORT)
    {
      Thread_VT100_create((ULONG)Mnsdrv_get_usbfs_vcom0_driver(),0);
    }
  }

  Thread_Outputs_create();
  Thread_Sound_Player_create();
  Thread_Net_create();
  Thread_FreeMaster_create();
  GUI_start();


  // Фиксируем время завершения старта основного цикла
  Get_hw_timestump(&g_main_thread_start_timestump);

  g_sys_ready = 1;

  if (wvar.app_type == APP_TYPE_SERVO_OPS_BLDC)
  {
    Servo_OPS_BLDC_thread();
  }
  else if (wvar.app_type == APP_TYPE_TRACTION_DC)
  {
    DC_motor_thread();
  }
  else if (wvar.app_type == APP_TYPE_SERVO__BLDC)
  {
    Servo_BLDC_thread();
  }
  else if (wvar.app_type == APP_TYPE_ASYNC_BLDC_ROTATION)
  {
    Asinc_Sinusoidal_BLDC_rotation_thread();
  }
  else if (wvar.app_type == APP_TYPE_BLDC_TEST)
  {
    BLDC_mot_test_thread();
  }

}

