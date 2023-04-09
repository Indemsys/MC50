// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.07.11
// 23:40:58
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


rtc_instance_ctrl_t rtc_ctrl;

const rtc_cfg_t rtc_cfg =
{
  .clock_source           = RTC_CLOCK_SOURCE_SUBCLK,
  .hw_cfg                 = false,
  .error_adjustment_value = 0,
  .error_adjustment_type  = RTC_ERROR_ADJUSTMENT_NONE,
  .p_callback             = NULL,
  .p_context              =&rtc_cbl,
  .alarm_ipl              =(BSP_IRQ_DISABLED),
  .periodic_ipl           =(BSP_IRQ_DISABLED),
  .carry_ipl              =(12),
};


const rtc_instance_t rtc_cbl =
{
  .p_ctrl        =&rtc_ctrl,
  .p_cfg         =&rtc_cfg,
  .p_api         =&g_rtc_on_rtc
};


T_RTC_init_res  rtc_init_res;

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return T_RTC_init_res*
-----------------------------------------------------------------------------------------------------*/
T_RTC_init_res* RTC_get_init_res(void)
{
  return &rtc_init_res;
}


/*-------------------------------------------------------------------------------------------------------------
  Инициализация часов реального времени чипа


 Поля структуры tm:
 tm.sec   int   seconds after the minute  0-61*
 tm.min   int   minutes after the hour  0-59
 tm.hour  int   hours since midnight  0-23
 tm.mday  int   day of the month  1-31
 tm.mon   int   months since January  0-11
 tm.year  int   years since 1900
 tm.wday  int   days since Sunday 0-6
 tm.yday  int   days since January 1  0-365
 tm.isdst int   Daylight Saving Time flag


-------------------------------------------------------------------------------------------------------------*/
void Init_RTC(void)
{
  rtc_info_t   info1;
  rtc_time_t   rt_time = {0};


  rtc_init_res.RTC_status = NO_ACTIVE_RTC;

  // Инициализируем внутренний чип микроконтроллера
  rtc_cbl.p_api->open(rtc_cbl.p_ctrl,rtc_cbl.p_cfg);
  rtc_cbl.p_api->infoGet(rtc_cbl.p_ctrl,&info1);

  /* initialize RTC if its status is stopped state, that is, on cold start */
  if (RTC_STATUS_STOPPED == info1.status)
  {
    /* if the RTC clock source is sub-clock, stop it so that the sub-clock drive capacity is set correctly in the configure API */
    g_cgc.p_api->clockStop(CGC_CLOCK_SUBCLOCK);
    rtc_cbl.p_api->configure(rtc_cbl.p_ctrl, NULL);
  }

  rtc_init_res.RTC_valid = 0;

  if (info1.status == RTC_STATUS_STOPPED)
  {
    rt_time.tm_sec    = 0;
    rt_time.tm_min    = 0;
    rt_time.tm_hour   = 0;
    rt_time.tm_mday   = 1;
    rt_time.tm_mon    = 0;           // Счет месяцев начинается с 0
    rt_time.tm_year   = 2020 - 1900; // 2020 год
    rt_time.tm_wday   = 0;
    rt_time.tm_yday   = 0;
    rt_time.tm_isdst  = 1;
    mktime(&rt_time);
    rtc_cbl.p_api->calendarTimeSet(rtc_cbl.p_ctrl,&rt_time,true);
  }
  else
  {
    rtc_init_res.RTC_valid = 1;
    rtc_init_res.RTC_status = ACTIVE_INTERNAL_RTC;
  }
}


/*-----------------------------------------------------------------------------------------------------
  Установка даты-времени
  Год в  API часов отсичитывается с 1900 года,
  внутри функции API их него вычитается 100 и таким образом часы считают от 2000 года до 2099 года

  \param p_rt_time
-----------------------------------------------------------------------------------------------------*/
void RTC_set_DateTime(rtc_time_t   *p_rt_time)
{
  rtc_cbl.p_api->calendarTimeSet(rtc_cbl.p_ctrl,p_rt_time,true);
}

/*-----------------------------------------------------------------------------------------------------
  Получаем значение времени в формате языка C.  Т.е. счет месяцев начинается с 0

  \param p_rt_time
-----------------------------------------------------------------------------------------------------*/
void RTC_get_system_DateTime(rtc_time_t   *p_rt_time)
{
  rtc_cbl.p_api->calendarTimeGet(rtc_cbl.p_ctrl, p_rt_time);
  p_rt_time->tm_wday  = 0;
  p_rt_time->tm_yday  = 0;
  p_rt_time->tm_isdst = 0;
}


