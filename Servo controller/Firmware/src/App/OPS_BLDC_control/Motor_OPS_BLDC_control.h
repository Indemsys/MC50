#ifndef OPS_BLDC_CONTROL_H
  #define OPS_BLDC_CONTROL_H

uint32_t OPS_init_module(void);
void     OPS_BLDC_do_6_step_commutation(void);
void     OPS_hard_stop(void);
void     OPS_ret_pins_to_periph_mode(void);
void     OPS_freewheeling(void);
void     OPS_BLDC_ISR_handler(void);

#endif



