#include "MC50.h"


WVAR_TYPE              wvar;
char                   g_cpu_id_str[CPU_ID_STR_LEN];
uint8_t                g_cpu_id[CPU_ID_LEN];

extern ssp_err_t sce_initialize(void);

extern uint32_t Create_Habr_Karma_task(void);
/*-----------------------------------------------------------------------------------------------------


  \param arg
-----------------------------------------------------------------------------------------------------*/
void Main_thread(ULONG arg)
{
  Get_reference_time();    // Получаем референсное время до того как будут созданны остальные задачи

  IWDT_refreshing_start(); // Организуема периодический сброс вотчдога.
                           // Включение вотчдога выполняется в настройках в файле bsp_mcu_family_cfg.h !!!

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
  Create_file_logger_task();
  Write_system_start_report();
  Accept_certificates_from_file();

  Set_usb_mode();
  if (wvar.usb_mode != USB_MODE_NONE)
  {
    Init_USB_stack();
    if (Get_usb_1_mode() == USB1_INTF_VIRTUAL_COM_PORT)
    {
      Task_VT100_create((ULONG)Mnsdrv_get_usbfs_vcom0_driver(),0);
    }
  }

  Thread_net_create();
  Thread_FreeMaster_create();

  //Create_Habr_Karma_task();

  // Фиксируем время завершения старта основного цикла
  Get_hw_timestump(&g_main_thread_start_timestump);

  do
  {
    RED_LED   = 1;
    GREEN_LED = 0;
    Wait_ms(25);
    RED_LED   = 0;
    GREEN_LED = 0;
    Wait_ms(300);
    RED_LED   = 0;
    GREEN_LED = 1;
    Wait_ms(25);
    RED_LED   = 0;
    GREEN_LED = 0;
    Wait_ms(300);

  } while (1);


}

