﻿// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-07-13
// 11:51:35
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


/*-----------------------------------------------------------------------------------------------------
  Функция определения состояния датчиков холла при вращении ротора в разные сторны для определения таблиц коммутации

-----------------------------------------------------------------------------------------------------*/
void Search_6step_commutation_sequence(void)
{
  // Таблицы коммутации в одну и другую стороны
  // Значения из таблицы записываются в 3-и младших бита регистра R_GPT_OPS->OPSCR
  // По этим значениям уставливаются 6 сигналов управления силовым драйвером
  uint8_t comm_num_fw[6] = {5,1,3,2,6,4};
  uint8_t comm_num_rv[6] = {6,2,3,1,5,4};

  // Таблицы содержащие результат
  // Напротив таблиц - полученные результаты в случае с мотором PB30960494 (Rev.A) 24Vdc, 4.0A, 90W (6 зубъев, 8 полюсов)
  volatile uint8_t hall_tbl_fw[8];  //   -  {2 6 4 5 1 3} - при тестировнаии результат оказался неверным
  volatile uint8_t hall_tbl_rv[8];  //   -  {5 4 6 2 3 1} - при тестировнаии результат подтвердился, на его основе была создана таблица и для обратного движения

  mot_cbl.pwm_val = 70;
  GPT0_update_PWM_value(); // Задаем ШИМ такой скважности чтобы ротор уверенно прокуручивался.

  // Вращаем некоторое время асинхронно в одну сторону делая задержки после каждого шага
  // и снимая показания сенсоров в таблицу по индексу равному номеру шага
  for (uint32_t n=0; n < 100; n++)
  {
    for (uint32_t i=0; i < 6; i++)
    {
      R_GPT_OPS->OPSCR = mot_cbl.opscr.w | comm_num_fw[i];

      if (mot_cbl.opscr.EN == 0)
      {
        R_GPT_OPS->OPSCR_b.EN = 1;
        mot_cbl.opscr.EN      = 1;
      }
      Wait_ms(3);
      hall_tbl_fw[i] = R_IOPORT5->PCNTR2 & 0x7;
    }
  }

  // Вращаем некоторое время асинхронно в противоположную сторону делая задержки после каждого шага
  // и снимая показания сенсоров в таблицу по индексу равному номеру шага
  for (uint32_t n=0; n < 100; n++)
  {
    for (uint32_t i=0; i < 6; i++)
    {
      R_GPT_OPS->OPSCR = mot_cbl.opscr.w | comm_num_rv[i];

      if (mot_cbl.opscr.EN == 0)
      {
        R_GPT_OPS->OPSCR_b.EN = 1;
        mot_cbl.opscr.EN      = 1;
      }
      Wait_ms(3);
      hall_tbl_rv[i] = R_IOPORT5->PCNTR2 & 0x7;
    }
  }
  R_GPT_OPS->OPSCR_b.EN = 0;
}


