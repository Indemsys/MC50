#ifndef __FREEMASTER_VARS
  #define __FREEMASTER_VARS
#include "MC50.h"
#include "freemaster_tsa.h"


FMSTR_TSA_TABLE_BEGIN(wvar_vars)
FMSTR_TSA_RW_VAR( wvar.def_screen_num                  ,FMSTR_TSA_UINT32    ) // Default screen number | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.en_compress_settins             ,FMSTR_TSA_UINT8     ) // Enable compress settings file (1-yes, 0-no) | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.en_dhcp_client                  ,FMSTR_TSA_UINT8     ) // Enable DHCP client (0-No, 1-Yes) | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.en_formated_settings            ,FMSTR_TSA_UINT8     ) // Enable formating in  settings file (1-yes, 0-no) | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.en_matlab                       ,FMSTR_TSA_UINT8     ) // Enable MATLAB communication server | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.en_sntp                         ,FMSTR_TSA_UINT8     ) // Enable SNTP client | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.en_sntp_time_receiving          ,FMSTR_TSA_UINT8     ) // Allow to receive time from time servers | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.enable_ftp_server               ,FMSTR_TSA_UINT8     ) // Enable FTP server | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.mqtt_client_id                  ,FMSTR_TSA_UINT8     ) // Client ID | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.mqtt_enable                     ,FMSTR_TSA_UINT8     ) // Enable MQTT client  | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.mqtt_server_port                ,FMSTR_TSA_UINT32    ) // MQTT server port number | def.val.= 1883
FMSTR_TSA_RW_VAR( wvar.rndis_config                    ,FMSTR_TSA_UINT8     ) // RNDIS interface configuration (0-Win home net, 1 - DHCP server) | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.sntp_poll_interval              ,FMSTR_TSA_UINT32    ) // Poll interval (s) | def.val.= 120
FMSTR_TSA_RW_VAR( wvar.usb_mode                        ,FMSTR_TSA_UINT32    ) // USB interface mode | def.val.= 6
FMSTR_TSA_RW_VAR( wvar.utc_offset                      ,FMSTR_TSA_FLOAT     ) // UTC offset (difference in hours +-) | def.val.= 3
FMSTR_TSA_TABLE_END();


#endif
