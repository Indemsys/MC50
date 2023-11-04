// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2018.10.22
// 13:51:56
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


/*-----------------------------------------------------------------------------------------------------


  \param x
  \param xLowerLimit
  \param xUpperLimit

  \return float
-----------------------------------------------------------------------------------------------------*/
static float _saturate(float x, float xLowerLimit, float xUpperLimit)
{
  if (x < xLowerLimit) x = xLowerLimit;
  else if (x > xUpperLimit) x = xUpperLimit;
  return x;
}

static int32_t _saturate_int(int32_t x, int32_t xLowerLimit, int32_t xUpperLimit)
{
  if (x < xLowerLimit) x = xLowerLimit;
  else if (x > xUpperLimit) x = xUpperLimit;
  return x;
}

/*-----------------------------------------------------------------------------------------------------
  Функция модулятора на основе исходных текстов MATLAB функции pwmSwitchingTimeTwoLevel
  Внутреннии переменные tgX_on и tgX_off выражаются в абсолютном времени

  \param vabc  - напряжение на фазах нормированное к 1
  \param tsw   - период PWM
  \param mode  - способ модуляции
  \param duty  - скважность модуляции на каждой фазе приведенная к диапазону -1..1
-----------------------------------------------------------------------------------------------------*/
void PWM_modulator_abs_float(T_float_3ph *vabc, float tsw, uint8_t mode, T_float_3ph *duty)
{
  float teff;
  float t0;
  float toffset;
  float tswh = 0.5f * tsw; // half switching period

  float tmax;
  float tmin;

  float tga_off_h;
  float tgb_off_h;
  float tgc_off_h;

  float tga_on;
  float tgb_on;
  float tgc_on;

  float tga_off;
  float tgb_off;
  float tgc_off;

  float tas = vabc->a * tswh / sqrtf(3);
  float tbs = vabc->b * tswh / sqrtf(3);
  float tcs = vabc->c * tswh / sqrtf(3);

  // Находим максимум
  if (tas > tbs)
  {
    if (tas > tcs) tmax = tas;
    else tmax = tcs;
  }
  else
  {
    if (tbs < tcs) tmax = tcs;
    else tmax = tbs;
  }

  // Находим минимум
  if (tas < tbs)
  {
    if (tas < tcs) tmin = tas;
    else tmin = tcs;
  }
  else
  {
    if (tbs > tcs) tmin = tcs;
    else tmin = tbs;
  }

  teff = tmax - tmin; // effective time
  t0 = tswh - teff;

  switch (mode)
  {
  case MDL_SINUSOIDAL:
    toffset = 0.5f * tswh;
    break;
  case MDL_CSVPWM:
    toffset = t0 / 2.0f - tmin;
    break;
  case MDL_DEGREE_60_DISCONTINUOUS:
    if ((tmin+tmax) >= 0) toffset = tswh - tmax;
    else toffset = -tmin;
    break;
  case MDL_DPWM_120_TOP:
    toffset = tswh - tmax;
    break;
  case MDL_DPWM_120_BOT:
    toffset = -tmin;
    break;
  }

  // OFF sequency gating time within half-cycle (Tsw/2) range
  tga_off_h = tas + toffset;
  tgb_off_h = tbs + toffset;
  tgc_off_h = tcs + toffset;

  // ON sequency gating time within half-cycle (Tsw/2) range, which is also the gating time within whole cycle (Tsw) range
  tga_on = tswh - tga_off_h;
  tgb_on = tswh - tgb_off_h;
  tgc_on = tswh - tgc_off_h;

  // OFF sequency gating time within whole cycle (Tsw) range
  tga_off = tga_off_h + tswh;
  tgb_off = tgb_off_h + tswh;
  tgc_off = tgc_off_h + tswh;

  //over modulation (vabc is large, e.g., 220 V peak vabc vs. 400 Vdc for SPWM)
  //(1). correct ON gating times
  //(2). correct OFF gating times
  //0<T_on<Tswh
  tga_on = _saturate(tga_on,0,tswh);
  tgb_on = _saturate(tgb_on,0,tswh);
  tgc_on = _saturate(tgc_on,0,tswh);
  //Tswh<T_off<Tsw
  tga_off = _saturate(tga_off,tswh,tsw);
  tgb_off = _saturate(tgb_off,tswh,tsw);
  tgc_off = _saturate(tgc_off,tswh,tsw);


  duty->a = 2 * (tga_off - tga_on) / tsw - 1;
  duty->b = 2 * (tgb_off - tgb_on) / tsw - 1;
  duty->c = 2 * (tgc_off - tgc_on) / tsw - 1;

}

