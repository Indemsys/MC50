#ifndef __FREEMASTER_LOOP
  #define __FREEMASTER_LOOP

  #define FMCMD_RESET_DEVICE                   0x01
  #define FMCMD_CHECK_LOG_PIPE                 0x07
  #define FMCMD_SAVE_WVARS                     0x08


uint32_t Thread_FreeMaster_create(void);
void     FreeMaster_task_delete(void);
void     Task_FreeMaster(uint32_t initial_data);

#endif

