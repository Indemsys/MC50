// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.07.12
// 16:33:29
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

void FMSTR_Recorder(FMSTR_INDEX recIndex);

// Объявляем прерывания таким образом чтобы это не конфликтовало с методом принятым в библиотеке SSP
void  ADC0_SCAN_END_isr(void);

SSP_VECTOR_DEFINE_CHAN(ADC0_SCAN_END_isr, ADC, SCAN_END, 0);

TX_EVENT_FLAGS_GROUP           adc_flags;
#define   ADC_AVERAGING_DONE   BIT(0)


static IRQn_Type adc0_scan_int_num;

T_adc_cbl  adc;

uint8_t   current_hall_state;  // Глобальная переменная содержащая маску состояния битов сигналов с датчиков Холла.
                               // Применяется для 6-step коммутации

uint8_t   hall_u;
uint8_t   hall_v;
uint8_t   hall_w;
uint8_t   hall_state;
uint8_t   skip_adc_res;

T_median_filter_uint32  flt_speed;

int32_t  hall_u_prev_arr[8];
int32_t  hall_v_prev_arr[8];
int32_t  hall_w_prev_arr[8];

int32_t  hall_u_capt_arr[8];
int32_t  hall_v_capt_arr[8];
int32_t  hall_w_capt_arr[8];

uint32_t hall_u_cnt;
uint32_t hall_v_cnt;
uint32_t hall_w_cnt;

int32_t  hall_u_capt;
int32_t  hall_v_capt;
int32_t  hall_w_capt;

// Отладочные переменные
//uint32_t hall_capt;
//uint32_t hall_capt_prev;

uint32_t one_turn_period;
int32_t  rotating_direction;


