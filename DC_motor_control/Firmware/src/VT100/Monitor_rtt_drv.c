#include "MC50.h"

static int Mn_rttdrv_init(void **pcbl, void *pdrv);
static int Mn_rttdrv_send_buf(const void *buf, unsigned int len);
static int Mn_rttdrv_wait_ch(unsigned char *b, int timeout);
static int Mn_rttdrv_printf(const char  *fmt_ptr, ...);
static int Mn_rttdrv_deinit(void **pcbl);


T_serial_io_driver mon_rtt_drv_driver =
{
  MN_DRIVER_MARK,
  MN_RTT0_DRIVER,
  Mn_rttdrv_init,
  Mn_rttdrv_send_buf,
  Mn_rttdrv_wait_ch,
  Mn_rttdrv_printf,
  Mn_rttdrv_deinit,
  0,
};


typedef struct
{
    uint32_t misc;

} T_rtt_drv_cbl;



/*-------------------------------------------------------------------------------------------------------------

  pdrv - указатель на структуру T_monitor_driver
  pcbl - указатель на указатель на структуру со специальными данными необходимыми драйверу
-------------------------------------------------------------------------------------------------------------*/
static int Mn_rttdrv_init(void **pcbl, void *pdrv)
{


  // Если драйвер еще не был инициализирован, то выделить память для управлющей структуры и ждать сигнала из интерфеса
  if (*pcbl == 0)
  {
    T_rtt_drv_cbl *p;

    // Выделяем память для управляющей структуры драйвера
    p = App_malloc_pending(sizeof(T_rtt_drv_cbl), 1);
    if (p != NULL)
    {
      return RES_ERROR;
    }

    // Инициализируем драйвер
    //
    //  ... ?
    //
    //


    *pcbl = p; //  Устанавливаем в управляющей структуре драйвера задачи указатель на управляющую структуру драйвера
  }
  return RES_OK;
}
/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
static int Mn_rttdrv_deinit(void **pcbl)
{
  App_free(*pcbl);
  *pcbl = 0;
  return RES_OK;
}


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
T_serial_io_driver* Mnsdrv_get_rtt_driver(void)
{
  return &mon_rtt_drv_driver;
}

/*-------------------------------------------------------------------------------------------------------------
  Вывод форматированной строки в коммуникационный канал порта
-------------------------------------------------------------------------------------------------------------*/
static int Mn_rttdrv_send_buf(const void *buf, unsigned int len)
{

  SEGGER_RTT_Write(RTT_MONITOR_CH,buf, len);

  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static int Mn_rttdrv_wait_ch(unsigned char *b, int timeout)
{
  int32_t  ticks = ms_to_ticks(timeout);
  int32_t  start_t = tx_time_get();
  int32_t  key;

  while ((tx_time_get()- start_t) < ticks)
  {
    key = SEGGER_RTT_GetKey();
    if (key >= 0)
    {
      *b = (unsigned char)key;
      return RES_OK;
    }
  }

  return RES_ERROR;

}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
#define   RTT_DRV_STR_SZ 512
static int Mn_rttdrv_printf(const char  *fmt_ptr, ...)
{
  uint32_t         n;
  uint32_t         ret;
  va_list          ap;
  char            *rtt_str;

  rtt_str = App_malloc_pending(RTT_DRV_STR_SZ, 10);
  if (rtt_str != NULL)
  {
    va_start(ap, fmt_ptr);
    n = vsnprintf(rtt_str, RTT_DRV_STR_SZ, (const char *)fmt_ptr, ap);
    va_end(ap);
    ret = SEGGER_RTT_Write(RTT_MONITOR_CH, rtt_str, n);
    App_free(rtt_str);
    if (ret != n)
    {
      return RES_ERROR;
    }
    return RES_OK;
  }

  return RES_ERROR;
}

