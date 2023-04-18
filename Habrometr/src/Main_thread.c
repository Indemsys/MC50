#include "MC50.h"


WVAR_TYPE              wvar;
char                   g_cpu_id_str[CPU_ID_STR_LEN];
uint8_t                g_cpu_id[CPU_ID_LEN];

extern ssp_err_t sce_initialize(void);

extern uint32_t Thread_Habr_Karma_create(void);

#define FLAG_IO_REFRESH        BIT(0)
static  TX_TIMER               io_timer;
static  TX_EVENT_FLAGS_GROUP   io_flags;


/*-----------------------------------------------------------------------------------------------------


  \param v
-----------------------------------------------------------------------------------------------------*/
void IO_refresh(ULONG v)
{
  tx_event_flags_set(&io_flags, FLAG_IO_REFRESH, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param arg
-----------------------------------------------------------------------------------------------------*/
void Main_thread(ULONG arg)
{
  Get_reference_time();    // Получаем референсное время до того как будут созданны остальные задачи

  IWDT_refreshing_start(); // Организуема периодический сброс вотчдога.
                           // Включение вотчдога выполняется в настройках в файле bsp_mcu_family_cfg.h !!!

  ADC_init();

  Create_Backgroung_task();

  // Разрешаем запись в регистры пинов, проскольку ранее она была запрещена драйвером HAL
  R_PMISC->PWPR_b.BOWI  = 0;
  R_PMISC->PWPR_b.PFSWE = 1;

  g_fmi.p_api->init();
  Get_CPU_UID(g_cpu_id_str, g_cpu_id, CPU_ID_STR_LEN); // Получаем уникальный идентификатор чипа
  Init_RTC();                           // Инициализируем энергонезависимые часы реального времени
  sce_initialize();                     // Инициализация движка шифрования и генерации случайных чисел. Применяется неявно в DNS
  Init_Flash_driver();                  // Инициализируем драйвер управления записью во внутреннюю Flash и EEPROM
  Init_SD_card_file_system();
  Restore_settings(&wvars_inst);
  Init_logger();
  Thread_file_logger_create();
  Write_system_start_report();
  Accept_certificates_from_file();

  Set_usb_mode();
  if (wvar.usb_mode != USB_MODE_NONE)
  {
    Init_USB_stack();
    if (Get_usb_1_mode() == USB1_INTF_VIRTUAL_COM_PORT)
    {
      Thread_VT100_create((ULONG)Mnsdrv_get_usbfs_vcom0_driver(),0);
    }
  }

  Thread_Sound_Player_create();
  Thread_Net_create();
  Thread_FreeMaster_create();
  GUI_start();
  Thread_Habr_Karma_create();

  // Фиксируем время завершения старта основного цикла
  Get_hw_timestump(&g_main_thread_start_timestump);






  tx_event_flags_create(&io_flags, "io");
  tx_timer_create(&io_timer,"io",IO_refresh, 0, 1 , 1, TX_AUTO_ACTIVATE);

  Set_output_off(OUTP_LED_GREEN);
  Set_output_on(OUTP_LED_RED);

  do
  {
    ULONG actual_flags;
    tx_event_flags_get(&io_flags, FLAG_IO_REFRESH, TX_OR_CLEAR, &actual_flags,  2);

    Outputs_state_automat();


  } while (1);


}

