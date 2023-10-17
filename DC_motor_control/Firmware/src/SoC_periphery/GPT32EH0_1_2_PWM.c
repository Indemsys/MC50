// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-03-01
// 12:52:05
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

// Объявляем прерывания таким образом чтобы это не конфликтовало с методом принятым в библиотеке SSP
static void  GPT0_underflow_isr(void);

SSP_VECTOR_DEFINE_CHAN(GPT0_underflow_isr, GPT, COUNTER_UNDERFLOW, 0);

static uint32_t gpt_pwm_cycle_set;

IRQn_Type gpt0_underflow_int_num;

volatile uint8_t   gpt0_out_mode;
volatile uint8_t   gpt1_out_mode;
volatile uint8_t   gpt2_out_mode;

/*-----------------------------------------------------------------------------------------------------
  В даном варианте не используем этот обработчик

  \param void
-----------------------------------------------------------------------------------------------------*/
static void  GPT0_underflow_isr(void)
{
  ITM_EVENT8(3,0);
  R_GPTA0->GTST = 0;
  R_ICU->IELSRn_b[gpt0_underflow_int_num].IR = 0;  // Сбрасываем IR флаг в ICU



  NVIC_DisableIRQ(gpt0_underflow_int_num);

  ITM_EVENT8(3,1);
}


/*-----------------------------------------------------------------------------------------------------
  Конфигурирование таймера для генерации ШИМ сигнала

  Учитываем время закрытия силовых транзисторов
  Для CSD18540Q5B время закрытия = 20 ns, время открытия 6    ns. Diode reverse recovery time = 82 ns
  Для STL320N4LF8 время закрытия = 89 ns, время открытия 12.5 ns. Diode reverse recovery time = 60 ns
  Микросхема драйвера  TMC6200 вносит внутреннюю задержку между закрытием одного плеча и открытием другого в 75 ns
  Рекомендуется иметь задержку на 30% больше чем специфицировано время выключчения для транзистра

  Один такт таймера равен 1/120 = 8.3 ns

  \param GPT
-----------------------------------------------------------------------------------------------------*/
static void GPT_push_pull_PWM_init(R_GPTA0_Type *R_GPT)
{
  R_GPT->GTWP_b.PRKEY  = 0xA5;    // Разрешаем запись в бит WP этого регистра
  R_GPT->GTWP_b.WP     = 0;       // 0: Enable writes to the register Разрешаем запись в остальные регистры таймера

  R_GPT->GTCR_b.CST    = 0;       // Останавливаем счет

  R_GPT->GTCNT         = 0;       // Обнуляем таймер
  R_GPT->GTPR          = gpt_pwm_cycle_set; // Устанавливаем регистр задающий верхний предел таймера
  R_GPT->GTIOR         = 0;       // Очищаем настройки выходов. Все запрещены

  // Отклчючаем все флаги счета внешних импульсов
  R_GPT->GTUPSR        = 0;       // General PWM Timer Up Count Source Select Register
  R_GPT->GTDNSR        = 0;       // General PWM Timer Down Count Source Select Register

  R_GPT->GTUDDTYC_b.UDF= 1;       // Сразу вступает в силу установка инкремента счетчика
  R_GPT->GTUDDTYC_b.UD = 1;       // Счетчик инкрементируется


  R_GPT->GTCR_b.TPCS   = 0;       // Timer Prescaler Select. 0 0 0: PCLKD/1
  R_GPT->GTCR_b.MD     = 4;       // 100: Triangle-wave PWM mode 1 (32-bit transfer at trough) (single buffer or double buffer possible)

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

  R_GPT->GTBER_b.CCRA  = 1;       // GTCCRA Buffer Operation. 01: Single buffer operation (GTCCRA <- GTCCRC).
  R_GPT->GTCCRA        = MIN_PWM_COMPARE_VAL; // Загружаем начальное значение в компаратор A. Устанавливаем минимальную длительность начального импульса
  R_GPT->GTCCRC        = MIN_PWM_COMPARE_VAL; // Загружаем буфферизированное начальное значение в компаратор A.
  R_GPT->GTIOR_b.GTIOA = 0x7;     // Set initial output low, Retain output at cycle end, Toggle output at GTCCRA compare match
  R_GPT->GTIOR_b.GTIOB = 0x1B;    // Set initial output high, Retain output at cycle end, Toggle output at GTCCRB compare match

  R_GPT->GTIOR_b.OADFLT= 0;       // 0: Output low on GTIOCA pin when counting is stopped
  R_GPT->GTIOR_b.OBDFLT= 0;       // 0: Output low on GTIOCB pin when counting is stopped

  R_GPT->GTIOR_b.OADF  = 2;       // 1 0: Set GTIOCA pin to 0 on output disable
  R_GPT->GTIOR_b.OBDF  = 2;       // 1 0: Set GTIOCB pin to 0 on output disable

  R_GPT->GTIOR_b.OAE   = 1;       // GTIOCA Pin Output Enable
  R_GPT->GTIOR_b.OBE   = 1;       // GTIOCB Pin Output Enable


  // В случае использования Dead Time Control Register запись в регистр компаратора GTCCRB запрещена. Компаратор устаноавливается автоматически
  //  R_GPT->GTBER_b.CCRB  = 1;       // GTCCRB Buffer Operation. 01: Single buffer operation (GTCCRB <- GTCCRE).
  //  R_GPT->GTCCRB        = gpt_cycle_set - 0x20; // Загружаем начальное значение в компаратор B. Устанавливаем минимальную длительность начального импульса
  //  R_GPT->GTCCRE        = gpt_cycle_set - 0x20; // Загружаем буфферизированное начальное значение в компаратор B
  //  R_GPT->GTIOR_b.GTIOB = 0x13;    // Set initial output high, Retain output at cycle end, Toggle output at GTCCRA/GTCCRB compare match
  //                                  // Здесь сигнал в начале имеет низкий уровень , во время нарастания счетчика на событии compare match он принимает высокий уровень и остается таким
  //                                  // пока на фазе убывания счетчика снова не произодет событие compare match и сигнал не переключится на 0


  __DSB();                     // Ожидаем пока все данные будут записаны в периферию
}

