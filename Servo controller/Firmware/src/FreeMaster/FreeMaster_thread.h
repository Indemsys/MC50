#ifndef __FREEMASTER_LOOP
  #define __FREEMASTER_LOOP

  #define FMCMD_RESET_DEVICE                   1
  #define FMCMD_CHECK_LOG_PIPE                 7
  #define FMCMD_SAVE_APP_PARAMS                8
  #define FMCMD_START_MOTOR                    16
  #define FMCMD_STOP_MOTOR                     17
  #define FMCMD_MOVING_PULE                    18
  #define FMCMD_OPEN                      19
  #define FMCMD_CLOSE                     20
  #define FMCMD_RESET_DRIVER_FAULTS            21
  #define FMCMD_CLOSE_PULSE                    22
  #define FMCMD_OPEN_PULSE                     23

  #define FREEMASTER_ON_NET                    0
  #define FREEMASTER_ON_SERIAL                 1

extern   T_serial_io_driver *frm_serial_drv;


uint32_t Thread_FreeMaster_create(void);
void     FreeMaster_task_delete(void);
void     Task_FreeMaster(uint32_t initial_data);

#endif