/*-----------------------------------------------------------------------------------------------------
  Установить системные дату и время

  Описание полей структуры rtc_time_t
  tm_sec     int  seconds after the minute  0-61*
  tm_min     int  minutes after the hour  0-59
  tm_hour    int  hours since midnight  0-23
  tm_mday    int  day of the month  1-31
  tm_mon     int  months since January  0-11
  tm_year    int  years since 1900
  tm_wday    int  days since Sunday 0-6
  tm_yday    int  days since January 1  0-365
  tm_isdst   int  Daylight Saving Time flag

  The Daylight Saving Time flag (tm_isdst) is greater than zero if Daylight Saving Time is in effect,
  zero if Daylight Saving Time is not in effect, and less than zero if the information is not available.

  \param p_rt_time
-----------------------------------------------------------------------------------------------------*/
void RTC_set_system_DateTime(rtc_time_t   *p_rt_time)
{
  p_rt_time->tm_isdst  = 1;
  mktime(p_rt_time);
  RTC_set_DateTime(p_rt_time);
}

/*-----------------------------------------------------------------------------------------------------
  Конвертирование количества секунд полученное по NTP (NTP начинается с 1/1/1900-00:00h) в значение времени в формате rtc_time_t (UTC начинается с 1/1/1970-00:00h)

  Время в формате NTP на (70*365+16)*60*60*24 = 2208902400 сек больше чем время в формате UTC
  либо на 25567 дней

  \param p_rt_time
-----------------------------------------------------------------------------------------------------*/
void Convert_NTP_to_UTC_time(ULONG seconds, rtc_time_t   *p_rt_time, float utc_offset)
{
  seconds = seconds  - 2208902400ll;
  p_rt_time->tm_sec    = seconds;
  p_rt_time->tm_min    = 0;
  p_rt_time->tm_hour   = (int32_t)utc_offset;
  p_rt_time->tm_mday   = 0;
  p_rt_time->tm_mon    = 0;
  p_rt_time->tm_year   = 0;
  p_rt_time->tm_wday   = 0;
  p_rt_time->tm_yday   = 0;
  p_rt_time->tm_isdst  = 1;
  mktime(p_rt_time);
}

/*-----------------------------------------------------------------------------------------------------
  Возвращает номер дня недели
  Воскресенье = 0, Понедельник = 1, ... Суббота = 6
  Аргумент передается в формате принятом для структуры даты времени языка С

  \param p_rt_time

  \return int8_t
-----------------------------------------------------------------------------------------------------*/
int WorkDay_from_date(rtc_time_t   *p_rt_time)
{
  int      wday = 0;
  int year  = p_rt_time->tm_year + 1900;
  int month = p_rt_time->tm_mon + 1;
  int day   = p_rt_time->tm_mday;
  wday =(day +((153 * (month + 12 * ((14 - month) / 12)- 3)+ 2) / 5)+(365 * (year + 4800 -((14 - month) / 12)))+((year + 4800 -((14 - month) / 12)) / 4)-((year + 4800 -((14 - month) / 12)) / 100)+((year + 4800 -((14 - month) / 12)) / 400)- 32044)% 7;
  return wday;
}


/*-----------------------------------------------------------------------------------------------------
  Сравнение даты времени
  Если p_dt1 > p_dt2, то возвращаем 1
  Если p_dt1 = p_dt2, то возвращаем 0
  Если p_dt1 < p_dt2, то возвращаем -1


  \param p_dt1
  \param p_dt2

  \return int
-----------------------------------------------------------------------------------------------------*/
int32_t Compare_date_time(rtc_time_t *p_dt1, rtc_time_t *p_dt2)
{
  if (p_dt1->tm_year > p_dt2->tm_year) return 1;
  else  if (p_dt1->tm_year < p_dt2->tm_year) return -1;

  if (p_dt1->tm_mon > p_dt2->tm_mon) return 1;
  else  if (p_dt1->tm_mon < p_dt2->tm_mon) return -1;

  if (p_dt1->tm_mday > p_dt2->tm_mday) return 1;
  else  if (p_dt1->tm_mday < p_dt2->tm_mday) return -1;

  if (p_dt1->tm_hour > p_dt2->tm_hour) return 1;
  else  if (p_dt1->tm_hour < p_dt2->tm_hour) return -1;

  if (p_dt1->tm_min > p_dt2->tm_min) return 1;
  else  if (p_dt1->tm_min < p_dt2->tm_min) return -1;

  if (p_dt1->tm_sec > p_dt2->tm_sec) return 1;
  else  if (p_dt1->tm_sec < p_dt2->tm_sec) return -1;

  return 0;
}

