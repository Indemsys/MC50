#ifndef MC50_H
  #define MC50_H


  #define     RAM_END                   (0x2007FFFF)


  #define     ICLK_FREQ                 120000000ul
  #define     PCLKA_FREQ                120000000ul
  #define     PCLKB_FREQ                60000000ul
  #define     PCLKC_FREQ                60000000ul
  #define     PCLKD_FREQ                120000000ul
  #define     BCLK_FREQ                 120000000ul
  #define     FCLK_FREQ                 60000000ul

  #define     ID_ADDRESS                0x0100A150  // OCD/Serial Programmer ID Setting Register (OSIS)

  #define     RES_OK     (0)
  #define     RES_ERROR  (1)

  #define     BIT(n) (1u << n)
  #define     LSHIFT(v,n) (((unsigned int)(v) << n))

// Приоритеты прерываний

  #define     INT_ADC_SCAN_PRIO            4
  #define     INT_DAC_DTC_PRIO             5
  #define     SPI0_PRIO                    6

// Приоритеты задач
  #define     THREAD_HIGHER_PRIO           2
  #define     PLAYER_TASK_PRIO             3
  #define     THREAD_BSD_PRIORITY          8
  #define     THREAD_NET_PRIORITY          9
  #define     MQTT_TASK_PRIO               10
  #define     MATLAB_TASK_PRIO             10
  #define     NX_FTP_SERVER_PRIORITY       11
  #define     FREEMASTER_TASK_PRIO         12
  #define     VT100_TASK_PRIO              13
  #define     LOGGER_TASK_PRIO             15
  #define     BACKGROUND_TASK_PRIO         30  // Низший приоритет для процесса IDLE измеряющего нагрузку процессора (следить за приоритетами сетевых задач!)


  #define     STR_CRLF                     "\r\n"
  #define     CPU_ID_STR_LEN               33
  #define     CPU_ID_LEN                   16

  #define     LCD_BLK                      R_PFS->P214PFS_b.PODR
  #define     LCD_DC                       R_PFS->P211PFS_b.PODR
  #define     LCD_RST                      R_PFS->P210PFS_b.PODR
  #define     RED_LED                      R_PFS->P504PFS_b.PODR
  #define     GREEN_LED                    R_PFS->P508PFS_b.PODR
  #define     CAN_LED                      R_PFS->P112PFS_b.PODR
  #define     RS485_LED                    R_PFS->P115PFS_b.PODR

  #define     APP_DO_SYSTEM_RESTART                    BIT(0)

  #define     WINDOWS_DIR                              "System Volume Information"
  #define     LOG_FILE_NAME                            "log.txt"
  #define     LOG_FILE_PATH                            LOG_FILE_NAME


  #define     PARAMS_SCHEMA_FILE_NAME                  "PS.jsn"
  #define     PARAMS_VALUES_FILE_NAME                  "PV.jsn"
  #define     FILES_LIST_FILE_NAME                     "FL.jsn"
  #define     PACKED_FILES_LIST_FILE_NAME              "FL.pck"
  #define     COMPRESSED_STREAM_FILE_NAME              "tmpc.dat"
  #define     UNCOMPESSED_STREAM_FILE_NAME             "tmpu.dat"


  #include    <ctype.h>
  #include    <stdint.h>
  #include    <limits.h>
  #include    <math.h>
  #include    <time.h>
  #include    <arm_itm.h>
  #include    <stdarg.h>

typedef struct
{
    uint32_t cycles;
    uint32_t ticks;

} T_sys_timestump;


  #define  SYSTEM_CLOCK    120000000
  #define  DELAY_1us       Delay_m7(33/2)           // 0.992       мкс при частоте   120 МГц
  #define  DELAY_4us       Delay_m7(136/2)          // 3.996       мкс при частоте   120 МГц
  #define  DELAY_8us       Delay_m7(273/2)          // 7.992       мкс при частоте   120 МГц
  #define  DELAY_32us      Delay_m7(1096/2)         // 31.996      мкс при частоте   120 МГц
  #define  DELAY_100us     Delay_m7(3428/2)         // ~100        мкс при частоте   120 МГц
  #define  DELAY_ms(x)     Delay_m7((34285*x)/2-1)    // 1000.008*N  мкс при частоте 120 МГц

