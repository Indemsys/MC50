// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-07-13
// 11:47:46
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


static   T_exp_filter_fp         expflt_V_IPWR;
static   T_exp_filter_fp         expflt_V_VPWR;
static   T_exp_filter_fp         expflt_motor_current;


static   T_exp_filter_fp         expflt_servo_sens;       // Экспоненциальный фильтр скорости вращения сервопотенциометра
static   T_exp_filter_fp         expflt_servo_sens_smoothly;  // Экспоненциальный фильтр скорости вращения сервопотенциометра

#define  SERVO_SENS_RAV_FLTR_LEN 8
static   T_run_average_float_N   ravflt_servo_sens;
static   T_run_average_float_N   ravflt_servo_sens_smoothly;
static   float                   servo_sens_flt_arr[SERVO_SENS_RAV_FLTR_LEN]; // Массив выборок фильтра бегущего среднего скорости вращения сервопотенциометра
static   float                   servo_sens_flt_arr2[SERVO_SENS_RAV_FLTR_LEN]; // Массив выборок фильтра бегущего среднего скорости вращения сервопотенциометра

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Currents_offset_calibration(void)
{
  uint32_t  res;

  adc.smpl_V_IU_acc   = 0;
  adc.smpl_V_IV_acc   = 0;
  adc.smpl_V_IW_acc   = 0;
  adc.smpl_VREF_R_acc = 0;
  adc.smpl_acc_cnt    = CURR_CALIBR_SMPLS_NUM;

  res = ADC_wait_averaging_done();
  if (res == TX_SUCCESS)
  {
    adc.smpl_V_IU_offs   = adc.smpl_V_IU_acc / CURR_CALIBR_SMPLS_NUM;
    adc.smpl_V_IV_offs   = adc.smpl_V_IV_acc / CURR_CALIBR_SMPLS_NUM;
    adc.smpl_V_IW_offs   = adc.smpl_V_IW_acc / CURR_CALIBR_SMPLS_NUM;
    adc.smpl_VREF_R_aver = (float)adc.smpl_VREF_R_acc / (float)CURR_CALIBR_SMPLS_NUM;
  }
  else
  {
    return RES_ERROR;
  }
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Проводим калибровку нуля усилителей токовых датчиков


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Calculating_scaling_factors(void)
{
  float     k;

  switch (tmc6200_cbl.gconf.amplification)
  {
  case TMC6200_AMPLIFICATION_5:
    k = 5.0f;
    break;
  case TMC6200_AMPLIFICATION_10:
    k = 10.0f;
    break;
  case TMC6200_AMPLIFICATION_20:
    k = 20.0f;
    break;
  }

  // Вычисляем цену деления АЦП на основе усредненного значения сигнала VREF_R и знания реального напряжения на нем
  adc.adc_scale           = REF_VOLTAGE / adc.smpl_VREF_R_aver;

  // Вычисляем коэффициенты приведения к физическим значениям
  adc.current_shunt_scale = adc.adc_scale / (k * wvar.shunt_resistor);
  adc.current_pwr_scale   = adc.adc_scale / (I_PWR_AMPL * wvar.input_shunt_resistor);
  adc.voltage_pwr_scale   = adc.adc_scale / V_PWR_DIV;


  // Вычисляем коэффициент приведения угла поворота выходного вала из отсчетов АЦП в градусы
  float diff = (float)((int32_t)wvar.open_position - (int32_t)wvar.close_position);
  diff = fabsf(diff);
  adc.samples_to_grad_scale =(90.0f) / diff;
  adc.grad_to_samples_scale = 1.0f / adc.samples_to_grad_scale;
  // Вычисляем коэффициент приведения скорости выходного вала из отсчетов АЦП в секунду в градусы в секунду
  adc.samples_ps_to_grad_ps = adc.samples_to_grad_scale * wvar.pwm_frequency;

  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
  Вычисление значений параметров в их физическом представлении
  Значения применяются в основном для диагностики и отладки
  Вызывается из задачи обслуживания мотора

  \param void
-----------------------------------------------------------------------------------------------------*/
void Real_values_calculation(void)
{
  adc.i_u       = adc.smpl_V_IU_norm * adc.current_shunt_scale;
  adc.i_v       = adc.smpl_V_IV_norm * adc.current_shunt_scale;
  adc.i_w       = adc.smpl_V_IW_norm * adc.current_shunt_scale;

  adc.i_pwr     = adc.V_IPWR_fltr * adc.current_pwr_scale;
  adc.v_pwr     = adc.V_VPWR_fltr * adc.voltage_pwr_scale;
  adc.input_pwr = adc.i_pwr * adc.v_pwr;

  adc.pos_sens  = adc.smpl_POS_SENS * adc.adc_scale;
  adc.misc      = adc.smpl_MISC * adc.adc_scale;


  if (wvar.app_type == APP_TYPE_SERVO_BLDC)
  {
    if (h3.turn_capt != 0)
    {
      drv_cbl.mot_rpm =((float)PCLKD_FREQ * 60.0f * (float)h3.rot_dir) / ((float)h3.turn_capt);
    }
    else
    {
      drv_cbl.mot_rpm = 0.0f;
    }
  }
  else if (wvar.app_type == APP_TYPE_TRACTION_DC)
  {
    if (qenc.delta_aver != 0)
    {
      drv_cbl.mot_rpm =((float)PCLKD_FREQ * 60.0f * (float)qenc.ror_dir) / ((float)qenc.delta_aver * 32.0f);
    }
    else
    {
      drv_cbl.mot_rpm = 0.0f;
    }
  }

  if (drv_cbl.opening_direction == OPENING_CLOCKWISE)
  {
    adc.shaft_position_grad =((int32_t)adc.smpl_POS_SENS - (int32_t)wvar.close_position) * adc.samples_to_grad_scale;
  }
  else
  {
    adc.shaft_position_grad =-((int32_t)adc.smpl_POS_SENS - (int32_t)wvar.close_position) * adc.samples_to_grad_scale;
  }

  if (adc.v_pwr <= MINIMAL_POWER_VOLTAGE)
  {
    App_set_error_flags(APP_ERR_LOW_PWR_SRC_VOLTAGE);
  }
  else if  (adc.v_pwr >= NORMAL_POWER_VOLTAGE)
  {
    App_clear_error_flags(APP_ERR_LOW_PWR_SRC_VOLTAGE);
  }

}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Filters_init(void)
{
  expflt_V_IPWR.alpha       = 1 / 512.0f;
  expflt_V_IPWR.init        = 0;
  expflt_V_VPWR.alpha       = 1 / 512.0f;
  expflt_V_VPWR.init        = 0;

  expflt_motor_current.alpha = 1 / 64.0f;
  expflt_motor_current.init  = 0;

  if (wvar.adrc_filter_k == 0)
  {
    expflt_servo_sens.alpha   = 1.0f / 256.0f;
  }
  else
  {
    expflt_servo_sens.alpha   = 1.0f / wvar.adrc_filter_k;
  }
  expflt_servo_sens.init    = 0;

  expflt_servo_sens_smoothly.alpha  = 1.0f / 2048.0f;
  expflt_servo_sens_smoothly.init   = 0;

  ravflt_servo_sens.arr     = servo_sens_flt_arr;
  ravflt_servo_sens.len     = SERVO_SENS_RAV_FLTR_LEN;
  ravflt_servo_sens.init    = 0;

  ravflt_servo_sens_smoothly.arr    = servo_sens_flt_arr2;
  ravflt_servo_sens_smoothly.len    = SERVO_SENS_RAV_FLTR_LEN;
  ravflt_servo_sens_smoothly.init   = 0;


}

/*-----------------------------------------------------------------------------------------------------
  Измерение мгновенного тока в фазах мотора

  Вызывается в процедуре обслуживания прерывания ADC
  Частота вызова равна частоте ШИМ мотора и равна PWM_FREQ_HZ


  \param void
-----------------------------------------------------------------------------------------------------*/
void Measure_instant_phases_current(void)
{
  // Вычисляем откорректированные отсчеты с датчиков тока мотора
  adc.smpl_V_IU_norm = (int32_t)adc.smpl_V_IU - adc.smpl_V_IU_offs;
  adc.smpl_V_IV_norm = (int32_t)adc.smpl_V_IV - adc.smpl_V_IV_offs;
  adc.smpl_V_IW_norm = (int32_t)adc.smpl_V_IW - adc.smpl_V_IW_offs;
}

/*-----------------------------------------------------------------------------------------------------
  Измерение суммарного абсолютного тока через BLDC мотор

  Вызывается в процедуре обслуживания прерывания ADC
  Частота вызова равна частоте ШИМ мотора и равна PWM_FREQ_HZ

  \param void
-----------------------------------------------------------------------------------------------------*/
void Measure_overall_BLDC_motor_current(void)
{
  adc.instant_bldc_motor_current   =((abs(adc.smpl_V_IU_norm)+ abs(adc.smpl_V_IV_norm)+ abs(adc.smpl_V_IW_norm)) / 2) * adc.current_shunt_scale;
  adc.filtered_bldc_motor_current  = Exponential_filter_fp(&expflt_motor_current , adc.instant_bldc_motor_current);
}

/*-----------------------------------------------------------------------------------------------------
  Измерение тока через DC моторы

  Вызывается в процедуре обслуживания прерывания ADC
  Частота вызова равна частоте ШИМ мотора и равна PWM_FREQ_HZ

  \param void
-----------------------------------------------------------------------------------------------------*/
void Measure_DC_motors_current(void)
{
  adc.instant_dc_motor_current   = adc.smpl_V_IU_norm * adc.current_shunt_scale;
  adc.filtered_dc_motor_current  = Exponential_filter_fp(&expflt_motor_current , adc.instant_dc_motor_current);

}

/*-----------------------------------------------------------------------------------------------------
  Измерение тока и напряжения на DC шине

  Вызывается в процедуре обслуживания прерывания ADC
  Частота вызова равна частоте ШИМ мотора и равна PWM_FREQ_HZ

  \param void
-----------------------------------------------------------------------------------------------------*/
void Measure_DC_bus_voltage_current(void)
{
  adc.V_IPWR_fltr  = Exponential_filter_fp(&expflt_V_IPWR , (float)adc.smpl_V_IPWR);
  adc.V_VPWR_fltr  = Exponential_filter_fp(&expflt_V_VPWR , (float)adc.smpl_V_VPWR);
}

/*-----------------------------------------------------------------------------------------------------
  Функция вычисления и фильтации скрости сервопотенциометра
  Вызывается в процедуре обслуживания прерывания ADC
  Частота вызова равна частоте ШИМ мотора и равна PWM_FREQ_HZ

  \param void
-----------------------------------------------------------------------------------------------------*/
void Measure_servo_sensor_speed(void)
{
  float     diff;
  float     f;

  diff = (int32_t)adc.smpl_POS_SENS - adc.prev_smpl_POS_SENS;
  f  = Exponential_filter_fp(&expflt_servo_sens , (float)diff);
  adc.shaft_speed_rt = RunAverageFilter_float_N(&ravflt_servo_sens, f) * adc.samples_ps_to_grad_ps;
  adc.shaft_abs_speed_rt = fabsf(adc.shaft_speed_rt);

  f  = Exponential_filter_fp(&expflt_servo_sens_smoothly , (float)diff);
  adc.shaft_speed_smoothly = RunAverageFilter_float_N(&ravflt_servo_sens_smoothly, f) * adc.samples_ps_to_grad_ps;
  adc.shaft_abs_speed_smoothly = fabsf(adc.shaft_speed_smoothly);

}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return void Temperature_values_
-----------------------------------------------------------------------------------------------------*/
void Temperatures_calculation(void)
{
  // Расчет температуры термистора возле силовых транзисторов
  float R0 = 10000.0f;
  float T0 = 298.15f; // 25 C в кельвинах
  float B  = 3930.0f; // Для NCP21XV103J03RA
  float RC = R0 * expf(-B / T0);
  float v  = adc.smpl_TEMP * adc.adc_scale;
  float r  = v / ((VREF-v) / R0);
  float t  = B / logf(r / RC)- 273.15f;
  adc.drv_temp = t;


  // Рсчет температуры термистора на моторе
  float B1 = 3988.0f;

  v  = adc.smpl_EXT_TEMP * adc.adc_scale;
  r  = v / ((VREF-v) / R0);
  t  =(25.0f + 273.15f) / ((logf(r / R0) * (25.0f + 273.15f)) / B1 + 1)- 273.15f;
  adc.ext_temp  = t;

  // Расчет температуры микроконтроллера
  float v1 =(R_TSN_CAL->TSCDR * 3.3f) / 4096.0f;
  t = adc.smpl_CPU_temp * adc.adc_scale;
  t =(t - v1) / 0.004f  + 127.0f;
  adc.cpu_temp  = t;

  if (adc.drv_temp >= MAXIMAL_DRIVER_TEMP)
  {
    App_set_error_flags(APP_ERR_DRV_OVERHEAT);
  }
  else if (adc.drv_temp <= (MAXIMAL_DRIVER_TEMP - 5.0f))
  {
    App_clear_error_flags(APP_ERR_DRV_OVERHEAT);
  }

  if (adc.ext_temp >= MAXIMAL_MOTOR_TEMP)
  {
    App_set_error_flags(APP_ERR_MOT_OVERHEAT);
  }
  else if (adc.ext_temp <= (MAXIMAL_MOTOR_TEMP - 5.0f))
  {
    App_clear_error_flags(APP_ERR_MOT_OVERHEAT);
  }

  if (adc.cpu_temp >= MAXIMAL_CPU_TEMP)
  {
    App_set_error_flags(APP_ERR_CPU_OVERHEAT);
  }
  else if (adc.cpu_temp <= (MAXIMAL_CPU_TEMP - 5.0f))
  {
    App_clear_error_flags(APP_ERR_CPU_OVERHEAT);
  }
}




