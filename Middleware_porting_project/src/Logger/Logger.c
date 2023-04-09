// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.05.13
// 16:37:49
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

T_app_log_record     app_log[EVENT_LOG_SIZE];

#define LOGGER_TASK_STACK_SIZE (1024*3)
static uint8_t logger_stacks[LOGGER_TASK_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.Logger_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static void Task_Logger(ULONG arg);

T_app_log_cbl                         log_cbl;
static bsp_lock_t                     p_rtt_log_lock;


#define TIME_DELAY_BEFORE_SAVE        100 // Время в мс перед тем как будут сохранены оставшиеся записи
#define LOG_RECS_BEFORE_SAVE_TO_FILE  10  // Количество запсией вызывающее немедленное сохранение


char                                  file_log_str[LOG_STR_MAX_SZ];
static TX_THREAD                      log_thread;

uint8_t                               request_to_reset_log;

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Req_to_reset_log_file(void)
{
  request_to_reset_log  = 1;
}


/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  Init_logger(void)
{
  uint32_t res = RES_OK;


  log_cbl.event_log_head = 0;
  log_cbl.event_log_tail = 0;
  Get_hw_timestump(&log_cbl.log_start_time);


  log_cbl.log_inited = 1;
  return res;
}


/*-----------------------------------------------------------------------------------------------------


  \param arg
-----------------------------------------------------------------------------------------------------*/
static void Task_Logger(ULONG arg)
{
  FX_FILE              f;
  uint32_t             res;
  int32_t              tail;
  int32_t              head;
  int32_t              n;
  uint32_t             t, t2;

  // Открыть файл для записи лога
  res = fx_file_create(&fat_fs_media,LOG_FILE_NAME);
  if ((res == FX_SUCCESS) ||  (res == FX_ALREADY_CREATED))
  {
    res = fx_file_open(&fat_fs_media,&f, LOG_FILE_NAME,  FX_OPEN_FOR_WRITE);
  }

  //Set_app_event(EVENT_LOGGER_TASK_READY);

  if (res != FX_SUCCESS)
  {
    tx_thread_terminate(tx_thread_identify());
    return;
  }

  t = tx_time_get();

  // Цикл записи в лог файл
  do
  {
    // Записываем если число записей превысило некторое количество или после истечения контрольного времени остались несохраненные записи


    // Вычисляем колическтво несохраненных записей в логе
    tail = log_cbl.file_log_tail;
    head = log_cbl.event_log_head;
    if (head >= tail)
    {
      n = head - tail;
    }
    else
    {
      n = EVENT_LOG_SIZE -(tail - head);
    }

    t2 = tx_time_get();
    if ((n > LOG_RECS_BEFORE_SAVE_TO_FILE) || ((n > 0) && ((t2 - t) > ms_to_ticks(TIME_DELAY_BEFORE_SAVE))))
    {

      do
      {
        // Сохраняем записи в файл
        if (log_cbl.file_log_overfl_f != 0)
        {
          log_cbl.file_log_overfl_f = 0;
          res = snprintf(file_log_str, LOG_STR_MAX_SZ, "... Overflow ...\r\n");
          fx_file_write(&f, file_log_str, res);
        }
        if (log_cbl.log_miss_f != 0)
        {
          log_cbl.log_miss_f = 0;
          res = snprintf(file_log_str, LOG_STR_MAX_SZ, "... Missed records ....\r\n");
          fx_file_write(&f, file_log_str, res);
        }

        if (rtc_init_res.RTC_valid)
        {
          rtc_time_t *pt =&app_log[tail].date_time;
          res = snprintf(file_log_str, LOG_STR_MAX_SZ, "%04d.%02d.%02d %02d:%02d:%02d |",pt->tm_year,pt->tm_mon,pt->tm_mday,pt->tm_hour, pt->tm_min, pt->tm_sec);
          if (res > 0) fx_file_write(&f, file_log_str, res);
        }

        uint64_t t64 = app_log[tail].delta_time;
        uint32_t t32;
        uint32_t time_msec      = t64 % 1000000ull;
        t32 = (uint32_t)(t64 / 1000000ull);
        uint32_t time_sec       = t32 % 60;
        uint32_t time_min       =(t32 / 60)% 60;
        uint32_t time_hour      =(t32 / (60 * 60))% 24;
        uint32_t time_day       = t32 / (60 * 60 * 24);

        res = snprintf(file_log_str, LOG_STR_MAX_SZ, "%03d d %02d h %02d m %02d s %06d us |",time_day, time_hour, time_min, time_sec, time_msec);
        if (res > 0) fx_file_write(&f, file_log_str, res);
        res = snprintf(file_log_str, LOG_STR_MAX_SZ, "%02d | %-36s | %5d |", app_log[tail].severity, app_log[tail].func_name, app_log[tail].line_num);
        if (res > 0) fx_file_write(&f, file_log_str, res);
        res = snprintf(file_log_str, LOG_STR_MAX_SZ, " %s\r\n", app_log[tail].msg);
        if (res > 0) fx_file_write(&f, file_log_str, res);

        // Проходим по всем не сохраненным записям
        __disable_interrupt();
        tail++;
        if (tail >= EVENT_LOG_SIZE) tail = 0;
        log_cbl.file_log_tail = tail;
        head = log_cbl.event_log_head;
        __enable_interrupt();
        if (head == tail) break;

      } while (1);


      fx_media_flush(&fat_fs_media); //  Очищаем кэш записи
      t = t2; // Запоминаем время последней записи
    }
    Wait_ms(10);


  } while (1);

}

/*------------------------------------------------------------------------------
  Запись сообщения в таблицу лога и в другие места назначения


 \param str         : сообщение
 \param func_name   : имя функции
 \param line_num    : номер строки
 \param severity    : важность сообщения
 ------------------------------------------------------------------------------*/
void Applog_write(char *str, const char *func_name, unsigned int line_num, unsigned int severity)
{
  int         head;
  int         tail;
  T_sys_timestump   ntime;
  rtc_time_t        date_time;


  if (log_cbl.log_inited == 1)
  {
    Get_hw_timestump(&ntime);
    RTC_get_system_DateTime(&date_time);
    date_time.tm_mon++;
    date_time.tm_year += 1900;

    // Вызов данной процедуры может производится из процедур обслуживания прерываний,
    // поэтому мьютексы и другие сервисы сихронизации здесь использовать нельзя
    __disable_interrupt();

    head = log_cbl.event_log_head;
        // Определяем время в микросекундах от старта лога
    app_log[head].date_time = date_time;
    app_log[head].delta_time = Hw_timestump_diff64_us(&log_cbl.log_start_time,&ntime);
    strncpy(app_log[head].msg, str, LOG_STR_MAX_SZ - 1);
    strncpy(app_log[head].func_name, func_name, EVNT_LOG_FNAME_SZ - 1);
    app_log[head].line_num = line_num;
    app_log[head].severity = severity;
        // Сдвигаем указатель головы лога
    head++;
    if (head >= EVENT_LOG_SIZE) head = 0;
    log_cbl.event_log_head = head;

    tail = log_cbl.event_log_tail;
        // Если голова достигла хвоста, то сдвигает указатель хвоста
    if (head == tail)
    {
      tail++;
      if (tail >= EVENT_LOG_SIZE) tail = 0;
      log_cbl.event_log_tail = tail;
      log_cbl.log_overfl_f = 1;
      log_cbl.log_overfl_err++;
    }
        // Если голова достигла хвоста записи в файл, то сдвигает указатель хвоста записи в файл
    tail = log_cbl.file_log_tail;
    if (head == tail)
    {
      tail++;
      if (tail >= EVENT_LOG_SIZE) tail = 0;
      log_cbl.file_log_tail = tail;
      log_cbl.file_log_overfl_f = 1;
      log_cbl.file_log_overfl_err++;
    }
    __enable_interrupt();
  }
}


/*-----------------------------------------------------------------------------------------------------

  Вызов данной процедуры может производится из процедур обслуживания прерываний,
  поэтому мьютексы и другие сервисы сихронизации здесь использовать нельзя

  \param err
  \param module
  \param line
-----------------------------------------------------------------------------------------------------*/
void ssp_error_log(ssp_err_t err, const char *module, int32_t line)
{
  char             str[32+1];
  if (err == SSP_ERR_TRANSFER_BUSY)
  {
    //int32_t8(1, 1);
  }
  if (err == SSP_ERR_IRQ_BSP_DISABLED)
  {
    snprintf(str, 32, "Periphery IRQ dissabled");
  }
  else
  {
    snprintf(str, 32, "Error = %d", err);
  }
  Applog_write(str, module, line , 0);
}

/*-----------------------------------------------------------------------------------------------------


  \param name
  \param line_num
  \param severity
  \param fmt_ptr
-----------------------------------------------------------------------------------------------------*/
void RTT_LOGs(const char *name, unsigned int line_num, unsigned int severity, const char *fmt_ptr, ...)
{
  unsigned int     n;
  char             log_str[LOG_STR_MAX_SZ+1];

  va_list          ap;

  while (R_BSP_SoftwareLock(&p_rtt_log_lock) != SSP_SUCCESS);


  va_start(ap, fmt_ptr);

  n = snprintf(log_str, LOG_STR_MAX_SZ, "%s (%d) :",name, line_num);
  SEGGER_RTT_Write(RTT_LOG_CH, log_str, n);

  n = vsnprintf(log_str, LOG_STR_MAX_SZ, (const char *)fmt_ptr, ap);
  va_end(ap);
  SEGGER_RTT_Write(RTT_LOG_CH, log_str, n);

  n = snprintf(log_str, LOG_STR_MAX_SZ, "\r\n");
  SEGGER_RTT_Write(RTT_LOG_CH, log_str, n);

  R_BSP_SoftwareUnlock(&p_rtt_log_lock);
}

/*-----------------------------------------------------------------------------------------------------


  \param name
  \param line_num
  \param severity
  \param fmt_ptr
-----------------------------------------------------------------------------------------------------*/
void LOGs(const char *name, unsigned int line_num, unsigned int severity, const char *fmt_ptr, ...)
{
  char             log_str[LOG_STR_MAX_SZ+1];

  va_list          ap;

  va_start(ap, fmt_ptr);

  vsnprintf(log_str, LOG_STR_MAX_SZ, (const char *)fmt_ptr, ap);
  Applog_write(log_str , name, line_num, severity);


  va_end(ap);
}

/*------------------------------------------------------------------------------
  Получить структуру записи лога от хвоста
  Возвращает 0 если записей в логе нет

 \param rec

 \return int
 ------------------------------------------------------------------------------*/
int32_t AppLog_get_tail_record(T_app_log_record *rec)
{
  int32_t res = 0;
  uint32_t tail;

  // Вызов логгера могут производится из процедур обслуживания прерываний,
  // поэтому мьютексы и другие сервисы сихронизации здесь использовать нельзя

  __disable_interrupt();

  tail = log_cbl.event_log_tail;
  if (log_cbl.event_log_head != tail)
  {
    memcpy(rec,&app_log[log_cbl.event_log_tail], sizeof(T_app_log_record));
    log_cbl.event_log_tail++;
    if (log_cbl.event_log_tail >= EVENT_LOG_SIZE)
    {log_cbl.event_log_tail = 0; }
    res = 1;
  }

  __enable_interrupt();
  return res;
}

/*------------------------------------------------------------------------------



 \param pvt100_cb
 ------------------------------------------------------------------------------*/
void AppLogg_monitor_output(void)
{
  uint32_t   i;
  uint32_t   n;
  uint32_t   reqn;
  uint32_t   outn;
  uint8_t    b;
  uint32_t   head;
  uint32_t   tail;

  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF("Events log. <R> - exit, <D> - print all log\n\r");
  MPRINTF("............................................\n\r");
  MPRINTF("Log overflows=%d, File log overflows=%d, Log miss count=%d\r\n",log_cbl.log_overfl_err, log_cbl.file_log_overfl_err,log_cbl.log_miss_err);
  MPRINTF("********************************************\n\r");

  do
  {
    VT100_set_cursor_pos(3, 0);


    // Вывод последних 22-х строк лога


    // Определяем количестово строк в логе
    head = log_cbl.event_log_head;
    tail = log_cbl.event_log_tail;
    if (head >= tail)
    {
      reqn = head - tail;
    }
    else
    {
      reqn = EVENT_LOG_SIZE -(tail - head);
    }

    // Определяем количество выводимых на экран строк лога
    if (reqn < EVENT_LOG_DISPLAY_ROW)
    {
      outn = reqn;
    }
    else
    {
      outn = EVENT_LOG_DISPLAY_ROW;
    }

    // Вычисляем индекс строки в логе с которой начинается вывод
    if (head >= tail)
    {
      n = head - outn;
    }
    else
    {
      if (outn > head)
      {
        n = EVENT_LOG_SIZE -(outn - head);
      }
      else
      {
        n = head - outn;
      }
    }


    for (i = 0; i < EVENT_LOG_DISPLAY_ROW; i++)
    {
      if (i < outn)
      {
        uint64_t t64 = app_log[n].delta_time;
        uint32_t time_usec = t64 % 1000000ull;
        uint32_t time_sec  = (uint32_t)(t64 / 1000000ull);

        if (app_log[n].line_num != 0)
        {
          MPRINTF(VT100_CLL_FM_CRSR"%05d.%06d %s (%s %d)\n\r",
                  time_sec, time_usec,
                  app_log[n].msg,
                  app_log[n].func_name,
                  app_log[n].line_num);
        }
        else
        {
          MPRINTF(VT100_CLL_FM_CRSR"%05d.%06d %s\n\r",
                  time_sec, time_usec,
                  app_log[n].msg);
        }
        n++;
        if (n >= EVENT_LOG_SIZE) n = 0;
      }
      else
      {
        MPRINTF(VT100_CLL_FM_CRSR"\n\r");
      }
    }



    if (WAIT_CHAR(&b, 200) == RES_OK)
    {
      switch (b)
      {
      case 'D':
      case 'd':
        MPRINTF(VT100_CLEAR_AND_HOME);
        // Вывод всего лога
        head = log_cbl.event_log_head;
        tail = log_cbl.event_log_tail;
        //
        if (head >= tail)
        {
          reqn = head - tail;
        }
        else
        {
          reqn = EVENT_LOG_SIZE -(tail - head);
        }

        MPRINTF("\n\r");
        n = tail;
        for (i = 0; i < reqn; i++)
        {
          uint64_t t64 = app_log[n].delta_time;
          uint32_t time_usec = t64 % 1000000ull;
          uint32_t time_sec  = (uint32_t)(t64 / 1000000ull);

          if (app_log[n].line_num != 0)
          {
            MPRINTF(VT100_CLL_FM_CRSR"%05d.%06d %s (%s %d)\n\r",
                    time_sec, time_usec,
                    app_log[n].msg,
                    app_log[n].func_name,
                    app_log[n].line_num);
          }
          else
          {
            MPRINTF(VT100_CLL_FM_CRSR"%05d.%06d %s\n\r",
                    time_sec, time_usec,
                    app_log[n].msg);
          }
          n++;
          if (n >= EVENT_LOG_SIZE) n = 0;
        }
        MPRINTF("\n\r");
        WAIT_CHAR(&b, 200000);

        break;
      case 'R':
      case 'r':
      case VT100_ESC:
        return;
      case 'C':
      case 'c':
        log_cbl.event_log_head = 0;
        log_cbl.event_log_tail = 0;
        break;
      }

    }
  }while (1);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Create_file_logger_task(void)
{
  uint32_t res;
  res =  tx_thread_create(
                          &log_thread,
                          (CHAR *)"Logger",
                          Task_Logger,
                          0,
                          logger_stacks,
                          LOGGER_TASK_STACK_SIZE,
                          LOGGER_TASK_PRIO,
                          LOGGER_TASK_PRIO,
                          1,
                          TX_AUTO_START
                         );
  APPLOG("Logger task creation result: %d", res);
  return res;
}


