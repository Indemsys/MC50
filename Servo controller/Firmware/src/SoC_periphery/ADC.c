// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.07.12
// 16:33:29
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

// Объявляем прерывания таким образом чтобы это не конфликтовало с методом принятым в библиотеке SSP
void  ADC0_SCAN_END_isr(void);

SSP_VECTOR_DEFINE_CHAN(ADC0_SCAN_END_isr, ADC, SCAN_END, 0);

#define   ADC_AVERAGING_DONE   BIT(0)


T_adc_cbl         adc;


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static inline void ADC_sampling_data_collection(void)
{
  if (g_sys_ready != 0)
  {

    // Время выполнения 1.59 мкс
    if (adc.skip_adc_res == 0)
    {
      adc.prev_smpl_POS_SENS = adc.smpl_POS_SENS;
      // Сохраняем в рабочие переменные результаты работы АЦП
      adc.smpl_V_IU     = R_S12ADC0->ADDRn[0]; // ADC0 AN000
      adc.smpl_V_IV     = R_S12ADC0->ADDRn[1]; // ADC0 AN001
      adc.smpl_V_IW     = R_S12ADC0->ADDRn[2]; // ADC0 AN002
      adc.smpl_V_IPWR   = R_S12ADC0->ADDRn[3]; // ADC0 AN003
      adc.smpl_VREF_R   = R_S12ADC0->ADDRn[7]; // ADC0 AN007

      adc.smpl_POS_SENS = R_S12ADC1->ADDRn[0]; // ADC1 AN000
      adc.smpl_EXT_TEMP = R_S12ADC1->ADDRn[1]; // ADC1 AN001
      adc.smpl_MISC     = R_S12ADC1->ADDRn[2]; // ADC1 AN002
      adc.smpl_TEMP     = R_S12ADC1->ADDRn[5]; // ADC1 AN005
      adc.smpl_V_VPWR   = R_S12ADC1->ADDRn[7]; // ADC1 AN007

      adc.smpl_CPU_temp     = R_S12ADC0->ADTSDR;
      adc.smpl_INT_REF_V    = R_S12ADC0->ADOCDR;


      // Накопление результатов если производится калибровка усилителей датчиков тока в фазах мотора
      if (adc.smpl_acc_cnt != 0)
      {
        adc.smpl_V_IU_acc   += adc.smpl_V_IU;
        adc.smpl_V_IV_acc   += adc.smpl_V_IV;
        adc.smpl_V_IW_acc   += adc.smpl_V_IW;
        adc.smpl_VREF_R_acc += adc.smpl_VREF_R;
        adc.smpl_acc_cnt--;
        if (adc.smpl_acc_cnt == 0)
        {
          ADC_set_averaging_done();
        }
      }
    }
    else
    {
      // Здесь если игнорируем некоторые выборки в которых вероятно присутствие сильных помех
      adc.skip_adc_res--;
    }
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ADC_set_averaging_done(void)
{
  return  tx_event_flags_set(&adc.adc_flags, ADC_AVERAGING_DONE, TX_OR);
}
/*-----------------------------------------------------------------------------------------------------
  Ожидаем окончания накопления результатов для усреднения

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ADC_wait_averaging_done(void)
{
  ULONG     actual_events;
  return tx_event_flags_get(&adc.adc_flags, ADC_AVERAGING_DONE , TX_OR_CLEAR,&actual_events, MS_TO_TICKS(1000));
}


/*-----------------------------------------------------------------------------------------------------
  Используем прерывание от ADC0 для обслуживания результатов и от ADC0 и от ADC1
  Частота вызова равна частоте ШИМ мотора и равна PWM_FREQ_HZ


  \param void
-----------------------------------------------------------------------------------------------------*/
void  ADC0_SCAN_END_isr(void)
{
  ADC_sampling_data_collection();

  if (adc.ISR_callback) adc.ISR_callback();

  FMSTR_Recorder(0);                          // 6.36 us Вызываем функцию записи сигнала для инструмента FreeMaster.
  R_ICU->IELSRn_b[adc.adc0_scan_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
}

/*-------------------------------------------------------------------------------------------------------------
  Инициализация ADC для работы синхронно с ШИМ управления моторм

  В качестве запускающего тригера используем сигнал с копмпаратора GTADTRA GPT0
-------------------------------------------------------------------------------------------------------------*/
void ADC_init(T_ADC_isr_callback isr_callback)
{

  R_MSTP->MSTPCRD_b.MSTPD15 = 0; // 12-Bit A/D Converter 1 Module. 0: Cancel the module-stop state
  R_MSTP->MSTPCRD_b.MSTPD16 = 0; // 12-Bit A/D Converter 0 Module. 0: Cancel the module-stop state
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

  // 1 такт ADC = 16.7 ns
  R_S12ADC0->ADANSA0_b.ANSA00  = 1;    // AN000  V_IU   - измеритель тока в фазе U
  R_S12ADC0->ADSSTR0n_b[0].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки

  R_S12ADC0->ADANSA0_b.ANSA01  = 1;    // AN001  V_IV   - измеритель тока в фазе V
  R_S12ADC0->ADSSTR0n_b[1].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки

  R_S12ADC0->ADANSA0_b.ANSA02  = 1;    // AN002  V_IW   - измеритель тока в фазе W
  R_S12ADC0->ADSSTR0n_b[2].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки

  R_S12ADC0->ADANSA0_b.ANSA03  = 1;    // AN003  V_IPWR - измеритель тока в шине питания
  R_S12ADC0->ADSSTR0n_b[3].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки

  R_S12ADC0->ADANSA0_b.ANSA07  = 1;    // AN007  VREF_R - измеритель референсного напряжения
  R_S12ADC0->ADSSTR0n_b[7].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки


  R_S12ADC1->ADANSA0_b.ANSA00  = 1;    // AN100  POS_SENS - изменитель напряжения на сенсоре позиции
  R_S12ADC1->ADSSTR0n_b[0].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки

  R_S12ADC1->ADANSA0_b.ANSA01  = 1;    // AN101  EXT_TEMP - изменитель напряжения на внешнем сенсоре температуры
  R_S12ADC1->ADSSTR0n_b[1].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки

  R_S12ADC1->ADANSA0_b.ANSA02  = 1;    // AN102  MISC     - измеритель без назначенной функции
  R_S12ADC1->ADSSTR0n_b[2].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки

  R_S12ADC1->ADANSA0_b.ANSA05  = 1;    // AN105  TEMP     - измеритель температуры у силового драйвера
  R_S12ADC1->ADSSTR0n_b[5].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки

  R_S12ADC1->ADANSA0_b.ANSA07  = 1;    // AN107  V_VPWR   - измениитель напряжения на шине питания
  R_S12ADC1->ADSSTR0n_b[7].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки


  R_S12ADC0->ADSSTRT_b.SST     = 0x40; // 64 тактов (1.05 мкс) - время выборки температурного сенсора
  R_S12ADC0->ADSSTRO_b.SST     = 0x40; // 64 тактов (1.05 мкс) - время выборки референсного напряжения

  // Здесь настраиваем одновременную выборку для каналов 0 1 2 ADC в обоих модулях ADC
  // В этих каналах у ADC0 производится измерение токов в фазах мотора,
  // и одновременная выборка исключает неточность вызванную неодновременностью измерения токов
  R_S12ADC0->ADSHCR_b.SSTSH = 0x20; // 32 тактов (0.334 мкс) - время выборки
  R_S12ADC0->ADSHCR_b.SHANS0 = 1;   // AN000 sample-and-hold circuit Select
  R_S12ADC0->ADSHCR_b.SHANS1 = 1;   // AN001 sample-and-hold circuit Select
  R_S12ADC0->ADSHCR_b.SHANS2 = 1;   // AN002 sample-and-hold circuit Select

  R_S12ADC1->ADSHCR_b.SSTSH = 0x20; // 32 тактов (0.334 мкс) - время выборки
  R_S12ADC1->ADSHCR_b.SHANS0 = 1;   // AN100 sample-and-hold circuit Select
  R_S12ADC1->ADSHCR_b.SHANS1 = 1;   // AN101 sample-and-hold circuit Select
  R_S12ADC1->ADSHCR_b.SHANS2 = 1;   // AN102 sample-and-hold circuit Select

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
  R_S12ADC0->ADSTRGR_b.TRSA = 0x0B; // Триггером будут служить сигналы  ELC_PERIPHERAL_ADC0 и ELC_PERIPHERAL_ADC1 от модуля Event Link Controller
  // Для ADC1 для группы А устанвливаем запускающий триггер
  R_S12ADC1->ADSTRGR_b.TRSA = 0x0B; // Триггером будут служить сигналы  ELC_PERIPHERAL_ADC0 и ELC_PERIPHERAL_ADC1 от модуля Event Link Controller

  // Программируем Event Link Setting Register (ELSRn), где указываем какие события от сторонней периферии будут направлены на триггеры ADC
  R_ELC->ELSRnRC0[ELC_PERIPHERAL_ADC0  ].ELSRn = ELC_EVENT_GPT0_AD_TRIG_A; // Направляем сигнал с канала A компаратора GPT0 на вход тригера ADC0
  R_ELC->ELSRnRC0[ELC_PERIPHERAL_ADC0_B].ELSRn = ELC_EVENT_GPT0_AD_TRIG_B; // Направляем сигнал с канала A компаратора GPT0 на вход тригера ADC0
  R_ELC->ELSRnRC0[ELC_PERIPHERAL_ADC1  ].ELSRn = ELC_EVENT_GPT0_AD_TRIG_A; // Направляем сигнал с канала A компаратора GPT0 на вход тригера ADC1
  R_ELC->ELSRnRC0[ELC_PERIPHERAL_ADC1_B].ELSRn = ELC_EVENT_GPT0_AD_TRIG_B; // Направляем сигнал с канала A компаратора GPT0 на вход тригера ADC1


  tx_event_flags_create(&adc.adc_flags, "adc_flags");

  adc.ISR_callback = isr_callback;
  adc.adc0_scan_int_num = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_ADC0_SCAN_END);
  NVIC_SetPriority(adc.adc0_scan_int_num, INT_ADC_SCAN_PRIO);

  R_ICU->IELSRn_b[adc.adc0_scan_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  NVIC_ClearPendingIRQ(adc.adc0_scan_int_num);
  NVIC_EnableIRQ(adc.adc0_scan_int_num);

}

/*-----------------------------------------------------------------------------------------------------


  \param isr_callback
-----------------------------------------------------------------------------------------------------*/
void ADC_set_callback(T_ADC_isr_callback isr_callback)
{
  __disable_interrupt();
  adc.ISR_callback = isr_callback;
  __enable_interrupt();
}