/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void Reset_rotor_speed_detector(void)
{
  for (uint32_t i=0;i<8;i++)
  {
    hall_u_prev_arr[i]= -1;
    hall_v_prev_arr[i]= -1;
    hall_w_prev_arr[i]= -1;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Измерение скорости вращения ротора и направления вращения

-----------------------------------------------------------------------------------------------------*/
static void Measure_rotor_speed_and_direction(void)
{
  uint32_t  st;
  int32_t  val;
  uint8_t   h;

  st = R_GPTB3->GTST;  // Читаем флаги состояния capture таймера  GPT3211

  if (st & BIT(0))     // Проверяем флаг TCFA. Input Capture/Compare Match Flag A
  {
    // Здесь если событие захвата произошло

    R_GPTB3->GTST = 0;      // Очищаем флаг захвата и остальные флаги.
                            // За другие флаги захвата не беспокоимся, поскольку они при штатной работе не могут быть взведены в этот момент.

    val = R_GPTB3->GTCCRA;  // Читаем регистр с захваченным состоянием счетчика

    // Здесь рассчитываем разницу по отношению к предыдущему захваченному значению на том же угле поворота ротора, т.е. для полюса с номером в переменной hall_u_cnt
    if (hall_u_prev_arr[hall_u_cnt] > 0)
    {
      if (val > hall_u_prev_arr[hall_u_cnt])
      {
        hall_u_capt_arr[hall_u_cnt] = val - hall_u_prev_arr[hall_u_cnt];
      }
      else
      {
        // Корректируем если было произошло переполнение счетчика между захватами
        hall_u_capt_arr[hall_u_cnt] =  0x7FFFFFFF -(hall_u_prev_arr[hall_u_cnt] - val -1);
      }

      hall_u_capt = hall_u_capt_arr[hall_u_cnt]; // Сохраняем разницу в промежуточную переменную. Эта переменная используется при отладке через FreeMaster
      // .......................................................
      one_turn_period = hall_u_capt;             // Записываем длительность полного оборота ротора в глобальную переменную для дальнейшего использования остальными задачами
      // .......................................................
    }
    hall_u_prev_arr[hall_u_cnt] = val;         // Сохраняем текущее захваченное значение в переменную предыдущего значения
    hall_u_cnt++;                              // Ведем счет полюсов. Для каждого полюса сохраняется своя измеренная величина
    if (hall_u_cnt >= 8) hall_u_cnt = 0;

    // Определяем направление вращения
    h = R_IOPORT5->PCNTR2 & 0x7;  // Читаем сигналы датчиков Холла здесь снова, несмотря на то что они уже были прочитаны в обработчике прерывания
                                  // Это нужно поскольку capture логика может сработать уже после того как в ISR были прочитаны состояния датчиков
    if ((h == 0b100) || (h == 0b011))      rotating_direction = 1; // Направление вращения определяем по паттернам сигнало с датчиков сразу после текущего фронта
    else if ((h == 0b010) || (h == 0b101)) rotating_direction = -1;

    R_GPTB5->GTCLR_b.CCLR13 = 1;  // Сброс счетчика отслеживающего остановку вращения
    R_GPTB5->GTST           = 0;  // Сброс флагов счетчика отслеживающего остановку вращения
  }

  if (st & BIT(1))
  {
    // Input capture/compare match of GTCCRB occurred
    R_GPTB3->GTST = 0;
    val = R_GPTB3->GTCCRB;
    if (hall_v_prev_arr[hall_v_cnt]>0)
    {
      if (val > hall_v_prev_arr[hall_v_cnt])
      {
        hall_v_capt_arr[hall_v_cnt] = val - hall_v_prev_arr[hall_v_cnt];
      }
      else
      {
        hall_v_capt_arr[hall_v_cnt] =  0x7FFFFFFF -(hall_v_prev_arr[hall_v_cnt] - val -1);
      }
      hall_v_capt = hall_v_capt_arr[hall_v_cnt];
      one_turn_period = hall_v_capt;
    }
    hall_v_prev_arr[hall_v_cnt] = val;
    hall_v_cnt++;
    if (hall_v_cnt >= 8) hall_v_cnt = 0;

    h = R_IOPORT5->PCNTR2 & 0x7;  // Читаем сигналы датчиков Холла здесь чтобы они были прочитаны не раньше чем стработает capture логика
    if ((h == 0b110) || (h == 0b001))      rotating_direction = 1;
    else if ((h == 0b011) || (h == 0b100)) rotating_direction = -1;

    R_GPTB5->GTCLR_b.CCLR13 = 1;  // Сброс счетчика отслеживающего остановку вращения
    R_GPTB5->GTST           = 0;
  }


  st = R_GPTB4->GTST;

  if (st & BIT(0))
  {
    // Input capture/compare match of GTCCRA occurred
    R_GPTB4->GTST = 0;
    val = R_GPTB4->GTCCRA;
    if (hall_w_prev_arr[hall_w_cnt]>0)
    {
      if (val > hall_w_prev_arr[hall_w_cnt])
      {
        hall_w_capt_arr[hall_w_cnt] = val - hall_w_prev_arr[hall_w_cnt];
      }
      else
      {
        hall_w_capt_arr[hall_w_cnt] =  0x7FFFFFFF -(hall_w_prev_arr[hall_w_cnt] - val -1);
      }
      hall_w_capt = hall_w_capt_arr[hall_w_cnt];
      one_turn_period = hall_w_capt;
    }
    hall_w_prev_arr[hall_w_cnt] = val;
    hall_w_cnt++;
    if (hall_w_cnt >= 8) hall_w_cnt = 0;

    h = R_IOPORT5->PCNTR2 & 0x7;  // Читаем сигналы датчиков Холла здесь чтобы они были прочитаны не раньше чем стработает capture логика
    if ((h == 0b101) || (h == 0b010))      rotating_direction = 1;
    else if ((h == 0b110) || (h == 0b001)) rotating_direction = -1;

    R_GPTB5->GTCLR_b.CCLR13 = 1;  // Сброс счетчика отслеживающего остановку вращения
    R_GPTB5->GTST           = 0;
  }

  // Блок линейно понижающий скорость в случает отсутствия сигналов с датчиков Холла
  if (one_turn_period != 0)
  {
    if (R_GPTB5->GTST & BIT(6))  // Проверяем флаг TCFPO. Overflow Flag
    {
      // В случае переполения тамера сразу отмечаем скорость как нулевую.
      // Поскольку переполение таймера происходит с периодом в две секунды, то такую низкую скрость принимаем как нулевую
      one_turn_period = 0;
    }
    else
    {
      // За один оброт мотора мы имеем 8 полюсов * 3 датчика = 24 события сброса таймера R_GPTB5
      // И если таймер R_GPTB5 смог набрать 2/3 (16/24) длительности одного оборота и не был сброшен значит скорость упала
      // и можно начинать снижать оценку текущей скорости
      uint32_t no_edge_period = R_GPTB5->GTCNT*16;
      if (no_edge_period > one_turn_period)
      {
        one_turn_period = no_edge_period;
      }
    }
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ADC_set_averaging_done(void)
{
  return  tx_event_flags_set(&adc_flags, ADC_AVERAGING_DONE, TX_OR);
}
/*-----------------------------------------------------------------------------------------------------
  Ожидаем окончания накопления результатов для усреднения

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ADC_wait_averaging_done(void)
{
  ULONG     actual_events;
  return tx_event_flags_get(&adc_flags, ADC_AVERAGING_DONE , TX_OR_CLEAR,&actual_events, MS_TO_TICKS(1000));
}


/*-----------------------------------------------------------------------------------------------------
  Используем прерывание от ADC0 для обслуживания результатов и от ADC0 и от ADC1
  Частота вызова равна частоте ШИМ мотора и равна PWM_FREQ_HZ

  При максимальной оптимизации функция выпоняется от 5...25 мкс

  \param void
-----------------------------------------------------------------------------------------------------*/
void  ADC0_SCAN_END_isr(void)
{
  SF_CONTEXT_SAVE;

  ITM_EVENT8(1,0);

  // Время выполнения 1.59 мкс
  if (skip_adc_res == 0)
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
    skip_adc_res--;
  }

  current_hall_state = R_IOPORT5->PCNTR2 & 0x7;

  if ((current_hall_state == 0) || (current_hall_state == 0x7))
  {
    g_app_errors |= APP_ERR_HALL_STATE_FAULT;
  }

  // Устаналиваем переменные применяемы при отладке через FreeMaster
  hall_u =(current_hall_state >> 0) & 1;
  hall_v =(current_hall_state >> 1) & 1;
  hall_w =(current_hall_state >> 2) & 1;

  ITM_EVENT8(1,1);
  Measure_instant_current();           // 2.30 us
  ITM_EVENT8(1,2);
  Measure_servo_sensor_speed();        // 1.46 us
  ITM_EVENT8(1,3);
  Measure_rotor_speed_and_direction(); // 0.72 us
  ITM_EVENT8(1,4);
  MC_speed_loop_PID_step();            //
  ITM_EVENT8(1,5);
  MC_current_loop_PID_step();          // 2.64 us
  ITM_EVENT8(1,6);
  MC_do_6_step_commutation();          // 0.80 us
  ITM_EVENT8(1,7);
  Manual_encoder_processing();         // 1.98 us Обрабатываем сигналы с ручного энкодера
  ITM_EVENT8(1,8);
  FMSTR_Recorder(0);                   // 6.36 us Вызываем функцию записи сигнала для инструмента FreeMaster.

  R_ICU->IELSRn_b[adc0_scan_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  ITM_EVENT8(1,9);
  SF_CONTEXT_RESTORE;
  __DSB();
}

/*-------------------------------------------------------------------------------------------------------------
  Инициализация ADC для работы синхронно с ШИМ управления моторм

  В качестве запускающего тригера используем сигнал с копмпаратора GTADTRA GPT0
-------------------------------------------------------------------------------------------------------------*/
void ADC_init(void)
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
  R_S12ADC0->ADANSA0_b.ANSA00 = 1; //    AN000  V_IU   - измеритель тока в фазе U
  R_S12ADC0->ADSSTR0n_b[0].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки

  R_S12ADC0->ADANSA0_b.ANSA01 = 1; //    AN001  V_IV   - измеритель тока в фазе V
  R_S12ADC0->ADSSTR0n_b[1].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки

  R_S12ADC0->ADANSA0_b.ANSA02 = 1; //    AN002  V_IW   - измеритель тока в фазе W
  R_S12ADC0->ADSSTR0n_b[2].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки

  R_S12ADC0->ADANSA0_b.ANSA03 = 1; //    AN003  V_IPWR - измеритель тока в шине питания
  R_S12ADC0->ADSSTR0n_b[3].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки

  R_S12ADC0->ADANSA0_b.ANSA07 = 1; //    AN007  VREF_R - измеритель референсного напряжения
  R_S12ADC0->ADSSTR0n_b[7].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки


  R_S12ADC1->ADANSA0_b.ANSA00 = 1; //    AN100  POS_SENS - изменитель напряжения на сенсоре позиции
  R_S12ADC1->ADSSTR0n_b[0].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки

  R_S12ADC1->ADANSA0_b.ANSA01 = 1; //    AN101  EXT_TEMP - изменитель напряжения на внешнем сенсоре температуры
  R_S12ADC1->ADSSTR0n_b[1].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки

  R_S12ADC1->ADANSA0_b.ANSA02 = 1; //    AN102  MISC     - измеритель без назначенной функции
  R_S12ADC1->ADSSTR0n_b[2].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки

  R_S12ADC1->ADANSA0_b.ANSA05 = 1; //    AN105  TEMP     - измеритель температуры у силового драйвера
  R_S12ADC1->ADSSTR0n_b[5].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки

  R_S12ADC1->ADANSA0_b.ANSA07 = 1; //    AN107  V_VPWR   - измениитель напряжения на шине питания
  R_S12ADC1->ADSSTR0n_b[7].SST = 0x20; // 32 тактов (0.334 мкс) - время выборки


  R_S12ADC0->ADSSTRT_b.SST = 0xFF; // 32 тактов (4.2 мкс) - время выборки температурного сенсора
  R_S12ADC0->ADSSTRO_b.SST = 0xFF; // 32 тактов (4.2 мкс) - время выборки референсного напряжения

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
  R_S12ADC0->ADSTRGR_b.TRSA = 0x09; // Триггером будут служить сигналы  ELC_PERIPHERAL_ADC0 и ELC_PERIPHERAL_ADC1 от модуля Event Link Controller
  // Для ADC1 для группы А устанвливаем запускающий триггер
  R_S12ADC1->ADSTRGR_b.TRSA = 0x09; // Триггером будут служить сигналы  ELC_PERIPHERAL_ADC0 и ELC_PERIPHERAL_ADC1 от модуля Event Link Controller

  // Программируем Event Link Setting Register (ELSRn) где указаываем какие события от сторонней периферии будут направлены на триггеры ADC
  R_ELC->ELSRnRC0[ELC_PERIPHERAL_ADC0].ELSRn = ELC_EVENT_GPT0_AD_TRIG_A; // Направляем сигнал с канала A компаратора GPT0 на вход тригера ADC0
  R_ELC->ELSRnRC0[ELC_PERIPHERAL_ADC1].ELSRn = ELC_EVENT_GPT0_AD_TRIG_A; // Направляем сигнал с канала A компаратора GPT0 на вход тригера ADC1


  tx_event_flags_create(&adc_flags, "adc_flags");

  adc0_scan_int_num = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_ADC0_SCAN_END);
  NVIC_SetPriority(adc0_scan_int_num, INT_ADC_SCAN_PRIO);

  R_ICU->IELSRn_b[adc0_scan_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  NVIC_ClearPendingIRQ(adc0_scan_int_num);
  NVIC_EnableIRQ(adc0_scan_int_num);

}




