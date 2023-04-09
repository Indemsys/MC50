/***********************************************************************************************************************
 * Copyright [2015-2021] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 * 
 * This file is part of Renesas SynergyTM Software Package (SSP)
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * This file is subject to a Renesas SSP license agreement. Unless otherwise agreed in an SSP license agreement with
 * Renesas: 1) you may not use, copy, modify, distribute, display, or perform the contents; 2) you may not use any name
 * or mark of Renesas for advertising or publicity purposes or in connection with your use of the contents; 3) RENESAS
 * MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED
 * "AS IS" WITHOUT ANY EXPRESS OR IMPLIED WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, AND NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR
 * CONSEQUENTIAL DAMAGES, INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF
 * CONTRACT OR TORT, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents
 * included in this file may be subject to different terms.
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * File Name    : r_sdmmc.c
 * Description  : SDMMC driver for SDHI/SDMMC device.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @file r_sdmmc.c
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "r_sdmmc.h"

#include "hw/hw_sdmmc.h"
#include "r_sdmmc_private_api.h"
#include "r_sdmmc_cfg.h"
#include "r_cgc.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** "SDHI" in ASCII, used to determine if channel is open. */
#define SDMMC_OPEN                (0x53444849U)
#define SDMMC_CLOSED              (0U)

#define SDMMC_CLK_CTRL_DIV_INVALID     (0xFFU)

/* Delay up to 250 ms per sector before timing out waiting for response or response timeout flag. */
#define SDMMC_ERASE_TIMEOUT_PER_SECTOR_US    (250000U)

/* Delay up to 10 ms before timing out waiting for response or response timeout flag. */
#define SDMMC_RESPONSE_TIMEOUT_US    (10000U)

/* Delay up to 5 seconds before timing out waiting for busy after updating bus width or high speed status for eMMC. */
#define SDMMC_BUSY_TIMEOUT_US        (5000000U)

/* Delay up to 500 ms before timing out waiting for data or data timeout flag. */
#define SDMMC_DATA_TIMEOUT_US        (500000U)

/* Delay up to 10 ms before timing out waiting for access end flag after receiving data during initialization. */
#define SDMMC_ACCESS_TIMEOUT_US      (10000U)

/* 400 kHz maximum clock required for initialization. */
#define SDMMC_INIT_MAX_CLOCK_RATE_HZ             (400000U)
#define SDMMC_BITS_PER_COMMAD                    (48U)
#define SDMMC_BITS_PER_RESPONSE                  (48U)
#define SDMMC_CLOCKS_BETWEEN_COMMANDS            (8U)
#define SDMMC_MIN_CYCLES_PER_COMMAND_RESPONSE   ((SDMMC_BITS_PER_COMMAD + \
                                                  SDMMC_BITS_PER_RESPONSE) + \
                                                  SDMMC_CLOCKS_BETWEEN_COMMANDS)
#define SDMMC_INIT_ONE_SECOND_TIMEOUT_ITERATIONS (SDMMC_INIT_MAX_CLOCK_RATE_HZ / SDMMC_MIN_CYCLES_PER_COMMAND_RESPONSE)

#define SDMMC_SDIO_REG_HIGH_SPEED         (0x13U)                  // SDIO High Speed register address
#define SDMMC_SDIO_REG_HIGH_SPEED_BIT_EHS (1U << 1)                // Enable high speed bit of SDIO high speed register
#define SDMMC_SDIO_REG_HIGH_SPEED_BIT_SHS (1U << 0)                // Support high speed bit of SDIO high speed register
#define SDMMC_CSD_REG_CCC_CLASS_10_BIT    ((uint32_t) (1U << 10))  // CCC_CLASS bit is set if the card supports high speed

/* Error bits in the R1 response. */
#define SDMMC_R1_ERROR_BITS (0xFDF98000U)

/* SDIO maximum bytes allows in writeIoExt() and readIoExt(). */
#define SDMMC_SDIO_EXT_MAX_BYTES  (512U)

/* SDIO maximum blocks allows in writeIoExt() and readIoExt(). */
#define SDMMC_SDIO_EXT_MAX_BLOCKS  (511U)

/* Masks for CMD53 argument. */
#define SDMMC_SDIO_CMD52_CMD53_COUNT_MASK    (0x1FFU)
#define SDMMC_SDIO_CMD52_CMD53_FUNCTION_MASK (0x7U)
#define SDMMC_SDIO_CMD52_CMD53_ADDRESS_MASK  (0x1FFFFU)

/* Startup delay in milliseconds. */
#define SDMMC_STARTUP_DELAY_MS          (37U)

/** Macro for error logger. */
#ifndef SDMMC_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define SDMMC_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_sdmmc_version)
#endif

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
#if SDMMC_CFG_PARAM_CHECKING_ENABLE
static ssp_err_t r_sdmmc_open_param_check(sdmmc_instance_ctrl_t * p_ctrl, sdmmc_cfg_t const * const p_cfg);
#endif

static ssp_err_t r_sdmmc_erase_error_check (sdmmc_instance_ctrl_t * const p_ctrl, uint32_t const start_sector, uint32_t const sector_count);

static ssp_err_t r_sdmmc_common_error_check(sdmmc_instance_ctrl_t * const p_ctrl, bool check_wp);

static void      r_sdmmc_irq_cfg(sdmmc_instance_ctrl_t * p_ctrl, IRQn_Type irq, uint8_t ipl);

static void      r_sdmmc_irq_disable(IRQn_Type irq);

static bool      r_sdmmc_clock_div_enable_get (sdmmc_instance_ctrl_t * p_ctrl);

static void      r_sdmmc_access_irq_process (sdmmc_instance_ctrl_t * p_ctrl, sdmmc_callback_args_t * event_flags);

static bool      r_sdmmc_command_send (sdmmc_instance_ctrl_t * p_ctrl, uint16_t command, uint32_t argument);

static bool      r_sdmmc_max_clock_rate_set (sdmmc_instance_ctrl_t * p_ctrl, uint32_t max_rate);

static bool      r_sdmmc_clock_div_set (sdmmc_instance_ctrl_t * p_ctrl, uint8_t divisor);

static bool      r_sdmmc_emmc_check (sdmmc_instance_ctrl_t * const p_ctrl);

static bool      r_sdmmc_sdio_check (sdmmc_instance_ctrl_t * const p_ctrl);

static bool      r_sdmmc_sd_card_check (sdmmc_instance_ctrl_t * const p_ctrl);

static bool      r_sdmmc_emmc_clock_optimize (sdmmc_instance_ctrl_t * const p_ctrl);

static bool      r_sdmmc_clock_optimize (sdmmc_instance_ctrl_t * const p_ctrl, sdmmc_priv_csd_reg_t * const p_csd_reg);

static bool      r_sdmmc_sdio_clock_optimize (sdmmc_instance_ctrl_t * const p_ctrl);

static bool      r_sdmmc_sd_high_speed (sdmmc_instance_ctrl_t * const p_ctrl);

static bool      r_sdmmc_csd_save (sdmmc_instance_ctrl_t * const p_ctrl, sdmmc_priv_csd_reg_t * const p_csd_reg);

static bool      r_sdmmc_csd_extended_get (sdmmc_instance_ctrl_t * const p_ctrl);

static bool      r_sdmmc_read_and_block (sdmmc_instance_ctrl_t * const p_ctrl, uint16_t command, uint32_t argument, uint32_t byte_count);

static bool      r_sdmmc_rca_get (sdmmc_instance_ctrl_t * const p_ctrl);

static bool      r_sdmmc_bus_width_set (sdmmc_instance_ctrl_t * const p_ctrl);

static ssp_err_t r_sdmmc_sdhi_cfg (sdmmc_instance_ctrl_t * const p_ctrl, sdmmc_extended_cfg_t const * const p_extend);

static ssp_err_t r_sdmmc_card_identify (sdmmc_instance_ctrl_t * const p_ctrl, sdmmc_extended_cfg_t const * const p_extend);

static ssp_err_t r_sdmmc_bus_cfg (sdmmc_instance_ctrl_t * const p_ctrl);

static void      r_sdmmc_write_protect_get (sdmmc_instance_ctrl_t * const p_ctrl);

static ssp_err_t r_sdmmc_read_write_common(sdmmc_instance_ctrl_t * const p_ctrl,
                                    uint32_t   block_count,
                                    uint16_t   block_size,
                                    uint16_t   command,
                                    uint32_t   argument);

static void      r_sdmmc_software_copy(void const * p_src, uint32_t bytes, void * p_dest);

static void      r_sdmmc_transfer_callback(transfer_callback_args_t * p_args);

static ssp_err_t r_sdmmc_transfer_read (sdmmc_instance_ctrl_t * const p_ctrl,
                                   uint32_t        block_count,
                                   uint32_t        bytes,
                                   uint8_t const * p_data);

static ssp_err_t r_sdmmc_transfer_write (sdmmc_instance_ctrl_t * const p_ctrl,
                                   uint32_t             block_count,
                                   uint32_t             bytes,
                                   const uint8_t      * p_data);

static ssp_err_t r_sdmmc_cmd52 (sdmmc_instance_ctrl_t * const        p_ctrl,
                            uint8_t  * const            p_data,
                            uint32_t const              function,
                            uint32_t const              address,
                            sdmmc_io_write_mode_t const read_after_write,
                            uint32_t const              command);

static void      r_sdmmc_transfer_end (sdmmc_instance_ctrl_t * const p_ctrl);

static void      r_sdmmc_card_inserted_get (sdmmc_instance_ctrl_t * const p_ctrl);

static ssp_err_t r_sdmmc_control_error_check(sdmmc_instance_ctrl_t * const p_ctrl, ssp_command_t const command, void * p_data);

void        sdhimmc_accs_isr (void);

void        sdhimmc_card_isr (void);

void        sdhimmc_dma_req_isr (void);

void        sdhimmc_sdio_isr (void);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/** Card Specific Data  */

#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug. This pragma suppresses the warnings in this
 * structure only, and will be removed when the SSP compiler is updated to v5.3.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t g_sdmmc_version =
{
    .api_version_minor  = SDMMC_API_VERSION_MINOR,
    .api_version_major  = SDMMC_API_VERSION_MAJOR,
    .code_version_major = SDMMC_CODE_VERSION_MAJOR,
    .code_version_minor = SDMMC_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "sdmmc";
#endif

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/
/** SDMMC function pointers   */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const sdmmc_api_t g_sdmmc_on_sdmmc =
{
    .open        = R_SDMMC_Open,
    .close       = R_SDMMC_Close,
    .read        = R_SDMMC_Read,
    .write       = R_SDMMC_Write,
    .control     = R_SDMMC_Control,
    .readIo      = R_SDMMC_ReadIo,
    .writeIo     = R_SDMMC_WriteIo,
    .readIoExt   = R_SDMMC_ReadIoExt,
    .writeIoExt  = R_SDMMC_WriteIoExt,
    .IoIntEnable = R_SDMMC_IoIntEnable,
    .versionGet  = R_SDMMC_VersionGet,
    .infoGet     = R_SDMMC_InfoGet,
    .erase       = R_SDMMC_Erase
};

/*******************************************************************************************************************//**
 * @addtogroup SDMMC
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Initializes the SDHI hardware and completes identification and configuration for the SD or eMMC device.  This
 * procedure requires several sequential commands.  This API blocks until all identification and configuration
 * commands are complete.
 *
 * For SDIO, SDIO interrupts are enabled after card identification is complete.  SDIO interrupts can be disabled using
 * sdmmc_api_t::ioIntEnable().
 *
 * Implements sdmmc_api_t::open().
 *
 * @retval SSP_SUCCESS               Port is available and is now open for read/write/control access.
 * @retval SSP_ERR_ASSERTION         Null Pointer or block size is not in the valid range of 1-512.
 * @retval SSP_ERR_INVALID_ARGUMENT  Block size must be 512 bytes for SD cards and eMMC devices.  It is configurable
 *                                   for SDIO only.
 * @retval SSP_ERR_ALREADY_OPEN      Driver has already been opened with this instance of the control structure.
 * @retval SSP_ERR_HW_LOCKED         The channel specified has already been opened.
 * @retval SSP_ERR_CARD_INIT_FAILED  Hardware related failure occurred, with the MCU or with the card itself.
 * @retval SSP_ERR_IRQ_BSP_DISABLED  Access interrupt is not enabled.
 * @retval SSP_ERR_CARD_NOT_INSERTED Card detection is enabled and no card is plugged in.
 * @return                           See @ref Common_Error_Codes or functions called by this function for other possible
 *                                   return codes. This function calls:
 *                                      * fmi_api_t::productFeatureGet
 *                                      * fmi_api_t::eventInfoGet
 *                                      * cgc_api_t::systemClockFreqGet
 *
 * @note This function is reentrant for different channels.  It is not reentrant for the same channel.
 **********************************************************************************************************************/
ssp_err_t R_SDMMC_Open (sdmmc_ctrl_t * const p_api_ctrl, sdmmc_cfg_t const * const p_cfg)
{
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t ret_val = SSP_SUCCESS;   // setup return value for success

#if SDMMC_CFG_PARAM_CHECKING_ENABLE
    /* Check valid input parameter before configuration*/
    ret_val = r_sdmmc_open_param_check(p_ctrl, p_cfg);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);
#endif

    /** Verify the requested hardware channel exists on the MCU. */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = p_cfg->hw.channel;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_SDHIMMC;
    fmi_feature_info_t info = {0U};
    ssp_err_t err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Configure interrupts. */
    IRQn_Type access_irq  = SSP_INVALID_VECTOR;
    IRQn_Type card_irq    = SSP_INVALID_VECTOR;
    IRQn_Type sdio_irq    = SSP_INVALID_VECTOR;
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_SDHIMMC_ACCS, &event_info);
    access_irq = event_info.irq;

    /* Access interrupt is required. */
    SDMMC_ERROR_RETURN(SSP_INVALID_VECTOR != access_irq, SSP_ERR_IRQ_BSP_DISABLED);

    /** Acquire lock before changing vector table or p_ctrl. */
    SDMMC_ERROR_RETURN(SSP_SUCCESS == R_BSP_HardwareLock(&ssp_feature), SSP_ERR_HW_LOCKED);

    r_sdmmc_irq_cfg(p_ctrl, access_irq, p_cfg->access_ipl);

    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_SDHIMMC_CARD, &event_info);
    card_irq = event_info.irq;
    r_sdmmc_irq_cfg(p_ctrl, card_irq, p_cfg->card_ipl);

    /* The SDIO and DMA_REQ interrupts are configured here, but enabled later in the driver. */
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_SDHIMMC_SDIO, &event_info);
    sdio_irq = event_info.irq;
    r_sdmmc_irq_cfg(p_ctrl, sdio_irq, p_cfg->sdio_ipl);

    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_SDHIMMC_DMA_REQ, &event_info);
    r_sdmmc_irq_cfg(p_ctrl, event_info.irq, p_cfg->dma_req_ipl);

    /* Initialize control block. */
    *p_ctrl                       = (const sdmmc_instance_ctrl_t) {0U};
    p_ctrl->transfer_irq          = event_info.irq;
    p_ctrl->p_reg                 = info.ptr;
    p_ctrl->p_context             = p_cfg->p_context;
    p_ctrl->p_callback            = p_cfg->p_callback;
    p_ctrl->hw                    = p_cfg->hw;
    p_ctrl->status.bus_width      = p_cfg->hw.bus_width;
    p_ctrl->p_lower_lvl_transfer  = p_cfg->p_lower_lvl_transfer;
    p_ctrl->status.sector_size    = SDMMC_MAX_BLOCK_SIZE;
    if (NULL != p_cfg->p_extend)
    {
        /* Set user's block size if provided. */
        sdmmc_extended_cfg_t * p_ext = (sdmmc_extended_cfg_t *) p_cfg->p_extend;
        p_ctrl->status.sector_size = p_ext->block_size;
    }

    /** Turn off module stop bit (turn module on). */
    R_BSP_ModuleStart(&ssp_feature);

    /** Perform the identification procedure for SD card or eMMC device. */
    ret_val = r_sdmmc_card_identify(p_ctrl, p_cfg->p_extend);

    if (SSP_SUCCESS == ret_val)
    {
        /** Configure bus clock, block size, and bus width. */
        ret_val = r_sdmmc_bus_cfg(p_ctrl);
    }

    if (SSP_SUCCESS == ret_val)
    {
        /** Check to see if the card is write protected (SD cards only). */
        r_sdmmc_write_protect_get(p_ctrl);

        p_ctrl->status.ready = true;
        p_ctrl->open = SDMMC_OPEN;
    }
    else
    {
        /* Disable SDHI interrupts. */
        r_sdmmc_irq_disable(access_irq);
        r_sdmmc_irq_disable(card_irq);
        r_sdmmc_irq_disable(sdio_irq);

        /* Turn on module stop bit (turn module off). */
        R_BSP_ModuleStop(&ssp_feature);

        /* Release hardware lock. */
        R_BSP_HardwareUnlock(&ssp_feature);
    }

    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);
    return ret_val;
}

