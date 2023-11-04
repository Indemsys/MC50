#include   "MC50.h"

#define OUTPUTS_TASK_STACK_SIZE 1024
static uint8_t outputs_stacks[OUTPUTS_TASK_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.outputs_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static TX_THREAD       outputs_thread;

#define FLAG_IO_REFRESH        BIT(0)
static  TX_TIMER               outputs_timer;
static  TX_EVENT_FLAGS_GROUP   outputs_flags;


#define __ON   1
#define _OFF   0
//  Шаблон состоит из массива груп слов.
//  Первое слово в группе - значение
//  Второе слово в группе - длительность интервала времени в мс или специальный маркер остановки(0xFFFFFFFF) или цикла(0x00000000)

const int32_t   OUT_BLINK[] =
{
  __ON, 100,
  _OFF, 200,
  0, 0
};

const int32_t   OUT_ON[] =
{
  __ON, 1000,
  __ON, 0xFFFFFFFF
};

const int32_t   OUT_OFF[] =
{
  _OFF, 1000,
  _OFF, 0xFFFFFFFF
};

const int32_t   OUT_UNDEF_BLINK[] =
{
  __ON, 30,
  _OFF, 900,
  _OFF, 0
};

const int32_t   OUT_3_BLINK[] =
{
  __ON, 50,
  _OFF, 50,
  __ON, 50,
  _OFF, 50,
  __ON, 50,
  _OFF, 250,
  _OFF, 0
};


const int32_t   OUT_3_OFF_BLINK[] =
{
  _OFF, 50,
  __ON, 50,
  _OFF, 50,
  __ON, 50,
  _OFF, 50,
  __ON, 0xFFFFFFFF
};



const int32_t   OUT_CAN_ACTIVE_BLINK[] =
{
  _OFF, 10,
  __ON, 50,
  _OFF, 10,
  __ON, 50,
  _OFF, 10,
  __ON, 50,
  _OFF, 10,
  __ON, 50,
  _OFF, 10,
  __ON, 50,
  _OFF, 10,
  __ON, 0xFFFFFFFF
};

T_outs_ptrn outs_cbl[DIG_OUTPUTS_NUM];


static void Set_LED_GREEN(int32_t val) { GREEN_LED = val; }
static void Set_LED_RED  (int32_t val) { RED_LED   = val; }
static void Set_LCD_LIGHT(int32_t val) { LCD_BLK   = val; }
static void Set_LED_CAN  (int32_t val) { CAN_LED   = val; }
static void Set_LED_RS485(int32_t val) { RS485_LED = val; }


T_Set_pin Output_set_functions[DIG_OUTPUTS_NUM] =
{
  { Set_LED_GREEN  },
  { Set_LED_RED    },
  { Set_LCD_LIGHT  },
  { Set_LED_CAN    },
  { Set_LED_RS485  },
};

static TX_MUTEX    outputs_mutex;

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return int32_t
-----------------------------------------------------------------------------------------------------*/
int32_t Outputs_mutex_create(void)
{
  if (tx_mutex_create(&outputs_mutex, "OutputsSM", TX_INHERIT) != TX_SUCCESS) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param out_num
-----------------------------------------------------------------------------------------------------*/
void Set_output_blink(uint32_t out_num)
{
  Outputs_set_pattern(OUT_BLINK, out_num);
}

void Set_output_on(uint32_t out_num)
{
  Outputs_set_pattern(OUT_ON, out_num);
}

void Set_output_off(uint32_t out_num)
{
  Outputs_set_pattern(OUT_OFF, out_num);
}

void Set_output_blink_undef(uint32_t out_num)
{
  Outputs_set_pattern(OUT_UNDEF_BLINK, out_num);
}

void Set_output_blink_3(uint32_t out_num)
{
  Outputs_set_pattern(OUT_3_BLINK, out_num);
}

void Set_output_can_active_blink(uint32_t out_num)
{
  Outputs_set_pattern(OUT_CAN_ACTIVE_BLINK, out_num);
}

void Set_output_off_blink_3(uint32_t out_num)
{
  Outputs_set_pattern(OUT_3_OFF_BLINK, out_num);
}



/*-----------------------------------------------------------------------------------------------------


  \param num
  \param state
-----------------------------------------------------------------------------------------------------*/
void Set_output_state(uint32_t out_num, uint32_t state)
{
  if (out_num < DIG_OUTPUTS_NUM)
  {
    Output_set_functions[out_num](state);
  }
}



/*-----------------------------------------------------------------------------------------------------
  Инициализация шаблона для машины состояний выходного сигнала

  Шаблон состоит из массива груп слов.
  Первое слово в группе - значение сигнала
  Второе слово в группе - длительность интервала времени в  мс
    интервал равный 0x00000000 - означает возврат в начало шаблона
    интервал равный 0xFFFFFFFF - означает застывание состояния


  \param pttn    - указатель на запись шаблоне
  \param n       - номер сигнала
  \param period  - периодичность вызова машины состояний
-----------------------------------------------------------------------------------------------------*/
void Outputs_set_pattern(const int32_t *pttn, uint32_t n)
{
  if (n >= DIG_OUTPUTS_NUM) return;
  if (pttn != 0)
  {
    if (outs_cbl[n].pattern_start_ptr != (int32_t *)pttn)
    {
      outs_cbl[n].pattern_start_ptr = (int32_t *)pttn;
      outs_cbl[n].pttn_ptr = (int32_t *)pttn;
      Set_output_state(n,*outs_cbl[n].pttn_ptr);
      outs_cbl[n].pttn_ptr++;
      Get_hw_timestump(&outs_cbl[n].lastt);
      outs_cbl[n].pttn_ptr++;
      outs_cbl[n].active = 1;
    }
    else
    {
      outs_cbl[n].repeat = 1;
    }
  }
}


/*-----------------------------------------------------------------------------------------------------
   Автомат состояний выходных сигналов


  \param tnow  - текущее время
-----------------------------------------------------------------------------------------------------*/
void Outputs_state_automat(void)
{
  uint32_t         duration;
  uint32_t         output_state;
  uint32_t         n;
  T_sys_timestump  tnow;


  Get_hw_timestump(&tnow);

  for (n = 0; n < DIG_OUTPUTS_NUM; n++)
  {
    // Управление состоянием выходного сигнала
    if (outs_cbl[n].active) // Отрабатываем шаблон только если активное состояние
    {
      uint32_t dt = Timestump_diff_to_msec(&outs_cbl[n].lastt  , &tnow);

      if (dt >= outs_cbl[n].duration)  // Меняем состояние сигнала при обнулении счетчика
      {
        memcpy(&outs_cbl[n].lastt, &tnow, sizeof(T_sys_timestump));

        if (outs_cbl[n].pattern_start_ptr != 0)  // Проверяем есть ли назначенный шаблон
        {
          output_state =*outs_cbl[n].pttn_ptr;   // Выборка значения состояния выхода
          outs_cbl[n].pttn_ptr++;
          duration =*outs_cbl[n].pttn_ptr;       // Выборка длительности состояния
          outs_cbl[n].pttn_ptr++;                // Переход на следующий элемент шаблона
          if (duration != 0xFFFFFFFF)
          {
            if (duration == 0)  // Длительность равная 0 означает возврат указателя элемента на начало шаблона и повторную выборку
            {
              outs_cbl[n].pttn_ptr = outs_cbl[n].pattern_start_ptr;
              output_state =*outs_cbl[n].pttn_ptr;
              outs_cbl[n].pttn_ptr++;
              outs_cbl[n].duration =*outs_cbl[n].pttn_ptr;
              outs_cbl[n].pttn_ptr++;
              Set_output_state(n , output_state);
            }
            else
            {
              outs_cbl[n].duration = duration;
              Set_output_state(n ,output_state);
            }
          }
          else
          {
            if (outs_cbl[n].repeat)
            {
              outs_cbl[n].repeat = 0;
              // Возврат указателя элемента на начало шаблона и повторная выборка
              outs_cbl[n].pttn_ptr = outs_cbl[n].pattern_start_ptr;
              output_state =*outs_cbl[n].pttn_ptr;
              outs_cbl[n].pttn_ptr++;
              outs_cbl[n].duration =*outs_cbl[n].pttn_ptr;
              outs_cbl[n].pttn_ptr++;
              Set_output_state(n , output_state);
            }
            else
            {
              // Обнуляем счетчик и таким образом выключаем обработку паттерна
              Set_output_state(n , output_state);
              outs_cbl[n].active = 0;
              outs_cbl[n].pattern_start_ptr = 0;
            }
          }
        }
        else
        {
          // Если нет шаблона обнуляем состояние выходного сигнала
          Set_output_state(n, 0);
        }
      }
    }
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param v
-----------------------------------------------------------------------------------------------------*/
void IO_refresh(ULONG v)
{
  tx_event_flags_set(&outputs_flags, FLAG_IO_REFRESH, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param arg
-----------------------------------------------------------------------------------------------------*/
static void Outputs_task(ULONG arg)
{
  static uint32_t prev_app_err = 0;
  static uint32_t app_err;
  tx_event_flags_create(&outputs_flags, "outputs");
  tx_timer_create(&outputs_timer,"outputs",IO_refresh, 0, 1 , 1, TX_AUTO_ACTIVATE);

  Set_output_off(OUTP_LED_GREEN);
  Set_output_off(OUTP_LED_RED);

  do
  {
    ULONG actual_flags;
    tx_event_flags_get(&outputs_flags, FLAG_IO_REFRESH, TX_OR_CLEAR, &actual_flags,  2);
    Outputs_state_automat();

    app_err = App_get_error_flags();

    if (app_err!=0)
    {
      Set_output_on(OUTP_LED_RED);
      if (app_err != prev_app_err)
      {
        Log_app_errors(app_err);
      }
    }
    else
    {
      Set_output_off(OUTP_LED_RED);
      if (prev_app_err != 0)
      {
        Log_app_errors(app_err);
      }
    }
    prev_app_err = app_err;



  } while (1);

}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Thread_Outputs_create(void)
{
  UINT              err;

  err = tx_thread_create(
                         &outputs_thread,
                         (CHAR *)"Outputs",
                         Outputs_task,
                         0,
                         outputs_stacks,
                         OUTPUTS_TASK_STACK_SIZE,
                         OUTPUTS_TASK_PRIO,
                         OUTPUTS_TASK_PRIO,
                         1,
                         TX_AUTO_START
                        );

  if (err == TX_SUCCESS)
  {
    APPLOG("Outputs task created.");
    return RES_OK;
  }
  else
  {
    APPLOG("Outputs creating error %d.", err);
    return RES_ERROR;
  }
}

