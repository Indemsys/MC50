// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2017-01-17
// 12:08:47
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
uint16_t MedianFilter_3uint16(uint16_t inp, T_median_filter_uint16 *fltr)
{
  if (fltr->en == 0)
  {
    fltr->arr[0] = inp;
    fltr->arr[1] = inp;
    fltr->arr[2] = inp;
    fltr->en = 1;
  }
  else
  {
    fltr->arr[2] = fltr->arr[1];
    fltr->arr[1] = fltr->arr[0];
    fltr->arr[0] = inp;
  }

  // Фильтрация медианным фильтром по выборке из 3-х
  if (fltr->arr[1] > fltr->arr[0])
  {
    if (fltr->arr[2] > fltr->arr[1]) return fltr->arr[1];
    else if (fltr->arr[2] < fltr->arr[0]) return fltr->arr[0];
    else return fltr->arr[2];
  }
  else
  {
    if (fltr->arr[0] < fltr->arr[2]) return fltr->arr[0];
    else if (fltr->arr[1] > fltr->arr[2]) return fltr->arr[1];
    else return fltr->arr[2];
  }
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
int16_t MedianFilter_3int16(int16_t inp, T_median_filter_uint16 *fltr)
{
  if (fltr->en == 0)
  {
    fltr->arr[0] = inp;
    fltr->arr[1] = inp;
    fltr->arr[2] = inp;
    fltr->en = 1;
  }
  else
  {
    fltr->arr[2] = fltr->arr[1];
    fltr->arr[1] = fltr->arr[0];
    fltr->arr[0] = inp;
  }

  // Фильтрация медианным фильтром по выборке из 3-х
  if (fltr->arr[1] > fltr->arr[0])
  {
    if (fltr->arr[2] > fltr->arr[1]) return fltr->arr[1];
    else if (fltr->arr[2] < fltr->arr[0]) return fltr->arr[0];
    else return fltr->arr[2];
  }
  else
  {
    if (fltr->arr[0] < fltr->arr[2]) return fltr->arr[0];
    else if (fltr->arr[1] > fltr->arr[2]) return fltr->arr[1];
    else return fltr->arr[2];
  }
}
/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
uint32_t MedianFilter_3uint32(uint32_t inp, T_median_filter_uint32 *fltr)
{
  if (fltr->en == 0)
  {
    fltr->arr[0] = inp;
    fltr->arr[1] = inp;
    fltr->arr[2] = inp;
    fltr->en = 1;
  }
  else
  {
    fltr->arr[2] = fltr->arr[1];
    fltr->arr[1] = fltr->arr[0];
    fltr->arr[0] = inp;
  }

  // Фильтрация медианным фильтром по выборке из 3-х
  if (fltr->arr[1] > fltr->arr[0])
  {
    if (fltr->arr[2] > fltr->arr[1]) return fltr->arr[1];
    else if (fltr->arr[2] < fltr->arr[0]) return fltr->arr[0];
    else return fltr->arr[2];
  }
  else
  {
    if (fltr->arr[0] < fltr->arr[2]) return fltr->arr[0];
    else if (fltr->arr[1] > fltr->arr[2]) return fltr->arr[1];
    else return fltr->arr[2];
  }
}
/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
int32_t MedianFilter_3int32(int32_t inp, T_median_filter_uint32 *fltr)
{
  if (fltr->en == 0)
  {
    fltr->arr[0] = inp;
    fltr->arr[1] = inp;
    fltr->arr[2] = inp;
    fltr->en = 1;
  }
  else
  {
    fltr->arr[2] = fltr->arr[1];
    fltr->arr[1] = fltr->arr[0];
    fltr->arr[0] = inp;
  }

  // Фильтрация медианным фильтром по выборке из 3-х
  if (fltr->arr[1] > fltr->arr[0])
  {
    if (fltr->arr[2] > fltr->arr[1]) return fltr->arr[1];
    else if (fltr->arr[2] < fltr->arr[0]) return fltr->arr[0];
    else return fltr->arr[2];
  }
  else
  {
    if (fltr->arr[0] < fltr->arr[2]) return fltr->arr[0];
    else if (fltr->arr[1] > fltr->arr[2]) return fltr->arr[1];
    else return fltr->arr[2];
  }
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
float RunAverageFilter_float_8(float inp, T_run_average_float_8 *fltr)
{
  uint32_t i;
  if (fltr->en == 0)
  {
    for (i = 0; i < RUN_AVERAGE_FILTER_8; i++)
    {
      fltr->arr[i] = inp;
    }
    fltr->acc = inp * RUN_AVERAGE_FILTER_8;
    fltr->head = 0;
    fltr->en = 1;
    return inp;
  }
  else
  {
    uint32_t head = (fltr->head + 1) & (RUN_AVERAGE_FILTER_8 - 1);
    fltr->acc -= fltr->arr[head]; // Удаляем из аккумулятора последнее старое значение
    fltr->arr[head] = inp;        // Добавляем в память фильтра новое значение
    fltr->acc += inp;             // Добавляем в аккумулятор новое значение
    fltr->head = head;
    return (fltr->acc / (float)RUN_AVERAGE_FILTER_8);
  }
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
float RunAverageFilter_float_N(float inp, T_run_average_float_N *fltr)
{
  uint32_t i;
  if (fltr->en == 0)
  {
    for (i = 0; i < fltr->len; i++)
    {
      fltr->arr[i] = inp;
    }
    fltr->acc = inp * fltr->len;
    fltr->head = 0;
    fltr->en = 1;
    return inp;
  }
  else
  {
    uint32_t head;

    head = fltr->head + 1;
    if (head >= fltr->len) head = 0;

    fltr->acc -= fltr->arr[head]; // Удаляем из аккумулятора последнее старое значение
    fltr->arr[head] = inp;        // Добавляем в память фильтра новое значение
    fltr->acc += inp;             // Добавляем в аккумулятор новое значение
    fltr->head = head;
    return (fltr->acc / (float)fltr->len);
  }
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
int32_t RunAverageFilter_int32_N(int32_t inp, T_run_average_int32_N *fltr)
{
  uint32_t i;
  if (fltr->en == 0)
  {
    for (i = 0; i < fltr->len; i++)
    {
      fltr->arr[i] = inp;
    }
    fltr->acc = inp * fltr->len;
    fltr->head = 0;
    fltr->en = 1;
    return inp;
  }
  else
  {
    uint32_t head;

    head = fltr->head + 1;
    if (head >= fltr->len) head = 0;

    fltr->acc -= fltr->arr[head]; // Удаляем из аккумулятора последнее старое значение
    fltr->arr[head] = inp;        // Добавляем в память фильтра новое значение
    fltr->acc += inp;             // Добавляем в аккумулятор новое значение
    fltr->head = head;
    return (fltr->acc / fltr->len);
  }
}

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
int16_t RunAverageFilter_int16_4(int16_t inp, T_run_average_int16_4 *fltr)
{
  uint32_t i;
  if (fltr->en == 0)
  {
    for (i = 0; i < RUN_AVERAGE_FILTER_4; i++)
    {
      fltr->arr[i] = inp;
    }
    fltr->acc = inp * RUN_AVERAGE_FILTER_4;
    fltr->head = 0;
    fltr->en = 1;
    return inp;
  }
  else
  {
    uint32_t head = (fltr->head + 1) & (RUN_AVERAGE_FILTER_4 - 1);
    fltr->acc -= fltr->arr[head]; // Удаляем из аккумулятора последнее старое значение
    fltr->arr[head] = inp;        // Добавляем в память фильтра новое значение
    fltr->acc += inp;             // Добавляем в аккумулятор новое значение
    fltr->head = head;
    return (fltr->acc / RUN_AVERAGE_FILTER_4);
  }
}


/*-----------------------------------------------------------------------------------------------------
  IIR фильтр высоких частот
  Пропускает с частоты 0.2 Гц
  Ослабление на частоте 0.02 Гц = 10дБ
  Убирает постоянную составляющую

  \param rtDW
  \param Input

  \return int16_t
-----------------------------------------------------------------------------------------------------*/
int16_t  IIR_HP_02_filter(T_hp02_filter1_dw *rtDW, int16_t Input)
{
  float  Output;
  float denAccum;

  /* S-Function (sdspbiquad): '<S1>/Filter' incorporates:
   *  Inport: '<Root>/Input'
   */
  denAccum = (0.999952853F * (float)Input - -0.999905765F * rtDW->states[0]) - 0.0F * rtDW->states[1];

  /* Outport: '<Root>/Output' incorporates:
   *  S-Function (sdspbiquad): '<S1>/Filter'
   */
  Output = (denAccum + -rtDW->states[0]) + 0.0F * rtDW->states[1];

  /* S-Function (sdspbiquad): '<S1>/Filter' */
  rtDW->states[1] = rtDW->states[0];
  rtDW->states[0] = denAccum;
  return (int16_t)Output;
}


/*-----------------------------------------------------------------------------------------------------
  IIR фильтр высоких частот
  Пропускает с частоты 10 Гц
  Ослабление на частоте 1 Гц = 10дБ
  Убирает постоянную составляющую

  \param rtDW
  \param Input

  \return int16_t
-----------------------------------------------------------------------------------------------------*/
int16_t IIR_HP_10_filter(T_hp10_filter1_dw *rtDW, int16_t Input)
{
  float  Output;
  float denAccum;

  /* S-Function (sdspbiquad): '<S1>/Filter_HP_10' incorporates:
   *  Inport: '<Root>/Input'
   */
  denAccum = (0.997649372F * (float)Input - -0.995298684F * rtDW->states[0]) - 0.0F * rtDW->states[1];

  /* Outport: '<Root>/Output' incorporates:
   *  S-Function (sdspbiquad): '<S1>/Filter_HP_10'
   */
  Output = (denAccum + -rtDW->states[0]) + 0.0F * rtDW->states[1];

  /* S-Function (sdspbiquad): '<S1>/Filter_HP_10' */
  rtDW->states[1] = rtDW->states[0];
  rtDW->states[0] = denAccum;
  return (int16_t)Output;
}



/*-----------------------------------------------------------------------------------------------------
 IIR фильтр c плавающей запятой
 Сэмплирование 2000 Гц
 Полоса пропускания 0-100 Гц
 Полоса заграждения от 200 Гц -35дБ

 Сэмплирование 4000 Гц
 Полоса пропускания 0-200 Гц
 Полоса заграждения от 400 Гц -35дБ

 \param dw
 \param Input

 \return float
-----------------------------------------------------------------------------------------------------*/
int16_t Eliptic_LPF_200Hz_filter(T_eliptic_filter1_dw *rtDW, int16_t Input)
{
  float  Output;
  float  rtb_Delay11;
  float  rtb_SumA31;
  float  rtb_SumA22;

  /* Delay: '<S1>/Delay11' */
  rtb_Delay11 = rtDW->Delay11_DSTATE;

  /* Sum: '<S1>/SumA31' incorporates:
   *  Delay: '<S1>/Delay11'
   *  Delay: '<S1>/Delay21'
   *  Gain: '<S1>/a(2)(1)'
   *  Gain: '<S1>/a(3)(1)'
   *  Gain: '<S1>/s(1)'
   *  Inport: '<Root>/Input'
   *  Sum: '<S1>/SumA21'
   */
  rtb_SumA31 = (0.10367614775896072f * (float)Input - -1.7808578014373779f * rtDW->Delay11_DSTATE) - 0.87342262268066406f * rtDW->Delay21_DSTATE;

  /* Sum: '<S1>/SumA22' incorporates:
   *  Delay: '<S1>/Delay11'
   *  Delay: '<S1>/Delay12'
   *  Delay: '<S1>/Delay21'
   *  Gain: '<S1>/a(2)(2)'
   *  Gain: '<S1>/b(2)(1)'
   *  Gain: '<S1>/s(2)'
   *  Sum: '<S1>/SumB21'
   *  Sum: '<S1>/SumB31'
   */
  rtb_SumA22 = ((-1.5203124284744263f * rtDW->Delay11_DSTATE + rtb_SumA31) + rtDW->Delay21_DSTATE) * 0.14596366882324219f - -0.84315669536590576f * rtDW->Delay12_DSTATE;

  /* Outport: '<Root>/Output' incorporates:
   *  Delay: '<S1>/Delay12'
   *  Sum: '<S1>/SumB22'
   */
  Output = rtb_SumA22 + rtDW->Delay12_DSTATE;

  /* Update for Delay: '<S1>/Delay11' */
  rtDW->Delay11_DSTATE = rtb_SumA31;

  /* Update for Delay: '<S1>/Delay21' */
  rtDW->Delay21_DSTATE = rtb_Delay11;

  /* Update for Delay: '<S1>/Delay12' */
  rtDW->Delay12_DSTATE = rtb_SumA22;

  return (int16_t)Output;
}

/*-----------------------------------------------------------------------------------------------------
 IIR фильтр c плавающей запятой

 Сэмплирование 4000 Гц
 Полоса пропускания 0...110 Гц
 Полоса заграждения от 150 Гц -48дБ


  \param rtDW
  \param Input
-----------------------------------------------------------------------------------------------------*/
int16_t Eliptic_LPF_110Hz_filter(T_eliptic_filter2_dw *rtDW, int16_t Input)
{
  float  Output;
  float  rtb_Delay11;
  float  rtb_SumA31;
  float  rtb_Delay12;
  float  rtb_SumA32;
  float  rtb_SumA23;

  /* Delay: '<S1>/Delay11' */
  rtb_Delay11 = rtDW->Delay11_DSTATE;

  /* Sum: '<S1>/SumA31' incorporates:
   *  Delay: '<S1>/Delay11'
   *  Delay: '<S1>/Delay21'
   *  Gain: '<S1>/a(2)(1)'
   *  Gain: '<S1>/a(3)(1)'
   *  Gain: '<S1>/s(1)'
   *  Inport: '<Root>/Input'
   *  Sum: '<S1>/SumA21'
   */
  rtb_SumA31 = (0.17559959F * (float)Input - -1.95003033F * rtDW->Delay11_DSTATE) - 0.979401946F * rtDW->Delay21_DSTATE;

  /* Delay: '<S1>/Delay12' */
  rtb_Delay12 = rtDW->Delay12_DSTATE;

  /* Sum: '<S1>/SumA32' incorporates:
   *  Delay: '<S1>/Delay11'
   *  Delay: '<S1>/Delay12'
   *  Delay: '<S1>/Delay21'
   *  Delay: '<S1>/Delay22'
   *  Gain: '<S1>/a(2)(2)'
   *  Gain: '<S1>/a(3)(2)'
   *  Gain: '<S1>/b(2)(1)'
   *  Gain: '<S1>/s(2)'
   *  Sum: '<S1>/SumA22'
   *  Sum: '<S1>/SumB21'
   *  Sum: '<S1>/SumB31'
   */
  rtb_SumA32 = (((-1.94088F * rtDW->Delay11_DSTATE + rtb_SumA31) + rtDW->Delay21_DSTATE) * 0.12542823F - -1.90927088F * rtDW->Delay12_DSTATE) - 0.924921453F * rtDW->Delay22_DSTATE;

  /* Sum: '<S1>/SumA23' incorporates:
   *  Delay: '<S1>/Delay12'
   *  Delay: '<S1>/Delay13'
   *  Delay: '<S1>/Delay22'
   *  Gain: '<S1>/a(2)(3)'
   *  Gain: '<S1>/b(2)(2)'
   *  Gain: '<S1>/s(3)'
   *  Sum: '<S1>/SumB22'
   *  Sum: '<S1>/SumB32'
   */
  rtb_SumA23 = ((-1.87522316F * rtDW->Delay12_DSTATE + rtb_SumA32) + rtDW->Delay22_DSTATE) * 0.0840368271F - -0.940594F * rtDW->Delay13_DSTATE;

  /* Outport: '<Root>/Output' incorporates:
   *  Delay: '<S1>/Delay13'
   *  Sum: '<S1>/SumB23'
   */
  Output = rtb_SumA23 + rtDW->Delay13_DSTATE;

  /* Update for Delay: '<S1>/Delay11' */
  rtDW->Delay11_DSTATE = rtb_SumA31;

  /* Update for Delay: '<S1>/Delay21' */
  rtDW->Delay21_DSTATE = rtb_Delay11;

  /* Update for Delay: '<S1>/Delay12' */
  rtDW->Delay12_DSTATE = rtb_SumA32;

  /* Update for Delay: '<S1>/Delay22' */
  rtDW->Delay22_DSTATE = rtb_Delay12;

  /* Update for Delay: '<S1>/Delay13' */
  rtDW->Delay13_DSTATE = rtb_SumA23;

  return (int16_t)Output;
}





/*-----------------------------------------------------------------------------------------------------
 Целочисленный  FIR фильтр
 Сэмплирование 2000 Гц
 Полоса пропускания 0-100 Гц
 Полоса заграждения от 200 Гц -20дБ

 Сэмплирование 4000 Гц
 Полоса пропускания 0-200 Гц
 Полоса заграждения от 400 Гц -20дБ


 \param rtDW
 \param Input

 \return int32_t
-----------------------------------------------------------------------------------------------------*/
int16_t FIR_LPF_200Hz(T_fir_filter *rtDW, int16_t Input)
{
  int32_t Output;
  int16_t rtb_BodyDelay17;
  int32_t tmp;
  int32_t tmp_0;
  int32_t tmp_1;
  int32_t tmp_2;
  int32_t tmp_3;
  int32_t tmp_4;
  int32_t tmp_5;
  int32_t tmp_6;
  int32_t tmp_7;
  int32_t tmp_8;
  int32_t tmp_9;
  int32_t tmp_a;
  int32_t tmp_b;
  int32_t tmp_c;
  int32_t tmp_d;
  int32_t tmp_e;
  int32_t tmp_f;
  int32_t tmp_g;

  /* Delay: '<S1>/BodyDelay17' */
  rtb_BodyDelay17 = rtDW->BodyDelay17_DSTATE;

  /* Gain: '<S1>/b(17)' incorporates:
   *  Delay: '<S1>/BodyDelay17'
   */
  tmp = -212 * rtDW->BodyDelay17_DSTATE;

  /* Gain: '<S1>/b(16)' incorporates:
   *  Delay: '<S1>/BodyDelay16'
   */
  tmp_0 = 219 * rtDW->BodyDelay16_DSTATE;

  /* Gain: '<S1>/b(15)' incorporates:
   *  Delay: '<S1>/BodyDelay15'
   */
  tmp_1 = 944 * rtDW->BodyDelay15_DSTATE;

  /* Gain: '<S1>/b(14)' incorporates:
   *  Delay: '<S1>/BodyDelay14'
   */
  tmp_2 = 1889 * rtDW->BodyDelay14_DSTATE;

  /* Gain: '<S1>/b(13)' incorporates:
   *  Delay: '<S1>/BodyDelay13'
   */
  tmp_3 = 2932 * rtDW->BodyDelay13_DSTATE;

  /* Gain: '<S1>/b(12)' incorporates:
   *  Delay: '<S1>/BodyDelay12'
   */
  tmp_4 = 3915 * rtDW->BodyDelay12_DSTATE;

  /* Gain: '<S1>/b(11)' incorporates:
   *  Delay: '<S1>/BodyDelay11'
   */
  tmp_5 = 4681 * rtDW->BodyDelay11_DSTATE;

  /* Gain: '<S1>/b(10)' incorporates:
   *  Delay: '<S1>/BodyDelay10'
   */
  tmp_6 = 5099 * rtDW->BodyDelay10_DSTATE;

  /* Gain: '<S1>/b(9)' incorporates:
   *  Delay: '<S1>/BodyDelay9'
   */
  tmp_7 = 5099 * rtDW->BodyDelay9_DSTATE;

  /* Gain: '<S1>/b(8)' incorporates:
   *  Delay: '<S1>/BodyDelay8'
   */
  tmp_8 = 4681 * rtDW->BodyDelay8_DSTATE;

  /* Gain: '<S1>/b(7)' incorporates:
   *  Delay: '<S1>/BodyDelay7'
   */
  tmp_9 = 3915 * rtDW->BodyDelay7_DSTATE;

  /* Gain: '<S1>/b(6)' incorporates:
   *  Delay: '<S1>/BodyDelay6'
   */
  tmp_a = 2932 * rtDW->BodyDelay6_DSTATE;

  /* Gain: '<S1>/b(5)' incorporates:
   *  Delay: '<S1>/BodyDelay5'
   */
  tmp_b = 1889 * rtDW->BodyDelay5_DSTATE;

  /* Gain: '<S1>/b(4)' incorporates:
   *  Delay: '<S1>/BodyDelay4'
   */
  tmp_c = 944 * rtDW->BodyDelay4_DSTATE;

  /* Gain: '<S1>/b(3)' incorporates:
   *  Delay: '<S1>/BodyDelay3'
   */
  tmp_d = 219 * rtDW->BodyDelay3_DSTATE;

  /* Gain: '<S1>/b(2)' incorporates:
   *  Delay: '<S1>/BodyDelay2'
   */
  tmp_e = -212 * rtDW->BodyDelay2_DSTATE;

  /* Gain: '<S1>/b(1)' incorporates:
   *  Inport: '<Root>/Input'
   */
  tmp_f = -2033 * Input;

  /* Gain: '<S1>/b(18)' incorporates:
   *  Delay: '<S1>/BodyDelay18'
   */
  tmp_g = -2033 * rtDW->BodyDelay18_DSTATE;

  /* DataTypeConversion: '<S1>/ConvertOut' incorporates:
   *  Gain: '<S1>/b(1)'
   *  Gain: '<S1>/b(10)'
   *  Gain: '<S1>/b(11)'
   *  Gain: '<S1>/b(12)'
   *  Gain: '<S1>/b(13)'
   *  Gain: '<S1>/b(14)'
   *  Gain: '<S1>/b(15)'
   *  Gain: '<S1>/b(16)'
   *  Gain: '<S1>/b(17)'
   *  Gain: '<S1>/b(18)'
   *  Gain: '<S1>/b(2)'
   *  Gain: '<S1>/b(3)'
   *  Gain: '<S1>/b(4)'
   *  Gain: '<S1>/b(5)'
   *  Gain: '<S1>/b(6)'
   *  Gain: '<S1>/b(7)'
   *  Gain: '<S1>/b(8)'
   *  Gain: '<S1>/b(9)'
   *  Sum: '<S1>/BodyLSum10'
   *  Sum: '<S1>/BodyLSum11'
   *  Sum: '<S1>/BodyLSum12'
   *  Sum: '<S1>/BodyLSum13'
   *  Sum: '<S1>/BodyLSum14'
   *  Sum: '<S1>/BodyLSum15'
   *  Sum: '<S1>/BodyLSum16'
   *  Sum: '<S1>/BodyLSum17'
   *  Sum: '<S1>/BodyLSum18'
   *  Sum: '<S1>/BodyLSum5'
   *  Sum: '<S1>/BodyLSum6'
   *  Sum: '<S1>/BodyLSum7'
   *  Sum: '<S1>/BodyLSum8'
   *  Sum: '<S1>/BodyLSum9'
   */
  tmp = ((((((((((((((((((tmp_b & 268435456) != 0 ? tmp_b | -268435456 : tmp_b &
                         268435455) + ((tmp_c & 268435456) != 0 ? tmp_c | -268435456 : tmp_c &
                                       268435455)) + ((tmp_a & 268435456) != 0 ? tmp_a | -268435456 :
                                                      tmp_a & 268435455)) + ((tmp_9 & 268435456) != 0 ? tmp_9 | -268435456 : tmp_9
                                                                             & 268435455)) + ((tmp_8 & 268435456) != 0 ? tmp_8 | -268435456 : tmp_8 &
                                                                                              268435455)) + ((tmp_7 & 268435456) != 0 ? tmp_7 |
                                                                                                             -268435456 : tmp_7 & 268435455)) + ((tmp_6 & 268435456) != 0 ? tmp_6 |
                                                                                                                                                 -268435456 : tmp_6 & 268435455)) + ((tmp_5 & 268435456) != 0 ? tmp_5 |
                                                                                                                                                                                     -268435456 : tmp_5 & 268435455)) + ((tmp_4 & 268435456) != 0 ? tmp_4 |
                                                                                                                                                                                                                         -268435456 : tmp_4 & 268435455)) + ((tmp_3 & 268435456) != 0 ? tmp_3 |
                                                                                                                                                                                                                                                             -268435456 : tmp_3 & 268435455)) + ((tmp_2 & 268435456) != 0 ? tmp_2 |
                                                                                                                                                                                                                                                                                                 -268435456 : tmp_2 & 268435455)) + ((tmp_1 & 268435456) != 0 ? tmp_1 |
                                                                                                                                                                                                                                                                                                                                     -268435456 : tmp_1 & 268435455)) + ((tmp_0 & 268435456) != 0 ? tmp_0 |
                                                                                                                                                                                                                                                                                                                                                                         -268435456 : tmp_0 & 268435455)) + ((tmp & 268435456) != 0 ? tmp |
                                                                                                                                                                                                                                                                                                                                                                                                             -268435456 : tmp & 268435455)) + ((tmp_d & 268435456) != 0 ? tmp_d |
                                                                                                                                                                                                                                                                                                                                                                                                                                               -268435456 : tmp_d & 268435455)) + ((tmp_e & 268435456) != 0 ? tmp_e
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   | -268435456 : tmp_e & 268435455)) + ((tmp_f & 268435456) != 0 ?
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         tmp_f | -268435456 : tmp_f & 268435455)) + ((tmp_g & 268435456) != 0 ?
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     tmp_g | -268435456 : tmp_g & 268435455);

  /* Outport: '<Root>/Output' incorporates:
   *  DataTypeConversion: '<S1>/ConvertOut'
   */
  Output = (int16_t)(((((tmp & 16384) != 0) && ((tmp & 16383) != 0)) + (tmp >> 15)) + (((tmp & 32767) == 16384) && ((tmp & 32768) != 0)));

  /* Update for Delay: '<S1>/BodyDelay17' incorporates:
   *  Delay: '<S1>/BodyDelay16'
   */
  rtDW->BodyDelay17_DSTATE = rtDW->BodyDelay16_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay16' incorporates:
   *  Delay: '<S1>/BodyDelay15'
   */
  rtDW->BodyDelay16_DSTATE = rtDW->BodyDelay15_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay15' incorporates:
   *  Delay: '<S1>/BodyDelay14'
   */
  rtDW->BodyDelay15_DSTATE = rtDW->BodyDelay14_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay14' incorporates:
   *  Delay: '<S1>/BodyDelay13'
   */
  rtDW->BodyDelay14_DSTATE = rtDW->BodyDelay13_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay13' incorporates:
   *  Delay: '<S1>/BodyDelay12'
   */
  rtDW->BodyDelay13_DSTATE = rtDW->BodyDelay12_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay12' incorporates:
   *  Delay: '<S1>/BodyDelay11'
   */
  rtDW->BodyDelay12_DSTATE = rtDW->BodyDelay11_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay11' incorporates:
   *  Delay: '<S1>/BodyDelay10'
   */
  rtDW->BodyDelay11_DSTATE = rtDW->BodyDelay10_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay10' incorporates:
   *  Delay: '<S1>/BodyDelay9'
   */
  rtDW->BodyDelay10_DSTATE = rtDW->BodyDelay9_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay9' incorporates:
   *  Delay: '<S1>/BodyDelay8'
   */
  rtDW->BodyDelay9_DSTATE = rtDW->BodyDelay8_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay8' incorporates:
   *  Delay: '<S1>/BodyDelay7'
   */
  rtDW->BodyDelay8_DSTATE = rtDW->BodyDelay7_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay7' incorporates:
   *  Delay: '<S1>/BodyDelay6'
   */
  rtDW->BodyDelay7_DSTATE = rtDW->BodyDelay6_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay6' incorporates:
   *  Delay: '<S1>/BodyDelay5'
   */
  rtDW->BodyDelay6_DSTATE = rtDW->BodyDelay5_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay5' incorporates:
   *  Delay: '<S1>/BodyDelay4'
   */
  rtDW->BodyDelay5_DSTATE = rtDW->BodyDelay4_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay4' incorporates:
   *  Delay: '<S1>/BodyDelay3'
   */
  rtDW->BodyDelay4_DSTATE = rtDW->BodyDelay3_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay3' incorporates:
   *  Delay: '<S1>/BodyDelay2'
   */
  rtDW->BodyDelay3_DSTATE = rtDW->BodyDelay2_DSTATE;

  /* Update for Delay: '<S1>/BodyDelay2' incorporates:
   *  Update for Inport: '<Root>/Input'
   */
  rtDW->BodyDelay2_DSTATE = Input;

  /* Update for Delay: '<S1>/BodyDelay18' */
  rtDW->BodyDelay18_DSTATE = rtb_BodyDelay17;
  return Output;
}


/*-----------------------------------------------------------------------------------------------------
  Экспоненциальный целочисленный фильтр

  \param flt
  \param input

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t Exponential_filter(T_exp_filter *flt, int32_t input )
{
  int32_t outp;

  if (flt->init == 0)
  {
    flt->v1 = input >> flt->k;
    flt->init = 1;
  }

  outp = input + flt->v1;
  flt->v1 = outp - (outp >> flt->k);

  return outp >> flt->k;
}


