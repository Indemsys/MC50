#ifndef SPEED_PID_H
  #define SPEED_PID_H

extern PIDParams           speed_PID_cbl;

void Speed_PID_init(void);
void Speed_PID_clear(void);
void Speed_PID_step(void);

#endif



