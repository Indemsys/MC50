#ifndef ADC_H
  #define ADC_H

  #define    INT_ADC_SMPL_RATE_HZ         16000ul
  #define    ADC_SMPL_RATE_DIV            (60000000ul/INT_ADC_SMPL_RATE_HZ)  // Прерывания ADC каждую мс
  #define    INPUTS_TASK_EVENT_RATE_DIV   (INT_ADC_SMPL_RATE_HZ/INPTUT_TASK_RATE_HZ)


  #define    VOLTAGE_3V3     3.31f
  #define    ADC_RANGE       4096.0f
  #define    CURR_SHUNT_AMP  50.0f
  #define    CURR_SHUNT_RES  0.1f
  #define    VIN_SCALE       (10.0f/(10.0f+100.0f))
  #define    U5V_SCALE       (10.0f/(10.0f+10.0f))
  #define    SYSV_SCALE      (10.0f/(10.0f+100.0f))


typedef struct
{
    uint16_t smpl_V_IU;     // ADC0 AN000
    uint16_t smpl_V_IV;     // ADC0 AN001
    uint16_t smpl_V_IW;     // ADC0 AN002
    uint16_t smpl_V_IPWR;   // ADC0 AN003
    uint16_t smpl_VREF_R;   // ADC0 AN007 Напряжение смещения для измерителей токов в фазах

    uint16_t smpl_POS_SENS; // ADC1 AN000
    uint16_t smpl_EXT_TEMP; // ADC1 AN001
    uint16_t smpl_MISC;     // ADC1 AN002
    uint16_t smpl_TEMP;     // ADC1 AN005 Температерный сенсор у силовых транзисторов
    uint16_t smpl_V_VPWR;   // ADC1 AN007


    uint16_t smpl_CPU_temp;
    uint16_t smpl_INT_REF_V;

} T_int_adc_res;


extern  T_int_adc_res    int_adc_res;


void ADC_init(void);


float Conv_to_u5v(int32_t v);
float Conv_to_intrefv(int32_t v);
float Conv_to_temp(int32_t v);


int32_t Get_CPU_TEMP(void);
int32_t Get_5V_BUS(void);


float   Value_CPU_temp(int32_t v);
float   Value_5V_bus(int32_t v);

#endif



