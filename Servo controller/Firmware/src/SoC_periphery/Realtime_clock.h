#ifndef REALTIME_CLOCK_H
  #define REALTIME_CLOCK_H


#define  NO_ACTIVE_RTC        0  // Нет валидных часов календарного  времени
#define  ACTIVE_INTERNAL_RTC  1  // Календарное время извлечено из внутренних часов микроконтроллера
#define  ACTIVE_EXTERNAL_RTC  2  // Календарное время извлечено из внешнего чипа часов

typedef struct
{
    uint32_t RTC_status;
    uint32_t RTC_valid;

} T_RTC_init_res;


extern T_RTC_init_res  rtc_init_res;
extern const rtc_instance_t rtc_cbl;

T_RTC_init_res* RTC_get_init_res(void);
void            Init_RTC(void);
void            RTC_set_DateTime(rtc_time_t   *p_rt_time);
void            RTC_get_system_DateTime(rtc_time_t   *p_rt_time);
void            RTC_set_system_DateTime(rtc_time_t   *p_rt_time);
int             WorkDay_from_date(rtc_time_t   *p_rt_time);
int32_t         Compare_date_time(rtc_time_t *p_dt1, rtc_time_t *p_dt2);
void            Convert_NTP_to_UTC_time(ULONG seconds, rtc_time_t   *p_rt_time, float utc_offset);


#endif // APP_REALTIME_CLOCK_H



