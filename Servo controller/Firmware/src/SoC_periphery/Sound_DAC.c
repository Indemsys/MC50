// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-02-26
// 14:27:39
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

int16_t                    audio_buf[2][AUDIO_BUF_SAMPLES_NUM];

const int16_t  *dest_pointers[2] = {audio_buf[1] , audio_buf[0]};
const uint16_t  dest_bl_sz[2]    = {AUDIO_BUF_SAMPLES_NUM , AUDIO_BUF_SAMPLES_NUM};
const uint8_t   int_mask = 0x41;

// Структура повторяющая управляющую структуру DTC
transfer_info_t DAC0_dtc[] =
{
  // DM[1:0]         | DTS             | DISEL            | CHNS & CHNE     | SM[1:0]         | SZ[1:0]       |  MD[1:0]      | SAR              | DAR                               | CRB              | CRA
  // Destination     | DTC Transfer    | DTC Interrupt    | DTC Chain       | Source          | DTC Data      |  DTC Transfer | DTC Transfer     | DTC Transfer                      | DTC Transfer     | DTC Transfer
  // Addressing Mode | Mode Select     | Select           | Transfer Select | Addressing Mode | Transfer Size |  Mode Select  | Source Register  | Destination Register              | Count Register B | Count Register A

  // Пересылаем аудиобуфер в DAC
  {ADDR_FIXED,        DST_REPEATED,    DISEL_0_IR_RES,    CHAIN_AFTER_LOOP,   ADDR__INCR,      UNIT_2_BYTE,      MODE_NORMAL   ,&audio_buf[0]    ,(void *)&R_DAC->DADRn[1]           ,0        ,AUDIO_BUF_SAMPLES_NUM },

  {ADDR_FIXED,        SRC_REPEATED,    DISEL_0_IR_RES,    CHAIN_NO_LOOPING,   ADDR__INCR,      UNIT_4_BYTE,      MODE_REPEAT   ,dest_pointers    ,((uint32_t *)&DAC0_dtc[0].p_src),   0        ,(2<<8)+2 },

  {ADDR_FIXED,        SRC_REPEATED,    DISEL_0_IR_RES,    CHAIN_NO_LOOPING,   ADDR__INCR,      UNIT_2_BYTE,      MODE_REPEAT   ,dest_bl_sz       ,((uint16_t *)&DAC0_dtc[0].length),  0        ,(2<<8)+2 },

  // При обработке этой строки движок DTC посылает маску в регистр ELSEGR1 и на последней пересылке маска содержит бит вызова прерывания по событию ELC_EVENT_ELC_SOFTWARE_EVENT_0
  // Наличие DISEL=0 (DISEL_0_IR_RES) в этой записи критично. В ином случае не происходит циклического повторения пересылок
  {ADDR_FIXED,        SRC_REPEATED,    DISEL_0_IR_RES,    CHAIN___DISABLED,   ADDR_FIXED,      UNIT_1_BYTE,      MODE_REPEAT   ,&int_mask        ,(void *)&R_ELC->ELSEGRnRC0[0]      ,0        ,(2<<8)+2 },

};

extern const transfer_instance_t   DTC_DAC1_AGT_inst;
dtc_instance_ctrl_t                DTC_DAC1_ctrl;

transfer_cfg_t DTC_DAC1_AGT_cfg =
{
  .p_info              = DAC0_dtc,
  .activation_source   = ELC_EVENT_AGT1_INT,
  .auto_enable         = false,              // Отключен автозапуск DTC после открытия драйвера командой open
  .p_callback          = NULL,               // Используется только в случае использования в качестве источника прерываний ELC_EVENT_ELC_SOFTWARE_EVENT_0 или ELC_EVENT_ELC_SOFTWARE_EVENT_1
                                             // И если включена опция компиляции DTC_CFG_SOFTWARE_START_ENABLE
  .p_context           =&DTC_DAC1_AGT_inst,
  .irq_ipl             =(BSP_IRQ_DISABLED)
};

/* Instance structure to use this module. */
const transfer_instance_t DTC_DAC1_AGT_inst =
{
  .p_ctrl             =&DTC_DAC1_ctrl,
  .p_cfg              =&DTC_DAC1_AGT_cfg,
  .p_api              =&g_transfer_on_dtc
};

static IRQn_Type               elc_soft0_int_num;
static IRQn_Type               agt1_int_num;
static void  elc_segr0_isr(void);
static void  agt1_isr(void);