/*-----------------------------------------------------------------------------------------------------
  Функция модулятора на основе исходных текстов MATLAB функции pwmSwitchingTimeTwoLevel
  Внутреннии переменные tgX_on и tgX_off выражаются в условном времени


  \param vabc  - напряжение на фазах нормированное к 1
  \param mode  - способ модуляции
  \param duty  - скважность модуляции на каждой фазе приведенная к диапазону -1..1
-----------------------------------------------------------------------------------------------------*/
void PWM_modulator_float(T_float_3ph *vabc, uint8_t mode, T_float_3ph *duty)
{
  float teff;
  float t0;
  float toffset;

  float tmax;
  float tmin;

  float tga_off_h;
  float tgb_off_h;
  float tgc_off_h;

  float tga_on;
  float tgb_on;
  float tgc_on;

  float tga_off;
  float tgb_off;
  float tgc_off;

  float tas = vabc->a * 0.5f / sqrtf(3);
  float tbs = vabc->b * 0.5f / sqrtf(3);
  float tcs = vabc->c * 0.5f / sqrtf(3);

  // Находим максимум
  if (tas > tbs)
  {
    if (tas > tcs) tmax = tas;
    else tmax = tcs;
  }
  else
  {
    if (tbs < tcs) tmax = tcs;
    else tmax = tbs;
  }

  // Находим минимум
  if (tas < tbs)
  {
    if (tas < tcs) tmin = tas;
    else tmin = tcs;
  }
  else
  {
    if (tbs > tcs) tmin = tcs;
    else tmin = tbs;
  }

  teff = tmax - tmin; // effective time
  t0   = 0.5f - teff;

  switch (mode)
  {
  case MDL_SINUSOIDAL:
    toffset = 0.5f * 0.5f;
    break;
  case MDL_CSVPWM:
    toffset = t0 / 2.0f - tmin;
    break;
  case MDL_DEGREE_60_DISCONTINUOUS:
    if ((tmin+tmax) >= 0) toffset = 0.5f - tmax;
    else toffset = -tmin;
    break;
  case MDL_DPWM_120_TOP:
    toffset = 0.5f - tmax;
    break;
  case MDL_DPWM_120_BOT:
    toffset = -tmin;
    break;
  }

  // OFF sequency gating time within half-cycle (Tsw/2) range
  tga_off_h = tas + toffset;
  tgb_off_h = tbs + toffset;
  tgc_off_h = tcs + toffset;

  // ON sequency gating time within half-cycle (Tsw/2) range, which is also the gating time within whole cycle (Tsw) range
  tga_on = 0.5f - tga_off_h;
  tgb_on = 0.5f - tgb_off_h;
  tgc_on = 0.5f - tgc_off_h;

  // OFF sequency gating time within whole cycle (Tsw) range
  tga_off = tga_off_h + 0.5f;
  tgb_off = tgb_off_h + 0.5f;
  tgc_off = tgc_off_h + 0.5f;

  //over modulation (vabc is large, e.g., 220 V peak vabc vs. 400 Vdc for SPWM)
  //(1). correct ON gating times
  //(2). correct OFF gating times
  //0<T_on<Tswh
  tga_on = _saturate(tga_on,0,0.5f);
  tgb_on = _saturate(tgb_on,0,0.5f);
  tgc_on = _saturate(tgc_on,0,0.5f);
  //Tswh<T_off<Tsw
  tga_off = _saturate(tga_off,0.5f,1);
  tgb_off = _saturate(tgb_off,0.5f,1);
  tgc_off = _saturate(tgc_off,0.5f,1);

  // Приведение скважности к диапазону -1...1
  duty->a = 2 * (tga_off - tga_on)- 1;
  duty->b = 2 * (tgb_off - tgb_on)- 1;
  duty->c = 2 * (tgc_off - tgc_on)- 1;

}