/*******************************************************************************************************************//**
 * Closes an open SD/MMC device.  Implements sdmmc_api_t::close().
 *
 * @retval  SSP_SUCCESS                  Successful close.
 * @retval  SSP_ERR_ASSERTION            The parameter p_ctrl is NULL.
 * @retval  SSP_ERR_NOT_OPEN             Driver has not been initialized.
 *
 * @note This function is reentrant for different channels.  It is not reentrant for the same channel.
 **********************************************************************************************************************/
ssp_err_t R_SDMMC_Close (sdmmc_ctrl_t * const p_api_ctrl)
{
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) p_api_ctrl;

#if SDMMC_CFG_PARAM_CHECKING_ENABLE
    /* Check pointers for NULL values */
    SSP_ASSERT(NULL != p_ctrl);

    SDMMC_ERROR_RETURN(SDMMC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    p_ctrl->open = SDMMC_CLOSED;

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = p_ctrl->hw.channel;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_SDHIMMC;

    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_SDHIMMC_ACCS, &event_info);
    IRQn_Type access_irq = event_info.irq;
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_SDHIMMC_SDIO, &event_info);
    IRQn_Type sdio_irq = event_info.irq;
    g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_SDHIMMC_CARD, &event_info);
    IRQn_Type card_irq = event_info.irq;

    /** Disable SDHI interrupts. */
    r_sdmmc_irq_disable(access_irq);
    r_sdmmc_irq_disable(card_irq);
    r_sdmmc_irq_disable(sdio_irq);

    /** Close the transfer driver. */
    p_ctrl->p_lower_lvl_transfer->p_api->close(p_ctrl->p_lower_lvl_transfer->p_ctrl);

    /* Clear any pending interrupt flags */
    HW_SDMMC_Info1Set(p_ctrl->p_reg, SDMMC_SDHI_INFO1_ACCESS_MASK);
    HW_SDMMC_Info2Set(p_ctrl->p_reg, SDMMC_SDHI_INFO2_MASK);

    /** Turn on module stop bit (turn module off). */
    R_BSP_ModuleStop(&ssp_feature);

    /** Release hardware lock. */
    R_BSP_HardwareUnlock(&ssp_feature);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Reads data from an SD or eMMC device.  Up to 0x10000 sectors can be read at a time.
 * Implements sdmmc_api_t::read().
 *
 * This function blocks until the command is sent and the response is received.  A callback with the event
 * SDMMC_EVENT_TRANSFER_COMPLETE is called when the read data is available.
 *
 * @retval  SSP_SUCCESS                  Data read successfully.
 * @retval  SSP_ERR_ASSERTION            NULL pointer.
 * @retval  SSP_ERR_NOT_OPEN             Driver has not been initialized.
 * @retval  SSP_ERR_CARD_NOT_READY       Card was unplugged.
 * @retval  SSP_ERR_TRANSFER_BUSY        Driver is busy with a previous operation.
 * @retval  SSP_ERR_READ_FAILED          Read operation failed.
 *
 * @note This function is reentrant for different channels.  It is not reentrant for the same channel.
 **********************************************************************************************************************/
ssp_err_t R_SDMMC_Read (sdmmc_ctrl_t * const p_api_ctrl, uint8_t * const p_dest, uint32_t const start_sector,
                        uint32_t const sector_count)
{
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t ret_val = SSP_SUCCESS;

#if SDMMC_CFG_PARAM_CHECKING_ENABLE
    /* Check pointers for NULL values */
    SSP_ASSERT(NULL != p_dest);
#endif

    /* Perform a error check on valid parameter and card status */
    ret_val = r_sdmmc_common_error_check(p_ctrl, false);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    /** Configure the transfer interface for reading. */
    ret_val = r_sdmmc_transfer_read(p_ctrl, sector_count, p_ctrl->status.sector_size, p_dest);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, SSP_ERR_READ_FAILED);

    uint16_t command = 0U;
    uint32_t argument = start_sector;
    if (!p_ctrl->status.hc)
    {
        /* Standard capacity SD cards and some eMMC devices use byte addressing. */
        argument *= p_ctrl->status.sector_size;
    }

    if (sector_count > 1U)
    {
        command = SDMMC_CMD_READ_MULTIPLE_BLOCK;
    }
    else
    {
        command = SDMMC_CMD_READ_SINGLE_BLOCK;
    }

    /* Casting to uint16_t safe because block size verified in R_SDMMC_Open */
    /** Read data from SD or eMMC device. */
    ret_val = r_sdmmc_read_write_common(p_ctrl, sector_count, (uint16_t) p_ctrl->status.sector_size, command, argument);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, SSP_ERR_READ_FAILED);

    return ret_val;
}

/*******************************************************************************************************************//**
 * Writes data to an SD or eMMC device.  Up to 0x10000 sectors can be written at a time.
 * Implements sdmmc_api_t::write().
 *
 * This function blocks until the command is sent and the response is received.  A callback with the event
 * SDMMC_EVENT_TRANSFER_COMPLETE is called when the all data has been written.
 *
 * @retval  SSP_SUCCESS                  Card write finished successfully.
 * @retval  SSP_ERR_ASSERTION            Handle or Source address is NULL.
 * @retval  SSP_ERR_NOT_OPEN             Driver has not been initialized.
 * @retval  SSP_ERR_CARD_NOT_READY       Card was unplugged.
 * @retval  SSP_ERR_TRANSFER_BUSY        Driver is busy with a previous operation.
 * @retval  SSP_ERR_WRITE_PROTECTED      SD card is Write Protected.
 * @retval  SSP_ERR_WRITE_FAILED         Write operation failed.
 *
 * @note This function is reentrant for different channels.
 **********************************************************************************************************************/
ssp_err_t R_SDMMC_Write (sdmmc_ctrl_t * const p_api_ctrl, uint8_t const * const p_source, uint32_t const start_sector,
                         uint32_t const sector_count)
{
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t ret_val = SSP_SUCCESS;

#if SDMMC_CFG_PARAM_CHECKING_ENABLE
    /* Check pointers for NULL values */
    SSP_ASSERT(NULL != p_source);
#endif

    /* Perform a error check on valid parameter and card status */
    ret_val = r_sdmmc_common_error_check(p_ctrl, true);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    /** Configure the transfer interface for writing. */
    ret_val = r_sdmmc_transfer_write(p_ctrl, sector_count, p_ctrl->status.sector_size, p_source);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, SSP_ERR_WRITE_FAILED);

    /** Call SDMMC protocol write function */
    uint16_t command = 0U;
    uint32_t argument = start_sector;
    if (!p_ctrl->status.hc)
    {
        /* Standard capacity SD cards and some eMMC devices use byte addressing. */
        argument *= p_ctrl->status.sector_size;
    }

    if (sector_count > 1U)
    {
        command = SDMMC_CMD_WRITE_MULTIPLE_BLOCK;
    }
    else
    {
        command = SDMMC_CMD_WRITE_SINGLE_BLOCK;
    }

    /* Casting to uint16_t safe because block size verified in R_SDMMC_Open */
    /** Write data to SD or eMMC device. */
    ret_val = r_sdmmc_read_write_common(p_ctrl, sector_count, (uint16_t) p_ctrl->status.sector_size, command, argument);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, SSP_ERR_WRITE_FAILED);

    return ret_val;
}

/*******************************************************************************************************************//**
 * Sends control commands to and receives the status of the SD/MMC device.  Implements sdmmc_api_t::control().
 *
 * @retval SSP_SUCCESS              Command executed successfully.
 * @retval SSP_ERR_ASSERTION        Null Pointer.
 * @retval SSP_ERR_INVALID_ARGUMENT Command is invalid.
 * @retval SSP_ERR_INVALID_SIZE     Block size not in valid range of 1-512 for SDIO or 512 only for SD cards and eMMC.
 *
 * @note This function is reentrant for different channels.  It is not reentrant for the same channel.
 **********************************************************************************************************************/
ssp_err_t R_SDMMC_Control (sdmmc_ctrl_t * const p_api_ctrl, ssp_command_t const command, void * p_data)
{
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t ret_val = SSP_SUCCESS;

    /* Perform a error check on input parameter */
    ret_val = r_sdmmc_control_error_check(p_ctrl, command, p_data);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    /** Get the command status and return to called function. */
    switch (command)
    {
        /* Get the media sector count. */
        case SSP_COMMAND_GET_SECTOR_COUNT:
            *((uint32_t *) p_data) = p_ctrl->status.sector_count;
            break;

        /* Get the size of sector */
        case SSP_COMMAND_GET_SECTOR_SIZE:
            *((uint32_t *) p_data) = p_ctrl->status.sector_size;
            break;

        /* Get the Write Protection status. */
        case SSP_COMMAND_GET_WRITE_PROTECTED:
            *((bool *) p_data) =  p_ctrl->status.write_protected;
            break;

        /* Set the block size. */
        case SSP_COMMAND_SET_BLOCK_SIZE:
            p_ctrl->status.sector_size = *((uint32_t *) p_data);
            break;

        /* Get flash sector release information. */
        case SSP_COMMAND_GET_SECTOR_RELEASE:
            *(uint8_t *)p_data = 0U;
            break;

        /* Set command is invalid */
        default:
            ret_val = SSP_ERR_INVALID_ARGUMENT;
            break;
    }
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);
    return ret_val;
}

/*******************************************************************************************************************//**
 * The Read function reads a one byte register from an SDIO card.  Implements sdmmc_api_t::readIo().
 *
 * This function blocks until the command is sent and the response is received.  p_data contains the register value
 * read when this function returns.
 *
 * @retval  SSP_SUCCESS                  Data read successfully.
 * @retval  SSP_ERR_ASSERTION            NULL pointer.
 * @retval  SSP_ERR_NOT_OPEN             Driver has not been initialized.
 * @retval  SSP_ERR_CARD_NOT_READY       Card was unplugged.
 * @retval  SSP_ERR_TRANSFER_BUSY        Driver is busy with a previous operation.
 * @retval  SSP_ERR_READ_FAILED          Read operation failed.
 *
 * @note This function is reentrant for different channels.  It is not reentrant for the same channel.
 **********************************************************************************************************************/
ssp_err_t R_SDMMC_ReadIo (sdmmc_ctrl_t * const p_api_ctrl,
                          uint8_t * const      p_data,
                          uint32_t const       function,
                          uint32_t const       address)
{
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t ret_val = SSP_SUCCESS;

#if SDMMC_CFG_PARAM_CHECKING_ENABLE
    /* Check pointers for NULL values */
    SSP_ASSERT(NULL != p_data);
#endif

    /* Perform a error check on valid parameter and card status */
    ret_val = r_sdmmc_common_error_check(p_ctrl, false);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    /** Call SDMMC protocol read function */
    *p_data = 0U;
    ret_val = r_sdmmc_cmd52(p_ctrl, p_data, function, address, SDMMC_IO_WRITE_MODE_NO_READ, SDMMC_SDIO_CMD52_READ);

    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, SSP_ERR_READ_FAILED);
    return ret_val;
}

/*******************************************************************************************************************//**
 * Writes a one byte register to an SDIO card.  Implements sdmmc_api_t::writeIo().
 *
 * This function blocks until the command is sent and the response is received.  The register has been written when
 * this function returns.  If read_after_write is true, p_data contains the register value read when this function
 * returns.
 *
 * @retval  SSP_SUCCESS                  Card write finished successfully.
 * @retval  SSP_ERR_ASSERTION            Handle or Source address is NULL.
 * @retval  SSP_ERR_NOT_OPEN             Driver has not been initialized.
 * @retval  SSP_ERR_CARD_NOT_READY       Card was unplugged.
 * @retval  SSP_ERR_TRANSFER_BUSY        Driver is busy with a previous operation.
 * @retval  SSP_ERR_WRITE_FAILED         Write operation failed.
 *
 * @note This function is reentrant for different channels.
 **********************************************************************************************************************/
ssp_err_t R_SDMMC_WriteIo (sdmmc_ctrl_t * const        p_api_ctrl,
                           uint8_t * const             p_data,
                           uint32_t const              function,
                           uint32_t const              address,
                           sdmmc_io_write_mode_t const read_after_write)
{
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t ret_val = SSP_SUCCESS;
#if SDMMC_CFG_PARAM_CHECKING_ENABLE
    /* Check pointers for NULL values */
    SSP_ASSERT(NULL != p_data);
#endif

    /* Perform a error check on valid parameter and card status */
    ret_val = r_sdmmc_common_error_check(p_ctrl, false);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    /** Call SDMMC protocol write function with valid parameters */
    ret_val = r_sdmmc_cmd52(p_ctrl, p_data, function, address, read_after_write, SDMMC_SDIO_CMD52_WRITE);

    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, SSP_ERR_WRITE_FAILED);
    return ret_val;
}

