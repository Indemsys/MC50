#ifndef __FREEMASTER_LOOP
  #define __FREEMASTER_LOOP

  #define FM_RESET_DEVICE                      1
  #define FM_CHECK_LOG_PIPE                    7
  #define FM_SAVE_APP_PARAMS                   8

  #define FM_MOTOR_HARD_STOP                   100
  #define FM_MOTOR_FREEWHEELING                101
  #define FM_RESET_TMC6200_FAULTS              102

  #define FM_BLDC_MOVING_PULSE_HSTOP           110
  #define FM_BLDC_MOVING_PULSE_FREE            111

  #define FM_BLDC_SERVO_OPEN                   121
  #define FM_BLDC_SERVO_CLOSE                  122
  #define FM_BLDC_SERVO_CLOSING_PULSE          123
  #define FM_BLDC_SERVO_OPENING_PULSE          124

  #define FM_DC_ROTATE_CW                      130
  #define FM_DC_ROTATE_CCW                     131

  #define FM_DC_ROT_CW_NO_ACCEL                132
  #define FM_DC_ROT_CCW_NO_ACCEL               133


  #define FM_SINUSOIDAL_ROT_ON                 134
  #define FM_SINUSOIDAL_ROT_OFF                135

  #define FM_COMMUTATION_LAW_SEARCH            136

  #define FREEMASTER_ON_NET                    0
  #define FREEMASTER_ON_SERIAL                 1

extern   T_serial_io_driver *frm_serial_drv;


uint32_t Thread_FreeMaster_create(void);
void     FreeMaster_task_delete(void);
void     Task_FreeMaster(uint32_t initial_data);

#endif