/*-----------------------------------------------------------------------------------------------------
  Программирование подачи сигнала на АЦП для организации синхронизированной с ШИМ выборкой


  \param void
-----------------------------------------------------------------------------------------------------*/
static void GPT_set_trigger_ADC(void)
{
  //
  // Передаем новое значение в компаратор тригера АЦП на гребне треугольника. Т.е. когда счетчик таймера достиг максимального значения
  R_GPTA0->GTBER_b.ADTTA = 1;          // GTADTRA Buffer Transfer Timing Select. 01: Transfer at crest. Передача на гребне
  R_GPTA0->GTADTRA       = gpt_pwm_cycle_set - 0x10; // Назначаем момент подачи сигнала триггера ADC. В данном случае тригеер сработает чуть раньше середины импульса ШИМ, чтобы АЦП корректно отработало выборку
  R_GPTA0->GTADTBRA      = gpt_pwm_cycle_set - 0x10; // Назначаем момент подачи сигнала триггера ADC в буфферный регистр
  R_GPTA0->GTINTAD_b.ADTRAUEN = 1;     // Разрешаем выдачу сигнала триггера ADC только в фазе нарастания значений счетчика. Чтобы не было двух сигналов от компаратора - при нарастании счетчика и при убывании счетчика
  __DSB();                             // Ожидаем пока все данные будут записаны в периферию

}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void GPT_0_1_2_PWM_start(void)
{
  R_GPTA0->GTSTR = BIT(0)+ BIT(1)+ BIT(2);  // Стартуем таймеры GPT0, GPT1, GPT2 одновременно
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void GPT_0_1_2_PWM_stop(void)
{
  R_GPTA0->GTSTP = BIT(0)+ BIT(1)+ BIT(2);  // Останавливаем таймеры GPT0, GPT1, GPT2 одновременно
  __DSB();
}


/*-----------------------------------------------------------------------------------------------------
  Инициализация таймеров GPT0, GPT1, GPT2 (согласно обозначению в даташите) или R_GPTA0, R_GPTA1, R_GPTA2 (согласно обозначению в хидере)

  Настройка ШИМ на треугольный режим с перегрузкой на впадине

  Таймер тактируется частотой PCLKD = 120 МГц

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t GPT_0_1_2_PWM_init(uint32_t freq)
{
  gpt_pwm_cycle_set = PCLKD_FREQ / (freq * 2)- 1;
  if (gpt_pwm_cycle_set < 0x21) return RES_ERROR; // Не даем ошибочно выбрать слишком высокую частоту ШИМ

  R_MSTP->MSTPCRD_b.MSTPD5 = 0;               // Разрешаем работу модулей GPT ch7-ch0

  GPT_push_pull_PWM_init(R_GPTA0);
  GPT_push_pull_PWM_init(R_GPTA1);
  GPT_push_pull_PWM_init(R_GPTA2);
  GPT_set_trigger_ADC();

  return RES_OK;
}




/*-----------------------------------------------------------------------------------------------------
  Установка состояния на выходе фазы U 3-фазного инвертера

  val =   0     - нижний ключ постоянно открыт, верхний постоянно закрыт
  val = 100     - нижний ключ постоянно закрыт, верхний постоянно открыт
  val =  -1     - нижний ключ постоянно закрыт, верхний постоянно закрыт
  0 < val < 100 - На выходе сигнал ШИМ с коэфициентом заполнения равным val в процентах
                  Установка ШИМ буфферизирована, и вступит в действие после окончания текущего цикла ШИМ

-----------------------------------------------------------------------------------------------------*/
void Phase_U_mode(int32_t val)
{
  if (val >= 100)    gpt0_out_mode = FL_PHASE_PULL_UP;
  else if (val == 0) gpt0_out_mode = FL_PHASE_PULL_DOWN;
  else if (val < 0)  gpt0_out_mode = FL_PHASE_Z_STATE;
  else
  {
    val =(gpt_pwm_cycle_set * (100 - val)) / 100;

    // Не даем прекратиться PWM и не даем импульсу стать слишком коротким
    if (val < MIN_PWM_COMPARE_VAL)
    {
      val = MIN_PWM_COMPARE_VAL;
    }
    else if ((gpt_pwm_cycle_set - val) < MIN_PWM_COMPARE_VAL)
    {
      val = gpt_pwm_cycle_set - MIN_PWM_COMPARE_VAL;
    }
    R_GPTA0->GTCCRC = val;
    gpt0_out_mode = FL_PHASE_PWM_ON;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Установка состояния на выходе фазы V 3-фазного инвертера

  val =   0     - нижний ключ постоянно открыт, верхний постоянно закрыт
  val = 100     - нижний ключ постоянно закрыт, верхний постоянно открыт
  val =  -1     - нижний ключ постоянно закрыт, верхний постоянно закрыт
  0 < val < 100 - На выходе сигнал ШИМ с коэфициентом заполнения равным val в процентах
                  Установка ШИМ буфферизирована, и вступит в действие после окончания текущего цикла ШИМ

-----------------------------------------------------------------------------------------------------*/
void Phase_V_mode(int32_t val)
{
  if (val >= 100)    gpt1_out_mode = FL_PHASE_PULL_UP;
  else if (val == 0) gpt1_out_mode = FL_PHASE_PULL_DOWN;
  else if (val < 0)  gpt1_out_mode = FL_PHASE_Z_STATE;
  else
  {
    val =(gpt_pwm_cycle_set * (100 - val)) / 100;

    // Не даем прекратиться PWM и не даем импульсу стать слишком коротким
    if (val < MIN_PWM_COMPARE_VAL)
    {
      val = MIN_PWM_COMPARE_VAL;
    }
    else if ((gpt_pwm_cycle_set - val) < MIN_PWM_COMPARE_VAL)
    {
      val = gpt_pwm_cycle_set - MIN_PWM_COMPARE_VAL;
    }
    R_GPTA1->GTCCRC = val;
    gpt1_out_mode = FL_PHASE_PWM_ON;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Установка состояния на выходе фазы W 3-фазного инвертера

  val =   0     - нижний ключ постоянно открыт, верхний постоянно закрыт
  val = 100     - нижний ключ постоянно закрыт, верхний постоянно открыт
  val =  -1     - нижний ключ постоянно закрыт, верхний постоянно закрыт
  0 < val < 100 - На выходе сигнал ШИМ с коэфициентом заполнения равным val в процентах
                  Установка ШИМ буфферизирована, и вступит в действие после окончания текущего цикла ШИМ

-----------------------------------------------------------------------------------------------------*/
void Phase_W_mode(int32_t val)
{
  if (val >= 100)    gpt2_out_mode = FL_PHASE_PULL_UP;
  else if (val == 0) gpt2_out_mode = FL_PHASE_PULL_DOWN;
  else if (val < 0)  gpt2_out_mode = FL_PHASE_Z_STATE;
  else
  {
    val =(gpt_pwm_cycle_set * (100 - val)) / 100;

    // Не даем прекратиться PWM и не даем импульсу стать слишком коротким
    if (val < MIN_PWM_COMPARE_VAL)
    {
      val = MIN_PWM_COMPARE_VAL;
    }
    else if ((gpt_pwm_cycle_set - val) < MIN_PWM_COMPARE_VAL)
    {
      val = gpt_pwm_cycle_set - MIN_PWM_COMPARE_VAL;
    }
    R_GPTA2->GTCCRC = val;
    gpt2_out_mode = FL_PHASE_PWM_ON;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Функция вызываемая из обработчика прерывания для приведения выходов к заданному задачей управления состоянию

  \param mode
-----------------------------------------------------------------------------------------------------*/
void Set_output_U(uint8_t mode)
{
  uint32_t  tmp_H = R_PFS->P415PFS;
  uint32_t  tmp_L = R_PFS->P414PFS;
  switch (mode)
  {
  case FL_PHASE_PULL_UP   :
    // Для фазы U устанавливаем P415 (UH) в 1, P414 (UL) в 0
    tmp_H = tmp_H & ~(LSHIFT(0x1F,24) | BIT(16));
    tmp_H = tmp_H | BIT(2) | BIT(0);

    tmp_L = tmp_L & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
    tmp_L = tmp_L | BIT(2);

    R_PFS->P414PFS = tmp_L;  // Записываем состояние выхода в порт данного выхода
    __DSB();                 // Оператор Data Synchronization Barrier (DSB) необходим для гарантии установления состояния выхода
                             // до того как начнет выполняться следующая инструкция
    R_PFS->P415PFS = tmp_H;
    __DSB();
    break;
  case FL_PHASE_PULL_DOWN :
    // Для фазы U устанавливаем P415 (UH) в 0, P414 (UL) в 1
    tmp_H = tmp_H & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
    tmp_H = tmp_H | BIT(2);

    tmp_L = tmp_L & ~(LSHIFT(0x1F,24) | BIT(16));
    tmp_L = tmp_L | BIT(2) | BIT(0);

    R_PFS->P415PFS = tmp_H;
    __DSB();
    R_PFS->P414PFS = tmp_L;
    __DSB();
    break;
  case FL_PHASE_Z_STATE       :
    // Для фазы U устанавливаем P415 (UH) в 0, P414 (UL) в 0
    tmp_H = tmp_H & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
    tmp_H = tmp_H | BIT(2);

    tmp_L = tmp_L & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
    tmp_L = tmp_L | BIT(2);

    R_PFS->P414PFS = tmp_L;
    __DSB();
    R_PFS->P415PFS = tmp_H;
    __DSB();
    break;
  case FL_PHASE_PWM_ON       :
    // Для фазы U переключаем P415 (UH) и P414 (UL) в режим управления периферией

    // Перепрограммирвать режим портов только если он не был уже в заданном состоянии
    if (((tmp_H >> 24) & 0x1F) != PSEL_03)
    {
      // Если выход H был установлен в 1, то предварительно установить его в 0
      if (tmp_H & BIT(0))
      {
        tmp_H = tmp_H & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
        tmp_H = tmp_H | BIT(2);
        R_PFS->P415PFS = tmp_H;
        __DSB();
      }

      // Если выход L был установлен в 1, то предварительно установить его в 0
      if (tmp_L & BIT(0))
      {
        tmp_L = tmp_L & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
        tmp_L = tmp_L | BIT(2);
        R_PFS->P414PFS = tmp_L;
        __DSB();
      }

      tmp_H = tmp_H & ~(LSHIFT(0x1F,24) | BIT(0));
      tmp_H = tmp_H | BIT(16) | BIT(2) | LSHIFT(PSEL_03,24);

      tmp_L = tmp_L & ~(LSHIFT(0x1F,24) | BIT(0));
      tmp_L = tmp_L | BIT(16) | BIT(2) | LSHIFT(PSEL_03,24);

      R_PFS->P414PFS = tmp_L;
      __DSB();
      R_PFS->P415PFS = tmp_H;
      __DSB();
    }
    break;
  }
}

/*-----------------------------------------------------------------------------------------------------

  \param mode
-----------------------------------------------------------------------------------------------------*/
void Set_output_V(uint8_t mode)
{
  uint32_t  tmp_H = R_PFS->P405PFS;
  uint32_t  tmp_L = R_PFS->P406PFS;
  switch (mode)
  {
  case FL_PHASE_PULL_UP   :
    // Для фазы V устанавливаем P405 (UH) в 1, P406 (UL) в 0
    tmp_H = tmp_H & ~(LSHIFT(0x1F,24) | BIT(16));
    tmp_H = tmp_H | BIT(2) | BIT(0);

    tmp_L = tmp_L & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
    tmp_L = tmp_L | BIT(2);

    R_PFS->P406PFS = tmp_L;
    __DSB();
    R_PFS->P405PFS = tmp_H;
    __DSB();
    break;
  case FL_PHASE_PULL_DOWN :
    // Для фазы V устанавливаем P405 (UH) в 0, P406 (UL) в 1
    tmp_H = tmp_H & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
    tmp_H = tmp_H | BIT(2);

    tmp_L = tmp_L & ~(LSHIFT(0x1F,24) | BIT(16));
    tmp_L = tmp_L | BIT(2) | BIT(0);

    R_PFS->P405PFS = tmp_H;
    __DSB();
    R_PFS->P406PFS = tmp_L;
    __DSB();
    break;
  case FL_PHASE_Z_STATE       :
    // Для фазы V устанавливаем P405 (UH) в 0, P406 (UL) в 0
    tmp_H = tmp_H & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
    tmp_H = tmp_H | BIT(2);

    tmp_L = tmp_L & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
    tmp_L = tmp_L | BIT(2);

    R_PFS->P406PFS = tmp_L;
    __DSB();
    R_PFS->P405PFS = tmp_H;
    __DSB();
    break;
  case FL_PHASE_PWM_ON       :
    // Для фазы V переключаем P405 (UH) и P406 (UL) в режим управления периферией

    // Перепрограммирвать режим портов только если он не был уже в заданном состоянии
    if (((tmp_H >> 24) & 0x1F) != PSEL_03)
    {
      // Если выход H был установлен в 1, то предварительно установить его в 0
      if (tmp_H & BIT(0))
      {
        tmp_H = tmp_H & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
        tmp_H = tmp_H | BIT(2);
        R_PFS->P405PFS = tmp_H;
        __DSB();
      }

      // Если выход L был установлен в 1, то предварительно установить его в 0
      if (tmp_L & BIT(0))
      {
        tmp_L = tmp_L & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
        tmp_L = tmp_L | BIT(2);
        R_PFS->P406PFS = tmp_L;
        __DSB();
      }

      tmp_H = tmp_H & ~(LSHIFT(0x1F,24) | BIT(0));
      tmp_H = tmp_H | BIT(16) | BIT(2) | LSHIFT(PSEL_03,24);

      tmp_L = tmp_L & ~(LSHIFT(0x1F,24) | BIT(0));
      tmp_L = tmp_L | BIT(16) | BIT(2) | LSHIFT(PSEL_03,24);

      R_PFS->P406PFS = tmp_L;
      __DSB();
      R_PFS->P405PFS = tmp_H;
      __DSB();
    }
    break;
  }
}


/*-----------------------------------------------------------------------------------------------------

  \param mode
-----------------------------------------------------------------------------------------------------*/
void Set_output_W(uint8_t mode)
{
  uint32_t  tmp_H = R_PFS->P113PFS;
  uint32_t  tmp_L = R_PFS->P114PFS;
  switch (mode)
  {
  case FL_PHASE_PULL_UP   :
    // Для фазы W устанавливаем P113 (UH) в 1, P114 (UL) в 0
    tmp_H = tmp_H & ~(LSHIFT(0x1F,24) | BIT(16));
    tmp_H = tmp_H | BIT(2) | BIT(0);

    tmp_L = tmp_L & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
    tmp_L = tmp_L | BIT(2);

    R_PFS->P114PFS = tmp_L;
    __DSB();
    R_PFS->P113PFS = tmp_H;
    __DSB();
    break;
  case FL_PHASE_PULL_DOWN :
    // Для фазы W устанавливаем P113 (UH) в 0, P114 (UL) в 1
    tmp_H = tmp_H & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
    tmp_H = tmp_H | BIT(2);

    tmp_L = tmp_L & ~(LSHIFT(0x1F,24) | BIT(16));
    tmp_L = tmp_L | BIT(2) | BIT(0);

    R_PFS->P113PFS = tmp_H;
    __DSB();
    R_PFS->P114PFS = tmp_L;
    __DSB();
    break;
  case FL_PHASE_Z_STATE       :
    // Для фазы W устанавливаем P113 (UH) в 0, P114 (UL) в 0
    tmp_H = tmp_H & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
    tmp_H = tmp_H | BIT(2);

    tmp_L = tmp_L & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
    tmp_L = tmp_L | BIT(2);

    R_PFS->P114PFS = tmp_L;
    __DSB();
    R_PFS->P113PFS = tmp_H;
    __DSB();
    break;
  case FL_PHASE_PWM_ON       :
    // Для фазы W переключаем P113 (UH) и P114 (UL) в режим управления периферией

    // Перепрограммирвать режим портов только если он не был уже в заданном состоянии
    if (((tmp_H >> 24) & 0x1F) != PSEL_03)
    {
      // Если выход H был установлен в 1, то предварительно установить его в 0
      if (tmp_H & BIT(0))
      {
        tmp_H = tmp_H & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
        tmp_H = tmp_H | BIT(2);
        R_PFS->P113PFS = tmp_H;
        __DSB();
      }

      // Если выход L был установлен в 1, то предварительно установить его в 0
      if (tmp_L & BIT(0))
      {
        tmp_L = tmp_L & ~(LSHIFT(0x1F,24) | BIT(16) | BIT(0));
        tmp_L = tmp_L | BIT(2);
        R_PFS->P114PFS = tmp_L;
        __DSB();
      }

      tmp_H = tmp_H & ~(LSHIFT(0x1F,24) | BIT(0));
      tmp_H = tmp_H | BIT(16) | BIT(2) | LSHIFT(PSEL_03,24);

      tmp_L = tmp_L & ~(LSHIFT(0x1F,24) | BIT(0));
      tmp_L = tmp_L | BIT(16) | BIT(2) | LSHIFT(PSEL_03,24);

      R_PFS->P114PFS = tmp_L;
      __DSB();
      R_PFS->P113PFS = tmp_H;
      __DSB();
    }
    break;
  }
}


