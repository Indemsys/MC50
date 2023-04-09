#ifndef __PARAMS_H
  #define __PARAMS_H

#define  APP_PROFILE        MC50
#define  MAIN_PARAMS_ROOT   MC50_main
#define  PARAMS_ROOT        MC50_0


#define  MC50_0                         0
#define  MC50_main                      1
#define  MC50_General                   2
#define  MC50_Display                   3
#define  MC50_USB_Interface             4
#define  MC50_Controller                5
#define  MC50_Settings                  6
#define  MC50_Network                   7
#define  MC50_MQTT                      8
#define  MC50_FTP_server                9
#define  MC50_MATLAB                    10
#define  MC50_SNTP                      11

typedef struct
{
  uint32_t       def_screen_num;                // Default screen number | def.val.= 0
  uint8_t        default_gateway_addr[16];      // Default gateway address | def.val.= 192.168.8.1
  uint8_t        default_ip_addr[16];           // Default IP address | def.val.= 192.168.8.200
  uint8_t        default_net_mask[16];          // Default network mask  | def.val.= 255.255.255.0
  uint8_t        en_compress_settins;           // Enable compress settings file (1-yes, 0-no) | def.val.= 1
  uint8_t        en_dhcp_client;                // Enable DHCP client (0-No, 1-Yes) | def.val.= 1
  uint8_t        en_formated_settings;          // Enable formating in  settings file (1-yes, 0-no) | def.val.= 1
  uint8_t        en_matlab;                     // Enable MATLAB communication server | def.val.= 0
  uint8_t        en_sntp;                       // Enable SNTP client | def.val.= 1
  uint8_t        en_sntp_time_receiving;        // Allow to receive time from time servers | def.val.= 1
  uint8_t        enable_ftp_server;             // Enable FTP server | def.val.= 1
  uint8_t        ftp_serv_login[16];            // Login | def.val.= ftp_login
  uint8_t        ftp_serv_password[64];         // Password  | def.val.= ftp_pass
  uint8_t        habr_user_name[64];            // Demo task user name | def.val.= User
  uint8_t        manuf_date[64];                // Manufacturing date | def.val.= 2023 03 26
  uint8_t        mqtt_client_id;                // Client ID | def.val.= 0
  uint8_t        mqtt_enable;                   // Enable MQTT client  | def.val.= 0
  uint8_t        mqtt_password[16];             // User password | def.val.= pass
  uint8_t        mqtt_server_ip[16];            // MQTT server IP address | def.val.= 192.168.3.2
  uint32_t       mqtt_server_port;              // MQTT server port number | def.val.= 1883
  uint8_t        mqtt_user_name[16];            // User name | def.val.= user
  uint8_t        name[64];                      // Product  name | def.val.= MC50
  uint8_t        rndis_config;                  // RNDIS interface configuration (0-Win home net, 1 - DHCP server) | def.val.= 1
  uint32_t       sntp_poll_interval;            // Poll interval (s) | def.val.= 120
  uint8_t        this_host_name[16];            // This device host name | def.val.= MC50
  uint8_t        time_server_1[64];             // Time server 1 URL | def.val.= pool.ntp.org
  uint8_t        time_server_2[64];             // Time server 2 URL | def.val.= 129.6.15.28
  uint8_t        time_server_3[64];             // Time serber 3 URL | def.val.= time.nist.gov
  uint32_t       usb_mode;                      // USB interface mode | def.val.= 6
  float          utc_offset;                    // UTC offset (difference in hours +-) | def.val.= 3
} WVAR_TYPE;


#endif



// Selector description:  Выбор между Yes и No
#define BINARY_NO                                 0
#define BINARY_YES                                1

// Selector description:  LED mode
#define LEDS_MODE_ALWAYS_OFF                      0
#define LEDS_MODE_NORMAL_WORK                     1

// Selector description:  USB mode
#define USB_MODE_HOST_ECM                         6
#define USB_MODE_NONE                             0
#define USB_MODE_VCOM_PORT                        1
#define USB_MODE_MASS_STORAGE_                    2
#define USB_MODE_VCOM_AND_MASS_STORAGE            3
#define USB_MODE_VCOM_AND_VCOM_PORT               4
#define USB_MODE_RNDIS                            5

// Selector description:  Confuguration of RNDIS interface
#define RNDIS_CONFIG_WINDOWS_HOME_NETWORK_        0
#define RNDIS_CONFIG_PRECONFIGURED_DHCP_SERVER    1


extern const T_NV_parameters_instance wvars_inst;

