// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-03-01
// 12:52:05
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"



#define  MIN_PWM  (-PWM_STEP_COUNT/2)
#define  MAX_PWM  ( PWM_STEP_COUNT/2)

/*-----------------------------------------------------------------------------------------------------
  Конфигурирование таймера для генерации ШИМ сигнала
  Формируется симметричный ШИМ.
  Таймер сначала нарастает потом спадает
  Используется встроенный формирователь мертвого времени.
  Загружаем только компаратор канала A (верхнего канала). Компаратор B вычисляется автоматически.
  Перезагрузка компаратора происходит при переходе таймера через 0 и через пик


  Учитываем время закрытия силовых транзисторов
  Для CSD18540Q5B время закрытия = 20 ns, время открытия 6    ns. Diode reverse recovery time = 82 ns
  Для STL320N4LF8 время закрытия = 89 ns, время открытия 12.5 ns. Diode reverse recovery time = 60 ns
  Микросхема драйвера  TMC6200 вносит внутреннюю задержку между закрытием одного плеча и открытием другого в 75 ns
  Рекомендуется иметь задержку на 30% больше чем специфицировано время выключчения для транзистра

  Один такт таймера равен 1/120 = 8.3 ns


  \param R_GPT      - указатель на структура таймера
-----------------------------------------------------------------------------------------------------*/
static void _PWM_triangle_2buffered_init(R_GPTA0_Type *R_GPT)
{
  R_GPT->GTWP_b.PRKEY  = 0xA5;    // Разрешаем запись в бит WP этого регистра
  R_GPT->GTWP_b.WP     = 0;       // 0: Enable writes to the register Разрешаем запись в остальные регистры таймера

  R_GPT->GTCR_b.CST    = 0;       // Останавливаем счет

  R_GPT->GTCNT         = 0;       // Обнуляем таймер
  R_GPT->GTPR          = r3ph.gpt_top_val; // Устанавливаем регистр задающий верхний предел таймера
  R_GPT->GTIOR         = 0;       // Очищаем настройки выходов. Все запрещены

  // Отклчючаем все флаги счета внешних импульсов
  R_GPT->GTUPSR        = 0;       // General PWM Timer Up Count Source Select Register
  R_GPT->GTDNSR        = 0;       // General PWM Timer Down Count Source Select Register

  R_GPT->GTUDDTYC_b.UDF= 1;       // Сразу вступает в силу установка инкремента счетчика
  R_GPT->GTUDDTYC_b.UD = 1;       // Счетчик инкрементируется


  R_GPT->GTCR_b.TPCS   = 0;       // Timer Prescaler Select. 0 0 0: PCLKD/1
  R_GPT->GTCR_b.MD     = 5;       // 101: Triangle-wave PWM mode 2 (32-bit transfer at crest and trough) (single buffer or double buffer possible)


  R_GPT->GTINTAD_b.GRPABH = 1;    // 1: Enable same time output level high disable request
                                  // Если оба входа установятся в 1 то будет выдан сигнал запрещения выходов

  R_GPT->GTSSR_b.CSTRT = 1;       // Разрешаем програмный запуск  от регитсра GTSTR
  R_GPT->GTPSR_b.CSTOP = 1;       // Разрешаем програмный останов от регитсра GTSTP
  R_GPT->GTCSR_b.CCLR  = 1;       // Разрешаем програмный сброс  от регитсра GTCTR
  R_GPT->GTST          = 0;       // Очищаем статус


  R_GPT->GTDTCR_b.TDE  = 1;       // Use GTDVU and GTDVD to set the compare match value for negative-phase waveform with dead time automatically in GTCCRB.
  R_GPT->GTDTCR_b.TDBUE= 0;       // 0: Disable GTDVU buffer operation
  R_GPT->GTDTCR_b.TDBDE= 0;       // 0: Disable GTDVD buffer operation
  R_GPT->GTDTCR_b.TDFER= 1;       // 1: Automatically set the value written to GTDVU to GTDVD
  R_GPT->GTDVU_b.GTDVU = PWM_DEAD_TIME_VAL;

  R_GPT->GTBER_b.BD    = 0;       // Снимаем запрещение буфферизированной записи в регистры GTCCR, GTPR, GTADTR, GTDV

  R_GPT->GTBER_b.CCRA  = 1;       // GTCCRA Buffer Operation. 01:  Single buffer operation (GTCCRA ↔ GTCCRC)
  R_GPT->GTCCRA        = MIN_PWM_COMPARE_VAL; // Загружаем  значение в компаратор A. Устанавливаем минимальную длительность начального импульса
  R_GPT->GTCCRC        = MIN_PWM_COMPARE_VAL; // Загружаем первое буфферизированное  значение в компаратор A.
  R_GPT->GTIOR_b.GTIOA = 0x7;     // Set initial output low, Retain output at cycle end, Toggle output at GTCCRA compare match
  R_GPT->GTIOR_b.GTIOB = 0x1B;    // Set initial output high, Retain output at cycle end, Toggle output at GTCCRB compare match

  R_GPT->GTIOR_b.OADFLT= 0;       // 0: Output low on GTIOCA pin when counting is stopped
  R_GPT->GTIOR_b.OBDFLT= 0;       // 0: Output low on GTIOCB pin when counting is stopped

  R_GPT->GTIOR_b.OADF  = 2;       // 1 0: Set GTIOCA pin to 0 on output disable
  R_GPT->GTIOR_b.OBDF  = 2;       // 1 0: Set GTIOCB pin to 0 on output disable

  R_GPT->GTIOR_b.OAE   = 1;       // GTIOCA Pin Output Enable
  R_GPT->GTIOR_b.OBE   = 1;       // GTIOCB Pin Output Enable


  __DSB();                        // Ожидаем пока все данные будут записаны в периферию
}

