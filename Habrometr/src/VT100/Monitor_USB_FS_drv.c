#include "MC50.h"

static int Mn_usbfs_drv_init(void **pcbl, void *pdrv);
static int Mn_usbfs_drv_send_buf(const void *buf, unsigned int len);
static int Mn_usbfs_drv_wait_ch(unsigned char *b, int timeout);
static int Mn_usbfs_drv_printf(const char  *fmt_ptr, ...);
static int Mn_usbfs_drv_deinit(void **pcbl);


T_serial_io_driver mon_usbfs_vcom0_drv_driver =
{
  MN_DRIVER_MARK,
  MN_USBFS_VCOM0_DRIVER,
  Mn_usbfs_drv_init,
  Mn_usbfs_drv_send_buf,
  Mn_usbfs_drv_wait_ch,
  Mn_usbfs_drv_printf,
  Mn_usbfs_drv_deinit,
  0,
};

T_serial_io_driver mon_usbfs_vcom1_drv_driver =
{
  MN_DRIVER_MARK,
  MN_USBFS_VCOM1_DRIVER,
  Mn_usbfs_drv_init,
  Mn_usbfs_drv_send_buf,
  Mn_usbfs_drv_wait_ch,
  Mn_usbfs_drv_printf,
  Mn_usbfs_drv_deinit,
  0,
};

#define   MB_USBFS_READ_REQUEST BIT(0)
#define   MB_USBFS_READ_DONE    BIT(1)


T_usbfs_drv_cbl usbd1_cbl;
T_usbfs_drv_cbl usbd2_cbl;


static void Mb_usbfs_rcv_task(ULONG ptr);


/*-----------------------------------------------------------------------------------------------------


  \param cdc_instance
-----------------------------------------------------------------------------------------------------*/
VOID ux_cdc_device0_instance_activate(VOID *cdc_instance)
{
  usbd1_cbl.cdc =  (UX_SLAVE_CLASS_CDC_ACM *) cdc_instance;
  usbd1_cbl.active = 1;
}