extern void Delay_m7(int cnt); // Задержка на (cnt+1)*7 тактов . Передача нуля не допускается

  #include "bsp_api.h"
  #include "tx_api.h"
  #include "fx_api.h"
  #include "ux_api.h"
  #include "nx_api.h"
  #include "nx_ip.h"
  #include "gx_api.h"
  #include "nxd_mdns.h"
  #include "tx_timer.h"
  #include "hal_data.h"
  #include "sf_el_fx.h"
  #include "sf_block_media_api.h"
  #include "sf_block_media_sdmmc.h"
  #include "sf_spi_api.h"
  #include "sf_i2c.h"
  #include "sf_i2c_api.h"
  #include "sf_spi.h"
  #include "ux_dcd_synergy.h"
  #include "ux_device_class_cdc_acm.h"
  #include "ux_device_class_storage.h"
  #include "ux_host_class_cdc_acm.h"
  #include "ux_host_class_cdc_ecm.h"
  #include "s7_sdmmc.h"
  #include "s7_sdmmc_utils.h"

  #include "Params_Types.h"
  #include "MC50_Params.h"
  #include "Pins.h"
  #include "SPI0.h"
  #include "DAC.h"
  #include "AGT.h"
  #include "ADC.h"
  #include "TFT_driver.h"
  #include "ELC_tbl.h"
  #include "Flasher.h"
  #include "Transfer_channels.h"
  #include "Watchdog_controller.h"
  #include "CHIP_utils.h"
  #include "Realtime_clock.h"
  #include "Sound_DAC.h"
  #include "compress.h"

  #include "SEGGER_RTT.h"
  #include "jansson.h"
  #include "Memory_manager.h"


  #include "Time_utils.h"
  #include "String_utils.h"
  #include "DSP_Filters.h"
  #include "CRC_utils.h"

  #include "FS_init.h"
  #include "FS_utils.h"

  #include "USB_init.h"
  #include "USB_storage.h"

  #include "Logger.h"
  #include "Start_report.h"

  #include "Main_monitor.h"
  #include "Params_editor.h"
  #include "Flash_monitor.h"
  #include "Chip_monitor.h"
  #include "SD_card_monitor.h"
  #include "System_info_monitor.h"
  #include "Player_monitor.h"
  #include "File_system_test.h"
  #include "Monitor_rtt_drv.h"
  #include "Monitor_USB_FS_drv.h"

  #include "NV_store.h"
  #include "JSON_serializer.h"
  #include "JSON_deserializer.h"

  #include "freemaster.h"
  #include "FreeMaster_thread.h"
  #include "Background_thread.h"

  #include "nxd_telnet_server.h"
  #include "nxd_mqtt_client.h"
  #include "nxd_sntp_client.h"
  #include "Net_common.h"
  #include "Net_utils.h"
  #include "Net_thread.h"
  #include "Net_mDNS.h"
  #include "USB_RNDIS_network.h"
  #include "USB_host_cdc_ecm.h"
  #include "USB_ECM_Host_network.h"
  #include "Net_Telnet_serv_driver.h"
  #include "Net_DNS.h"
  #include "NXD_exFAT_ftp_server.h"
  #include "Net_MQTT_client_man.h"
  #include "Net_MQTT_Msg_Controller.h"
  #include "Net_FTP_server_man.h"
  #include "Net_TCP_server.h"
  #include "Net_MATLAB_connection.h"
  #include "Net_SNTP_client.h"

  #include "Outputs_control.h"
  #include "Encoder_handler.h"
  #include "Sound_player.h"
  #include "HMI.h"




extern  WVAR_TYPE               wvar;
extern  char                    g_cpu_id_str[CPU_ID_STR_LEN];
extern  uint8_t                 g_cpu_id[CPU_ID_LEN];
extern  uint8_t                 rstsr0;
extern  uint16_t                rstsr1;
extern  uint8_t                 rstsr2;
extern  uint32_t                g_file_system_ready;
extern  uint32_t                g_sd_card_status;
extern  uint32_t                g_fs_free_space_val_ready;
extern  uint64_t                g_fs_free_space;
extern  T_sys_timestump         g_main_thread_start_timestump;

void    Main_thread(ULONG arg);
void    Request_save_app_settings(void);
uint8_t Is_need_save_app_settings(void);



#endif

