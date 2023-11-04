// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-02-06
// 17:23:49
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


static void can0_error_isr(void);
static void can0_fifo_rx_isr(void);
static void can0_fifo_tx_isr(void);
static void can0_mailbox_rx_isr(void);
static void can0_mailbox_tx_isr(void);



static IRQn_Type can0_error_int_num;
static IRQn_Type can0_fifo_rx_int_num;
static IRQn_Type can0_fifo_tx_int_num;
static IRQn_Type can0_mailbox_rx_int_num;
static IRQn_Type can0_mailbox_tx_int_num;


SSP_VECTOR_DEFINE_CHAN(can0_error_isr,         CAN, ERROR, 0)
SSP_VECTOR_DEFINE_CHAN(can0_fifo_rx_isr     ,  CAN , FIFO_RX     , 0)
SSP_VECTOR_DEFINE_CHAN(can0_fifo_tx_isr     ,  CAN , FIFO_TX     , 0)
SSP_VECTOR_DEFINE_CHAN(can0_mailbox_rx_isr  ,  CAN , MAILBOX_RX  , 0)
SSP_VECTOR_DEFINE_CHAN(can0_mailbox_tx_isr  ,  CAN , MAILBOX_TX  , 0)



T_can_error_counters  can_error_counters;
volatile uint8_t      can0_recr;
volatile uint8_t      can0_tecr;
volatile T_can_str    can0_str;

TX_QUEUE              can0_rx_queue;
TX_QUEUE              can0_tx_queue;
TX_EVENT_FLAGS_GROUP  can0_flags;

T_can_msg             can0_rx_queue_buf[RX_MESSAGES_RING_SZ];
T_can_msg             can0_tx_queue_buf[TX_MESSAGES_RING_SZ];

#define CAN0_MSG_RECEIVED BIT(0)
#define CAN0_MSG_TO_SEND  BIT(1)


