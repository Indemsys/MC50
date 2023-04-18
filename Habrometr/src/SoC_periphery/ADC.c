// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.07.12
// 16:33:29
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

// Объявляем прерывания таким образом чтобы это не конфликтовало с методом принятым в библиотеке SSP
void  ADC0_SCAN_END_isr(void);

SSP_VECTOR_DEFINE_CHAN(ADC0_SCAN_END_isr, ADC, SCAN_END, 0);

static IRQn_Type adc0_scan_int_num;

T_int_adc_res  int_adc_res;

/*-----------------------------------------------------------------------------------------------------
  Используем прерывание от ADC0 для обслуживания результатов и от ADC0 и от ADC1

  \param void
-----------------------------------------------------------------------------------------------------*/
void  ADC0_SCAN_END_isr(void)
{
  SF_CONTEXT_SAVE;

  int_adc_res.smpl_V_IU     = R_S12ADC0->ADDRn[0]; // ADC0 AN000
  int_adc_res.smpl_V_IV     = R_S12ADC0->ADDRn[1]; // ADC0 AN001
  int_adc_res.smpl_V_IW     = R_S12ADC0->ADDRn[2]; // ADC0 AN002
  int_adc_res.smpl_V_IPWR   = R_S12ADC0->ADDRn[3]; // ADC0 AN003
  int_adc_res.smpl_VREF_R   = R_S12ADC0->ADDRn[7]; // ADC0 AN007

  int_adc_res.smpl_POS_SENS = R_S12ADC1->ADDRn[0]; // ADC1 AN000
  int_adc_res.smpl_EXT_TEMP = R_S12ADC1->ADDRn[1]; // ADC1 AN001
  int_adc_res.smpl_MISC     = R_S12ADC1->ADDRn[2]; // ADC1 AN002
  int_adc_res.smpl_TEMP     = R_S12ADC1->ADDRn[5]; // ADC1 AN005
  int_adc_res.smpl_V_VPWR   = R_S12ADC1->ADDRn[7]; // ADC1 AN007

  int_adc_res.smpl_CPU_temp     = R_S12ADC0->ADTSDR;
  int_adc_res.smpl_INT_REF_V    = R_S12ADC0->ADOCDR;

  Manual_encoder_processing();

  R_ICU->IELSRn_b[adc0_scan_int_num].IR = 0;  // Сбрасываем IR флаг в ICU

  SF_CONTEXT_RESTORE;
  __DSB();
}