#define  H1      0x7FFFFFFFul
#define  H05     0x40000000ul
#define  H025    0x20000000ul
//#define  SCALE_K 0x49e27498ull  // 49e69d16
#define  SCALE_K 0x49e00000ull

/*-----------------------------------------------------------------------------------------------------
  Функция принимает три синусоиды со сдвигом 120 град в формате с фиксированной точкой. Бервый бит знак и 31 бит мантиссы = fixdt(1,32,31) . Диапазон -1...1
  На выходе получаем 3-и сигнала модуляции в формате с фиксированной точкой. Бервый бит знак и 31 бит мантиссы = fixdt(1,32,31) . Диапазон -1...1



  \param vabc
  \param mode
  \param duty
-----------------------------------------------------------------------------------------------------*/
void PWM_modulator_integer(T_int32_3ph *vabc, uint8_t mode, T_int32_3ph *duty, T_SVPWM_debug *dbg)
{
  int32_t toffset;
  int32_t tmax;
  int32_t tmin;
  int32_t tga_off_h;
  int32_t tgb_off_h;
  int32_t tgc_off_h;
  int32_t tga_on;
  int32_t tgb_on;
  int32_t tgc_on;
  int32_t tga_off;
  int32_t tgb_off;
  int32_t tgc_off;
  int32_t tas;
  int32_t tbs;
  int32_t tcs;
  int64_t v;


  if (mode == 0)
  {
    if (vabc->a > 0) duty->a = 0x7FFFFFFF;
    else duty->a = 0x80000000;
    if (vabc->b > 0) duty->b = 0x7FFFFFFF;
    else duty->b = 0x80000000;
    if (vabc->c > 0) duty->c = 0x7FFFFFFF;
    else duty->c = 0x80000000;

    if (dbg != NULL)
    {
      dbg->tmin =       0;
      dbg->tmax =       0;
      dbg->toffset =    0;
      dbg->tga_on =     0;
      dbg->tgb_on =     0;
      dbg->tgc_on =     0;
      dbg->tga_off =    0;
      dbg->tgb_off =    0;
      dbg->tgc_off =    0;
    }
    return;
  }

  v =  (int64_t)vabc->a * SCALE_K;
  tas =  v / (0x100000000 * 2l);

  v =  (int64_t)vabc->b * SCALE_K;
  tbs =  v / (0x100000000 * 2l);

  v =  (int64_t)vabc->c * SCALE_K;
  tcs =  v / (0x100000000 * 2l);


  // Находим максимум
  if (tas > tbs)
  {
    if (tas > tcs) tmax = tas;
    else tmax = tcs;
  }
  else
  {
    if (tbs < tcs) tmax = tcs;
    else tmax = tbs;
  }

  // Находим минимум
  if (tas < tbs)
  {
    if (tas < tcs) tmin = tas;
    else tmin = tcs;
  }
  else
  {
    if (tbs > tcs) tmin = tcs;
    else tmin = tbs;
  }

  switch (mode)
  {
  case MDL_CSVPWM:
    toffset =(H05 -(tmax - tmin)) / 2l - tmin;
    break;

  case MDL_DEGREE_60_DISCONTINUOUS:
    if ((tmin+tmax) >= 0) toffset = H05 / 2l - tmax;
    else toffset = -tmin;
    break;

  case MDL_DPWM_120_TOP:
    toffset = H05 / 2l - tmax;
    break;

  case MDL_DPWM_120_BOT:
    toffset = -tmin;
    break;
  }

  // OFF sequency gating time within half-cycle (Tsw/2) range
  tga_off_h = tas + toffset;
  tgb_off_h = tbs + toffset;
  tgc_off_h = tcs + toffset;

  // ON sequency gating time within half-cycle (Tsw/2) range, which is also the gating time within whole cycle (Tsw) range
  tga_on = H05 - tga_off_h;
  tgb_on = H05 - tgb_off_h;
  tgc_on = H05 - tgc_off_h;

  // OFF sequency gating time within whole cycle (Tsw) range
  tga_off = tga_off_h + H05;
  tgb_off = tgb_off_h + H05;
  tgc_off = tgc_off_h + H05;

  //(1). correct ON gating times
  //(2). correct OFF gating times
  //0<T_on<Tswh
  tga_on = _saturate_int(tga_on,0,H05-1);
  tgb_on = _saturate_int(tgb_on,0,H05-1);
  tgc_on = _saturate_int(tgc_on,0,H05-1);

  //Tswh<T_off<Tsw
  tga_off = _saturate_int(tga_off,H05,H1);
  tgb_off = _saturate_int(tgb_off,H05,H1);
  tgc_off = _saturate_int(tgc_off,H05,H1);

  // Приведение скважности к диапазону -1...1
  duty->a =((tga_off - tga_on)- H025 - 1);
  duty->b =((tgb_off - tgb_on)- H025 - 1);
  duty->c =((tgc_off - tgc_on)- H025 - 1);
  if (mode == 1)
  {
    duty->a -= H025;
    duty->b -= H025;
    duty->c -= H025;
  }

  duty->a *= 4l;
  duty->b *= 4l;
  duty->c *= 4l;

  if (dbg != NULL)
  {
    dbg->tmin =       tmin;
    dbg->tmax =       tmax;
    dbg->toffset =    toffset;
    dbg->tga_on =     tga_on;
    dbg->tgb_on =     tgb_on;
    dbg->tgc_on =     tgc_on;
    dbg->tga_off =    tga_off;
    dbg->tgb_off =    tgb_off;
    dbg->tgc_off =    tgc_off;
  }

}


