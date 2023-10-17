#ifndef __FREEMASTER_WVAR_H
  #define __FREEMASTER_WVAR_H
#include "MC50.h"
#include "freemaster_tsa.h"


FMSTR_TSA_TABLE_BEGIN(wvar_tbl)
FMSTR_TSA_RW_VAR( wvar.acceleration_time_ms            ,FMSTR_TSA_UINT32    ) // Acceleration time (msec) | def.val.= 1000
FMSTR_TSA_RW_VAR( wvar.adrc_b0                         ,FMSTR_TSA_FLOAT     ) // Critical gain | def.val.= 10000000
FMSTR_TSA_RW_VAR( wvar.adrc_cbw                        ,FMSTR_TSA_FLOAT     ) // Controller bandwidth | def.val.= 1000
FMSTR_TSA_RW_VAR( wvar.adrc_filter_k                   ,FMSTR_TSA_FLOAT     ) // Speed filter value  | def.val.= 64
FMSTR_TSA_RW_VAR( wvar.adrc_pwm_lo_lim                 ,FMSTR_TSA_FLOAT     ) // PWM_lower_limit | def.val.= 3
FMSTR_TSA_RW_VAR( wvar.app_type                        ,FMSTR_TSA_UINT8     ) // Applicationl type (0- Servo  BLDC with OPS, 1- DC traction motor) | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.can_node_number                 ,FMSTR_TSA_UINT8     ) // CAN node number (1...31) | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.can_speed_select                ,FMSTR_TSA_UINT8     ) // Speed select (0-555Kbit, 1-100Kbit) | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.close_position                  ,FMSTR_TSA_UINT32    ) // Angle sensor value on 0 dgr.  | def.val.= 1060
FMSTR_TSA_RW_VAR( wvar.closing_margine                 ,FMSTR_TSA_FLOAT     ) // Closing margine (deg) | def.val.= 10
FMSTR_TSA_RW_VAR( wvar.dc_acceleration_time            ,FMSTR_TSA_UINT32    ) // Acceleration time (ms) | def.val.= 5000
FMSTR_TSA_RW_VAR( wvar.dc_deceleration_time            ,FMSTR_TSA_UINT32    ) // Deceleration time (ms) | def.val.= 5000
FMSTR_TSA_RW_VAR( wvar.dc_motor_max_pwm                ,FMSTR_TSA_UINT32    ) // Max PWM level (%) | def.val.= 100
FMSTR_TSA_RW_VAR( wvar.deceleration_time_ms            ,FMSTR_TSA_UINT32    ) // Deceleration time (msec) | def.val.= 2500
FMSTR_TSA_RW_VAR( wvar.en_adrc                         ,FMSTR_TSA_UINT8     ) // Enable ADRC algorithm (1-yes, 0-no) | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.en_compress_settins             ,FMSTR_TSA_UINT8     ) // Enable compress settings file (1-yes, 0-no) | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.en_dhcp_client                  ,FMSTR_TSA_UINT8     ) // Enable DHCP client (0-No, 1-Yes) | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.en_formated_settings            ,FMSTR_TSA_UINT8     ) // Enable formating in  settings file (1-yes, 0-no) | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.en_matlab                       ,FMSTR_TSA_UINT8     ) // Enable MATLAB communication server | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.en_sntp                         ,FMSTR_TSA_UINT8     ) // Enable SNTP client | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.en_sntp_time_receiving          ,FMSTR_TSA_UINT8     ) // Allow to receive time from time servers | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.en_soft_commutation             ,FMSTR_TSA_UINT8     ) // Enable soft commutation (1-yes, 0 -no) | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.en_sounds                       ,FMSTR_TSA_UINT8     ) // Enable sounds | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.enable_ftp_server               ,FMSTR_TSA_UINT8     ) // Enable FTP server | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.enable_short_to_gnd_prot        ,FMSTR_TSA_UINT8     ) // Disable short to GND protection (1-Yes, 0-No) | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.enable_short_to_vs_prot         ,FMSTR_TSA_UINT8     ) // Disable short to VS protection (1-Yes, 0-No) | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.gate_driver_current_param       ,FMSTR_TSA_UINT32    ) // Gate driver current parameter (0-weak..4-strong) | def.val.= 2
FMSTR_TSA_RW_VAR( wvar.i_k_diff                        ,FMSTR_TSA_FLOAT     ) // Kd (Derivative) | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.i_k_filter                      ,FMSTR_TSA_FLOAT     ) // Kn (Filter coefficient)  | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.i_k_integr                      ,FMSTR_TSA_FLOAT     ) // Ki (Integral) | def.val.= 1000
FMSTR_TSA_RW_VAR( wvar.i_k_prop                        ,FMSTR_TSA_FLOAT     ) // Kp (Proporcional) | def.val.= 5
FMSTR_TSA_RW_VAR( wvar.i_max_out                       ,FMSTR_TSA_FLOAT     ) // Maximum output value (% PWM) | def.val.= 100
FMSTR_TSA_RW_VAR( wvar.i_max_slew_rate                 ,FMSTR_TSA_FLOAT     ) // Maximum slew rate | def.val.= 100
FMSTR_TSA_RW_VAR( wvar.i_min_out                       ,FMSTR_TSA_FLOAT     ) // Minimum output value (% PWM) | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.input_shunt_resistor            ,FMSTR_TSA_FLOAT     ) // Input shunt resistor (Ohm) | def.val.= 0.00400000018998981
FMSTR_TSA_RW_VAR( wvar.min_rotation_speed              ,FMSTR_TSA_FLOAT     ) // Minimal rotation speed (deg per sec) | def.val.= 3
FMSTR_TSA_RW_VAR( wvar.movement_max_duration           ,FMSTR_TSA_UINT32    ) // Movement max duration (sec) | def.val.= 10
FMSTR_TSA_RW_VAR( wvar.mqtt_enable                     ,FMSTR_TSA_UINT8     ) // Enable MQTT client  | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.mqtt_server_port                ,FMSTR_TSA_UINT32    ) // MQTT server port number | def.val.= 1883
FMSTR_TSA_RW_VAR( wvar.open_position                   ,FMSTR_TSA_UINT32    ) // Angle sensor value on 90 dgr. | def.val.= 2566
FMSTR_TSA_RW_VAR( wvar.opening_margine                 ,FMSTR_TSA_FLOAT     ) // Opening margine (deg) | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.opposit_sensor_mnt              ,FMSTR_TSA_UINT8     ) // Opposite sensor mounting (1-yes, 0-no) | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.pwm_frequency                   ,FMSTR_TSA_UINT32    ) // PWM frequency (Hz) | def.val.= 16000
FMSTR_TSA_RW_VAR( wvar.rndis_config                    ,FMSTR_TSA_UINT8     ) // RNDIS interface configuration (0-Win home net, 1 - DHCP server) | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.rotation_speed                  ,FMSTR_TSA_FLOAT     ) // Nominal rotation speed (deg per sec) | def.val.= 25
FMSTR_TSA_RW_VAR( wvar.s_k_diff                        ,FMSTR_TSA_FLOAT     ) // PID Kd (Derivative) | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.s_k_filter                      ,FMSTR_TSA_FLOAT     ) // PID Kn (Filter coefficient)  | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.s_k_integr                      ,FMSTR_TSA_FLOAT     ) // PID Ki (Integral) | def.val.= 5
FMSTR_TSA_RW_VAR( wvar.s_k_prop                        ,FMSTR_TSA_FLOAT     ) // PID Kp (Proporcional) | def.val.= 1
FMSTR_TSA_RW_VAR( wvar.s_max_out                       ,FMSTR_TSA_FLOAT     ) // PID Maximum output value (A) | def.val.= 6
FMSTR_TSA_RW_VAR( wvar.s_max_slew_rate                 ,FMSTR_TSA_FLOAT     ) // PID Maximum slew rate | def.val.= 6
FMSTR_TSA_RW_VAR( wvar.s_min_out                       ,FMSTR_TSA_FLOAT     ) // PID Minimum output value (A) | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.screen_orientation              ,FMSTR_TSA_UINT32    ) // Screen orientation (0..3) | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.short_det_delay_param           ,FMSTR_TSA_UINT32    ) // Short detection delay parameter (0..1) | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.short_det_spike_filter          ,FMSTR_TSA_UINT32    ) // Spike filtering bandwidth for short detection (0-100ns..3-3us) | def.val.= 0
FMSTR_TSA_RW_VAR( wvar.short_gnd_det_level             ,FMSTR_TSA_UINT32    ) // Short to GND detector level for highside FETs (2- highest..15lowest) | def.val.= 12
FMSTR_TSA_RW_VAR( wvar.short_vs_det_level              ,FMSTR_TSA_UINT32    ) // Short to VS detector level for lowside FETs (1- highest..15-lowest) | def.val.= 12
FMSTR_TSA_RW_VAR( wvar.shunt_resistor                  ,FMSTR_TSA_FLOAT     ) // Shunt resistor (Ohm) | def.val.= 0.00400000018998981
FMSTR_TSA_RW_VAR( wvar.sntp_poll_interval              ,FMSTR_TSA_UINT32    ) // Poll interval (s) | def.val.= 120
FMSTR_TSA_RW_VAR( wvar.sound_loudness                  ,FMSTR_TSA_UINT32    ) // Sound loudness (0...10) | def.val.= 5
FMSTR_TSA_RW_VAR( wvar.usb_mode                        ,FMSTR_TSA_UINT32    ) // USB interface mode | def.val.= 4
FMSTR_TSA_RW_VAR( wvar.utc_offset                      ,FMSTR_TSA_FLOAT     ) // UTC offset (difference in hours +-) | def.val.= 3
FMSTR_TSA_RW_VAR( wvar.voice_language                  ,FMSTR_TSA_UINT32    ) // Language of voice | def.val.= 0
FMSTR_TSA_TABLE_END();


#endif