/*******************************************************************************************************************//**
 * Reads data from an SDIO card function.  Implements sdmmc_api_t::readIoExt().
 *
 * This function blocks until the command is sent and the response is received.  A callback with the event
 * SDMMC_EVENT_TRANSFER_COMPLETE is called when the read data is available.
 *
 * @retval  SSP_SUCCESS                  Data read successfully.
 * @retval  SSP_ERR_ASSERTION            NULL pointer, or count is not in the valid range of 1-512 for byte mode or
 *                                       1-511 for block mode.
 * @retval  SSP_ERR_NOT_OPEN             Driver has not been initialized.
 * @retval  SSP_ERR_CARD_NOT_READY       Card was unplugged.
 * @retval  SSP_ERR_TRANSFER_BUSY        Driver is busy with a previous operation.
 * @retval  SSP_ERR_READ_FAILED          Read operation failed.
 *
 * @note This function is reentrant for different channels.  It is not reentrant for the same channel.
 **********************************************************************************************************************/
ssp_err_t R_SDMMC_ReadIoExt (sdmmc_ctrl_t * const     p_api_ctrl,
                             uint8_t * const          p_dest,
                             uint32_t const           function,
                             uint32_t const           address,
                             uint32_t * const         count,
                             sdmmc_io_transfer_mode_t transfer_mode,
                             sdmmc_io_address_mode_t  address_mode)
{
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t ret_val = SSP_SUCCESS;

#if SDMMC_CFG_PARAM_CHECKING_ENABLE
    /* Check pointers for NULL values */
    SSP_ASSERT(NULL != p_dest);
    SSP_ASSERT(0U != (*count));
#endif

    /* Perform a error check on valid parameter and card status */
    ret_val = r_sdmmc_common_error_check(p_ctrl, false);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    uint16_t command     = SDMMC_CMD_IO_READ_EXT_SINGLE_BLOCK;
    uint16_t byte_count  = 0U;
    uint32_t block_count = 0U;
    if (SDMMC_IO_MODE_TRANSFER_BLOCK == transfer_mode)
    {
#if SDMMC_CFG_PARAM_CHECKING_ENABLE
        SSP_ASSERT((*count) <= SDMMC_SDIO_EXT_MAX_BLOCKS);
#endif
        block_count = *count;
        byte_count  = (uint16_t) p_ctrl->status.sector_size;
        if (block_count > 1U)
        {
            command |= SDMMC_CMD_IO_EXT_MULTI_BLOCK;
        }
    }
    else
    {
#if SDMMC_CFG_PARAM_CHECKING_ENABLE
        SSP_ASSERT((*count) <= SDMMC_SDIO_EXT_MAX_BYTES);
#endif
        block_count = 1U;
        byte_count = (uint16_t) *count;
    }

    /** Configure the transfer interface for reading. */
    ret_val = r_sdmmc_transfer_read(p_ctrl, block_count, byte_count, p_dest);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, SSP_ERR_READ_FAILED);

    sdmmc_priv_sdio_arg_t argument = {0U};
    /* According to SDIO spec, 512 = 0. */
    argument.cmd_53_arg.count            = (*count & SDMMC_SDIO_CMD52_CMD53_COUNT_MASK);
    argument.cmd_53_arg.function_number  = (function & SDMMC_SDIO_CMD52_CMD53_FUNCTION_MASK);
    argument.cmd_53_arg.block_mode       = transfer_mode;
    argument.cmd_53_arg.op_code          = address_mode;
    argument.cmd_53_arg.register_address = (address & SDMMC_SDIO_CMD52_CMD53_ADDRESS_MASK);
    argument.cmd_53_arg.rw_flag          = 0U;

    /** Read data from SDIO device. */
    ret_val = r_sdmmc_read_write_common(p_ctrl, block_count, byte_count, command, argument.arg);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, SSP_ERR_READ_FAILED);

    return ret_val;
}

/*******************************************************************************************************************//**
 * Writes data to an SDIO card function.  Implements sdmmc_api_t::writeIoExt().
 *
 * This function blocks until the command is sent and the response is received.  A callback with the event
 * SDMMC_EVENT_TRANSFER_COMPLETE is called when the all data has been written.
 *
 * @retval  SSP_SUCCESS                  Card write finished successfully.
 * @retval  SSP_ERR_ASSERTION            NULL pointer, or count is not in the valid range of 1-512 for byte mode or
 *                                       1-511 for block mode.
 * @retval  SSP_ERR_NOT_OPEN             Driver has not been initialized.
 * @retval  SSP_ERR_CARD_NOT_READY       Card was unplugged.
 * @retval  SSP_ERR_TRANSFER_BUSY        Driver is busy with a previous operation.
 * @retval  SSP_ERR_WRITE_FAILED         Write operation failed.
 *
 * @note This function is reentrant for different channels.
 **********************************************************************************************************************/
ssp_err_t R_SDMMC_WriteIoExt (sdmmc_ctrl_t * const     p_api_ctrl,
                              uint8_t const * const    p_source,
                              uint32_t const           function,
                              uint32_t const           address,
                              uint32_t const           count,
                              sdmmc_io_transfer_mode_t transfer_mode,
                              sdmmc_io_address_mode_t  address_mode)
{
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t ret_val = SSP_SUCCESS;

#if SDMMC_CFG_PARAM_CHECKING_ENABLE
    /* Check pointers for NULL values */
    SSP_ASSERT(NULL != p_source);
    SSP_ASSERT(0U != count);
#endif

    /* Perform a error check on valid parameter and card status */
    ret_val = r_sdmmc_common_error_check(p_ctrl, false);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    uint16_t command     = SDMMC_CMD_IO_WRITE_EXT_SINGLE_BLOCK;
    uint16_t byte_count  = 0U;
    uint32_t block_count = 0U;
    if (SDMMC_IO_MODE_TRANSFER_BLOCK == transfer_mode)
    {
#if SDMMC_CFG_PARAM_CHECKING_ENABLE
        SSP_ASSERT(count <= SDMMC_SDIO_EXT_MAX_BLOCKS);
#endif
        block_count = count;
        byte_count  = (uint16_t) p_ctrl->status.sector_size;
        if (block_count > 1U)
        {
            command |= SDMMC_CMD_IO_EXT_MULTI_BLOCK;
        }
    }
    else
    {
#if SDMMC_CFG_PARAM_CHECKING_ENABLE
        SSP_ASSERT(count <= SDMMC_SDIO_EXT_MAX_BYTES);
#endif
        block_count = 1U;
        byte_count = (uint16_t) count;
    }

    /** Configure the transfer interface for writing. */
    ret_val = r_sdmmc_transfer_write(p_ctrl, block_count, byte_count, p_source);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, SSP_ERR_WRITE_FAILED);

    sdmmc_priv_sdio_arg_t argument = {0U};
    /* According to SDIO spec, 512 = 0. */
    argument.cmd_53_arg.count            = (count & SDMMC_SDIO_CMD52_CMD53_COUNT_MASK);
    argument.cmd_53_arg.function_number  = (function & SDMMC_SDIO_CMD52_CMD53_FUNCTION_MASK);
    argument.cmd_53_arg.block_mode       = transfer_mode;
    argument.cmd_53_arg.op_code          = address_mode;
    argument.cmd_53_arg.register_address = (address & SDMMC_SDIO_CMD52_CMD53_ADDRESS_MASK);
    argument.cmd_53_arg.rw_flag          = 1U;

    /** Write data to SDIO device. */
    ret_val = r_sdmmc_read_write_common(p_ctrl, block_count, byte_count, command, argument.arg);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, SSP_ERR_WRITE_FAILED);

    return ret_val;
}

/*******************************************************************************************************************//**
 * Enables or disables the SDIO Interrupt.  Implements sdmmc_api_t::ioIntEnable().
 *
 * @retval  SSP_SUCCESS                  Card enabled or disabled SDIO interrupts successfully.
 * @retval  SSP_ERR_ASSERTION            NULL pointer.
 * @retval  SSP_ERR_TRANSFER_BUSY        Driver is busy with a previous operation.
 *
 * @note This function is reentrant for different channels.
 **********************************************************************************************************************/
ssp_err_t R_SDMMC_IoIntEnable (sdmmc_ctrl_t * const p_api_ctrl, bool enable)
{
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) p_api_ctrl;

#if SDMMC_CFG_PARAM_CHECKING_ENABLE
    /* Check pointers for NULL values */
    SSP_ASSERT (NULL != p_ctrl);
#endif

    /** Make sure the card is not busy. */
    SDMMC_ERROR_RETURN(!HW_SDMMC_CommandBusyGet(p_ctrl->p_reg), SSP_ERR_TRANSFER_BUSY);
    SDMMC_ERROR_RETURN(!HW_SDMMC_CardBusyGet(p_ctrl->p_reg), SSP_ERR_TRANSFER_BUSY);

    /** Enable or disable interrupt. */
    HW_SDMMC_SdioIrqEnable(p_ctrl->p_reg, enable);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Returns the version of the firmware and API.  Implements sdmmc_api_t::versionGet().
 *
 * @retval SSP_SUCCESS              Function executed successfully.
 * @retval SSP_ERR_ASSERTION        Null Pointer.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_SDMMC_VersionGet (ssp_version_t * const p_version)
{
    ssp_err_t ret_val = SSP_SUCCESS;

#if SDMMC_CFG_PARAM_CHECKING_ENABLE
    /* Check pointers for NULL values */
    SSP_ASSERT (NULL != p_version);
#endif
    p_version->version_id = g_sdmmc_version.version_id;

    return ret_val;
}

/*******************************************************************************************************************//**
 * Provides information about the connected device and driver status.  Implements sdmmc_api_t::infoGet().
 *
 * @retval SSP_SUCCESS              Function executed successfully.
 * @retval SSP_ERR_ASSERTION        Null Pointer.
 * @retval SSP_ERR_NOT_OPEN         Driver has not been initialized.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t R_SDMMC_InfoGet (sdmmc_ctrl_t * const p_api_ctrl, sdmmc_info_t * const p_info)
{
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) p_api_ctrl;

#if SDMMC_CFG_PARAM_CHECKING_ENABLE
    /* Check pointers for NULL values */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_info);

    SDMMC_ERROR_RETURN(SDMMC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    r_sdmmc_card_inserted_get(p_ctrl);

    /** Copy information stored during open. */
    *p_info = p_ctrl->status;

    /** Check if the card is busy. */
    p_info->transfer_in_progress = ((HW_SDMMC_CommandBusyGet(p_ctrl->p_reg)) ||
                                   (HW_SDMMC_CardBusyGet(p_ctrl->p_reg)));

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Erases sectors of an SD card or eMMC device.  Implements sdmmc_api_t::erase().
 *
 * This function blocks until erase is complete.
 *
 * @retval  SSP_SUCCESS                  Erase operation requested.
 * @retval  SSP_ERR_ASSERTION            A required pointer is NULL.
 * @retval  SSP_ERR_NOT_OPEN             Driver has not been initialized.
 * @retval  SSP_ERR_CARD_NOT_READY       Card was unplugged.
 * @retval  SSP_ERR_TRANSFER_BUSY        Driver is busy with a previous operation.
 * @retval  SSP_ERR_WRITE_PROTECTED      SD card is Write Protected.
 * @retval  SSP_ERR_ERASE_FAILED         Erase operation unsuccessful.
 *
 * @note This function is reentrant for different channels.
 **********************************************************************************************************************/
ssp_err_t R_SDMMC_Erase (sdmmc_ctrl_t * const p_api_ctrl, uint32_t const start_sector, uint32_t const sector_count)
{
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t ret_val = SSP_SUCCESS;
    uint32_t start_address;
    uint32_t end_address;
    uint16_t start_command;
    uint16_t end_command;
    uint32_t argument;

    /* Perform a error check on valid parameter and card status */
    ret_val = r_sdmmc_erase_error_check(p_ctrl, start_sector, sector_count);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    /*  SDHC, SDXC and eMMC high capacity media use block address. */
    if (true == p_ctrl->status.hc)
    {
        start_address = start_sector;
        end_address   = ((start_sector + sector_count) - 1);
    }
    else
    {
        start_address = (start_sector * p_ctrl->status.sector_size);
        end_address   = ((start_sector + sector_count) * p_ctrl->status.sector_size) - 1U;
    }

    if (SDMMC_CARD_TYPE_MMC == p_ctrl->status.card_type)
    {
        start_command = SDMMC_CMD_TAG_ERASE_GROUP_START;
        end_command   = SDMMC_CMD_TAG_ERASE_GROUP_END;
        argument      = SDMMC_EMMC_ERASE_ARGUMENT_TRIM;
    }
    else
    {
        start_command = SDMMC_CMD_ERASE_WR_BLK_START;
        end_command   = SDMMC_CMD_ERASE_WR_BLK_END;
        argument      = 0U;                               // Argument unused for SD
    }

    /** Send command to set start erase address (CMD35 for eMMC, CMD32 for SD). */
    bool command_return = false;
    command_return = r_sdmmc_command_send(p_ctrl, start_command, start_address);
    SDMMC_ERROR_RETURN(command_return, SSP_ERR_ERASE_FAILED);

    /** Send command to set end erase address (CMD36 for eMMC, CMD33 for SD). */
    command_return = r_sdmmc_command_send(p_ctrl, end_command, end_address);
    SDMMC_ERROR_RETURN(command_return, SSP_ERR_ERASE_FAILED);

    /** Send erase command (CMD38). */
    command_return = r_sdmmc_command_send(p_ctrl, SDMMC_CMD_ERASE, argument);
    if (!command_return)
    {
        /* Wait at least 3s + 250 ms * sector_count for erase to complete. */
        uint64_t timeout = (SDMMC_ERASE_TIMEOUT_PER_SECTOR_US * ((uint64_t) sector_count)) + 3000000U;

        /* The event status is updated in the access interrupt.  Use a local copy of the event status to make sure
         * it isn't updated during the loop. */
        volatile sdhi_event_t event;
        event.word = p_ctrl->sdhi_event.word;
        while (timeout > 0U)
        {
            /* Return an error if a hardware error occurred. */
            SDMMC_ERROR_RETURN(!event.bit.event_error, SSP_ERR_ERASE_FAILED);
            if (event.bit.response_end)
            {
                /* If the response end bit is set, the erase is complete. */
                return SSP_SUCCESS;
            }
            R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MICROSECONDS);
            timeout--;

            /* Check for updates to the event status. */
            event.word = p_ctrl->sdhi_event.word;
        }

        ret_val = SSP_ERR_ERASE_FAILED;
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @} (end addtogroup SDMMC)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
#if SDMMC_CFG_PARAM_CHECKING_ENABLE
/*******************************************************************************************************************//**
 * Parameter checking for the open function.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 * @param[in]     p_cfg     Pointer to SDMMC instance configuration structure.
 *
 * @retval SSP_SUCCESS          Parameters to open() are in the valid range.
 * @retval SSP_ERR_ASSERTION    A required input pointer is NULL, or the block size is 0 or > 512 bytes.
 **********************************************************************************************************************/
static ssp_err_t r_sdmmc_open_param_check(sdmmc_instance_ctrl_t * p_ctrl, sdmmc_cfg_t const * const p_cfg)
{

    /* Check pointers for NULL values */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_cfg->p_lower_lvl_transfer);

    if (SDMMC_BUS_WIDTH_8_BITS == p_cfg->hw.bus_width)
    {
        /** If 8-bit width is selected, make sure the MCU supports 8-bit MMC. */
        bsp_feature_sdhi_t sdhi_feature = {0U};
        R_BSP_FeatureSdhiGet(&sdhi_feature);
        SSP_ASSERT(1U == sdhi_feature.supports_8_bit_mmc);
    }

    /* Check block size, 512 bytes is the maximum block size the peripheral supports */
    if (NULL != p_cfg->p_extend)
    {
        uint32_t block_size = ((sdmmc_extended_cfg_t *) p_cfg->p_extend)->block_size;
        SSP_ASSERT(0U != block_size);
        SSP_ASSERT(block_size <= SDMMC_MAX_BLOCK_SIZE);
    }

    return SSP_SUCCESS;
}
#endif

