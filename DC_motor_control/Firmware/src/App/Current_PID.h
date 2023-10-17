#ifndef CURRENT_PID_H
  #define CURRENT_PID_H

extern PIDParams           current_PID_cbl;


void      Current_PID_init(void);
void      Current_PID_clear(void);
uint32_t  Current_PID_step(void);

#endif



