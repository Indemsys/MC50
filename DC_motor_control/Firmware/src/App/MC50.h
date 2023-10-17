#ifndef MC50_H
  #define MC50_H

  #define   CURR_SHUNT_RES              0.001f         // Сопротивление шунта измерителей тока
  #define   MIN_PWM_PULSE_nS            500ll          // Минимальная длительность импульса ШИМ в наносекундах
  #define   MIN_PWM_COMPARE_VAL         ((MIN_PWM_PULSE_nS*PCLKD_FREQ)/2000000000ll)
  #define   PWM_DEAD_TIME_nS            100ll             // Задержка от момента выключения верхнего сигнала до включения нижнего в наносекундах
  #define   PWM_DEAD_TIME_VAL           ((PWM_DEAD_TIME_nS*PCLKD_FREQ)/1000000000ll)

  #define   MAN_ENC_SAMLE_RATE          10000ul          // Частота сэмплирования ручного энкодера на плате для пользовательского управления

  #define   RAM_END                     (0x2007FFFF)


  #define   ICLK_FREQ                   120000000ul
  #define   PCLKA_FREQ                  120000000ul
  #define   PCLKB_FREQ                  60000000ul
  #define   PCLKC_FREQ                  60000000ul
  #define   PCLKD_FREQ                  120000000ul
  #define   BCLK_FREQ                   120000000ul
  #define   FCLK_FREQ                   60000000ul

  #define   ID_ADDRESS                  0x0100A150           // OCD/Serial Programmer ID Setting Register (OSIS)

  #define   MAX_CAN_NODE_NUM            31

  #define   SPEED_LOOP_FREQ_HZ          (1000)               // Частота петли управления скоростью
  #define   CURRENT_LOOP_FREQ_HZ        (4000)               // Частота петли управления током

  #define   VREF                        (3.3f)               // Напряжение питания AVDD
  #define   REF_VOLTAGE                 (VREF/2.0f)          // Референсное напряжение


  #define   I_PWR_AMPL                  50.0f                // Усиление измерителя тока в шине питания устройства
  #define   V_PWR_DIV                   (5.1f/(100.0f+5.1f)) // Делитель в цепи измерени напряжения шины питания

  #define   MINIMAL_POWER_VOLTAGE       (18.0f)
  #define   NORMAL_POWER_VOLTAGE        (22.0f)
  #define   MAXIMAL_DRIVER_TEMP         (80.0f)
  #define   MAXIMAL_MOTOR_TEMP          (80.0f)
  #define   MAXIMAL_CPU_TEMP            (80.0f)

  #define   CURRENT_PULSE_LEN_MS        (1500)               // Длительность импульса тока при выполнении команд подачи импульса на открытие или закрытие
  #define   CURRENT_PULSE_VALUE         (6.0f)               // Величина тока импульса

  #define   MIN_ROT_SPEED               (10.0f)
  #define   MAX_ROT_SPEED               (45.0f)

  #define     RES_OK                    (0)
  #define     RES_ERROR                 (1)

  #define     BIT(n)                    (1u << n)
  #define     LSHIFT(v,n)               (((unsigned int)(v) << n))



// ....................................................................
// Таблица использования таймеров
//
//  R_GPTA0 (GPT0)  : Тип таймера - GPT32EH0. Используеся для управления коммутацией BLDC мотора с использованием функции Output Phase Switching Control (OPS)
//                                            Альтернативно используеся для формировния ШИМ сигналов для фазы U драйвера мотора
//  R_GPTA1 (GPT1)  : Тип таймера - GPT32EH0. Используеся для формировния ШИМ сигналов для фазы V драйвера мотора
//  R_GPTA2 (GPT2)  : Тип таймера - GPT32EH0. Используеся для формировния ШИМ сигналов для фазы W драйвера мотора
//
//  R_GPTB3 (GPT11) : Тип таймера - GPT32.    Два канала измерения длительности импульсов с датчиков Холла. Прием сигналов ENC_U, ENC_V
//  R_GPTB4 (GPT12) : Тип таймера - GPT32.    Один канал измерения длительности импульсов с датчиков Холла. Прием сигналов ENC_W
//  R_GPTB5 (GPT13) : Тип таймера - GPT32.    Используется для определения длительного отсутствия импульсов на входах ENC_U, ENC_V, ENC_W
//
//  R_AGT0                                    Используется для для обслуживания ручного энкодера
//  R_AGT1                                    Используется для стробирования DAC при формировании звука
// ....................................................................


// Приоритеты прерываний

  #define     INT_ADC_SCAN_PRIO                        1
  #define     TMC6200_FAULT_IRQ0_INT_PRIO              2
  #define     INT_DAC_DTC_PRIO                         3
  #define     INT_CAN0_MAILBOX_TX_PRIO                 4
  #define     AGT0_PRIO                                5  // Приоритет прерывания обслуживающего ручной энкодер
  #define     INT_CAN0_ERROR_PRIO                      6
  #define     INT_CAN0_FIFO_RX_PRIO                    6
  #define     INT_CAN0_FIFO_TX_PRIO                    6
  #define     INT_CAN0_MAILBOX_RX_PRIO                 6
  #define     SPI0_PRIO                                7  // Приоритет прерывания канала SPI используемого для генерации события приводящего к отсылке данных на дисплей

