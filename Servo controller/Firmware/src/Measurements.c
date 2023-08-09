// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-07-13
// 11:47:46
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


static   T_exp_filter_fp         expflt_V_IPWR;
static   T_exp_filter_fp         expflt_V_VPWR;
static   T_exp_filter_fp         expflt_inst_current;


static   T_exp_filter_fp         expflt_servo_sens; // Экспоненциальный фильтр скорости вращения сервопотенциометра

#define  SERVO_SENS_RAV_FLTR_LEN 8
static   T_run_average_float_N   ravflt_servo_sens;
static   float                   servo_sens_flt_arr[SERVO_SENS_RAV_FLTR_LEN]; // Массив выборок фильтра бегущего среднего скорости вращения сервопотенциометра


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
  adc.current_shunt_scale = adc.adc_scale / (k * CURR_SHUNT_RES);
  adc.current_pwr_scale   = adc.adc_scale / (I_PWR_AMPL * CURR_SHUNT_RES);
  adc.voltage_pwr_scale   = adc.adc_scale / V_PWR_DIV;


  // Вычисляем коэффициент приведения угла поворота выходного вала из отсчетов АЦП в градусы
  float diff = (float)((int32_t)wvar.open_position - (int32_t)wvar.close_position);
  diff = fabsf(diff);
  adc.samples_to_grad_scale =(90.0f) / diff;
  adc.grad_to_samples_scale = 1.0f / adc.samples_to_grad_scale;
  // Вычисляем коэффициент приведения скорости выходного вала из отсчетов АЦП в секунду в градусы в секунду
  adc.samples_ps_to_grad_ps = adc.samples_to_grad_scale * PWM_FREQ_HZ;

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

  if (one_turn_period != 0)
  {
    mot_cbl.mot_rpm =((float)PCLKD_FREQ * 60.0f * (float)rotating_direction) / ((float)one_turn_period);
  }
  else
  {
    mot_cbl.mot_rpm = 0.0f;
  }

  if (mot_cbl.opening_direction == OPENING_CLOCKWISE)
  {
    adc.shaft_position_grad =((int32_t)adc.smpl_POS_SENS - (int32_t)wvar.close_position) * adc.samples_to_grad_scale;
  }
  else
  {
    adc.shaft_position_grad =-((int32_t)adc.smpl_POS_SENS - (int32_t)wvar.close_position) * adc.samples_to_grad_scale;
  }

  if (adc.v_pwr <= MINIMAL_POWER_VOLTAGE)
  {
    App_set_error_flags(APP_ERR_LOW_VOLTAGE);
  }
  else if  (adc.v_pwr >= NORMAL_POWER_VOLTAGE)
  {
    App_clear_error_flags(APP_ERR_LOW_VOLTAGE);
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
  expflt_inst_current.alpha = 1 / 64.0f;
  expflt_inst_current.init  = 0;
  expflt_servo_sens.alpha   = 1 / 2048.0f;
  expflt_servo_sens.init    = 0;

  ravflt_servo_sens.arr     = servo_sens_flt_arr;
  ravflt_servo_sens.len     = SERVO_SENS_RAV_FLTR_LEN;
  ravflt_servo_sens.init    = 0;
}

/*-----------------------------------------------------------------------------------------------------
  Измерение мгновенного тока и среднего тока за период коммутационного шага
  Вызывается в процедуре обслуживания прерывания ADC
  Частота вызова равна частоте ШИМ мотора и равна PWM_FREQ_HZ


  \param void
-----------------------------------------------------------------------------------------------------*/
void Measure_instant_current(void)
{
  // Вычисляем откорректированные отсчеты с датчиков тока мотора
  adc.smpl_V_IU_norm = (int32_t)adc.smpl_V_IU - adc.smpl_V_IU_offs;
  adc.smpl_V_IV_norm = (int32_t)adc.smpl_V_IV - adc.smpl_V_IV_offs;
  adc.smpl_V_IW_norm = (int32_t)adc.smpl_V_IW - adc.smpl_V_IW_offs;

  adc.V_IPWR_fltr  = Exponential_filter_fp(&expflt_V_IPWR , (float)adc.smpl_V_IPWR);
  adc.V_VPWR_fltr  = Exponential_filter_fp(&expflt_V_VPWR , (float)adc.smpl_V_VPWR);

  adc.instant_motor_current       =(abs(adc.smpl_V_IU_norm)+ abs(adc.smpl_V_IV_norm)+ abs(adc.smpl_V_IW_norm)) / 2;
  adc.instant_motor_current_fltr  = Exponential_filter_fp(&expflt_inst_current , (float)adc.instant_motor_current);
  adc.filtered_motor_current      = adc.instant_motor_current_fltr  * adc.current_shunt_scale;

  if (hall_state != current_hall_state)
  {
    //skip_adc_res = 3; // При смене коммутационной конфигурации , происходят рандомные осцилляции,
    // поэтому отсчеты АЦП на первом периоде ШИМ после перекоммутации игнорируем
    // Фича требует перепроверки

    if (adc.smpl_avr_cnt != 0)
    {
      // Расчет среднего тока за предыдущий коммутационный период
      adc.smpl_V_IU_norm_avr = adc.smpl_V_IU_norm_acc / adc.smpl_avr_cnt;
      adc.smpl_V_IV_norm_avr = adc.smpl_V_IV_norm_acc / adc.smpl_avr_cnt;
      adc.smpl_V_IW_norm_avr = adc.smpl_V_IW_norm_acc / adc.smpl_avr_cnt;
    }

    // Начало накопление сумматоров тока на новом коммутационном периоде
    adc.smpl_avr_cnt = 1;
    adc.smpl_V_IU_norm_acc  = adc.smpl_V_IU_norm;
    adc.smpl_V_IV_norm_acc  = adc.smpl_V_IV_norm;
    adc.smpl_V_IW_norm_acc  = adc.smpl_V_IW_norm;

    hall_state = current_hall_state;
  }
  else
  {
    // Накопление сумматоров тока на коммутационном периоде
    adc.smpl_V_IU_norm_acc  += adc.smpl_V_IU_norm;
    adc.smpl_V_IV_norm_acc  += adc.smpl_V_IV_norm;
    adc.smpl_V_IW_norm_acc  += adc.smpl_V_IW_norm;
    adc.smpl_avr_cnt++;
  }
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
  adc.shaft_speed = RunAverageFilter_float_N(&ravflt_servo_sens, f) * adc.samples_ps_to_grad_ps;
  adc.shaft_abs_speed = fabsf(adc.shaft_speed);
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