/*******************************************************************************************************************//**
 * Parameter checking for erase.
 *
 * @param[in]     p_ctrl        Pointer to an open SD/MMC instance control block.
 * @param[in]     start_sector  First sector to erase. Must be a multiple of sdmmc_info_t::erase_sector_count.
 * @param[in]     sector_count  Number of sectors to erase. Must be a multiple of sdmmc_info_t::erase_sector_count.
 *                              All sectors must be in the range of sdmmc_info_t::sector_count.
 *
 * @retval  SSP_SUCCESS                  Erase operation requested.
 * @retval  SSP_ERR_ASSERTION            A required pointer is NULL.
 * @retval  SSP_ERR_NOT_OPEN             Driver has not been initialized.
 * @retval  SSP_ERR_CARD_NOT_READY       Card was unplugged.
 * @retval  SSP_ERR_TRANSFER_BUSY        Driver is busy with a previous operation.
 * @retval  SSP_ERR_WRITE_PROTECTED      SD card is Write Protected.
 * @retval  SSP_ERR_ERASE_FAILED         Erase operation unsuccessful.
 **********************************************************************************************************************/
static ssp_err_t r_sdmmc_erase_error_check (sdmmc_instance_ctrl_t * const p_ctrl, uint32_t const start_sector, uint32_t const sector_count)
{
    ssp_err_t ret_val = r_sdmmc_common_error_check(p_ctrl, true);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == ret_val, ret_val);

    /* Check for valid sector count.  Must be a non-zero multiple of erase_sector_count. */
    SDMMC_ERROR_RETURN(0U != sector_count, SSP_ERR_ERASE_FAILED);
    SDMMC_ERROR_RETURN(0U == (sector_count % p_ctrl->status.erase_sector_count), SSP_ERR_ERASE_FAILED);

    /* Check for valid start sector.  Must be a multiple of erase_sector_count. */
    SDMMC_ERROR_RETURN(0U == (start_sector % p_ctrl->status.erase_sector_count), SSP_ERR_ERASE_FAILED);

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Parameter checking for runtime APIs.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 * @param[in]     check_wp  true = error if card is write protected, false = don't care if card is write protected
 *
 * @retval  SSP_SUCCESS                  Device is ready to be accessed.
 * @retval  SSP_ERR_ASSERTION            A required pointer is NULL.
 * @retval  SSP_ERR_NOT_OPEN             Driver has not been initialized.
 * @retval  SSP_ERR_CARD_NOT_READY       Card was unplugged.
 * @retval  SSP_ERR_TRANSFER_BUSY        Driver is busy with a previous operation.
 * @retval  SSP_ERR_WRITE_PROTECTED      SD card is Write Protected.
 **********************************************************************************************************************/
