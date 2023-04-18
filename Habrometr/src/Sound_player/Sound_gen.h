#ifndef SOUND_GEN_H
  #define SOUND_GEN_H


#define PH_MAX                0x100000000ull



void      Tone_gen_start(unsigned int freq, uint32_t sample_rate);
int16_t   Get_16bit_tone_sample(void);
void      Generate_sound_to_buf(int16_t *abuf, int samples_num, uint8_t attenuation);
#endif
