#include "MC50.h"

char                   g_cpu_id_str[CPU_ID_STR_LEN];
uint8_t                g_cpu_id[CPU_ID_LEN];
uint32_t               g_app_errors;


extern ssp_err_t sce_initialize(void);

/*-----------------------------------------------------------------------------------------------------


  \param flag
-----------------------------------------------------------------------------------------------------*/
void App_set_error_flags(uint32_t flag)
{
  __disable_interrupt();
  g_app_errors |= flag;
  __enable_interrupt();
}

/*-----------------------------------------------------------------------------------------------------


  \param flag
-----------------------------------------------------------------------------------------------------*/
void App_clear_error_flags(uint32_t flag)
{
  __disable_interrupt();
  g_app_errors &= ~flag;
  __enable_interrupt();
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t App_get_error_flags(void)
{
  return g_app_errors;
}

/*-----------------------------------------------------------------------------------------------------


  \param app_err
-----------------------------------------------------------------------------------------------------*/
void Log_app_errors(int32_t app_err)
{
   APPLOG("App errors bitmask: %08X", app_err);
   if (app_err & APP_ERR_NO_TMC6200      )  APPLOG("E: No comm. with driver IC");
   if (app_err & APP_ERR_TMC6200_FAULT   )  APPLOG("E: Driver IC fault");
   if (app_err & APP_ERR_HALL_STATE_FAULT)  APPLOG("E: Hall sens.err.");
   if (app_err & APP_ERR_LOW_VOLTAGE     )  APPLOG("E: Low voltage");
   if (app_err & APP_ERR_CPU_OVERHEAT    )  APPLOG("E: CPU overheat");
   if (app_err & APP_ERR_DRV_OVERHEAT    )  APPLOG("E: PCB overheat");
   if (app_err & APP_ERR_MOT_OVERHEAT    )  APPLOG("E: Motor overheat");
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

  GPT_11_12_Capture_init();     // Инициализируем измерение скрости вращения двигателя по сигналам с датчиков Холла
  GPT0_PWM_init(PWM_FREQ_HZ);   // Инициализируем PWM предназначенный для управления мотором
  Filters_init();               // Инициализация фильтров измеряемых сигналов
  ADC_init();                   // Инициализируем ADC работающий синхронно с PWM мотора


  MC_create_sync_obj();
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


  Motor_control_thread();

}