/*-----------------------------------------------------------------------------------------------------
  Программирование подачи сигнала на АЦП для организации синхронизированной с ШИМ выборкой

  \param void
-----------------------------------------------------------------------------------------------------*/
static void _PWM_set_ADC_trigger(void)
{
  //
  // Передаем новое значение в компаратор тригера АЦП на гребне треугольника. Т.е. когда счетчик таймера достиг максимального значения
  R_GPTA0->GTBER_b.ADTTA = 1;          // GTADTRA Buffer Transfer Timing Select. 01: Transfer at crest. Передача на гребне
  R_GPTA0->GTADTRA       = r3ph.gpt_top_val - ADC_SAMPLING_ADVANCE; // Назначаем момент подачи сигнала триггера ADC. В данном случае тригеер сработает чуть раньше середины импульса ШИМ, чтобы АЦП корректно отработало выборку
  R_GPTA0->GTADTBRA      = r3ph.gpt_top_val - ADC_SAMPLING_ADVANCE; // Назначаем момент подачи сигнала триггера ADC в буфферный регистр
  R_GPTA0->GTINTAD_b.ADTRAUEN = 1;     // Compare Match (Up-Counting) A/D Converter Start Request Interrupt Enable
                                       // Разрешаем выдачу сигнала триггера A ADC только в фазе нарастания значений счетчика. Чтобы не было двух сигналов от компаратора - при нарастании счетчика и при убывании счетчика

  R_GPTA0->GTBER_b.ADTTB = 2;          // GTADTRB Buffer Transfer Timing Select. 10: Transfer at trough. Передача на нуле
  R_GPTA0->GTADTRB       = ADC_SAMPLING_ADVANCE; // Назначаем момент подачи сигнала триггера ADC. В данном случае тригеер сработает чуть раньше середины импульса ШИМ, чтобы АЦП корректно отработало выборку
  R_GPTA0->GTADTBRB      = ADC_SAMPLING_ADVANCE; // Назначаем момент подачи сигнала триггера ADC в буфферный регистр
  R_GPTA0->GTINTAD_b.ADTRBDEN = 1;     // Compare Match (DownCounting) A/D Converter Start Request Enable
                                       // Разрешаем выдачу сигнала триггера B ADC только в фазе спада значений счетчика. Чтобы не было двух сигналов от компаратора - при нарастании счетчика и при убывании счетчика

  __DSB();                             // Ожидаем пока все данные будут записаны в периферию
}


