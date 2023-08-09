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
* File Name    : r_sci_i2c.c
* Description  : This module contains API functions and HLD layer functions for SCI simple I2C module.
**********************************************************************************************************************/


/**********************************************************************************************************************
Includes
**********************************************************************************************************************/
#include "r_sci_i2c.h"
#include "r_sci_i2c_private_api.h"
#include "hw/hw_sci_i2c_private.h"
#include "hw/hw_sci_common.h"

#include "r_cgc.h"
#include "math.h"
#include "string.h"

/**********************************************************************************************************************
Macro definitions
**********************************************************************************************************************/

/** "SIIC" in ASCII, used to determine if channel is open. */
#define SIIC_OPEN                (0x83737343ULL)
#define MDDR_MIN                  128U
#define MDDR_MAX                  256U
/** Macro for error logger. */
#ifndef SCI_SIIC_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define SCI_SIIC_ERROR_RETURN(a, err)  SSP_ERROR_RETURN((a), (err), &g_module_name[0], NULL)
#endif

#define I2C_CODE_READ   (0x01U)
#define I2C_CODE_10BIT  (0xF0U)

#define SCI_SIIC_NUM_DIVISORS_SYNC       (4U)   // Number of synchronous divisors
#define SCI_SIIC_BRR_MAX                 (255U) // Maximum Bit Rate Register (BRR)
#define SCI_SIIC_BRR_MIN                 (0U)   // Minimum Bit Rate Register (BRR)

/*  In order to get the standard 300ns SDA output delay, the PCLK speed must be less than 93 MHz.
 *  Otherwise with PCLKA at 120MHz (max frequency on an S7 device for PCLKA),
 *  and CKS[1:0] bits in SMR = 0 (PCLKA/1), it is not possible to set the required 300ns delay
 *  (max delay is 31/120MHz = 258ns).
 *  */
#define SCI_SIIC_MAX_PCLK                (0xFFFFFFFFFFFFFFFFULL)

#define SCI_I2C_NANOSECONDS_PER_SECOND   (1000000000U)

/* One bit period is required to complete stop condition generation.
 * Number of cpu cycles representing one I2C clock period when using highest possible CPU CLK
 * and lowest possible I2C clock
 * CPU CLK = 240 MHz , I2C CLK = 100 KHz => 1 I2C CLK = 2400 CPU CLK */
#define SCI_I2C_STOPPENDING_TIMEOUT      (2400U)

/* 0x1FU: 5 bit field SIMR1.IICDL[4:0] = 31 cycles is the maximum sda delay */
#define SCI_I2C_SIMR1_IICDL_MAXIMUM_VALUE   (0x1FU)
    
/**********************************************************************************************************************
Typedef definitions
**********************************************************************************************************************/
/**********************************************************************************************************************
Private function prototypes
**********************************************************************************************************************/
/* Convenience functions */
void  sci_siic_notify(sci_i2c_instance_ctrl_t  * const p_ctrl,
                      i2c_event_t   const event);
static void   sci_siic_clock_settings    (uint32_t * const   p_rate,
		                                       uint8_t  * const   p_brr,
		                                       uint8_t  * const   p_divisor,
                                               uint32_t  * const   p_mddr,
                                               uint16_t      sda_delay,
                                               uint32_t *     cycles);
static void sci_siic_sda_delay_settings(uint32_t const  clk_divisor,
                                               uint16_t  sda_delay,
                                               uint32_t  * const   p_cycles);
static ssp_err_t   sci_siic_abort_seq_master  (sci_i2c_instance_ctrl_t         * const p_ctrl);

/* Functions that manipulate hardware */
static ssp_err_t   sci_siic_open_hw_master    (sci_i2c_instance_ctrl_t         * const p_ctrl);
static void        sci_siic_close_hw_master   (sci_i2c_instance_ctrl_t         * const p_ctrl);
static void        sci_siic_abort_hw_master   (sci_i2c_instance_ctrl_t         * const p_ctrl);
static ssp_err_t   sci_siic_run_hw_master     (sci_i2c_instance_ctrl_t         * const p_ctrl);

static void sci_siic_set_irq_parameters(uint8_t ipl, void * p_ctrl, IRQn_Type * p_irq);

static ssp_err_t sci_siic_configure_irqs(ssp_feature_t * p_feature,i2c_ctrl_t * const p_api_ctrl, i2c_cfg_t  const * const p_cfg );

void sci_i2c_rxi_isr (void);
void sci_i2c_txi_isr (void);
void sci_i2c_tei_isr (void);

static ssp_err_t    sci_siic_open_transfer_interface        (sci_i2c_instance_ctrl_t      * const p_ctrl,
                                                             i2c_cfg_t const * const p_cfg);
static ssp_err_t sci_siic_reconfigure_interrupts_for_transfer       (sci_i2c_instance_ctrl_t      * const p_ctrl);
static void      sci_siic_stop_transfer_interface           (sci_i2c_instance_ctrl_t      * const p_ctrl);

static ssp_err_t  sci_siic_transfer_configure_rx               (sci_i2c_instance_ctrl_t * const p_ctrl,
                                                             i2c_cfg_t const * const p_cfg,
                                                             R_SCI0_Type * p_sci_reg);
static ssp_err_t  sci_siic_transfer_configure_tx              (sci_i2c_instance_ctrl_t * const p_ctrl,
                                                             i2c_cfg_t const * const p_cfg,
                                                             R_SCI0_Type * p_sci_reg);
static  void  sci_siic_enable_transfer_support_tx  (sci_i2c_instance_ctrl_t      * const p_ctrl);

static  void  sci_siic_enable_transfer_support_rx  (sci_i2c_instance_ctrl_t      * const p_ctrl);

static  void  sci_siic_txi_send_data     (sci_i2c_instance_ctrl_t      * const p_ctrl,
                                                             R_SCI0_Type * p_sci_reg);
static  void  sci_siic_tei_send_address     (sci_i2c_instance_ctrl_t      * const p_ctrl,
                                                             R_SCI0_Type * p_sci_reg);
static ssp_err_t sci_siic_open_parameter_check(sci_i2c_instance_ctrl_t * const p_ctrl,i2c_cfg_t  const * const p_cfg);

static void sci_siic_tei_handler(sci_i2c_instance_ctrl_t      * const p_ctrl);
static void sci_siic_txi_handler(sci_i2c_instance_ctrl_t      * const p_ctrl);
static void sci_siic_rxi_handler(sci_i2c_instance_ctrl_t      * const p_ctrl);
static void sci_siic_txi_process_nack(sci_i2c_instance_ctrl_t       * const p_ctrl);
static ssp_err_t sci_siic_dtc_max_length_check(sci_i2c_instance_ctrl_t * const p_ctrl, const uint32_t bytes);

/**********************************************************************************************************************
Private global variables
**********************************************************************************************************************/
/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "sci_iic";
#endif