SSP_VECTOR_DEFINE(elc_segr0_isr,  ELC,  SOFTWARE_EVENT_0)
SSP_VECTOR_DEFINE_CHAN(agt1_isr, AGT, INT, 1);

/*-----------------------------------------------------------------------------------------------------
  Используем прерывание по событию ELC_EVENT_ELC_SOFTWARE_EVENT_0 для установки флага окончания пересылки по DTC массива данных в DAC

  Выбран именно такой способ, поскольку вызвать прерывания записью в регитсры NVIC не удалось.
  Пересылка DTC, по всей видимости, не может выполнить запись в область системных регистров ARM с адреса 0xE0000000

  \param void
-----------------------------------------------------------------------------------------------------*/
static void  elc_segr0_isr(void)
{
  Player_Set_evt_to_read_next_block();

  R_ICU->IELSRn_b[elc_soft0_int_num].IR = 0;    // Сбрасываем IR флаг в ICU
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------
  Не используется, но надо объявить иначе будет ошибка при открытии канала DTC

  \param void
-----------------------------------------------------------------------------------------------------*/
static void  agt1_isr(void)
{
  R_ICU->IELSRn_b[agt1_int_num].IR = 0;    // Сбрасываем IR флаг в ICU
  __DSB();
}

/*-----------------------------------------------------------------------------------------------------
  Обновить частоту сэмплирования проигрывателя

  \param sr
-----------------------------------------------------------------------------------------------------*/
void Sound_DAC_update_sample_rate(uint32_t sample_rate)
{
  if (sample_rate > PLAYER_MAX_SAMPLE_RATE) sample_rate = PLAYER_MAX_SAMPLE_RATE;

  Init_AGT1_ticks_to_DTC(AGT_CLOCK_FREQ / sample_rate);

}


/*-----------------------------------------------------------------------------------------------------
  Узнаем c какой половиной аудиобуфера в данный момент рабаотает DMA

  \param bnk
-----------------------------------------------------------------------------------------------------*/
uint8_t Sound_DAC_get_current_buf(void)
{
  if ((uint32_t)DAC0_dtc[0].p_src < (uint32_t)(&audio_buf[1][0]))
  {
    return 0;
  }
  return  1;
}


/*------------------------------------------------------------------------------
   Заполнение аудиосэмплами тишины
 ------------------------------------------------------------------------------*/
void Sound_DAC_clear_for_silience(int16_t *dest, uint32_t samples_num)
{
  uint32_t i;
  for (i = 0; i < samples_num; i++)
  {
    *dest =  DAC_MID_LEVEL;
    dest++;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Sound_DAC_init_player_periphery(uint32_t sample_rate)
{
  Init_DAC();

  elc_soft0_int_num    = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_ELC_SOFTWARE_EVENT_0);
  agt1_int_num         = (IRQn_Type)Find_IRQ_number_by_evt(ELC_EVENT_AGT1_INT);
  NVIC_DisableIRQ(elc_soft0_int_num);
  NVIC_DisableIRQ(agt1_int_num);
  NVIC_ClearPendingIRQ(elc_soft0_int_num);
  NVIC_ClearPendingIRQ(agt1_int_num);
  R_ICU->IELSRn_b[elc_soft0_int_num].IR = 0;  // Сбрасываем IR флаг в ICU
  R_ICU->IELSRn_b[agt1_int_num].IR = 0;       // Сбрасываем IR флаг в ICU
  NVIC_SetPriority(elc_soft0_int_num,  INT_DAC_DTC_PRIO);
  NVIC_EnableIRQ(elc_soft0_int_num);

  Sound_DAC_clear_for_silience(&audio_buf[0][0], AUDIO_BUF_SAMPLES_NUM * 2);

  // Используем регистр ELSEGRnRC0[0] для вызова прерываний по окончании циклов пересылки с помощью механизма DTC
  R_ELC->ELSEGRnRC0[0].ELSEGRn_b.WI = 0; // Сбросом в 0 разрешаем запись в регистр
  R_ELC->ELSEGRnRC0[0].ELSEGRn_b.WE = 1; // Установкой в 1 разрешаем запись в бит прерываний

  Sound_DAC_update_sample_rate(PLAYER_DEFAULT_SAMPLE_RATE);
  DTC_DAC1_AGT_inst.p_api->open(DTC_DAC1_AGT_inst.p_ctrl,DTC_DAC1_AGT_inst.p_cfg);
  DTC_DAC1_AGT_inst.p_api->enable(DTC_DAC1_AGT_inst.p_ctrl);
}



