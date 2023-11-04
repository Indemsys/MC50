// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-10-23
// 15:25:39
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


// Структура описания варианта тестирования ШИМ
typedef struct
{
    uint32_t start_pwm;  // Начальное сосотояние ШИМ
    uint32_t end_pwm;    // Конечное сосотояние ШИМ

} T_pwm_test_mode;


#define M___0    0
#define M___1    PWM_STEP_COUNT
#define M_PWM   (PWM_STEP_COUNT/10)

// .............................................................
// Массив вариантов тестирования ШИМ
// .............................................................
T_pwm_test_mode pwm_test_steps[] =
{
  {M___0            , M___1   }, // N0:  0 %   -> 100 %
  {M___1            , M___0   }, // N1:  100 % -> 0 %

  {M___0            , M_PWM   }, // N2:  0 %   -> PWM
  {M_PWM            , M___0   }, // N3:  PWM   -> 0 %

  {M___1            , M_PWM   }, // N4:  100%  -> PWM
  {M_PWM            , M___1   }, // N5:  PWM   -> 100 %
};

uint32_t test_step = 0; // Переменная задающая вариант тестирования.
                        // Менять значение переменной можно из окна Live Watch отладчика в реальном времени

uint8_t  test_flag;     // Переменная вызывающая генерацию импульса на светодиоде

/*-----------------------------------------------------------------------------------------------------
  Функция вызываемая из обработчика прерывания синхронно с треугольным ШИМ на подъеме или спаде


  \param R_GPT          - указатель на структуру таймера
  \param top_comp_value - верхнее значение компаратора таймера
  \param pwm_lev        - коэффициент заполнения ШИМ от 0 до PWM_STEP_COUNT
-----------------------------------------------------------------------------------------------------*/
static inline void Set_gpt_pwm_level(R_GPTA0_Type *R_GPT, uint32_t top_comp_value, uint32_t pwm_lev)
{
  uint32_t comp_now  = R_GPT->GTCCRA;
  uint32_t count_dir =(R_GPT->GTST) & BIT(15); // Если счет идет вниз, то значение будет равно 0

  if (pwm_lev == LEVEL_0)
  {
    if (comp_now != 0)
    {
      //============================
      // pwm -> 0
      //============================
      if (count_dir == GPT_COUNT_DIR_DOWN)
      {
        R_GPT->GTCCRC = top_comp_value;
      }
    }
    else
    {
      //============================
      // 1 -> 0
      //============================
      if (count_dir != GPT_COUNT_DIR_DOWN)
      {
        R_GPT->GTCCRC = top_comp_value >> 1;
      }
    }
  }
  else if (pwm_lev == LEVEL_1)
  {
    if (comp_now != top_comp_value)
    {
      //============================
      // pwm -> 1
      //============================
      if (count_dir == GPT_COUNT_DIR_DOWN)
      {
        R_GPT->GTCCRC = 0;
      }
    }
    else
    {
      //============================
      // 0 -> 1
      //============================
      if (count_dir != GPT_COUNT_DIR_DOWN)
      {
        R_GPT->GTCCRC = top_comp_value >> 1;
      }
    }
  }
  else
  {
    uint32_t comp_val = r3ph.pwm_indx_to_comp[pwm_lev];
    if (comp_now == top_comp_value)
    {
      //============================
      // 0 -> pwm
      //============================
      if (count_dir == GPT_COUNT_DIR_DOWN)
      {
        R_GPT->GTCCRC = comp_val;
      }
    }
    else if  (comp_now == 0)
    {
      //============================
      // 1 -> pwm
      //============================
      if (count_dir != GPT_COUNT_DIR_DOWN)
      {
        R_GPT->GTCCRC = comp_val;
      }
    }
    else
    {
      //============================
      // pwm -> pwm
      //============================
      R_GPT->GTCCRC = comp_val;
    }
  }
}


/*-----------------------------------------------------------------------------------------------------
  Функция вызываемая в обработчике прерываний по окончании выборок АЦП
  Вызывается синхронно с ШИМ с частотой ШИМ


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Test_BLDC_ISR_handler(void)
{
  // Функции измерения
  Hall_3PH_capture_bitmask();
  Measure_instant_phases_current();
  Measure_overall_BLDC_motor_current();
  Measure_DC_bus_voltage_current();
  Measure_servo_sensor_speed();
  Hall_3PH_measure_speed_and_direction();

  if (test_flag)
  {
    test_flag = 0;
    RS485_LED = 1;              // Генерация синхроимпульса на светодиоде платы сервоконтроллера
  }
  Set_gpt_pwm_level(R_GPTA0, r3ph.gpt_top_val, r3ph.gpt_U_pwm_val);         // Обновление регистра копаратора фазы U
  Set_gpt_pwm_level(R_GPTA1, r3ph.gpt_top_val, r3ph.gpt_V_pwm_val);         // Обновление регистра копаратора фазы V
  Set_gpt_pwm_level(R_GPTA2, r3ph.gpt_top_val, r3ph.gpt_W_pwm_val);         // Обновление регистра копаратора фазы W

  RS485_LED = 0;

  App_set_flags(TEST_FLAG);     // Выставляется флаг задаче тестирования для уведомления
                                // об окончании очередного цикла обновления регистров комапараторов
}


/*-----------------------------------------------------------------------------------------------------
  Выполнение теста переключения из одного состояния ШИМ в другое

  \param void
-----------------------------------------------------------------------------------------------------*/
void Exec_test_step(void)
{
  uint32_t actual_events;
  uint32_t start_pwm ;
  uint32_t end_pwm   ;

  start_pwm = pwm_test_steps[test_step].start_pwm;
  end_pwm   = pwm_test_steps[test_step].end_pwm;

  Post_phase_pwm(PHASE_U, start_pwm);      // Устанавливаем начальное состояние ШИМ для заданного варианта теста

  for (uint32_t i=0;i<5;i++)
  {
    App_wait_flags(TEST_FLAG,&actual_events, 10); // Пропускаем нечетное количество циклов
  }

  Post_phase_pwm(PHASE_U, end_pwm);         // Устанавливаем конечное состояние ШИМ для заданного варианта теста

  test_flag = 1;                                  // Сигнализируем о необходимости в ближайшем цикле сформировать синхроимпульс на  светодиоде
                                                  // Синхроимпульс нужен для осциллографа для привязки к моменту смены состояний ШИМ

  for (uint32_t i=0;i<5;i++)
  {
    App_wait_flags(TEST_FLAG,&actual_events, 10); // Пропускаем нечетное количество циклов
  }
}

/*-----------------------------------------------------------------------------------------------------
  Задача тестирования переключения состояния ШИМ

  \param void
-----------------------------------------------------------------------------------------------------*/
void _Test_PWM(void)
{
  uint32_t actual_events;
  uint32_t res;

  Hall_3PH_reinit();
  Rot_Encoders_capturing_init(RPM_MEASURE_OVERFL_TIME);  // Инициализируем измерение скрости вращения двигателя по сигналам с датчиков Холла

  if (PWM_3ph_triangle_2buffered_init(wvar.pwm_frequency) != RES_OK) return;
  PWM_3ph_start();

  ADC_init(Test_BLDC_ISR_handler);                       // Инициализируем ADC работающий синхронно с PWM мотора

  MC50_3Ph_PWM_pins_init();

  do
  {
    res = App_wait_flags(TEST_FLAG,&actual_events, 10);
    if (res == TX_SUCCESS)
    {
      Exec_test_step();
    }

  } while (1);
}



