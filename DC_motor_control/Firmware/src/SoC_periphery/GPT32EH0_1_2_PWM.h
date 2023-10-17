#ifndef GPT32EH0_1_2_PWM_H
  #define GPT32EH0_1_2_PWM_H

#define  PHASE_PULL_UP        100
#define  PHASE_PULL_DOWN      0
#define  PHASE_Z_STATE       -1


#define  FL_PHASE_PULL_UP     1
#define  FL_PHASE_PULL_DOWN   2
#define  FL_PHASE_Z_STATE     3
#define  FL_PHASE_PWM_ON      4

extern volatile uint8_t   gpt0_out_mode;
extern volatile uint8_t   gpt1_out_mode;
extern volatile uint8_t   gpt2_out_mode;



uint32_t GPT_0_1_2_PWM_init(uint32_t freq);
void     GPT_0_1_2_PWM_start(void);
void     GPT_0_1_2_PWM_stop(void);
void     Phase_U_mode(int32_t val);
void     Phase_V_mode(int32_t val);
void     Phase_W_mode(int32_t val);

void     Set_output_U(uint8_t mode);
void     Set_output_V(uint8_t mode);
void     Set_output_W(uint8_t mode);

#endif