/*-----------------------------------------------------------------------------------------------------
  Преобразование нормированной величины заполнения ШИМ в диапазон таблицы загрузочных значений для компараторов

    input            output
  -------------------------------
  -1...0...1  ->  0 ... range
  -------------------------------

  \param pwm           Величина заполнения ШИМ для фаз. Первый бит знак и 31 бит мантиссы = fixdt(1,32,31) . Диапазон -1...1
  \param half_range    Половина диапазона счетчика компаратора.
  \param scale         Масшабирующий амплитуду выхода коэфициент. Первый бит знак и 31 бит мантиссы = fixdt(1,32,31) . Диапазон -1...1
  \param cv            Получаемые величины для загрузки в компраторы гереатора PWM
-----------------------------------------------------------------------------------------------------*/
void PWM_converter(T_int32_3ph *pwm, int32_t half_range, int32_t scale, T_pwm_3ph *cv)
{

  int32_t pwm_scaled;  // Формат fixdt(1,32,31)

  pwm_scaled = (((int64_t)pwm->a*(int64_t)scale) + 0x40000000) >> 31;            // Масштабирование с округлением. Выполняется за 6 тактов
  pwm_scaled = (((int64_t)pwm_scaled * (int64_t)half_range) + 0x40000000) >> 31; // Приведение к диапазону с округлением. Выполняется за 6 тактов
  cv->pa    =  pwm_scaled + half_range;

  pwm_scaled = (((int64_t)pwm->b*(int64_t)scale) + 0x40000000) >> 31;
  pwm_scaled = (((int64_t)pwm_scaled * (int64_t)half_range) + 0x40000000) >> 31;
  cv->pb    =  pwm_scaled + half_range;

  pwm_scaled = (((int64_t)pwm->c*(int64_t)scale) + 0x40000000) >> 31;
  pwm_scaled = (((int64_t)pwm_scaled * (int64_t)half_range) + 0x40000000) >> 31;
  cv->pc    =  pwm_scaled + half_range;

}



