#ifndef BACKGROUND_THREAD_H
  #define BACKGROUND_THREAD_H

#define  REF_TIME_INTERVAL   5       // Интервал времени в милисекундах на котором производлится калибровка и измерение загруженности процессора

extern volatile uint32_t     g_aver_cpu_usage;
extern volatile uint32_t     g_cpu_usage;

void     Get_reference_time(void);
uint32_t Create_Backgroung_task(void);
uint64_t Measure_reference_time_interval(uint32_t time_delay_ms);
uint32_t Send_flag_to_background(uint32_t flag);

#endif



