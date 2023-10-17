#ifndef SOUND_DAC_H
  #define SOUND_DAC_H


  #define DAC_MID_LEVEL               (4096/2)

  #define PLAYER_DEFAULT_SAMPLE_RATE  8000ul
  #define PLAYER_MAX_SAMPLE_RATE      48000ul


  #define AUDIO_BUF_SAMPLES_NUM       512
  #define SAMPLE_SIZE                 2

extern int16_t                        audio_buf[2][AUDIO_BUF_SAMPLES_NUM];

void    Sound_DAC_update_sample_rate(uint32_t sample_rate);
uint8_t Sound_DAC_get_current_buf(void);
void    Sound_DAC_init_player_periphery(uint32_t sample_rate);
void    Sound_DAC_clear_for_silience(int16_t *dest, uint32_t samples_num);

#endif



