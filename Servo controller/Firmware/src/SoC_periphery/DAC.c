// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-02-24
// 18:58:29
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_DAC(void)
{
  R_MSTP->MSTPCRD_b.MSTPD20 = 0; // Разрешаем работу модуля DAC

  R_DAC->DADRn_b[0].DADR  = 0;
  R_DAC->DADRn_b[1].DADR  = 0;
  R_DAC->DADPR_b.DPSEL    = 0; // 0: Right-justified format

  R_DAC->DAAMPCR_b.DAAMP0 = 0; // 0: Do not use channel 0 output amplifier. 1: Use channel 0 output amplifier.
  R_DAC->DAAMPCR_b.DAAMP1 = 1; // 0: Do not use channel 1 output amplifier. 1: Use channel 1 output amplifier.
  R_DAC->DAADSCR_b.DAADST = 0; // 0: Do not synchronize DAC12 operation with ADC12 (unit 1) operation
  R_DAC->DACR_b.DAE       = 0; // 0: Control D/A conversion of channels 0 and 1 individually
  R_DAC->DACR_b.DAOE0     = 0; // 1: Enable D/A conversion of channel 0 (DA0).
  R_DAC->DACR_b.DAOE1     = 1; // 1: Enable D/A conversion of channel 1 (DA1).
}

/*-----------------------------------------------------------------------------------------------------


  \param channel
  \param val
-----------------------------------------------------------------------------------------------------*/
void Set_DAC_val(uint8_t channel, uint16_t val)
{
  switch (channel)
  {
  case 0:
    R_DAC->DADRn_b[0].DADR = val;
    break;
  case 1:
    R_DAC->DADRn_b[1].DADR = val;
    break;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param channel

  \return uint16_t
-----------------------------------------------------------------------------------------------------*/
uint16_t Get_DAC_val(uint8_t channel)
{
  switch (channel)
  {
  case 0:
    return R_DAC->DADRn_b[0].DADR;
  case 1:
    return R_DAC->DADRn_b[1].DADR;
  }
  return 0;
}

