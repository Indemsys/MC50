#ifndef RTW_HEADER_sadrc_h_
#define RTW_HEADER_sadrc_h_
#ifndef sadrc_COMMON_INCLUDES_
#define sadrc_COMMON_INCLUDES_
#include "rtwtypes.h"
#endif

#include "sadrc_types.h"

typedef struct
{
  real32_T UnitDelay_DSTATE[3];
}
DW_sadrc_T;

typedef struct
{
  real32_T Gain2_Gain[9];
}
ConstP_sadrc_T;

typedef struct
{
  real32_T ref_speed;
  real32_T speed;
}
ExtU_sadrc_T;

typedef struct
{
  real32_T u;
}
ExtY_sadrc_T;

extern DW_sadrc_T sadrc_DW;
extern ExtU_sadrc_T sadrc_U;
extern ExtY_sadrc_T sadrc_Y;
extern const ConstP_sadrc_T sadrc_ConstP;
extern void sadrc_initialize(void);
extern void sadrc_step(void);
extern void sadrc_terminate(void);

unsigned int ADRC_loop_step(float speed, float ref_speed);
void         ADRC_init(float cbw, float b0, float pwm_lim);

#endif