/*-------------------------------------------------------------------------------------------------------------


  Перечень аналоговых сигналов:


  В качестве запускающего тригера используем таймер AGT0
-------------------------------------------------------------------------------------------------------------*/
void ADC_init(void)
{

  R_MSTP->MSTPCRD_b.MSTPD15 = 0; // 12-Bit A/D Converter 1 Module. 0: Cancel the module-stop state
  R_MSTP->MSTPCRD_b.MSTPD16 = 0; // 12-Bit A/D Converter 0 Module. 0: Cancel the module-stop state
  R_MSTP->MSTPCRD_b.MSTPD3  = 0; // AGT0.                          0: Cancel the module-stop state
  R_MSTP->MSTPCRD_b.MSTPD22 = 0; // Temperature Sensor Module.     0: Cancel the module-stop state


  //
  R_S12ADC0->ADCSR = 0
                    + LSHIFT(0, 15) // ADST        | A/D Conversion Start  | 1: Start A/D conversion process.
                    + LSHIFT(0, 13) // ADCS[1:0]   | Scan Mode Select      | 0 0: Single scan mode, 0 1: Group scan mode, 1 0: Continuous scan mode, 1 1: Setting prohibited
                    + LSHIFT(1,  9) // TRGE        | Trigger Start Enable  | 1: Enable A/D conversion to be started by the synchronous or asynchronous trigger.
                    + LSHIFT(0,  8) // EXTRG       | Trigger Select        | 0: Start A/D conversion by a synchronous trigger (ELC), 1: Start A/D conversion by the asynchronous trigger (ADTRGi).
                    + LSHIFT(0,  7) // DBLE        | Double Trigger Mode   | 0: Deselect double-trigger mode, 1: Select double-trigger mode.
                    + LSHIFT(0,  6) // GBADIE      | Group B Scan          | 1: Enable ADC12i_GBADI interrupt generation on Group B scan completion
                    + LSHIFT(0,  0) // DBLANS[4:0] | Double Trigger Channel Select
  ;
  R_S12ADC1->ADCSR = 0
                    + LSHIFT(0, 15) // ADST        | A/D Conversion Start  | 1: Start A/D conversion process.
                    + LSHIFT(0, 13) // ADCS[1:0]   | Scan Mode Select      | 0 0: Single scan mode, 0 1: Group scan mode, 1 0: Continuous scan mode, 1 1: Setting prohibited
                    + LSHIFT(1,  9) // TRGE        | Trigger Start Enable  | 1: Enable A/D conversion to be started by the synchronous or asynchronous trigger.
                    + LSHIFT(0,  8) // EXTRG       | Trigger Select        | 0: Start A/D conversion by a synchronous trigger (ELC), 1: Start A/D conversion by the asynchronous trigger (ADTRGi).
                    + LSHIFT(0,  7) // DBLE        | Double Trigger Mode   | 0: Deselect double-trigger mode, 1: Select double-trigger mode.
                    + LSHIFT(0,  6) // GBADIE      | Group B Scan          | 1: Enable ADC12i_GBADI interrupt generation on Group B scan completion
                    + LSHIFT(0,  0) // DBLANS[4:0] | Double Trigger Channel Select
  ;

  // Устанавливаем путь сигнал минуя PGA
  R_S12ADC0->ADPGACR_b.P000SEL0 = 1;
  R_S12ADC0->ADPGACR_b.P001SEL0 = 1;
  R_S12ADC0->ADPGACR_b.P002SEL0 = 1;
  R_S12ADC0->ADPGACR_b.P003SEL0 = 1;

  R_S12ADC1->ADPGACR_b.P000SEL0 = 1;
  R_S12ADC1->ADPGACR_b.P001SEL0 = 1;
  R_S12ADC1->ADPGACR_b.P002SEL0 = 1;
  R_S12ADC1->ADPGACR_b.P003SEL0 = 1;

  // Отключаем PGA
  R_S12ADC0->ADPGADCR0_b.P000DEN = 0;
  R_S12ADC0->ADPGADCR0_b.P001DEN = 0;
  R_S12ADC0->ADPGADCR0_b.P002DEN = 0;
  R_S12ADC0->ADPGADCR0_b.P003DEN = 0;

  R_S12ADC1->ADPGADCR0_b.P000DEN = 0;
  R_S12ADC1->ADPGADCR0_b.P001DEN = 0;
  R_S12ADC1->ADPGADCR0_b.P002DEN = 0;
  R_S12ADC1->ADPGADCR0_b.P003DEN = 0;


  R_S12ADC0->ADANSA0_b.ANSA00 = 1; //    AN000
  R_S12ADC0->ADSSTR0n_b[0].SST = 0x80; // 128 тактов (2.1 мкс) - время выборки

  R_S12ADC0->ADANSA0_b.ANSA01 = 1; //    AN001
  R_S12ADC0->ADSSTR0n_b[1].SST = 0x80; // 128 тактов (2.1 мкс) - время выборки

  R_S12ADC0->ADANSA0_b.ANSA02 = 1; //    AN002
  R_S12ADC0->ADSSTR0n_b[2].SST = 0x80; // 128 тактов (2.1 мкс) - время выборки

  R_S12ADC0->ADANSA0_b.ANSA03 = 1; //    AN003
  R_S12ADC0->ADSSTR0n_b[3].SST = 0x80; // 128 тактов (2.1 мкс) - время выборки

  R_S12ADC0->ADANSA0_b.ANSA07 = 1; //    AN007
  R_S12ADC0->ADSSTR0n_b[7].SST = 0x80; // 128 тактов (2.1 мкс) - время выборки


  R_S12ADC1->ADANSA0_b.ANSA00 = 1; //    AN100
  R_S12ADC1->ADSSTR0n_b[0].SST = 0x80; // 128 тактов (2.1 мкс) - время выборки

  R_S12ADC1->ADANSA0_b.ANSA01 = 1; //    AN101
  R_S12ADC1->ADSSTR0n_b[1].SST = 0x80; // 128 тактов (2.1 мкс) - время выборки

  R_S12ADC1->ADANSA0_b.ANSA02 = 1; //    AN102
  R_S12ADC1->ADSSTR0n_b[2].SST = 0x80; // 128 тактов (2.1 мкс) - время выборки

  R_S12ADC1->ADANSA0_b.ANSA05 = 1; //    AN105
  R_S12ADC1->ADSSTR0n_b[5].SST = 0x80; // 128 тактов (2.1 мкс) - время выборки

  R_S12ADC1->ADANSA0_b.ANSA07 = 1; //    AN107
  R_S12ADC1->ADSSTR0n_b[7].SST = 0x80; // 128 тактов (2.1 мкс) - время выборки


  R_S12ADC0->ADSSTRT_b.SST = 0x80; // 128 тактов (2.1 мкс) - время выборки температурного сенсора
  R_S12ADC0->ADSSTRO_b.SST = 0x80; // 128 тактов (2.1 мкс) - время выборки референсного напряжения


  //
  R_S12ADC0->ADCER = 0
                    + LSHIFT(0, 15) // ADRFMT       | A/D Data Register Format Select             | 0: Select flush-right for the A/D data register format, 1: Select flush-left for the A/D data register format.
                    + LSHIFT(0, 11) // DIAGM        | Self-Diagnosis Enable                       | 0: Disable ADC12 self-diagnosis, 1: Enable ADC12 self-diagnosis.
                    + LSHIFT(0, 10) // DIAGLD       | Self-Diagnosis Mode Select                  | 0: Select rotation mode for self-diagnosis voltage, 1: Select fixed mode for self-diagnosis voltage.
                    + LSHIFT(0,  8) // DIAGVAL[1:0] | Self-Diagnosis Conversion Voltage Select    | 0 0: Setting prohibited when self-diagnosis is enabled, 0 1: 0 V, 1 0: Reference power supply voltage x 1/2, 1 1: Reference power supply voltage.
                    + LSHIFT(0,  5) // ACE          | A/D Data Register Automatic Clearing Enable | 1: Enable automatic clearing.
                    + LSHIFT(0,  1) // ADPRC[1:0]   | A/D Conversion Accuracy Specify             | 0 0: 12-bit accuracy
  ;

  R_S12ADC1->ADCER = 0
                    + LSHIFT(0, 15) // ADRFMT       | A/D Data Register Format Select             | 0: Select flush-right for the A/D data register format, 1: Select flush-left for the A/D data register format.
                    + LSHIFT(0, 11) // DIAGM        | Self-Diagnosis Enable                       | 0: Disable ADC12 self-diagnosis, 1: Enable ADC12 self-diagnosis.
                    + LSHIFT(0, 10) // DIAGLD       | Self-Diagnosis Mode Select                  | 0: Select rotation mode for self-diagnosis voltage, 1: Select fixed mode for self-diagnosis voltage.
                    + LSHIFT(0,  8) // DIAGVAL[1:0] | Self-Diagnosis Conversion Voltage Select    | 0 0: Setting prohibited when self-diagnosis is enabled, 0 1: 0 V, 1 0: Reference power supply voltage x 1/2, 1 1: Reference power supply voltage.
                    + LSHIFT(0,  5) // ACE          | A/D Data Register Automatic Clearing Enable | 1: Enable automatic clearing.
                    + LSHIFT(0,  1) // ADPRC[1:0]   | A/D Conversion Accuracy Specify             | 0 0: 12-bit accuracy
  ;


  //
  R_S12ADC0->ADEXICR = 0
                      + LSHIFT(0, 11) // OCSB  | Internal Reference Voltage A/D Conversion Select for Group B               | 1: Enable A/D conversion of temperature sensor output.
                      + LSHIFT(0, 10) // TSSB  | Temperature Sensor Output A/D Conversion Select for Group B                | 1: Enable A/D conversion of temperature sensor output.
                      + LSHIFT(1,  9) // OCSA  | Internal Reference Voltage A/D Conversion Select                           | 1: Enable A/D conversion of internal reference voltage.
                      + LSHIFT(1,  8) // TSSA  | Temperature Sensor Output A/D Conversion Select                            | 1: Enable A/D conversion of temperature sensor output.
                      + LSHIFT(0,  1) // OCSAD | Internal Reference Voltage A/DConverted Value Addition/Average Mode Select | 1: Select addition/average mode for internal reference voltage.
                      + LSHIFT(0,  0) // TSSAD | Temperature Sensor Output A/D Converted Value Addition/Average Mode Select | 1: Select addition/average mode for temperature sensor output.
  ;

  //
  R_S12ADC1->ADEXICR = 0
                      + LSHIFT(0, 11) // OCSB  | Internal Reference Voltage A/D Conversion Select for Group B               | 1: Enable A/D conversion of temperature sensor output.
                      + LSHIFT(0, 10) // TSSB  | Temperature Sensor Output A/D Conversion Select for Group B                | 1: Enable A/D conversion of temperature sensor output.
                      + LSHIFT(0,  9) // OCSA  | Internal Reference Voltage A/D Conversion Select                           | 1: Enable A/D conversion of internal reference voltage.
                      + LSHIFT(0,  8) // TSSA  | Temperature Sensor Output A/D Conversion Select                            | 1: Enable A/D conversion of temperature sensor output.
                      + LSHIFT(0,  1) // OCSAD | Internal Reference Voltage A/DConverted Value Addition/Average Mode Select | 1: Select addition/average mode for internal reference voltage.
                      + LSHIFT(0,  0) // TSSAD | Temperature Sensor Output A/D Converted Value Addition/Average Mode Select | 1: Select addition/average mode for temperature sensor output.
  ;

  R_TSN->TSCR_b.TSEN = 1; // Включаем температурный сенсор
  Wait_ms(1);             // Задержка для стабилизации сенсора
  R_TSN->TSCR_b.TSOE = 1; // Включаем выход температурного сенсора


  // Для ADC0 для группы А устанвливаем запускающий триггер
  R_S12ADC0->ADSTRGR_b.TRSA = 0x09; // Триггером будут служить сигналы  ELC_PERIPHERAL_ADC0 и ELC_PERIPHERAL_ADC1 от модуля Event Link Controller
  // Для ADC1 для группы А устанвливаем запускающий триггер
  R_S12ADC1->ADSTRGR_b.TRSA = 0x09; // Триггером будут служить сигналы  ELC_PERIPHERAL_ADC0 и ELC_PERIPHERAL_ADC1 от модуля Event Link Controller

  // Программируем Event Link Setting Register (ELSRn) где указаываем какие события от сторонней периферии будут направлены на триггеры ADC
  R_ELC->ELSRnRC0[ELC_PERIPHERAL_ADC0].ELSRn = ELC_EVENT_AGT0_INT; // Направляем сигнал с AGT0 на вход тригера ADC0
  R_ELC->ELSRnRC0[ELC_PERIPHERAL_ADC1].ELSRn = ELC_EVENT_AGT0_INT; // Направляем сигнал с AGT0 на вход тригера ADC1


  adc0_scan_int_num = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_ADC0_SCAN_END);
  NVIC_SetPriority(adc0_scan_int_num, INT_ADC_SCAN_PRIO);

  R_ICU->IELSRn_b[adc0_scan_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  NVIC_ClearPendingIRQ(adc0_scan_int_num);
  NVIC_EnableIRQ(adc0_scan_int_num);


  // Инициализируем таймер AGT0
  Init_AGT0_ticks_to_ADC(ADC_SMPL_RATE_DIV);


}

