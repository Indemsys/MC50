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
 * File Name    : r_riic_slave.c
 * Description  : I2C Slave mode driver implementation on the RIIC peripheral.
 *********************************************************************************************************************/


/**********************************************************************************************************************
Includes
 *********************************************************************************************************************/
#include "r_riic_slave.h"
#include "r_riic_slave_private_api.h"
#include "r_riic_slave_private.h"
#include "r_cgc.h"

/**********************************************************************************************************************
Macro definitions
 *********************************************************************************************************************/

#define ONE_SEC_IN_NANO_SEC                 1000000000U

/* I2C data setup times as defined in HW manual */
#define I2C_STANDARD_MODE_SETUP_TIME_IN_NS  250U
#define I2C_FAST_MODE_SETUP_TIME_IN_NS      100U
#define I2C_FAST_MODE_PLUS_SETUP_TIME_IN_NS 50U

/* Analog filter time (reference) */
#define ANALOG_FILTER_TIME_IN_NS            120U

/* Ref clock multiplier */
#define REF_CLOCK_MULTIPLIER                1.5

/* Maximum number of digital filter stages */
#define MAX_NUM_FILTER_STAGES               3U

/* Variant bit for FM+ support */
#define VARIANT_FMPLUS_MASK                 0x04U

/*******************************************************************************************************************//**
 * @addtogroup RIIC_SLAVE
 * @{
 *********************************************************************************************************************/

/** Macro for error logger. */
#ifndef RIIC_SLAVE_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define RIIC_SLAVE_ERROR_RETURN(a, err)  SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_riic_slave_version)
#endif

/** "I2CS" in ASCII, used to determine if channel is open. */
#define RIIC_SLAVE_OPEN                     (0x49324353ULL)

/* Maximum divisor shift for internal reference clock */
#define MAX_CKS_DIVISOR                     7U

/* Bit depth of BRCL counter */
#define MAX_BRCL_VALUE                      31U

/**********************************************************************************************************************
Typedef definitions
 *********************************************************************************************************************/

/**********************************************************************************************************************
Private function prototypes
 *********************************************************************************************************************/
/** Internal helper functions */
static void riic_notify                (riic_slave_instance_ctrl_t  * const p_ctrl,
        i2c_event_t             const event);
static ssp_err_t riic_slave_set_irq_parameters(ssp_feature_t * p_feature, ssp_signal_t signal, uint8_t ipl,
        void * p_ctrl, IRQn_Type * p_irq);

/** Functions that manipulate hardware */
static void  riic_open_hw_slave  (riic_slave_instance_ctrl_t * const p_ctrl, ssp_feature_t * p_feature);
static void         riic_close_hw_slave (riic_slave_instance_ctrl_t * const p_ctrl);
static ssp_err_t    riic_run_hw_slave   (riic_slave_instance_ctrl_t  * const p_ctrl);
static void         riic_slave_clock_settings(riic_slave_instance_ctrl_t * const p_ctrl);
static void         riic_clear_all_pending_interrupts (riic_slave_instance_ctrl_t * const p_ctrl);
static ssp_err_t    riic_slave_configure_interrupts  (ssp_feature_t * ssp_feature,
                              riic_slave_instance_ctrl_t * const p_api_ctrl, i2c_cfg_t  const   * const p_cfg );

/** Interrupt handlers */
static void riic_rxi_slave (riic_slave_instance_ctrl_t * p_ctrl);
static void riic_txi_slave (riic_slave_instance_ctrl_t * p_ctrl);
static void riic_tei_slave (riic_slave_instance_ctrl_t * p_ctrl);
static void riic_err_slave (riic_slave_instance_ctrl_t * p_ctrl);
static void riic_rxi_read_data(riic_slave_instance_ctrl_t * p_ctrl);

/**********************************************************************************************************************
Private global variables
 *********************************************************************************************************************/
/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "riic_slave";
#endif

#if defined(__GNUC__)
/* This structure is affected by warnings from the GCC compiler bug https:/gcc.gnu.org/bugzilla/show_bug.cgi?id=60784
 * This pragma suppresses the warnings in this structure only, and will be removed when the SSP compiler is updated to
 * v5.3.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static ssp_version_t const g_riic_slave_version =
{
    .api_version_minor  = I2C_MASTER_API_VERSION_MINOR,
    .api_version_major  = I2C_MASTER_API_VERSION_MAJOR,
    .code_version_major = RIIC_SLAVE_CODE_VERSION_MAJOR,
    .code_version_minor = RIIC_SLAVE_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif


/**********************************************************************************************************************
Global variables
 *********************************************************************************************************************/

/** RIIC Implementation of I2C device slave interface */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
i2c_api_slave_t const g_i2c_slave_on_riic =
{
    .open                       = R_RIIC_SlaveOpen,
    .close                      = R_RIIC_SlaveClose,
    .masterWriteSlaveRead       = R_RIIC_MasterWriteSlaveRead,
    .masterReadSlaveWrite       = R_RIIC_MasterReadSlaveWrite,
    .versionGet                 = R_RIIC_SlaveVersionGet
};

/**********************************************************************************************************************
Functions
 *********************************************************************************************************************/

/******************************************************************************************************************//**
 * @brief   Gets version information and stores it in the provided version struct.
 *
 * @retval  SSP_SUCCESS                 Successful version get.
 * @retval  SSP_ERR_ASSERTION           p_version is NULL.
 *********************************************************************************************************************/