/*-----------------------------------------------------------------------------------------------------


  \param cdc_instance
-----------------------------------------------------------------------------------------------------*/
VOID ux_cdc_device0_instance_deactivate(VOID *cdc_instance)
{
  usbd1_cbl.active = 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param cdc_instance
-----------------------------------------------------------------------------------------------------*/
VOID ux_cdc_device1_instance_activate(VOID *cdc_instance)
{
  usbd2_cbl.cdc =  (UX_SLAVE_CLASS_CDC_ACM *) cdc_instance;
  usbd2_cbl.active = 1;
}

/*-----------------------------------------------------------------------------------------------------


  \param cdc_instance
-----------------------------------------------------------------------------------------------------*/
VOID ux_cdc_device1_instance_deactivate(VOID *cdc_instance)
{
  usbd2_cbl.active = 0;
}


/*-----------------------------------------------------------------------------------------------------
  Создаем объект событий и задачу приема

  \param p

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Create_recv_task(T_usbfs_drv_cbl *p, char *task_name, char *evg_name)
{
  uint32_t  prio;

  if (tx_event_flags_create(&(p->evt),evg_name) != TX_SUCCESS)
  {

    return RES_ERROR;
  }

  // Получаем приоритет текущей задачи
  tx_thread_info_get(tx_thread_identify(), TX_NULL, TX_NULL, TX_NULL,&prio, TX_NULL, TX_NULL, TX_NULL, TX_NULL);

  if (tx_thread_create(
      &(p->recv_thread),
      (CHAR *)task_name,
      Mb_usbfs_rcv_task,
      (ULONG)p,  // Передаем в задачу указатель на служебную стуктуру
      p->recv_thread_stack,
      USB_DRV_RECV_THREAD_STACK_SZ,
      prio,
      prio,
      1,
      TX_AUTO_START
      ) != TX_SUCCESS)
  {
    tx_event_flags_delete(&(p->evt));
    return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param p
-----------------------------------------------------------------------------------------------------*/
void Delete_recv_task(T_usbfs_drv_cbl *p)
{
  if (tx_thread_terminate(&(p->recv_thread)) == TX_SUCCESS)
  {
    tx_thread_delete(&(p->recv_thread));
  }
  tx_event_flags_delete(&(p->evt));
}


/*-------------------------------------------------------------------------------------------------------------

  pcbl - указатель на указатель на структуру со специальными данными необходимыми драйверу
  pdrv - указатель на структуру T_monitor_driver
-------------------------------------------------------------------------------------------------------------*/
static int Mn_usbfs_drv_init(void **pcbl, void *pdrv)
{
   T_usbfs_drv_cbl * p;
  // Если драйвер еще не был инициализирован, то выделить память для управлющей структуры и ждать сигнала из интерфеса
  if (*pcbl == 0)
  {
    uint32_t t;

    t =((T_serial_io_driver *)pdrv)->driver_type;
    switch (t)
    {
    case MN_USBFS_VCOM0_DRIVER:
      p = &usbd1_cbl;  //  Устанавливаем в управляющей структуре драйвера задачи указатель на управляющую структуру драйвера
      *pcbl = p;
      sprintf(p->recv_task_name, "VCOM_RXT_%d",(uint32_t)t);
      sprintf(p->evt_grp_name,   "VCOM_RX_EVG_%d",(uint32_t)t);
      break;
    case MN_USBFS_VCOM1_DRIVER:
      p = &usbd2_cbl;  //  Устанавливаем в управляющей структуре драйвера задачи указатель на управляющую структуру драйвера
      *pcbl = p;
      sprintf(p->recv_task_name, "VCOM_RXT_%d",(uint32_t)t);
      sprintf(p->evt_grp_name,   "VCOM_RX_EVG_%d",(uint32_t)t);
      break;
    default:
      return RES_ERROR;
    }
    // Создаем задачу занимающуюся приемом данных

    if (Create_recv_task(p, p->recv_task_name, p->evt_grp_name) != RES_OK)
    {
      return RES_ERROR;
    }

  }
  return RES_OK;
}
/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
static int Mn_usbfs_drv_deinit(void **pcbl)
{
  T_usbfs_drv_cbl *p = (T_usbfs_drv_cbl *)(*pcbl);
  Delete_recv_task(p);

  *pcbl = 0;
  return RES_OK;
}


/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
T_serial_io_driver* Mnsdrv_get_usbfs_vcom0_driver(void)
{
  return &mon_usbfs_vcom0_drv_driver;
}

/*-------------------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------------------*/
T_serial_io_driver* Mnsdrv_get_usbfs_vcom1_driver(void)
{
  return &mon_usbfs_vcom1_drv_driver;
}

/*-------------------------------------------------------------------------------------------------------------
  Вывод форматированной строки в коммуникационный канал порта
-------------------------------------------------------------------------------------------------------------*/
static int Mn_usbfs_drv_send_buf(const void *buf, unsigned int len)
{

  UINT                     res;
  ULONG                    actual_length;
  T_serial_io_driver *mdrv = (T_serial_io_driver *)(tx_thread_identify()->driver);
  T_usbfs_drv_cbl  *p    = (T_usbfs_drv_cbl *)(mdrv->pdrvcbl);

  if (p->active)
  {
    res = ux_device_class_cdc_acm_write(p->cdc, (UCHAR*)buf, len ,&actual_length);
    if ((res != UX_SUCCESS) || (actual_length != len))
    {
      return RES_ERROR;
    }
  }
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static int Mn_usbfs_drv_wait_ch(unsigned char *b, int timeout)
{
  ULONG        actual_flags = 0;
  int32_t      n;
  int32_t      h;

  T_serial_io_driver *mdrv = (T_serial_io_driver *)(tx_thread_identify()->driver);
  T_usbfs_drv_cbl  *p    = (T_usbfs_drv_cbl *)(mdrv->pdrvcbl);

  if (p->active)
  {
    // Если индексы буферов равны то это значит отсутствие принятых пакетов
    h = p->head_n;
    if (p->tail_n == h)
    {
      if (tx_event_flags_get(&(p->evt), MB_USBFS_READ_DONE, TX_OR_CLEAR,&actual_flags, ms_to_ticks(timeout)) != TX_SUCCESS)
      {
        return RES_ERROR;
      }
      // Еще раз проверяем наличие данных поскольку флаг мог остаться от предыдущего чтения, когда данные били приняты без проверки флага и соответственно без его сброса
      h = p->head_n;
      if (p->tail_n == h)
      {
        return RES_ERROR;
      }
    }

    n = p->tail_n;                      // Получаем индекс хвостового буфера
    *b = p->rd_pack[n].buff[p->rd_pos]; // Читаем байт данных из хвостового буфера
    p->rd_pos++;                        // Смещаем указатель на следующий байт данных

    // Если позиция достигла конца данных в текущем буфере, то буфер освобождается для приема
    if (p->rd_pos >= p->rd_pack[n].len)
    {
      p->rd_pos = 0;
      // Смещаем указатель хвоста очереди приемных буфферов
      // Появляется место для движения головы очереди приемных буфферов

      n++;
      if (n >= IN_BUF_QUANTITY) n = 0;
      p->tail_n = n;

      // Если очередь пакетов была заполнена, то сообщить задаче о продолжении приема
      if (p->no_space == 1)
      {
        p->no_space = 0;
        if (tx_event_flags_set(&(p->evt), MB_USBFS_READ_REQUEST, TX_OR) != TX_SUCCESS)
        {
          Wait_ms(timeout); // Задержка после ошибки нужна для того чтобы задача не захватила все ресурсы в случает постоянного появления ошибки
          return RES_ERROR;
        }
      }
    }
    return RES_OK;
  }
  else
  {
    Wait_ms(timeout);
  }
  return RES_ERROR;
}


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static int Mn_usbfs_drv_printf(const char  *fmt_ptr, ...)
{
  UINT              res;
  ULONG             actual_length;
  uint32_t          n;
  T_serial_io_driver *mdrv = (T_serial_io_driver *)(tx_thread_identify()->driver);
  T_usbfs_drv_cbl  *p    = (T_usbfs_drv_cbl *)(mdrv->pdrvcbl);
  char             *s = p->str;
  va_list           ap;


  va_start(ap, fmt_ptr);
  n = vsnprintf(s, USB_DRV_STR_SZ, (const char *)fmt_ptr, ap);
  va_end(ap);
  if (p->active)
  {
    res = ux_device_class_cdc_acm_write(p->cdc, (UCHAR*)s, n ,&actual_length);
    if ((res != UX_SUCCESS) || (actual_length != n))
    {
      return RES_ERROR;
    }
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param ptr
-----------------------------------------------------------------------------------------------------*/
static void Mb_usbfs_rcv_task(ULONG ptr)
{
  UINT              res;
  ULONG             actual_length;
  ULONG             actual_flags;
  T_usbfs_drv_cbl * p = (T_usbfs_drv_cbl *)ptr;
  int32_t           n;
  int32_t           tail;

  do
  {
    if (p->active)
    {
      n = p->head_n;
      res = ux_device_class_cdc_acm_read(p->cdc, p->rd_pack[n].buff,USBDRV_BUFFER_MAX_LENGTH,  &actual_length);
      p->rd_pack[n].len = actual_length;
      if (res == UX_SUCCESS)
      {
        tail = p->tail_n;
        n++;
        if (n >= IN_BUF_QUANTITY) n = 0;
        p->head_n = n;

        // Выставляем флаг выполненного чтения
        if (tx_event_flags_set(&(p->evt), MB_USBFS_READ_DONE, TX_OR) != TX_SUCCESS)
        {
          tx_thread_sleep(2); // Задержка после ошибки
        }

        // Если все буферы на прием заполнены, то значит системе не требуются данные
        if (tail == n)
        {
          // Перестаем принимать данные из USB и ждем когда система обработает уже принятые данные и подаст сигнал к началу приема по USB
          p->no_space = 1;
          if (tx_event_flags_get(&(p->evt), MB_USBFS_READ_REQUEST, TX_OR_CLEAR,&actual_flags, TX_WAIT_FOREVER) != TX_SUCCESS)
          {
            tx_thread_sleep(2); // Задержка после ошибки
          }
        }
      }
      else
      {
        tx_thread_sleep(2); // Задержка после ошибки
      }
    }
    else
    {
      tx_thread_sleep(2); // Задержки после ошибки нужна для того чтобы задача не захватила все ресурсы в случает постоянного появления ошибки
    }

  } while (1);
}

