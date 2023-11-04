#ifndef __PARAMS_WVAR_H
  #define __PARAMS_WVAR_H

#define  MC50_0                         0
#define  MC50_main                      1
#define  MC50_General                   2
#define  MC50_Display                   3
#define  MC50_USB_Interface             4
#define  MC50_Interfaces                5
#define  MC50_Settings                  6
#define  MC50_Network                   7
#define  MC50_MQTT                      8
#define  MC50_FTP_server                9
#define  MC50_MATLAB                    10
#define  MC50_SNTP                      11
#define  MC50_Sounds_player             12
#define  MC50_CAN_interface             13
#define  MC50_BLDC_servo_controller     14
#define  MC50_Calibrating               15
#define  MC50_CurrentLoop               16
#define  MC50_SpeedLoop                 17
#define  MC50_Movement                  18
#define  MC50_DriverIC                  19
#define  MC50_ADRC_loop                 20
#define  MC50_DC_motor_controller       21
#define  MC50_Applications              22
#define  MC50_BLDC_motor                23

typedef struct
{
  uint32_t       acceleration_time_ms;          // Acceleration time (msec) | def.val.= 1000
  float          adrc_b0;                       // Critical gain | def.val.= 10000000
  float          adrc_cbw;                      // Controller bandwidth | def.val.= 1000
  float          adrc_filter_k;                 // Speed filter value  | def.val.= 64
  float          adrc_pwm_lo_lim;               // PWM_lower_limit | def.val.= 3
  uint8_t        app_type;                      // Applicationl type (0- Servo  BLDC with OPS, 1- DC traction motor, 2- Servo  BLDC, 3 - Async.rot., 4 - Test) | def.val.= 0
  uint32_t       bldc_comm_law;                 // Commutation law (6 digit) | def.val.= 264513
  float          bldc_max_rot_speed;            // Rotor maximum rotation speed (turn/s) | def.val.= 84
  uint8_t        bldc_mot_pole_num;             // Rotor pole numbers | def.val.= 8
  float          bldc_sett_max_current;         // Maximum current during setting (A) | def.val.= 5
  uint8_t        can_id[9+1];                   // CAN ID (8 digit hex number) | def.val.= 1A00FFFF
  uint8_t        can_node_number;               // CAN node number (1...31) | def.val.= 1
  uint8_t        can_speed_select;              // Speed select (0-555Kbit, 1-100Kbit) | def.val.= 0
  uint32_t       close_position;                // Angle sensor value on 0 dgr.  | def.val.= 1060
  float          closing_margine;               // Closing margine (deg) | def.val.= 10
  uint32_t       dc_acceleration_time;          // Acceleration time (ms) | def.val.= 5000
  uint32_t       dc_deceleration_time;          // Deceleration time (ms) | def.val.= 5000
  uint32_t       dc_motor_max_pwm;              // Max PWM level (%) | def.val.= 100
  uint32_t       deceleration_time_ms ;         // Deceleration time (msec) | def.val.= 2500
  uint8_t        default_gateway_addr[16+1];    // Default gateway address | def.val.= 192.168.8.1
  uint8_t        default_ip_addr[16+1];         // Default IP address | def.val.= 192.168.8.200
  uint8_t        default_net_mask[16+1];        // Default network mask  | def.val.= 255.255.255.0
  uint8_t        en_adrc;                       // Enable ADRC algorithm (1-yes, 0-no) | def.val.= 0
  uint8_t        en_compress_settins;           // Enable compress settings file (1-yes, 0-no) | def.val.= 1
  uint8_t        en_dhcp_client;                // Enable DHCP client (0-No, 1-Yes) | def.val.= 1
  uint8_t        en_formated_settings;          // Enable formating in  settings file (1-yes, 0-no) | def.val.= 1
  uint8_t        en_matlab;                     // Enable MATLAB communication server | def.val.= 0
  uint8_t        en_sntp;                       // Enable SNTP client | def.val.= 1
  uint8_t        en_sntp_time_receiving;        // Allow to receive time from time servers | def.val.= 1
  uint8_t        en_soft_commutation;           // Enable soft commutation (1-yes, 0 -no) | def.val.= 1
  uint8_t        en_sounds;                     // Enable sounds | def.val.= 0
  uint8_t        enable_ftp_server;             // Enable FTP server | def.val.= 1
  uint8_t        enable_short_to_gnd_prot;      // Disable short to GND protection (1-Yes, 0-No) | def.val.= 1
  uint8_t        enable_short_to_vs_prot;       // Disable short to VS protection (1-Yes, 0-No) | def.val.= 1
  uint8_t        ftp_serv_login[16+1];          // Login | def.val.= ftp_login
  uint8_t        ftp_serv_password[64+1];       // Password  | def.val.= ftp_pass
  uint32_t       gate_driver_current_param;     // Gate driver current parameter (0-weak..4-strong) | def.val.= 2
  float          i_k_diff;                      // Kd (Derivative) | def.val.= 0
  float          i_k_filter;                    // Kn (Filter coefficient)  | def.val.= 0
  float          i_k_integr;                    // Ki (Integral) | def.val.= 1000
  float          i_k_prop;                      // Kp (Proporcional) | def.val.= 5
  float          i_max_out;                     // Maximum output value (% PWM) | def.val.= 100
  float          i_max_slew_rate;               // Maximum slew rate | def.val.= 100
  float          i_min_out;                     // Minimum output value (% PWM) | def.val.= 0
  float          input_shunt_resistor;          // Input shunt resistor (Ohm) | def.val.= 0.00400000018998981
  uint8_t        manuf_date[64+1];              // Manufacturing date | def.val.= 2023 07 28
  float          min_rotation_speed;            // Minimal rotation speed (deg per sec) | def.val.= 3
  uint32_t       movement_max_duration;         // Movement max duration (sec) | def.val.= 10
  uint8_t        mqtt_client_id[16+1];          // Client ID | def.val.= Client1
  uint8_t        mqtt_enable;                   // Enable MQTT client  | def.val.= 0
  uint8_t        mqtt_password[16+1];           // User password | def.val.= pass
  uint8_t        mqtt_server_ip[16+1];          // MQTT server IP address | def.val.= 192.168.3.2
  uint32_t       mqtt_server_port;              // MQTT server port number | def.val.= 1883
  uint8_t        mqtt_user_name[16+1];          // User name | def.val.= user
  uint8_t        name[64+1];                    // Product  name | def.val.= MC50
  uint32_t       open_position;                 // Angle sensor value on 90 dgr. | def.val.= 2566
  float          opening_margine;               // Opening margine (deg) | def.val.= 0
  uint8_t        opposit_sensor_mnt;            // Opposite sensor mounting (1-yes, 0-no) | def.val.= 0
  uint32_t       pwm_frequency;                 // PWM frequency (Hz) | def.val.= 16000
  uint8_t        rndis_config;                  // RNDIS interface configuration (0-Win home net, 1 - DHCP server) | def.val.= 1
  float          rotation_speed;                // Nominal rotation speed (deg per sec) | def.val.= 25
  float          s_k_diff;                      // PID Kd (Derivative) | def.val.= 0
  float          s_k_filter;                    // PID Kn (Filter coefficient)  | def.val.= 0
  float          s_k_integr;                    // PID Ki (Integral) | def.val.= 5
  float          s_k_prop;                      // PID Kp (Proporcional) | def.val.= 1
  float          s_max_out;                     // PID Maximum output value (A) | def.val.= 6
  float          s_max_slew_rate;               // PID Maximum slew rate | def.val.= 6
  float          s_min_out;                     // PID Minimum output value (A) | def.val.= 0
  uint32_t       screen_orientation;            // Screen orientation (0..3) | def.val.= 0
  uint32_t       short_det_delay_param;         // Short detection delay parameter (0..1) | def.val.= 0
  uint32_t       short_det_spike_filter;        // Spike filtering bandwidth for short detection (0-100ns..3-3us) | def.val.= 0
  uint32_t       short_gnd_det_level;           // Short to GND detector level for highside FETs (2- highest..15lowest) | def.val.= 12
  uint32_t       short_vs_det_level;            // Short to VS detector level for lowside FETs (1- highest..15-lowest) | def.val.= 12
  float          shunt_resistor;                // Shunt resistor (Ohm) | def.val.= 0.00400000018998981
  uint32_t       sntp_poll_interval;            // Poll interval (s) | def.val.= 120
  uint32_t       sound_loudness;                // Sound loudness (0...10) | def.val.= 5
  uint8_t        this_host_name[16+1];          // This device host name | def.val.= MC50
  uint8_t        time_server_1[64+1];           // Time server 1 URL | def.val.= pool.ntp.org
  uint8_t        time_server_2[64+1];           // Time server 2 URL | def.val.= 129.6.15.28
  uint8_t        time_server_3[64+1];           // Time serber 3 URL | def.val.= time.nist.gov
  uint32_t       usb_mode;                      // USB interface mode | def.val.= 4
  float          utc_offset;                    // UTC offset (difference in hours +-) | def.val.= 3
  uint32_t       voice_language;                // Language of voice | def.val.= 0
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
#define USB_MODE_VCOM_AND_FREEMASTER_PORT         4
#define USB_MODE_RNDIS                            5

// Selector description:  Confuguration of RNDIS interface
#define RNDIS_CONFIG_WINDOWS_HOME_NETWORK_        0
#define RNDIS_CONFIG_PRECONFIGURED_DHCP_SERVER    1

// Selector description:  Application type
#define APP_TYPE_SERVO_OPS_BLDC                   0
#define APP_TYPE_TRACTION_DC                      1
#define APP_TYPE_SERVO__BLDC                      2
#define APP_TYPE_ASYNC_BLDC_ROTATION              3
#define APP_TYPE_BLDC_TEST                        4


extern WVAR_TYPE  wvar;
extern const T_NV_parameters_instance wvar_inst;

