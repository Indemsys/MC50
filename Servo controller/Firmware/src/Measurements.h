﻿#ifndef MEASUREMENTS_H
  #define MEASUREMENTS_H

  #define CURR_CALIBR_SMPLS_NUM     256  // Количество сэмплов для калибровки сенсоров тока

void       Filters_init(void);
void       Measure_instant_current(void);
uint32_t   Currents_offset_calibration(void);
uint32_t   Calculating_scaling_factors(void);

void       Real_values_calculation(void);
void       Temperatures_calculation(void);
void       Measure_servo_sensor_speed(void);

#endif // MEASUREMENTS_H