ssp_err_t R_RIIC_SlaveVersionGet   (ssp_version_t          * const p_version)
{
#if RIIC_SLAVE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_version != NULL);
#endif

    p_version->version_id = g_riic_slave_version.version_id;

    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief   Opens the I2C device. May power on IIC peripheral and perform initialization described in hardware manual.
 *
 * @retval  SSP_SUCCESS               Opened identical configuration of already open instance.
 * @retval  SSP_ERR_ASSERTION         p_api_ctrl or p_cfg is NULL.
 * @retval  SSP_ERR_IN_USE            Attempted to open an already open device instance.
 * @retval  SSP_ERR_IRQ_BSP_DISABLED  Interrupt does not exist in the vector table.
 * @retval  SSP_ERR_INVALID_ARGUMENT  If fast mode plus is configured and the channel does not support it
 * @return                            See @ref Common_Error_Codes for other possible return codes. This function calls
 *                                  * fmi_api_t::productFeatureGet
 *                                  * g_cgc_on_cgc.systemClockFreqGet
 *********************************************************************************************************************/
ssp_err_t R_RIIC_SlaveOpen         (i2c_ctrl_t             * const p_api_ctrl,
        i2c_cfg_t      const   * const p_cfg)
{
    riic_slave_instance_ctrl_t * p_ctrl = (riic_slave_instance_ctrl_t *) p_api_ctrl;

#if RIIC_SLAVE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl != NULL);
    SSP_ASSERT(p_cfg != NULL);

    /** p_extend not supported currently*/
    SSP_ASSERT(p_cfg->p_extend == NULL);
#endif

    ssp_err_t err;
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = p_cfg->channel;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_IIC;
    fmi_feature_info_t info = {0U};
    err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &info);
    RIIC_SLAVE_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_ctrl->p_reg = (R_IIC0_Type *) info.ptr;

    /** If rate is configured as Fast mode plus, check whether the channel supports it */
    if(I2C_RATE_FASTPLUS == p_cfg->rate)
    {
        RIIC_SLAVE_ERROR_RETURN((0U != (info.variant_data & VARIANT_FMPLUS_MASK)), SSP_ERR_INVALID_ARGUMENT);
    }

    /** Attempt to acquire hardware lock */
    err = R_BSP_HardwareLock(&ssp_feature);
    RIIC_SLAVE_ERROR_RETURN(SSP_SUCCESS == err, err);

    /* From this point on, a failure should release the hardware lock */
    err = riic_slave_configure_interrupts(&ssp_feature, p_ctrl, p_cfg);

    if (SSP_SUCCESS == err)
    {
        /* Record the configuration on the device for use later */
        p_ctrl->info = *p_cfg;

        /** Open the hardware in slave mode */
        riic_open_hw_slave(p_ctrl, &ssp_feature);

        /* Clear all status flags */
        HW_RIIC_ClearICSR2(p_ctrl->p_reg);

        /** Clear all interrupt bits */
        HW_RIIC_ClearICIER(p_ctrl->p_reg, 0xFFU);

        /* Indicate that restart and stop condition detection yet to be enabled */
        p_ctrl->start_interrupt_enabled = false;

        /* Clear any pending interrupts in NVIC and ICU */
        riic_clear_all_pending_interrupts(p_ctrl);

        /* Keep I2C peripheral out of reset state */
        HW_RIIC_Reset(p_ctrl->p_reg, false);

        /** Enable both TXI and RXI interrupt sources */
        HW_RIIC_SetICIER(p_ctrl->p_reg, (RIIC_RXI_EN_BIT | RIIC_TXI_EN_BIT));

        /* Configure the RXI ISR to trigger at the rising edge of the eighth clock cycle */
        HW_RIIC_RDRFSSet(p_ctrl->p_reg);

        /** Set ACK as slave is now ready to serve requests from master */
        HW_RIIC_SetACKTransmission(p_ctrl->p_reg);

        /** Enable all RIIC interrupts in NVIC, that need to be serviced */
        NVIC_EnableIRQ(p_ctrl->rxi_irq);
        NVIC_EnableIRQ(p_ctrl->txi_irq);
        NVIC_EnableIRQ(p_ctrl->tei_irq);
        NVIC_EnableIRQ(p_ctrl->eri_irq);

        /* Finally, we can consider the device opened */
        p_ctrl->slave_busy  = false;
        p_ctrl->p_buff      = NULL;
        p_ctrl->total       = 0U;
        p_ctrl->remain      = 0U;
        p_ctrl->loaded      = 0U;
        p_ctrl->read        = false;
        p_ctrl->err         = false;
        p_ctrl->restarted   = false;
        p_ctrl->open        = RIIC_SLAVE_OPEN;
        p_ctrl->do_dummy_read  = false;
        p_ctrl->notify_request = false;
        p_ctrl->transaction_count = 0U;
    }
    else
    {
        /** Release hardware lock on failure */
        R_BSP_HardwareUnlock(&ssp_feature);
    }

    RIIC_SLAVE_ERROR_RETURN(SSP_SUCCESS == err, err);
    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief   Closes the I2C device. Power down IIC peripheral.
 *
 * @retval  SSP_SUCCESS         Device closed without issue.
 * @retval  SSP_ERR_NOT_OPEN    Device not opened.
 * @retval  SSP_ERR_ASSERTION   p_api_ctrl is NULL.
 * @retval  SSP_ERR_ABORTED     Device was closed while a transfer was in progress.
 *********************************************************************************************************************/
ssp_err_t R_RIIC_SlaveClose        (i2c_ctrl_t             * const p_api_ctrl)
{
    riic_slave_instance_ctrl_t * p_ctrl = (riic_slave_instance_ctrl_t *) p_api_ctrl;

#if RIIC_SLAVE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl != NULL);
#endif

    /** Check if the device is even open, return an error if not */
    RIIC_SLAVE_ERROR_RETURN(RIIC_SLAVE_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    /* The device is now considered closed */
    p_ctrl->open = 0U;

    /** De-configure everything. */
    riic_close_hw_slave(p_ctrl);

    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief   Performs a read from the I2C Master device.
 *
 *  This function will fail if there is already an in-progress I2C transfer on the associated channel. Otherwise, the
 *  I2C read operation will begin. When no callback is provided by the user, this function performs a blocking read.
 *  Otherwise, the read operation is non-blocking and the caller will be notified when the operation has finished by
 *  an I2C_EVENT_RX_COMPLETE in the callback.
 *
 * @retval  SSP_SUCCESS        Function executed without issue; if no callback was provided, the process was kicked off
 * @retval  SSP_ERR_ASSERTION  p_api_ctrl, bytes or p_dest is NULL.
 * @retval  SSP_ERR_IN_USE     Another transfer was in progress.
 * @retval  SSP_ERR_NOT_OPEN   device is not open.
 * @retval  SSP_ERR_ABORTED    If transaction encounter an error.
 *********************************************************************************************************************/
ssp_err_t R_RIIC_MasterWriteSlaveRead         (i2c_ctrl_t             * const p_api_ctrl,
        uint8_t                * const p_dest,
        uint32_t                 const bytes)
{
    riic_slave_instance_ctrl_t * p_ctrl = (riic_slave_instance_ctrl_t *) p_api_ctrl;

#if RIIC_SLAVE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl != NULL);
    SSP_ASSERT(p_dest != NULL);
    SSP_ASSERT(bytes != 0U);
#endif

    ssp_err_t err = SSP_SUCCESS;

    /** Check if the device is open, return an error if not */
    RIIC_SLAVE_ERROR_RETURN(RIIC_SLAVE_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    /** Return an error if transfer is in progress. */
    if (true == p_ctrl->slave_busy)
    {
        err = SSP_ERR_IN_USE;
    }
    else
    {
        /** Record the new information about this transfer */
        p_ctrl->p_buff  = p_dest;
        p_ctrl->total   = bytes;
        p_ctrl->read    = true;

        /** Start the read operation as a slave */
        err = riic_run_hw_slave(p_ctrl);
    }

    RIIC_SLAVE_ERROR_RETURN(SSP_SUCCESS == err, err);
    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief   Performs a write to the I2C Master device.
 *
 *  This function will fail if there is already an in-progress I2C transfer on the associated channel. Otherwise, the
 *  I2C write operation will begin. When no callback is provided by the user, this function performs a blocking write.
 *  Otherwise, the write operation is non-blocking and the caller will be notified when the operation has finished by
 *  an I2C_EVENT_TX_COMPLETE in the callback.
 *
 * @retval  SSP_SUCCESS        Function executed without issue; if no callback was provided, the process was kicked off
 * @retval  SSP_ERR_ASSERTION  p_api_ctrl or p_src is NULL.
 * @retval  SSP_ERR_IN_USE     Another transfer was in progress.
 * @retval  SSP_ERR_NOT_OPEN   device is not open.
 * @retval  SSP_ERR_ABORTED    If transaction encounter an error.
 *********************************************************************************************************************/
ssp_err_t R_RIIC_MasterReadSlaveWrite        (i2c_ctrl_t             * const p_api_ctrl,
        uint8_t                * const p_src,
        uint32_t                 const bytes)
{
    riic_slave_instance_ctrl_t * p_ctrl = (riic_slave_instance_ctrl_t *) p_api_ctrl;

#if RIIC_SLAVE_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_ctrl != NULL);
    SSP_ASSERT(p_src != NULL);
#endif

    ssp_err_t err = SSP_SUCCESS;

    /** Check if the device is open, return an error if not */
    RIIC_SLAVE_ERROR_RETURN(RIIC_SLAVE_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    /** Return an error if transfer is in progress */
    if (true == p_ctrl->slave_busy)
    {
        err = SSP_ERR_IN_USE;
    }
    else
    {
        /** Record the new information about this transfer */
        p_ctrl->p_buff  = p_src;
        p_ctrl->total   = bytes;
        p_ctrl->remain  = bytes;
        p_ctrl->read    = false;

        /** Start the write operation as a slave */
        err = riic_run_hw_slave(p_ctrl);
    }

    RIIC_SLAVE_ERROR_RETURN(SSP_SUCCESS == err, err);
    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @} (end addtogroup RIIC_SLAVE)
 *********************************************************************************************************************/

/**********************************************************************************************************************
Private Functions
 *********************************************************************************************************************/

/******************************************************************************************************************//**
 * @brief   Single point for managing the logic around notifying a transfer has finished.
 *
 * @param[in]       p_ctrl  Pointer to transfer that is ending.
 * @param[in]       event       The event code to pass to the callback.
 *********************************************************************************************************************/
static void        riic_notify             (riic_slave_instance_ctrl_t * const p_ctrl,
        i2c_event_t            const event)
{
    /* Disable all the interrupt sources. */
    HW_RIIC_ClearICIER(p_ctrl->p_reg, 0xFFU);

    /** Enable only the RXI and TXI interrupt sources */
    HW_RIIC_SetICIER(p_ctrl->p_reg, (RIIC_RXI_EN_BIT | RIIC_TXI_EN_BIT));

    /* Reset the status flags */
    p_ctrl->notify_request = false;
    p_ctrl->do_dummy_read  = false;

    /* Check if the transaction ended with a restart */
    if (p_ctrl->restarted)
    {
        /* Clear all status flags */
        HW_RIIC_ClearICSR2(p_ctrl->p_reg);
    }
    /* Transaction ended with STOP or the transaction resulted in error */
    else
    {
        /* Reset the bus */
        HW_RIIC_Reset(p_ctrl->p_reg, true);
        HW_RIIC_Reset(p_ctrl->p_reg, false);
    }

    /** If this was a valid transaction and user has configured a callback, invoke it */
    if (NULL != p_ctrl->info.p_callback)
    {
        /* Fill in the argument to the callback */
        i2c_callback_args_t args =
        {
            .p_context  = p_ctrl->info.p_context,
            .bytes      = p_ctrl->transaction_count,
            .event      = event
        };

        /* Invoke the callback */
        p_ctrl->info.p_callback(&args);
    }

    /* Reset the transaction count here */
    p_ctrl->transaction_count = 0U;

    /* Set the flag indicating that the transaction is completed */
    p_ctrl->slave_busy = false;
}

/******************************************************************************************************************//**
 * @brief   Performs the hardware initialization sequence when operating as a slave.
 *
 * @param[in]       p_ctrl     Pointer to control struct of specific device
 * @param[in]       p_feature  Pointer to module specific ssp features
 *********************************************************************************************************************/
static void  riic_open_hw_slave  (riic_slave_instance_ctrl_t * const p_ctrl, ssp_feature_t * p_feature)
{
    /* Perform the first part of the initialization sequence */
    R_BSP_ModuleStart(p_feature);

    /** Set the IIC module in IIC reset */
    HW_RIIC_Output(p_ctrl->p_reg, false);

    /* Move to internal reset */
    HW_RIIC_Reset(p_ctrl->p_reg, true);

    /** Set the I2C pins to active state */
    HW_RIIC_Output(p_ctrl->p_reg, true);

    /** Clear any pending interrupts with the MCU */
    riic_clear_all_pending_interrupts(p_ctrl);

    /* Set Slave address in SARLx and SARUx. and Set ICSER */
    HW_RIIC_SetupSlaveAddress (p_ctrl->p_reg, 0U, p_ctrl->info.addr_mode, p_ctrl->info.slave);

    /* Set the clock and slope circuits to match the data setup time specified by the I2C standard */
    riic_slave_clock_settings(p_ctrl);

    /* Enable FM+ slope circuit if fast mode plus is enabled */
    if(I2C_RATE_FASTPLUS == p_ctrl->info.rate)
    {
        HW_RIIC_FMPSlopeCircuit(p_ctrl->p_reg, true);
    }

    /* Allow timeouts to be generated on the low value of SCL using long count mode */
    HW_RIIC_TimeoutCfg(p_ctrl->p_reg, RIIC_TIMEOUT_ON_SCL_LOW, false);

    /* Disable master arbitration loss detection */
    HW_RIIC_EnableMALE(p_ctrl->p_reg, false);

    /* Enable NACK arbitration loss detection */
    HW_RIIC_EnableNALE(p_ctrl->p_reg, true);

    /* Enable Slave arbitration loss detection */
    HW_RIIC_EnableSALE(p_ctrl->p_reg, true);

    /* Enable NACK reception transfer suspension */
    HW_RIIC_EnableNACKE(p_ctrl->p_reg, true);

    /** Set NACK as slave is not ready to serve any request from master */
    HW_RIIC_SetNACKTransmission(p_ctrl->p_reg);
}

/******************************************************************************************************************//**
 * @brief   Performs the hardware initialization sequence when operating as a slave.
 *
 * @param[in]       p_ctrl  Pointer to control struct of specific device
 *********************************************************************************************************************/
static void        riic_close_hw_slave    (riic_slave_instance_ctrl_t * const p_ctrl)
{
    /* Clear the slave address */
    HW_RIIC_ClearSlaveAddress(p_ctrl->p_reg, 0U);

    /** Clear all interrupt bits */
    HW_RIIC_ClearICIER(p_ctrl->p_reg, 0xFFU);

    /** Disable all interrupts in NVIC */
    NVIC_DisableIRQ(p_ctrl->eri_irq);
    NVIC_DisableIRQ(p_ctrl->rxi_irq);
    NVIC_DisableIRQ(p_ctrl->txi_irq);
    NVIC_DisableIRQ(p_ctrl->tei_irq);

    /* Shutdown the hardware to save some power */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.id = SSP_IP_IIC;
    ssp_feature.channel = p_ctrl->info.channel;
    ssp_feature.unit = 0U;
    R_BSP_ModuleStop(&ssp_feature);

    /** Release hardware lock. */
    R_BSP_HardwareUnlock(&ssp_feature);
}

/******************************************************************************************************************//**
 * @brief   Enables and assigns the interrupts to be used in slave mode.
 *
 * @param[in]       p_ctrl  Pointer to control struct of specific device
 *********************************************************************************************************************/
static void         riic_clear_all_pending_interrupts(riic_slave_instance_ctrl_t * const p_ctrl)
{
    /** Clear any pending IRQs from the NVIC and ICU */
    R_BSP_IrqStatusClear(p_ctrl->eri_irq);
    NVIC_ClearPendingIRQ(p_ctrl->eri_irq);
    R_BSP_IrqStatusClear(p_ctrl->rxi_irq);
    NVIC_ClearPendingIRQ(p_ctrl->rxi_irq);
    R_BSP_IrqStatusClear(p_ctrl->txi_irq);
    NVIC_ClearPendingIRQ(p_ctrl->txi_irq);
    R_BSP_IrqStatusClear(p_ctrl->tei_irq);
    NVIC_ClearPendingIRQ(p_ctrl->tei_irq);
}

/******************************************************************************************************************//**
 * @brief   Performs the data transfer described by the parameters when operating as a slave.
 *
 * @param[in]       p_ctrl  Pointer to transfer that needs to be done.
 *
 * @retval   SSP_SUCCESS       Transaction completed successfully.
 * @retval   SSP_ERR_ABORTED   If transaction encounter an error.
 *********************************************************************************************************************/
static          ssp_err_t   riic_run_hw_slave      (riic_slave_instance_ctrl_t    * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;

    /** Initialize fields used during transfer */
    p_ctrl->loaded                  = 0U;
    p_ctrl->remain                  = p_ctrl->total;
    p_ctrl->err                     = false;
    p_ctrl->slave_busy              = true;
    p_ctrl->restarted               = false;

    /* Indicate that restart and stop condition detection yet to be enabled */
    p_ctrl->start_interrupt_enabled = false;

    /** Clear any pending interrupts in NVIC and ICU */
    riic_clear_all_pending_interrupts(p_ctrl);

    /** Check if callback is registered, if not block till transfer ends */
    if (NULL == p_ctrl->info.p_callback)
    {
        /* Note: There is a hardware timeout that will allow this loop to exit */
        while (true == p_ctrl->slave_busy)
        {
            /* The transfer descriptor is updated during interrupt processing */
        }

        /* If transaction failed with error, return error */
        if (p_ctrl->err)
        {
            err = SSP_ERR_ABORTED;
        }
    }

    return err;
}

/******************************************************************************************************************//**
 * Sets interrupt priority and initializes vector info
 * @param[in]     p_feature  SSP feature
 * @param[in]     signal     SSP signal ID
 * @param[in]     ipl        Interrupt priority level
 * @param[in]     p_ctrl     Pointer to driver control block
 * @param[out]    p_irq      Pointer to IRQ for this signal, set here
 *
 * @retval        SSP_SUCCESS               Interrupt enabled
 * @retval        SSP_ERR_IRQ_BSP_DISABLED  Interrupt does not exist in the vector table
 * @return                            See @ref Common_Error_Codes for other possible return codes. This function calls
 *                                  * fmi_api_t::eventInfoGet
 *********************************************************************************************************************/
static ssp_err_t riic_slave_set_irq_parameters(ssp_feature_t * p_feature, ssp_signal_t signal, uint8_t ipl,
        void * p_ctrl, IRQn_Type * p_irq)
{
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    ssp_vector_info_t * p_vector_info;
    ssp_err_t err = g_fmi_on_fmi.eventInfoGet(p_feature, signal, &event_info);
    *p_irq = event_info.irq;
    if (SSP_SUCCESS == err)
    {
        NVIC_SetPriority(*p_irq, ipl);
        R_SSP_VectorInfoGet(*p_irq, &p_vector_info);
        *(p_vector_info->pp_ctrl) = p_ctrl;
    }

    return err;
}

/******************************************************************************************************************//**
 * @brief   Sets the I2C clock and BRCL counter to a value greater than the operation mode setup time
 *
 * @param[in]   p_ctrl     Pointer to driver control block
 *********************************************************************************************************************/
static void riic_slave_clock_settings(riic_slave_instance_ctrl_t * const p_ctrl)
{
    uint32_t pclk_frequency = 0U;
    uint32_t setup_time_in_ns = 0U;
    uint32_t clock_divisor_shift = 0U;
    uint32_t clock_period_in_ns = 0U;
    uint32_t brcl_value = MAX_BRCL_VALUE + 1U;
    uint32_t brcl_time_in_ns = 0U;
    uint8_t digital_filter_stages = 0U;

    /* Refer to the hardware manual for the setup times */
    if (I2C_RATE_FASTPLUS == p_ctrl->info.rate)
    {
        setup_time_in_ns = I2C_FAST_MODE_PLUS_SETUP_TIME_IN_NS;
    }
    else if (I2C_RATE_FAST == p_ctrl->info.rate)
    {
        setup_time_in_ns = I2C_FAST_MODE_SETUP_TIME_IN_NS;
    }
    else
    {
        setup_time_in_ns = I2C_STANDARD_MODE_SETUP_TIME_IN_NS;
    }

    /* get RIIC peripheral clock frequency, this will error out only on giving an invalid clock. Since
     * clock is hard-coded here we are not explicitly checking for error */
    g_cgc_on_cgc.systemClockFreqGet(CGC_SYSTEM_CLOCKS_PCLKB, &pclk_frequency);

    do
    {
        /* Get the clock period in ns */
        clock_period_in_ns = (ONE_SEC_IN_NANO_SEC / (pclk_frequency / (1U << clock_divisor_shift)));

        /* If the PCLK frequency is greater than 1 GHz (Current max on synergy MCU is 120 MHz),
         * clock_period_in_ns will be zero. since clock_period_in_ns
         * is used as a divisor, we need to check for zero. */
        if(clock_period_in_ns > 0U)
        {
            /* Calculate the time that should be represented by brcl */
            brcl_time_in_ns = setup_time_in_ns + ANALOG_FILTER_TIME_IN_NS +
                    (uint32_t)(REF_CLOCK_MULTIPLIER * clock_period_in_ns);

            /* Get the minimum required BRCL value (division with round up) */
            brcl_value = (((brcl_time_in_ns  + clock_period_in_ns) - 1U) / clock_period_in_ns);

        }

        /* divide the clock once and try again */
        clock_divisor_shift++;

        /* NOTE: Minimum time period these calculations can handle is 1 ns which is greater than the
         * periods possible on current Synergy MCUs. Maximum setup time is required for 100 KHz operation which is
         * 250 ns as per the I2C manual. To achieve this value, the number of times the clock need to be divided is
         * always less than the maximum possible dividers. Since these values are not dynamically changed, we need
         * not check for the validity of the clock divisor (doing so would result in dead code).
         */

    }while (brcl_value > MAX_BRCL_VALUE);

    clock_divisor_shift--;
    HW_RIIC_SetBRCL(p_ctrl->p_reg, (uint8_t)brcl_value);
    HW_RIIC_SetCKS(p_ctrl->p_reg, (uint8_t)clock_divisor_shift);

    /* Set the digital filter */
    digital_filter_stages = (uint8_t)(brcl_value / MAX_NUM_FILTER_STAGES);
    digital_filter_stages = (3U < digital_filter_stages) ? 3U : digital_filter_stages;
    HW_RIIC_EnableNFE(p_ctrl->p_reg, digital_filter_stages);
}

/******************************************************************************************************************//**
 * @brief   Handles the receive data full interrupt when operating as a slave.
 *
 * @param[in]       p_ctrl     The target RIIC block's control block.
 *********************************************************************************************************************/
static void riic_rxi_slave (riic_slave_instance_ctrl_t * p_ctrl)
{
    /** Perform dummy read after an address match detection. */
    if (!p_ctrl->do_dummy_read)
    {
        p_ctrl->do_dummy_read = true;
        volatile uint8_t dummy = HW_RIIC_ReadByte(p_ctrl->p_reg);
        (void)dummy;

        /** If address match detected, enable STOP detection */
        if(HW_RIIC_SlaveAddressMatchFlag(p_ctrl->p_reg))
        {
            /* Clear the Stop condition flag */
            HW_RIIC_ClearStopBitFlag(p_ctrl->p_reg);
        }
        /* Enable the Stop condition detection interrupt */
        HW_RIIC_SetICIER(p_ctrl->p_reg, RIIC_STP_EN_BIT);
    }
    else
    {
        /** Perform the read when data byte received */
        riic_rxi_read_data(p_ctrl);
    }
}

/******************************************************************************************************************//**
  * @brief   Handles the receive data full interrupt by reading data when operating as a slave.
  *
  * @param[in]       p_ctrl     The target RIIC block's control block.
  *********************************************************************************************************************/
static void riic_rxi_read_data(riic_slave_instance_ctrl_t * p_ctrl)
{
    /** Check if the read request event has been notified through callback, if not provide the callback */
    if (!p_ctrl->notify_request)
    {
        /* Set the status flag to ensure this conditional clause execution only
         * once, after the first data byte has received */
        p_ctrl->notify_request = true;

        /* Buffer the present value of the busy status indication flag */
        volatile bool status_buff = p_ctrl->slave_busy;

        /* Reset the status indication flag. This allows the API invocation to be successful with in callback */
        p_ctrl->slave_busy = false;

        /* Enable the Error ISR for servicing timeout,arbitration loss and a NACK detection conditions */
        HW_RIIC_SetICIER(p_ctrl->p_reg, (RIIC_TMO_EN_BIT | RIIC_ALD_EN_BIT | RIIC_NAK_EN_BIT));

        if (NULL != p_ctrl->info.p_callback)
        {
            /* Fill in the argument to the callback */
            i2c_callback_args_t args =
            {
                .p_context  = p_ctrl->info.p_context,
                .event      = I2C_SLAVE_EVENT_RX_REQUEST
            };

            /* Disable timeout function */
            HW_RIIC_TimeoutCfg(p_ctrl->p_reg, RIIC_TIMEOUT_OFF, false);

            /* Invoke the callback to notify the read request */
            p_ctrl->info.p_callback(&args);

            /* Allow timeouts to be generated on the low value of SCL using long count mode */
            HW_RIIC_TimeoutCfg(p_ctrl->p_reg, RIIC_TIMEOUT_ON_SCL_LOW, false);
        }

        if (!p_ctrl->slave_busy)
        {
            /* Restore the previous state of the busy status if no API invocation has been made in callback */
            p_ctrl->slave_busy = status_buff;
            /* Set the response as ACK */
            HW_RIIC_SetACKTransmission(p_ctrl->p_reg);
        }

        if (!p_ctrl->start_interrupt_enabled)
        {
            /* Enable the Start condition detection to trigger ERI ISR */
            HW_RIIC_EnableInterruptOnSTART(p_ctrl->p_reg);
            p_ctrl->start_interrupt_enabled = true;
        }
    }

    /** If read API is invoked, proceed reading data */
    if ((p_ctrl->read) && (p_ctrl->slave_busy))
    {
        /* If master is requesting still more data than configured to be read, notify
         * with a read more event in callback */
        if (p_ctrl->total == p_ctrl->loaded)
        {
            /* Reset the status indication flag. This allows the API invocation to be successful with in callback */
            p_ctrl->slave_busy = false;
            /* If this was a valid transaction and user has configured a callback, invoke it */
            if (NULL != p_ctrl->info.p_callback)
            {
                /* Fill in the argument to the callback */
                i2c_callback_args_t args =
                {
                    .p_context  = p_ctrl->info.p_context,
                    .event      = I2C_SLAVE_EVENT_RX_MORE_REQUEST,
                    .bytes      = p_ctrl->transaction_count
                };

                /* Disable timeout function */
                HW_RIIC_TimeoutCfg(p_ctrl->p_reg, RIIC_TIMEOUT_OFF, false);

                /* Invoke the callback */
                p_ctrl->info.p_callback(&args);

                /* Allow timeouts to be generated on the low value of SCL using long count mode */
                HW_RIIC_TimeoutCfg(p_ctrl->p_reg, RIIC_TIMEOUT_ON_SCL_LOW, false);
            }
            if (p_ctrl->slave_busy)
            {
                /* Read data */
                p_ctrl->p_buff[p_ctrl->loaded++] = HW_RIIC_ReadByte(p_ctrl->p_reg);

                /* Keep track of the the actual number of transactions */
                p_ctrl->transaction_count++;

                /*Set the response as ACK*/
                HW_RIIC_SetACKTransmission(p_ctrl->p_reg);
            }
            else
            {
                /* Do dummy read to release SCL */
                volatile uint8_t dummy = HW_RIIC_DummyRead(p_ctrl->p_reg);
                (void)dummy;
                /* Set the response as NACK, since there was no API invocation in the callback with event RX_MORE */
                HW_RIIC_SetNACKTransmission(p_ctrl->p_reg);
            }
        }
        else
        {
            /* Read data */
            p_ctrl->p_buff[p_ctrl->loaded++] = HW_RIIC_ReadByte(p_ctrl->p_reg);

            /* Keep track of the the actual number of transactions */
            p_ctrl->transaction_count++;

            /*Set the response as ACK*/
            HW_RIIC_SetACKTransmission(p_ctrl->p_reg);
        }
    }
    else
    {
        /* No processing, should not come here */
    }
}

/******************************************************************************************************************//**
 * @brief   Handles the transmit data empty interrupt when operating as a slave.
 *
 * @param[in]       p_ctrl     The target RIIC block's control block.
 *********************************************************************************************************************/
static void riic_txi_slave (riic_slave_instance_ctrl_t * p_ctrl)
{
    /** Check if the write request event has been notified through callback, if not provide the callback */
    if (!p_ctrl->notify_request)
    {
        /* Set the status flag to ensure this conditional clause execution only
         * once, after the address byte has received */
        p_ctrl->notify_request = true;

        /* Buffer the present value of the busy status indication flag */
        volatile bool status_buff = p_ctrl->slave_busy;

        /* Reset the status indication flag. This allows the API invocation to be successful with in callback */
        p_ctrl->slave_busy = false;

        /* Enable the Error ISR for servicing timeout,arbitration loss and a NACK detection conditions */
        HW_RIIC_SetICIER(p_ctrl->p_reg, (RIIC_TMO_EN_BIT | RIIC_ALD_EN_BIT | RIIC_NAK_EN_BIT));

        /* If this was a valid transaction and  user has configured a callback, invoke it */
        if ((NULL != p_ctrl->info.p_callback))
        {
            p_ctrl->do_dummy_read = false;
            /* Fill in the argument to the callback */
            i2c_callback_args_t args =
            {
                .p_context  = p_ctrl->info.p_context,
                .event      = I2C_SLAVE_EVENT_TX_REQUEST
            };

            /* Disable timeout function */
            HW_RIIC_TimeoutCfg(p_ctrl->p_reg, RIIC_TIMEOUT_OFF, false);

            /* Invoke the callback */
            p_ctrl->info.p_callback(&args);

            /* Allow timeouts to be generated on the low value of SCL using long count mode */
            HW_RIIC_TimeoutCfg(p_ctrl->p_reg, RIIC_TIMEOUT_ON_SCL_LOW, false);
        }
        if (!p_ctrl->slave_busy)
        {
            /* Restore the previous state of the busy status if no API invocation has been made in callback */
            p_ctrl->slave_busy = status_buff;
        }
    }
    /** If write API is invoked, proceed writing data */
    if ((!p_ctrl->read) && (p_ctrl->slave_busy))
    {
        /* Enable start interrupt to detect restart condition */
        if (!p_ctrl->start_interrupt_enabled)
        {
            HW_RIIC_EnableInterruptOnSTART(p_ctrl->p_reg);
            p_ctrl->start_interrupt_enabled = true;

            /* Address match detected, enable STOP detection */
            if(HW_RIIC_SlaveAddressMatchFlag(p_ctrl->p_reg))
            {
                /* Clear the Stop condition flag */
                HW_RIIC_ClearStopBitFlag(p_ctrl->p_reg);
            }

            /* Enable the Stop condition detection interrupt */
            HW_RIIC_SetICIER(p_ctrl->p_reg, (RIIC_STP_EN_BIT));
        }

        /* If slave has exhausted the buffer length from application, wait for slave to transmit last byte on the bus
         * and check for master is sending ACK or NACK. If master ACKs this last byte, it is expecting more data
         * from slave. Slave has to notify this event to application via callback */
        if (p_ctrl->total == p_ctrl->loaded)
        {
            /* Clear the pending interrupts for TEI */
            R_BSP_IrqStatusClear(p_ctrl->tei_irq);
            NVIC_ClearPendingIRQ(p_ctrl->tei_irq);

            /* Enable the TEI interrupt source */
            HW_RIIC_SetICIER(p_ctrl->p_reg, (uint8_t) RIIC_TEI_EN_BIT);
        }
        else
        {
            /* Write the data byte, this will also release SCL */
            HW_RIIC_WriteByte(p_ctrl->p_reg, p_ctrl->p_buff[p_ctrl->loaded]);
            p_ctrl->loaded++;
            /* Keep track of the the actual number of transactions */
            p_ctrl->transaction_count++;
        }
    }
    else
    {
        /* No processing, should not come here */
    }
}

/******************************************************************************************************************//**
 * @brief   Handles the transmission end interrupt when operating as a slave.
 *
 * @param[in]       p_ctrl     The target RIIC block's control block.
 *********************************************************************************************************************/
static void riic_tei_slave (riic_slave_instance_ctrl_t * p_ctrl)
{
    /* Check if ACK has been detected from master expecting further data */
    if (HW_RIIC_AckNackCheck(p_ctrl->p_reg))
    {
        p_ctrl->slave_busy = false;
        /* If this was a valid transaction and  user has configured a callback, invoke it */
        if (NULL != p_ctrl->info.p_callback)
        {
            /* Fill in the argument to the callback */
            i2c_callback_args_t args =
            {
                .p_context  = p_ctrl->info.p_context,
                .event      = I2C_SLAVE_EVENT_TX_MORE_REQUEST,
                .bytes      = p_ctrl->transaction_count
            };

            /* Disable timeout function */
            HW_RIIC_TimeoutCfg(p_ctrl->p_reg, RIIC_TIMEOUT_OFF, false);

            /** Invoke the callback to notify that master requests more data than configured */
            p_ctrl->info.p_callback(&args);

            if ((p_ctrl->slave_busy) && (!p_ctrl->read))
            {
                HW_RIIC_WriteByte(p_ctrl->p_reg, p_ctrl->p_buff[p_ctrl->loaded]);
                p_ctrl->loaded++;
                p_ctrl->transaction_count++;
            }

            /* Allow timeouts to be generated on the low value of SCL using long count mode */
            HW_RIIC_TimeoutCfg(p_ctrl->p_reg, RIIC_TIMEOUT_ON_SCL_LOW, false);
        }
    }
    else
    {
        /* No processing, should not come here */
    }

    /* Disable the TEND ISR */
    HW_RIIC_ClearICIER(p_ctrl->p_reg, RIIC_TEI_EN_BIT);

    /* Clear the Status flag as described in the HW manual */
    HW_RIIC_ClearTxEnd(p_ctrl->p_reg);

    /* Clear any pending interrupts in ICU and NVIC */
    R_BSP_IrqStatusClear(p_ctrl->tei_irq);
    NVIC_ClearPendingIRQ(p_ctrl->tei_irq);
}

/******************************************************************************************************************//**

 * @brief   Handles the error interrupts when operating as a slave.
 *
 * @param[in]       p_ctrl     The target RIIC block's control block.
 *********************************************************************************************************************/
static void riic_err_slave (riic_slave_instance_ctrl_t * p_ctrl)
{
    uint8_t error_events = HW_RIIC_GetICSR2(p_ctrl->p_reg);
    i2c_event_t i2c_event = I2C_EVENT_ABORTED;

    /** Check if timeout or arbitration loss detected */
    if ((error_events &  ICSR2_TMOF_BIT) || (error_events &  ICSR2_AL_BIT))
    {
        /* Clear the restarted flag */
        p_ctrl->restarted = false;

        /* Let the user know that the transaction failed on error */
        p_ctrl->err = true;
        riic_notify(p_ctrl, i2c_event);
    }
    /** Check if stop or restart condition detected, indicating a valid end of transaction */
    else if ((error_events & ICSR2_START_BIT) || (error_events & ICSR2_STOP_BIT))
    {
        /* In case of restart, set the flag */
        p_ctrl->restarted = (bool) (0 != (error_events & ICSR2_START_BIT));

        /* Set the I2C event */
        i2c_event = p_ctrl->read ? I2C_EVENT_RX_COMPLETE : I2C_EVENT_TX_COMPLETE;

        /* Decrement the transaction count when slave configured to write more data than master requested.
         * Addresses the exception raised from double buffer hardware implementation */
        if ((p_ctrl->total > p_ctrl->loaded) && (!p_ctrl->read))
        {
            p_ctrl->transaction_count -= 1U;
        }

        /* Clear the error flag */
        p_ctrl->err = false;

        /** Notify the user */
        riic_notify(p_ctrl, i2c_event);
    }
    /* NACK detected */
    else if (error_events & ICSR2_NACKF_BIT)
    {
        /* Do dummy read to release SCL */
        volatile uint8_t dummy = HW_RIIC_DummyRead(p_ctrl->p_reg);
        (void)dummy;

        /* NACK interrupt will be triggered on MasterReadSlaveWrite operation.
         * Disable NACK interrupt, this is required since we will clear NACK flag only on detection of STOP bit or
         * when a timeout occurs. Not clearing the flag will cause error interrupt to get triggered again.
         */
        HW_RIIC_ClearICIER(p_ctrl->p_reg, (uint8_t) RIIC_NAK_EN_BIT);
    }
    else
    {
        /* No processing, should not come here */
    }
}

/**********************************************************************************************************************
 * Interrupt Vectors
 *********************************************************************************************************************/

/******************************************************************************************************************//**
 * @brief   Receive data full interrupt routine.
 *
 * This function implements the RIIC Receive buffer full ISR routine.
 *
 *********************************************************************************************************************/
void iic_slave_rxi_isr  (void);
void iic_slave_rxi_isr  (void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    /* Clear the IR flag */
    R_BSP_IrqStatusClear (R_SSP_CurrentIrqGet());

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    riic_slave_instance_ctrl_t * p_ctrl = (riic_slave_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    riic_rxi_slave(p_ctrl);

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}

/******************************************************************************************************************//**
 * @brief   Transmit data empty interrupt routine.
 *
 * This function implements the Transmit buffer empty ISR routine.
 *
 *********************************************************************************************************************/
void iic_slave_txi_isr  (void);
void iic_slave_txi_isr  (void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    /* Clear the IR flag */
    R_BSP_IrqStatusClear (R_SSP_CurrentIrqGet());

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    riic_slave_instance_ctrl_t * p_ctrl = (riic_slave_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    riic_txi_slave(p_ctrl);

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}

/*******************************************************************************************************************//**
 * @brief   Transmit end interrupt routine.
 *
 * This function implements the RIIC Transmission End ISR routine.
 *
***********************************************************************************************************************/
void iic_slave_tei_isr  (void);
void iic_slave_tei_isr  (void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    riic_slave_instance_ctrl_t * p_ctrl = (riic_slave_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    riic_tei_slave(p_ctrl);

    /* Clear the IR flag */
    R_BSP_IrqStatusClear (R_SSP_CurrentIrqGet());

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}

/******************************************************************************************************************//**
 * @brief   Error and event interrupt routine.
 *
 * This function implements the RIIC Event/Error.
 *
 *********************************************************************************************************************/
void iic_slave_eri_isr (void);
void iic_slave_eri_isr (void)
{
    /* Save context if RTOS is used */
    SF_CONTEXT_SAVE

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    riic_slave_instance_ctrl_t * p_ctrl = (riic_slave_instance_ctrl_t *) *(p_vector_info->pp_ctrl);

    riic_err_slave(p_ctrl);

    /* Clear the IR flag */
    R_BSP_IrqStatusClear (R_SSP_CurrentIrqGet());

    /* Restore context if RTOS is used */
    SF_CONTEXT_RESTORE
}

/******************************************************************************************************************//**
 * Sets interrupt priority and initializes vector info
 * @param[in]     ssp_feature  SSP feature
 * @param[in]     p_ctrl       Pointer to driver control block
 * @param[out]    p_cfg        Pointer to driver configuration block
 *
 * @retval        SSP_SUCCESS               Interrupt enabled.
 * @retval        SSP_ERR_IRQ_BSP_DISABLED  Interrupt does not exist in the vector table.
 *********************************************************************************************************************/
static ssp_err_t riic_slave_configure_interrupts (ssp_feature_t * ssp_feature,
                                    riic_slave_instance_ctrl_t * const p_ctrl, i2c_cfg_t  const   * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;
    /* Configure RXI interrupt */
    err = riic_slave_set_irq_parameters(ssp_feature, SSP_SIGNAL_IIC_RXI, p_cfg->rxi_ipl, p_ctrl, &p_ctrl->rxi_irq);

    if (SSP_SUCCESS == err)
    {
        /* Configure TXI interrupt */
        err = riic_slave_set_irq_parameters(ssp_feature, SSP_SIGNAL_IIC_TXI, p_cfg->txi_ipl, p_ctrl, &p_ctrl->txi_irq);
    }

    if (SSP_SUCCESS == err)
    {
        /* Configure TEI interrupt */
        err = riic_slave_set_irq_parameters(ssp_feature, SSP_SIGNAL_IIC_TEI, p_cfg->tei_ipl, p_ctrl, &p_ctrl->tei_irq);
    }

    if (SSP_SUCCESS == err)
    {
        /* Configure ERI interrupt */
        err = riic_slave_set_irq_parameters(ssp_feature, SSP_SIGNAL_IIC_ERI, p_cfg->eri_ipl, p_ctrl, &p_ctrl->eri_irq);
    }

    return err;
}
