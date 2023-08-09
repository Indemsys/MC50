#include "MC50.h"
#include "freemaster_cfg.h"
#include "freemaster.h"
#include "freemaster_tsa.h"


#define FM_PIPE_RX_BUF_SIZE 64
#define FM_PIPE_TX_BUF_SIZE 1024
#define FM_PIPE_MAX_STR_LEN 512

#define FM_PIPE_PORT_NUM    0
#define FM_PIPE_CALLBACK    0
#define FM_PIPE_TYPE        FMSTR_PIPE_TYPE_ANSI_TERMINAL
FMSTR_ADDR                  pipeRxBuff;
FMSTR_PIPE_SIZE             pipeRxSize;
FMSTR_ADDR                  pipeTxBuff;
FMSTR_PIPE_SIZE             pipeTxSize;

FMSTR_HPIPE                 fm_pipe = NULL;
T_app_log_record            *p_log_rec;
char                        *log_str;
uint8_t                     f_unsent_record;
uint32_t                    g_freemaster_interface_type;

#define FREEMASTER_TASK_STACK_SIZE (1024*2)
static uint8_t freemaster_stack[FREEMASTER_TASK_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.freemaster_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static TX_THREAD       freemaster_thread;



static void Thread_FreeMaster(ULONG initial_data);

/*-----------------------------------------------------------------------------------------------------
  Вызывается из контекста удаляемой задачи после того как она выключена

  \param thread_ptr
  \param condition
-----------------------------------------------------------------------------------------------------*/
static void FreeMaster_entry_exit_notify(TX_THREAD *thread_ptr, UINT condition)
{
  if (condition == TX_THREAD_ENTRY)
  {

  }
  else if (condition == TX_THREAD_EXIT)
  {
    FreeMaster_task_delete();
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void FreeMaster_task_delete(void)
{
  tx_thread_terminate(&freemaster_thread);
  tx_thread_delete(&freemaster_thread);
  App_free(pipeRxBuff);
  App_free(pipeTxBuff);
  App_free(p_log_rec);
  App_free(log_str);

}

/*-----------------------------------------------------------------------------------------------------
  Обработка пользовательских комманд поступающих движку FreeMaster

 \param app_command

 \return uint8_t
-----------------------------------------------------------------------------------------------------*/
static uint8_t Freemaster_Command_Manager(uint16_t app_command)
{
  uint8_t  res;
  res = 0;
  //uint32_t len;
  //uint8_t  *dbuf;

  // Получаем указатель на буфер с данными команды
  //dbuf = FMSTR_GetAppCmdData(&len);

  switch (app_command)
  {
  case FMCMD_CHECK_LOG_PIPE:
    FMSTR_PipePuts(fm_pipe, "Log pipe checked.\r\n");
    break;

  case FMCMD_SAVE_APP_PARAMS:
    Request_save_app_settings();
    break;

  case FMCMD_STOP_MOTOR:
    MC_set_stop_motor();
    break;

  case FMCMD_START_MOTOR:
    MC_set_start_motor();
    break;

  case FMCMD_MOVING_PULE:
    MC_set_moving_pulse();
    break;

  case FMCMD_OPEN:
    MC_set_open();
    break;

  case FMCMD_CLOSE:
    MC_set_close();
    break;

  case FMCMD_CLOSE_PULSE:
    MC_set_closing_pulse();
    break;

  case FMCMD_OPEN_PULSE:
    MC_set_opening_pulse();
    break;

  case FMCMD_RESET_DRIVER_FAULTS:
    MC_set_reset_tmc6200();
    break;

  case FMCMD_RESET_DEVICE:
    Reset_SoC();
    break;
  default:
    res = 0;
    break;
  }


  return res;
}


/*-----------------------------------------------------------------------------------------------------


  \param str
  \param max_str_len
  \param p_log_rec
-----------------------------------------------------------------------------------------------------*/
void Format_log_string(char *str, uint32_t max_str_len, T_app_log_record  *p_log_rec)
{

  uint64_t t64 = p_log_rec->delta_time;
  uint32_t t32;
  uint32_t time_msec      = t64 % 1000000ull;
  t32 = (uint32_t)(t64 / 1000000ull);
  uint32_t time_sec       = t32 % 60;
  uint32_t time_min       =(t32 / 60)% 60;
  uint32_t time_hour      =(t32 / (60 * 60))% 24;
  uint32_t time_day       = t32 / (60 * 60 * 24);

  snprintf(str, max_str_len, "%03d d %02d h %02d m %02d s %06d us |",time_day, time_hour, time_min, time_sec, time_msec);
  uint32_t  len = strlen(str);

  if (p_log_rec->line_num != 0)
  {
    snprintf(&str[len], max_str_len - len, " %s (%s %d)\n\r",
             p_log_rec->msg,
             p_log_rec->func_name,
             p_log_rec->line_num);
  }
  else
  {
    snprintf(&str[len], max_str_len - len, " %s\n\r",
             p_log_rec->msg);
  }
}

/*-----------------------------------------------------------------------------------------------------
  Пересылаем данные из лога приложения в канал FreeMaster

  \param void
-----------------------------------------------------------------------------------------------------*/
void Freemaster_send_log_to_pipe(void)
{
  if (f_unsent_record != 0)
  {
    if (FMSTR_PipePuts(fm_pipe, log_str) != FMSTR_TRUE) return;
    f_unsent_record = 0;
  }

  while (AppLog_get_tail_record(p_log_rec) == RES_OK)
  {
    Format_log_string(log_str, FM_PIPE_MAX_STR_LEN, p_log_rec);
    if (FMSTR_PipePuts(fm_pipe, log_str) != FMSTR_TRUE)
    {
      f_unsent_record = 1;
      return;
    }
  }
}


/*-------------------------------------------------------------------------------------------------------------
  Цикл движка FreeMaster
-------------------------------------------------------------------------------------------------------------*/
static void Thread_FreeMaster(ULONG initial_data)
{
  uint16_t app_command;
  uint8_t  res;

  if (initial_data == FREEMASTER_ON_NET)
  {
    // Ожидаем инициализации стека сетевого стека BSD
    while (g_BSD_initialised == 0)
    {
      Wait_ms(10);
    }

    if (!FMSTR_Init((void *)&FMSTR_NET))
    {
      return;
    }
  }
  else if (initial_data == FREEMASTER_ON_SERIAL)
  {
    frm_serial_drv = Mnsdrv_get_usbfs_vcom1_driver();
    if (frm_serial_drv->_init(&frm_serial_drv->pdrvcbl, frm_serial_drv) != RES_OK)
    {
      return;
    }
    tx_thread_identify()->driver =  (ULONG)(frm_serial_drv);
    if (!FMSTR_Init((void *)&FMSTR_SERIAL))
    {
      return;
    }
  }
  else return;


  pipeRxSize = FM_PIPE_RX_BUF_SIZE;
  pipeRxBuff = App_malloc(pipeRxSize);
  pipeTxSize = FM_PIPE_TX_BUF_SIZE;
  pipeTxBuff = App_malloc(pipeTxSize);
  p_log_rec  = App_malloc(sizeof(T_app_log_record));
  log_str    = App_malloc(FM_PIPE_MAX_STR_LEN);
  if ((pipeRxBuff != NULL) && (pipeTxBuff != NULL) && (p_log_rec != NULL) && (log_str != NULL))
  {
    fm_pipe = FMSTR_PipeOpen(FM_PIPE_PORT_NUM, FM_PIPE_CALLBACK,  pipeRxBuff, pipeRxSize, pipeTxBuff, pipeTxSize, FM_PIPE_TYPE, "SysLog");
  }

  while (1)
  {
    app_command = FMSTR_GetAppCmd();

    if (app_command != FMSTR_APPCMDRESULT_NOCMD)
    {
      res = Freemaster_Command_Manager(app_command);
      FMSTR_AppCmdAck(res);
    }
    FMSTR_Poll();
    if (fm_pipe != NULL)
    {
      Freemaster_send_log_to_pipe();
    }
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Determine_FreeMaster_interface_type(void)
{

  if (wvar.usb_mode == USB_MODE_VCOM_AND_FREEMASTER_PORT)
  {
    g_freemaster_interface_type = FREEMASTER_ON_SERIAL;
  }
  else if (wvar.usb_mode == USB_MODE_RNDIS)
  {
    g_freemaster_interface_type = FREEMASTER_ON_NET;
  }
  else if (wvar.usb_mode == USB_MODE_HOST_ECM)
  {
    g_freemaster_interface_type = FREEMASTER_ON_NET;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Thread_FreeMaster_create(void)
{
  UINT                res;

  Determine_FreeMaster_interface_type();

  res = tx_thread_create(
                         &freemaster_thread,
                         (CHAR *)"FreeMaster",
                         Thread_FreeMaster,
                         (ULONG)g_freemaster_interface_type,
                         freemaster_stack,
                         FREEMASTER_TASK_STACK_SIZE,
                         FREEMASTER_TASK_PRIO,
                         FREEMASTER_TASK_PRIO,
                         1,
                         TX_AUTO_START
                        );

  APPLOG("FreeMaster task creation result: %d", res);
  if (res != TX_SUCCESS) return RES_ERROR;

  tx_thread_entry_exit_notify(&freemaster_thread, FreeMaster_entry_exit_notify);

  return RES_OK;
}

