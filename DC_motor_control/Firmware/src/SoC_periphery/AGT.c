// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-02-26
// 14:46:41
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

IRQn_Type             agt0_int_num;

void  AGT0_isr(void);

SSP_VECTOR_DEFINE_CHAN(AGT0_isr, AGT, INT, 0);


/*-----------------------------------------------------------------------------------------------------
  Обработчик прерывания по исчерпанию счетчика AGT0
  Используется для обслуживания ручного энкодера

  \param void
-----------------------------------------------------------------------------------------------------*/
void  AGT0_isr(void)
{
  Manual_encoder_processing();           // 1.98 us Обрабатываем сигналы с ручного энкодера
  R_ICU->IELSRn_b[agt0_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
}


/*-----------------------------------------------------------------------------------------------------
  Инициализируем AGT0

  \param rate
-----------------------------------------------------------------------------------------------------*/
void Init_AGT0_ticks_man_enc(uint16_t rate)
{
  // Инициализируем таймер AGT0
  R_MSTP->MSTPCRD_b.MSTPD3  = 0; // AGT0.                          0: Cancel the module-stop state

  R_AGT0->AGTCR = 0; // Выключаем счет

  R_AGT0->AGT           = rate;
  R_AGT0->AGTMR1_b.TMOD = 0; // 0 0 0: Timer mode
  R_AGT0->AGTMR1_b.TCK  = 0; // 0 0 0: PCLKB - 60 МГц
  //
  R_AGT0->AGTCR = 0
             + LSHIFT(0,  7) // TCMBF  | Compare Match B Flag      | 1: Underflow.
             + LSHIFT(0,  6) // TCMAF  | Compare Match A Flag      | 1: Underflow.
             + LSHIFT(0,  5) // TUNDF  | Underflow Flag            | 1: Underflow.
             + LSHIFT(0,  4) // TEDGF  | Active Edge Judgment Flag | 1: Active edge received
             + LSHIFT(0,  2) // TSTOP  | AGT Count Forced Stop     | 1: Force count to stop.
             + LSHIFT(0,  1) // TCSTF  | AGT Count Status Flag     | 1: Count in progress.
             + LSHIFT(1,  0) // TSTART | AGT Count Start           | 1: Start count.
  ;


  agt0_int_num = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_AGT0_INT);
  NVIC_SetPriority(agt0_int_num, AGT0_PRIO);

  R_ICU->IELSRn_b[agt0_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  NVIC_ClearPendingIRQ(agt0_int_num);
  NVIC_EnableIRQ(agt0_int_num);

}

/*-----------------------------------------------------------------------------------------------------
  Инициализируем AGT1 для генерации тактовых сигналов DAC для вывода аудио

  \param rate
-----------------------------------------------------------------------------------------------------*/
void Init_AGT1_ticks_to_DAC_DTC(uint16_t rate)
{
  if (R_MSTP->MSTPCRD_b.MSTPD2 != 0)
  {
    // Инициализируем таймер AGT1
    R_MSTP->MSTPCRD_b.MSTPD2  = 0; // AGT1.                          0: Cancel the module-stop state

    R_AGT1->AGTCR         = 0; // Выключаем счет

    R_AGT1->AGT           = rate;
    R_AGT1->AGTMR1_b.TMOD = 0; // 0 0 0: Timer mode
    R_AGT1->AGTMR1_b.TCK  = 0; // 0 0 0: PCLKB - 60 МГц
    //
    R_AGT1->AGTCR = 0
               + LSHIFT(0,  7) // TCMBF  | Compare Match B Flag      | 1: Underflow.
               + LSHIFT(0,  6) // TCMAF  | Compare Match A Flag      | 1: Underflow.
               + LSHIFT(0,  5) // TUNDF  | Underflow Flag            | 1: Underflow.
               + LSHIFT(0,  4) // TEDGF  | Active Edge Judgment Flag | 1: Active edge received
               + LSHIFT(0,  2) // TSTOP  | AGT Count Forced Stop     | 1: Force count to stop.
               + LSHIFT(0,  1) // TCSTF  | AGT Count Status Flag     | 1: Count in progress.
               + LSHIFT(1,  0) // TSTART | AGT Count Start           | 1: Start count.
    ;
  }
  else
  {
    R_AGT1->AGT           = rate;
  }

}