/*-----------------------------------------------------------------------------------------------------


  \param v

  \return float
-----------------------------------------------------------------------------------------------------*/
float Conv_to_u5v(int32_t v)
{
  float f;

  f = (float)v*((VOLTAGE_3V3/ADC_RANGE)/U5V_SCALE) ;
  return f;
}


/*-----------------------------------------------------------------------------------------------------


  \param v

  \return float
-----------------------------------------------------------------------------------------------------*/
float Conv_to_intrefv(int32_t v)
{
  float f;

  f = (float)v*(VOLTAGE_3V3/ADC_RANGE) ;

  return f;
}

/*-----------------------------------------------------------------------------------------------------


  \param v

  \return float
-----------------------------------------------------------------------------------------------------*/
float Conv_to_temp(int32_t v)
{
  float f;

  f = (float)v*(VOLTAGE_3V3/ADC_RANGE) ;

  f = f - 1.24f; // Находим разницу по сравнению с напряжением при температуре 25 С
  f = f/0.0041f; // Находим дельту температуры
  f = f + 25.0f; // Находим температуру

  return f;
}


int32_t Get_CPU_TEMP   (void)         { return (int32_t)int_adc_res.smpl_CPU_temp;    }
int32_t Get_5V_BUS     (void)         { return (int32_t)int_adc_res.smpl_VREF_R;       }


float   Value_CPU_temp(int32_t v)     { return Conv_to_temp(v); }
float   Value_5V_bus(int32_t v)       { return Conv_to_u5v(v); }


