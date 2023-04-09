// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2020-01-30
// 10:51:25
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"



#define  FLAG_FLASH_OP_DONE   BIT(0)      // Флаг завершения операции FLASH




flash_hp_instance_ctrl_t flash_ctrl;

extern const flash_instance_t flash_cbl;

static void  Flash_callback(flash_callback_args_t *p_args);

const flash_cfg_t flash_cfg =
{
  .data_flash_bgo      = true,
  .p_callback          = Flash_callback,
  .p_context           =&flash_cbl,
  .irq_ipl             =(10),
  .err_irq_ipl         =(10),
};

const flash_instance_t flash_cbl =
{
  .p_ctrl        =&flash_ctrl,
  .p_cfg         =&flash_cfg,
  .p_api         =&g_flash_on_flash_hp
};


static TX_EVENT_FLAGS_GROUP   flash_flags;
static uint32_t               flash_op_status;



/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Init_Flash_driver(void)
{
  UINT     res;

  // Создаем объект синхронизации
  res = tx_event_flags_create(&flash_flags, "Flash");
  if (res != TX_SUCCESS)
  {
    APPLOG("Failed to create event. Error %d", res);
    return RES_ERROR;
  }

  flash_cbl.p_api->open(&flash_ctrl,&flash_cfg);

  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Функция вызываема после окончания оперции с DataFlash

  \param p_args
-----------------------------------------------------------------------------------------------------*/
static void  Flash_callback(flash_callback_args_t *p_args)
{
  switch (p_args->event)
  {
  case FLASH_EVENT_ERASE_COMPLETE :
    flash_op_status|=BIT_FLASH_EVENT_ERASE_COMPLETE;
    break;
  case FLASH_EVENT_WRITE_COMPLETE :
    flash_op_status|=BIT_FLASH_EVENT_WRITE_COMPLETE;
    break;
  case FLASH_EVENT_BLANK          :
    flash_op_status|=BIT_FLASH_EVENT_BLANK;
    break;
  case FLASH_EVENT_NOT_BLANK      :
    flash_op_status|=BIT_FLASH_EVENT_NOT_BLANK;
    break;
  case FLASH_EVENT_ERR_DF_ACCESS  :
    flash_op_status|=BIT_FLASH_EVENT_ERR_DF_ACCESS;
    break;
  case FLASH_EVENT_ERR_CF_ACCESS  :
    flash_op_status|=BIT_FLASH_EVENT_ERR_CF_ACCESS;
    break;
  case FLASH_EVENT_ERR_CMD_LOCKED :
    flash_op_status|=BIT_FLASH_EVENT_ERR_CMD_LOCKED;
    break;
  case FLASH_EVENT_ERR_FAILURE    :
    flash_op_status|=BIT_FLASH_EVENT_ERR_FAILURE;
    break;
  case FLASH_EVENT_ERR_ONE_BIT    :
    flash_op_status|=BIT_FLASH_EVENT_ERR_ONE_BIT;
    break;
  }
  tx_event_flags_set(&flash_flags,FLAG_FLASH_OP_DONE,TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param flags
  \param actual_flags
  \param wait_option

  \return uint32_t - TX_SUCCESS  если флаг получен
-----------------------------------------------------------------------------------------------------*/
uint32_t Wait_for_flash_op_end(ULONG wait_option)
{
  ULONG  actual_flags = 0;
  return tx_event_flags_get(&flash_flags, FLAG_FLASH_OP_DONE, TX_OR_CLEAR,&actual_flags,  wait_option);
}


uint32_t Get_flash_op_status(void)
{
  return flash_op_status;
}

void Set_flash_op_status(uint32_t stat)
{
  flash_op_status = stat;
}


/*-----------------------------------------------------------------------------------------------------
  Стираем облать в DataFlash

  \param start_addr
  \param area_size

  \return int возвращает RES_OK если стирание произведено успешно
-----------------------------------------------------------------------------------------------------*/
uint32_t S7G2_DataFlash_EraseArea(uint32_t start_addr, uint32_t area_size)
{
  ssp_err_t res;
  UINT      os_res;
  uint32_t  addr;
  uint32_t  num_blocks;
  addr = start_addr & 0xFFFFFFC0; // Корректирруем адрес чтобы он был выровнен по границе 64-х байт
  num_blocks =(((start_addr + area_size - 1) & 0xFFFFFFC0)+ 0x40  - addr) / 0x40;

  Set_flash_op_status(0);
  res = flash_cbl.p_api->erase(flash_cbl.p_ctrl, addr , num_blocks);
  os_res = Wait_for_flash_op_end(ms_to_ticks(100));
  if (os_res != TX_SUCCESS) return RES_ERROR;
  if (res != SSP_SUCCESS)   return RES_ERROR;
  if ((Get_flash_op_status() & BIT_FLASH_EVENT_ERASE_COMPLETE) == 0) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param start_addr
  \param buf
  \param buf_size  - размер буфера должен быть кратен 4, поскольку запись возможна только по 4-е байта с выравниванием по границе 4

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t S7G2_DataFlash_WriteArea(uint32_t start_addr, uint8_t *buf, uint32_t buf_size)
{
  ssp_err_t res;
  UINT      os_res;

  Set_flash_op_status(0);
  res = flash_cbl.p_api->write(flash_cbl.p_ctrl, (uint32_t) buf  , start_addr, buf_size);
  os_res = Wait_for_flash_op_end(ms_to_ticks(100));
  if (os_res != TX_SUCCESS) return RES_ERROR;
  if (res != SSP_SUCCESS)   return RES_ERROR;
  if ((Get_flash_op_status() & BIT_FLASH_EVENT_WRITE_COMPLETE) == 0) return RES_ERROR;
  return RES_OK;
}