#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug.  This pragma suppresses the warnings in this
 * structure only.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static ssp_version_t const g_sci_siic_master_version =
{
    .api_version_major  = I2C_MASTER_API_VERSION_MAJOR,
	.api_version_minor  = I2C_MASTER_API_VERSION_MINOR,
	.code_version_minor = SCI_SIIC_MASTER_CODE_VERSION_MINOR,
    .code_version_major = SCI_SIIC_MASTER_CODE_VERSION_MAJOR,
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/** constant used as the source location for the DTC dummy write  */
static const uint8_t g_dummy_write_data_for_read_op = 0xFF;

/**********************************************************************************************************************
Global variables
**********************************************************************************************************************/
/** SCI SIIC Implementation of I2C device master interface */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
i2c_api_master_t const g_i2c_master_on_sci =
{
    .versionGet = R_SCI_SIIC_MasterVersionGet,
    .open       = R_SCI_SIIC_MasterOpen,
    .close      = R_SCI_SIIC_MasterClose,
    .read       = R_SCI_SIIC_MasterRead,
    .write      = R_SCI_SIIC_MasterWrite,
    .reset      = R_SCI_SIIC_MasterReset,
    .slaveAddressSet = R_SCI_SIIC_MasterSlaveAddressSet
};

/** Baud rate divisor information(simple I2C mode) */
static const baud_setting_t sync_baud[SCI_SIIC_NUM_DIVISORS_SYNC] =
{
    {   32U, 1U, 1U, 1U, 0U }, /* divisor, BGDM, ABCS, ABCSE, n */
    {  128U, 1U, 1U, 1U, 1U },
    {  512U, 1U, 1U, 1U, 2U },
    { 2048U, 1U, 1U, 1U, 3U }
};

/**********************************************************************************************************************
Functions
**********************************************************************************************************************/

/*****************************************************************************************************************//**
 * @addtogroup SIIC
 * @{
 ********************************************************************************************************************/

/******************************************************************************************************************//**
 * @brief   Sets driver version based on compile time macros.
 *
 * @retval  SSP_SUCCESS                 Successful version get.
 * @retval  SSP_ERR_ASSERTION           The parameter p_version is NULL.
**********************************************************************************************************************/
ssp_err_t R_SCI_SIIC_MasterVersionGet(ssp_version_t * const p_version)
{
    /** Verify parameter is valid */
#if SCI_SIIC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_version != NULL);
#endif

    p_version->version_id = g_sci_siic_master_version.version_id;

    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief   Opens the I2C device. Power on I2C peripheral and perform initialization described in hardware manual.
 *
 *  This function will reconfigure the clock settings of the peripheral when a device with a lower rate than
 *  previously configured is opened.
 *
 * @retval  SSP_SUCCESS                 Requested baud rate was valid.
 * @retval  SSP_ERR_ASSERTION           The parameter p_ctrl or p_cfg is NULL or if clock rate greater than 400KHz.
 * @retval  SSP_ERR_INVALID_RATE        The requested rate cannot be set.
 * @retval  SSP_ERR_IN_USE              Lock was not acquired
 * @retval  SSP_ERR_IRQ_BSP_DISABLED    Event information could not be found.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * fmi_api_t::productFeatureGet
 *                                   * fmi_api_t::eventInfoGet
**********************************************************************************************************************/
ssp_err_t R_SCI_SIIC_MasterOpen(i2c_ctrl_t         * const p_api_ctrl,
                                i2c_cfg_t  const   * const p_cfg)
{
    sci_i2c_instance_ctrl_t * p_ctrl = (sci_i2c_instance_ctrl_t *) p_api_ctrl;
	ssp_err_t err = SSP_SUCCESS;

#if SCI_SIIC_CFG_PARAM_CHECKING_ENABLE
    err = sci_siic_open_parameter_check(p_ctrl,p_cfg);
    SCI_SIIC_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif

    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = p_cfg->channel;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_SCI;
    fmi_feature_info_t info = {0U};
    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    SCI_SIIC_ERROR_RETURN(SSP_SUCCESS == err, err);

    /* Lock specified SCI channel */
    err = R_BSP_HardwareLock(&ssp_feature);
    SCI_SIIC_ERROR_RETURN((SSP_SUCCESS == err), err);

    p_ctrl->p_reg = info.ptr;

    /** Disable, clear and configure interrupts*/
    err = sci_siic_configure_irqs(&ssp_feature, p_api_ctrl, p_cfg);
    if(SSP_SUCCESS != err)
    {
        /* Release hardware lock. */
        R_BSP_HardwareUnlock(&ssp_feature);
        return err;
    }
    R_BSP_ModuleStart(&ssp_feature);

    /* Record the configuration on the device for use later */
    p_ctrl->info = *p_cfg;

    /** Open the hardware in master mode */
    err = sci_siic_open_hw_master(p_ctrl);

    /* Now handle any fatal errors that may have occurred */
    if (SSP_SUCCESS == err)
    {
        /* Open transfer interfaces if available */
        p_ctrl->p_transfer_rx = p_cfg->p_transfer_rx;
        p_ctrl->p_transfer_tx = p_cfg->p_transfer_tx;
        err = sci_siic_open_transfer_interface(p_ctrl, p_cfg);
        if(SSP_SUCCESS == err)
        {
            /* Finally, we can consider the device opened */
            p_ctrl->transaction_completed = true;
            p_ctrl->p_buff = NULL;
            p_ctrl->total = 0U;
            p_ctrl->remain = 0U;
            p_ctrl->loaded = 0U;
            p_ctrl->read = false;
            p_ctrl->restart = false;
            p_ctrl->err = false;
            p_ctrl->restarted = false;
            p_ctrl->open = SIIC_OPEN;
        }
    }
    if(SSP_SUCCESS != err)
    {
        /* Undo the initialization if we were in the middle of it */
        R_BSP_ModuleStop(&ssp_feature);
        /* Unlock specified SCI channel */
        R_BSP_HardwareUnlock(&ssp_feature);
    }
    SCI_SIIC_ERROR_RETURN(SSP_SUCCESS == err, err);
    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief   Closes the I2C device. Power down I2C peripheral.
 *
 *  This function will safely terminate any in-progress I2C transfer with the device. If a transfer is aborted, the user
 *  will be notified via callback with an abort event. Since the callback is optional, this function will also return
 *  a specific error code in this situation.
 *
 * @retval  SSP_SUCCESS                 Device closed without issue.
 * @retval  SSP_ERR_ASSERTION           The parameter p_ctrl is NULL.
 * @retval  SSP_ERR_ABORTED             Device was closed while a transfer was in-progress.
 * @retval  SSP_ERR_NOT_OPEN            Device was not even opened.
**********************************************************************************************************************/
ssp_err_t R_SCI_SIIC_MasterClose(i2c_ctrl_t * const p_api_ctrl)
{
    sci_i2c_instance_ctrl_t * p_ctrl = (sci_i2c_instance_ctrl_t *) p_api_ctrl;
#if SCI_SIIC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl != NULL);
#endif

    ssp_err_t err = SSP_SUCCESS;

    /* Check if the device is even open, return an error if not */
    if (SIIC_OPEN != p_ctrl->open)
    {
        err = SSP_ERR_NOT_OPEN;
    }
    else
    {
        /** Abort an in-progress transfer with this device only */
        err = sci_siic_abort_seq_master(p_ctrl);

        /** The device is now considered closed */
        p_ctrl->open = 0U;

        /** De-configure everything. */
        sci_siic_close_hw_master(p_ctrl);

        if (NULL != p_ctrl->info.p_transfer_rx)
        {
            p_ctrl->info.p_transfer_rx->p_api->close(p_ctrl->info.p_transfer_rx->p_ctrl);
        }

        if (NULL != p_ctrl->info.p_transfer_tx)
        {
            p_ctrl->info.p_transfer_tx->p_api->close(p_ctrl->info.p_transfer_tx->p_ctrl);
        }
    }
    SCI_SIIC_ERROR_RETURN(SSP_SUCCESS == err, err);
    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief   Performs a read from the I2C device.
 *
 *  This function will fail if there is already an in-progress I2C transfer on the associated channel. Otherwise, the
 *  I2C read operation will begin. When no callback is provided by the user, this function performs a blocking read.
 *  Otherwise, the read operation is non-blocking and the caller will be notified when the operation has finished by
 *  an I2C_EVENT_RX_COMPLETE in the callback.
 *
 * @retval  SSP_SUCCESS                 Function executed without issue.
 * @retval  SSP_ERR_ASSERTION           The parameter p_ctrl, p_dest is NULL, bytes is 0.
 * @retval  SSP_ERR_INVALID_SIZE        Provided number of bytes more than uint16_t size(65535) while DTC is used
 *                                      for data transfer.
 * @retval  SSP_ERR_IN_USE              Another transfer was in-progress.
 * @retval  SSP_ERR_NOT_OPEN            Device was not even opened.
**********************************************************************************************************************/
ssp_err_t R_SCI_SIIC_MasterRead(i2c_ctrl_t * const p_api_ctrl,
                                uint8_t    * const p_dest,
                                uint32_t     const bytes,
                                bool         const restart)
{
    sci_i2c_instance_ctrl_t * p_ctrl = (sci_i2c_instance_ctrl_t *) p_api_ctrl;
#if SCI_SIIC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl != NULL);
    SSP_ASSERT(p_dest != NULL);
    SSP_ASSERT(bytes != 0U);
#endif

    ssp_err_t err = SSP_SUCCESS;
    SCI_SIIC_ERROR_RETURN(SIIC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    /* Fail if there is already a transfer in progress */
    if (false == p_ctrl->transaction_completed)
    {
        err = SSP_ERR_IN_USE;
    }
    else
    {
        /* If DTC is used for data transfer validate the data length provided by user,
         * If length not supported then return error. */
        if (NULL != p_ctrl->p_transfer_tx)
        {
            uint32_t num_transfers = bytes;
            transfer_properties_t transfer_max = {0U};
            p_ctrl->info.p_transfer_tx->p_api->infoGet(p_ctrl->info.p_transfer_tx->p_ctrl, &transfer_max);
            if (num_transfers >= transfer_max.transfer_length_max)
            {
                return SSP_ERR_INVALID_SIZE;
            }
        }
        /** Record the new information about this transfer */
        p_ctrl->p_buff  = p_dest;
        p_ctrl->total   = bytes;
        p_ctrl->remain  = bytes;
        p_ctrl->restart = restart;
        p_ctrl->read    = true;

        /** Handle the different addressing modes */
        if (p_ctrl->info.addr_mode == I2C_ADDR_MODE_7BIT)
        {
            /* Set the address bytes according to a 7-bit slave read command */
            p_ctrl->addr_high  = 0U;
            p_ctrl->addr_low   = (uint8_t) (p_ctrl->info.slave << 1) | I2C_CODE_READ;
            p_ctrl->addr_total = 1U;
        }
        else
        {
        	/* Set the address bytes according to a 10-bit slave read command */
                p_ctrl->addr_high   = (uint8_t)((p_ctrl->info.slave >> 7) | I2C_CODE_10BIT) & (uint8_t)~I2C_CODE_READ;
                p_ctrl->addr_low    = (uint8_t)p_ctrl->info.slave;
                p_ctrl->addr_total  = 3U;
        }

        /** Kickoff the read operation as a master */
        err = sci_siic_run_hw_master(p_ctrl);
    }
    SCI_SIIC_ERROR_RETURN(SSP_SUCCESS == err, err);
    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief   Performs a write to the I2C device.
 *
 *  This function will fail if there is already an in-progress I2C transfer on the associated channel. Otherwise, the
 *  I2C write operation will begin. When no callback is provided by the user, this function performs a blocking write.
 *  Otherwise, the write operation is non-blocking and the caller will be notified when the operation has finished by
 *  an I2C_EVENT_TX_COMPLETE in the callback.
 *
 * @retval  SSP_SUCCESS                 Function executed without issue.
 * @retval  SSP_ERR_ASSERTION           p_ctrl, p_src is NULL.
 * @retval  SSP_ERR_INVALID_SIZE        Provided number of bytes more than uint16_t size(65535) while DTC is used
 *                                      for data transfer.
 * @retval  SSP_ERR_IN_USE              Another transfer was in-progress.
 * @retval  SSP_ERR_NOT_OPEN            Device was not even opened.
**********************************************************************************************************************/
ssp_err_t R_SCI_SIIC_MasterWrite(i2c_ctrl_t * const p_api_ctrl,
                                 uint8_t    * const p_src,
                                 uint32_t     const bytes,
                                 bool         const restart)
{
    sci_i2c_instance_ctrl_t * p_ctrl = (sci_i2c_instance_ctrl_t *) p_api_ctrl;
#if SCI_SIIC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl != NULL);
    SSP_ASSERT(p_src != NULL);
#endif

    ssp_err_t err = SSP_SUCCESS;

    SCI_SIIC_ERROR_RETURN(SIIC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    /* Fail if there is already a transfer in progress */
    if (false == p_ctrl->transaction_completed)
    {
        err = SSP_ERR_IN_USE;
    }
    else
    {
        /* If DTC is used for data transfer validate the data length provided by user,
         * If length not supported then return error. */
        err = sci_siic_dtc_max_length_check(p_ctrl, bytes);
        SCI_SIIC_ERROR_RETURN(SSP_SUCCESS == err, err);

        /** Record the new information about this transfer */
        p_ctrl->p_buff  = p_src;
        p_ctrl->total   = bytes;
        p_ctrl->remain  = bytes;
        p_ctrl->restart = restart;
        p_ctrl->read    = false;

        /** Handle the different addressing modes */
        if (p_ctrl->info.addr_mode == I2C_ADDR_MODE_7BIT)
        {
            /* Set the address bytes according to a 7-bit slave write command */
            p_ctrl->addr_high  = 0U;
            p_ctrl->addr_low   = (uint8_t) (p_ctrl->info.slave << 1) & ~I2C_CODE_READ;
            p_ctrl->addr_total = 1U;
        }
        else
        {
        	/* Set the address bytes according to a 10-bit slave read command */
                p_ctrl->addr_high   = (uint8_t)(((p_ctrl->info.slave >> 7) | I2C_CODE_10BIT) & (uint8_t)~I2C_CODE_READ);
                p_ctrl->addr_low    = (uint8_t)p_ctrl->info.slave;
                p_ctrl->addr_total  = 2U;
        }

        /** Kickoff the write operation as a master */
        err = sci_siic_run_hw_master(p_ctrl);
    }
    SCI_SIIC_ERROR_RETURN(SSP_SUCCESS == err, err);
    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief   Aborts any in-progress transfer and forces the I2C peripheral into a ready state.
 *
 *  This function will safely terminate any in-progress I2C transfer with the device. If a transfer is aborted, the user
 *  will be notified via callback with an abort event. Since the callback is optional, this function will also return
 *  a specific error code in this situation.
 *
 * @retval  SSP_SUCCESS                 Channel was reset without issue.
 * @retval  SSP_ERR_ASSERTION           p_ctrl is NULL.
 * @retval  SSP_ERR_ABORTED             A transfer was aborted while resetting the hardware.
 * @retval  SSP_ERR_NOT_OPEN            Device was not even opened.
**********************************************************************************************************************/
ssp_err_t R_SCI_SIIC_MasterReset(i2c_ctrl_t * const p_api_ctrl)
{
    sci_i2c_instance_ctrl_t * p_ctrl = (sci_i2c_instance_ctrl_t *) p_api_ctrl;
#if SCI_SIIC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl != NULL);
#endif

    SCI_SIIC_ERROR_RETURN(SIIC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    /** Abort any transfer happening on the channel */
    ssp_err_t err = sci_siic_abort_seq_master(p_ctrl);
    SCI_SIIC_ERROR_RETURN(SSP_SUCCESS == err, err);
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief Sets address and addressing mode of the slave device.
 *
 * This function is used to set the device address and addressing mode of the slave without reconfiguring the entire bus.
 *
 * @retval  SSP_SUCCESS                 Address of the slave is set correctly.
 * @retval  SSP_ERR_ASSERTION           p_ctrl or address is NULL.
 * @retval  SSP_ERR_IN_USE              Another transfer was in-progress.
 * @retval  SSP_ERR_NOT_OPEN            Device was not even opened.
 *
 **********************************************************************************************************************/
ssp_err_t R_SCI_SIIC_MasterSlaveAddressSet (i2c_ctrl_t    * const p_api_ctrl,
		                                    uint16_t        const slave,
		                                    i2c_addr_mode_t const addr_mode)
{
	sci_i2c_instance_ctrl_t * p_ctrl = (sci_i2c_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t err = SSP_SUCCESS;

#if SCI_SIIC_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl != NULL);
#endif

    SCI_SIIC_ERROR_RETURN(SIIC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    /* Fail if there is already a transfer in progress */
    if (false == p_ctrl->transaction_completed)
    {
        err = SSP_ERR_IN_USE;
    }
    else
    {
    	/** Sets the address of the slave device */
    	p_ctrl->info.slave     = slave;

    	/** Sets the mode of addressing */
    	p_ctrl->info.addr_mode = addr_mode;
    }

    return err;
}

/***************************************************************************************************************//**
 * @} (end addtogroup SIIC)
*******************************************************************************************************************/

/**********************************************************************************************************************
Private Functions
**********************************************************************************************************************/

/******************************************************************************************************************//**
 * @brief   Single point for managing the logic around notifying a transfer has finished.
 *
 * @param[in]       p_ctrl      Pointer to transfer that is ending.
 * @param[in]       event       The event code to pass to the callback.
**********************************************************************************************************************/
void  sci_siic_notify(sci_i2c_instance_ctrl_t  * const p_ctrl,
                      i2c_event_t   const event)
{
    /* Check if we can notify the caller of the abort via callback */
    if (NULL != p_ctrl->info.p_callback)
    {
        /* Fill in the argument to the callback */
        i2c_callback_args_t args =
        {
            .bytes      = p_ctrl->total - p_ctrl->remain,
            .p_context  = p_ctrl->info.p_context,
            .event      = event
        };

        /* Now do the callback here */
        p_ctrl->info.p_callback(&args);

    }

    /** Set the flag indicating that the transaction is completed*/
    p_ctrl->transaction_completed = true;

}

/*******************************************************************************************************************//**
 * @brief   This function changes baud rate. It evaluates and determines the best possible settings for the baud rate
 * registers.
 * @param[in] p_rate      bitrate[bps] e.g. 10000,100000; 400000(max), etc.
 * @param[in] p_brr       bit rate register value
 * @param[in] p_divisor   Divisor value from CKS[1:0]
 * @param[in] sda_delay   Requested SDA delay in nano seconds.
 *                        Standard value is for sda_delay 300ns and Maximum supported value is 1000ns.
 * @param[out] p_mddr     MDDR register value
 * @param[out] cycles     value to be set in IICDL bits
 *
 * @retval  SSP_SUCCESS                  Baud rate is set successfully
 * @retval  SSP_ERR_ASSERTION            Baud rate is '0' or cannot set properly
 * @note    The application must pause for 1 bit time after the BRR register is loaded
 *          before transmitting/receiving to allow time for the clock to settle.
 ********************************************************************************************************************/
static void sci_siic_clock_settings(uint32_t * const   p_rate,
                                         uint8_t  * const   p_brr,
                                         uint8_t  * const   p_divisor,
                                         uint32_t * const   p_mddr,
                                         uint16_t      sda_delay,
                                         uint32_t *     cycles)
{
    uint32_t         i             = 0;
    uint32_t         temp_brr      = SCI_SIIC_BRR_MIN;
    baud_setting_t   * p_baudinfo;
    ssp_err_t        result;
    uint32_t         clock_hz = 0U;
    uint32_t         bitrate;
    uint32_t         temp_mbbr;

    bsp_feature_sci_t sci_feature = {0U};
    R_BSP_FeatureSciGet(&sci_feature);
    g_cgc_on_cgc.systemClockFreqGet((cgc_system_clocks_t) sci_feature.clock, &clock_hz);

    p_baudinfo = (baud_setting_t *) sync_baud;
    bitrate = *p_rate;

    /* Calculate maximum internal baud rate generator clock */
    /* Based on the max sda delay cycles supported, see SIMR1.IICDL[4:0] */
    /*LDRA_INSPECTED 488 S since no limit to clock we consider it max when sda delay is 0. */
    /*LDRA_INSPECTED 93 S no limit to clock we consider it max when sda delay is 0. */
    uint64_t sci_ii2c_max_clock = SCI_SIIC_MAX_PCLK;
    if(0U != sda_delay)
    {
        /* 31U: 5 bit field SIMR1.IICDL[4:0] = 31 */
        /* 1000000000U: Convert sda_delay from nanoseconds to seconds */
        /*LDRA_INSPECTED 93 S changed to 64 bit in order to avoid overflow for
         *  faster clock rates on new chips in the future. */

        sci_ii2c_max_clock =
                /*LDRA_INSPECTED 93 S changed to 64 bit in order to avoid overflow for
                 * faster clock rates on new chips in the future. */
                ((uint64_t)SCI_I2C_SIMR1_IICDL_MAXIMUM_VALUE *
                        (uint64_t)SCI_I2C_NANOSECONDS_PER_SECOND) / (uint64_t)sda_delay;
    }

    /* FIND BEST_BRR_VALUE
     *  In table sync_baud", divisor value is associated with
     *  the divisor coefficient and the n values (clock divisor) , so once best divisor value is found,
     *  baud rate related register setting values are decided.
     *  The formula to calculate BRR is as follows and it must be 255 or less.
     *  BRR = (PCLKA/(div_coefficient * baud)) - 1
     */
    result = SSP_ERR_ASSERTION;
    for (i = 0U; i < SCI_SIIC_NUM_DIVISORS_SYNC; i++)
    {
        uint32_t pclk_hz = clock_hz / (p_baudinfo[i].div_coefficient >> 5);
        /* Calculate and Set SDA Output Delay */
        sci_siic_sda_delay_settings(i, sda_delay, cycles);
        /* Compare the PCLK to the maximum possible internal baud rate generator clock */
        if ((sci_ii2c_max_clock >= (uint64_t)pclk_hz) && (*cycles <= SCI_I2C_SIMR1_IICDL_MAXIMUM_VALUE))
        {
            uint32_t temp_divisor = p_baudinfo[i].div_coefficient * bitrate;
            temp_brr = (clock_hz + (temp_divisor - 1)) / temp_divisor;
            if ((SCI_SIIC_BRR_MIN < temp_brr) && (temp_brr < ((uint32_t) SCI_SIIC_BRR_MAX + 1U)))
            {
                /* save the calculated value */
                *p_brr = (uint8_t)(temp_brr - 1U);
                *p_divisor = (uint8_t)i;

                /* re-calculate the bit rate based on the temp_brr and divisor coefficient */
                *p_rate = clock_hz / (temp_brr * (p_baudinfo[i].div_coefficient) );

                result = SSP_SUCCESS;
                break;
            }
        }
    }

    /* if the appropriate BRR value is not found, zero out all the value */
    if (SSP_SUCCESS != result)
    {
    	*p_rate = 0U;
    	*p_brr = 0U;
    	*p_divisor = 0U;
    }

    /* Calculate the MBBR (M) value,
     * The formula to calculate MBBR (from the M and N relationship given in the HM) is as follows and it
     * must between 128 and 256.
     * MBBR = ((div_coefficient * baud * 256) * (BRR + 1)) / PCLKA */
    temp_mbbr = ((uint32_t) p_baudinfo[*p_divisor].div_coefficient * bitrate * 256);
    *p_mddr = (temp_mbbr * ((uint32_t)*p_brr+1)) / clock_hz;

}

/*******************************************************************************************************************//**
 * @brief  This function calculates the SDA delay value (clock cycles) to be set to IICDL bits, based on the PCLCK frequency
 *         and divider settings.
 * @param[in]  clk_divisor   Clock divisor value
 * @param[in]  sda_delay     Requested SDA delay in nano seconds.
 *                           Standard value is for sda_delay 300ns and Maximum supported value is 1000ns.
 * @param[out] p_cycles        value to be set in IICDL bits
 ********************************************************************************************************************/
static void sci_siic_sda_delay_settings(uint32_t const  clk_divisor,
                                         uint16_t      sda_delay,
                                         uint32_t  * const   p_cycles)
{
    uint32_t clk_divisor_value = 0U;
    uint32_t clock_hz = 0U;

    clk_divisor_value = (1U << (clk_divisor * 2U));

    bsp_feature_sci_t sci_feature = {0U};
    R_BSP_FeatureSciGet(&sci_feature);
    g_cgc_on_cgc.systemClockFreqGet((cgc_system_clocks_t) sci_feature.clock, &clock_hz);

    /* Determine the internal baud rate generator clock frequency */
    uint32_t baud_rate_generator_frequency_hz = clock_hz / clk_divisor_value;

    /* Determine ratio of a nanosecond (1GHz) clock to the baud rate generator source clock */
    uint32_t ratio_ns_to_baud_gen_clock = 
        SCI_I2C_NANOSECONDS_PER_SECOND / baud_rate_generator_frequency_hz;

    /* sda_delay is cycles of a nanosecond clock, convert to cycles of the internal baud generator clock */
    uint32_t baud_gen_clock_cycles = ((uint32_t)sda_delay / ratio_ns_to_baud_gen_clock);

    /* Only use 0 delay if the configuration specifies 0 delay */
    /* Adding one to ensure the user spcified minimum sda delay is not violated */
    if(0U != sda_delay)
    {
        baud_gen_clock_cycles++;
    }

    *p_cycles = baud_gen_clock_cycles;
}

/******************************************************************************************************************//**
 * @brief   Single point for managing the logic around aborting a transfer when operating as a master.
 *
 * @param[in]       p_ctrl  Pointer to control struct of specific device
 *
 * @retval SSP_ERR_ABORTED             If there is an in-progress transfer
**********************************************************************************************************************/
static ssp_err_t sci_siic_abort_seq_master (sci_i2c_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;

    /* Check if there is an in-progress transfer associated with the match */
    if ((0U != p_ctrl->remain) || (p_ctrl->restarted))
    {

    	/* stop the transfer interface if running */
    	sci_siic_stop_transfer_interface(p_ctrl);

    	/* Safely stop the hardware from operating */
        sci_siic_abort_hw_master(p_ctrl);

        /* Notify anyone waiting that the transfer is finished */
        sci_siic_notify(p_ctrl, I2C_EVENT_ABORTED);

        /* Update the transfer descriptor to show no longer in-progress and an error */
        p_ctrl->remain = 0U;
        p_ctrl->err    = true;
        p_ctrl->restarted = false;

        /* Update the transfer descriptor to make sure interrupts no longer process */
        p_ctrl->addr_loaded = p_ctrl->addr_total;
        p_ctrl->loaded     = p_ctrl->total;

        /* Notify the caller via error code since the callback is optional */
        err = SSP_ERR_ABORTED;
    }
    return err;
}

/******************************************************************************************************************//**
 * @brief   Performs the hardware initialization sequence when operating as a master.
 *
 * @param[in]       p_ctrl  Pointer to control structure of specific device
 *
 * @retval  SSP_SUCCESS                 Hardware initialized with proper configurations
 * @retval  SSP_ERR_INVALID_RATE        The requested rate cannot be set.
**********************************************************************************************************************/
static ssp_err_t sci_siic_open_hw_master(sci_i2c_instance_ctrl_t * const p_ctrl)
{
    sci_i2c_extended_cfg * pextend;

    pextend = (sci_i2c_extended_cfg *) p_ctrl->info.p_extend;

    /* Perform the first part of the initialization sequence */
    R_SCI0_Type * p_sci_reg = (R_SCI0_Type *) p_ctrl->p_reg;
    HW_SCI_RegisterReset(p_sci_reg);

    /* Clear the Serial Status register */
    HW_SCI_SerialStatusClear(p_sci_reg);

    /* Clear the Serial Control register */
    HW_SCI_SerialControlClear(p_sci_reg);

    /* Clear the Smart Card Mode register */
    HW_SCI_SmartCardModeClear(p_sci_reg);

    /* Clear the Serial Mode register - SMR */
    HW_SCI_SerialModeClear(p_sci_reg);

    /* Disable FIFO mode */
    HW_SCI_SIIC_FIFODisable(p_sci_reg);

    /* Set SDA and SCL output pin to high-impedance state */
    HW_SCI_SIIC_Output(p_sci_reg, SCI_SIIC_OUTPUT_OFF);

    HW_SCI_ClockSynModeClear(p_sci_reg);

    /* Set the CKE bit in the SCR register to use internal clock */
    HW_SCI_BaudRateGenInternalClkSelect(p_sci_reg);

    /* Set bit order to be MSB first for I2C mode */
    HW_SCI_TransferDirection (p_sci_reg, true);

    /* No data inversion for I2C mode */
    HW_SCI_DataInversionEnable(p_sci_reg, false);

    /* No Smart Card selection for I2C mode */
    HW_SCI_SmartCardEnable(p_sci_reg, false);

    /* Enable or Disable bitrate modulation function */
    if ((pextend))
    {
        /** enables or disable the bitrate modulation function */
        HW_SCI_BitRateModulationEnable(p_sci_reg, pextend->bitrate_modulation);

    }

    /* May need to configure a new clock rate for the channel */
    uint32_t bestMatch;
    uint8_t brr=0U;
    uint8_t clk_divisor=0U;
    uint32_t  cycles =0U;
    uint32_t mddr=0U;

    /* We will be looking for the closest rate less-than-or-equal-to the requested rate */
    /* Also calculates the SDA output delay */
    bestMatch = p_ctrl->info.rate;
    sci_siic_clock_settings(&bestMatch, &brr, &clk_divisor, &mddr, p_ctrl->info.sda_delay, &cycles);

    /* Abort if we could not find good settings */
    if (bestMatch == 0U)
    {
        return SSP_ERR_INVALID_RATE;
    }

    /* Set the bit rate register in the hardware */
    HW_SCI_BitRateBRRSet(p_sci_reg, brr, clk_divisor);

    /* Check Bitrate Modulation function is enabled or not.
     * If it is enabled,set the MBBR register to correct the bit rate generated by the on-chip baud rate generator */
    if(HW_SCI_BitRateModulationCheck(p_sci_reg))
    {
        /* Set MDDR register only for values between 128 and 256, do not set otherwise */
        if ((mddr >= MDDR_MIN) && (mddr <= MDDR_MAX))
        {
            HW_SCI_UartBitRateModulationSet(p_sci_reg, (uint8_t)mddr);
        }
    }

    /* Set SDA Output Delay */
    /* Casting (uint8_t)cycles is safe because the value is verified to be less than 0x1F in sci_siic_clock_settings */
    HW_SCI_SIIC_DataOutputDelay (p_sci_reg, (uint8_t)cycles);

    /* Enable Noise Cancellation function  and set level to 1 */
    HW_SCI_NoiseFilterSet(p_sci_reg, (noise_cancel_lvl_t)1);

    /* Set this SCI channel to operate in simple I2C mode */
    HW_SCI_SIIC_ModeSet(p_sci_reg, SCI_SIIC_MODE_I2C);

    /* Enable Clock Synchronization feature */
    HW_SCI_SIIC_ClockSynchronizationEnable (p_sci_reg, true);

    /* Set NACK transmission */
    HW_SCI_SIIC_ACKNACKEnable(p_sci_reg, true);

    /* Select Interrupt Source */
    HW_SCI_SIIC_InterruptSource(p_sci_reg, SCI_SIIC_INTERRUPT_SOURCE_RECEPTION_TRANSMISSION);

    /* Clear SPI Mode register (SPMR) */
    HW_SCI_SPIModeClear(p_sci_reg);

    /* Enable both transmitter and receiver */
    HW_SCI_TransmitterReceiverEnable(p_sci_reg);

    /* Set the SCL/SDA line to High Impedance state */
    HW_SCI_SIIC_Output(p_sci_reg, SCI_SIIC_OUTPUT_OFF);

    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief   Performs the hardware initialization sequence when operating as a master.
 *
 * @param[in]       p_ctrl  Pointer to control structure of specific device
**********************************************************************************************************************/
static void sci_siic_close_hw_master(sci_i2c_instance_ctrl_t * const p_ctrl)
{
    /* Disables all interrupts */
    R_SCI0_Type * p_sci_reg = (R_SCI0_Type *) p_ctrl->p_reg;
    HW_SCI_RxIrqDisable(p_sci_reg, p_ctrl);
    HW_SCI_TxIrqDisable(p_sci_reg, p_ctrl);
    HW_SCI_TeIrqDisable(p_sci_reg, p_ctrl);

    /* Clear command issuing flag and sets SDA/SCL to high-impedance state */
    HW_SCI_SIIC_LineHalt(p_sci_reg);

    /* Shutdown the hardware to save some power */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = p_ctrl->info.channel;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_SCI;
    R_BSP_ModuleStop(&ssp_feature);

    /* Unlock specified SCI channel */
    R_BSP_HardwareUnlock(&ssp_feature);
}

/******************************************************************************************************************//**
 * @brief   Safely stops the current data transfer when operating as a master.
 *
 * @param[in]       p_ctrl  Pointer to control structure of specific device
**********************************************************************************************************************/
static void sci_siic_abort_hw_master(sci_i2c_instance_ctrl_t * const p_ctrl)
{
    /** Disable channel interrupts */
    R_SCI0_Type * p_sci_reg = (R_SCI0_Type *) p_ctrl->p_reg;
    HW_SCI_RxIrqDisable(p_sci_reg, p_ctrl);
    HW_SCI_TxIrqDisable(p_sci_reg, p_ctrl);
    HW_SCI_TeIrqDisable(p_sci_reg, p_ctrl);

    /* Check if the peripheral is holding the line low */
    if (HW_SCI_SIIC_SDALow(p_sci_reg))
    {
        /* Release the line manually by setting the SDA and SCL line to high impedance, command flag to 0 */
    	HW_SCI_SIIC_LineHalt(p_sci_reg);
    }

    /* Try to issue the stop condition now */
    HW_SCI_SIIC_SendStop(p_sci_reg);

    /* Initialize the timeout to one bit period */
    volatile int16_t timeout = SCI_I2C_STOPPENDING_TIMEOUT;

    /** Wait until the stop condition is sent or break the loop when timeout occurs */
    while ((HW_SCI_SIIC_StopPending(p_sci_reg)) && (0 < timeout))
    {
        timeout--;
    }

    /* Shut off the SDA/SCL output pins anyway */
    HW_SCI_SIIC_LineHalt(p_sci_reg);
}

/******************************************************************************************************************//**
 * @brief   Performs the data transfer described by the parameters when operating as a master.
 *
 * @param[in]       p_ctrl  Pointer to Control structure of specific device.
 *
 * @retval   SSP_SUCCESS                     Data transferred when operating as a master.
 * @retval   SSP_ERR_IN_USE                  If a transfer is in progress.
 * @retval   SSP_ERR_ABORTED                 If there is an in-progress transfer.
**********************************************************************************************************************/
static ssp_err_t sci_siic_run_hw_master(sci_i2c_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err     = SSP_SUCCESS;
    R_SCI0_Type * p_sci_reg = (R_SCI0_Type *) p_ctrl->p_reg;

    /* Initialize fields used during transfer */
    p_ctrl->addr_loaded     = 0U;
    p_ctrl->loaded          = 0U;
    p_ctrl->remain          = p_ctrl->total;
    p_ctrl->addr_remain     = p_ctrl->addr_total;
    p_ctrl->err             = false;
    p_ctrl->do_dummy_read   = false;
    p_ctrl->activation_on_txi = false;
    p_ctrl->activation_on_rxi = false;

    if (true == p_ctrl->read)
    {
        p_ctrl->do_dummy_read = true;
    }

    /* Set the flag indicating that the transaction is not completed*/
    p_ctrl->transaction_completed = false;

    err = sci_siic_reconfigure_interrupts_for_transfer(p_ctrl);
    SCI_SIIC_ERROR_RETURN((SSP_SUCCESS == err), err);

    /* Enable RXI Irq only if we are using ACK/NACK interrupt mode */
    if(HW_SCI_SIIC_InterruptSourceIsACKNACK(p_sci_reg))
    {
        /* Enable  RXI  interrupt */
        HW_SCI_RxIrqEnable(p_sci_reg, p_ctrl);
    }
    else
    {
        /* Disable RXI  interrupt */
        HW_SCI_RxIrqDisable(p_sci_reg, p_ctrl);
    }

    /* Enable TXI and TEI interrupts */
    HW_SCI_TxIrqEnable(p_sci_reg, p_ctrl);
    HW_SCI_TeIrqEnable(p_sci_reg, p_ctrl);

    /* Set NACK transmission, since we are going to transmit address */
    HW_SCI_SIIC_ACKNACKEnable(p_sci_reg, true);

    /* Check if this is a new transaction or a continuation */
    if (!p_ctrl->restarted)
    {
        /* Set the I2C into master mode and start interrupt processing */
        HW_SCI_SIIC_SendStart(p_sci_reg);
    }
    else
    {
    	/* Clear for next transfer */
        p_ctrl->restarted = false;

    	/* Send MSB or LSB based on the addressing mode */
    	uint8_t	data = (1U == p_ctrl->addr_total) ? (p_ctrl->addr_low) : (p_ctrl->addr_high);

    	/* Enable transfer support for tx if this is the last address byte */
        if (1U == p_ctrl->addr_total)
    	{
    		sci_siic_enable_transfer_support_tx(p_ctrl);
    	}

    	/* Write into the TDR */
    	HW_SCI_Write(p_sci_reg, data);

    	/* Update the trackers */
    	p_ctrl->addr_remain--;
    	p_ctrl->addr_loaded++;
    }

    /* Check if we must block until the transfer is done */
    if (NULL == p_ctrl->info.p_callback)
    {
    	while (false == p_ctrl->transaction_completed)
        {
            /* The transfer descriptor is updated during interrupt processing */
        }

        if (p_ctrl->err)
        {
            err = SSP_ERR_ABORTED;
        }
    }
    return err;
}

/*******************************************************************************************************************//**
 * Sets interrupt priority and initializes vector info for the callback
 * @param[in]     ipl        Interrupt priority level
 * @param[in]     p_ctrl     Pointer to driver control block
 * @param[in]     p_irq      Pointer to IRQ for this signal
 **********************************************************************************************************************/
static void sci_siic_set_irq_parameters(uint8_t ipl, void * p_ctrl, IRQn_Type * p_irq)
{
    ssp_vector_info_t * p_vector_info;
    /** Set the interrupt priority for the specified interrupt number(*p_irq)*/
    NVIC_SetPriority(*p_irq, ipl);
    /** Set the p_ctrl into the vector_info structure for the element. This information
     * will be used by the ISR to determine the p_ctrl to be used in the ISR processing. */
    R_SSP_VectorInfoGet(*p_irq, &p_vector_info);
    *(p_vector_info->pp_ctrl) = p_ctrl;
}

/*******************************************************************************************************************//**
 * Sets interrupt priority and initializes vector info to be used in the callback. Then, Disables and clears
 * interrupts in the NVIC and the peripheral.
 * @param[in]     p_feature  SSP feature
 * @param[in]     p_api_ctrl Pointer to driver control block
 * @param[in]     p_cfg      Pointer to driver configuration block
 *
 * @retval        SSP_SUCCESS                Interrupt enabled
 * @retval        SSP_ERR_IRQ_BSP_DISABLED   Interrupt does not exist in the vector table
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                   * fmi_api_t::eventInfoGet
 **********************************************************************************************************************/
static ssp_err_t sci_siic_configure_irqs(ssp_feature_t * p_feature,i2c_ctrl_t * const p_api_ctrl, i2c_cfg_t  const * const p_cfg )
{
    ssp_err_t err = SSP_SUCCESS;
    sci_i2c_instance_ctrl_t * p_ctrl = (sci_i2c_instance_ctrl_t *) p_api_ctrl;

    R_SCI0_Type * p_sci_reg = (R_SCI0_Type *) p_ctrl->p_reg;
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    /** Get the interrupt vector number for SSP_SIGNAL_SCI_RXI */
    err = g_fmi_on_fmi.eventInfoGet(p_feature, SSP_SIGNAL_SCI_RXI, &event_info);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    /** Save the interrupt vector number to the control block */
    p_ctrl->rxi_irq = event_info.irq;
    /** Get the interrupt vector number for SSP_SIGNAL_SCI_TXI */
    err = g_fmi_on_fmi.eventInfoGet(p_feature, SSP_SIGNAL_SCI_TXI, &event_info);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    /** Save the interrupt vector number to the control block */
    p_ctrl->txi_irq = event_info.irq;
    /** Get the interrupt vector number for SSP_SIGNAL_SCI_TEI */
    err = g_fmi_on_fmi.eventInfoGet(p_feature, SSP_SIGNAL_SCI_TEI, &event_info);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    /** Save the interrupt vector number to the control block */
    p_ctrl->tei_irq = event_info.irq;

    sci_siic_set_irq_parameters(p_cfg->rxi_ipl, p_ctrl, &p_ctrl->rxi_irq);
    sci_siic_set_irq_parameters(p_cfg->txi_ipl, p_ctrl, &p_ctrl->txi_irq);
    sci_siic_set_irq_parameters(p_cfg->tei_ipl, p_ctrl, &p_ctrl->tei_irq);

    /* Disable all interrupts and clear all pending interrupts */
    HW_SCI_RxIrqDisable(p_sci_reg, p_ctrl);    /** Disable the interrupt in the NVIC and the peripheral*/
    R_BSP_IrqStatusClear (p_ctrl->rxi_irq);    /** Clear the IR flag in the ICU */
    NVIC_ClearPendingIRQ (p_ctrl->rxi_irq);    /** Clear interrupt flag in the NVIC */

    HW_SCI_TxIrqDisable(p_sci_reg, p_ctrl);    /** Disable the interrupt in the NVIC and the peripheral*/
    R_BSP_IrqStatusClear (p_ctrl->txi_irq);    /** Clear the IR flag in the ICU */
    NVIC_ClearPendingIRQ (p_ctrl->txi_irq);    /** Clear interrupt flag in the NVIC */

    HW_SCI_TeIrqDisable(p_sci_reg, p_ctrl);    /** Disable the interrupt in the NVIC and the peripheral*/
    R_BSP_IrqStatusClear (p_ctrl->tei_irq);    /** Clear the IR flag in the ICU */
    NVIC_ClearPendingIRQ (p_ctrl->tei_irq);    /** Clear interrupt flag in the NVIC */
    return SSP_SUCCESS;
}
/******************************************************************************************************************//**
 * @brief   ISR for ACK/RXI interrupt
 *
 **********************************************************************************************************************/
void sci_i2c_rxi_isr(void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE;

    /* Clear pending IRQ */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    /* Get the context information from the  interrupt vector */
    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    sci_i2c_instance_ctrl_t * p_ctrl = (sci_i2c_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    /* Call the handler */
    sci_siic_rxi_handler(p_ctrl);

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE;
}
/******************************************************************************************************************//**
 * @brief   ISR for NACK/TXI interrupt
 *
 **********************************************************************************************************************/
void sci_i2c_txi_isr(void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE;

    /* Clear pending IRQ */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    /* Get the context information from the  interrupt vector */
    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    sci_i2c_instance_ctrl_t * p_ctrl = (sci_i2c_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    /* Call the handler */
    sci_siic_txi_handler(p_ctrl);

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE;
}

/******************************************************************************************************************//**
 * @brief   Handles the STI interrupt
 *
 *********************************************************************************************************************/
void sci_i2c_tei_isr(void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE;

    /* Get the context information from the  interrupt vector */
    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    sci_i2c_instance_ctrl_t * p_ctrl = (sci_i2c_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    /* Call the handler */
    sci_siic_tei_handler(p_ctrl);

    /* Clear pending IRQ */
    R_BSP_IrqStatusClear(R_SSP_CurrentIrqGet());

    /* Give time for value to be updated */
    volatile uint32_t dummy;
    dummy = R_ICU->IELSRn[(uint32_t)R_SSP_CurrentIrqGet()];
    SSP_PARAMETER_NOT_USED(dummy);

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE;
}

/******************************************************************************************************************//**
 * @brief   Handles the NACK/TXI interrupt .
 *
 * @param[in]       p_ctrl     pointer to the I2C control block.
 **********************************************************************************************************************/

static void sci_siic_txi_handler(sci_i2c_instance_ctrl_t      * const p_ctrl)
{
    R_SCI0_Type * p_sci_reg = (R_SCI0_Type *) p_ctrl->p_reg;

    /* This is TXI interrupt and ACK is detected  */
    if((!HW_SCI_SIIC_InterruptSourceIsACKNACK(p_sci_reg)) && (HW_SCI_SIIC_AckReceived(p_sci_reg)))
    {
        /* 10 bit address, transfer the remaining address or generate RESTART */
        if (p_ctrl->addr_total != p_ctrl->addr_loaded)
        {
            /* Issue a RESTART as this is 10 bit address read and we have already transmitted the LSB */
            if((p_ctrl->read) && (2U == p_ctrl->addr_loaded))
            {
                HW_SCI_SIIC_SendRestart(p_sci_reg);
            }
            /* Transmit the LSB of the address */
            else
            {
                HW_SCI_Write(p_sci_reg, p_ctrl->addr_low);

                /* Update tracker that we have completed an address byte */
                p_ctrl->addr_remain--;
                p_ctrl->addr_loaded++;
            }
        }
        /* This is the first interrupt after the completion of DTC operation, ignore it */
        else if(true == p_ctrl->activation_on_txi)
        {
            p_ctrl->activation_on_txi = false;
        }
        /* Transmit/Receive data */
        else if (0U < p_ctrl->remain)
        {
            sci_siic_txi_send_data(p_ctrl, p_sci_reg);
        }
        /* All transfers complete, send the required bus condition */
        else
        {
            if(p_ctrl->restart)
            {
                HW_SCI_SIIC_SendRestart(p_sci_reg);
                p_ctrl->restarted = true;
            }
            else
            {
                HW_SCI_SIIC_SendStop(p_sci_reg);
            }
        }
    }
    /* NACK detected: Either a NACK interrupt or TXI interrupt with NACK flag set */
    else
    {
        sci_siic_txi_process_nack(p_ctrl);
    }

}

/******************************************************************************************************************//**
 * @brief   Handles the ACK/RXI interrupt .
 *
 * @param[in]       p_ctrl     pointer to the I2C control block.
 **********************************************************************************************************************/
static void sci_siic_rxi_handler(sci_i2c_instance_ctrl_t      * const p_ctrl)
{
    R_SCI0_Type * p_sci_reg = (R_SCI0_Type *) p_ctrl->p_reg;

    /* This is an ACK interrupt, Only used for DTC assisted master writes */
    if(HW_SCI_SIIC_InterruptSourceIsACKNACK(p_sci_reg))
    {
        /* This is a 10 master write, send the low address byte */
        if (p_ctrl->addr_total != p_ctrl->addr_loaded)
        {
                sci_siic_enable_transfer_support_tx(p_ctrl);
                HW_SCI_Write(p_sci_reg, p_ctrl->addr_low);

                /* Update tracker that we have completed an address byte */
                p_ctrl->addr_remain--;
                p_ctrl->addr_loaded++;
        }
        /* Interrupt after the DTC transfer, ignore it */
        else if(true == p_ctrl->activation_on_rxi)
        {
            p_ctrl->activation_on_rxi = false;
        }
        /* Data transmission is complete, send STOP or RESTART as requested by the user */
        else if (0U == p_ctrl->remain)
        {
            /* Since this is an ACK interrupt, we are currently on the rising edge of the 9th clock pulse.
                 Wait for the falling edge before generating any bus condition */
            HW_SCI_TimedWaitForTEND(p_sci_reg);
            if(p_ctrl->restart)
            {
                HW_SCI_SIIC_SendRestart(p_sci_reg);
                p_ctrl->restarted = true;
            }
            else
            {
                HW_SCI_SIIC_SendStop(p_sci_reg);
            }
        }
        else
        {
            /*Do nothing*/
        }
    }
    /* This is a RXI interrupt, this is only used when we are doing an I2C read with DTC */
    else if(true == p_ctrl->activation_on_rxi)
    {
        p_ctrl->activation_on_rxi = false;

        /* Disable RXI  interrupt */
        HW_SCI_RxIrqDisable(p_sci_reg, p_ctrl);
    }
    else
    {
        /* Do nothing */
    }
}

/******************************************************************************************************************//**
 * @brief   Handles the START/STOP interrupt .
 *
 * @param[in]       p_ctrl     pointer to the I2C control block.
 **********************************************************************************************************************/
static void sci_siic_tei_handler(sci_i2c_instance_ctrl_t      * const p_ctrl)
{
    R_SCI0_Type * p_sci_reg = (R_SCI0_Type *) p_ctrl->p_reg;

    /* Clear the interrupt flag */
    HW_SCI_SIIC_ClearSTIF(p_sci_reg);

    if(p_ctrl->err)
    {
        /* Release SCL and SDA lines */
        HW_SCI_SIIC_LineHalt(p_sci_reg);

        /* Notify the user of the error */
        sci_siic_notify(p_ctrl, I2C_EVENT_ABORTED);
    }
    /* Address transmission is not complete, this could be the START condition prior to the transmission
     *  of first address byte or RESTART condition in between a 10 bit address read */
    else if (p_ctrl->addr_total != p_ctrl->addr_loaded)
    {
        /* Check to send address */
        sci_siic_tei_send_address (p_ctrl,p_sci_reg);
    }

    /*This a STOP or RESTART after completion of the read/write operation */
    else if (0U == p_ctrl->remain)
    {
        i2c_event_t event = p_ctrl->read ? I2C_EVENT_RX_COMPLETE : I2C_EVENT_TX_COMPLETE;

        /* Set the SCL/SDA lines to the required state */
        if(p_ctrl->restarted)
        {
             /* Ready for transfer */
             HW_SCI_SIIC_LineClear(p_sci_reg);
        }
        else
        {
            /* Release SCL and SDA lines */
            HW_SCI_SIIC_LineHalt(p_sci_reg);

            /* Disable the transmitter and receiver */
            HW_SCI_TransmitterReceiverDisable(p_sci_reg);
        }

        /* Notify anyone waiting that the transfer is finished */
        sci_siic_notify(p_ctrl, event);
    }
    else
    {
        /*Do nothing*/
    }
}


/*******************************************************************************************************************//**
 * @brief Configures SCI I2C related transfer drivers (if enabled).
 *
 * @param[in]     p_ctrl   Pointer to SCI I2C specific control structure
 * @param[in]     p_cfg    Pointer to SCI I2C specific configuration structure
 *
 * @retval SSP_SUCCESS                 If configures SCI I2C related transfer drivers
 * @retval SSP_ERR_ASSERTION           Transfer configuration for tx/rx not proper.
 **********************************************************************************************************************/
static ssp_err_t    sci_siic_open_transfer_interface     (sci_i2c_instance_ctrl_t      * const p_ctrl,
                                                          i2c_cfg_t const * const p_cfg)
{
    ssp_err_t result_rx = SSP_SUCCESS;
    ssp_err_t result_tx = SSP_SUCCESS;

    R_SCI0_Type * p_sci_reg = (R_SCI0_Type *) p_ctrl->p_reg;
    if (NULL != p_cfg->p_transfer_rx)
    {
        result_rx = sci_siic_transfer_configure_rx(p_ctrl, p_cfg, p_sci_reg);
        SCI_SIIC_ERROR_RETURN(SSP_SUCCESS == result_rx, result_rx);
    }

    if (NULL != p_cfg->p_transfer_tx)
    {
        result_tx = sci_siic_transfer_configure_tx(p_ctrl, p_cfg, p_sci_reg);

        /* If TX transfer instance configuration is failed and RX transfer instance is not NULL,
         * then close the RX transfer instance which is already open. */
        if (SSP_SUCCESS != result_tx)
        {
            if (NULL != p_cfg->p_transfer_rx)
            {
                p_cfg->p_transfer_rx->p_api->close(p_cfg->p_transfer_rx->p_ctrl);
            }
            return result_tx;
        }
    }
    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief   Stop any running instance of the transfer interface
 *
 * @param[in]  p_ctrl     transfer control block
 **********************************************************************************************************************/
static void  sci_siic_stop_transfer_interface(sci_i2c_instance_ctrl_t      * const p_ctrl)
{
    ssp_err_t err;
    transfer_properties_t info;
    info.in_progress = false;

    const transfer_instance_t * p_transfer_rx = p_ctrl->p_transfer_rx;
    if(NULL != p_transfer_rx)
    {
        err = p_transfer_rx->p_api->infoGet(p_transfer_rx->p_ctrl, &info);
        if((SSP_SUCCESS == err) && (info.in_progress))
        {
            p_transfer_rx->p_api->stop(p_transfer_rx->p_ctrl);
        }
    }

    const transfer_instance_t * p_transfer_tx = p_ctrl->p_transfer_tx;
    if(NULL != p_transfer_tx)
    {
        err = p_transfer_tx->p_api->infoGet(p_transfer_tx->p_ctrl, &info);
        if((SSP_SUCCESS == err) && (info.in_progress))
        {
            p_transfer_tx->p_api->stop(p_transfer_tx->p_ctrl);
        }
    }

}

/******************************************************************************************************************//**
 * @brief   Reconfigure the address mode for transfer interface
 *
 * @param[in]  p_ctrl     transfer control block
 *
 * @retval SSP_SUCCESS                       Address mode for transfer interface reconfigured.
 * @retval SSP_ERR_IN_USE                    If a transfer is in progress.
 **********************************************************************************************************************/
static ssp_err_t sci_siic_reconfigure_interrupts_for_transfer(sci_i2c_instance_ctrl_t      * const p_ctrl)
{
    ssp_err_t       err      = SSP_SUCCESS;

    const transfer_instance_t * p_transfer_rx = p_ctrl->p_transfer_rx;
    R_SCI0_Type * p_sci_reg = (R_SCI0_Type *) p_ctrl->p_reg;

    if(NULL != p_transfer_rx)
    {
        transfer_properties_t info;
        info.in_progress = false;
        p_transfer_rx->p_api->infoGet(p_transfer_rx->p_ctrl, &info);

        /* If a transfer is not in progress, re-adjust the transfer info */
        if(!info.in_progress)
        {
            transfer_info_t * p_info = p_transfer_rx->p_cfg->p_info;

            /* Disable the transmitter and receiver for reconfiguring interrupt source */
            HW_SCI_TransmitterReceiverDisable(p_sci_reg);

            /* This is a write operation */
            if(!p_ctrl->read)
            {
                /* Set the destination address to TDR and re-adjust address modes*/
                p_info->mode           = TRANSFER_MODE_NORMAL;
                p_info->src_addr_mode  = TRANSFER_ADDR_MODE_INCREMENTED;
                p_info->dest_addr_mode = TRANSFER_ADDR_MODE_FIXED;
                p_info->p_dest         = (void *) HW_SCI_SIICWriteAddrGet(p_sci_reg);

                /* Set the interrupt source to RXI/TXI */
                HW_SCI_SIIC_InterruptSource(p_sci_reg, SCI_SIIC_INTERRUPT_SOURCE_ACK_NACK);
            }
            /* This is a read operation */
            else
            {
                /* Set the source address to RDR and and re-adjust address modes */
                p_info->mode           = TRANSFER_MODE_NORMAL;
                p_info->src_addr_mode  = TRANSFER_ADDR_MODE_FIXED;
                p_info->p_src          = (void *) HW_SCI_SIICReadAddrGet(p_sci_reg);
                p_info->dest_addr_mode = TRANSFER_ADDR_MODE_INCREMENTED;

                /* Set the interrupt source to ACK/NACK */
                HW_SCI_SIIC_InterruptSource(p_sci_reg, SCI_SIIC_INTERRUPT_SOURCE_RECEPTION_TRANSMISSION);
            }

            /* Enable the transmitter and receiver */
            HW_SCI_TransmitterReceiverEnable(p_sci_reg);
        }
        else
        {
            err = SSP_ERR_IN_USE;
        }
    }
    else if(!p_ctrl->restarted)
    {
        /* Enable both transmitter and receiver */
        HW_SCI_TransmitterReceiverEnable(p_sci_reg);
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief   Enables the dtc transfer interface for the transmit operation
 *
 * @param[in]       p_ctrl  Pointer to transfer control block
***********************************************************************************************************************/
static void       sci_siic_enable_transfer_support_tx    (sci_i2c_instance_ctrl_t      * const p_ctrl)
{
    /* rx interface is mapped to ACK interrupt */
    const transfer_instance_t * p_transfer_rx = p_ctrl->p_transfer_rx;

    if((NULL != p_transfer_rx) && (!p_ctrl->read))
    {
        /* Enable transfer interface to write to TDR */
        p_transfer_rx->p_api->reset(p_transfer_rx->p_ctrl, (void *) (p_ctrl->p_buff), NULL,
                (uint16_t) (p_ctrl->remain));

        p_ctrl->remain = 0U;
        p_ctrl->loaded = p_ctrl->total;

        p_ctrl->activation_on_rxi = true;
    }
}

/*******************************************************************************************************************//**
 * @brief   Enables the dtc transfer interface for the receive operation
 *
 * @param[in]       p_ctrl  Pointer to transfer control block
***********************************************************************************************************************/
static void       sci_siic_enable_transfer_support_rx    (sci_i2c_instance_ctrl_t      * const p_ctrl)
{
    const transfer_instance_t * p_transfer_rx = p_ctrl->p_transfer_rx;
    const transfer_instance_t * p_transfer_tx = p_ctrl->p_transfer_tx;
    R_SCI0_Type * p_sci_reg = (R_SCI0_Type *) p_ctrl->p_reg;
    /* Enable the transfer interfaces if the number of bytes to be read is greater than 2.
     * The last two bytes will be read through interrupt, this is for support NACK before STOP.
     */
    if((NULL != p_transfer_tx) && (NULL != p_transfer_rx) && (p_ctrl->read) && (p_ctrl->total > 2U))
    {
        /* Enable RXI  interrupt */
        HW_SCI_RxIrqEnable(p_sci_reg, p_ctrl);

        /* Enable transfer interface for reading data from RDR */
        p_transfer_rx->p_api->reset(p_transfer_rx->p_ctrl, NULL, (void *) (p_ctrl->p_buff),
                (uint16_t) (p_ctrl->total - 2U));

        /* Enable transfer interface to do dummy write into TDR */
        p_transfer_tx->p_api->reset(p_transfer_tx->p_ctrl, NULL, NULL,(uint16_t) (p_ctrl->total - 2U));

        /* Update the tracker variables */
        p_ctrl->remain = 2U;
        p_ctrl->loaded = p_ctrl->total - 2U;

        /* Mark DTC activation for RXI and TXI */
        p_ctrl->activation_on_rxi = true;
        p_ctrl->activation_on_txi = true;
    }
}

/******************************************************************************************************************//**
 * @brief   Configures  SCI I2C RX related transfer (if enabled).
 *
 * @param[in]     p_ctrl    Pointer to SCI I2C specific control structure
 * @param[in]     p_cfg     Pointer to SCI I2C specific configuration structure
 * @param[in]     p_sci_reg Pointer to hardware registers for SCI I2C module
 *
 *@retval SSP_SUCCESS                 Configurations proper for RX related transfer.
 *@retval SSP_ERR_ASSERTION           Pointer to the  transfer instance for I2C receive in p_cfg is NULL.
 **********************************************************************************************************************/
static ssp_err_t  sci_siic_transfer_configure_rx                  (sci_i2c_instance_ctrl_t * const p_ctrl,
                                                                 i2c_cfg_t const * const p_cfg,
                                                                 R_SCI0_Type * p_sci_reg)
{
    ssp_err_t result = SSP_SUCCESS;
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    ssp_feature_t feature = {{(ssp_ip_t) 0U}};
    feature.id = SSP_IP_SCI;
    feature.channel = p_cfg->channel;
    feature.unit = 0U;

#if (SCI_SIIC_CFG_PARAM_CHECKING_ENABLE)
        SSP_ASSERT(NULL != p_cfg->p_transfer_rx->p_api);
        SSP_ASSERT(NULL != p_cfg->p_transfer_rx->p_ctrl);
        SSP_ASSERT(NULL != p_cfg->p_transfer_rx->p_cfg);
        SSP_ASSERT(NULL != p_cfg->p_transfer_rx->p_cfg->p_info);

        SSP_ASSERT(SSP_INVALID_VECTOR != p_ctrl->rxi_irq);
#endif
        /* Configure the transfer interface for rx operation and open it */
        transfer_info_t * p_info = p_cfg->p_transfer_rx->p_cfg->p_info;
        p_info->mode           = TRANSFER_MODE_NORMAL;
        p_info->dest_addr_mode = TRANSFER_ADDR_MODE_FIXED;
        p_info->p_dest         = (void *) HW_SCI_SIICWriteAddrGet(p_sci_reg);
        p_info->src_addr_mode  = TRANSFER_ADDR_MODE_INCREMENTED;
        p_info->irq            = TRANSFER_IRQ_END;
        p_info->size           = TRANSFER_SIZE_1_BYTE;

        transfer_cfg_t cfg     = *(p_cfg->p_transfer_rx->p_cfg);
        g_fmi_on_fmi.eventInfoGet(&feature, SSP_SIGNAL_SCI_RXI, &event_info);
        cfg.activation_source  = event_info.event;
        cfg.auto_enable        = false;
        cfg.p_callback         = NULL;

        result                 = p_cfg->p_transfer_rx->p_api->open(p_cfg->p_transfer_rx->p_ctrl, &cfg);
        SCI_SIIC_ERROR_RETURN((SSP_SUCCESS == result), result);
        return result;
}

/******************************************************************************************************************//**
 * @brief   Configures  SCI I2C TX related transfer (if enabled).
 *
 * @param[in]     p_ctrl    Pointer to SCI I2C specific control structure
 * @param[in]     p_cfg     Pointer to SCI I2C specific configuration structure
 * @param[in]     p_sci_reg Pointer to hardware registers for SCI I2C module
 *
 * @retval SSP_SUCCESS                 Configurations proper for TX related transfer.
 * @retval SSP_ERR_ASSERTION           Pointer to the  transfer instance for I2C transmit in p_cfg is NULL.
 **********************************************************************************************************************/
static ssp_err_t  sci_siic_transfer_configure_tx                  (sci_i2c_instance_ctrl_t * const p_ctrl,
                                                                 i2c_cfg_t const * const p_cfg,
                                                                 R_SCI0_Type * p_sci_reg)
{
    ssp_err_t result = SSP_SUCCESS;
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    ssp_feature_t feature = {{(ssp_ip_t) 0U}};
    feature.id = SSP_IP_SCI;
    feature.channel = p_cfg->channel;
    feature.unit = 0U;

#if (SCI_SIIC_CFG_PARAM_CHECKING_ENABLE)
        SSP_ASSERT(NULL != p_cfg->p_transfer_tx->p_api);
        SSP_ASSERT(NULL != p_cfg->p_transfer_tx->p_ctrl);
        SSP_ASSERT(NULL != p_cfg->p_transfer_tx->p_cfg);
        SSP_ASSERT(NULL != p_cfg->p_transfer_tx->p_cfg->p_info);
        SSP_ASSERT(SSP_INVALID_VECTOR != p_ctrl->txi_irq);
#endif
        transfer_info_t * p_info = p_cfg->p_transfer_tx->p_cfg->p_info;
        p_info->mode           = TRANSFER_MODE_NORMAL;
        p_info->src_addr_mode  = TRANSFER_ADDR_MODE_FIXED;
        p_info->p_src          = (void *)&g_dummy_write_data_for_read_op;
        p_info->dest_addr_mode = TRANSFER_ADDR_MODE_FIXED;
        p_info->p_dest         = (void *) HW_SCI_SIICWriteAddrGet(p_sci_reg);
        p_info->irq            = TRANSFER_IRQ_END;
        p_info->size           = TRANSFER_SIZE_1_BYTE;

        transfer_cfg_t cfg = *(p_cfg->p_transfer_tx->p_cfg);
        g_fmi_on_fmi.eventInfoGet(&feature, SSP_SIGNAL_SCI_TXI, &event_info);
        cfg.activation_source = event_info.event;
        cfg.auto_enable       = false;
        cfg.p_callback        = NULL;
        result                = p_cfg->p_transfer_tx->p_api->open(p_cfg->p_transfer_tx->p_ctrl, &cfg);
        return result;
}

/*******************************************************************************************************************//**
 * @brief   Check for the receive condition.
 *
 * @param[in]  p_ctrl     Pointer to transfer control block
 * @param[in]  p_sci_reg  Pointer to hardware registers for SCI I2C module
***********************************************************************************************************************/
static  void  sci_siic_txi_send_data (sci_i2c_instance_ctrl_t       * const p_ctrl,
                                                            R_SCI0_Type * p_sci_reg)
{
    /*As per the hardware manual, a byte should be written to TDR to generate SCL.
     *If we are doing an I2C read, we will write 0xFF to the TDR  */
    uint8_t data = g_dummy_write_data_for_read_op;

    /* This is a write operation, update the data byte from user buffer */
    if(!p_ctrl->read)
    {
        data = p_ctrl->p_buff[p_ctrl->loaded];

        /* Update trackers */
        p_ctrl->remain--;
        p_ctrl->loaded++;
    }
    /* This is a read operation, move data from RDR and do dummy write to TDR */
    else
    {
        /* Skip updating the buffer on first interrupt as it is the ACK of address */
        if(true == p_ctrl->do_dummy_read)
        {
            p_ctrl->do_dummy_read = false;

            /* If transfer interface is available, use it */
            sci_siic_enable_transfer_support_rx(p_ctrl);

            /* If number of bytes to be received is greater than one, set ACK transmission */
            if(1U < p_ctrl->remain)
            {
                HW_SCI_SIIC_ACKNACKEnable(p_sci_reg, false);
            }
        }
        /* Read data into the buffer */
        else
        {
            p_ctrl->p_buff[p_ctrl->loaded] = HW_SCI_Read(p_sci_reg);

            /* Update trackers */
            p_ctrl->remain--;
            p_ctrl->loaded++;
        }

        /* Enable NACK transmission prior to the reception of the last byte */
        if(1U == p_ctrl->remain)
        {
            HW_SCI_SIIC_ACKNACKEnable(p_sci_reg, true);
        }
    }

    /* Write to TDR */
    HW_SCI_Write(p_sci_reg,data);
}

/*******************************************************************************************************************//**
 * @brief   Enables transfer support while handling the tei interrupt
 *
 * @param[in] p_ctrl     Pointer to transfer control block
 * @param[in] p_sci_reg  Pointer to hardware registers for SCI I2C module
***********************************************************************************************************************/
static void        sci_siic_tei_send_address       (sci_i2c_instance_ctrl_t       * const p_ctrl,
                                                                    R_SCI0_Type * p_sci_reg)
{
    bool do_write_op = true;
    uint8_t data = 0U;

    /* This is the first address byte */
    if (0U == p_ctrl->addr_loaded)
    {
        /* Enable transfer support for tx if this is the last address byte */
        if(1U == p_ctrl->addr_total)
        {
            sci_siic_enable_transfer_support_tx(p_ctrl);
        }

        /* if 7 bit addressing, send LSB else send MSB of address word */
        data = (1U == p_ctrl->addr_total) ? p_ctrl->addr_low : p_ctrl->addr_high;
    }
    /* This is a 10 bit read operation, issue the address as per the protocol */
    else if((p_ctrl->read) && (p_ctrl->addr_loaded == 2U))
    {
        /* Write address MSB with R/W bit set to read */
        data =  p_ctrl->addr_high | (uint8_t) I2C_CODE_READ;
    }
    /* Shouldn't have come here, do nothing */
    else
    {
        do_write_op = false;
    }

    if(do_write_op)
    {
        /* Clear the SDA/SCL line for data transmission */
        HW_SCI_SIIC_LineClear(p_sci_reg);

        /* Write the data to TDR for transmission */
        HW_SCI_Write(p_sci_reg, data);

        /* Update the number of address bytes remain for next pass */
        p_ctrl->addr_remain--;
        p_ctrl->addr_loaded++;
    }
}

/*******************************************************************************************************************//**
 * @brief   Parameter check for control block and configuration block
 *
 * @param[in] p_ctrl     Pointer to transfer control block
 * @param[in] p_cfg      Pointer to driver configuration block
 *
 * @retval  SSP_SUCCESS                 If parameter p_ctrl, p_cfg or clock rate not NULL.
 * @retval  SSP_ERR_ASSERTION           The parameter p_ctrl or p_cfg is NULL or if clock rate greater than 400KHz.
***********************************************************************************************************************/
static ssp_err_t sci_siic_open_parameter_check(sci_i2c_instance_ctrl_t * const p_ctrl,i2c_cfg_t  const   * const p_cfg)
{

    SSP_ASSERT(p_ctrl != NULL);
    SSP_ASSERT(p_cfg != NULL);
    SSP_ASSERT((p_cfg->rate == I2C_RATE_STANDARD) || (p_cfg->rate == I2C_RATE_FAST));

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   Process NACK reception within TXI interrupt
 *
 * @param[in] p_ctrl     Pointer to transfer control block
***********************************************************************************************************************/
static void sci_siic_txi_process_nack(sci_i2c_instance_ctrl_t       * const p_ctrl)
{
    R_SCI0_Type * p_sci_reg = (R_SCI0_Type *) p_ctrl->p_reg;

    /* Wait for TEND flag to be set. If interrupt is TXI, this will immediately return.
     * If interrupt is NACK, this will spin around for half I2C clock period. This is required
     * to ensure that a bus condition is generated when the SCL is low.
     */
    HW_SCI_TimedWaitForTEND(p_sci_reg);

    /* If we are doing master write, or address is NACKed during read operation, this is an error condition, set error and send stop  */
    if((!p_ctrl->read) || (true == p_ctrl->do_dummy_read))
    {
    	p_ctrl->remain = 0U;
    	p_ctrl->err = true;
        HW_SCI_SIIC_SendStop(p_sci_reg);
    }
    /* If we are doing master read, this marks the end of transaction, read the last byte from RDR  before
     * generating the required bus condition */
    else
    {
        /* Load the RDR into user buffer */
        p_ctrl->p_buff[p_ctrl->loaded] = HW_SCI_Read(p_sci_reg);

        /* Update trackers */
        p_ctrl->remain--;
        p_ctrl->loaded++;

        /* Send a RESTART or STOP as requested by the user */
        if(p_ctrl->restart)
        {
            HW_SCI_SIIC_SendRestart(p_sci_reg);
            p_ctrl->restarted = true;
        }
        else
        {
            HW_SCI_SIIC_SendStop(p_sci_reg);
        }
    }
}

/*******************************************************************************************************************//**
 * @brief   Parameter check for Read and Write when DTC is used for transfer.
 *
 * @param[in] p_ctrl     Pointer to transfer control block
 * @param[in] bytes      number of bytes to be transferred
 *
 * @retval  SSP_SUCCESS                 Provided length supported by DTC.
 * @retval  SSP_ERR_INVALID_SIZE        Provided number of bytes more than uint16_t size(65535) while DTC is used
 *                                      for data transfer.
***********************************************************************************************************************/
static ssp_err_t sci_siic_dtc_max_length_check(sci_i2c_instance_ctrl_t * const p_ctrl, const uint32_t bytes)
{
    ssp_err_t err = SSP_SUCCESS;
    if ((NULL != p_ctrl->p_transfer_rx) && (NULL != p_ctrl->p_transfer_tx))
    {
        uint32_t num_transfers = bytes;
        transfer_properties_t transfer_max = {0U};
        p_ctrl->info.p_transfer_rx->p_api->infoGet(p_ctrl->info.p_transfer_rx->p_ctrl, &transfer_max);
        if (num_transfers < transfer_max.transfer_length_max)
        {
            p_ctrl->info.p_transfer_tx->p_api->infoGet(p_ctrl->info.p_transfer_tx->p_ctrl, &transfer_max);
            if (num_transfers >= transfer_max.transfer_length_max)
            {
                return SSP_ERR_INVALID_SIZE;
            }
        }
        else
        {
            err = SSP_ERR_INVALID_SIZE;
        }
    }
    return err;
}
/* End of file */

