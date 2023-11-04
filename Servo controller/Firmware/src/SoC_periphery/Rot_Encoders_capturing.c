// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-07-09
// 15:39:30
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

/*-----------------------------------------------------------------------------------------------------
  Инициализируем таймер GPT11 (согласно обозначению в даташите) или R_GPTB3 (согласно обозначению в хидере)

  Два канала измерения длительности импульсов с датчиков Холла. Прием сигналов ENC_U, ENC_V

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t GPT_11_Capture_init(void)
{
  R_GPTB3->GTWP_b.PRKEY     = 0xA5;       // Разрешаем запись в бит WP этого регистра
  R_GPTB3->GTWP_b.WP        = 0;          // 0: Enable writes to the register Разрешаем запись в остальные регистры таймера

  R_GPTB3->GTCR_b.CST       = 0;          // Останавливаем счет

  R_GPTB3->GTCNT            = 0;          // Обнуляем таймер
  R_GPTB3->GTPR             = 0x7FFFFFFF; // Устанавливаем регистр задающий верхний предел таймера. Ставим максимально положительное число,
                                          // отрицательное число служит вспомогательным целям в блоке обпределения сорости вращения
  R_GPTB3->GTIOR            = 0;          // Очищаем настройки выходов. Все запрещены

  R_GPTB3->GTCR_b.TPCS      = 0;          // Timer Prescaler Select. 0 0 0: PCLKD/1
  R_GPTB3->GTCR_b.MD        = 0;          // 000: Saw-wave PWM mode (single buffer or double buffer possible)

  R_GPTB3->GTUDDTYC_b.UDF   = 1;          //
  R_GPTB3->GTUDDTYC_b.UD    = 1;          // Счетчик инкрементируется

  R_GPTB3->GTICASR_b.ASCARBL= 1;          // GTIOCA Pin Rising  Input during GTIOCB Value Low  Source GTCCRA Input Capture Enable
  R_GPTB3->GTICASR_b.ASCARBH= 1;          // GTIOCA Pin Rising  Input during GTIOCB Value High Source GTCCRA Input Capture Enable
  R_GPTB3->GTICASR_b.ASCAFBL= 1;          // GTIOCA Pin Falling Input during GTIOCB Value Low  Source GTCCRA Input Capture Enable
  R_GPTB3->GTICASR_b.ASCAFBH= 1;          // GTIOCA Pin Falling Input during GTIOCB Value High Source GTCCRA Input Capture Enable

  R_GPTB3->GTICBSR_b.BSCBRAL= 1;          // GTIOCB Pin Rising  Input during GTIOCA Value Low  Source GTCCRB Input Capture Enable
  R_GPTB3->GTICBSR_b.BSCBRAH= 1;          // GTIOCB Pin Rising  Input during GTIOCA Value High Source GTCCRB Input Capture Enable
  R_GPTB3->GTICBSR_b.BSCBFAL= 1;          // GTIOCB Pin Falling Input during GTIOCA Value Low  Source GTCCRB Input Capture Enable
  R_GPTB3->GTICBSR_b.BSCBFAH= 1;          // GTIOCB Pin Falling Input during GTIOCA Value High Source GTCCRB Input Capture Enable

  R_GPTB3->GTSSR_b.CSTRT    = 1;          // Разрешаем програмный запуск от регитсра GTSTR
  R_GPTB3->GTST             = 0;          // Очищаем статус

  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Инициализируем таймер GPT12 (согласно обозначению в даташите) или R_GPTB4 (согласно обозначению в хидере)

  Два канала измерения длительности импульсов с датчиков Холла. Прием сигналов ENC_W

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t GPT_12_Capture_init(void)
{
  R_GPTB4->GTWP_b.PRKEY     = 0xA5;       // Разрешаем запись в бит WP этого регистра
  R_GPTB4->GTWP_b.WP        = 0;          // 0: Enable writes to the register Разрешаем запись в остальные регистры таймера

  R_GPTB4->GTCR_b.CST       = 0;          // Останавливаем счет

  R_GPTB4->GTCNT            = 0;          // Обнуляем таймер
  R_GPTB4->GTPR             = 0x7FFFFFFF; // Устанавливаем регистр задающий верхний предел таймера. Ставим максимально положительное число,
                                          // отрицательное число служит вспомогательным целям в блоке обпределения сорости вращения
  R_GPTB4->GTIOR            = 0;          // Очищаем настройки выходов. Все запрещены

  R_GPTB4->GTCR_b.TPCS      = 0;          // Timer Prescaler Select. 0 0 0: PCLKD/1
  R_GPTB4->GTCR_b.MD        = 0;          // 000: Saw-wave PWM mode (single buffer or double buffer possible)

  R_GPTB4->GTUDDTYC_b.UDF   = 1;          //
  R_GPTB4->GTUDDTYC_b.UD    = 1;          // Счетчик инкрементируется

  R_GPTB4->GTICASR_b.ASCARBL= 1;          // GTIOCA Pin Rising  Input during GTIOCB Value Low  Source GTCCRA Input Capture Enable
  R_GPTB4->GTICASR_b.ASCARBH= 1;          // GTIOCA Pin Rising  Input during GTIOCB Value High Source GTCCRA Input Capture Enable
  R_GPTB4->GTICASR_b.ASCAFBL= 1;          // GTIOCA Pin Falling Input during GTIOCB Value Low  Source GTCCRA Input Capture Enable
  R_GPTB4->GTICASR_b.ASCAFBH= 1;          // GTIOCA Pin Falling Input during GTIOCB Value High Source GTCCRA Input Capture Enable

  R_GPTB4->GTSSR_b.CSTRT    = 1;          // Разрешаем програмный запуск от регитсра GTSTR
  R_GPTB4->GTST             = 0;          // Очищаем статус

  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Инициализация таймеа GPT13 (согласно обозначению в даташите) или R_GPTB5 (согласно обозначению в хидере)

  Отсчитывает переполения для обнаружения остановки вращения ротора

  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t GPT_13_Overflow_init(float overfl_time)
{
  R_GPTB5->GTWP_b.PRKEY     = 0xA5;       // Разрешаем запись в бит WP этого регистра
  R_GPTB5->GTWP_b.WP        = 0;          // 0: Enable writes to the register Разрешаем запись в остальные регистры таймера

  R_GPTB5->GTCR_b.CST       = 0;          // Останавливаем счет

  R_GPTB5->GTCNT            = 0;            // Обнуляем таймер
  R_GPTB5->GTPR             = (uint32_t)(PCLKD_FREQ*overfl_time); // Устанавливаем регистр задающий время переполения таймера
  R_GPTB5->GTIOR            = 0;            // Очищаем настройки выходов. Все запрещены

  R_GPTB5->GTCR_b.TPCS      = 0;          // Timer Prescaler Select. 0 0 0: PCLKD/1
  R_GPTB5->GTCR_b.MD        = 0;          // 000: Saw-wave PWM mode (single buffer or double buffer possible)

  R_GPTB5->GTUDDTYC_b.UDF   = 1;          //
  R_GPTB5->GTUDDTYC_b.UD    = 1;          // Счетчик инкрементируется

  R_GPTB5->GTSSR_b.CSTRT    = 1;          // Разрешаем програмный запуск от регитсра GTSTR
  R_GPTB5->GTCSR_b.CCLR     = 1;          // Разрешаем програмный сброс  от регитсра GTCTR
  R_GPTB5->GTST             = 0;          // Очищаем статус

  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void GPT_13_clear(void)
{
  R_GPTB5->GTCLR_b.CCLR13 = 1;
}


/*-----------------------------------------------------------------------------------------------------
  Инициализация таймеров 11 и 12 для работы в режиме capture для измерения скрости вращения двигателя по сигналам с датчиков Холла
  Таймер тактируется частотой PCLKD = 120 МГц

  \param void
-----------------------------------------------------------------------------------------------------*/
uint32_t Rot_Encoders_capturing_init(float overfl_time)
{
  R_MSTP->MSTPCRD_b.MSTPD6  = 0;          // Разрешаем работу модулей GPT ch13-ch8

  GPT_11_Capture_init();
  GPT_12_Capture_init();
  GPT_13_Overflow_init(overfl_time);

  R_GPTB4->GTSTR = BIT(11) + BIT(12) + BIT(13);     // Стартуем таймеры одновременно

  return RES_OK;

}


