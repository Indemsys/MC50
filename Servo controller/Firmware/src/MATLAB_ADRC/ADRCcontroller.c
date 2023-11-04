#include "ADRCcontroller.h"
#include "mw_cmsis.h"
#include "rtwtypes.h"

DW_sadrc_T   sadrc_DW;
ExtU_sadrc_T sadrc_U;
ExtY_sadrc_T sadrc_Y;

typedef struct
{
    float f1;
    float f2;
    float k1;
    float k2;
    float k3;
    float out_lo_lim;

} T_ADRC_pars;

T_ADRC_pars adrcp;

/*-----------------------------------------------------------------------------------------------------
  ADRC step

  Observer bandwidth = 10000
-----------------------------------------------------------------------------------------------------*/
void sadrc_step(void)
{
  int32_T i;
  real32_T rtb_Gain3[3];
  real32_T tmp[3];
  real32_T tmp_0[3];
  real32_T UnitDelay_DSTATE;
  real32_T rtb_Saturation;
  real32_T rtb_Subtract;
  rtb_Subtract = sadrc_U.speed -(((0.0F * sadrc_DW.UnitDelay_DSTATE[1])+ sadrc_DW.UnitDelay_DSTATE[0])+(0.0F * sadrc_DW.UnitDelay_DSTATE[2]));
  tmp[0] = 0.846645057F * rtb_Subtract;
  tmp[1] = 7958.13F * rtb_Subtract;
  tmp[2] = 2.5696036E+7F * rtb_Subtract;
  mw_arm_add_f32(&tmp[0],&sadrc_DW.UnitDelay_DSTATE[0],&rtb_Gain3[0], 3U);
  rtb_Saturation =((((sadrc_U.ref_speed - rtb_Gain3[0]) * adrcp.f1)-(adrcp.f2 * rtb_Gain3[1])) - rtb_Gain3[2]) * adrcp.k1;
  if (rtb_Saturation > 100.0F)
  {
    rtb_Saturation = 100.0F;
  }
  else if (rtb_Saturation < adrcp.out_lo_lim)
  {
    rtb_Saturation = adrcp.out_lo_lim;
  }

  sadrc_Y.u = rtb_Saturation;
  tmp[0] = adrcp.k2 * rtb_Saturation;
  tmp_0[0] = 1.3942157F * rtb_Subtract;
  tmp[1] = adrcp.k3 * rtb_Saturation;
  tmp_0[1] = 9564.13184F * rtb_Subtract;
  tmp[2] = 0.0F * rtb_Saturation;
  tmp_0[2] = 2.5696036E+7F * rtb_Subtract;
  mw_arm_add_f32(&tmp[0],&tmp_0[0],&rtb_Gain3[0], 3U);
  rtb_Subtract = sadrc_DW.UnitDelay_DSTATE[1];
  rtb_Saturation = sadrc_DW.UnitDelay_DSTATE[0];
  UnitDelay_DSTATE = sadrc_DW.UnitDelay_DSTATE[2];
  for (i = 0; i < 3; i++)
  {
    tmp[i] =((sadrc_ConstP.Gain2_Gain[i + 3] * rtb_Subtract)+(sadrc_ConstP.Gain2_Gain[i] * rtb_Saturation))+(sadrc_ConstP.Gain2_Gain[i + 6] * UnitDelay_DSTATE);
  }
  mw_arm_add_f32(&rtb_Gain3[0],&tmp[0],&sadrc_DW.UnitDelay_DSTATE[0], 3U);
}

/*-----------------------------------------------------------------------------------------------------


  \param contr_bw
  \param b0
-----------------------------------------------------------------------------------------------------*/
void sadrc_config(float contr_bw, float b0, float out_lo_lim)
{
  adrcp.f1 = contr_bw * contr_bw;
  adrcp.f2 = 2.0f * contr_bw;
  adrcp.k1 = 1.0f / b0;
  adrcp.k2 = 1.953125E-9F * b0;
  adrcp.k3 = 6.25E-5F * b0;
  adrcp.out_lo_lim = out_lo_lim;
}


/*-----------------------------------------------------------------------------------------------------

  \param void
-----------------------------------------------------------------------------------------------------*/
void ADRC_init(float cbw, float b0, float pwm_lim)
{
  sadrc_DW.UnitDelay_DSTATE[0] = 0;
  sadrc_DW.UnitDelay_DSTATE[1] = 0;
  sadrc_DW.UnitDelay_DSTATE[2] = 0;
  sadrc_config(cbw, b0, pwm_lim);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t  ADRC_loop_step(float speed, float ref_speed)
{
  float adrc_output;

  sadrc_U.speed     = speed;
  sadrc_U.ref_speed = ref_speed;

  sadrc_step();

  adrc_output = sadrc_Y.u;

  return lrintf(adrc_output);
}


void sadrc_terminate(void)
{
}
