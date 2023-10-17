#ifndef LOGGER_H
  #define LOGGER_H


  #define SEVERITY_RED 1

  #define    EVENT_LOG_SIZE         (32)  // Должно быть степенью 2

  #define    LOG_STR_MAX_SZ         (150)
  #define    EVNT_LOG_FNAME_SZ      (64  )

  #define    EVENT_LOG_DISPLAY_ROW  (22) // Количество строк лога выводимых на экран при автообновлении

// Структура хранения записи лога
typedef struct
{
    rtc_time_t       date_time;
    uint64_t         delta_time;
    char             msg[LOG_STR_MAX_SZ+1];
    char             func_name[EVNT_LOG_FNAME_SZ+1];
    unsigned int     line_num;
    unsigned int     severity;
} T_app_log_record;

typedef struct
{
    T_sys_timestump   log_start_time;  // Время старта лога

    volatile uint32_t event_log_head;
    volatile uint32_t event_log_tail;
    volatile uint32_t file_log_tail;

    unsigned int      log_miss_err;         // Счетчик ошибок ожидания доступа к логу
    unsigned int      log_overfl_err;       // Счетчик ошибок переполнения лога
    unsigned int      file_log_overfl_err;  // Счетчик ошибок переполнения файлового лога

    unsigned int      log_miss_f;           // Флаг ошибоки ожидания доступа к логу
    unsigned int      log_overfl_f;         // Флаг ошибоки переполнения лога
    unsigned int      file_log_overfl_f;    // Флаг ошибоки переполнения файлового лога


    uint32_t          log_inited; // Флаг готовности к записи в лог

} T_app_log_cbl;

  #define APPLOG(...)    LOGs(__FUNCTION__, __LINE__, SEVERITY_RED, ##__VA_ARGS__);

uint32_t  Init_logger(void);
void      Applog_write(char *str, const char *func_name, unsigned int line_num, unsigned int severity);

void      LOGs(const char *name, unsigned int line_num, unsigned int severity, const char *fmt_ptr, ...);
void      AppLogg_monitor_output(void);
uint32_t  Thread_file_logger_create(void);
void      Req_to_reset_log_file(void);
int32_t   AppLog_get_tail_record(T_app_log_record *rec);

#endif // APP_DEBUG_LOGGS_H