// Приоритеты задач
  #define     THREAD_HIGHER_PRIO                       2
  #define     OUTPUTS_TASK_PRIO                        3
  #define     PLAYER_TASK_PRIO                         4
  #define     TMC6200_TASK_PRIO                        5
  #define     CAN_TASK_PRIO                            6
  #define     THREAD_BSD_PRIORITY                      8
  #define     THREAD_NET_PRIORITY                      9
  #define     MQTT_TASK_PRIO                           10
  #define     MATLAB_TASK_PRIO                         10
  #define     NX_FTP_SERVER_PRIORITY                   11
  #define     FREEMASTER_TASK_PRIO                     12
  #define     VT100_TASK_PRIO                          13
  #define     LOGGER_TASK_PRIO                         15
  #define     BACKGROUND_TASK_PRIO                     30  // Низший приоритет для процесса IDLE измеряющего нагрузку процессора (следить за приоритетами сетевых задач!)


  #define     STR_CRLF                                 "\r\n"
  #define     CPU_ID_STR_LEN                           33
  #define     CPU_ID_LEN                               16

  #define     LCD_BLK                                  R_PFS->P214PFS_b.PODR
  #define     LCD_DC                                   R_PFS->P211PFS_b.PODR
  #define     LCD_RST                                  R_PFS->P210PFS_b.PODR
  #define     RED_LED                                  R_PFS->P504PFS_b.PODR
  #define     GREEN_LED                                R_PFS->P508PFS_b.PODR
  #define     CAN_LED                                  R_PFS->P112PFS_b.PODR
  #define     RS485_LED                                R_PFS->P115PFS_b.PODR
  #define     MDRV_SC                                  R_PFS->P104PFS_b.PODR
  #define     MDRV_EN                                  R_PFS->P111PFS_b.PODR

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


  #define     APP_ERR_NO_TMC6200                        BIT(1)  // 2   Ошибка. Нет связи с драйвером TMC6200
  #define     APP_ERR_TMC6200_FAULT                     BIT(2)  // 4   Ошибка. Сигнал Fault в драйвере TMC6200
  #define     APP_ERR_HALL_STATE_FAULT                  BIT(3)  // 8   Ошибка.
  #define     APP_ERR_LOW_PWR_SRC_VOLTAGE                       BIT(5)  // 32  Ошибка. Слишком низкое напряжение на DC шине
  #define     APP_ERR_CPU_OVERHEAT                      BIT(6)  // 64  Ошибка. Перегрев CPU
  #define     APP_ERR_DRV_OVERHEAT                      BIT(7)  // 128 Ошибка. Перегрев драйверов
  #define     APP_ERR_MOT_OVERHEAT                      BIT(8)  // 256 Ошибка. Перегрев мотора



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

  #include "USB_descriptors.h"
  #include "Params_Types.h"
  #include "MC50_Params.h"
  #include "Pins.h"
  #include "GPT32EH0_PWM.h"
  #include "GPT32EH0_1_2_PWM.h"
  #include "GPT_11_12_Capture.h"
  #include "SPI0.h"
  #include "DAC.h"
  #include "AGT.h"
  #include "ADC.h"
  #include "CAN0.h"
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

  #include "App_utils.h"
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

  #include "Common_control.h"
  #include "Hall_3PH_encoder.h"
  #include "Quad_encoder.h"
  #include "PIDController.h"
  #include "ADRCcontroller.h"
  #include "Outputs_control.h"
  #include "Sound_player.h"
  #include "Manual_Encoder.h"
  #include "HMI.h"
  #include "MotDriver_TMC6200.h"
  #include "Motor_OPS_BLDC_control.h"
  #include "DC_servo_control.h"
  #include "Measurements.h"
  #include "CAN_bus_defs.h"
  #include "CAN_communication.h"
  #include "Speed_S-curve.h"
  #include "Current_PID.h"
  #include "Speed_PID.h"
  #include "Motor_OPS_BLDC_control.h"
  #include "Servo_OPS_BLDC_control.h"

extern  uint8_t                 g_sys_ready;
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


void            App_create_sync_obj(void);
uint32_t        App_set_flags(uint32_t flags);
uint32_t        App_wait_flags(uint32_t flags, uint32_t *actual_events, uint32_t timeout);
void            App_set_error_flags(uint32_t flag);
void            App_clear_error_flags(uint32_t flag);
uint32_t        App_get_error_flags(void);

void            Main_thread(ULONG arg);
void            Request_save_app_settings(void);
uint8_t         Is_need_save_app_settings(void);
void            Log_app_errors(int32_t app_err);

#endif

