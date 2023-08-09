// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-02-26
// 14:46:41
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


/*-----------------------------------------------------------------------------------------------------
  Инициализируем AGT0 для генерации сигналов выборки ADC

  \param rate
-----------------------------------------------------------------------------------------------------*/
void Init_AGT0_ticks_to_ADC(uint16_t rate)
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

}

/*-----------------------------------------------------------------------------------------------------
  Инициализируем AGT1 для генерации тактовых сигналов DAC для вывода аудио

  \param rate
-----------------------------------------------------------------------------------------------------*/
void Init_AGT1_ticks_to_DTC(uint16_t rate)
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

