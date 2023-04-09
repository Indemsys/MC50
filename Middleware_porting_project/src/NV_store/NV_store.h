#ifndef NV_STORE_H
  #define NV_STORE_H


  #define  SETTINGS_INI_FILE_NAME              "PARAMS.INI"
  #define  SETTINGS_USED_INI_FILE_NAME         "PARAMS._NI"
  #define  SETTINGS_JSON_FILE_NAME             "Settings.json"
  #define  COMPESSED_SETTINGS_JSON_FILE_NAME   "Settings.dat"
  #define  CA_CERTIFICATE_FILE_NAME            "CA.der"


  #define COMMAND_KEY                          "OpCode"               // Идентификатор JSON блока с командой устройству
  #define MAIN_PARAMETERS_KEY                  "Parameters"           //
  #define DATETIME_SETTINGS_KEY                "DateTime"             //
  #define DEVICE_HEADER_KEY                    "Device"               //
  #define PARAMETERS_TREE_KEY                  "Parameters_tree"      //
  #define CLIENT_AP_LIST_KEY                   "Client_AP_list"       //
  #define WIFI_SCHEDULER_KEY                   "WiFi_scheduler"       //
  #define RECORDER_SCHEDULER_KEY               "Recorder_scheduler"   //
  #define FTP_SERVERS_LIST_KEY                 "FTP_servers_list"     //


  #define RESTORED_DEFAULT_SETTINGS            0
  #define RESTORED_SETTINGS_FROM_DATAFLASH     1
  #define RESTORED_SETTINGS_FROM_JSON_FILE     2
  #define RESTORED_SETTINGS_FROM_INI_FILE      3

  #define SAVED_TO_DATAFLASH_NONE              0
  #define SAVED_TO_DATAFLASH_OK                1
  #define SAVED_TO_DATAFLASH_ERROR             2


  #define  SETTINGS_START_ADDR1_IN_DATAFLASH  (DATA_FLASH_START)
  #define  SETTINGS_AREA_SIZE_IN_DATAFLASH    (0x1000)
  #define  SETTINGS_START_ADDR2_IN_DATAFLASH  (SETTINGS_START_ADDR1_IN_DATAFLASH + SETTINGS_AREA_SIZE_IN_DATAFLASH)
  #define  DATAFLASH_CHECK_BLOCK_SIZE         (4+4+4)   // Размер дополнительных данных размещаемых в DataFlash: размер юлока данных, номер записи и CRC

  #define  CA_CERT_START_IN_DATAFLASH         (SETTINGS_START_ADDR2_IN_DATAFLASH + SETTINGS_AREA_SIZE_IN_DATAFLASH)
  #define  CA_CERT_AREA_SIZE_IN_DATAFLASH     (0x1000)  // Размер области корневого сертификата
  #define  CA_CERT_SIZE_IN_DATAFLASH          (CA_CERT_START_IN_DATAFLASH + CA_CERT_AREA_SIZE_IN_DATAFLASH - 4)

  #define  MEDIA_TYPE_FILE        1
  #define  MEDIA_TYPE_DATAFLASH   2


typedef struct
{
    uint32_t dataflash_restoring_error;
    uint32_t settings_source;
    uint32_t dataflash_saving_error;

} T_settings_restore_results;


T_settings_restore_results* Get_Setting_restoring_res(void);

int32_t                     Restore_settings(const T_NV_parameters_instance *p_pars);
uint32_t                    Save_settings(const T_NV_parameters_instance *p_pars);
void                        Return_def_params(const T_NV_parameters_instance *p_pars);

uint32_t                    Restore_settings_from_INI_file(const T_NV_parameters_instance  *p_pars);
uint32_t                    Save_settings_to_INI_file(const T_NV_parameters_instance *p_pars);

uint32_t                    Save_settings_to(const T_NV_parameters_instance *p_pars, uint8_t media_type,  char *file_name);
uint32_t                    Restore_settings_from_JSON_file(const T_NV_parameters_instance  *p_pars, char *file_name);

uint32_t                    Accept_certificates_from_file(void);

void                        Delete_app_settings_file(void);

#endif



