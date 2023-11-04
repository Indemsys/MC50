#ifndef ASINC_SINUSOIDAL_ROTATION_H
  #define ASINC_SINUSOIDAL_ROTATION_H


#define  DEF_SIN_ROT_FREQ    (30.0f) // Частота вращения ротора по умолчанию
#define  DEF_SIN_ROT_AMP     (100)    // Амплитуда синусоиды в процентах от максимальной
#define  MIR_ROT_SPEED       (1.0f)  // Минимальная скорость вращения ротора в оборотах в сек


typedef struct
{
  // Переменные для установки через FreeMaster
  float    set_rot_freq;
  float    set_rot_ampl;
  uint8_t  set_pwm_mode;

  // Текущие переменные установки параметров синусоидального вращения
  float    rot_freq;
  float    rot_ampl;
  uint8_t  pwm_mode;
  int32_t  scale;

} T_sinusoidal_rotation;

extern T_sinusoidal_rotation  sr;


void Asinc_Sinusoidal_BLDC_rotation_thread(void);


#endif



