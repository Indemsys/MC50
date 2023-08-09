/***********************************************************************************************************************
 * Copyright [2015-2023] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : r_rspi.c
 * Description  : This module contains API functions and HLD layer functions for RSPI module.
 ********************************************************************************************************************/


/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/
#include <string.h>
#include "r_rspi.h"
#include "r_spi_api.h"
#include "r_rspi_private.h"
#include "./hw/hw_rspi_private.h"
#include "r_rspi_private_api.h"
#include "r_cgc.h"
#include "r_cgc_api.h"
#include "r_fmi.h"

/*********************************************************************************************************************
 * Macro definitions
 *********************************************************************************************************************/
/** "RSPI" in ASCII, used to determine if channel is open. */
#define RSPI_OPEN               (0x52535049ULL)

#ifndef RSPI_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define RSPI_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &module_version)
#endif

#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug.*/
/*LDRA_INSPECTED 69 S This will result is GCC compiler warning if not suppressed. */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** SPI HAL module version data structure */
static const ssp_version_t module_version =
{
    .api_version_major  = SPI_API_VERSION_MAJOR,
    .api_version_minor  = SPI_API_VERSION_MINOR,
    .code_version_major = RSPI_CODE_VERSION_MAJOR,
    .code_version_minor = RSPI_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S This will result is GCC compiler warning (due to a GCC compiler bug) if not suppressed. */
#pragma GCC diagnostic pop
#endif

/**********************************************************************************************************************
 * Typedef definitions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private global variables
 *********************************************************************************************************************/
/** Name of module used by error logger macro. */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "spi";
#endif

/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const spi_api_t g_spi_on_rspi =
{
    .open      = R_RSPI_Open,
    .read      = R_RSPI_Read,
    .write     = R_RSPI_Write,
    .writeRead = R_RSPI_WriteRead,
    .close     = R_RSPI_Close,
    .versionGet= R_RSPI_VersionGet
};

/*********************************************************************************************************************
 * Private function declarations
 ********************************************************************************************************************/
/* Common routine used by RSPI API write or read functions. */
static ssp_err_t rspi_write_read_common(rspi_instance_ctrl_t       * const p_ctrl,
                                        void                 const *       p_src,
                                        void                 const *       p_dest,
                                        uint32_t             const         length,
                                        spi_bit_width_t      const         bit_width,
                                        spi_operation_t                    tx_rx_mode);

/* This function determines the RSPI channel SPBR register setting for the requested baud rate. */
static uint32_t rspi_baud_set(rspi_instance_ctrl_t * p_ctrl, uint32_t baud_target);

/* This function is the common ISR handler for transmitting data. */
static void rspi_tx_handler(rspi_instance_ctrl_t * p_ctrl);

/* This function is the common ISR handler for the SPRI interrupt. */
static void rspi_rx_handler(rspi_instance_ctrl_t * p_ctrl);

/* This function is the common ISR handler for SPEI interrupts. */
static void rspi_spei_isr_common (rspi_instance_ctrl_t * p_ctrl);

/* Set the default configuration for the registers. */
static void rspi_default_config_set(rspi_instance_ctrl_t * p_ctrl);

/* Set the basic configuration (simple SPI) from the user's configuration. */
static void rspi_common_config_set(rspi_instance_ctrl_t * p_ctrl, spi_cfg_t const * const p_cfg);

/* Set the extended configuration (RSPI) from the user's extended configuration. */
static void rspi_extended_config_set(rspi_instance_ctrl_t * p_ctrl, spi_cfg_t const * const p_cfg);

/* Configures SCI SPI related transfer drivers (if enabled). */
static ssp_err_t rspi_transfer_open(spi_cfg_t const * const p_cfg);

/* Opens transfer instances for dtc transfer. */
static ssp_err_t rspi_transfer_reopen(rspi_instance_ctrl_t  *const p_ctrl);

/* Closes transfer instance and reconfigures transfer instance structure based on bit width */
static void rspi_dtc_close_reconfig_p_info(rspi_instance_ctrl_t    * const p_ctrl,
                                     spi_bit_width_t const         bit_width);

/* Configures SCI SPI related transfer drivers (if enabled). */
static void rspi_transfer_close(spi_cfg_t const * const p_cfg);

/* Configure interrupts. */
static ssp_err_t rspi_irq_cfg(ssp_feature_t * p_feature,
                              ssp_signal_t    signal,
                              uint8_t         ipl,
                              void          * p_ctrl,
                              IRQn_Type     * p_irq);

/* Sets interrupt priority and initializes vector info. */
static ssp_err_t rspi_initialize_hardware(ssp_feature_t              *       ssp_feature,
                                          rspi_instance_ctrl_t       *       p_ctrl,
                                          spi_cfg_t            const * const p_cfg);

#if (RSPI_CFG_PARAM_CHECKING_ENABLE)
/* Checks for valid parameters. */
static ssp_err_t rspi_parameter_check(transfer_instance_t const *p_tx_rx);
#endif

/* Tries doing RX/TX using transfer interface. */
static ssp_err_t rspi_tx_transfer_reset(rspi_instance_ctrl_t * const p_ctrl,
                                        spi_operation_t              tx_rx_mode,
                                        uint32_t             *       p_data_reg,
                                        uint32_t               const length );

/* Tries doing RX/TX using transfer interface. */
static ssp_err_t rspi_rx_transfer_reset(rspi_instance_ctrl_t * const p_ctrl,
                                        uint32_t             *       p_data_reg,
                                        uint32_t               const length );

/* Tries doing RX/TX using transfer interface. */
static ssp_err_t rspi_transfer_reset(rspi_instance_ctrl_t * const p_ctrl,
                                     spi_operation_t              tx_rx_mode,
                                     uint32_t               const length );

/* Update bit width for current transaction. */
static void rspi_update_transaction_bit_width (rspi_instance_ctrl_t * const p_ctrl, spi_bit_width_t bit_width);

/* Service functions for various interrupts*/

/* This function is the ISR function for RSPI error (SPEI) interrupts. */
void spi_eri_isr(void);

/* This function is the ISR function for RSPI receive buffer full (SPRI) interrupts. */
void spi_rxi_isr(void);

/* This is the ISR function for RSPI RSPI SPTI transmit buffer empty (SPTI) interrupts. */
void spi_txi_isr(void);

/* This is the ISR function for RSPI RSPI SPTI idle interrupts. */
void spi_tei_isr(void);

/* Read-only variable for writing blank data when doing Read operations using DTC. */
static const uint32_t g_dummy = (0xFFFFFFFFU);

/*****************************************************************************************************************//**
 * @addtogroup SPI
 * @{
 *********************************************************************************************************************/

/***************************************************************************************************************
 * Functions
 ****************************************************************************************************************/

/*************************************************************************************************************//**
 * @brief  This functions initializes a channel for SPI communication mode.
 *
 * Implements spi_api_t::open
 *          This function performs the following tasks:
 *          Performs parameter checking and processes error conditions.
 *          Applies power to the SPI channel.
 *          Disables interrupts.
 *          Initializes the associated registers with some default value and the user-configurable options.
 *          Provides the channel control for use with other API functions.
 *          Updates user-configurable file if necessary.
 *
 * @retval  SSP_SUCCESS               Channel initialized successfully.
 * @retval  SSP_ERR_ASSERTION         NULL pointer to following parameters
 *                                    p_ctrl, p_cfg,
 *                                    p_cfg::p_transfer_rx::p_api, p_cfg::p_transfer_rx::p_ctrl,
 *                                    p_cfg::p_transfer_rx::p_cfg, p_cfg::p_transfer_rx::p_cfg::p_info.
 *                                    or failed to set the baud rate,
 * @retval  SSP_ERR_INVALID_ARGUMENT  An element of the r_spi_cfg_t structure contains an invalid value.
 *                                    The parameters is out of range.
 *                                    Both transfer modules need to be present or absent.
 * @retval  SSP_ERR_HW_LOCKED         The lock could not be acquired. The channel is busy.
 * @return                            See @ref Common_Error_Codes or functions called by this function
 *                                    for other possible return codes. This function calls:
 *                                    * transfer_api_t::open
 *                                    * fmi_api_t::productFeatureGet
 *                                    * fmi_api_t::eventInfoGet
 * @note  This function is reentrant.
 ***************************************************************************************************************/
ssp_err_t R_RSPI_Open (spi_ctrl_t * p_api_ctrl, spi_cfg_t const * const p_cfg)
{
    rspi_instance_ctrl_t * p_ctrl = (rspi_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t result  = SSP_SUCCESS;
    uint8_t   channel = (uint8_t)p_cfg->channel;

#if RSPI_CFG_PARAM_CHECKING_ENABLE
    /* Perform parameter checking. */
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_ctrl);
#endif /* If RSPI_CFG_PARAM_CHECKING_ENABLE. */

    ssp_err_t err;
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = p_cfg->channel;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_SPI;
    fmi_feature_info_t info = {0U};
    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    RSPI_ERROR_RETURN(SSP_SUCCESS == err, err);

    /* Attempt to acquire lock for this RSPI channel. Prevents re-entrance conflict. */
    result = R_BSP_HardwareLock(&ssp_feature);
    RSPI_ERROR_RETURN((SSP_SUCCESS == result), SSP_ERR_HW_LOCKED);

    p_ctrl->p_reg = (R_RSPI0_Type *) info.ptr;

    /* Initialize interrupts, transfer instances and start the module. */
    result = rspi_initialize_hardware(&ssp_feature, p_ctrl, p_cfg);
    if (SSP_SUCCESS != result)
    {
        rspi_transfer_close(p_cfg);
        R_BSP_HardwareUnlock(&ssp_feature);
        return result;
    }

    /* Update info to the control block. */
    p_ctrl->channel = channel;
    p_ctrl->p_dest = NULL;
    p_ctrl->p_dest = NULL;
    p_ctrl->xfr_length = 0U;
    p_ctrl->tx_count = 0U;
    p_ctrl->rx_count = 0U;
    p_ctrl->p_transfer_rx  = p_cfg->p_transfer_rx;
    p_ctrl->p_transfer_tx  = p_cfg->p_transfer_tx;
    p_ctrl->using_dtc = ((NULL != p_cfg->p_transfer_rx) && (NULL != p_cfg->p_transfer_tx));
    p_ctrl->tx_dtc_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED;
    p_ctrl->rx_dtc_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED;
    p_ctrl->tx_handler_spdr_ha = 0U;

    /* Set the default register configuration condition. */
    rspi_default_config_set(p_ctrl);

    if (SPI_MODE_MASTER == p_cfg->operating_mode)
    {
        /* Set the base bit rate. Modifies the SPBR register setting with requested baud rate.*/
        if (0 == rspi_baud_set(p_ctrl, p_cfg->bitrate))
        {
            rspi_transfer_close(p_cfg);

            /* Failed to set the baud rate. */
            R_BSP_HardwareUnlock(&ssp_feature);
            SSP_ERROR_LOG((SSP_ERR_ASSERTION), (&g_module_name[0]), (&module_version));

            /* Could not calculate settings for the requested baud rate. */
            return SSP_ERR_ASSERTION;
        }
    }

    /* Set the common configuration based on the user's basic configuration for simple SPI mode. */
    rspi_common_config_set(p_ctrl, p_cfg);

    /* Do the extended configuration if it's needed based on the user's extended configuration. */
    rspi_extended_config_set(p_ctrl, p_cfg);

    /* Peripheral Initialized. */
    p_ctrl->p_callback = p_cfg->p_callback;
    p_ctrl->p_context = p_cfg->p_context;

    /* Enable all IRQ. */
    NVIC_EnableIRQ(p_ctrl->txi_irq);
    NVIC_EnableIRQ(p_ctrl->rxi_irq);
    NVIC_EnableIRQ(p_ctrl->eri_irq);
    NVIC_EnableIRQ(p_ctrl->tei_irq);

    /* Mark control block as opened */
    p_ctrl->channel_opened = RSPI_OPEN;

    return SSP_SUCCESS;
}/* End of function R_RSPI_Open(). */

/*************************************************************************************************************//**
 * @brief   This function receives data from a SPI device.
 *
 * Implements spi_api_t::read
 *          The function performs the following tasks:
 *          Performs parameter checking and processes error conditions.
 *          Disable Interrupts.
 *          Disable the SPI bus.
 *          Setup data bit width per user request.
 *          Enable the SPI bus.
 *          Enable interrupts.
 *          Start data transmission with dummy data via transmit buffer empty interrupt.
 *          Copy data from source buffer to the SPI data register for transmission.
 *          Receive data from receive buffer full interrupt occurs and copy data to the buffer of destination.
 *          Complete data reception via receive buffer full interrupt and transmitting dummy data.
 *
 * @retval  SSP_SUCCESS                   Read operation successfully completed.
 * @retval  SSP_ERR_ASSERTION             NULL pointer to control or destination parameters or transfer length is zero.
 * @retval  SSP_ERR_UNSUPPORTED           With DTC transfer mode, bit_width must match configured DTC transfer width
 * @retval  SSP_ERR_HW_LOCKED             The lock could not be acquired. The channel is busy.
 * @retval  SSP_ERR_NOT_OPEN              The channel has not been opened. Open channel first.
 * @retval  SSP_ERR_INVALID_HW_CONDITION  Failed to clear errors in the module
 * @note  This function is reentrant.
 ***************************************************************************************************************/
ssp_err_t R_RSPI_Read (spi_ctrl_t            * const p_api_ctrl,
                       void            const *       p_dest,
                       uint32_t        const         length,
                       spi_bit_width_t const         bit_width)
{
    rspi_instance_ctrl_t * p_ctrl = (rspi_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t result;

#if RSPI_CFG_PARAM_CHECKING_ENABLE

    /* Perform parameter checking. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_dest);

    /* Check the data length, should not be 0. */
    SSP_ASSERT(0 != length);
#endif /* If RSPI_CFG_PARAM_CHECKING_ENABLE. */

    /* Configure the module for full duplex operation. */
    HW_RSPI_CommunicationTransmitOnly(p_ctrl->p_reg, RSPI_COMMUNICATION_FULL_DUPLEX);

    result = rspi_write_read_common(p_ctrl, NULL, p_dest, length, bit_width, SPI_OPERATION_DO_RX);
    return result;
}/* End of function R_RSPI_Read(). */

/*************************************************************************************************************//**
 * @brief   This function transmits data to a SPI device using the TX Only Communications Operation Mode.
 *
 * Implements spi_api_t::write
 *          The function performs the following tasks:
 *          Performs parameter checking and processes error conditions.
 *          Disable Interrupts.
 *          Disable the SPI bus.
 *          Setup data bit width per user request.
 *          Enable the SPI bus.
 *          Enable interrupts.
 *          Start data transmission with dummy data via transmit buffer empty interrupt.
 *          Copy data from source buffer to the SPI data register for transmission.
 *          Receive data from receive buffer full interrupt occurs and do nothing with the received data.
 *          Complete data transmission via receive buffer full interrupt.
 *
 * @retval  SSP_SUCCESS                     Write operation successfully completed.
 * @retval  SSP_ERR_ASSERTION               NULL pointer to control or source parameters or transfer length is zero.
 * @retval  SSP_ERR_UNSUPPORTED             With DTC transfer mode, bit_width must match configured DTC transfer width
 * @retval  SSP_ERR_HW_LOCKED               The lock could not be acquired. The channel is busy.
 * @retval  SSP_ERR_NOT_OPEN                The channel has not been opened. Open the channel first.
 * @retval  SSP_ERR_INVALID_HW_CONDITION    Failed to clear errors in the module
 * @note  This function is reentrant.
 ***************************************************************************************************************/
ssp_err_t R_RSPI_Write (spi_ctrl_t            * const p_api_ctrl,
                        void            const *       p_src,
                        uint32_t                const length,
                        spi_bit_width_t         const bit_width)
{
    rspi_instance_ctrl_t * p_ctrl = (rspi_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t result;

#if RSPI_CFG_PARAM_CHECKING_ENABLE
    /* Perform parameter checking. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_src);

    /* Check the data length, should not be 0. */
    SSP_ASSERT(0 != length);
#endif /* If RSPI_CFG_PARAM_CHECKING_ENABLE. */

    /* Configure the module for transmit only operation. */
    HW_RSPI_CommunicationTransmitOnly(p_ctrl->p_reg, RSPI_COMMUNICATION_TRANSMIT_ONLY);

    result = rspi_write_read_common(p_ctrl, p_src, NULL, length, bit_width, SPI_OPERATION_DO_TX);
    return result;
} /* End of function R_RSPI_Write(). */

/*************************************************************************************************************//**
 * @brief   This function simultaneously transmits data to a SPI device while receiving data from a SPI device
 *          (full duplex).
 *
 * Implements spi_api_t::writeread
 *          The function performs the following tasks:
 *          Performs parameter checking and processes error conditions.
 *          Disable Interrupts.
 *          Disable the SPI bus.
 *          Setup data bit width per user request.
 *          Enable the SPI bus.
 *          Enable interrupts.
 *          Start data transmission using transmit buffer empty interrupt.
 *          Copy data from source buffer to the SPI data register for transmission.
 *          Receive data from receive buffer full interrupt occurs and copy data to the buffer of destination.
 *          Complete data transmission and reception via receive buffer full interrupt.
 *
 * @retval  SSP_SUCCESS                   Write operation successfully completed.
 * @retval  SSP_ERR_ASSERTION             NULL pointer to control, source or destination parameters or
 *                                        transfer length is zero.
 * @retval  SSP_ERR_UNSUPPORTED           With DTC transfer mode, bit_width must match configured DTC transfer width
 * @retval  SSP_ERR_HW_LOCKED             The lock could not be acquired. The channel is busy.
 * @retval  SSP_ERR_NOT_OPEN              The channel has not been opened. Open the channel first.
 * @retval  SSP_ERR_INVALID_HW_CONDITION  Failed to clear errors in the module
 * @note  This function is reentrant.
 ***************************************************************************************************************/
ssp_err_t R_RSPI_WriteRead (spi_ctrl_t            * const p_api_ctrl,
                            void            const *       p_src,
                            void            const *       p_dest,
                            uint32_t                const length,
                            spi_bit_width_t         const bit_width)

{
    rspi_instance_ctrl_t * p_ctrl = (rspi_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t result;

#if RSPI_CFG_PARAM_CHECKING_ENABLE
    /* Perform parameter checking. */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_src);
    SSP_ASSERT(NULL != p_dest);

    /* Check the data length, should not be 0. */
    SSP_ASSERT(0 != length);
#endif /* If RSPI_CFG_PARAM_CHECKING_ENABLE. */

    /* Configure the module for full duplex operation. */
    HW_RSPI_CommunicationTransmitOnly(p_ctrl->p_reg, RSPI_COMMUNICATION_FULL_DUPLEX);

    result = rspi_write_read_common(p_ctrl, p_src, p_dest, length, bit_width, SPI_OPERATION_DO_TX_RX);
    return result;
}/* End of function R_RSPI_WriteRead(). */

/**************************************************************************************************************//**
 * @brief   This function initiates write or read process. Common routine used by RSPI API write or read functions.
 *
 * @param[in]  p_ctrl                       Pointer to the control block.
 * @param[in]  p_src                        Pointer to data buffer which need to be sent.
 * @param[out] p_dest                       Pointer to buffer where received data will be stored.
 * @param[in]  length                       Number of data transactions to be performed.
 * @param[in]  bit_width                    Size of data for each transaction.
 * @param[in]  tx_rx_mode                   Mode of the data transaction.
 *
 * @retval  SSP_SUCCESS                     Operation successfully completed.
 * @retval  SSP_ERR_HW_LOCKED               The lock could not be acquired. The channel is busy.
 * @retval  SSP_ERR_NOT_OPEN                The channel has not been opened. Perform R_RSPI_Open() first
 * @retval  SSP_ERR_INVALID_HW_CONDITION    Failed to clear errors in the module
 * @return                                  See @ref Common_Error_Codes or functions called by this function for
 *                                          other possible return codes. This function calls: *
 *                                          * transfer_api_t::close
 *                                          * transfer_api_t::open
 *                                          * transfer_api_t::reset
 * @note  This function is reentrant.
 ***************************************************************************************************************/
static ssp_err_t rspi_write_read_common (rspi_instance_ctrl_t      * const p_ctrl,
                                         void                const *       p_src,
                                         void                const *       p_dest,
                                         uint32_t                    const length,
                                         spi_bit_width_t             const bit_width,
                                         spi_operation_t                   tx_rx_mode)
{
    ssp_err_t result = SSP_SUCCESS;

    /* Check if the device is even open, return an error if not */
    RSPI_ERROR_RETURN((RSPI_OPEN == p_ctrl->channel_opened), SSP_ERR_NOT_OPEN);

    /* Attempt to acquire lock for this transfer operation. Prevents re-entrance conflict. */
    result = R_BSP_SoftwareLock(&p_ctrl->resource_lock_tx_rx);
    RSPI_ERROR_RETURN(SSP_SUCCESS == result, SSP_ERR_HW_LOCKED);

    p_ctrl->tx_count           = 0U;
    p_ctrl->rx_count           = 0U;
    p_ctrl->xfr_length         = length;
    p_ctrl->bytes_per_transfer = bit_width;
    p_ctrl->p_src              = (void *) p_src;
    p_ctrl->p_dest             = (void *) p_dest;
    p_ctrl->transfer_mode      = tx_rx_mode;

    p_ctrl->do_tx = (bool)((uint8_t) tx_rx_mode & (uint8_t) SPI_OPERATION_DO_TX);

    if (RSPI_BYTE_SWAP_ENABLE == p_ctrl->byte_swap)
    {
        if (SPI_BIT_WIDTH_8_BITS != p_ctrl->bytes_per_transfer)
        {
            /* For swapping of data register */
            HW_RSPI_Byte_Swap_Enable(p_ctrl->p_reg, true);
            /* Disable Parity when swapping is enable. */
            /* As per HM, When byte swap is valid, set parity function to invalid else the behavior is not
             * guaranteed.*/
            HW_RSPI_ParityEnable (p_ctrl->p_reg, false);
        }
        else
        {
            /* As per HM, Byte swap operation is not valid for SPI_BIT_WIDTH_8_BITS */
            R_BSP_SoftwareUnlock(&p_ctrl->resource_lock_tx_rx);
            SSP_ERROR_LOG((result), (&g_module_name[0]), (&module_version));
            return SSP_ERR_INVALID_SIZE;
        }
    }
    else
    {
        /* Disable byte swap if not enabled */
        HW_RSPI_Byte_Swap_Enable(p_ctrl->p_reg, false);
    }

    /* Clear error sources: the SPSR.MODF, OVRF, and PERF flags. */
    HW_RSPI_RxBufferFull_Underrun_Set(p_ctrl->p_reg);
    if (HW_RSPI_ErrorsClear(p_ctrl->p_reg))
    {
        R_BSP_SoftwareUnlock(&p_ctrl->resource_lock_tx_rx);
        SSP_ERROR_LOG((result), (&g_module_name[0]), (&module_version));
        return SSP_ERR_INVALID_HW_CONDITION;
    }

    /* Check compatibility in transfer mode. For 8 bit operations the first transfer rx block has a 4byte
     * transfer size so compare the bit_width with the tx transfer. */
    if (NULL != p_ctrl->p_transfer_tx)
    {
        /* Check if user API bit width and dtc transfer size are different */
        if((1U << p_ctrl->p_transfer_tx->p_cfg->p_info->size) != bit_width)
        {
            /* Reconfigure dtc by closing dtc,reconfiguring parameters and opening dtc */
            rspi_dtc_close_reconfig_p_info(p_ctrl, bit_width);
            result = rspi_transfer_reopen(p_ctrl);
            if (SSP_SUCCESS != result)
            {
                R_BSP_SoftwareUnlock(&p_ctrl->resource_lock_tx_rx);
                return result;
            }
        }
    }

    if (p_ctrl->using_dtc)
    {
        /* Try to do transaction using transfer interface. */
        result = rspi_transfer_reset(p_ctrl, tx_rx_mode, length);
        if (SSP_SUCCESS != result)
        {
            R_BSP_SoftwareUnlock(&p_ctrl->resource_lock_tx_rx);
            SSP_ERROR_LOG((result), (&g_module_name[0]), (&module_version));
            return result;
        }
    }
    else
    {
        /* Update bit width for current transaction. */
        rspi_update_transaction_bit_width(p_ctrl, bit_width);
    }

    /* Disable the idle interrupt. */
    HW_RSPI_IdleInterruptDisable(p_ctrl->p_reg);

    /* Disable transmit end interrupt. */
    NVIC_DisableIRQ(p_ctrl->tei_irq);
    NVIC_ClearPendingIRQ(p_ctrl->tei_irq);

    /* Clear all IRQ interrupt status. */
    R_BSP_IrqStatusClear(p_ctrl->txi_irq);
    R_BSP_IrqStatusClear(p_ctrl->rxi_irq);
    R_BSP_IrqStatusClear(p_ctrl->eri_irq);
    R_BSP_IrqStatusClear(p_ctrl->tei_irq);

    NVIC_ClearPendingIRQ(p_ctrl->txi_irq);
    NVIC_EnableIRQ(p_ctrl->txi_irq);

    if(tx_rx_mode == SPI_OPERATION_DO_TX)
    {
        HW_RSPI_InterruptEnableTx(p_ctrl->p_reg);
    }
    else
    {
        HW_RSPI_InterruptEnable(p_ctrl->p_reg);
    }

    return result;
}/* End of function rspi_write_read_common(). */

/*************************************************************************************************************//**
 * @brief   This function manages the closing of a channel by the following task.
 *
 * Implements spi_api_t::close
 *          Disables SPI operations by disabling the SPI bus.
 *          Power off the channel.
 *          Disables all the associated interrupts.
 *          Update channel status.
 *
 * @retval  SSP_SUCCESS              Channel successfully closed.
 * @retval  SSP_ERR_ASSERTION        A required pointer argument is NULL.
 * @retval  SSP_ERR_NOT_OPEN         The channel has not been opened. Open the channel first.
 * @note  This function is reentrant.
 ****************************************************************************************************************/
ssp_err_t R_RSPI_Close (spi_ctrl_t * const p_api_ctrl)
{
    rspi_instance_ctrl_t * p_ctrl = (rspi_instance_ctrl_t *) p_api_ctrl;

#if RSPI_CFG_PARAM_CHECKING_ENABLE
    /* Perform parameter checking. */
    SSP_ASSERT(NULL != p_ctrl);
#endif /* If RSPI_CFG_PARAM_CHECKING_ENABLE. */

    /* Check if the device is even open, return an error if not */
    RSPI_ERROR_RETURN((RSPI_OPEN == p_ctrl->channel_opened), SSP_ERR_NOT_OPEN);

    /* Mark control block as uninitialized */
    p_ctrl->channel_opened = 0U;

    /* SPE and SPTIE should be cleared simultaneously. */
    HW_RSPI_InterruptDisable(p_ctrl->p_reg);

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = p_ctrl->channel;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_SPI;

    /* Disable module interrupts. */
    NVIC_DisableIRQ(p_ctrl->txi_irq);
    NVIC_DisableIRQ(p_ctrl->rxi_irq);
    NVIC_DisableIRQ(p_ctrl->eri_irq);
    NVIC_DisableIRQ(p_ctrl->tei_irq);

    /* Turn off power. */
    R_BSP_ModuleStop(&ssp_feature);

    /* Close transfer block. */
    if (NULL != p_ctrl->p_transfer_rx)
    {
        p_ctrl->p_transfer_rx->p_api->close(p_ctrl->p_transfer_rx->p_ctrl);
    }

    if (NULL != p_ctrl->p_transfer_tx)
    {
        p_ctrl->p_transfer_tx->p_api->close(p_ctrl->p_transfer_tx->p_ctrl);
    }

    /* Release software lock for this control block. */
    R_BSP_SoftwareUnlock(&p_ctrl->resource_lock_tx_rx);

    /* Release lock for this channel. */
    /* All RSPI channels are listed in order in the bsp_hw_lock_t enum, so adding the channel number offset from
     * the base channel 0 lock yields the channel's lock type. */
    R_BSP_HardwareUnlock(&ssp_feature);

    return SSP_SUCCESS;
}/* End of function R_RSPI_Close(). */

/************************************************************************************************************//**
 * @brief       This function determines the RSPI channel SPBR register setting for the requested baud rate.
 *
 *              Returns the actual bit rate that the setting will achieve which may differ from requested.
 *              If the requested bit rate cannot be exactly achieved, the next lower bit rate setting will be applied.
 *              If successful, applies the calculated setting to the SPBR register.
 *
 * @param[in]   p_ctrl      Pointer to the control block for the channel.
 * @param[in]   bps_target  The requested baud rate.
 * @retval      0           Error conditions.
 * @retval      bps_calc    The actual BPS rate achieved
 * @note        Target baud must be >= PCLK/4 to get anything out.
 *              The BRDV[1:0} bits are set from 0 to 3 to get the target bit rate.
 ***************************************************************************************************************/
static uint32_t rspi_baud_set (rspi_instance_ctrl_t * p_ctrl, uint32_t bps_target)
{
    uint8_t spbr_result = 0;
    uint32_t bps_calc = 0;
    uint32_t n = 0;
    uint32_t clock_mhz = 0;
    uint32_t temp_n_brdv = 0;
    uint32_t n_brdv = 0;

    bsp_feature_rspi_t rspi_feature = {0};
    R_BSP_FeatureRspiGet(&rspi_feature);

    /* Read the clock frequency. */
    g_cgc_on_cgc.systemClockFreqGet((cgc_system_clocks_t) rspi_feature.clock, &clock_mhz);

    /* Get the register settings for requested baud rate. */
    if ((clock_mhz / bps_target) < 2U)
    {
        /* Baud_bps_target too high for the PCLK. */
        return 0;
    }

    /*
     * From Hardware manual: Bit rate = f / (2(n + 1)(2^N))
     * where:
     *      f = PCLK, n = SPBR setting, N = BRDV bits
     * Solving for n:
     *      n = (((f/(2^N))/2) / bps) - 1
     */

    /* Calculate BRDV value to get SPBR setting for the board PCLK.
     * BRDV setting will be done during write/read operations.
     *
     * Bit rate (BPS)  = f / (2(n+1)(2^N)).
     *
     * n = (f / (2 * (2^N) * BPS)) - 1.
     */
    for (temp_n_brdv = 0x0U; temp_n_brdv < 0x04U; temp_n_brdv++)
    {
        uint32_t denominator = (0x02U * (0x0001U << temp_n_brdv) * bps_target);

        /* Solve for SPBR setting. */
        n = (((uint32_t) clock_mhz / denominator) - 0x01U);
        if (n <= RSPI_SPBR_MAX_VAL)
        {
            /* For N, SPBR setting is in valid range. So don't have to try for next N. */
            n_brdv = temp_n_brdv;
            break;
        }
        else
        {
            /* For N, SPBR setting is in invalid range.
               So continue. */
        }
    }
    /* Must be <= SPBR register max value for any of valid N (= 0,1,2,3). Must not be negative. */
    if (n <= RSPI_SPBR_MAX_VAL)
    {
        /* Now plug n back into the formula for BPS and check it. */
        bps_calc = (uint32_t) (clock_mhz / (2U * (0x0001U << n_brdv) * (n + 1U)));
        while ((bps_calc > bps_target) && ((n <= RSPI_SPBR_MAX_VAL)))
        {
            n += 1U;
            bps_calc = (uint32_t) (clock_mhz / (2U * (0x0001U<< n_brdv) * (n + 1U)));
        }
        if (n > RSPI_SPBR_MAX_VAL)
        {
            /* Result out of range for the PCLK. */
            return 0;
        }
        else
        {
            /* Achieved bit rate. */
            bps_calc = (uint32_t) (clock_mhz / (2U * (0x0001U << n_brdv) * (n + 1U)));
        }
        spbr_result = (uint8_t) n;

        /* Apply the SPBR and SPCMDm.BRDV register values. */
        HW_RSPI_BitRateSet(p_ctrl->p_reg, spbr_result);
        HW_RSPI_BRDVSet(p_ctrl->p_reg, (uint16_t)n_brdv);
    }
    else
    {
        /* Result is out of range for the PCLK. */
        bps_calc = (uint32_t) 0;
    }
    /* Return the actual BPS rate achieved. */
    return bps_calc;
}/* End of function rspi_baud_set(). */

/*****************************************************************************************************************//**
 * @brief       This function gets the version information of the underlying driver.
 *
 * Implements spi_api_t::versionget
 *
 * @retval      void
 * @retval      SSP_SUCCESS            Successful version get.
 * @retval      SSP_ERR_ASSERTION      The parameter p_version is NULL.
 * @note        This function is reentrant.
 ********************************************************************************************************************/
ssp_err_t R_RSPI_VersionGet (ssp_version_t * p_version)
{
#if RSPI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_version != NULL);
#endif

    p_version->version_id = module_version.version_id;

    return SSP_SUCCESS;
}/* End of function R_RSPI_VersionGet(). */

/*****************************************************************************************************************//**
 * @} (end addtogroup SPI)
 ********************************************************************************************************************/

/*************************************************************************************************************//**
 * @brief   This function is the common ISR handler for SPTI and SPRI interrupts.
 *
 * @param[in]  p_ctrl       Channel control block to use.
 ***************************************************************************************************************/
static void rspi_tx_handler (rspi_instance_ctrl_t * p_ctrl)
{
    /* If there is still data to be transmitted. */
    if (p_ctrl->tx_count < p_ctrl->xfr_length)
    {
        SSP_CRITICAL_SECTION_DEFINE;
        uint32_t transfer = RSPI_DUMMY_TXDATA;
        if (p_ctrl->do_tx)
        {
            uint8_t * p_dest_8 = (uint8_t *) &transfer;
            uint8_t * p_src_8 = (uint8_t *) &((uint8_t *) p_ctrl->p_src)[p_ctrl->tx_count * p_ctrl->bytes_per_transfer];
            for (uint8_t i = 0U; i < p_ctrl->bytes_per_transfer; i++)
            {
                /* Default words access for 32bit/8bit and loading data into SPDR_HA when in swap mode.*/
                p_dest_8[i + p_ctrl->tx_handler_spdr_ha] = p_src_8[i];
            }
        }

        /* Enter critical section */
        SSP_CRITICAL_SECTION_ENTER;
        /* Clear the transmit complete status and transfer the next data. */
        R_BSP_IrqStatusClear(p_ctrl->tei_irq);
        NVIC_ClearPendingIRQ(p_ctrl->tei_irq);
        HW_RSPI_Write(p_ctrl->p_reg, transfer);
        /* Exit critical section */
        SSP_CRITICAL_SECTION_EXIT;
        p_ctrl->tx_count++;
    }
} /* End rspi_tx_handler(). */

/*************************************************************************************************************//**
 * @brief   This function is the common ISR handler for SPTI and SPRI interrupts.
 *
 * @param[in]  p_ctrl       Channel control block to use.
 ***************************************************************************************************************/
static void rspi_rx_handler (rspi_instance_ctrl_t * p_ctrl)
{
    if( p_ctrl->rx_count < p_ctrl->xfr_length )
    {
        uint8_t * p_src_8 = (uint8_t *) &(p_ctrl->rx_data);
        uint8_t * p_dest_8 = (uint8_t *) &((uint8_t *) p_ctrl->p_dest)[p_ctrl->rx_count*p_ctrl->bytes_per_transfer];
        for (uint32_t i = 0U; i < p_ctrl->bytes_per_transfer; i++)
        {
            p_dest_8[i] = p_src_8[i];
        }
        p_ctrl->rx_count++;
    }
} /* End rspi_rx_handler(). */

/*************************************************************************************************************//**
 * @brief   This function is the ISR function for RSPI idle interrupts.
 *          Each ISR calls a common function but passes its channel number.
 * @retval  void
 ***************************************************************************************************************/
void spi_tei_isr (void)
{
    SF_CONTEXT_SAVE

    /* Get the IRQ type. */
    IRQn_Type irq = R_SSP_CurrentIrqGet();

    /* Get the control block. */
    ssp_vector_info_t * p_vector_info;
    R_SSP_VectorInfoGet(irq, &p_vector_info);
    rspi_instance_ctrl_t * p_ctrl = (rspi_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    /* Disable interrupts. */
    HW_RSPI_InterruptDisable(p_ctrl->p_reg);

    NVIC_DisableIRQ(p_ctrl->tei_irq);

    /* Clear TXI interrupt status in ICU. */
    R_BSP_IrqStatusClear(irq);

    /* Transfer complete. Call the user callback function passing pointer to the result structure. */
    if ((NULL != p_ctrl->p_callback))
    {
        spi_callback_args_t rspi_cb_data;
        rspi_cb_data.channel            = p_ctrl->channel;
        rspi_cb_data.event              = SPI_EVENT_TRANSFER_COMPLETE;
        rspi_cb_data.p_context          = p_ctrl->p_context;
        p_ctrl->p_callback((spi_callback_args_t *) &(rspi_cb_data));
    }

    /* Transfer is done, release the lock for this operation. */
    R_BSP_SoftwareUnlock(&p_ctrl->resource_lock_tx_rx);

    SF_CONTEXT_RESTORE
} /* End spi_tei_isr. */

/*************************************************************************************************************//**
 * @brief   This function is the ISR function for RSPI receive buffer full (SPRI) interrupts.
 *          Each ISR calls a common function but passes its channel number.
 * @retval  void
 ***************************************************************************************************************/
void spi_rxi_isr (void)
{
    SF_CONTEXT_SAVE

    /* Clear TXI interrupt status in ICU. */
    IRQn_Type irq = R_SSP_CurrentIrqGet();
    R_BSP_IrqStatusClear(irq);

    ssp_vector_info_t * p_vector_info;
    R_SSP_VectorInfoGet(irq, &p_vector_info);
    rspi_instance_ctrl_t * p_ctrl = (rspi_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    if (!p_ctrl->using_dtc)
    {
        /* Read the received data. */
        p_ctrl->rx_data = HW_RSPI_Read(p_ctrl->p_reg);

        /* Transmit the next data. */
        rspi_tx_handler(p_ctrl);

        /* Process the received data. */
        rspi_rx_handler(p_ctrl);
    }
    else
    {
        p_ctrl->rx_count = p_ctrl->xfr_length;
    }


    if (p_ctrl->rx_count == p_ctrl->xfr_length)
    {
        /*If master mode or slave mode, enable and wait for transmit end ISR. */
        NVIC_EnableIRQ(p_ctrl->tei_irq);
    }
    SF_CONTEXT_RESTORE
} /* End spi_rxi_isr. */

/* End spi_rxi_isr. */
/*************************************************************************************************************//**
 * @brief   This is the ISR function for RSPI RSPI SPTI transmit buffer empty (SPTI) interrupts.
 *          Each ISR calls a common function but passes its channel number.
 * @retval  void
 ***************************************************************************************************************/
void spi_txi_isr (void)
{
    SF_CONTEXT_SAVE
    /* Clear RX interrupt status in ICU. */
    IRQn_Type irq = R_SSP_CurrentIrqGet();

    R_BSP_IrqStatusClear(irq);

    ssp_vector_info_t * p_vector_info;
    R_SSP_VectorInfoGet(irq, &p_vector_info);
    rspi_instance_ctrl_t * p_ctrl = (rspi_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    /* If master mode then disable further SPTI interrupts on first transmit.
     * If slave mode then we do two transmits to fill the double buffer, then disable SPTI interrupts.
     * The receive interrupt will handle any remaining data. Refer to the Jama functional design description
     * for more information. */
    if (!p_ctrl->using_dtc)
    {
        if (((HW_RSPI_MasterModeCheck(p_ctrl->p_reg)) || (p_ctrl->tx_count > 0U)) && (SPI_OPERATION_DO_TX != p_ctrl->transfer_mode))
        {
            HW_RSPI_TxIntrDisable(p_ctrl->p_reg);
        }
        rspi_tx_handler(p_ctrl);
    }
    else
    {
        /* Set transfer count to size of DTC transfer. */
        p_ctrl->tx_count = p_ctrl->xfr_length;
    }

    /* Check if the last data has been transmitted */
    if (p_ctrl->tx_count == p_ctrl->xfr_length)
    {
        /* Disable transmit buffer empty interrupt in NVIC to prevent any pending interrupts from firing. */
        HW_RSPI_TxIntrDisable(p_ctrl->p_reg);
        NVIC_DisableIRQ(p_ctrl->txi_irq);
        R_BSP_IrqStatusClear(p_ctrl->txi_irq);
        NVIC_ClearPendingIRQ(p_ctrl->txi_irq);


        /* In master mode, the RX interrupt will handle the transaction completion. If in TX only mode, we need
         * to enable the idle interrupt here to handle the transaction completion. */
        if (SPI_OPERATION_DO_TX == p_ctrl->transfer_mode)
        {
            /* Simply wait for the idle interrupt to complete the transaction. */
            NVIC_EnableIRQ(p_ctrl->tei_irq);
        }
    }
    SF_CONTEXT_RESTORE
} /* End spi_txi_isr. */

/*************************************************************************************************************//**
 * @brief       This function is common ISR handler for  RSPI SPEI-error interrupts.
 *
 * @param[in]   p_ctrl  Channel control block to use.
 * @retval      void
 ***************************************************************************************************************/
static void rspi_spei_isr_common (rspi_instance_ctrl_t * p_ctrl)
{
    uint8_t channel = p_ctrl->channel;
    uint8_t status_flags = HW_RSPI_Status(p_ctrl->p_reg);
    spi_callback_args_t rspi_cb_data;

    /* Identify and clear error condition. */
    if (status_flags & RSPI_SPSR_OVRF)
    {
        /* Overrun error occurred. */
        rspi_cb_data.event = SPI_EVENT_ERR_OVERRUN;

        /* Clear error source: Overrun flag. */
        HW_RSPI_OverRunErrorClear(p_ctrl->p_reg);
    }
    else if (status_flags & RSPI_SPSR_MODF)
    {
        /* If Mode Flag is set and Underflow flag is also set, then its an underrun error*/
        if (status_flags & RSPI_SPSR_UDRF)
        {
            rspi_cb_data.event = SPI_EVENT_ERR_MODE_UNDERRUN;

            /* Clear error source: Underflow error flag. */
            HW_RSPI_UnderflowErrorClear(p_ctrl->p_reg);
        }
        /* If Mode Flag is set and Underflow flag is not set, then its a mode fault*/
        else
        {
            rspi_cb_data.event = SPI_EVENT_ERR_MODE_FAULT;
        }

        /* Clear error source: Mode fault error flag. */
        HW_RSPI_ModeFaultErrorClear(p_ctrl->p_reg);
    }
    else if (status_flags & RSPI_SPSR_PERF)
    {
        rspi_cb_data.event = SPI_EVENT_ERR_PARITY;

        /* Clear error source: parity error flag. */
        HW_RSPI_ParityErrorrClear(p_ctrl->p_reg);
    }
    else
    {
        /* Set transfer abort event as default, don't leave the event unassigned*/
        rspi_cb_data.event = SPI_EVENT_TRANSFER_ABORTED;
    }

    /* Disable all interrupts. */
    NVIC_DisableIRQ(p_ctrl->tei_irq);
    HW_RSPI_InterruptDisable(p_ctrl->p_reg);

    /* Call the user callback function passing pointer to the result structure. */
    if (NULL != p_ctrl->p_callback)
    {
        rspi_cb_data.channel = channel;
        rspi_cb_data.p_context = p_ctrl->p_context;
        p_ctrl->p_callback((spi_callback_args_t *) &(rspi_cb_data));
    }

    /* Error condition occurs, release the software lock for this operation. */
    R_BSP_SoftwareUnlock(&p_ctrl->resource_lock_tx_rx);
} /* End rspi_spei_isr_common(). */

/*************************************************************************************************************//**
 * @brief   This function is the ISR function for RSPI error (SPEI) interrupts.
 *          Each ISR calls a common function but passes its channel number.
 * @retval  void
 ***************************************************************************************************************/
void spi_eri_isr (void)
{
    SF_CONTEXT_SAVE

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    rspi_instance_ctrl_t * p_ctrl = (rspi_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    rspi_spei_isr_common(p_ctrl);

    /* Clear ERI interrupt status in ICU. */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    SF_CONTEXT_RESTORE
} /* End of spi_eri_isr. */

/*************************************************************************************************************//**
 * @brief       This function sets the common configuration for a SPI channel.
 * @param[in]   p_ctrl  Pointer to the control block for the channel.
 * @param[in]   p_cfg   Pointer to SPI configuration structure.
 * @retval      void
 ***************************************************************************************************************/
static void rspi_common_config_set (rspi_instance_ctrl_t * p_ctrl, spi_cfg_t  const * const p_cfg)
{
    /* Process the user configuration to update the local configuration image. */
    /* Set SPCR-MSTR -- set control register-master/slave mode. */
    /* Set SPI operating mode to master or slave. True-master; false-slave. */
    HW_RSPI_OperatingModeMaster(p_ctrl->p_reg, (SPI_MODE_MASTER == p_cfg->operating_mode));

    /* Set SPCMD0-CPHA bit -- set RSPCK phase. */
    /* Set clock phase. True-even, false-odd. */
    HW_RSPI_ClockPhaseEven(p_ctrl->p_reg, (SPI_CLK_PHASE_EDGE_EVEN == p_cfg->clk_phase));

    /* Set SPCMD0-CPOL bit -- set RSPCK polarity. */
    /* Set clock polarity. True-high at idle, false-low at idle. */
    HW_RSPI_ClockPolarityHigh(p_ctrl->p_reg, (SPI_CLK_POLARITY_HIGH == p_cfg->clk_polarity));

    /* Set SPCR-MODFEN bit -- set control register mode fault error detection enable. */
    /* Set mode fault detection. True-on; false-off. */
    HW_RSPI_ModeFaultDetectionOn(p_ctrl->p_reg, (SPI_MODE_FAULT_ERROR_ENABLE == p_cfg->mode_fault));

    /* Set SPCMD0-LSBF bit -- set command register0 LSB/MSB first mode. */
    /* Set Bit order to MSB or LSB. True-LSB; false-MSB. */
    HW_RSPI_BitOrderLSB(p_ctrl->p_reg, (SPI_BIT_ORDER_LSB_FIRST == p_cfg->bit_order));

    /* If master mode is set enable RSPCK auto stop function. */
    if(SPI_MODE_MASTER == p_cfg->operating_mode)
    {
        /* Set the SPCR2-SCKASE bit -- set RSPCK auto stop function. */
        /* Enable RSPI clock auto stop function. */
        HW_RSPI_RSPCKAutoStop(p_ctrl->p_reg);
    }

    /* If using DTC */
    if (p_ctrl->using_dtc)
    {
        /* Configure the appropriate data and access length. */
        if (p_ctrl->p_transfer_tx->p_cfg->p_info->size == TRANSFER_SIZE_1_BYTE)
        {
            HW_RSPI_DataBitLength(p_ctrl->p_reg, RSPI_SPCMD_SPB_8BIT);
            HW_RSPI_DefaultDataBitLength(p_ctrl->p_reg);
        }
        else if (p_ctrl->p_transfer_tx->p_cfg->p_info->size == TRANSFER_SIZE_2_BYTE)
        {
            HW_RSPI_DataBitLength(p_ctrl->p_reg, RSPI_SPCMD_SPB_16BIT);
            HW_RSPI_DataBitLength_HalfWord(p_ctrl->p_reg);
        }
        else
        {
            HW_RSPI_DataBitLength(p_ctrl->p_reg, RSPI_SPCMD_SPB_32BIT);
            HW_RSPI_DefaultDataBitLength(p_ctrl->p_reg);
        }
    }
    else  /* Otherwise set default to 32 bits access. */
    {
        HW_RSPI_DefaultDataBitLength(p_ctrl->p_reg);
    }

} /* End of rspi_common_config_set(). */

/*************************************************************************************************************//**
 * @brief       This function sets the extended configuration for a SPI channel.
 * @param[in]   p_ctrl  Pointer to the control block for the channel.
 * @param[in]   p_cfg   Pointer to SPI configuration structure.
 * @retval      void
 ***************************************************************************************************************/
static void rspi_extended_config_set (rspi_instance_ctrl_t * p_ctrl, spi_cfg_t  const * const p_cfg)
{
    if (NULL == p_cfg->p_extend)
    {
        return;
    }
    /* Process the user extended configuration to update the local configuration image. */
    spi_on_rspi_cfg_t * p_rspi_cfg = 0U;
    p_rspi_cfg = (spi_on_rspi_cfg_t *) p_cfg->p_extend;

    /* Set clock and communication modes. */
    /* Set SPCR-SPMS bit -- set RSPI SPI or Clock synchronous mode. */
    HW_RSPI_OperationClkSyn(p_ctrl->p_reg, (RSPI_OPERATION_CLK_SYN == p_rspi_cfg->rspi_clksyn));

    /* Sets the slave select polarity level. */
    HW_RSPI_SlaveSelectPolarity(p_ctrl->p_reg, RSPI_SSL_SELECT_SSL0,
            (RSPI_SSLP_HIGH == p_rspi_cfg->ssl_polarity.rspi_ssl0));

    /* Set loop back mode. */
    /* Set SPPCR-SPLP bit -- set loopback mode with inverted data. */
    HW_RSPI_loopback1(p_ctrl->p_reg, (RSPI_LOOPBACK1_INVERTED_DATA == p_rspi_cfg->loopback.rspi_loopback1));

    /* Set SPPCR-SPLP2 bit -- set loopback2 mode with not inverted data. */
    HW_RSPI_loopback2(p_ctrl->p_reg, (RSPI_LOOPBACK2_NOT_INVERTED_DATA == p_rspi_cfg->loopback.rspi_loopback2));

    /* Set SPPCR-MOIFV bit -- set mosi idle fixed value. */
    HW_RSPI_MOSIIdleLevelHigh(p_ctrl->p_reg,
            (RSPI_MOSI_IDLE_FIXED_VAL_HIGH == p_rspi_cfg->mosi_idle.rspi_mosi_idle_fixed_val));

    /* Set SPPCR-MOIFE bit -- set mosi idle value fixing enable. */
    HW_RSPI_MOSIIdleEnable(p_ctrl->p_reg,
            (RSPI_MOSI_IDLE_VAL_FIXING_ENABLE == p_rspi_cfg->mosi_idle.rspi_mosi_idle_val_fixing));

    /* Set SPCR2-SPPE bit -- enable/disable parity. */
    HW_RSPI_ParityEnable(p_ctrl->p_reg,
            (RSPI_PARITY_STATE_ENABLE == p_rspi_cfg->parity.rspi_parity));

    /* Set SPCR2-SPOE bit -- select even/odd parity. */
    HW_RSPI_ParityOdd(p_ctrl->p_reg,
            (RSPI_PARITY_MODE_ODD == p_rspi_cfg->parity.rspi_parity_mode));

    /* Set SPCMD0-SSLA bits -- select SSL signal assertion setting. */
    HW_RSPI_SlaveSelect(p_ctrl->p_reg, p_rspi_cfg->ssl_select);

    bsp_feature_rspi_t rspi_feature = {0};

    R_BSP_FeatureRspiGet(&rspi_feature);
    if (1U == rspi_feature.has_ssl_level_keep)
    {
        /* Set SPCMD0-SSLKP bits -- set SSL signal level keeping. */
        HW_RSPI_SlaveSelectLevelKeep(p_ctrl->p_reg, (RSPI_SSL_LEVEL_KEEP == p_rspi_cfg->ssl_level_keep));
    }

    /* Enable byte swapping for S5 series MCU's */
    if (1U == rspi_feature.swap)
    {
        p_ctrl->byte_swap = p_rspi_cfg->byte_swap;
    }
    else /* Disable byte swapping */
    {
        p_ctrl->byte_swap = RSPI_BYTE_SWAP_DISABLE;
    }

    /* Set SPCMD0-SCKDEN bit & SPCKD register -- set RSPI RSPCK delay enable and clock delay register. */
    if (RSPI_CLOCK_DELAY_STATE_ENABLE == p_rspi_cfg->clock_delay.rspi_clock_delay_state)
    {
        HW_RSPI_ClockDelay(p_ctrl->p_reg, true, p_rspi_cfg->clock_delay.rspi_clock_delay_count);
    }

    /* Set SPCMD0-SLNDEN bit & SSLND register -- RSPI SSL negation delay and slave select negation delay register. */
    if (RSPI_SSL_NEGATION_DELAY_ENABLE == p_rspi_cfg->ssl_neg_delay.rspi_ssl_neg_delay_state)
    {
        HW_RSPI_SlaveSelectNegationDelay(p_ctrl->p_reg, true, p_rspi_cfg->ssl_neg_delay.rspi_ssl_neg_delay_count);
    }

    /* Set SPCMD0-SPNDEN bit & SPND register -- set RSPI next access delay enable and next-access delay register. */
    if (RSPI_NEXT_ACCESS_DELAY_STATE_ENABLE == p_rspi_cfg->access_delay.rspi_next_access_delay_state)
    {
        HW_RSPI_NextAccessDelay(p_ctrl->p_reg, true, p_rspi_cfg->access_delay.rspi_next_access_delay_count);
    }
} /* End of rspi_extended_config_set. */

/*************************************************************************************************************//**
 * @brief       This function sets the necessary default configuration for a SPI channel.
 * @param[in]   p_ctrl  Pointer to SPI control structure.
 * @retval      void
 ***************************************************************************************************************/
static void rspi_default_config_set (rspi_instance_ctrl_t * p_ctrl)
{
    /* Clear SPCMD0 command register and the Data Control register. */
    HW_RSPI_CommandClear(p_ctrl->p_reg);
    HW_RSPI_DataControlClear(p_ctrl->p_reg);

    /* Set default data frames to use 1 frame for transmission. */
    HW_RSPI_DefaultDataFrame(p_ctrl->p_reg);

    /* Default Setting - Set maximum clock rate (BRDV=0), user does not have the option to
     * configure this parameter from the configuration file. */
    HW_RSPI_DefaultBRDV(p_ctrl->p_reg);

    /*  Default Setting - set SPSCR (sequence control register) to use 1 sequence.
     *  We don't support multiple sequences at this time. */
    HW_RSPI_DefaultSequence(p_ctrl->p_reg);
} /* End of rspi_default_config_set. */

/******************************************************************************************************************//**
 * @brief Configures RSPI related transfer drivers (if enabled).
 * @param[in]   p_cfg                       Pointer to RSPI specific configuration structure
 * @retval      SSP_SUCCESS                 Successfully configured driver
 * @retval      SSP_ERR_INVALID_ARGUMENT    Both or neither transfer instances must be used
 * @return                                  See @ref Common_Error_Codes or functions called by this function
 *                                          for other possible return codes. This function calls:
 *                                          * transfer_api_t::open
 *********************************************************************************************************************/
static ssp_err_t rspi_transfer_open (spi_cfg_t const * const p_cfg)
{
    ssp_err_t result = SSP_SUCCESS;
#if (RSPI_CFG_PARAM_CHECKING_ENABLE)
    RSPI_ERROR_RETURN((p_cfg->p_transfer_rx==NULL)==(p_cfg->p_transfer_tx==NULL), SSP_ERR_INVALID_ARGUMENT);
#endif

    if (p_cfg->p_transfer_tx && p_cfg->p_transfer_rx)
    {
#if (RSPI_CFG_PARAM_CHECKING_ENABLE)
        result = rspi_parameter_check(p_cfg->p_transfer_tx);
        RSPI_ERROR_RETURN((SSP_SUCCESS == result), result);
        result = rspi_parameter_check(p_cfg->p_transfer_rx);
        RSPI_ERROR_RETURN((SSP_SUCCESS == result), result);
#endif

        /* Set default transfer info and open receive transfer module, if enabled. */
        transfer_cfg_t cfg = *(p_cfg->p_transfer_rx->p_cfg);
        cfg.activation_source = rspi_rxi_event_lookup((uint32_t) p_cfg->channel);
        cfg.auto_enable = false;
        cfg.p_callback  = NULL;
        result = p_cfg->p_transfer_rx->p_api->open(p_cfg->p_transfer_rx->p_ctrl, &cfg);
        RSPI_ERROR_RETURN((SSP_SUCCESS == result), result);

        cfg = *(p_cfg->p_transfer_tx->p_cfg);
        cfg.activation_source = rspi_txi_event_lookup((uint32_t) p_cfg->channel);
        cfg.auto_enable = false;
        cfg.p_callback  = NULL;
        result = p_cfg->p_transfer_tx->p_api->open(p_cfg->p_transfer_tx->p_ctrl, &cfg);
        if (SSP_SUCCESS != result)
        {
            p_cfg->p_transfer_rx->p_api->close(p_cfg->p_transfer_rx->p_ctrl);
        }
    }

    return result;
} /* End of function rspi_transfer_open(). */

/*******************************************************************************************************************//**
 * @brief Closes transfer instances associated with the RSPI instance
 * @param[in]   p_cfg       Pointer to RSPI specific configuration structure
 * @retval      void
 *********************************************************************************************************************/
static void rspi_transfer_close (spi_cfg_t const * const p_cfg)
{
    if (p_cfg->p_transfer_tx && p_cfg->p_transfer_rx)
    {
        /* Set default transfer info and open receive transfer module, if enabled. */
        p_cfg->p_transfer_rx->p_api->close(p_cfg->p_transfer_rx->p_ctrl);
        p_cfg->p_transfer_tx->p_api->close(p_cfg->p_transfer_tx->p_ctrl);
    }
} /* End of function rspi_transfer_close(). */

/******************************************************************************************************************//**
 * Configure interrupts.
 * @param[in]     p_feature                 SSP feature
 * @param[in]     signal                    SSP signal ID
 * @param[in]     ipl                       Interrupt priority level
 * @param[in]     p_ctrl                    Pointer to driver control block
 * @param[out]    p_irq                     Pointer to IRQ for this signal, set here
 *
 * @retval        SSP_SUCCESS               Interrupt enabled
 * @return                                  See @ref Common_Error_Codes or functions called by this function
 *                                          for other possible return codes. This function calls:
 *                                          * fmi_api_t::eventInfoGet
 *********************************************************************************************************************/
static ssp_err_t rspi_irq_cfg (ssp_feature_t * p_feature,
                               ssp_signal_t    signal,
                               uint8_t         ipl,
                               void          * p_ctrl,
                               IRQn_Type     * p_irq)
{
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    ssp_vector_info_t * p_vector_info;
    ssp_err_t err = g_fmi_on_fmi.eventInfoGet(p_feature, signal, &event_info);
    *p_irq = event_info.irq;
    if (SSP_SUCCESS == err)
    {
        NVIC_DisableIRQ(*p_irq);
        NVIC_ClearPendingIRQ(*p_irq);
        NVIC_SetPriority(*p_irq, ipl);
        R_SSP_VectorInfoGet(*p_irq, &p_vector_info);
        *(p_vector_info->pp_ctrl) = p_ctrl;
    }

    return err;
} /* End of function rspi_irq_cfg(). */

/*******************************************************************************************************************//**
 * Sets interrupt priority and initializes vector info
 * @param[in]     ssp_feature               SSP feature
 * @param[in]     p_ctrl                    Pointer to driver control block
 * @param[in]     p_cfg                     Pointer to SPI configuration structure.
 *
 * @retval        SSP_SUCCESS               Interrupts configured and driver started.
 *********************************************************************************************************************/
static ssp_err_t rspi_initialize_hardware (ssp_feature_t              *       ssp_feature,
                                           rspi_instance_ctrl_t       *       p_ctrl,
                                           spi_cfg_t            const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;

    /* Configure the receive ISR. */
    err = rspi_irq_cfg(ssp_feature, SSP_SIGNAL_SPI_RXI, p_cfg->rxi_ipl, p_ctrl, &p_ctrl->rxi_irq);
    if (SSP_SUCCESS == err)
    {
        /* Configure the transmit ISR. */
        err = rspi_irq_cfg(ssp_feature, SSP_SIGNAL_SPI_TXI, p_cfg->txi_ipl, p_ctrl, &p_ctrl->txi_irq);
    }
    if (SSP_SUCCESS == err)
    {
        /* Configure the error ISR. */
        err = rspi_irq_cfg(ssp_feature, SSP_SIGNAL_SPI_ERI, p_cfg->eri_ipl, p_ctrl, &p_ctrl->eri_irq);
    }
    if (SSP_SUCCESS == err)
    {
        /* Configure the idle ISR. */
        err = rspi_irq_cfg(ssp_feature, SSP_SIGNAL_SPI_TEI, p_cfg->tei_ipl, p_ctrl, &p_ctrl->tei_irq);
    }
    if (SSP_SUCCESS == err)
    {
        /* Configure the transfer instances. */
        err = rspi_transfer_open(p_cfg);
    }
    if (SSP_SUCCESS == err)
    {
        /* Start the hardware module. */
        err = R_BSP_ModuleStart(ssp_feature);
    }
    if (SSP_SUCCESS != err)
    {
        SSP_ERROR_LOG((err), (&g_module_name[0]), (&module_version));
    }

    return (err);
} /* End of function rspi_initialize_hardware(). */

/*******************************************************************************************************************//**
 * This function resets the TX transfer interface.
 * @param[in]   p_ctrl              Pointer to driver control block
 * @param[in]   tx_rx_mode          Current transmit receive mode
 * @param[in]   p_data_reg          Data register of the module
 * @param[in]   length              Length of data to be transmitted/received
 *
 * @retval      SSP_SUCCESS         Transaction successful or Transfer interface not configured
 * @return                  See @ref Common_Error_Codes or functions called by this function
 *                          for other possible return codes. This function calls:
 *                          * transfer_api_t::close
 *                          * transfer_api_t::open
 *                          * transfer_api_t::reset
 *********************************************************************************************************************/
static ssp_err_t rspi_tx_transfer_reset (rspi_instance_ctrl_t * const p_ctrl,
                                         spi_operation_t              tx_rx_mode,
                                         uint32_t             *       p_data_reg,
                                         uint32_t               const length )
{
    ssp_err_t result = SSP_SUCCESS;
    transfer_addr_mode_t addr_mode = TRANSFER_ADDR_MODE_INCREMENTED;

    /* If the current mode is receive only */
    if (tx_rx_mode == SPI_OPERATION_DO_RX)
    {
        /* Set the transmit location to a fixed source location. */
        p_ctrl->p_src = (void *)&g_dummy;
        addr_mode = TRANSFER_ADDR_MODE_FIXED;
    }

    /* If the mode has changed or DTC is configured for 8bit chained mode it needs reset. */
    if ((TRANSFER_CHAIN_MODE_EACH == p_ctrl->p_transfer_tx->p_cfg->p_info->chain_mode) ||
        (p_ctrl->tx_dtc_addr_mode != addr_mode))
    {
        /* Close the transfer instance to reconfigure it. */
        result = p_ctrl->p_transfer_tx->p_api->disable(p_ctrl->p_transfer_tx->p_ctrl);
        RSPI_ERROR_RETURN((SSP_SUCCESS == result), result);

        /* Reset the transfer config block with provided addresses and length. */
        p_ctrl->p_transfer_tx->p_cfg->p_info[0].src_addr_mode = addr_mode;
        p_ctrl->p_transfer_tx->p_cfg->p_info[0].p_src = p_ctrl->p_src;
        p_ctrl->p_transfer_tx->p_cfg->p_info[0].length = (uint16_t)length;

        /* If the transfer instance is chained reset the 2nd transfer config block. */
        if (TRANSFER_CHAIN_MODE_EACH == p_ctrl->p_transfer_tx->p_cfg->p_info->chain_mode)
        {
            p_ctrl->p_transfer_tx->p_cfg->p_info[1].p_dest = p_data_reg;
            p_ctrl->p_transfer_tx->p_cfg->p_info[1].length = (uint16_t)length;
        }
        else
        {
            p_ctrl->p_transfer_tx->p_cfg->p_info[0].p_dest = p_data_reg;
        }
        result = p_ctrl->p_transfer_tx->p_api->enable(p_ctrl->p_transfer_tx->p_ctrl);
        p_ctrl->tx_dtc_addr_mode = addr_mode;
    }
    else
    {
    	/* Set source address mode to incremented mode for SPI transmission only,
    	  and transmission/reception operation */
    	 p_ctrl->p_transfer_tx->p_cfg->p_info->src_addr_mode = addr_mode;
        /* Reset the transfer instance if it does not need reconfigured. */
        result = p_ctrl->p_transfer_tx->p_api->reset(p_ctrl->p_transfer_tx->p_ctrl,
                                                 p_ctrl->p_src,
                                                 (void *)p_data_reg,
                                                 (uint16_t) length);
    }
    return result;
} /* End of function rspi_tx_transfer_reset(). */

/*******************************************************************************************************************//**
 * This function resets the RX transfer interface.
 * @param[in]   p_ctrl              Pointer to driver control block
 * @param[in]   p_data_reg          Data register of the module
 * @param[in]   length              Length of data to be transmitted/received
 *
 * @retval      SSP_SUCCESS         Transaction successful or Transfer interface not configured
 * @return                  See @ref Common_Error_Codes or functions called by this function
 *                          for other possible return codes. This function calls:
 *                          * transfer_api_t::close
 *                          * transfer_api_t::open
 *                          * transfer_api_t::reset
 *********************************************************************************************************************/
static ssp_err_t rspi_rx_transfer_reset (rspi_instance_ctrl_t * const p_ctrl,
                                         uint32_t             *       p_data_reg,
                                         uint32_t               const length )
{
    ssp_err_t result = SSP_SUCCESS;

    /* If the DTC is configured for 8bit chained mode it needs reset. */
    if (TRANSFER_CHAIN_MODE_EACH == p_ctrl->p_transfer_rx->p_cfg->p_info->chain_mode)
    {
        /* Close the transfer instance to reconfigure it. */
        result = p_ctrl->p_transfer_rx->p_api->disable(p_ctrl->p_transfer_rx->p_ctrl);
        RSPI_ERROR_RETURN((SSP_SUCCESS == result), result);

        /* Reset the transfer config block with provided addresses and length. */
        p_ctrl->p_transfer_rx->p_cfg->p_info[0].p_src = p_data_reg;
        p_ctrl->p_transfer_rx->p_cfg->p_info[0].length = (uint16_t)length;

        /* Since the transfer instance is chained, reset the 2nd transfer config block. */

        p_ctrl->p_transfer_rx->p_cfg->p_info[1].p_dest = p_ctrl->p_dest;
        p_ctrl->p_transfer_rx->p_cfg->p_info[1].length = (uint16_t)length;

        result = p_ctrl->p_transfer_rx->p_api->enable(p_ctrl->p_transfer_rx->p_ctrl);
    }
    else
    {
        /* Reset the transfer instance if it does not need reconfigured. */
        result = p_ctrl->p_transfer_rx->p_api->reset(p_ctrl->p_transfer_rx->p_ctrl,
                                                     (void const *)p_data_reg,
                                                     (void *)p_ctrl->p_dest,
                                                     (uint16_t) length);
    }
    return result;
} /* End of function rspi_rx_transfer_reset(). */

/*******************************************************************************************************************//**
 * This function tries RX/TX using transfer interface, if configured, else returns success anyway for CPU to do it.
 * @param[in]   p_ctrl              Pointer to driver control block
 * @param[in ]  tx_rx_mode          Bit width value to set
 * @param[in ]  length              Bit width value to set
 *
 * @retval      SSP_ERR_ASSERTION   Invalid argument for transfer device.
 * @retval      SSP_ERR_NOT_OPEN    Invalid transfer device handle.
 * @retval      SSP_ERR_NOT_ENABLED Invalid transaction parameter
 * @retval      SSP_SUCCESS         Transaction successful or Transfer interface not configured
 *********************************************************************************************************************/
static ssp_err_t rspi_transfer_reset (rspi_instance_ctrl_t * const p_ctrl,
                                      spi_operation_t              tx_rx_mode,
                                      uint32_t               const length )
{
#if (RSPI_CFG_PARAM_CHECKING_ENABLE)
        RSPI_ERROR_RETURN(length <= 0xFFFFU, SSP_ERR_ASSERTION);
#endif
    ssp_err_t result = SSP_SUCCESS;

    /* Get the address of the data register. */
    uint32_t * p_data_reg = (uint32_t *) HW_RSPI_WriteReadAddrGet(p_ctrl->p_reg);
    if (NULL != p_ctrl->p_transfer_tx)
    {
        /* Reset the transmit transfer instance. */
        result = rspi_tx_transfer_reset(p_ctrl, tx_rx_mode, p_data_reg, length);
        RSPI_ERROR_RETURN((SSP_SUCCESS == result), result);
    }

    if ((NULL != p_ctrl->p_transfer_rx) && (SPI_OPERATION_DO_TX != tx_rx_mode))
    {
        if(SPI_OPERATION_DO_TX_RX == tx_rx_mode)
        {
            /* Restore source address mode to default mode if transmit and receive operation is to be performed. */
            p_ctrl->p_transfer_tx->p_cfg->p_info->src_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED;
        }
        else
        {
            /* Set source address mode to fixed mode if receive operation is to be performed. */
            p_ctrl->p_transfer_tx->p_cfg->p_info->src_addr_mode = TRANSFER_ADDR_MODE_FIXED;
        }

        /* Reset the receive transfer instance. */
        result = rspi_rx_transfer_reset(p_ctrl, p_data_reg, length);
        RSPI_ERROR_RETURN((SSP_SUCCESS == result), result);
    }
    return SSP_SUCCESS;
} /* End of function rspi_transfer_reset(). */

/*******************************************************************************************************************//**
 * Update bit width for current transaction
 * @param[in]   p_ctrl          Pointer to driver control block
 * @param[in ]  bit_width       Bit width value to set
 *
 * @retval      void
 *********************************************************************************************************************/
static void rspi_update_transaction_bit_width (rspi_instance_ctrl_t * const p_ctrl, spi_bit_width_t bit_width)
{
    /* Set default configuration. */
    /* Update the SPCMD0 command register with the setting for 32 bit width. */
    HW_RSPI_DataBitLength(p_ctrl->p_reg, RSPI_SPCMD_SPB_32BIT);
    HW_RSPI_DefaultDataBitLength(p_ctrl->p_reg);
    p_ctrl->tx_handler_spdr_ha = 0U;

    /* Convert spi_bit_width_t to equivalent value required by hardware. */
    if (bit_width == SPI_BIT_WIDTH_8_BITS)
    {
        /* Update the SPCMD0 command register with the setting for 8 bit width. */
        HW_RSPI_DataBitLength(p_ctrl->p_reg, RSPI_SPCMD_SPB_8BIT);
    }
    else if (bit_width == SPI_BIT_WIDTH_16_BITS)
    {
        /* Update the SPCMD0 command register with the setting for 16 bit width. */
        HW_RSPI_DataBitLength(p_ctrl->p_reg, RSPI_SPCMD_SPB_16BIT);
        /* Active when swap is enabled */
        if(p_ctrl->byte_swap == RSPI_BYTE_SWAP_ENABLE)
        {
            HW_RSPI_DataBitLength_HalfWord(p_ctrl->p_reg);
            p_ctrl->tx_handler_spdr_ha = RSPI_WORD_DATA;
        }
    }
} /* End of function update_transaction_bit_width(). */

#if (RSPI_CFG_PARAM_CHECKING_ENABLE)
/*******************************************************************************************************************//**
 * Checks for valid parameters.
 * @param[in]     p_tx_rx           Pointer to driver control block.
 * @retval        SSP_SUCCESS       Valid parameters are provided.
 * @retval        SSP_ERR_ASSERTION NULL value for any of the p_tx_rx members p_api, p_ctrl, p_cfg or p_cfg::p_info
 *********************************************************************************************************************/
static ssp_err_t rspi_parameter_check (transfer_instance_t const *p_tx_rx)
{
    SSP_ASSERT(NULL != p_tx_rx->p_api);
    SSP_ASSERT(NULL != p_tx_rx->p_ctrl);
    SSP_ASSERT(NULL != p_tx_rx->p_cfg);
    SSP_ASSERT(NULL != p_tx_rx->p_cfg->p_info);
    return SSP_SUCCESS;
} /* End of function rspi_parameter_check(). */
#endif


/******************************************************************************************************************//**
 * @brief Reconfigures RSPI related transfer drivers after reconfiguring transfer_info.
 * @param[in]   p_ctrl                      Pointer to RSPI specific Control structure
 * @retval      SSP_SUCCESS                 Successfully configured driver
 * @return                                  See @ref Common_Error_Codes or functions called by this function
 *                                          for other possible return codes. This function calls:
 *                                          * transfer_api_t::open
 *********************************************************************************************************************/
static ssp_err_t rspi_transfer_reopen (rspi_instance_ctrl_t      * const p_ctrl)
{
    ssp_err_t result = SSP_SUCCESS;

    /* Set default transfer info and open receive transfer module, if enabled. */
    transfer_cfg_t cfg = *(p_ctrl->p_transfer_rx->p_cfg);
    cfg.activation_source = rspi_rxi_event_lookup((uint32_t) p_ctrl->channel);
    cfg.auto_enable = false;
    cfg.p_callback  = NULL;
    result = p_ctrl->p_transfer_rx->p_api->open(p_ctrl->p_transfer_rx->p_ctrl, &cfg);
    RSPI_ERROR_RETURN((SSP_SUCCESS == result), result);

    cfg = *(p_ctrl->p_transfer_tx->p_cfg);
    cfg.activation_source = rspi_txi_event_lookup((uint32_t) p_ctrl->channel);
    cfg.auto_enable = false;
    cfg.p_callback  = NULL;
    result = p_ctrl->p_transfer_tx->p_api->open(p_ctrl->p_transfer_tx->p_ctrl, &cfg);
    if (SSP_SUCCESS != result)
    {
        p_ctrl->p_transfer_rx->p_api->close(p_ctrl->p_transfer_rx->p_ctrl);
    }

    return result;
} /* End of function rspi_transfer_reopen(). */

/*****************************************************************************************************************//**
 * @brief Closes transfer instance and reconfigures transfer instance structure based on bit width
 * @param[in] p_ctrl Pointer to the control block..
 * @param[in] bit_width    Size of data for each transaction.
 * @note      the parameter check must be held by HLD
 ********************************************************************************************************************/
static void rspi_dtc_close_reconfig_p_info(rspi_instance_ctrl_t            * const p_ctrl,
                                     spi_bit_width_t const         bit_width)
{
    static uint32_t tx_dummy_address = 0U;
    static uint32_t rx_dummy_address = 0U;

    if (p_ctrl->p_transfer_tx && p_ctrl->p_transfer_rx)
    {
        /* Set default transfer info and open receive transfer module, if enabled. */
        p_ctrl->p_transfer_rx->p_api->close(p_ctrl->p_transfer_rx->p_ctrl);
        p_ctrl->p_transfer_tx->p_api->close(p_ctrl->p_transfer_tx->p_ctrl);

        switch(bit_width)
        {
            case SPI_BIT_WIDTH_8_BITS:
                /*Configures 8bit structure for 8bit transfer */
                p_ctrl->p_transfer_tx->p_cfg->p_info->chain_mode = TRANSFER_CHAIN_MODE_EACH;;
                p_ctrl->p_transfer_tx->p_cfg->p_info[1].p_src  = (void const *)&tx_dummy_address;
                p_ctrl->p_transfer_tx->p_cfg->p_info[0].size = TRANSFER_SIZE_1_BYTE;
                p_ctrl->p_transfer_tx->p_cfg->p_info[0].p_dest = (void *)&tx_dummy_address;

                p_ctrl->p_transfer_rx->p_cfg->p_info->chain_mode = TRANSFER_CHAIN_MODE_EACH;
                p_ctrl->p_transfer_rx->p_cfg->p_info[0].p_dest = (void *)&rx_dummy_address;
                p_ctrl->p_transfer_rx->p_cfg->p_info[1].p_src  = (void const *)&rx_dummy_address;
                p_ctrl->p_transfer_rx->p_cfg->p_info->size = TRANSFER_SIZE_4_BYTE;
                p_ctrl->p_transfer_rx->p_cfg->p_info->repeat_area = TRANSFER_REPEAT_AREA_SOURCE;
                p_ctrl->p_transfer_rx->p_cfg->p_info->dest_addr_mode =  TRANSFER_ADDR_MODE_FIXED;

                /* Update the SPCMD0 command register with the setting for 8 bit width. */
                HW_RSPI_DataBitLength(p_ctrl->p_reg, RSPI_SPCMD_SPB_8BIT);
                HW_RSPI_DefaultDataBitLength(p_ctrl->p_reg);
            break;

            case SPI_BIT_WIDTH_16_BITS:
                p_ctrl->p_transfer_tx->p_cfg->p_info->size = TRANSFER_SIZE_2_BYTE;
                p_ctrl->p_transfer_rx->p_cfg->p_info->size = TRANSFER_SIZE_2_BYTE;

                /* Update the SPCMD0 command register with the setting for 16 bit width. */
                HW_RSPI_DataBitLength(p_ctrl->p_reg, RSPI_SPCMD_SPB_16BIT);
                HW_RSPI_DataBitLength_HalfWord(p_ctrl->p_reg);
            break;

            default:
                p_ctrl->p_transfer_tx->p_cfg->p_info->size = TRANSFER_SIZE_4_BYTE;
                p_ctrl->p_transfer_rx->p_cfg->p_info->size = TRANSFER_SIZE_4_BYTE;

                /* Update the SPCMD0 command register with the setting for 32 bit width. */
                HW_RSPI_DataBitLength(p_ctrl->p_reg, RSPI_SPCMD_SPB_32BIT);
                HW_RSPI_DefaultDataBitLength(p_ctrl->p_reg);
            break;
        }

        /* Common reconfigurable parameter for 16/32 bit transfer_info*/
        if((bit_width == SPI_BIT_WIDTH_16_BITS) || (bit_width == SPI_BIT_WIDTH_32_BITS))
        {
            p_ctrl->p_transfer_tx->p_cfg->p_info->chain_mode = TRANSFER_CHAIN_MODE_DISABLED;

            p_ctrl->p_transfer_rx->p_cfg->p_info->chain_mode = TRANSFER_CHAIN_MODE_DISABLED;
            p_ctrl->p_transfer_rx->p_cfg->p_info->repeat_area = TRANSFER_REPEAT_AREA_DESTINATION;
            p_ctrl->p_transfer_rx->p_cfg->p_info->dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED;
        }
    }
}/*End of function rspi_reconfig_dtc_p_info().*/