#define CAN_TASK_STACK_SIZE 2048
static uint8_t can_stacks[CAN_TASK_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.can_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static TX_THREAD       can_thread;

uint8_t  volatile      can_task_ready;

static void CAN0_send_mailbox(T_can_msg  *p_mb);


extern void CAN_packet_reciever(T_can_msg  *mailbox_ptr);


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void CAN0_set_and_clear_interrupts(void)
{
  can0_error_int_num         = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_CAN0_ERROR);
  can0_fifo_rx_int_num       = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_CAN0_FIFO_RX);
  can0_fifo_tx_int_num       = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_CAN0_FIFO_TX);
  can0_mailbox_rx_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_CAN0_MAILBOX_RX);
  can0_mailbox_tx_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_CAN0_MAILBOX_TX);


  R_BSP_IrqStatusClear(can0_error_int_num);
  R_BSP_IrqStatusClear(can0_fifo_rx_int_num);
  R_BSP_IrqStatusClear(can0_fifo_tx_int_num);
  R_BSP_IrqStatusClear(can0_mailbox_rx_int_num);
  R_BSP_IrqStatusClear(can0_mailbox_tx_int_num);

  NVIC_ClearPendingIRQ(can0_error_int_num);
  NVIC_ClearPendingIRQ(can0_fifo_rx_int_num);
  NVIC_ClearPendingIRQ(can0_fifo_tx_int_num);
  NVIC_ClearPendingIRQ(can0_mailbox_rx_int_num);
  NVIC_ClearPendingIRQ(can0_mailbox_tx_int_num);

}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void CAN0_enable_interrupts(void)
{

  NVIC_SetPriority(can0_error_int_num      ,  INT_CAN0_ERROR_PRIO);
  NVIC_SetPriority(can0_fifo_rx_int_num    ,  INT_CAN0_FIFO_RX_PRIO);
  NVIC_SetPriority(can0_fifo_tx_int_num    ,  INT_CAN0_FIFO_TX_PRIO);
  NVIC_SetPriority(can0_mailbox_rx_int_num ,  INT_CAN0_MAILBOX_RX_PRIO);
  NVIC_SetPriority(can0_mailbox_tx_int_num ,  INT_CAN0_MAILBOX_TX_PRIO);

  NVIC_EnableIRQ(can0_error_int_num);
  NVIC_EnableIRQ(can0_fifo_rx_int_num);
  NVIC_EnableIRQ(can0_fifo_tx_int_num);
  NVIC_EnableIRQ(can0_mailbox_rx_int_num);
  NVIC_EnableIRQ(can0_mailbox_tx_int_num);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void can0_error_isr(void)
{
  // Читаем регистр с флагами ошибок
  uint8_t eifr = R_CAN0->EIFR;
  uint8_t ecsr = R_CAN0->ECSR;

  uint8_t eifr_reset_mask = 0xFF;
  uint8_t ecsr_reset_mask = 0xFF;

  if (eifr & BIT(0)) // BEIE Bus Error Detect Flag
  {
    if (can_error_counters.bus_error_cnt < UINT_MAX)
    {
      can_error_counters.bus_error_cnt++;
    }
    eifr_reset_mask ^= BIT(0);
    // Запрещаем прерывание BEIE чтобы не перегружать процессор
    R_CAN0->EIER &= ~BIT(0);
  }

  if (eifr & BIT(1)) //EWIF flag (Error-Warning Detect Flag)
  {
    // The EWIF flag is set to 1 when the value of the receive error counter (REC) or transmit error counter (TEC) exceeds 95
    can_error_counters.error_warning_cnt++;
    eifr_reset_mask ^= BIT(1);
  }

  if (eifr & BIT(2)) // EPIF flag (Error-Passive Detect Flag)
  {
    // The EPIF flag is set to 1 when the CAN error state becomes error-passive, when the receive error counter (REC) or transmit error counter (TEC) value exceeds 127
    can_error_counters.error_passive_cnt++;
    eifr_reset_mask ^= BIT(2);
  }

  if (eifr & BIT(3)) // BOEIF flag (Bus-Off Entry Detect Flag)
  {
    // The BOEIF flag is set to 1 when the CAN error state becomes bus-off, when the transmit error counter (TEC) value exceeds 255
    can_error_counters.bus_off_entry_cnt++;
    eifr_reset_mask ^= BIT(3);
  }

  if (eifr & BIT(4)) // BORIF flag (Bus-Off Recovery Detect Flag)
  {
    // The BORIF flag is set to 1 when the CAN module recovers from the bus-off state normally by detecting 11 consecutive recessive bits 128 times in some conditions
    can_error_counters.bus_off_recovery_cnt++;
    eifr_reset_mask ^= BIT(4);
    // Разрешаем прерывание BEIE Bus Error Detect Flag, если оно было запрещено
    R_CAN0->EIER &= ~BIT(0);
  }

  if (eifr & BIT(5))
  {
    can_error_counters.receive_overrun_cnt++;
    eifr_reset_mask ^= BIT(5);
  }

  if (eifr & BIT(6))
  {
    can_error_counters.overload_frame_cnt++;
    eifr_reset_mask ^= BIT(6);
  }

  if (eifr & BIT(7))
  {
    can_error_counters.bus_lock_cnt++;
    eifr_reset_mask ^= BIT(7);
  }

  if (ecsr & BIT(0))
  {
    can_error_counters.stuff_error_cnt++;
    ecsr_reset_mask ^= BIT(0);
  }

  if (ecsr & BIT(1))
  {
    can_error_counters.form_error_cnt++;
    ecsr_reset_mask ^= BIT(1);
  }

  if (ecsr & BIT(2))
  {
    can_error_counters.ack_error_cnt++;
    ecsr_reset_mask ^= BIT(2);
  }

  if (ecsr & BIT(3))
  {
    can_error_counters.crc_error_cnt++;
    ecsr_reset_mask ^= BIT(3);
  }

  if (ecsr & BIT(4))
  {
    can_error_counters.bit_error_recessive_cnt++;
    ecsr_reset_mask ^= BIT(4);
  }

  if (ecsr & BIT(5))
  {
    can_error_counters.bit_error_dominant_cnt++;
    ecsr_reset_mask ^= BIT(5);
  }

  if (ecsr & BIT(6))
  {
    can_error_counters.ack_delimiter_cnt++;
    ecsr_reset_mask ^= BIT(6);
  }

  R_CAN0->ECSR = ecsr_reset_mask;
  R_CAN0->EIFR = eifr_reset_mask;


  R_ICU->IELSRn_b[can0_error_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void can0_fifo_rx_isr(void)
{

  R_ICU->IELSRn_b[can0_fifo_rx_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void can0_fifo_tx_isr(void)
{


  R_ICU->IELSRn_b[can0_fifo_tx_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void can0_mailbox_rx_isr(void)
{
  uint8_t   mssr = R_CAN0->MSSR;
  uint8_t   mctl;
  uint8_t   box_num;
  T_can_msg mb;

  // Проверем есть ли какой либо майл бокс с полученным пакетом
  if ((mssr & BIT(7)) == 0)
  {
    box_num = mssr & 0x1F;  // Узнаем номер майлбокса

    mctl = R_CAN0->MCTLn_RX[box_num]; // Получаем контрольное слова майлбокса

    if (mctl & BIT(0)) // Проверяем флаг NEWDATA
    {
      // Тут возможно надо еще проверять флаг INVALDATA , но по мануалу он в этот момент должен быть уже сброшен
      // Получены новые данные извлекаем из R_CAN0->MBn[box_num].MBn

      mb.can_id = R_CAN0->MBn[box_num].MBn_ID & 0x1FFFFFFF;
      mb.len = R_CAN0->MBn[box_num].MBn_DL_b.DLC;
      mb.data[0] = R_CAN0->MBn[box_num].MBn_D[0];
      mb.data[1] = R_CAN0->MBn[box_num].MBn_D[1];
      mb.data[2] = R_CAN0->MBn[box_num].MBn_D[2];
      mb.data[3] = R_CAN0->MBn[box_num].MBn_D[3];
      mb.data[4] = R_CAN0->MBn[box_num].MBn_D[4];
      mb.data[5] = R_CAN0->MBn[box_num].MBn_D[5];
      mb.data[6] = R_CAN0->MBn[box_num].MBn_D[6];
      mb.data[7] = R_CAN0->MBn[box_num].MBn_D[7];

      tx_queue_send(&can0_rx_queue,&mb,  TX_NO_WAIT);

      R_CAN0->MCTLn_RX[box_num] = mctl ^ BIT(0); // Сбрасываем флаг принятых данных
    }

  }

  R_ICU->IELSRn_b[can0_mailbox_rx_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void can0_mailbox_tx_isr(void)
{
  uint8_t          mctl;
  ULONG            enqueued;
  T_can_msg        mb = {0};

  mctl = R_CAN0->MCTLn_TX[31]; // Получаем контрольное слова майлбокса

  R_CAN0->MCTLn_TX[31] = 0; // Сбрасываем флаг отправки данных
  R_CAN0->MCTLn_TX[31] = 0;


  if (mctl & BIT(0)) // Проверяем флаг SENTDATA
  {

    // Проверяем пуста ли очередь
    if (tx_queue_info_get(&can0_tx_queue,TX_NULL,&enqueued,TX_NULL,TX_NULL, TX_NULL,TX_NULL) == TX_SUCCESS)
    {
      if (enqueued > 0)
      {
        // Извлекаем новый пакет из очереди и отправлем его здесь
        if (tx_queue_receive(&can0_tx_queue,&mb, TX_NO_WAIT) == TX_SUCCESS)
        {
          CAN0_send_mailbox(&mb);
        }
      }
    }
  }

  R_ICU->IELSRn_b[can0_mailbox_tx_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  __DSB();
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static uint32_t CAN0_go_to_reset_state(void)
{
  uint32_t n;


  if (R_CAN0->STR_b.SLPST == 1)
  {
    // Находимся в режиме Sleep. Нужно выключить его
    R_CAN0->CTLR_b.SLPM = 0;

    n = 0;
    while (R_CAN0->STR_b.SLPST == 1)
    {
      DELAY_1us;
      if (++n > 10000) return RES_ERROR;
    }
  }

  R_CAN0->CTLR_b.CANM = 1; // Команда перехода в режим Reset
  n = 0;
  while (R_CAN0->STR_b.RSTST == 0)
  {
    DELAY_1us;
    if (++n > 10000) return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t CAN0_go_to_halt_state(void)
{
  uint32_t n;

  if (R_CAN0->STR_b.HLTST == 1)  return RES_OK;

  R_CAN0->CTLR_b.CANM = 2; // Команда перехода в режим Halt
  n = 0;
  while (R_CAN0->STR_b.HLTST == 0)
  {
    DELAY_1us;
    if (++n > 10000) return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t CAN0_go_to_operation_state(void)
{
  uint32_t n;
  if (R_CAN0->STR_b.SLPST == 1) return RES_ERROR;
  if ((R_CAN0->STR_b.HLTST == 0) && (R_CAN0->STR_b.RSTST == 0)) return RES_OK;

  R_CAN0->CTLR_b.CANM = 0; // Команда перехода в режим Operation
  n = 0;
  while ((R_CAN0->STR_b.HLTST != 0) || (R_CAN0->STR_b.RSTST != 0))
  {
    DELAY_1us;
    if (++n > 10000) return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param box_num
  \param ID
-----------------------------------------------------------------------------------------------------*/
static void CAN0_set_mailbox_to_recieve(uint8_t box_num, uint32_t msg_id)
{
  R_CAN0->MCTLn_RX[box_num] = 0;
  R_CAN0->MBn[box_num].MBn_ID = msg_id;
  R_CAN0->MBn[box_num].MBn_DL = 8; // Длина сообщения
  R_CAN0->MBn[box_num].MBn_D[0] = 0;
  R_CAN0->MBn[box_num].MBn_D[1] = 0;
  R_CAN0->MBn[box_num].MBn_D[2] = 0;
  R_CAN0->MBn[box_num].MBn_D[3] = 0;
  R_CAN0->MBn[box_num].MBn_D[4] = 0;
  R_CAN0->MBn[box_num].MBn_D[5] = 0;
  R_CAN0->MBn[box_num].MBn_D[6] = 0;
  R_CAN0->MBn[box_num].MBn_D[7] = 0;
  R_CAN0->MBn[box_num].MBn_TS_b.TSL = 0;
  R_CAN0->MBn[box_num].MBn_TS_b.TSH = 0;
  R_CAN0->MCTLn_RX_b[box_num].RECREQ = 1; // Запуск работы приемника

}

/*-----------------------------------------------------------------------------------------------------


  \param box_num
  \param msg_id
-----------------------------------------------------------------------------------------------------*/
static void CAN0_set_mailbox_to_stop(uint8_t box_num, uint32_t msg_id)
{
  R_CAN0->MCTLn_RX_b[box_num].RECREQ = 0; // Остановка работы приемника
  R_CAN0->MCTLn_RX[box_num]         = 0;
  R_CAN0->MBn[box_num].MBn_ID       = 0;
  R_CAN0->MBn[box_num].MBn_DL_b.DLC = 0; // Длина сообщения
  R_CAN0->MBn[box_num].MBn_D[0]     = 0;
  R_CAN0->MBn[box_num].MBn_D[1]     = 0;
  R_CAN0->MBn[box_num].MBn_D[2]     = 0;
  R_CAN0->MBn[box_num].MBn_D[3]     = 0;
  R_CAN0->MBn[box_num].MBn_D[4]     = 0;
  R_CAN0->MBn[box_num].MBn_D[5]     = 0;
  R_CAN0->MBn[box_num].MBn_D[6]     = 0;
  R_CAN0->MBn[box_num].MBn_D[7]     = 0;
  R_CAN0->MBn[box_num].MBn_TS_b.TSL = 0;
  R_CAN0->MBn[box_num].MBn_TS_b.TSH = 0;

}


/*-----------------------------------------------------------------------------------------------------


  \param p_mb
-----------------------------------------------------------------------------------------------------*/
static void CAN0_send_mailbox(T_can_msg  *p_mb)
{
  R_CAN0->MBn[31].MBn_ID = p_mb->can_id;
  R_CAN0->MBn[31].MBn_ID_b.IDE = 1;
  R_CAN0->MBn[31].MBn_ID_b.RTR = 0;

  R_CAN0->MBn[31].MBn_DL_b.DLC = p_mb->len;
  R_CAN0->MBn[31].MBn_D[0]     = p_mb->data[0];
  R_CAN0->MBn[31].MBn_D[1]     = p_mb->data[1];
  R_CAN0->MBn[31].MBn_D[2]     = p_mb->data[2];
  R_CAN0->MBn[31].MBn_D[3]     = p_mb->data[3];
  R_CAN0->MBn[31].MBn_D[4]     = p_mb->data[4];
  R_CAN0->MBn[31].MBn_D[5]     = p_mb->data[5];
  R_CAN0->MBn[31].MBn_D[6]     = p_mb->data[6];
  R_CAN0->MBn[31].MBn_D[7]     = p_mb->data[7];

  R_CAN0->MCTLn_TX_b[31].TRMREQ = 1; // Запуск работы передатчика
}


/*-----------------------------------------------------------------------------------------------------


  \param p_queue
-----------------------------------------------------------------------------------------------------*/
static void CAN0_receved_notify(TX_QUEUE *p_queue)
{
  tx_event_flags_set(&can0_flags, CAN0_MSG_RECEIVED, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param p_queue
-----------------------------------------------------------------------------------------------------*/
static void CAN0_sended_notify(TX_QUEUE *p_queue)
{
  tx_event_flags_set(&can0_flags, CAN0_MSG_TO_SEND, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t CAN0_create_RTOS_services(void)
{
  if (tx_event_flags_create(&can0_flags, "CAN0") != TX_SUCCESS)
  {
    return RES_ERROR;
  }

  if (tx_queue_create(&can0_rx_queue, (CHAR *) "CAN0_RX", sizeof(T_can_msg) / sizeof(uint32_t),can0_rx_queue_buf, sizeof(T_can_msg) * RX_MESSAGES_RING_SZ) != TX_SUCCESS)
  {
    tx_event_flags_delete(&can0_flags);
    return RES_ERROR;
  }
  tx_queue_send_notify(&can0_rx_queue, CAN0_receved_notify);

  if (tx_queue_create(&can0_tx_queue, (CHAR *) "CAN0_TX", sizeof(T_can_msg) / sizeof(uint32_t),can0_tx_queue_buf, sizeof(T_can_msg) * TX_MESSAGES_RING_SZ) != TX_SUCCESS)
  {
    tx_event_flags_delete(&can0_flags);
    tx_queue_delete(&can0_rx_queue);
    return RES_ERROR;
  }
  tx_queue_send_notify(&can0_tx_queue, CAN0_sended_notify);

  return RES_OK;
}


typedef union
{
    struct
    {
        uint32_t  CCLKS      :  1;               // CAN Clock Source Selection
        uint32_t             :  7;               //
        uint32_t  TSEG2      :  3;               // Time Segment 2 Control
        uint32_t             :  1;               //
        uint32_t  SJW        :  2;               // Resynchronization Jump Width Control
        uint32_t             :  2;               //
        uint32_t  BRP        : 10;               // Prescaler Division Ratio Select . These bits set the frequency  of the CAN communication clock (fCANCLK).
        uint32_t             :  2;               //
        uint32_t  TSEG1      :  4;               // Time Segment 1 Control
    };
    uint32_t v;
}
T_BCR_b;

/*-----------------------------------------------------------------------------------------------------
   Частота тактирования CAN периферии здесь равна PCLKB = 60 МГц
   Скорость по умолчанию 555555 бит в сек

-----------------------------------------------------------------------------------------------------*/
uint32_t CAN_init(uint8_t speed_code)
{
  uint16_t reg16;

  // Отключаем входы VBATT Input Control Register которые конфликтуют с функцией CAN на портах P402 P403
  R_SYSTEM->PRCR                = 0xA50B; // Открываем доступ на запись к регистрам
  R_SYSTEM->VBTICTLR_b.VCH0INEN = 0;
  R_SYSTEM->VBTICTLR_b.VCH1INEN = 0;
  R_SYSTEM->VBTICTLR_b.VCH2INEN = 0;


  R_MSTP->MSTPCRB_b.MSTPB2 = 0; // Разрешаем работу модуля CAN0
  DELAY_1us;

  if (CAN0_go_to_reset_state() != RES_OK) return RES_ERROR;

  CAN0_set_and_clear_interrupts();

  R_CAN0->MIER = 0; // Запрещаем прерывания от всех майлбоксов

  T_BCR_b bcr;

  bcr.v     = 0;

  if (speed_code == CAN_SPEED_555555)
  {
    bcr.CCLKS = 0;  // Тактирование от будет производится от сигнала PCLKB (60 МГц)
    bcr.BRP   = 5;  // 6  - 1
    bcr.TSEG2 = 4;  // 5  - 1
    bcr.TSEG1 = 11; // 12 - 1
    bcr.SJW   = 2;  // 3  - 1
  }
  else if (speed_code == CAN_SPEED_100000)
  {
    bcr.CCLKS = 0;  // Тактирование от будет производится от сигнала PCLKB (60 МГц)
    bcr.BRP   = 32; // 33 - 1
    bcr.TSEG2 = 3;  // 4  - 1
    bcr.TSEG1 = 12; // 13 - 1
    bcr.SJW   = 2;  // 3  - 1
  }
  else
  {
    bcr.CCLKS = 0;  // Тактирование от будет производится от сигнала PCLKB (60 МГц)
    bcr.BRP   = 5;  // 6  - 1
    bcr.TSEG2 = 4;  // 5  - 1
    bcr.TSEG1 = 11; // 12 - 1
    bcr.SJW   = 2;  // 3  - 1
  }

  R_CAN0->BCR = bcr.v;

  //if (CAN0_go_to_halt_state() != RES_OK) return RES_ERROR;

  // Конфигурируем регистр CTLR. Его можно конфигурировать только в режиме halt
  reg16 = 0
         + LSHIFT(0, 15) // Reserver  |
         + LSHIFT(0, 14) // Reserver  |
         + LSHIFT(1, 13) // RBOC      | Forcible Return from Bus-Off
                         //             0: Nothing occurred
                         //             1: Forced return from bus-off state
         //
         + LSHIFT(0, 11) // BOM[1:0]  | Bus-Off Recovery Mode
                         //             00: Normal mode (ISO11898-1-compliant)
                         //             01: Enter CAN halt mode automatically on entering bus-off state
                         //             10: Enter CAN halt mode automatically on end of bus-off state
                         //             11: Enter CAN halt mode during bus-off recovery period through a software request.
                         //
         + LSHIFT(0, 10) // SLPM      | CAN Sleep Mode
                         //             0: All other modes
                         //             1: CAN sleep mode
                         //
         + LSHIFT(2,  8) // CANM[1:0] | CAN Mode of Operation Select
                         //             00: CAN operation mode
                         //             01: CAN reset mode
                         //             10: CAN halt mode
                         //             11: CAN reset mode (forced transition).
                         //
         + LSHIFT(0,  6) // TSPS[1:0] | Time Stamp Prescaler Select
                         //             00: Every bit time
                         //             01: Every 2-bit time
                         //             10: Every 4-bit time
                         //             11: Every 8-bit time
                         //
         + LSHIFT(1,  5) // TSRC      | Time Stamp Counter Reset Command
                         //             0: Nothing occurred
                         //             1: Reset (This bit automatically is set to 0 after being set to 1. It should read as 0.)
                         //
         + LSHIFT(0,  4) // TPM       | Transmission Priority Mode Select
                         //             0: ID priority transmit mode
                         //             1: Mailbox number priority transmit mode.
                         //
         + LSHIFT(0,  3) // MLM       | Message Lost Mode Select
                         //             0: Overwrite mode
                         //             1: Overrun mode.
                         //
         + LSHIFT(1,  1) // IDFM[1:0] | ID Format Mode Select
                         //             00: Standard ID mode All mailboxes, including FIFO mailboxes, handle only standard IDs.
                         //             01: Extended ID mode All mailboxes, including FIFO mailboxes, handle only extended IDs
                         //             10: Mixed ID mode. All mailboxes, including FIFO mailboxes, handle both standard and extended IDs.
                         //             11: Setting prohibited
                         //
         + LSHIFT(0,  0) // MBM       | CAN Mailbox Mode Select
                         //             0: Normal mailbox mode
                         //             1: FIFO mailbox mode
  ;
  R_CAN0->CTLR = reg16;

  R_CAN0->MKIVLR = 0; // Для всех майл боксов действуют маски из регистров MKRx

  // Маски программируются так что принимаются пакеты с любыми идентификаторами
  R_CAN0->MKRn[0] = 0; // Маска для майлбоксов 0 to 3    | в каждой позиции бит 0 - означает отсутствие необходимости сравнения с соответствующим битом идентификатора в майлбоксе
  R_CAN0->MKRn[1] = 0; // Маска для майлбоксов 4 to 7    |
  R_CAN0->MKRn[2] = 0; // Маска для майлбоксов 8 to 11   |
  R_CAN0->MKRn[3] = 0; // Маска для майлбоксов 12 to 15  |
  R_CAN0->MKRn[4] = 0; // Маска для майлбоксов 16 to 19  |
  R_CAN0->MKRn[5] = 0; // Маска для майлбоксов 20 to 23  |
  R_CAN0->MKRn[6] = 0; // Маска для майлбоксов 24 to 27  |
  R_CAN0->MKRn[7] = 0; // Маска для майлбоксов  28 to 31 |

  R_CAN0->MSMR_b.MBSM = 0; //  Mailbox Search настраиваем на поиск майлбокса с принятыми данными


  CAN0_enable_interrupts();

  R_CAN0->MIER = 0XFFFFFFFF; // Разрешаем прерывания от всех майлбоксов
  R_CAN0->EIER = 0xFF;       // Разрешаем прерывания от всех возможных ошибок


  // 31 майлбокс сконфигурировать на прием пакетов с любыми идентификаторами
  for (uint32_t i=0; i < 31; i++)
  {
    CAN0_set_mailbox_to_recieve(i, 0);
  }


  if (CAN0_go_to_operation_state() != RES_OK)
  {
    R_CAN0->EIER = 0; // Запрещаем все прерывания от ошибок
    R_CAN0->MIER = 0; // Запрещаем прерывания от всех майлбоксов
    CAN0_go_to_reset_state();
    return RES_ERROR;
  }


  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
   Частота тактирования CAN периферии здесь равна PCLKB = 60 МГц

   Бит состоит из 18-и квантов.
   CAN бит состоит из последовательности битовых сегментов:
   SYNC_SEG   PROP_SEG   PHASE_SEG1   PHASE_SEG2
                                    ^
                                    |
                                    |_________В этой точке берется отсчет принимаемого бита


   Для скорости 555555 бит в сек
   ----------------------------------------
   Предделитель частоты = 6. Получаем квант Tq = 6/60000000 = 0.1 мкс Длительность бита = 0.1*18 = 1.8
   Длина бита вычисляется как: SYNC_SEG + PROP_SEG + PHASE_SEG1 + PHASE_SEG2 = 18Tq = 18*0.1 = 1.8 мкс

   BRP[9:0]                           = 6   Предделитель частоты
   SYNC_SEG                           = 1   Не настраивается. Всегда равен 1
   TSEG2[2:0] = PHASE_SEG2            = 5
   SJW[1:0                            = 3   Эта переменная не влияет на скорость.
   TSEG1[3:0] = PROP_SEG + PHASE_SEG1 = 12

   Для скорости 100000 бит в сек
   ----------------------------------------
   Предделитель частоты = 6. Получаем квант Tq = 33/60000000 = 0.55 мкс Длительность бита = 0.55*18 = 9.9 мкс
   Длина бита вычисляется как: SYNC_SEG + PROP_SEG + PHASE_SEG1 + PHASE_SEG2 = 18Tq = 18*0.55 = 9.9 мкс

   BRP[9:0]                           = 33  Предделитель частоты
   SYNC_SEG                           = 1   Не настраивается. Всегда равен 1
   TSEG2[2:0] = PHASE_SEG2            = 4
   SJW[1:0                            = 3   Эта переменная не влияет на скорость.
   TSEG1[3:0] = PROP_SEG + PHASE_SEG1 = 13

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t CAN_reinit(uint8_t speed_code)
{
  uint16_t reg16;

  R_MSTP->MSTPCRB_b.MSTPB2 = 0; // Разрешаем работу модуля CAN0
  DELAY_1us;

  if (CAN0_go_to_reset_state() != RES_OK) return RES_ERROR;

  CAN0_set_and_clear_interrupts();

  // Остановить работу 31 майлбокса
  for (uint32_t i=0; i < 31; i++)
  {
    CAN0_set_mailbox_to_stop(i, 0);
  }


  R_CAN0->MIER = 0; // Запрещаем прерывания от всех майлбоксов

  if (speed_code == CAN_SPEED_555555)
  {
    R_CAN0->BCR_b.CCLKS = 0;  // Тактирование от будет производится от сигнала PCLKB (60 МГц)
    R_CAN0->BCR_b.BRP   = 5;  // 6  - 1
    R_CAN0->BCR_b.TSEG2 = 4;  // 5  - 1
    R_CAN0->BCR_b.TSEG1 = 11; // 12 - 1
    R_CAN0->BCR_b.SJW   = 2;  // 3  - 1
  }
  else if (speed_code == CAN_SPEED_100000)
  {
    R_CAN0->BCR_b.CCLKS = 0;  // Тактирование от будет производится от сигнала PCLKB (60 МГц)
    R_CAN0->BCR_b.BRP   = 32; // 33 - 1
    R_CAN0->BCR_b.TSEG2 = 3;  // 4  - 1
    R_CAN0->BCR_b.TSEG1 = 12; // 13 - 1
    R_CAN0->BCR_b.SJW   = 2;  // 3  - 1
  }
  else
  {
    R_CAN0->BCR_b.CCLKS = 0;  // Тактирование от будет производится от сигнала PCLKB (60 МГц)
    R_CAN0->BCR_b.BRP   = 5;  // 6  - 1
    R_CAN0->BCR_b.TSEG2 = 4;  // 5  - 1
    R_CAN0->BCR_b.TSEG1 = 11; // 12 - 1
    R_CAN0->BCR_b.SJW   = 2;  // 3  - 1
  }


  if (CAN0_go_to_halt_state() != RES_OK) return RES_ERROR;

  // Конфигурируем регистр CTLR. Его можно конфигурировать только в режиме halt
  reg16 = 0
         + LSHIFT(0, 15) // Reserver  |
         + LSHIFT(0, 14) // Reserver  |
         + LSHIFT(0, 13) // RBOC      | Forcible Return from Bus-Off
                         //             0: Nothing occurred
         //             1: Forced return from bus-off state
         //
         + LSHIFT(0, 11) // BOM[1:0]  | Bus-Off Recovery Mode
                         //             00: Normal mode (ISO11898-1-compliant)
         //             01: Enter CAN halt mode automatically on entering bus-off state
         //             10: Enter CAN halt mode automatically on end of bus-off state
         //             11: Enter CAN halt mode during bus-off recovery period through a software request.
         //
         + LSHIFT(0, 10) // SLPM      | CAN Sleep Mode
                         //             0: All other modes
         //             1: CAN sleep mode
         //
         + LSHIFT(2,  8) // CANM[1:0] | CAN Mode of Operation Select
                         //             00: CAN operation mode
         //             01: CAN reset mode
         //             10: CAN halt mode
         //             11: CAN reset mode (forced transition).
         //
         + LSHIFT(0,  6) // TSPS[1:0] | Time Stamp Prescaler Select
                         //             00: Every bit time
         //             01: Every 2-bit time
         //             10: Every 4-bit time
         //             11: Every 8-bit time
         //
         + LSHIFT(1,  5) // TSRC      | Time Stamp Counter Reset Command
                         //             0: Nothing occurred
         //             1: Reset (This bit automatically is set to 0 after being set to 1. It should read as 0.)
         //
         + LSHIFT(0,  4) // TPM       | Transmission Priority Mode Select
                         //             0: ID priority transmit mode
         //             1: Mailbox number priority transmit mode.
         //
         + LSHIFT(0,  3) // MLM       | Message Lost Mode Select
                         //             0: Overwrite mode
         //             1: Overrun mode.
         //
         + LSHIFT(1,  1) // IDFM[1:0] | ID Format Mode Select
                         //             00: Standard ID mode All mailboxes, including FIFO mailboxes, handle only standard IDs.
         //             01: Extended ID mode All mailboxes, including FIFO mailboxes, handle only extended IDs
         //             10: Mixed ID mode. All mailboxes, including FIFO mailboxes, handle both standard and extended IDs.
         //             11: Setting prohibited
         //
         + LSHIFT(0,  0) // MBM       | CAN Mailbox Mode Select
                         //             0: Normal mailbox mode
  //             1: FIFO mailbox mode
  ;
  R_CAN0->CTLR = reg16;

  R_CAN0->MKIVLR = 0; // Для всех майл боксов действуют маски из регистров MKRx

  // Маски программируются так что принимаются пакеты с любыми идентификаторами
  R_CAN0->MKRn[0] = 0; // Маска для майлбоксов 0 to 3    | в каждой позиции бит 0 - означает отсутствие необходимости сравнения с соответствующим битом идентификатора в майлбоксе
  R_CAN0->MKRn[1] = 0; // Маска для майлбоксов 4 to 7    |
  R_CAN0->MKRn[2] = 0; // Маска для майлбоксов 8 to 11   |
  R_CAN0->MKRn[3] = 0; // Маска для майлбоксов 12 to 15  |
  R_CAN0->MKRn[4] = 0; // Маска для майлбоксов 16 to 19  |
  R_CAN0->MKRn[5] = 0; // Маска для майлбоксов 20 to 23  |
  R_CAN0->MKRn[6] = 0; // Маска для майлбоксов 24 to 27  |
  R_CAN0->MKRn[7] = 0; // Маска для майлбоксов  28 to 31 |

  R_CAN0->MSMR_b.MBSM = 0; //  Mailbox Search настраиваем на поиск майлбокса с принятыми данными

  CAN0_enable_interrupts();

  R_CAN0->MIER = 0XFFFFFFFF; // Разрешаем прерывания от всех майлбоксов
  R_CAN0->EIER = 0xFF;       // Разрешаем прерывания от всех возможных ошибок

  if (CAN0_go_to_operation_state() != RES_OK)
  {
    R_CAN0->EIER = 0; // Запрещаем все прерывания от ошибок
    R_CAN0->MIER = 0; // Запрещаем прерывания от всех майлбоксов
    CAN0_go_to_reset_state();
    return RES_ERROR;
  }

  // 31 майлбокс сконфигурировать на прием пакетов с любыми идентификаторами
  for (uint32_t i=0; i < 31; i++)
  {
    CAN0_set_mailbox_to_recieve(i, 0);
  }

  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Помещение пакета на отправку в очередь.
  При помещении в очередь автоматически будет взведен флаг CAN0_MSG_TO_SEND


  \param canid
  \param data
  \param len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t CAN0_post_packet_to_send(uint32_t canid, uint8_t *data, uint8_t len)
{
  T_can_msg mb = {0};

  if (Is_CAN0_task_ready() == 0) return RES_ERROR;

  mb.can_id = canid;
  memcpy(mb.data , data, len);
  mb.len = len;

  if (tx_queue_send(&can0_tx_queue,&mb,  TX_NO_WAIT) != TX_SUCCESS)
  {
    return RES_ERROR;
  }

  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t Is_CAN0_in_error_state(void)
{
  return R_CAN0->STR_b.BOST | R_CAN0->STR_b.EPST;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t  Is_CAN0_task_ready(void)
{
  if (can_task_ready == 0) Wait_ms(2);
  return can_task_ready;
}

/*-----------------------------------------------------------------------------------------------------


  \param arg
-----------------------------------------------------------------------------------------------------*/
static void CAN_task(ULONG arg)
{
  ULONG            actual_flags;
  T_can_msg        mb = {0};

  if (CAN0_create_RTOS_services() != RES_OK) return;
  if (CAN_init(wvar.can_speed_select) != RES_OK) return;

  can_task_ready = 1;

  // Здесь цикл ожидания появления в очереди пакетов на отсылку и организация отсылки пакетов

  do
  {
    actual_flags = 0;
    if (tx_event_flags_get(&can0_flags, 0xFFFFFFFF, TX_OR_CLEAR,&actual_flags, 1) == TX_SUCCESS)
    {
      if (actual_flags & CAN0_MSG_TO_SEND)
      {
        // Проверить не продолжается ли еще отправка предыдущего пакета
        if (R_CAN0->MCTLn_TX_b[31].TRMREQ == 0)
        {
          // Майл бокс не назначен как передающий и значит свободен для отправки
          // Извлекаем  пакет из очереди и отправлем его здесь
          if (tx_queue_receive(&can0_tx_queue,&mb, TX_NO_WAIT) == TX_SUCCESS)
          {
            CAN0_send_mailbox(&mb);
          }
        }
      }

      if (actual_flags & CAN0_MSG_RECEIVED)
      {
        Set_output_can_active_blink(OUTP_LED_CAN);
        if (tx_queue_receive(&can0_rx_queue,&mb, TX_NO_WAIT) == TX_SUCCESS)
        {
          CAN_packet_reciever(&mb);
        }
      }
    }

    can0_recr =R_CAN0->RECR;
    can0_tecr =R_CAN0->TECR;
    can0_str = (T_can_str)(R_CAN0->STR);

  } while (1);

}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Thread_CAN_create(void)
{
  UINT              err;

  err = tx_thread_create(
                         &can_thread,
                         (CHAR *)"CAN0",
                         CAN_task,
                         0,
                         can_stacks,
                         CAN_TASK_STACK_SIZE,
                         CAN_TASK_PRIO,
                         CAN_TASK_PRIO,
                         1,
                         TX_AUTO_START
                        );

  if (err == TX_SUCCESS)
  {
    APPLOG("CAN task created.");
    return RES_OK;
  }
  else
  {
    APPLOG("CAN creating error %d.", err);
    return RES_ERROR;
  }
}


