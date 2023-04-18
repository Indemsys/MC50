// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-01-27
// 18:08:36
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"




//........................................................................................
//  Организация канала передачи для SD карты
//  Используется модуль DMAC канал 2
//........................................................................................
SSP_VECTOR_DEFINE_CHAN(dmac_int_isr,        DMAC, INT, 2);

SSP_VECTOR_DEFINE_CHAN(sdhimmc_accs_isr,    SDHIMMC, ACCS,    0);
SSP_VECTOR_DEFINE_CHAN(sdhimmc_card_isr,    SDHIMMC, CARD,    0);
SSP_VECTOR_DEFINE_CHAN(sdhimmc_dma_req_isr, SDHIMMC, DMA_REQ, 0);

dmac_instance_ctrl_t DMA_CH2_control_block;

transfer_info_t DMA_CH2_transfer_info =
{
  .dest_addr_mode      = TRANSFER_ADDR_MODE_FIXED,
  .repeat_area         = TRANSFER_REPEAT_AREA_SOURCE,
  .irq                 = TRANSFER_IRQ_EACH,
  .chain_mode          = TRANSFER_CHAIN_MODE_DISABLED,
  .src_addr_mode       = TRANSFER_ADDR_MODE_INCREMENTED,
  .size                = TRANSFER_SIZE_1_BYTE,
  .mode                = TRANSFER_MODE_NORMAL,
  .p_dest              = (void *) NULL,
  .p_src               = (void const *) NULL,
  .num_blocks          = 0,
  .length              = 0,
};

const transfer_on_dmac_cfg_t DMA_CH2_extended_configuration =
{
  .channel             = 2,
  .offset_byte         = 0,
};

const transfer_cfg_t DMA_CH2_configuration =
{
  .p_info              =&DMA_CH2_transfer_info,
  .activation_source   = ELC_EVENT_ADC0_COMPARE_MATCH,
  .auto_enable         = false,
  .p_callback          = NULL,
  .p_context           =&DMA_CH2_transfer_instance,
  .irq_ipl             =(2),
  .p_extend            =&DMA_CH2_extended_configuration,
};

const transfer_instance_t DMA_CH2_transfer_instance =
{
  .p_ctrl        =&DMA_CH2_control_block,
  .p_cfg         =&DMA_CH2_configuration,
  .p_api         =&g_transfer_on_dmac
};



