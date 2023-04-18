#include "hal_data.h"


SSP_VECTOR_DEFINE(rtc_carry_isr, RTC, CARRY);

rtc_instance_ctrl_t g_rtc0_ctrl;

const rtc_cfg_t g_rtc0_cfg =
{
  .clock_source           = RTC_CLOCK_SOURCE_SUBCLK,
  .hw_cfg                 = true,
  .error_adjustment_value = 0,
  .error_adjustment_type  = RTC_ERROR_ADJUSTMENT_NONE,
  .p_callback             = NULL,
  .p_context              =&g_rtc0,
  .alarm_ipl              =(BSP_IRQ_DISABLED),
  .periodic_ipl           =(BSP_IRQ_DISABLED),
  .carry_ipl              =(12),
};


const rtc_instance_t g_rtc0 =
{
  .p_ctrl        =&g_rtc0_ctrl,
  .p_cfg         =&g_rtc0_cfg,
  .p_api         =&g_rtc_on_rtc
};

SSP_VECTOR_DEFINE(fcu_frdyi_isr, FCU, FRDYI);
SSP_VECTOR_DEFINE(fcu_fiferr_isr, FCU, FIFERR);

flash_hp_instance_ctrl_t g_flash0_ctrl;

const flash_cfg_t g_flash0_cfg =
{
  .data_flash_bgo      = true,
  .p_callback          = NULL,
  .p_context           =&g_flash0,
  .irq_ipl             =(10),
  .err_irq_ipl         =(10),
};

const flash_instance_t g_flash0 =
{
  .p_ctrl        =&g_flash0_ctrl,
  .p_cfg         =&g_flash0_cfg,
  .p_api         =&g_flash_on_flash_hp
};

