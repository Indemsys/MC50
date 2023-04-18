// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-02-02
// 14:56:58
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Get_system_timings(uint32_t *p_sys_div, uint32_t *p_sys_freq, uint32_t *p_ticks_freq)
{
  uint32_t sys_freq;
  uint32_t ticks_freq;
  uint32_t sys_div = SysTick->LOAD+1;  // Получаем делитель системной частоты для получения системного тика

  g_cgc_on_cgc.systemClockFreqGet(CGC_SYSTEM_CLOCKS_ICLK,&sys_freq); // Получаем системную частоту
  ticks_freq = sys_freq / sys_div;

  if (p_sys_div!=0) *p_sys_div = sys_div;
  if (p_sys_freq!=0) *p_sys_freq = sys_freq;
  if (p_ticks_freq!=0) *p_ticks_freq = ticks_freq;

}

/*-----------------------------------------------------------------------------------------------------


  \param v
-----------------------------------------------------------------------------------------------------*/
uint32_t  Get_system_ticks(uint32_t *v)
{
  uint32_t t;
  t = tx_time_get();
  if (v != 0) *v = t;
  return t;
}

/*-----------------------------------------------------------------------------------------------------


  \param start_time_val
  \param stop_time_val

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Time_diff_seconds(uint32_t start_time_val, uint32_t stop_time_val)
{
  return (stop_time_val - start_time_val) / TX_TIMER_TICKS_PER_SECOND;
}

/*-----------------------------------------------------------------------------------------------------


  \param start_time_val
  \param stop_time_val

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Time_diff_microseconds(uint32_t start_time_val, uint32_t stop_time_val)
{
  return ((stop_time_val - start_time_val)*1000000ull)/TX_TIMER_TICKS_PER_SECOND;
}

/*-----------------------------------------------------------------------------------------------------
  Разница во времени в миллисекундах
  Аргументы выражаются в количестве тиков

  \param start_time_val
  \param stop_time_val

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Time_diff_miliseconds(uint32_t start_time_val, uint32_t stop_time_val)
{
  return ((stop_time_val - start_time_val) * 1000ull) / TX_TIMER_TICKS_PER_SECOND;
}

/*-----------------------------------------------------------------------------------------------------


  \param st
-----------------------------------------------------------------------------------------------------*/
void Get_hw_timestump(T_sys_timestump* pst)
{
  uint32_t scy;
  ULONG    scl1;
  ULONG    scl2;

  scl1 = _tx_timer_system_clock;
  scy  = SysTick->VAL;
  scl2 = _tx_timer_system_clock;
  if (scl1!=scl2)
  {
    pst->cycles  = SysTick->VAL;
    pst->ticks   = scl2;
  }
  else
  {
    pst->cycles  = scy;
    pst->ticks   = scl1;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param p_begin время в прошлом
  \param p_end   врямя в настоящем

  \return uint64_t
-----------------------------------------------------------------------------------------------------*/
uint64_t Hw_timestump_diff64_us(T_sys_timestump* p_begin, T_sys_timestump* p_end)
{
  uint64_t  val;
  int32_t   tmp;
  int32_t   uc_in_tick;
  uint32_t  c1;
  uint32_t  c2;
  uint32_t  sys_div    = ICLK_FREQ/TX_TIMER_TICKS_PER_SECOND;
  uint32_t  sys_freq   = ICLK_FREQ;
  uint32_t  ticks_freq = TX_TIMER_TICKS_PER_SECOND;

  c1 = sys_div - p_begin->cycles -1;
  c2 = sys_div - p_end->cycles - 1;
  uc_in_tick = 1000000ul/ticks_freq;
  val = (uint64_t)(p_end->ticks - p_begin->ticks)*uc_in_tick;

  if (c2 >= c1)
  {
    tmp = (c2 - c1)/(sys_freq/1000000ul);
    val += (uint64_t)tmp;
  }
  else
  {
    tmp = (sys_div -  (c1 - c2))/(sys_freq/1000000ul);
    val = val - (uint64_t)uc_in_tick +  (uint64_t)tmp;
  }
  return val;
}

/*-----------------------------------------------------------------------------------------------------


  \param start_time_val
  \param stop_time_val

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Hw_timestump_diff32_us(T_sys_timestump *p_begin, T_sys_timestump *p_end)
{
  return (uint32_t)Hw_timestump_diff64_us(p_begin,p_end);
}


/*-----------------------------------------------------------------------------------------------------


  \param time_ms

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ms_to_ticks(uint32_t time_ms)
{
    return (((time_ms * TX_TIMER_TICKS_PER_SECOND) / 1000U) + 1U);
}

/*-----------------------------------------------------------------------------------------------------


  \param ms

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Wait_ms(uint32_t ms )
{
  return tx_thread_sleep(ms_to_ticks(ms));;
}


/*-----------------------------------------------------------------------------------------------------


  \param p_time

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Time_elapsed_sec(T_sys_timestump *p_time)
{
  uint64_t secs;
  T_sys_timestump now;
  Get_hw_timestump(&now);
  secs = Hw_timestump_diff64_us(p_time, &now)/1000000ull;
  return secs;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_time

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Time_elapsed_msec(T_sys_timestump *p_time)
{
  uint64_t msecs;
  T_sys_timestump now;
  Get_hw_timestump(&now);
  msecs = Hw_timestump_diff64_us(p_time, &now)/1000ull;
  return msecs;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_time

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Time_elapsed_usec(T_sys_timestump *p_time)
{
  uint64_t usecs;
  T_sys_timestump now;
  Get_hw_timestump(&now);
  usecs = Hw_timestump_diff64_us(p_time, &now);
  return usecs;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_time
  \param sec
  \param usec

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
void Timestump_convert_to_sec_usec(T_sys_timestump *p_timestump, uint32_t *sec, uint32_t *usec)
{
  *sec  = p_timestump->ticks/TX_TIMER_TICKS_PER_SECOND;
  *usec = (p_timestump->ticks % TX_TIMER_TICKS_PER_SECOND)*(1000000ul/TX_TIMER_TICKS_PER_SECOND) + p_timestump->cycles/(SYSTEM_CLOCK/1000000ul);
}


/*-----------------------------------------------------------------------------------------------------


  \param p_old_time   время в прошлом, более старое  время
  \param p_new_time   врямя в настоящем, более новое время

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Timestump_diff_to_usec(T_sys_timestump *p_old_time,T_sys_timestump *p_new_time)
{
  uint64_t diff64;
  diff64 = Hw_timestump_diff64_us(p_old_time, p_new_time);
  return (uint32_t)(diff64);
}

/*-----------------------------------------------------------------------------------------------------


  \param p_old_time   время в прошлом, более старое  время
  \param p_new_time   врямя в настоящем, более новое время

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Timestump_diff_to_msec(T_sys_timestump *p_old_time,T_sys_timestump *p_new_time)
{
  uint64_t diff64;
  diff64 = Hw_timestump_diff64_us(p_old_time, p_new_time);
  return (uint32_t)(diff64/1000ull);
}

/*-----------------------------------------------------------------------------------------------------


  \param p_old_time   время в прошлом, более старое  время
  \param p_new_time   врямя в настоящем, более новое время

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Timestump_diff_to_sec(T_sys_timestump *p_old_time,T_sys_timestump *p_new_time)
{
  uint64_t diff64;
  diff64 = Hw_timestump_diff64_us(p_old_time, p_new_time);
  return (uint32_t)(diff64/1000000ull);
}


