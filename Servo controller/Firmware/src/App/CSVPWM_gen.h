#ifndef CSVPWM_H
#define CSVPWM_H

#define MDL_SINUSOIDAL               0
#define MDL_CSVPWM                   1
#define MDL_DEGREE_60_DISCONTINUOUS  2
#define MDL_DPWM_120_TOP             3
#define MDL_DPWM_120_BOT             4


typedef struct
{
    int32_t a;
    int32_t b;
    int32_t c;
}
T_int32_3ph;

typedef struct
{
    float a;
    float b;
    float c;

} T_float_3ph;


typedef struct
{
    int32_t pa;
    int32_t pb;
    int32_t pc;
} T_pwm_3ph;


typedef struct
{
    int32_t tmin;
    int32_t tmax;
    int32_t toffset;
    int32_t tga_on;
    int32_t tgb_on;
    int32_t tgc_on;
    int32_t tga_off;
    int32_t tgb_off;
    int32_t tgc_off;


} T_SVPWM_debug;


uint32_t  SinGen_get_delta(float sig_freq, float pwm_freq);
uint32_t  SinGen_get_3ph_sin(uint32_t norm_angle, uint32_t delta, T_int32_3ph *psin);
void      PWM_modulator_abs_float(T_float_3ph *vabc, float tsw, uint8_t mode, T_float_3ph *duty);
void      PWM_modulator_float(T_float_3ph *vabc, uint8_t mode, T_float_3ph *duty);
void      PWM_modulator_integer(T_int32_3ph *vabc, uint8_t mode, T_int32_3ph *duty, T_SVPWM_debug *dbg);
void      PWM_converter(T_int32_3ph *pwm, int32_t half_range, int32_t scale, T_pwm_3ph *cv);


#endif