/*-----------------------------------------------------------------------------------------------------
  Инициализация таймеров GPT0, GPT1, GPT2 (согласно обозначению в даташите) или R_GPTA0, R_GPTA1, R_GPTA2 (согласно обозначению в хидере)

  Настройка ШИМ на треугольный режим с перегрузкой на впадине и на пике

  Таймер тактируется частотой PCLKD = 120 МГц

  \param freq       - частота ШИМ

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t PWM_3ph_triangle_2buffered_init(uint32_t pwm_freq)
{
  r3ph.gpt_top_val =(PCLKD_FREQ / pwm_freq)- 1;

  if (r3ph.gpt_top_val < PWM_STEP_COUNT)
  {
    // Не даем ошибочно выбрать слишком высокую частоту ШИМ
    APPLOG("Wrong PWM frequency value %d", pwm_freq);
    return RES_ERROR;
  }

  // Заполняем массив загрузочными данными компаратор для разных значений модуляции
  for (uint32_t i=0; i < PWM_STEP_COUNT; i++)
  {
    int32_t comp_val =(r3ph.gpt_top_val * (PWM_STEP_COUNT - i)) / PWM_STEP_COUNT;

    // Не даем прекратиться PWM и не даем импульсу стать слишком коротким
    if (comp_val < MIN_PWM_COMPARE_VAL)
    {
      comp_val = MIN_PWM_COMPARE_VAL;
    }
    else if ((r3ph.gpt_top_val - comp_val) < MIN_PWM_COMPARE_VAL)
    {
      comp_val = r3ph.gpt_top_val - MIN_PWM_COMPARE_VAL;
    }
    r3ph.pwm_indx_to_comp[i] = comp_val;

  }

  R_MSTP->MSTPCRD_b.MSTPD5 = 0;                   // Разрешаем работу модулей GPT ch7-ch0

  _PWM_triangle_2buffered_init(R_GPTA0);
  _PWM_triangle_2buffered_init(R_GPTA1);
  _PWM_triangle_2buffered_init(R_GPTA2);
  _PWM_set_ADC_trigger();

  return RES_OK;
}



/*-----------------------------------------------------------------------------------------------------


  \param phase
  \param pwm_lev
-----------------------------------------------------------------------------------------------------*/
void Post_phase_pwm(uint8_t phase, uint32_t pwm_lev)
{
  if (pwm_lev > PWM_STEP_COUNT) return;

  switch (phase)
  {
  case PHASE_U:
    r3ph.gpt_U_pwm_val = pwm_lev;
    break;
  case PHASE_V:
    r3ph.gpt_V_pwm_val = pwm_lev;
    break;
  case PHASE_W:
    r3ph.gpt_W_pwm_val = pwm_lev;
    break;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param pwm_U - Значение PWM фазы U (A) от 0 до
  \param pwm_V - Значение PWM фазы V (B) от 0 до
  \param pwm_W - Значение PWM фазы W (C) от 0 до
-----------------------------------------------------------------------------------------------------*/
void PWM_send(int32_t pwm_U, int32_t pwm_V, int32_t pwm_W)
{
  __disable_interrupt();
  r3ph.gpt_U_pwm_val = pwm_U;
  r3ph.gpt_V_pwm_val = pwm_V;
  r3ph.gpt_W_pwm_val = pwm_W;
  __enable_interrupt();

}