static ssp_err_t r_sdmmc_common_error_check(sdmmc_instance_ctrl_t * const p_ctrl, bool check_wp)
{
#if SDMMC_CFG_PARAM_CHECKING_ENABLE
    /* Check pointers for NULL values */
    SSP_ASSERT(NULL != p_ctrl);

    SDMMC_ERROR_RETURN(SDMMC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    SDMMC_ERROR_RETURN(!p_ctrl->transfer_in_progress, SSP_ERR_TRANSFER_BUSY);
    SDMMC_ERROR_RETURN(HW_SDMMC_ClockDivEnableGet(p_ctrl->p_reg), SSP_ERR_TRANSFER_BUSY);

    /* Has card been removed since initialized? */
    r_sdmmc_card_inserted_get(p_ctrl);
    SDMMC_ERROR_RETURN(p_ctrl->status.ready, SSP_ERR_CARD_NOT_READY);

    if (check_wp && (p_ctrl->write_protect != SDMMC_WRITE_PROTECT_NONE))
    {
        /* Check for write protection */
        SDMMC_ERROR_RETURN(!p_ctrl->status.write_protected, SSP_ERR_CARD_WRITE_PROTECTED);
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Enables an interrupt.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 * @param[in]     irq       Interrupt to configure.
 * @param[in]     ipl       Interrupt priority to set.
 **********************************************************************************************************************/
static void r_sdmmc_irq_cfg(sdmmc_instance_ctrl_t * p_ctrl, IRQn_Type irq, uint8_t ipl)
{
    ssp_vector_info_t * p_vector_info;
    if (SSP_INVALID_VECTOR != irq)
    {
        /** Stores control block in the vector information array so it can be accessed from the interrupt. */
        R_SSP_VectorInfoGet(irq, &p_vector_info);

        /** Sets interrupt priority and interrupts in the NVIC. */
        NVIC_SetPriority(irq, ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;
        R_BSP_IrqStatusClear(irq);
        NVIC_ClearPendingIRQ(irq);
        NVIC_EnableIRQ(irq);
    }
}

/*******************************************************************************************************************//**
 * Disables an interrupt.
 *
 * @param[in]     irq    Interrupt to disable.
 **********************************************************************************************************************/
static void r_sdmmc_irq_disable(IRQn_Type irq)
{
    ssp_vector_info_t * p_vector_info;
    if (SSP_INVALID_VECTOR != irq)
    {
        /** Disables interrupts in the NVIC. */
        NVIC_DisableIRQ(irq);

        /** Clears the control block from the vector information array. */
        R_SSP_VectorInfoGet(irq, &p_vector_info);
        *(p_vector_info->pp_ctrl) = NULL;
    }
}

/*******************************************************************************************************************//**
 * Stores access interrupt flags in the control block and calls the callback.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 * @param[in]     p_args    Pointer to SDMMC callback arguments.
 **********************************************************************************************************************/
static void r_sdmmc_access_irq_process (sdmmc_instance_ctrl_t * p_ctrl, sdmmc_callback_args_t * p_args)
{
    uint32_t       info1;
    uint32_t       info2;
    sdhi_event_t   flags = {0U};
    flags.word = p_ctrl->sdhi_event.word;

    /* Clear stop register after access end. */
    HW_SDMMC_DataStop(p_ctrl->p_reg, 0U);

    /* Read interrupt flag registers. */
    info1        = HW_SDMMC_Info1Get(p_ctrl->p_reg);
    info2        = HW_SDMMC_Info2Get(p_ctrl->p_reg);

    info1       &= SDMMC_SDHI_INFO1_ACCESS_MASK;
    info2       &= SDMMC_SDHI_INFO2_MASK;

    /* Combine all flags in one 32 bit word. */
    flags.word |= (info1 | (info2 << 16));

    /* Combine only errors in one bit. */
    if (flags.word & SDMMC_ACCESS_ERROR_MASK)
    {
        flags.bit.event_error = 1U;
    }

    p_ctrl->sdhi_event.word |= flags.word;

    if (flags.bit.response_end)
    {
        /* Enable the access interrupt. */
        HW_SDMMC_AccessEndEnable(p_ctrl->p_reg);
    }

    /* Clear interrupt flags */
    HW_SDMMC_Info1Set(p_ctrl->p_reg, info1);
    HW_SDMMC_Info2Set(p_ctrl->p_reg, info2);

    if (p_ctrl->transfer_in_progress)
    {
        p_args->event = SDMMC_EVENT_NONE;

        /* Check for errors */
        if (p_ctrl->sdhi_event.bit.event_error)
        {
            p_args->event = SDMMC_EVENT_TRANSFER_ERROR;
            HW_SDMMC_DataStop(p_ctrl->p_reg, 1U);
            r_sdmmc_transfer_end(p_ctrl);
        }
        else
        {
            /* Check for access end */
            if (p_ctrl->sdhi_event.bit.access_end)
            {
                /* Read transfer end and callback performed in transfer interrupt. */
                if (SDMMC_TRANSFER_DIR_READ != p_ctrl->transfer_dir)
                {
                    r_sdmmc_transfer_end(p_ctrl);
                    p_args->event = SDMMC_EVENT_TRANSFER_COMPLETE;
                }
            }
        }
    }
}

/*******************************************************************************************************************//**
 * Send a command to the SD, eMMC, or SDIO device.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 * @param[in]     command   Command to send.
 * @param[in]     argument  Argument to send with the command.
 *
 * @retval true             Command sent and response received.
 * @retval false            Command send failed.
 **********************************************************************************************************************/
static bool r_sdmmc_command_send (sdmmc_instance_ctrl_t * p_ctrl, uint16_t command, uint32_t argument)
{
    /** Clear Status */
    HW_SDMMC_StatusClear(p_ctrl->p_reg);
    p_ctrl->sdhi_event.word = 0U;

    /** Enable response end interrupt. */
    HW_SDMMC_ResponseEndEnable(p_ctrl->p_reg);
    HW_SDMMC_InterruptMaskInfo2Set(p_ctrl->p_reg, SDMMC_SDHI_INFO2_MASK_CMD_SEND);

    /** Enable Clock */
    HW_SDMMC_ClockEnable(p_ctrl->p_reg);

    /** Write argument, then command to the SDHI peripheral. */
    HW_SDMMC_SetArguments(p_ctrl->p_reg, argument);
    HW_SDMMC_CommandSend(p_ctrl->p_reg, command);

    /** Wait for end of response, error or timeout */
    uint32_t timeout = SDMMC_RESPONSE_TIMEOUT_US;

    /* The event status is updated in the access interrupt.  Use a local copy of the event status to make sure
     * it isn't updated during the loop. */
    volatile sdhi_event_t event;
    event.word = p_ctrl->sdhi_event.word;
    while (timeout > 0U)
    {
        /* Return an error if a hardware error occurred. */
        if (event.bit.event_error)
        {
            return false;
        }
        else if (event.bit.response_end)
        {
            /* If the response end bit is set, the command response was received with no error. */
            return true;
        }
        R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MICROSECONDS);
        timeout--;

        /* Check for updates to the event status. */
        event.word = p_ctrl->sdhi_event.word;
    }

    /* Timeout. */
    return false;
}

/*******************************************************************************************************************//**
 * Set the SD clock to a rate less than or equal to the requested maximum rate.
 *
 * @param[in]     p_ctrl   Pointer to SDMMC instance control block.
 * @param[in]     max_rate Maximum SD clock rate to set
 *
 * @retval true   SD clock rate is set to a value less than or equal to the requested maximum rate.
 * @retval false  Operation failed.
 **********************************************************************************************************************/
static bool r_sdmmc_max_clock_rate_set (sdmmc_instance_ctrl_t * p_ctrl, uint32_t max_rate)
{
    ssp_err_t err;
    uint32_t  frequency = 0U;
    uint32_t   setting = SDMMC_CLK_CTRL_DIV_INVALID;

    /** Get the runtime frequency of PCLKA, the source of the SD clock */
    err      = g_cgc_on_cgc.systemClockFreqGet(CGC_SYSTEM_CLOCKS_PCLKA, &frequency);
    if (err != SSP_SUCCESS)
    {
        return false;
    }

    /** Iterate over all possible divisors, starting with the smallest, until the resulting clock rate is less than
     * or equal to the requested maximum rate. */
    uint8_t divisor_shift;
    for (divisor_shift = SDMMC_MIN_CLOCK_DIVISION_SHIFT; divisor_shift <= SDMMC_MAX_CLOCK_DIVISION_SHIFT; divisor_shift++)
    {
        if ((frequency >> divisor_shift) <= max_rate)
        {
            /* If the calculated frequency is less than or equal to the maximum supported by the device,
             * select this frequency. */
            setting = 1U << divisor_shift;
            /* The register setting is the divisor value divided by 4. */
            setting >>= 2U;

            /* Set the clock setting. */
            if (r_sdmmc_clock_div_set(p_ctrl, (uint8_t) setting))
            {
                /* Store the updated value. */
                p_ctrl->status.clock_rate = frequency >> divisor_shift;
                return true;
            }

            /* Valid setting already found, stop looking. */
            break;
        }
    }

    return false;
}

/*******************************************************************************************************************//**
 * Set the clock rate with the given divisor
 * 
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 * @param[in]     divisor   Divider value to set
 *
 * @retval true   SD clock divisor set to requested value.
 * @retval false  Operation failed.
 **********************************************************************************************************************/
static bool r_sdmmc_clock_div_set (sdmmc_instance_ctrl_t * p_ctrl, uint8_t divisor)
{
    if (!r_sdmmc_clock_div_enable_get(p_ctrl))
    {
        return false;               /* clock divider enable timeout */
    }

    HW_SDMMC_ClockRateSet(p_ctrl->p_reg, divisor);
    return true;
}

/*******************************************************************************************************************//**
 * Get the clock divider enable state.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 *
 * @retval false  Timeout, write to clock register not permitted.
 * @retval true   Write to clock register permitted.
 **********************************************************************************************************************/
static bool r_sdmmc_clock_div_enable_get (sdmmc_instance_ctrl_t * p_ctrl)
{
    /* The clock register is accessible 8 SD clock counts after the last command completes.  Each register access
     * requires at least one PCLKA count, so check the register at least 8 times the PCLKA divisor value. */
    /* PCLKA divisor setting is 0 for PCLKA / 2, and divisor / 4 otherwise. */
    uint16_t divisor = (uint16_t) ((uint16_t) HW_SDMMC_ClockDivGet(p_ctrl->p_reg) << 2);
    if (0U == divisor)
    {
        divisor = 2U;
    }
    uint32_t timeout = SDMMC_SDHI_CLKEN_TIMEOUT_SD_CLOCK * divisor;

    while (timeout > 0U)
    {
        /* Can't write to clock control register until this bit is set. */
        if (HW_SDMMC_ClockDivEnableGet(p_ctrl->p_reg))
        {
            return true;
        }

        timeout--;
    }

    return false;
}

/*******************************************************************************************************************//**
 * Initializes SD host interface hardware, including power supply ramp up time prior to card detection.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 * @param[in]     p_extend  Pointer to SDMMC extended configuration.
 *
 * @retval SSP_SUCCESS                Operation completed successfully.
 * @retval SSP_ERR_CARD_INIT_FAILED   Error in clock configuration.
 * @retval SSP_ERR_INVALID_ARGUMENT   Card detection not supported on MCU or embedded devices.
 * @retval SSP_ERR_CARD_NOT_INSERTED  Card detection is enabled and no card is plugged in
 **********************************************************************************************************************/
static ssp_err_t r_sdmmc_sdhi_cfg (sdmmc_instance_ctrl_t * const p_ctrl, sdmmc_extended_cfg_t const * const p_extend)
{
    /** Initialize SD Host Interface Hardware */
    /* Set card detect counter to minimum and busy timeout counter to maximum */
    HW_SDMMC_ResetAssert(p_ctrl->p_reg);
    HW_SDMMC_ResetRelease(p_ctrl->p_reg);
    HW_SDMMC_RegisterDefaultSet (p_ctrl->p_reg);          /* Set normally untouched registers to default */
    HW_SDMMC_CardDetectTimeCounterSet(p_ctrl->p_reg, SDMMC_SDHI_CD_TIMEOUT_2_10);
    HW_SDMMC_TimeoutCounterSet(p_ctrl->p_reg, SDMMC_SDHI_TIMEOUT_2_27);
    HW_SDMMC_InterruptMaskInfo1Set(p_ctrl->p_reg, SDMMC_CARD_DETECT_NONE);

    /** Set the clock frequency to 400 kHz or less for identification. */
    /* Clock setting enable bit does not need to be checked here because no commands have been sent yet. */
    if (!r_sdmmc_max_clock_rate_set(p_ctrl, SDMMC_INIT_MAX_CLOCK_RATE_HZ))
    {
        return SSP_ERR_CARD_INIT_FAILED;
    }
    HW_SDMMC_ClockEnable(p_ctrl->p_reg);

    /** Set initial bus width to one bit wide. */
    HW_SDMMC_BusWidthSet(p_ctrl->p_reg, (uint8_t)1);

    /** From the SD specification: "The maximum ramp up time should be 35ms for 2.7-3.6V power supply. Host shall wait
     * until VDD is stable. After 1ms VDD stable time, host provides at least 74 clocks before issuing the first
     * command." At this point PCLKA is running at at 200-400 kHz, and 74 clocks at 200 kHz take less than 1ms,
     * so this wait is 37 ms = 35 ms for max ramp up + 1 ms at stable VDD + 1 ms for 74 clocks. */
    R_BSP_SoftwareDelay(SDMMC_STARTUP_DELAY_MS, BSP_DELAY_UNITS_MILLISECONDS);

    /** consider write protect is Not Used. */
    p_ctrl->write_protect = false;

    /** If card detection is enabled in the configuration, enable card detection and check to make sure a card is
     * inserted. */
    if (NULL != p_extend)
    {
        if (SDMMC_CARD_DETECT_NONE != p_extend->card_detect)
        {
            bsp_feature_sdhi_t sdhi_feature = {0U};
            R_BSP_FeatureSdhiGet(&sdhi_feature);

            SDMMC_ERROR_RETURN(1U == sdhi_feature.has_card_detection, SSP_ERR_INVALID_ARGUMENT);
            SDMMC_ERROR_RETURN(SDMMC_MEDIA_TYPE_CARD == p_ctrl->hw.media_type, SSP_ERR_INVALID_ARGUMENT);

            HW_SDMMC_InterruptMaskInfo1Set(p_ctrl->p_reg, p_extend->card_detect);

            /* Return an error if card is not plugged in and card detection is enabled in the configuration. */
            SDMMC_ERROR_RETURN(HW_SDMMC_CardInsertedGet(p_ctrl->p_reg), SSP_ERR_CARD_NOT_INSERTED);
        }

        /** Updating the user configured value to SDMMC instance control block. */
        p_ctrl->write_protect = p_extend->write_protect;

    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Initializes driver and device.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 * @param[in]     p_extend  Pointer to SDMMC extended configuration.
 *
 * @retval SSP_SUCCESS                Operation completed successfully.
 * @retval SSP_ERR_CARD_INIT_FAILED   Media failed to initialize.
 * @retval SSP_ERR_INVALID_ARGUMENT   Card detection configured but not supported.
 * @retval SSP_ERR_CARD_NOT_INSERTED  Card detection is enabled and no card is plugged in
 **********************************************************************************************************************/
static ssp_err_t r_sdmmc_card_identify (sdmmc_instance_ctrl_t * const p_ctrl, sdmmc_extended_cfg_t const * const p_extend)
{
    /** Configure SDHI peripheral. */
    ssp_err_t err = r_sdmmc_sdhi_cfg(p_ctrl, p_extend);
    SDMMC_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Reset I/O: In order to reset an I/O only card or the I/O portion of a combo card, use
    CMD52 to write a 1 to the RES bit in the CCCR (bit 3 of register 6). */
    uint8_t data = 1U << 3;
    r_sdmmc_cmd52(p_ctrl, &data, 0U, 6U, SDMMC_IO_WRITE_READ_AFTER_WRITE, SDMMC_SDIO_CMD52_WRITE);

    /** Put the card in idle state. */
    r_sdmmc_command_send(p_ctrl, SDMMC_CMD_GO_IDLE_STATE, 0);

    /** See if the device is SDIO, SD, or eMMC.*/
    /** Order matters - Check if the card has SDIO capabilities first (CMD5). */
    if (r_sdmmc_sdio_check(p_ctrl))
    {
        p_ctrl->status.card_type = SDMMC_CARD_TYPE_SD;
    }
    else
    {
        /* SD and eMMC cards only support block size of 512 bytes on the SDHI hardware. */
        SDMMC_ERROR_RETURN(SDMMC_MAX_BLOCK_SIZE == (uint32_t) p_ctrl->status.sector_size, SSP_ERR_INVALID_ARGUMENT);

        /** If the device is not SDIO, issue CMD8 to enable ACMD41 for SD memory devices. */
        r_sdmmc_command_send(p_ctrl, (uint16_t)SDMMC_CMD_IF_COND, ((SDMMC_IF_COND_VOLTAGE << 8) | SDMMC_IF_COND_CHECK_PATTERN));

        /** If the device is not SDIO, check to see if it is an SD memory card (ACMD41).
         * NOTE: Not supporting memory on SDIO combo cards. */
        if (r_sdmmc_sd_card_check(p_ctrl))
        {
            p_ctrl->status.card_type = SDMMC_CARD_TYPE_SD;
        }
        else
        {
            /** If the device is not SDIO or SD memory, check to see if it is an eMMC device (CMD1). */
            if (r_sdmmc_emmc_check(p_ctrl))
            {
                p_ctrl->status.card_type = SDMMC_CARD_TYPE_MMC;
            }
            else
            {
                /* If the device is not identified as SDIO, SD memory card, or eMMC, return an error. */
                return SSP_ERR_CARD_INIT_FAILED;
            }
        }

        /** Enter identification state (CMD2). */
        if (!(r_sdmmc_command_send(p_ctrl, SDMMC_CMD_ALL_SEND_CID, 0))) /* send SD CMD2 */
        {
            return SSP_ERR_CARD_INIT_FAILED;
        }
    }

    /** Get relative card address (CMD3). */
    if (!r_sdmmc_rca_get(p_ctrl))
    {
        return SSP_ERR_CARD_INIT_FAILED;
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Initializes bus clock, block length, and bus width.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 *
 * @retval SSP_SUCCESS                Operation completed successfully.
 * @retval SSP_ERR_CARD_INIT_FAILED   Operation failed.
 **********************************************************************************************************************/
static ssp_err_t r_sdmmc_bus_cfg (sdmmc_instance_ctrl_t * const p_ctrl)
{
    /** Set clock rate to highest supported by both host and device.  Move card to transfer state during this
     * process. */
    if (p_ctrl->status.sdio == true)
    {
        /* Switch to data transfer mode (CMD7). */
        if (!(r_sdmmc_command_send(p_ctrl, SDMMC_CMD_SEL_DES_CARD, p_ctrl->status.sdhi_rca << 16)))
        {
            return SSP_ERR_CARD_INIT_FAILED;
        }

        /* Set the clock speed to the highest . */
        if (!r_sdmmc_sdio_clock_optimize(p_ctrl))
        {
            return SSP_ERR_CARD_INIT_FAILED;
        }

        /* Set bus width.
         * Note: Low speed SDIO not supported, so no need to check if 4-bit mode is supported. */
        uint8_t bus_width_setting = (uint8_t) p_ctrl->hw.bus_width;
        bus_width_setting = ((uint8_t) (bus_width_setting >> 1) & 0x03U);
        ssp_err_t ret_val = r_sdmmc_cmd52(p_ctrl, &bus_width_setting, 0U, 0x07U, SDMMC_IO_WRITE_READ_AFTER_WRITE,
                                          SDMMC_SDIO_CMD52_WRITE);
        if (SSP_SUCCESS == ret_val)
        {
            HW_SDMMC_BusWidthSet(p_ctrl->p_reg, p_ctrl->hw.bus_width);
        }
        else
        {
            return SSP_ERR_CARD_INIT_FAILED;
        }

        /* Enable SDIO interrupts. Busy flag must be cleared before enabling interrupts. */
        HW_SDMMC_SdioIrqEnable(p_ctrl->p_reg, true);
    }
    else
    {
        /* Decode CSD register depending on version of card */
        sdmmc_priv_csd_reg_t csd_reg;
        if (!r_sdmmc_csd_save(p_ctrl, &csd_reg))
        {
            return SSP_ERR_CARD_INIT_FAILED;
        }

        /* Switch to data transfer mode (CMD7). */
        if (!(r_sdmmc_command_send(p_ctrl, SDMMC_CMD_SEL_DES_CARD, p_ctrl->status.sdhi_rca << 16)))
        {
            return SSP_ERR_CARD_INIT_FAILED;
        }

        /* Set clock to highest supported frequency. */
        if (!r_sdmmc_clock_optimize(p_ctrl, &csd_reg))
        {
            return SSP_ERR_CARD_INIT_FAILED;
        }

        /* Set the block length (CMD16) to 512 bytes. */
        if (!(r_sdmmc_command_send(p_ctrl, SDMMC_CMD_SET_BLOCKLEN, p_ctrl->status.sector_size)))
        {
            return SSP_ERR_CARD_INIT_FAILED;
        }

        /* Set bus width. */
        if (!r_sdmmc_bus_width_set(p_ctrl))
        {
            return SSP_ERR_CARD_INIT_FAILED;
        }
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Read or write data.
 *
 * @param[in]     p_ctrl          Pointer to an open SD/MMC instance control block.
 * @param[in]     block_count     Number of blocks/sectors to write/read.
 * @param[in]     block_size      Sector/Block size.
 * @param[in]     command         Command to send.
 * @param[in]     argument        Argument to send with the command.
 *
 * @retval SSP_SUCCESS            Response received with no error.
 * @retval SSP_ERR_INTERNAL       Operation failed.
 **********************************************************************************************************************/
static ssp_err_t r_sdmmc_read_write_common(sdmmc_instance_ctrl_t * const p_ctrl,
                                    uint32_t   block_count,
                                    uint16_t   block_size,
                                    uint16_t   command,
                                    uint32_t   argument)
{
    ssp_err_t ret_val = SSP_ERR_INTERNAL;

    p_ctrl->transfer_in_progress = true;

    /** Set the block count. */
    if (block_count > 1U)
    {
        HW_SDMMC_BlockCountEnable(p_ctrl->p_reg, 1U);
        HW_SDMMC_BlockCountSet(p_ctrl->p_reg, block_count);
    }
    else
    {
        HW_SDMMC_BlockCountEnable(p_ctrl->p_reg, 0U);
    }

    /** Set block size */
    HW_SDMMC_BlockSizeSet(p_ctrl->p_reg, block_size);

    /** Send command. */
    if (r_sdmmc_command_send(p_ctrl, command, argument))
    {
        /** Check the R1 response. */
        sdmmc_priv_card_status_t response = {0U};
        if (block_count > 1U)
        {
            /* Get the R1 response for multiple block read and write from SD_RSP54 since the response in SD_RSP10 may
             * have been overwritten by the response to CMD12. */
            HW_SDMMC_Response54Get(p_ctrl->p_reg, &response);
        }
        else
        {
            HW_SDMMC_ResponseGet(p_ctrl->p_reg, &response);
        }

        /* Verify no error bits are set in the response. */
        uint32_t error = (SDMMC_R1_ERROR_BITS & response.status);
        if (0U == error)
        {
            ret_val = SSP_SUCCESS;
        }
    }
    if (SSP_SUCCESS != ret_val)
    {
        /* If there was an error, stop the transfer. */
        HW_SDMMC_DataStop(p_ctrl->p_reg, 1U);
        r_sdmmc_transfer_end(p_ctrl);
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * Update ready status in the control block if card detection is enabled.
 *
 * @param[in,out] p_ctrl    Pointer to SDMMC instance control block.
 **********************************************************************************************************************/
static void r_sdmmc_card_inserted_get (sdmmc_instance_ctrl_t * const p_ctrl)
{
    /** Checks if card detection is enabled. If not, does nothing. */
    uint32_t info1_mask = HW_SDMMC_InterruptMaskInfo1Get(p_ctrl->p_reg);
    if (0U != ((uint32_t) (~info1_mask) & SDMMC_SDHI_INFO1_CARD_MASK))
    {
        if (p_ctrl->status.ready)
        {
            p_ctrl->status.ready = (bool) HW_SDMMC_CardInsertedGet(p_ctrl->p_reg);
        }
    }
}

/*******************************************************************************************************************//**
 *   Command 52 Write.
 *
 * @param[in]     p_ctrl            Pointer to SDMMC instance control block.
 * @param[in]     p_data            Pointer to argument data and stores response of card.
 * @param[in]     function          SDIO Function Number.
 * @param[in]     address           SDIO register address.
 * @param[in]     read_after_write  Whether or not to read back the same register after writing.
 * @param[in]     command           Command to send.
 *
 *   @retval SSP_SUCCESS            Operation completed successfully.
 *   @retval SSP_ERR_INTERNAL       Operation failed.
 **********************************************************************************************************************/
static ssp_err_t r_sdmmc_cmd52 (sdmmc_instance_ctrl_t * const        p_ctrl,
                            uint8_t  * const            p_data,
                            uint32_t const              function,
                            uint32_t const              address,
                            sdmmc_io_write_mode_t const read_after_write,
                            uint32_t const              command)
{
    ssp_err_t  ret_val = SSP_SUCCESS;

    /* Send Write I/O command. */
    sdmmc_priv_sdio_arg_t argument = {0U};
    argument.cmd_52_arg.function_number  = (function & SDMMC_SDIO_CMD52_CMD53_FUNCTION_MASK);
    argument.cmd_52_arg.rw_flag          = (command & 1U);
    argument.cmd_52_arg.raw              = read_after_write;
    argument.cmd_52_arg.register_address = (address & SDMMC_SDIO_CMD52_CMD53_ADDRESS_MASK);
    argument.cmd_52_arg.data             = *p_data;

    /** Send CMD52. */
    if (!(r_sdmmc_command_send(p_ctrl, SDMMC_CMD_IO_RW_DIRECT, argument.arg)))
    {
        /* Command send failed.  Return an error. */
        ret_val = SSP_ERR_INTERNAL;
    }
    else
    {
        /* Store data read from the response. */
        sdmmc_priv_card_status_t response = {0U};
        HW_SDMMC_ResponseGet(p_ctrl->p_reg, &response);
        *p_data = (uint8_t) response.r5.read_write_data;
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * Check to see if the device is an SDIO card.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 *
 * @retval  true          Device is an SDIO card.
 * @retval  false         Not identified as SDIO card or failed to complete initialization in 1 second.
 **********************************************************************************************************************/
static bool r_sdmmc_sdio_check (sdmmc_instance_ctrl_t * const p_ctrl)
{
    sdmmc_priv_card_status_t response = {0U};
    uint32_t      ocr      = SDMMC_OCR_VDD_SUPPORTED;

    /** Check for SDIO capabilities (CMD5). */
    p_ctrl->status.sdio = false;
    if (r_sdmmc_command_send(p_ctrl, SDMMC_CMD_SDIO, 0x00))
    {
        /** Check response of CMD5 (R4). */
        HW_SDMMC_ResponseGet(p_ctrl->p_reg, &response);
        if (response.r4.io_functions)
        {
            /** If the card supports SDIO, check for the card to be ready for at least one second. */
            /* To ensure the 1 second timeout, consider that there are 48 bits in a command, 48 bits
             * in a response, and 8 clock cycles minimum between commands, so there are 104 clocks minimum,
             * and the maximum clock rate at this point is 400 kHz, so issue the command 400000 / 104
             * times to ensure a timeout of at least 1 second. */
            for (uint32_t i = 0U; i < SDMMC_INIT_ONE_SECOND_TIMEOUT_ITERATIONS; i++)
            {
                if (!(r_sdmmc_command_send(p_ctrl, SDMMC_CMD_SDIO, ocr)))
                {
                    return false;
                }

                /* Get response of CMD5 (R4) */
                HW_SDMMC_ResponseGet(p_ctrl->p_reg, &response);
                if (response.r4.ready)
                {
                    /* SDIO card is ready. */
                    p_ctrl->status.sdio = true;
                    return true;
                }
            }
        }
    }

    return false;
}

/*******************************************************************************************************************//**
 * Checks to see if the device is an SD card.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 *
 * @retval  true          Device is an SD card.
 * @retval  false         Not identified as SD card or failed to complete initialization in 1 second.
 **********************************************************************************************************************/
static bool r_sdmmc_sd_card_check (sdmmc_instance_ctrl_t * const p_ctrl)
{
    uint32_t      ocr;
    sdmmc_priv_card_status_t response = {0U};

    /** Tries to send ACMD41 for up to 1 second as long as the card is responding and initialization is not complete.
     * Returns immediately if the card fails to respond to ACMD41. */
    /* To ensure the 1 second timeout, consider that there are 48 bits in a command, 48 bits
     * in a response, and 8 clock cycles minimum between commands, so there are 104 clocks minimum,
     * and the maximum clock rate at this point is 400 kHz, so issue the command 400000 / 104
     * times to ensure a timeout of at least 1 second. */
    for (uint32_t i = 0U; i < SDMMC_INIT_ONE_SECOND_TIMEOUT_ITERATIONS; i++)
    {
        /*/Send App Command - CMD55 */
        if (!(r_sdmmc_command_send(p_ctrl, SDMMC_CMD_APP_CMD, 0U)))
        {
            return false;
        }

        ocr  = SDMMC_OCR_VDD_SUPPORTED | SDMMC_OCR_CAPACITY_HC;

        /* ACMD41 */
        if (r_sdmmc_command_send(p_ctrl, SDMMC_CMD_C_ACMD | SDMMC_CMD_SD_SEND_OP_COND, ocr))
        {
            /* get response of ACMD41 (R3) */
            HW_SDMMC_ResponseGet(p_ctrl->p_reg, &response);

            /*  Initialization complete? */
            if (response.r3.power_up_status)
            {
                /* High capacity card ? */
                /*  0 = SDSC, 1 = SDHC or SDXC */
                p_ctrl->status.hc = (response.r3.card_capacity_status > 0U);
                return true;
            }
        }
        else
        {
            return false;
        }
    }

    return false;
}

/*******************************************************************************************************************//**
 * Checks to see if the device is an eMMC.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 *
 * @retval  true          Device is an eMMC device.
 * @retval  false         Not identified as an eMMC device or failed to complete initialization in 1 second.
 **********************************************************************************************************************/
static bool r_sdmmc_emmc_check (sdmmc_instance_ctrl_t * const p_ctrl)
{
    uint32_t      ocr;
    sdmmc_priv_card_status_t response = {0U};
    uint32_t      capacity;
    capacity = SDMMC_OCR_CAPACITY_HC;                      /* SDHC cards supported */

    /** Tries to send CMD1 for up to 1 second as long as the device is responding and initialization is not complete.
     * Returns immediately if the device fails to respond to CMD1. */
    /* To ensure the 1 second timeout, consider that there are 48 bits in a command, 48 bits
     * in a response, and 8 clock cycles minimum between commands, so there are 104 clocks minimum,
     * and the maximum clock rate at this point is 400 kHz, so issue the command 400000 / 104
     * times to ensure a timeout of at least 1 second. */
    for (uint32_t i = 0U; i < SDMMC_INIT_ONE_SECOND_TIMEOUT_ITERATIONS; i++)
    {
        /*  Format and send cmd: Volt. window is usually 0x00300000 (3.4-3.2v) */
        ocr  = SDMMC_OCR_VDD_SUPPORTED;
        ocr |= capacity;

        if (!(r_sdmmc_command_send(p_ctrl, SDMMC_CMD_SEND_OP_COND, ocr)))
        {
            return false; /* SD cards will not respond to CMD1  */
        }

        /* get response of CMD1  */
        HW_SDMMC_ResponseGet(p_ctrl->p_reg, &response);

        /* Initialization complete? */
        if (response.r3.power_up_status)
        {
            p_ctrl->status.hc = (response.r3.card_capacity_status > 0U);
            return true;
        }
    }

    return false;
}

/*******************************************************************************************************************//**
 * Update write protection status in control block.
 *
 * @param[in]     p_ctrl      Pointer to SDMMC instance control block.
 **********************************************************************************************************************/
static void r_sdmmc_write_protect_get (sdmmc_instance_ctrl_t * const p_ctrl)
{
    /** Update write protection status in the control block if the device is a card, and if the user has enabled
     * the write protect pin */
    if ((SDMMC_MEDIA_TYPE_CARD ==  p_ctrl->hw.media_type) && (SDMMC_WRITE_PROTECT_WP == p_ctrl->write_protect))
    {
        p_ctrl->status.write_protected = (HW_SDMMC_WriteProtectGet(p_ctrl->p_reg) > 0U);
    }
}

/*******************************************************************************************************************//**
 * Wait for the busy line to go inactive on an eMMC device.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 *
 * @retval  true          eMMC device is no longer busy.
 * @retval  false         Operation timed out.
 **********************************************************************************************************************/
static bool r_sdmmc_emmc_wait_for_device (sdmmc_instance_ctrl_t * const p_ctrl)
{
    /* For eMMC, The device may signal busy after CMD6.  Wait for busy to clear. */
    uint32_t timeout = SDMMC_BUSY_TIMEOUT_US;
    while (!HW_SDMMC_ClockDivEnableGet(p_ctrl->p_reg))
    {
        if (0U == timeout)
        {
            return false;
        }
        R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MICROSECONDS);
        timeout--;
    }

    timeout = SDMMC_BUSY_TIMEOUT_US;
    while (HW_SDMMC_CardBusyGet(p_ctrl->p_reg))
    {
        if (0U == timeout)
        {
            return false;
        }
        R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MICROSECONDS);
        timeout--;
    }

    return true;
}

/*******************************************************************************************************************//**
 * Set SD clock rate to the maximum supported by both host and eMMC device.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 *
 * @retval  true          SD clock rate is set to the maximum supported by both host and eMMC device.
 * @retval  false         Operation failed.
 **********************************************************************************************************************/
static bool r_sdmmc_emmc_clock_optimize (sdmmc_instance_ctrl_t * const p_ctrl)
{
    /** Ask eMMC to switch to high speed using CMD6 if high speed is supported. */
    r_sdmmc_csd_extended_get(p_ctrl);
    p_ctrl->status.max_clock_rate = SDMMC_EMMC_DEFAULT_CLOCK_RATE;
    if ((SDMMC_EMMC_HIGH_SPEED_52_MHZ_BIT & p_ctrl->status.device_type) > 0U)
    {
        /* 52 MHz high speed supported, switch to this mode (CMD6). */
        if (!(r_sdmmc_command_send(p_ctrl, SDMMC_EMMC_CMD_SWITCH_WBUSY, SDMMC_EMMC_HIGH_SPEED_MODE)))
        {
            return false;
        }
        p_ctrl->status.max_clock_rate = SDMMC_EMMC_HIGH_SPEED_CLOCK_RATE; // Set clock rate to 52 MHz maximum
        p_ctrl->status.hs_timing      = 1U;

        /* The device may signal busy after CMD6.  Wait for busy to clear. */
        return r_sdmmc_emmc_wait_for_device(p_ctrl);
    }

    return true;
}

/*******************************************************************************************************************//**
 * Set SDHI clock to fastest allowable speed for card.
 *
 * @param[in]     p_ctrl     Pointer to SDMMC instance control block.
 * @param[in]     p_csd_reg  Pointer to card specific data.
 *
 * @retval  true          Set SD or MMC device in high speed mode and Sets SDHI clock to the maximum value
 *                        supported by both host and device.
 * @retval  false         Clock setting failed.
 **********************************************************************************************************************/
static bool r_sdmmc_clock_optimize (sdmmc_instance_ctrl_t * const p_ctrl, sdmmc_priv_csd_reg_t * const p_csd_reg)
{
    p_ctrl->status.max_clock_rate = SDMMC_SD_DEFAULT_CLOCK_RATE;
    if (SDMMC_CARD_TYPE_MMC == p_ctrl->status.card_type)
    {
        if (!r_sdmmc_emmc_clock_optimize(p_ctrl))
        {
            return false;
        }
    }
    else /* SD card */
    {
        /* Ask SD card to switch to high speed if CMD6 is supported.  CMD6 is supported if bit 10 of the CCC field
         * in the CSD is set.*/
        if (SDMMC_CSD_REG_CCC_CLASS_10_BIT == (SDMMC_CSD_REG_CCC_CLASS_10_BIT & p_csd_reg->csd_v1_b.ccc))
        {
            if (r_sdmmc_sd_high_speed(p_ctrl))
            {
                p_ctrl->status.max_clock_rate = SDMMC_SD_HIGH_SPEED_CLOCK_RATE;   // Set clock rate to 50 MHz maximum
            }
        }
    }

    if (!r_sdmmc_max_clock_rate_set(p_ctrl, p_ctrl->status.max_clock_rate))
    {
        return false;
    }

    return true;
}

/*******************************************************************************************************************//**
 * Checks to see if the SDIO card supports high speed.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 *
 * @retval  true          SDIO clock set to the maximum supported by both host and device.
 * @retval  false         Clock setting failed.
 **********************************************************************************************************************/
static bool r_sdmmc_sdio_clock_optimize (sdmmc_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t ret_val;
    uint8_t   data      = 0U;

    /** Issue CMD52 to set the high speed register. */
    data    = SDMMC_SDIO_REG_HIGH_SPEED_BIT_EHS;
    ret_val = r_sdmmc_cmd52(p_ctrl, &data, 0U, SDMMC_SDIO_REG_HIGH_SPEED, SDMMC_IO_WRITE_READ_AFTER_WRITE, SDMMC_SDIO_CMD52_WRITE);
    if (SSP_SUCCESS != ret_val)
    {
        return false;
    }

    /** Check to see if high speed mode was successfully enabled.  Both EHS and SHS bits must be set. */
    uint8_t high_speed_enabled_mask = SDMMC_SDIO_REG_HIGH_SPEED_BIT_EHS | SDMMC_SDIO_REG_HIGH_SPEED_BIT_SHS;
    p_ctrl->status.max_clock_rate = SDMMC_SD_DEFAULT_CLOCK_RATE;
    if (high_speed_enabled_mask == (data & high_speed_enabled_mask))
    {
        p_ctrl->status.max_clock_rate = SDMMC_SDIO_HIGH_SPEED_CLOCK_RATE;
    }

    /** Set the clock rate to the maximum supported by both host and device. */
    if (!r_sdmmc_max_clock_rate_set(p_ctrl, p_ctrl->status.max_clock_rate))
    {
        return false;
    }

    return true;
}

/*******************************************************************************************************************//**
 * Waits for the access end interrupt.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 *
 * @retval  true          Access end interrupt bit set.
 * @retval  false         Timeout.
 **********************************************************************************************************************/
static bool r_sdmmc_wait_for_access_end (sdmmc_instance_ctrl_t * const p_ctrl)
{
    uint32_t timeout = SDMMC_ACCESS_TIMEOUT_US;
    while (0U == p_ctrl->sdhi_event.bit.access_end)
    {
        if (0U == timeout)
        {
            return false;
        }
        R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MICROSECONDS);
        timeout--;
    }

    return true;
}

/*******************************************************************************************************************//**
 * Checks to see if the SD card supports high speed.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 *
 * @retval  true          SD clock set to the maximum supported by both host and device.
 * @retval  false         Clock setting failed.
 **********************************************************************************************************************/
static bool r_sdmmc_sd_high_speed (sdmmc_instance_ctrl_t * const p_ctrl)
{
    bool     ret_val;

    ret_val = false;

    /** Issue CMD6 to switch to high speed. */
    if (!(r_sdmmc_read_and_block(p_ctrl, SDMMC_CMD_SWITCH, SDMMC_SD_HIGH_SPEED_MODE_SWITCH, SDMMC_SD_SWITCH_STATUS_SIZE)))
    {
        return false;
    }

    /** Read the switch response to see if the high speed switch is supported and completed successfully. */
    uint32_t read_data;
    uint8_t * p_read_data_8 = (uint8_t *) &read_data;
    for (uint32_t i = 0U; (i < (SDMMC_SD_SWITCH_STATUS_SIZE / sizeof(uint32_t))); i++)
    {
        read_data = HW_SDMMC_DataGet(p_ctrl->p_reg);              /* read word from bus */

        if ((SDMMC_SD_SWITCH_HIGH_SPEED_ERROR_RESPONSE / sizeof(uint32_t)) == i)
        {
            /* Check for error response to High speed Function. */
            if (SDMMC_SD_SWITCH_HIGH_SPEED_ERROR == (p_read_data_8[SDMMC_SD_SWITCH_HIGH_SPEED_ERROR_RESPONSE % sizeof(uint32_t)] & SDMMC_SD_SWITCH_HIGH_SPEED_ERROR))
            {
                ret_val = false;
            }
        }
        if ((SDMMC_SD_SWITCH_HIGH_SPEED_RESPONSE / sizeof(uint32_t)) == i)
        {
            /* Check for successful switch. */
            if (SDMMC_SD_SWITCH_HIGH_SPEED_OK == (p_read_data_8[SDMMC_SD_SWITCH_HIGH_SPEED_RESPONSE % sizeof(uint32_t)] & SDMMC_SD_SWITCH_HIGH_SPEED_OK))
            {
                ret_val = true;
            }
        }
    }

    if (!r_sdmmc_wait_for_access_end(p_ctrl))
    {
        return false;
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * Save Card Specific Data.
 *
 * @param[in]     p_ctrl     Pointer to SDMMC instance control block.
 * @param[in]     p_csd_reg  Pointer to card specific data.
 *
 * @retval  true          Card specific data stored in control block.
 * @retval  false         Operation failed.
 **********************************************************************************************************************/
static bool r_sdmmc_csd_save (sdmmc_instance_ctrl_t * const p_ctrl, sdmmc_priv_csd_reg_t * const p_csd_reg)
{
    /** Send CMD9 to get CSD */
    if (!(r_sdmmc_command_send(p_ctrl, SDMMC_CMD_SEND_CSD, p_ctrl->status.sdhi_rca << 16)))
    {
        return false;
    }

    HW_SDMMC_ResponseR2Get(p_ctrl->p_reg, p_csd_reg);

    /** Get the CSD version. */
    p_ctrl->status.csd_version = (uint8_t) p_csd_reg->csd_v1_b.csd_structure;

    /** Save sector count (total number of sectors on device) and erase sector count (minimum erasable unit in
     * sectors). */
    uint32_t mult;
    if ((SDMMC_CSD_VERSION_1_0 == p_ctrl->status.csd_version) || (SDMMC_CARD_TYPE_MMC == p_ctrl->status.card_type))
    {
        mult                        =  (uint32_t) (1U << (p_csd_reg->csd_v1_b.c_size_mult + 2));
        p_ctrl->status.sector_count =  (uint32_t) ((uint32_t) (p_csd_reg->csd_v1_b.c_size + 1) * mult);

        /* Scale the sector count by the actual block size. */
        uint32_t read_sector_size   = (uint32_t) 1U << p_csd_reg->csd_v1_b.read_bl_len;
        p_ctrl->status.sector_count = p_ctrl->status.sector_count * (read_sector_size / SDMMC_MAX_BLOCK_SIZE);

        if (SDMMC_CARD_TYPE_MMC == p_ctrl->status.card_type)
        {
            /* If c_size is 0xFFF, then sector_count should be obtained from the extended CSD. Set it to 0 to indicate it
             * should come from the extended CSD later. */
            if (0xFFFU == p_csd_reg->csd_v1_b.c_size)
            {
                p_ctrl->status.sector_count = 0U;
            }
        }
    }
    else if (SDMMC_CSD_VERSION_2_0 == p_ctrl->status.csd_version)
    {
        p_ctrl->status.sector_count       = (p_csd_reg->csd_v2_b.c_size + 1U) * 1024;
    }
    if (SDMMC_CSD_VERSION_1_0 == p_ctrl->status.csd_version)
    {
        /* Get the minimum erasable unit (in 512 byte sectors). */
        p_ctrl->status.erase_sector_count = p_csd_reg->csd_v1_b.sector_size + 1U;
    }
    else
    {
        /* For SDHC and SDXC cards, there are no erase group restrictions.
         *
         * Using the eMMC TRIM operation, there are no erase group restrictions. */
        p_ctrl->status.erase_sector_count = 1U;
    }

    return true;
}

/*******************************************************************************************************************//**
 * Get and store relevant extended card specific data.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 *
 * @retval  true          Relevant extended card specific data obtained.
 * @retval  false         Operation failed.
 **********************************************************************************************************************/
static bool r_sdmmc_csd_extended_get (sdmmc_instance_ctrl_t * const p_ctrl)
{
    /** Ask card to send extended CSD (CMD8). */
    if (!(r_sdmmc_read_and_block(p_ctrl, SDMMC_EMMC_CMD_SEND_EXT_CSD, p_ctrl->status.sdhi_rca << 16, SDMMC_EMMC_EXT_CSD_SIZE)))
    {
        return false;
    }

    /** Store CSD version and device type.  Also store sector count if it is not already determined. */
    uint32_t read_data = 0U;
    uint8_t * p_read_data_8 = (uint8_t *) &read_data;
    for (uint32_t i = 0U; (i < (SDMMC_EMMC_EXT_CSD_SIZE / sizeof(uint32_t))); i++)
    {
        read_data = HW_SDMMC_DataGet(p_ctrl->p_reg);

        if ((SDMMC_EMMC_EXT_CSD_REVISION_OFFSET / sizeof(uint32_t)) == i)
        {
            p_ctrl->status.csd_version = p_read_data_8[SDMMC_EMMC_EXT_CSD_REVISION_OFFSET % sizeof(uint32_t)];
        }

        if ((SDMMC_EMMC_EXT_CSD_DEVICE_TYPE_OFFSET / sizeof(uint32_t)) == i)
        {
            p_ctrl->status.device_type = p_read_data_8[SDMMC_EMMC_EXT_CSD_DEVICE_TYPE_OFFSET % sizeof(uint32_t)];
        }

        if ((SDMMC_EMMC_EXT_CSD_SEC_COUNT_OFFSET / sizeof(uint32_t)) == i)
        {
            if (0U == p_ctrl->status.sector_count)
            {
                p_ctrl->status.sector_count = read_data;
            }
        }
    }

    return r_sdmmc_wait_for_access_end(p_ctrl);
}

/*******************************************************************************************************************//**
 * Issue command expecting data to be returned, and block until read data is returned.
 *
 * @param[in]     p_ctrl      Pointer to SDMMC instance control block.
 * @param[in]     command     Command to issue.
 * @param[in]     argument    Argument to send with command.
 * @param[in]     byte_count  Expected number of bytes to read.
 *
 * @retval  true          Read data available.
 * @retval  false         Operation failed.
 **********************************************************************************************************************/
static bool r_sdmmc_read_and_block (sdmmc_instance_ctrl_t * const p_ctrl, uint16_t command, uint32_t argument, uint32_t byte_count)
{
    /* Set the block count. */
    HW_SDMMC_BlockCountEnable(p_ctrl->p_reg, 0U);

    /** Set block size */
    HW_SDMMC_BlockSizeSet(p_ctrl->p_reg, (uint16_t) byte_count);

    /** Issue command. */
    if (!(r_sdmmc_command_send(p_ctrl, command, argument)))
    {
        return false;
    }

    /** Wait for the read buffer to fill up. */
    uint32_t timeout = SDMMC_DATA_TIMEOUT_US;

    /* The event status is updated in the access interrupt.  Use a local copy of the event status to make sure
     * it isn't updated during the loop. */
    volatile sdhi_event_t event;
    event.word = p_ctrl->sdhi_event.word;
    while (timeout > 0U)
    {
        if (event.bit.event_error)
        {
            /* Error occurred. */
            return false;
        }
        if (event.bit.bre)
        {
            /* Read buffer is full. */
            return true;
        }
        R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MICROSECONDS);
        timeout--;

        /* Check for updates to the event status. */
        event.word = p_ctrl->sdhi_event.word;
    }

    /* Timeout. */
    return false;
}

/*******************************************************************************************************************//**
 * Gets or assigns the relative card address.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 *
 * @retval  true          Relative card address is assigned and device is in standby state.
 * @retval  false         Operation failed.
 **********************************************************************************************************************/
static bool r_sdmmc_rca_get (sdmmc_instance_ctrl_t * const p_ctrl)
{
    sdmmc_priv_card_status_t response = {0U};

    /** Send CMD3.  For eMMC, assign an RCA of SDHI channel number + 2. These bits of the argument are ignored for SD
     * cards. */
    p_ctrl->status.sdhi_rca = (uint32_t) (p_ctrl->hw.channel + 2U);
    if (!(r_sdmmc_command_send(p_ctrl, SDMMC_CMD_SEND_RELATIVE_ADDR, p_ctrl->status.sdhi_rca << 16)))
    {
        return false;
    }

    HW_SDMMC_ResponseGet(p_ctrl->p_reg, &response);
    if (SDMMC_CARD_TYPE_SD == p_ctrl->status.card_type)
    {
        /** Get relative card address from the response if the device is an SD card. */
        p_ctrl->status.sdhi_rca = (response.r6.rca);
    }

    return true;
}

/*******************************************************************************************************************//**
 * Set bus width.
 *
 * @param[in]     p_ctrl      Pointer to SDMMC instance control block.
 *
 * @retval  true          Bus width set successfully.
 * @retval  false         Operation failed.
 **********************************************************************************************************************/
static bool r_sdmmc_bus_width_set (sdmmc_instance_ctrl_t * const p_ctrl)
{
    uint8_t  bus_width;
    uint32_t bus_width_setting;

    bus_width = p_ctrl->status.bus_width;

    if (SDMMC_CARD_TYPE_MMC == p_ctrl->status.card_type)
    {
        /** For eMMC, set bus width using CMD6. */
        bus_width_setting = ((bus_width >> 2U) & 0x03U);
        if (!(r_sdmmc_command_send(p_ctrl, SDMMC_CMD_SWITCH,
                (uint32_t) ((0x1U << SDMMC_SWITCH_ACCESS_SHIFT) | (uint32_t) (183U << SDMMC_SWITCH_INDEX_SHIFT) | (bus_width_setting << SDMMC_SWITCH_VALUE_SHIFT)))))
        {
            return false;
        }

        /* The device may signal busy after CMD6.  Wait for busy to clear. */
        if (!r_sdmmc_emmc_wait_for_device(p_ctrl))
        {
            return false;
        }
    }
    else
    {
        /* Send CMD55, app command. */
        bus_width_setting = ((bus_width >> 1U) & 0x03U);
        if (!(r_sdmmc_command_send(p_ctrl, SDMMC_CMD_APP_CMD, p_ctrl->status.sdhi_rca << 16)))
        {
            return false;
        }

        /** For SD cards, set bus width using ACMD6. */
        if (!r_sdmmc_command_send(p_ctrl, SDMMC_CMD_C_ACMD | SDMMC_CMD_SET_BUS_WIDTH, bus_width_setting))
        {
            return false;
        }
    }

    /** Set the bus width in the SDHI peripheral. */
    HW_SDMMC_BusWidthSet(p_ctrl->p_reg, bus_width);
    return true;
}

/*******************************************************************************************************************//**
 * Copies data from source to destination in software.  Used to move data between unaligned and aligned addresses to
 * enable support of DMAC and DTC with unaligned source or destination addresses.  This is used instead of memcpy
 * because this function is called in an interrupt and it is significantly faster than memcpy for GCC.
 *
 * @param[in]     p_src         Source data.
 * @param[in]     bytes         Number of bytes to copy.
 * @param[out]    p_dest        Destination pointer.
 **********************************************************************************************************************/
static void r_sdmmc_software_copy(void const * p_src, uint32_t bytes, void * p_dest)
{
    /** Use 32-bit access as long as possible.  This is possible even if a pointer is unaligned because the CM4
     * architecture supports 32-bit access on unaligned addresses.  If any other core needs to be supported, ensure
     * unaligned access is supported or re-implement this. */
    uint32_t * p_src_32 = (uint32_t *) p_src;
    uint32_t * p_dest_32 = (uint32_t *) p_dest;

    uint32_t words = bytes / sizeof(uint32_t);
    for (uint32_t i = 0U; i < words; i++)
    {
        p_dest_32[i] = p_src_32[i];
    }

    /** If the number of bytes to copy is not a multiple of 4, finish the copy using 8-bit access. */
    uint8_t * p_src_8 = (uint8_t *) p_src;
    uint8_t * p_dest_8 = (uint8_t *) p_dest;
    for (uint32_t i = words * sizeof(uint32_t); i < bytes; i++)
    {
        p_dest_8[i] = p_src_8[i];
    }
}

/*******************************************************************************************************************//**
 * Performs workaround in the transfer interrupt for unaligned reads and writes.
 *
 * @param[in]     p_args         Transfer callback arguments.
 **********************************************************************************************************************/
static void r_sdmmc_transfer_callback(transfer_callback_args_t * p_args)
{
    /** Get the SD/MMC control block from the callback context. */
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) p_args->p_context;

    if (p_ctrl->transfer_blocks_total != p_ctrl->transfer_block_current)
    {
        if (SDMMC_TRANSFER_DIR_READ == p_ctrl->transfer_dir)
        {
            /** If the transfer is a read operation into an unaligned buffer, copy the block read from the aligned
             * buffer in the control block to the application data buffer. */
            r_sdmmc_software_copy((void *) &p_ctrl->aligned_buff[0], p_ctrl->transfer_block_size, p_ctrl->p_transfer_data);
        }
        if (SDMMC_TRANSFER_DIR_WRITE == p_ctrl->transfer_dir)
        {
            /** If the transfer is a write operation from an unaligned buffer, copy the next block to write from the
             * application data buffer to the aligned buffer in the control block. */
            r_sdmmc_software_copy(p_ctrl->p_transfer_data, p_ctrl->transfer_block_size, (void *) &p_ctrl->aligned_buff[0]);
        }
        p_ctrl->transfer_block_current++;
        p_ctrl->p_transfer_data += p_ctrl->transfer_block_size;
    }
    if (p_ctrl->transfer_blocks_total == p_ctrl->transfer_block_current)
    {
        if (SDMMC_TRANSFER_DIR_READ == p_ctrl->transfer_dir)
        {
            /** After the entire read transfer to an unaligned buffer is complete, read transfer end and callback are
             * performed in transfer interrupt to ensure the last block is in the application buffer before the
             * callback is called. */
            r_sdmmc_transfer_end(p_ctrl);

            if (NULL != p_ctrl->p_callback)
            {
                sdmmc_callback_args_t args;
                args.p_context = p_ctrl->p_context;
                args.event = SDMMC_EVENT_TRANSFER_COMPLETE;
                p_ctrl->p_callback(&args);
            }
        }
    }
}

/*******************************************************************************************************************//**
 * Set up transfer to read from device.
 *
 * @param[in]     p_ctrl         Pointer to SDMMC instance control block.
 * @param[in]     block_count    Number of blocks to transfer.
 * @param[in]     bytes          Bytes per block.
 * @param[in]     p_data         Pointer to data to read data from device into.
 *
 * @retval         SSP_SUCCESS      Transfer successfully configured to write data.
 * @return                          See @ref Common_Error_Codes or functions called by this function for other possible
 *                                  return codes. This function calls:
 *                                      * transfer_api_t::open
 **********************************************************************************************************************/
static ssp_err_t r_sdmmc_transfer_read (sdmmc_instance_ctrl_t * const p_ctrl, uint32_t block_count, uint32_t bytes, uint8_t const * p_data)
{
    ssp_err_t      ssp_ret_val;
    transfer_cfg_t cfg;

    /* Check pointer for NULL, transfer function is optional. */
    cfg.p_info                 = p_ctrl->p_lower_lvl_transfer->p_cfg->p_info;
    cfg.p_extend               = p_ctrl->p_lower_lvl_transfer->p_cfg->p_extend;
    cfg.irq_ipl                = p_ctrl->p_lower_lvl_transfer->p_cfg->irq_ipl;
    cfg.p_callback             = r_sdmmc_transfer_callback;
    cfg.p_context              = p_ctrl;
    cfg.auto_enable            = true;

    R_BSP_IrqStatusClear(p_ctrl->transfer_irq);
    HW_SDMMC_DMAModeEnable(p_ctrl->p_reg, true);

    cfg.p_info->p_src          = HW_SDMMC_DataBufferAddressGet(p_ctrl->p_reg);
    cfg.p_info->p_dest         = (void *) p_data;
    cfg.p_info->num_blocks     = (uint16_t) block_count;
    cfg.p_info->src_addr_mode  = TRANSFER_ADDR_MODE_FIXED;
    cfg.p_info->dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED;
    cfg.p_info->size           = TRANSFER_SIZE_4_BYTE;
    cfg.p_info->mode           = TRANSFER_MODE_BLOCK;
    cfg.p_info->irq            = TRANSFER_IRQ_END;
    /* Round up to the nearest multiple of 4 bytes for the transfer. */
    uint32_t words             = (bytes + (sizeof(uint32_t) - 1U)) / sizeof(uint32_t);
    cfg.p_info->length         = (uint16_t) words;
    cfg.p_info->repeat_area    = TRANSFER_REPEAT_AREA_SOURCE;
    cfg.p_info->chain_mode     = TRANSFER_CHAIN_MODE_DISABLED;
    cfg.activation_source      = HW_SDMMC_DmaEventGet((uint8_t) p_ctrl->hw.channel);

    /* If the pointer is not 4-byte aligned or the number of bytes is not a multiple of 4, use a temporary buffer.
     * Data will be transferred from the temporary buffer into the user buffer in an interrupt after each block transfer. */
    if ((0U != ((uint32_t) p_data & 0x3U)) || (0U != (bytes & 3U)))
    {
        cfg.p_info->irq           = TRANSFER_IRQ_EACH;
        cfg.p_info->repeat_area   = TRANSFER_REPEAT_AREA_DESTINATION;

        cfg.p_info->p_dest         = &p_ctrl->aligned_buff[0];

        p_ctrl->transfer_block_current = 0U;
        p_ctrl->transfer_blocks_total = block_count;
        p_ctrl->p_transfer_data = (uint8_t *) &p_data[0];
        p_ctrl->transfer_dir = SDMMC_TRANSFER_DIR_READ;
        p_ctrl->transfer_block_size = bytes;
    }

    /** Configure the transfer driver to read from the SD buffer. */
    ssp_ret_val = p_ctrl->p_lower_lvl_transfer->p_api->open(p_ctrl->p_lower_lvl_transfer->p_ctrl, &cfg);

    return ssp_ret_val;
}

/*******************************************************************************************************************//**
 * Set up transfer to write to device.
 *
 * @param[in]     p_ctrl         Pointer to SDMMC instance control block.
 * @param[in]     block_count    Number of blocks to transfer.
 * @param[in]     bytes          Bytes per block.
 * @param[in]     p_data         Pointer to data to write to device.
 *
 * @retval         SSP_SUCCESS      Transfer successfully configured to write data.
 * @return                          See @ref Common_Error_Codes or functions called by this function for other possible
 *                                  return codes. This function calls:
 *                                      * transfer_api_t::open
 **********************************************************************************************************************/
static ssp_err_t r_sdmmc_transfer_write (sdmmc_instance_ctrl_t * const p_ctrl,
                                uint32_t             block_count,
                                uint32_t             bytes,
                                const uint8_t      * p_data)
{
    ssp_err_t      ssp_ret_val;
    transfer_cfg_t cfg;

    cfg.p_callback             = r_sdmmc_transfer_callback;
    cfg.p_context              = p_ctrl;
    cfg.p_info                 = p_ctrl->p_lower_lvl_transfer->p_cfg->p_info;
    cfg.p_extend               = p_ctrl->p_lower_lvl_transfer->p_cfg->p_extend;
    cfg.irq_ipl                = p_ctrl->p_lower_lvl_transfer->p_cfg->irq_ipl;
    cfg.auto_enable            = true;

    transfer_info_t * p_info = cfg.p_info;

    p_info->p_src          = p_data;
    p_info->num_blocks     = (uint16_t) block_count;
    p_info->src_addr_mode  = TRANSFER_ADDR_MODE_INCREMENTED;
    p_info->dest_addr_mode = TRANSFER_ADDR_MODE_FIXED;
    p_info->mode           = TRANSFER_MODE_BLOCK;
    p_info->irq            = TRANSFER_IRQ_END;
    p_info->repeat_area    = TRANSFER_REPEAT_AREA_DESTINATION;
    p_info->chain_mode     = TRANSFER_CHAIN_MODE_DISABLED;
    p_info->p_dest         = HW_SDMMC_DataBufferAddressGet(p_ctrl->p_reg);
    p_info->size           = TRANSFER_SIZE_4_BYTE;

    /* Round up to the nearest multiple of 4 bytes for the transfer. */
    uint32_t words         = (bytes + (sizeof(uint32_t) - 1U)) / sizeof(uint32_t);
    p_info->length         = (uint16_t) words;

    R_BSP_IrqStatusClear(p_ctrl->transfer_irq);
    HW_SDMMC_DMAModeEnable(p_ctrl->p_reg, true);

    if ((0U != ((uint32_t) p_data & 0x3U)) || (0U != (bytes & 3U)))
    {
        p_info->irq           = TRANSFER_IRQ_EACH;
        p_info->repeat_area   = TRANSFER_REPEAT_AREA_SOURCE;

        /* If the pointer is not 4-byte aligned or the number of bytes is not a multiple of 4, use a temporary buffer.
         * Transfer the first block to the temporary buffer before enabling the transfer.  Subsequent blocks will be
         * transferred from the user buffer to the temporary buffer in an interrupt after each block transfer. */
        r_sdmmc_software_copy(p_data, bytes, (void *) &p_ctrl->aligned_buff[0]);
        p_info->p_src         = &p_ctrl->aligned_buff[0];

        p_ctrl->transfer_block_current = 1U;
        p_ctrl->transfer_blocks_total = block_count;
        p_ctrl->p_transfer_data = (uint8_t *) &p_data[bytes];
        p_ctrl->transfer_dir = SDMMC_TRANSFER_DIR_WRITE;
        p_ctrl->transfer_block_size = bytes;
    }

    cfg.activation_source      = HW_SDMMC_DmaEventGet((uint8_t) p_ctrl->hw.channel);

    /** Configure the transfer driver to write to the SD buffer. */
    ssp_ret_val = p_ctrl->p_lower_lvl_transfer->p_api->open(p_ctrl->p_lower_lvl_transfer->p_ctrl, &cfg);

    return ssp_ret_val;
}

/*******************************************************************************************************************//**
 * Close transfer driver, clear transfer data, and disable transfer in the SDHI peripheral.
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 **********************************************************************************************************************/
static void r_sdmmc_transfer_end (sdmmc_instance_ctrl_t * const p_ctrl)
{
    p_ctrl->transfer_block_current = 0U;
    p_ctrl->transfer_blocks_total = 0U;
    p_ctrl->p_transfer_data = NULL;
    p_ctrl->transfer_dir = SDMMC_TRANSFER_DIR_NONE;
    p_ctrl->transfer_block_size = 0U;

    p_ctrl->p_lower_lvl_transfer->p_api->close(p_ctrl->p_lower_lvl_transfer->p_ctrl);
    HW_SDMMC_DMAModeEnable(p_ctrl->p_reg, false);
    p_ctrl->transfer_in_progress = false;
}

/*******************************************************************************************************************//**
 * Error checking for control().
 *
 * @param[in]     p_ctrl    Pointer to SDMMC instance control block.
 * @param[in]     command   Which control command to execute.
 * @param[in,out] p_data    Pointer to store input or output data.
 *
 * @retval  SSP_SUCCESS                  Device is ready to be accessed.
 * @retval  SSP_ERR_ASSERTION            A required pointer is NULL.
 * @retval  SSP_ERR_NOT_OPEN             Driver has not been initialized.
 * @retval  SSP_ERR_INVALID_SIZE         Block size must be 512 bytes except when an SDIO card is used, in which case
 *                                       block size must be in the range of 1-512 bytes.
 **********************************************************************************************************************/
static ssp_err_t r_sdmmc_control_error_check(sdmmc_instance_ctrl_t * const p_ctrl, ssp_command_t const command, void * p_data)
{
#if SDMMC_CFG_PARAM_CHECKING_ENABLE
    /* Check pointers for NULL values */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_data);

    SDMMC_ERROR_RETURN(SDMMC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);
#endif

    if (SSP_COMMAND_SET_BLOCK_SIZE == command)
    {
        /* Block size is fixed at 512 except for SDIO readIoExt() and writeIoExt() CMD53 APIs. */
        uint32_t block_size = *((uint32_t *) p_data);
        if (p_ctrl->status.sdio)
        {
            /* Check block size, 512 is the maximum block size the peripheral supports */
            SDMMC_ERROR_RETURN(0U != block_size, SSP_ERR_INVALID_SIZE);
            SDMMC_ERROR_RETURN(block_size <= SDMMC_MAX_BLOCK_SIZE, SSP_ERR_INVALID_SIZE);
        }
        else
        {
            /* Check block size, only 512 supported for SD cards and eMMC. */
            SDMMC_ERROR_RETURN(SDMMC_MAX_BLOCK_SIZE == block_size, SSP_ERR_INVALID_SIZE);
        }
    }

    return SSP_SUCCESS;
}


/*******************************************************************************************************************//**
 * Access ISR.
 *
 * Saves context if RTOS is used, clears interrupts, saves events and restores context if RTOS is used.
 **********************************************************************************************************************/
void sdhimmc_accs_isr (void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    if (NULL != p_ctrl)
    {
        sdmmc_callback_args_t args;
        args.event = SDMMC_EVENT_ACCESS;
        r_sdmmc_access_irq_process(p_ctrl, &args);                     /* Service the interrupt flags. */

        /** Call user p_callback */
        if (NULL != p_ctrl->p_callback)                /* Check for user callback function. */
        {
            if (SDMMC_EVENT_NONE != args.event)
            {
                args.p_context = p_ctrl->p_context;
                p_ctrl->p_callback(&args);                 /* Call user callback function. */
            }
        }
    }

    /* Clear the IR flag in the ICU */
    /* Clearing the IR bit must be done after clearing the interrupt source in the the peripheral */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}

/*******************************************************************************************************************//**
 * Card ISR.
 *
 * Saves context if RTOS is used, clears interrupts, checks for card insertion or removal, and restores context if
 * RTOS is used.
 **********************************************************************************************************************/
void sdhimmc_card_isr (void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE
    sdmmc_callback_args_t args;

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    if (NULL != p_ctrl)
    {
        sdhi_event_t info1 = {0U};
        info1.word  = HW_SDMMC_Info1Get(p_ctrl->p_reg);
        info1.word &= SDMMC_SDHI_INFO1_CARD_MASK;

        if (0U != (info1.word & SDMMC_SDHI_INFO1_CARD_REMOVED_MASK))
        {
            p_ctrl->status.ready = false;
            args.event = SDMMC_EVENT_CARD_REMOVED;
        }
        if (0U != (info1.word & SDMMC_SDHI_INFO1_CARD_INSERTED_MASK))
        {
            args.event = SDMMC_EVENT_CARD_INSERTED;
        }

        /* Clear interrupt flags */
        HW_SDMMC_Info1Set(p_ctrl->p_reg, info1.word);

        /** Call user p_callback */
        if (NULL != p_ctrl->p_callback)
        {
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }
    }

    /* Clear the IR flag in the ICU */
    /* Clearing the IR bit must be done after clearing the interrupt source in the the peripheral */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}

/*******************************************************************************************************************//**
 * DMA ISR.  Called when a DTC transfer completes.  Not used for DMAC.
 *
 * Saves context if RTOS is used, clears interrupts, checks for DMA Interrupt.
 **********************************************************************************************************************/
void sdhimmc_dma_req_isr (void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    if (NULL != p_ctrl)
    {
        transfer_callback_args_t args;
        args.p_context = p_ctrl;
        r_sdmmc_transfer_callback(&args);
    }

    /* Clear the IR flag in the ICU.
     * This must be after the callback because the next DTC transfer will begin when this bit is cleared. */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}

/*******************************************************************************************************************//**
 * SDIO ISR.
 *
 * Saves context if RTOS is used, clears interrupts, checks for SDIO Interrupt.
 **********************************************************************************************************************/
void sdhimmc_sdio_isr (void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    sdmmc_instance_ctrl_t * p_ctrl = (sdmmc_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    if (NULL != p_ctrl)
    {
        /** Call user p_callback */
        if (NULL != p_ctrl->p_callback)
        {
            sdmmc_callback_args_t args;
            uint32_t info1 = HW_SDMMC_SDIO_Info1Get(p_ctrl->p_reg);
            if (info1 & 0xC000)
            {
                /* A multi-block CMD53 transfer is complete. */
                args.event     = SDMMC_EVENT_TRANSFER_COMPLETE;
            }
            else
            {
                /* I/O interrupt requested by device. */
                args.event     = SDMMC_EVENT_SDIO;
            }
            args.p_context = p_ctrl->p_context;
            p_ctrl->p_callback(&args);
        }

        /* Clear interrupt flags */
        HW_SDMMC_SDIO_Info1Set(p_ctrl->p_reg, 0xFFFF3FFEU);
    }

    /* Clear the IR flag in the ICU */
    /* Clearing the IR bit must be done after clearing the interrupt source in the the peripheral */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}
