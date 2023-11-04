#ifndef PWM_TRIANGLE_3PH_2BUFF_H
  #define PWM_TRIANGLE_3PH_2BUFF_H


#define  PHASE_U  0
#define  PHASE_V  1
#define  PHASE_W  2

#define  ADC_SAMPLING_ADVANCE   16 // Количество тактов таймера на которое сдвигается вперед момент начала выборки АЦП относительно центра импульса ШИМ

uint32_t PWM_3ph_triangle_2buffered_init(uint32_t pwm_freq);

void     Post_phase_pwm(uint8_t phase, uint32_t pwm_lev);
void     PWM_send(int32_t pwm_U, int32_t pwm_V, int32_t pwm_W);


#endif



