#ifndef MC50_FILTERS_H
# define MC50_FILTERS_H


typedef struct
{
    uint8_t  en;
    uint16_t arr[3];

} T_median_filter_uint16;

typedef struct
{
    uint8_t  en;
    uint32_t arr[3];

} T_median_filter_uint32;



# define RUN_AVERAGE_FILTER_8 (8)          // Длина фильтра скользящего среднего
typedef struct
{
    uint8_t  en;
    uint32_t head;
    float    acc;
    float    arr[RUN_AVERAGE_FILTER_8];

} T_run_average_float_8;


typedef struct
{
    uint8_t  en;
    uint32_t head;
    float    acc;
    float    *arr;
    uint32_t len;

} T_run_average_float_N;


typedef struct
{
    uint8_t  en;
    uint32_t head;
    int32_t  acc;
    int32_t  *arr;
    int32_t  len;

} T_run_average_int32_N;


# define RUN_AVERAGE_FILTER_4 (4)          // Длина фильтра скользящего среднего
typedef struct
{
    uint8_t  en;
    uint32_t head;
    int16_t  acc;
    int16_t  arr[RUN_AVERAGE_FILTER_4];

}T_run_average_int16_4;


typedef struct
{
    float states[2]; /* '<S1>/Filter_HP_10' */
}
T_hp10_filter1_dw;


typedef struct
{
    float states[2]; /* '<S1>/Filter_HP_10' */
}
T_hp02_filter1_dw;


/* Block signals and states (auto storage) for system '<Root>' */
typedef struct
{
    float  Delay11_DSTATE;               /* '<S1>/Delay11' */
    float  Delay21_DSTATE;               /* '<S1>/Delay21' */
    float  Delay12_DSTATE;               /* '<S1>/Delay12' */
}
T_eliptic_filter1_dw;

typedef struct
{
    float  Delay11_DSTATE;             /* '<S1>/Delay11' */
    float  Delay21_DSTATE;             /* '<S1>/Delay21' */
    float  Delay12_DSTATE;             /* '<S1>/Delay12' */
    float  Delay22_DSTATE;             /* '<S1>/Delay22' */
    float  Delay13_DSTATE;             /* '<S1>/Delay13' */
}
T_eliptic_filter2_dw;



/* Block signals and states (auto storage) for system '<Root>' */
typedef struct {
    int16_t BodyDelay17_DSTATE;          /* '<S1>/BodyDelay17' */
    int16_t BodyDelay16_DSTATE;          /* '<S1>/BodyDelay16' */
    int16_t BodyDelay15_DSTATE;          /* '<S1>/BodyDelay15' */
    int16_t BodyDelay14_DSTATE;          /* '<S1>/BodyDelay14' */
    int16_t BodyDelay13_DSTATE;          /* '<S1>/BodyDelay13' */
    int16_t BodyDelay12_DSTATE;          /* '<S1>/BodyDelay12' */
    int16_t BodyDelay11_DSTATE;          /* '<S1>/BodyDelay11' */
    int16_t BodyDelay10_DSTATE;          /* '<S1>/BodyDelay10' */
    int16_t BodyDelay9_DSTATE;           /* '<S1>/BodyDelay9' */
    int16_t BodyDelay8_DSTATE;           /* '<S1>/BodyDelay8' */
    int16_t BodyDelay7_DSTATE;           /* '<S1>/BodyDelay7' */
    int16_t BodyDelay6_DSTATE;           /* '<S1>/BodyDelay6' */
    int16_t BodyDelay5_DSTATE;           /* '<S1>/BodyDelay5' */
    int16_t BodyDelay4_DSTATE;           /* '<S1>/BodyDelay4' */
    int16_t BodyDelay3_DSTATE;           /* '<S1>/BodyDelay3' */
    int16_t BodyDelay2_DSTATE;           /* '<S1>/BodyDelay2' */
    int16_t BodyDelay18_DSTATE;          /* '<S1>/BodyDelay18' */
} T_fir_filter;

typedef struct
{
   int32_t  k;    // Степеь коэффициента фильтра - n. Коэффициент выражается в виде 2^n , т.е. является степенью двойки для того чтобы можно было применять сдвиги вместо делений и умножений
   int32_t  v1;   // Сохраненное значение
   int8_t   init; // Флаг инициализации.  Если 0, то фильтрация начинается от текущего входного значения, иначе от нуля

}  T_exp_filter;


uint16_t MedianFilter_3uint16(uint16_t inp, T_median_filter_uint16 *fltr);
int16_t  MedianFilter_3int16(int16_t inp, T_median_filter_uint16 *fltr);
uint32_t MedianFilter_3uint32(uint32_t inp, T_median_filter_uint32 *fltr);
int32_t  MedianFilter_3int32(int32_t inp, T_median_filter_uint32 *fltr);
float    RunAverageFilter_float_8(float inp, T_run_average_float_8 *fltr);
int16_t  RunAverageFilter_int16_4(int16_t inp, T_run_average_int16_4 *fltr);
float    RunAverageFilter_float_N(float inp, T_run_average_float_N *fltr);
int32_t  RunAverageFilter_int32_N(int32_t inp, T_run_average_int32_N *fltr);

int16_t  Eliptic_LPF_200Hz_filter(T_eliptic_filter1_dw *rtDW, int16_t Input);
int16_t  Eliptic_LPF_110Hz_filter(T_eliptic_filter2_dw *rtDW, int16_t Input);
int16_t  FIR_LPF_200Hz(T_fir_filter *rtDW, int16_t Input);
int16_t  IIR_HP_10_filter(T_hp10_filter1_dw *rtDW, int16_t Input);
int16_t  IIR_HP_02_filter(T_hp02_filter1_dw *rtDW, int16_t Input);

int32_t Exponential_filter(T_exp_filter *flt, int32_t input );

#endif // FILTERS_H



