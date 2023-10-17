#ifndef S7V30_DAC_H
  #define S7V30_DAC_H

void     Init_DAC(void);
void     Set_DAC_val(uint8_t channel, uint16_t val);
uint16_t Get_DAC_val(uint8_t channel);

#endif



