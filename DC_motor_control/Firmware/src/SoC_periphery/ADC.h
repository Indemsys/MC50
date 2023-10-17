#ifndef ADC_H
  #define ADC_H

typedef void (*T_ADC_isr_callback)(void);

typedef struct
{
    uint16_t smpl_V_IU;        // ADC0 AN000
    uint16_t smpl_V_IV;        // ADC0 AN001
    uint16_t smpl_V_IW;        // ADC0 AN002
    uint16_t smpl_V_IPWR;      // ADC0 AN003
    uint16_t smpl_VREF_R;      // ADC0 AN007 Напряжение смещения для измерителей токов в фазах

    uint16_t smpl_POS_SENS;    // ADC1 AN000
    uint16_t smpl_EXT_TEMP;    // ADC1 AN001
    uint16_t smpl_MISC;        // ADC1 AN002
    uint16_t smpl_TEMP;        // ADC1 AN005 Температерный сенсор у силовых транзисторов
    uint16_t smpl_V_VPWR;      // ADC1 AN007

    uint8_t  skip_adc_res;

    float    V_IPWR_fltr;
    float    V_VPWR_fltr;

    uint16_t smpl_CPU_temp;
    uint16_t smpl_INT_REF_V;

    uint32_t smpl_acc_cnt;       // Счетчик аккумулирования результатов
    uint32_t smpl_V_IU_acc;      // Аккумулированные результаты
    uint32_t smpl_V_IV_acc;      //
    uint32_t smpl_V_IW_acc;      //
    uint32_t smpl_VREF_R_acc;    //

    int32_t  smpl_V_IU_offs;     // Корректировочные смещения для сэмплов с датчиков тока в фазах мотора
    int32_t  smpl_V_IV_offs;     //
    int32_t  smpl_V_IW_offs;     //
    float    smpl_VREF_R_aver;   //

    int32_t  smpl_V_IU_norm;     // Откорректированные сэмплы с датчиков тока в фазах мотора
    int32_t  smpl_V_IV_norm;     //
    int32_t  smpl_V_IW_norm;     //


    float    instant_bldc_motor_current;     //
    float    filtered_bldc_motor_current;

    float    instant_dc_motor_current;      //
    float    filtered_dc_motor_current;

    float    i_u;                    // Ток в фазе U
    float    i_v;                    // Ток в фазе V
    float    i_w;                    // Ток в фазе W

    float    adc_scale;              // Коэффициент приведения отсчетов АЦП к значению напряжения на входе АЦП
    float    current_shunt_scale;    // Коэффициент приведения напряжения к току через измерительный шунт
    float    current_pwr_scale;      // Коэффициент приведения напряжения к току через шину питания
    float    voltage_pwr_scale;
    float    i_pwr;                  // Ток в шине питания
    float    pos_sens;               // Напряжение на сенсоре позиции
    float    ext_temp;               // Температура внешнего датчика
    float    misc;                   // Напряжение на входе misc
    float    drv_temp;               // Температура платы
    float    v_pwr;                  // Напряжение источника питания
    float    cpu_temp;               // Температура микроконтроллера
    float    input_pwr;              // Потреблемая устройством мощность

    int32_t  prev_smpl_POS_SENS;
    float    shaft_speed_rt;            // Скорость вращения выходного вала.  Скорость имеет знак. Выражается в градусах в секунду
                                     // Знак + означает движение в сторону открывания, если  открываестя по часовой стрелке при взгляде сверху
                                     // знак - означает движение в сторону закрывания, если  открываестя по часовой стрелке при взгляде сверху
    float    shaft_abs_speed_rt;     // Беззнаковая скорость вращения выходного вала
    float    shaft_speed_smoothly;   // Скорость вращения выходного вала.  Скорость имеет знак. Выражается в градусах в секунду
    float    shaft_abs_speed_smoothly;// Беззнаковая скорость вращения выходного вала

    float    shaft_position_grad;    // Позиция выходного вала относительно нулевой позиции в градусах

    float    samples_to_grad_scale;  // Коэффициент приведения угла поворота выходного вала из отсчетов АЦП в градусы
    float    grad_to_samples_scale;  // Коэффициент приведения угла поворота выходного вала из градусов в отсчеты АЦП
    float    samples_ps_to_grad_ps;  // Коэффициент приведения скорости выходного вала из отсчетов АЦП в секунду в градусы в секунду


    IRQn_Type             adc0_scan_int_num;
    TX_EVENT_FLAGS_GROUP  adc_flags;

    T_ADC_isr_callback    ISR_callback;

} T_adc_cbl;


extern  T_adc_cbl        adc;


void       ADC_init(T_ADC_isr_callback isr_callback);
uint32_t   ADC_set_averaging_done(void);
uint32_t   ADC_wait_averaging_done(void);

#endif



