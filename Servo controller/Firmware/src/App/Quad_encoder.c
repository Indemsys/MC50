// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-10-13
// 17:58:56
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

T_QuadEncoder_cbl  qenc;

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void QEnc_reinit(void)
{
  qenc.ra_filtr.arr   = qenc.ra_filtr_arr;
  qenc.ra_filtr.len   = QENC_RAV_FLTR_LEN;
  qenc.ra_filtr.init  = 0;
  qenc.capt_prev      = -1;
  qenc.delta          = 0;
  qenc.delta_aver     = 0;
}


/*-----------------------------------------------------------------------------------------------------
  Измерение скорости вращения ротора DC мотора и направления вращения c квадратурного энкодера
  Измерение производится с помощью датчиков Холла и логики захвата таймеров GPT_11 и GPT_12
  Логика захвата реагирует на подъем и на спад


  При вращении CW
       _________        _________        _________
       |       |        |       |        |       |
   V   |       |________|       |________|       |________

            _________        _________        _________
   W        |       |        |       |        |       |
            |       |________|       |________|       |________


-----------------------------------------------------------------------------------------------------*/
void QEnc_measure_speed_and_direction(void)
{
  uint32_t  st;
  int32_t   val;
  int32_t   delta = 0;

  st = R_GPTB3->GTST;  // Читаем флаги состояния capture таймера  GPT3211


  // ......................................................
  // Обработка сигнала ENCV
  // ......................................................
  if (st & BIT(1))
  {
    // Input capture/compare match of GTCCRB occurred
    R_GPTB3->GTST = 0;
    val = R_GPTB3->GTCCRB;

    if (qenc.capt_prev > 0)
    {
      if (val > qenc.capt_prev)
      {
        delta = val - qenc.capt_prev;
      }
      else
      {
        delta =  0x7FFFFFFF -(qenc.capt_prev - val -1);
      }
    }

    // BIT(1) - сигнал ENC_V,  BIT(2) - сигнал ENV_W
    qenc.bitmask = R_IOPORT5->PCNTR2 & 0x6;  // Читаем сигналы датчиков здесь чтобы они были прочитаны не раньше чем стработает capture логика

    if (qenc.capt_prev > 0)
    {
      if (((qenc.bitmask & BIT(1)) != 0) && ((qenc.bitmask_prev & BIT(2)) == 0))
      {
        qenc.ror_dir = QENC_ROTATION_CW;
      }
      else if (((qenc.bitmask & BIT(1)) == 0) && ((qenc.bitmask_prev & BIT(2)) != 0))
      {
        qenc.ror_dir = QENC_ROTATION_CW;
      }
      else
      {
        qenc.ror_dir = QENC_ROTATION_CCW;
      }
    }

    qenc.bitmask_prev = qenc.bitmask;
    qenc.capt_prev = val;

    R_GPTB5->GTCLR_b.CCLR13 = 1;  // Сброс счетчика отслеживающего остановку вращения
    R_GPTB5->GTST           = 0;  // Сбрасываем флаг переполнения
  }


  st = R_GPTB4->GTST;

  // ......................................................
  // Обработка сигнала ENCW
  // ......................................................
  if (st & BIT(0))
  {
    // Input capture/compare match of GTCCRA occurred
    R_GPTB4->GTST = 0;
    val = R_GPTB4->GTCCRA;

    if (qenc.capt_prev > 0)
    {
      if (val > qenc.capt_prev)
      {
        delta = val - qenc.capt_prev;
      }
      else
      {
        delta =  0x7FFFFFFF -(qenc.capt_prev - val -1);
      }
    }

    // BIT(1) - сигнал ENC_V,  BIT(2) - сигнал ENV_W
    qenc.bitmask = R_IOPORT5->PCNTR2 & 0x6;  // Читаем сигналы датчиков здесь чтобы они были прочитаны не раньше чем стработает capture логика

    if (qenc.capt_prev > 0)
    {
      if (((qenc.bitmask & BIT(2)) == 0) && ((qenc.bitmask_prev & BIT(1)) == 0))
      {
        qenc.ror_dir = QENC_ROTATION_CW;
      }
      else if (((qenc.bitmask & BIT(2)) != 0) && ((qenc.bitmask_prev & BIT(1)) != 0))
      {
        qenc.ror_dir = QENC_ROTATION_CW;
      }
      else
      {
        qenc.ror_dir = QENC_ROTATION_CCW;
      }
    }

    qenc.bitmask_prev = qenc.bitmask;
    qenc.capt_prev    = val;

    R_GPTB5->GTCLR_b.CCLR13 = 1;  // Сброс счетчика отслеживающего остановку вращения
    R_GPTB5->GTST           = 0;  // Сбрасываем флаг переполнения
  }


  // ......................................................
  // Блок линейно понижающий скорость в случает отсутствия сигналов с датчиков квадратурного энкодера
  // ......................................................
  if (R_GPTB5->GTST & BIT(6))  // Проверяем флаг TCFPO. Overflow Flag
  {
    // В случае переполения тамера сразу отмечаем скорость как нулевую.
    // Поскольку переполение таймера происходит с периодом в две секунды, то такую низкую скрость принимаем как нулевую
    QEnc_reinit();
    R_GPTB5->GTST           = 0;  // Сбрасываем флаг переполнения
  }
  else if (delta != 0)
  {
    qenc.delta      = delta;
    qenc.delta_aver = RunAverageFilter_int32_N(&qenc.ra_filtr, delta);
  }
 // else
 // {
 //   if (qenc.delta_prev != 0)
 //   {
 //     int32_t signal_absence_span = R_GPTB5->GTCNT;
 //     if (signal_absence_span > (qenc.delta_prev*2))
 //     {
 //       qenc.delta      = signal_absence_span;
 //       qenc.delta_aver = RunAverageFilter_int32_N(&qenc.ra_filtr, signal_absence_span);
 //     }
 //   }
 // }
}

