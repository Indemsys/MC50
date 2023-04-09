#ifndef TIME_UTILS_H
  #define TIME_UTILS_H

#define  MS_TO_TICKS(x) (((x * TX_TIMER_TICKS_PER_SECOND) / 1000U) + 1U)


void     Get_system_timings(uint32_t *p_sys_div, uint32_t *p_sys_freq, uint32_t *p_ticks_freq);
uint32_t Get_system_ticks(uint32_t *v );
uint32_t Time_diff_seconds(uint32_t start_time_val, uint32_t stop_time_val);
uint32_t Time_diff_miliseconds(uint32_t start_time_val, uint32_t stop_time_val);
uint32_t Time_diff_microseconds(uint32_t start_time_val, uint32_t stop_time_val);

void     Get_hw_timestump(T_sys_timestump* pst);
uint64_t Hw_timestump_diff64_us(T_sys_timestump* p_begin, T_sys_timestump* p_end);
uint32_t Hw_timestump_diff32_us(T_sys_timestump *p_begin, T_sys_timestump *p_end);

uint32_t ms_to_ticks(uint32_t time_ms);
uint32_t Wait_ms(uint32_t ms );

uint32_t Time_elapsed_sec(T_sys_timestump *p_time);
uint32_t Time_elapsed_msec(T_sys_timestump *p_time);
uint32_t Time_elapsed_usec(T_sys_timestump *p_time);
void     Timestump_convert_to_sec_usec(T_sys_timestump *p_timestump, uint32_t *sec, uint32_t *usec);

uint32_t Timestump_diff_to_usec(T_sys_timestump *p_old_time,T_sys_timestump *p_new_time);
uint32_t Timestump_diff_to_msec(T_sys_timestump *p_old_time,T_sys_timestump *p_new_time);
uint32_t Timestump_diff_to_sec(T_sys_timestump *p_old_time,T_sys_timestump *p_new_time);

#endif



