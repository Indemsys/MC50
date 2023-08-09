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
/***********************************************************************************************************************
* File Name    : r_adc.c
* Description  : Primary source code for 12-bit A/D Converter driver.
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/

#include "bsp_api.h"
#include "r_adc_api.h"
/* Configuration for this package. */
#include "r_adc_cfg.h"
/* Private header file for this package. */
#include "r_adc_private_api.h"
#include "r_adc.h"
#include "./hw/hw_adc_private.h"
#include "r_cgc.h"

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/** Macro for error logger. */
#ifndef ADC_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define ADC_ERROR_RETURN(a, err)  SSP_ERROR_RETURN((a), (err), &g_module_name[0], &s_adc_version)
#endif

/** ADC resolution is defined by b2:4 of the variant data. ADC resolution is 8 bits when resolution_variant = 0,
 * 10 bits when resolution_variant = 1, 12 bits when resolution_variant = 2, and 14 bits when resolution_variant = 3.
 */
#define ADC_VARIANT_RESOLUTION_MASK    (0x1C)
#define ADC_VARIANT_RESOLUTION_SHIFT   (2)

/** ADC PGA availability  is defined by b5 of the variant data. PGA is present when resolution_variant = 1; not present
 * when resolution_variant = 0.
 */
#define ADC_VARIANT_PGA_MASK (0x20U)
#define ADC_VARIANT_PGA_SHIFT (5)

/** Maximum number of units on any Synergy ADC. */
#define ADC_MAX_UNITS                  (2)

/** Length of extended data provided by the FMI for ADC. */
#define ADC_FMI_EXTENDED_DATA_COUNT_WORDS (2U)

#define ADC_1MHZ_CLOCK                        (1000000U)
#define ADC_MAX_CALIBRATION_CLOCKS_MILLISECS  (780U)

#define ADC_SAMPLE_STATE_MULTIPLIER           (1000U)

#define ADC_SHIFT_LEFT_ALIGNED_32_BIT         (16U)

#define ADC_OPEN                              (0x52414443U)

#define ADC_ADADC_AVEE_BIT                    (0x80U)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
static ssp_err_t r_adc_open_cfg_check(adc_mode_t const mode, adc_cfg_t const * const p_cfg);
static ssp_err_t r_adc_open_cfg_align_add_clear_check(adc_cfg_t const * const p_cfg);
static ssp_err_t r_adc_open_cfg_trigger_mode_check(adc_cfg_t const * const p_cfg);
static ssp_err_t r_adc_open_cfg_resolution_check(adc_cfg_t const * const p_cfg, uint8_t resolution);
static ssp_err_t r_adc_sample_state_cfg_check(adc_instance_ctrl_t * p_ctrl, adc_sample_state_t * p_sample);
static ssp_err_t r_adc_scan_cfg_check_sample_hold_group(ADC_BASE_PTR              const p_regs,
                                                        adc_channel_cfg_t const * const p_cfg);

static ssp_err_t r_adc_scan_cfg_check_sample_hold(ADC_BASE_PTR              const p_regs,
                                                  adc_channel_cfg_t const * const p_cfg);

static ssp_err_t r_adc_scan_cfg_check_sensors_exclusive(adc_instance_ctrl_t     * const p_ctrl,
                                                        adc_channel_cfg_t const * const p_cfg);

static ssp_err_t r_adc_scan_cfg_check_sensors(adc_instance_ctrl_t     * const p_ctrl,
                                              adc_channel_cfg_t const * const p_cfg);
#endif /* (1 == ADC_CFG_PARAM_CHECKING_ENABLE) */
static ssp_err_t r_adc_infoget_param_check(adc_instance_ctrl_t * p_ctrl, adc_info_t * p_adc_info);


static ssp_err_t r_adc_sensor_cfg_temperature(adc_instance_ctrl_t * const p_ctrl,
                                              ADC_BASE_PTR              const p_regs,
                                              adc_channel_cfg_t const * const p_cfg);

static void r_adc_close_sub(ADC_BASE_PTR          const p_regs,
                            adc_instance_ctrl_t * const p_ctrl);

static ssp_err_t r_adc_open_sub(adc_instance_ctrl_t * const p_ctrl,
                           adc_cfg_t const * const p_cfg);

static ssp_err_t r_adc_sensor_cfg(adc_instance_ctrl_t * const p_ctrl,
                                   ADC_BASE_PTR              const p_regs,
                                  adc_channel_cfg_t const * const p_cfg);

#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
static ssp_err_t r_adc_scan_cfg_check_addition(ADC_BASE_PTR              const p_regs,
                                               adc_channel_cfg_t const * const p_cfg);

static ssp_err_t r_adc_scan_cfg_check(adc_instance_ctrl_t     * const p_ctrl,
                                      adc_channel_cfg_t const * const p_cfg,
                                      uint32_t          const * const p_valid_channels);
static ssp_err_t r_adc_check_addition_supported(adc_cfg_t const * const p_cfg);
#endif /* (1 == ADC_CFG_PARAM_CHECKING_ENABLE) */

static void r_adc_interrupts_cfg(adc_instance_ctrl_t     * const p_ctrl,
                                 adc_channel_cfg_t const * const p_cfg);

static ssp_err_t r_adc_interrupts_initialize(adc_instance_ctrl_t * const p_ctrl,
                                             adc_cfg_t     const * const p_cfg,
                                             ssp_feature_t       * const p_feature);

static ssp_err_t r_adc_fmi_query(adc_instance_ctrl_t * const p_ctrl,
                                 adc_cfg_t     const * const p_cfg,
                                 ssp_feature_t       * const p_feature);

static ssp_err_t r_adc_scan_cfg(adc_instance_ctrl_t     * const p_ctrl,
                                adc_channel_cfg_t const * const p_cfg,
                                uint32_t          const * const p_valid_channels);
static ssp_err_t r_adc_sensor_sample_state_calculation(uint32_t  * const p_sample_states);
void r_adc_open_configure_pga_settings(adc_instance_ctrl_t * const p_ctrl,adc_cfg_t     const * const p_cfg);
static ssp_err_t r_adc_retrieve_temp_sensor_type(adc_instance_ctrl_t * const p_ctrl);
void adc_scan_end_b_isr(void);
void adc_scan_end_isr(void);
static int32_t r_adc_lowest_channel_get(uint32_t adc_mask);
static int32_t r_adc_highest_channel_get(uint32_t adc_mask);
#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug. This pragma suppresses the warnings in this 
 * structure only.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t s_adc_version =
{
        .api_version_minor  = ADC_API_VERSION_MINOR,
        .api_version_major  = ADC_API_VERSION_MAJOR,
        .code_version_major = ADC_CODE_VERSION_MAJOR,
        .code_version_minor = ADC_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "adc";
#endif

/** Mask of valid channels on this MCU. */
static uint32_t g_adc_valid_channels[ADC_MAX_UNITS] = {0};

/***********************************************************************************************************************
 Global Variables
 **********************************************************************************************************************/

/** ADC Implementation of ADC*/
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const adc_api_t g_adc_on_adc =
{
        .open                   = R_ADC_Open,
        .scanCfg                = R_ADC_ScanConfigure,
        .infoGet                = R_ADC_InfoGet,
        .scanStart              = R_ADC_ScanStart,
        .scanStop               = R_ADC_ScanStop,
        .scanStatusGet          = R_ADC_CheckScanDone,
        .sampleStateCountSet    = R_ADC_SetSampleStateCount,
        .read                   = R_ADC_Read,
        .read32                 = R_ADC_Read32,
        .close                  = R_ADC_Close,
        .versionGet             = R_ADC_VersionGet,
        .calibrate              = R_ADC_Calibrate,
        .offsetSet              = R_ADC_OffsetSet,
};

/*******************************************************************************************************************//**
 * @addtogroup ADC
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief
 * The Open function applies power to the A/D peripheral, sets the operational mode, trigger sources, interrupt
 * priority, and configurations for the peripheral as a whole. If interrupt priority is non-zero in BSP_IRQ_Cfg.h,
 * the function takes a callback function pointer for notifying the user at interrupt level whenever a scan has completed.
 * On MCUs where calibration is possible, this function will only return after calibration is completed if enabled
 * in the user configuration. The calibration times vary depending on PCLKB and ADCLK.
 *
 * @retval  SSP_SUCCESS                Call successful.
 * @retval  SSP_ERR_ASSERTION          The parameter p_api_ctrl or p_cfg is NULL.
 * @retval  SSP_ERR_INVALID_ARGUMENT   Mode or element of p_cfg structure has invalid value or is illegal based on mode.
 * @retval  SSP_ERR_IN_USE             Calibration timed out.
 *
***********************************************************************************************************************/
ssp_err_t R_ADC_Open(adc_ctrl_t * p_api_ctrl,  adc_cfg_t const * const p_cfg)
{
    adc_instance_ctrl_t * p_ctrl = (adc_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err = SSP_SUCCESS;

    /**  Perform parameter checking */
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    /** Verify the pointers are valid */
    SSP_ASSERT (NULL != p_ctrl);
    SSP_ASSERT (NULL != p_cfg);

    /** Verify the configuration parameters are valid   */
    err = r_adc_open_cfg_check(p_cfg->mode, p_cfg);
    ADC_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif

    /** Verify this unit has not already been initialized   */
    ADC_ERROR_RETURN(ADC_OPEN != p_ctrl->opened, SSP_ERR_IN_USE);

    /** Set all p_ctrl fields prior to using it in any functions**/
    /** Save callback function pointer  */
    p_ctrl->callback = p_cfg->p_callback;
    /** Store the Unit number into the control structure*/
    p_ctrl->unit = p_cfg->unit;
    /** Store the user context into the control structure*/
    p_ctrl->p_context = p_cfg->p_context;
    /** Store the mode into the control structure*/
    p_ctrl->mode = p_cfg->mode;
    /** Store the alignment into the control structure*/
    p_ctrl->alignment = p_cfg->alignment;
    /** Save the regular mode/Group A trigger in the internal control block*/
    p_ctrl->trigger = p_cfg->trigger;
    /** Save the context */
    p_ctrl->p_context = p_cfg->p_context;
    /** Store the voltage reference into the control structure */
    p_ctrl->voltage_ref = p_cfg->voltage_ref;
    /** Store the over_current into the control structure */
    p_ctrl->over_current = p_cfg->over_current;

    /** pga0 setting*/
    p_ctrl->pga0 =  p_cfg->pga0;

    /** pga1 setting*/
    p_ctrl->pga1 =  p_cfg->pga1;

    /** pga2 setting*/
    p_ctrl->pga2 =  p_cfg->pga2;

    /** Confirm the requested unit exists on this MCU and record available channels. */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = p_cfg->unit;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_ADC;
    err = r_adc_fmi_query(p_ctrl, p_cfg, &ssp_feature);
    ADC_ERROR_RETURN(SSP_SUCCESS == err, err);

#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    /** Check for valid argument values for options that are unique to the IP */
    err = r_adc_open_cfg_resolution_check(p_cfg, p_ctrl->max_resolution);
    ADC_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif

    /** Lock specified ADC channel */
    err = R_BSP_HardwareLock(&ssp_feature);
    ADC_ERROR_RETURN((SSP_SUCCESS == err), err);

    err = r_adc_interrupts_initialize(p_ctrl, p_cfg, &ssp_feature);
    if (SSP_SUCCESS != err)
    {
        R_BSP_HardwareUnlock(&ssp_feature);
        return err;
    }
    /** Retrieve temperature sensor information into control block */
    r_adc_retrieve_temp_sensor_type(p_ctrl);
    /** Set ADC and Temperature sensors to a stop state*/
    r_adc_close_sub(p_ctrl->p_reg, p_ctrl);
    /** Initialize the hardware based on the configuration*/
    err = r_adc_open_sub(p_ctrl, p_cfg);
    if (SSP_SUCCESS != err)
    {
        R_BSP_HardwareUnlock(&ssp_feature);
        /** Set ADC and Temperature sensors to a stop state*/
        r_adc_close_sub(p_ctrl->p_reg, p_ctrl);
        return err;
    }
    /** Configure PGA for the supported MCU's */
    r_adc_open_configure_pga_settings(p_ctrl,p_cfg);
    /** Invalid scan mask (initialized for later). */
    p_ctrl->scan_mask = 0U;
    /** Mark driver as opened by initializing it to "RADC" in its ASCII equivalent for this unit. */
    p_ctrl->opened = ADC_OPEN;

    /** Return the error code */
    return err;
}

/*******************************************************************************************************************//**
 * @brief  Set the sample state count for individual channels. This only needs to be set
 *                                    for special use cases. Normally, use the default values out of Reset.
 *
 * @retval  SSP_SUCCESS                Call successful.
 * @retval  SSP_ERR_ASSERTION          The parameter p_api_ctrl or p_sample is NULL.
 * @retval  SSP_ERR_NOT_OPEN           Unit is not open.
 * @retval  SSP_ERR_INVALID_ARGUMENT   Parameter has invalid value.
***********************************************************************************************************************/
ssp_err_t R_ADC_SetSampleStateCount(adc_ctrl_t * p_api_ctrl, adc_sample_state_t * p_sample)
{
    adc_instance_ctrl_t * p_ctrl = (adc_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err = SSP_SUCCESS;

    /** Perform parameter checking */
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    /** Verify the pointers are valid */
    SSP_ASSERT (NULL != p_ctrl);
    SSP_ASSERT (NULL != p_sample);
#endif
    /** Ensure ADC Unit is already open */
    if (ADC_OPEN != p_ctrl->opened)
    {
        return SSP_ERR_NOT_OPEN;
    }
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    /** Verify arguments are legal */
    err = r_adc_sample_state_cfg_check(p_ctrl, p_sample);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    SSP_ASSERT (NULL != p_ctrl->p_reg);
#endif

    /** Set the sample state count for the specified register */
    ADC_BASE_PTR p_regs = (ADC_BASE_PTR) p_ctrl->p_reg;
    HW_ADC_SampleStateRegSet(p_regs, p_sample->reg_id, p_sample->num_states);

    /** Return the error code */
    return err;
}

/*******************************************************************************************************************//**
 * @brief  Configure the ADC scan parameters. Channel specific settings are set in this function.
 *
 * @retval  SSP_SUCCESS                   Call successful.
 * @retval  SSP_ERR_ASSERTION             The parameter p_api_ctrl or p_channel_cfg is NULL.
 * @retval  SSP_ERR_NOT_OPEN              Unit is not open.
 * @retval  SSP_ERR_INVALID_ARGUMENT      Parameter has invalid value.
 *
 * @note If the Group Mode Priority configuration is set to ADC_GROUP_A_GROUP_B_CONTINUOUS_SCAN, then since Group B
 * will be scanning continuously, Group B Interrupts are disabled and the application will not receive a callback
 * for Group B scan completion even if a callback is provided. The application will still receive a callback for
 * Group A scan completion if a callback is provided.
 * @note If the ADC conversion clock is faster than 50 MHz, the Temperature and Voltage sensor will not be accurate
 * across the operating temperature range, so an error is returned.
***********************************************************************************************************************/
ssp_err_t R_ADC_ScanConfigure(adc_ctrl_t * p_api_ctrl, adc_channel_cfg_t const * const p_channel_cfg)
{
    adc_instance_ctrl_t * p_ctrl = (adc_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err = SSP_SUCCESS;

    /**  Perform parameter checking */
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    /** Verify the pointers are valid */
    SSP_ASSERT (NULL != p_ctrl);
    SSP_ASSERT (NULL != p_channel_cfg);
#endif
    /** Ensure ADC Unit is already open  */
    if (ADC_OPEN != p_ctrl->opened)
    {
        return SSP_ERR_NOT_OPEN;
    }
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT (NULL != p_ctrl->p_reg);
#endif

    /** Configure the hardware based on the configuration */
    err = r_adc_scan_cfg(p_ctrl, p_channel_cfg, &g_adc_valid_channels[0]);

    /** Save the scan mask locally; this is required for the infoGet function*/
    p_ctrl->scan_mask = p_channel_cfg->scan_mask;

    /** Return the error code */
    return err;
}

/*******************************************************************************************************************//**
 * @brief  This function returns the address of the lowest number configured channel and the total number of bytes
 * to be read in order to read the results of the configured channels and return the ELC Event name. If no channels
 * are configured, then a length of 0 is returned.
 * This function retrieves the temperature sensor slope. It also returns the calibration data for the sensor
 * if available on this MCU otherwise an invalid calibration data of 0xFFFFFFFF will be returned.
 *
 * @retval  SSP_SUCCESS                Call successful.
 * @retval  SSP_ERR_ASSERTION          The parameter p_api_ctrl is NULL.
 * @retval  SSP_ERR_NOT_OPEN           Unit is not open.
 * @retval  SSP_ERR_INVALID_ARGUMENT   Parameter has invalid value.
 * @return                             See @ref Common_Error_Codes or functions called by this function for other possible
 *                                     return codes. This function calls:
 *                                        * fmi_api_t::eventInfoGet
 * @note: Currently this function call does not support Group Mode operation.
***********************************************************************************************************************/
ssp_err_t R_ADC_InfoGet(adc_ctrl_t * p_api_ctrl, adc_info_t * p_adc_info)
{
    adc_instance_ctrl_t * p_ctrl = (adc_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err = SSP_SUCCESS;
    uint32_t adc_mask = 0;
    int32_t adc_mask_count = -1;
    __I uint16_t * end_address;


    /** Verify the parameters are valid */
    err = r_adc_infoget_param_check(p_api_ctrl, p_adc_info);
    /** Return an error if the parameter check failed*/
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Get a pointer to the base register for the current unit */
    ADC_BASE_PTR p_regs = (ADC_BASE_PTR) p_ctrl->p_reg;
    /** Retrieve the scan mask of active channels from the control structure */
    adc_mask = p_ctrl->scan_mask;
    /** If at least one channel is configured, determine the highest and lowest configured channels*/
    if (adc_mask != 0U)
    {
    	/** Determine the lowest channel that is configured*/
    	adc_mask_count = r_adc_lowest_channel_get(adc_mask);
        p_adc_info->p_address = HW_ADC_ResultRegAddrGet(p_regs, adc_mask_count);

        /** Determine the highest channel that is configured*/
        adc_mask_count = r_adc_highest_channel_get(adc_mask);
        end_address = HW_ADC_ResultRegAddrGet(p_regs, adc_mask_count);

        /** Determine the size of data that must be read to read all the channels between and including the
         * highest and lowest channels.*/
        p_adc_info->length = (uint32_t)((end_address - p_adc_info->p_address) + 1);
    }
    /** If no channels are configured, set the return length 0*/
    else
    {
        p_adc_info->length = 0U;
    }
    p_adc_info->transfer_size = TRANSFER_SIZE_2_BYTE;
    /** Specify the peripheral name in the ELC list */
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = p_ctrl->unit;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_ADC;
    
	/**Verify the return value from fmi event information*/
    err = g_fmi_on_fmi.eventInfoGet(&ssp_feature, SSP_SIGNAL_ADC_SCAN_END, &event_info);
    ADC_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_adc_info->elc_event = event_info.event;
    p_adc_info->elc_peripheral = (elc_peripheral_t) (ELC_PERIPHERAL_ADC0 + (2U * p_ctrl->unit));

    /** Set Temp Sensor calibration data to invalid value */
    p_adc_info->calibration_data = 0xFFFFFFFFUL;
    /** If calibration register is available, retrieve it from the MCU */
    if (1U == p_ctrl->tsn_calib_available)
    {
        HW_ADC_CalibrationRegRead(p_ctrl->p_tsn_calib_regs, &p_adc_info->calibration_data);
    }
    /** Provide the previously retrieved slope information */
    p_adc_info->slope_microvolts = p_ctrl->slope_microvolts;

    return err;
}

/*******************************************************************************************************************//**
 * @brief  This function starts a software scan or enables the hardware trigger for a scan depending
 *                          on how the triggers were configured in the Open() call. If the Unit was configured for
 *                          hardware triggering, then this function  simply allows the trigger signal (hardware or
 *                          software) to get to the ADC Unit. The function is not able to control the generation of the
 *                          trigger itself. If the Unit was configured for software triggering, then this function
 *                          starts the software triggered scan.
 *
 * @retval  SSP_SUCCESS                Call successful.
 * @retval  SSP_ERR_ASSERTION          The parameter p_api_ctrl is NULL.
 * @retval  SSP_ERR_NOT_OPEN           Unit is not open.
 * @retval  SSP_ERR_IN_USE             Running scan is still in progress
***********************************************************************************************************************/
ssp_err_t R_ADC_ScanStart(adc_ctrl_t * p_api_ctrl)
{
    adc_instance_ctrl_t * p_ctrl = (adc_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err = SSP_SUCCESS;

    /** Perform parameter checking  */
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    /** Verify the pointers are valid */
    SSP_ASSERT (NULL != p_ctrl);
#endif
    /** Ensure ADC Unit is already open  */
    if (ADC_OPEN != p_ctrl->opened)
    {
        return SSP_ERR_NOT_OPEN;
    }
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT (NULL != p_ctrl->p_reg);
#endif

    /** If the the normal/GroupA trigger is not set to software, then that the Unit is configured for hardware triggering */
    if (ADC_TRIGGER_SOFTWARE != p_ctrl->trigger)
    {
        HW_ADC_ADCSR_TRGE_Set(p_ctrl->p_reg, HW_ADC_ADCSR_TRGE_SET);
    }
    /** Otherwise, enable software triggering */
    else
    {
        /** Check to see if there is an ongoing scan else start the scan */
        if (HW_ADC_ADCSR_ADCST_NOT_SET == HW_ADC_ADCSR_ADST_Get(p_ctrl->p_reg))
        {
            HW_ADC_ADCSR_ADST_Set(p_ctrl->p_reg, HW_ADC_ADCSR_ADCST_SET);
        }
        else
        {
            err = SSP_ERR_IN_USE;
        }
    }

    /** Return the error code */
    return err;
}

/*******************************************************************************************************************//**
 * @brief  This function stops the software scan or disables the Unit from being triggered by the
 *                         hardware trigger (internal or external) based on what type of trigger the unit was configured
 *                         for in the Open() function. Stopping a hardware triggered scan via this function does not abort
 *                         an ongoing scan, but  prevents the next scan from occurring. Stopping a software triggered
 *                         scan aborts an ongoing scan.
 *
 * @retval  SSP_SUCCESS                Call successful.
 * @retval  SSP_ERR_ASSERTION          The parameter p_api_ctrl is NULL.
 * @retval  SSP_ERR_NOT_OPEN           Unit is not open.
 * @note    Stopping a software scan results in immediate stoppage of the scan irrespective of current state of
 *          of the scan. Stopping the hardware scan results in disabling the trigger to prevent future scans
 *          from starting but does not affect the current scan.
***********************************************************************************************************************/
ssp_err_t R_ADC_ScanStop(adc_ctrl_t * p_api_ctrl)
{
    adc_instance_ctrl_t * p_ctrl = (adc_instance_ctrl_t *) p_api_ctrl;

    /**  Perform parameter checking */
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    /** Verify the pointers are valid */
    SSP_ASSERT (NULL != p_ctrl);
#endif
    /** Ensure ADC Unit is already open  */
    if (ADC_OPEN != p_ctrl->opened)
    {
        return SSP_ERR_NOT_OPEN;
    }
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)	
    SSP_ASSERT (NULL != p_ctrl->p_reg);
#endif

    /** If the trigger is not software scan, then disallow hardware triggering*/
    if (ADC_TRIGGER_SOFTWARE != p_ctrl->trigger)
    {
        HW_ADC_ADCSR_TRGE_Set(p_ctrl->p_reg, HW_ADC_ADCSR_TRGE_NOT_SET);
    }
    /** Otherwise, disable software triggering*/
    else
    {
        HW_ADC_ADCSR_ADST_Set(p_ctrl->p_reg, HW_ADC_ADCSR_ADCST_NOT_SET);
    }

    /** Return the error code */
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  This function returns the status of any scan process that was started.
 *         On supported MCUs, the status of the ADC calibration is returned.
 *
 * @retval  SSP_SUCCESS                Successful; the scan is complete.
 * @retval  SSP_ERR_ASSERTION          The parameter p_api_ctrl is NULL.
 * @retval  SSP_ERR_NOT_OPEN           Unit is not open.
 * @retval  SSP_ERR_IN_USE             Running scan or calibration is still in progress.
 *
 * @note    If the peripheral was configured in single scan mode, then the return value of this function is an
 *          indication of the scan status. However, if the peripheral was configured in group mode, then the return
 *          value of this function could be an indication of either the group A or group B scan state. This is because
 *          the ADST bit is set when a scan is ongoing and cleared when the scan is done. This function should normally
 *          only be used when using software trigger in single scan mode.
***********************************************************************************************************************/
ssp_err_t R_ADC_CheckScanDone(adc_ctrl_t * p_api_ctrl)
{
    adc_instance_ctrl_t * p_ctrl = (adc_instance_ctrl_t *) p_api_ctrl;
    ssp_err_t err = SSP_SUCCESS;

    /**  Perform parameter checking */
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    /** Verify the pointers are valid */
    SSP_ASSERT (NULL != p_ctrl);
#endif
    /** Ensure ADC Unit is already open  */
    if (ADC_OPEN != p_ctrl->opened)
    {
        return SSP_ERR_NOT_OPEN;
    }
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT (NULL != p_ctrl->p_reg);
#endif

    /** Read status of ADC calibration and return busy status if calibration is ongoing */
    if (ADC_ADCALEXE_CALIBRATION_STATUS == HW_ADC_ADCALEXE_Get(p_ctrl->p_reg))
    {
    	return SSP_ERR_IN_USE;
    }
    /** Read the status of the ADST bit*/
    if (HW_ADC_ADCSR_ADCST_SET == HW_ADC_ADCSR_ADST_Get(p_ctrl->p_reg))
    {
        err = SSP_ERR_IN_USE;
    }

    /** Return the error code */
    return err;
}

/*******************************************************************************************************************//**
 * @brief   This function reads conversion results from a single channel or sensor
 *          register.
 * @retval  SSP_SUCCESS                Call successful.
 * @retval  SSP_ERR_ASSERTION          The parameter p_api_ctrl is NULL.
 * @retval  SSP_ERR_INVALID_POINTER    The parameter p_data is NULL.
 * @retval  SSP_ERR_NOT_OPEN           Unit is not open.
 * @retval  SSP_ERR_INVALID_ARGUMENT   Parameter has invalid value.
***********************************************************************************************************************/
ssp_err_t R_ADC_Read(adc_ctrl_t * p_api_ctrl, adc_register_t const  reg_id, adc_data_size_t * const p_data)
{
    adc_instance_ctrl_t * p_ctrl = (adc_instance_ctrl_t *) p_api_ctrl;
    ADC_BASE_PTR p_regs;

    /** Perform parameter checking*/
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    /** Verify the pointers are valid */
    SSP_ASSERT (NULL != p_ctrl);

    /** Verify that the channel is valid for this MCU */
    if ((reg_id > ADC_REG_CHANNEL_27) || (reg_id < ADC_REG_TEMPERATURE))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }
    if (reg_id >= ADC_REG_CHANNEL_0)
    {
        uint32_t requested_channel_mask = (1U << (uint32_t) reg_id);
        if (0 == (requested_channel_mask & g_adc_valid_channels[p_ctrl->unit]))
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }
    }
#endif

    /** Verify that the ADC is already open */
    if (ADC_OPEN != p_ctrl->opened)
    {
        return SSP_ERR_NOT_OPEN;
    }
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    if (NULL == p_data)
    {
        return SSP_ERR_INVALID_POINTER;
    }
    SSP_ASSERT (NULL != p_ctrl->p_reg);
#endif
    /** Get pointer to appropriate base address. This is repeated here in case
     * parameter checking is disabled. */
    p_regs = (ADC_BASE_PTR) p_ctrl->p_reg;
    /** Read the data from the requested ADC conversion register and return it */
    *p_data = HW_ADC_ResultGet(p_regs, reg_id);

    /** Return the error code */
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   This function reads conversion results from a single channel or sensor register into a 32-bit result.
 *
 * @retval  SSP_SUCCESS                Call successful.
 * @retval  SSP_ERR_ASSERTION          The parameter p_api_ctrl is NULL.
 * @retval  SSP_ERR_INVALID_POINTER    The parameter p_data is NULL.
 * @retval  SSP_ERR_NOT_OPEN           Unit is not open.
 * @retval  SSP_ERR_INVALID_ARGUMENT   Parameter has invalid value.
***********************************************************************************************************************/
ssp_err_t R_ADC_Read32(adc_ctrl_t * p_api_ctrl, adc_register_t const reg_id, uint32_t * const p_data)
{
    /** Read the 16-bit result. */
    uint16_t result = 0U;
    uint32_t result_32 = 0U;
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    /** Verify that the pointer is valid. */
    ADC_ERROR_RETURN(NULL != p_data, SSP_ERR_INVALID_POINTER)
#endif

    ssp_err_t err = R_ADC_Read(p_api_ctrl, reg_id, &result);
    result_32 = result;
    if (SSP_SUCCESS == err)
    {
        /** Left shift the result into the upper 16 bits if the unit is configured for left alignment. */
        adc_instance_ctrl_t * p_ctrl = (adc_instance_ctrl_t *) p_api_ctrl;
        if (ADC_ALIGNMENT_LEFT == p_ctrl->alignment)
        {
            result_32 <<= ADC_SHIFT_LEFT_ALIGNED_32_BIT;
        }
    }

    *p_data = result_32;

    ADC_ERROR_RETURN(SSP_SUCCESS == err, err);
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  This function ends any scan in progress, disables interrupts, and removes power to the A/D
 *                       peripheral.
 *
 * @retval  SSP_SUCCESS                Call successful.
 * @retval  SSP_ERR_ASSERTION          The parameter p_api_ctrl is NULL.
 * @retval  SSP_ERR_NOT_OPEN           Unit is not open.
***********************************************************************************************************************/
ssp_err_t R_ADC_Close(adc_ctrl_t * p_api_ctrl)
{
    adc_instance_ctrl_t * p_ctrl = (adc_instance_ctrl_t *) p_api_ctrl;

    /** Perform parameter checking*/
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    /** Verify the pointers are valid */
    SSP_ASSERT (NULL != p_ctrl);
#endif
    /** Verify that the ADC is already open */
    if (ADC_OPEN != p_ctrl->opened)
    {
        return SSP_ERR_NOT_OPEN;
    }
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT (NULL != p_ctrl->p_reg);
#endif

    /** Mark driver as closed   */
    p_ctrl->opened = 0x00000000U;

    /** Perform hardware stop for the specific unit*/
    ssp_vector_info_t * p_vector_info;
    if (SSP_INVALID_VECTOR != p_ctrl->scan_end_irq)
    {
        NVIC_DisableIRQ (p_ctrl->scan_end_irq);         /*** Disable interrupts in ICU    */
        NVIC_ClearPendingIRQ (p_ctrl->scan_end_irq);    /*** Clear interrupt flag         */
        R_BSP_IrqStatusClear (p_ctrl->scan_end_irq);
        R_SSP_VectorInfoGet(p_ctrl->scan_end_irq, &p_vector_info);
        *(p_vector_info->pp_ctrl) = NULL;
    }
    if (SSP_INVALID_VECTOR != p_ctrl->scan_end_b_irq)
    {
        NVIC_DisableIRQ (p_ctrl->scan_end_b_irq);         /*** Disable interrupts in ICU    */
        NVIC_ClearPendingIRQ (p_ctrl->scan_end_b_irq);    /*** Clear interrupt flag         */
        R_BSP_IrqStatusClear (p_ctrl->scan_end_b_irq);
        R_SSP_VectorInfoGet(p_ctrl->scan_end_b_irq, &p_vector_info);
        *(p_vector_info->pp_ctrl) = NULL;
    }
    r_adc_close_sub(p_ctrl->p_reg, p_ctrl);

    /** Release the lock */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.channel = p_ctrl->unit;
    ssp_feature.unit = 0U;
    ssp_feature.id = SSP_IP_ADC;
    R_BSP_HardwareUnlock(&ssp_feature);

    /** Return the error code */
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   Retrieve the API version number.
 *
 * @retval  SSP_SUCCESS        Successful return.
 * @retval  SSP_ERR_ASSERTION  The parameter p_version is NULL.
***********************************************************************************************************************/
ssp_err_t R_ADC_VersionGet(ssp_version_t * const p_version)
{
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    /** Verify parameters are valid */
    SSP_ASSERT(NULL != p_version);
#endif
    /** Return the version number */
    p_version->version_id =  s_adc_version.version_id;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   This function initiates calibration of the ADC on supported MCUs. Calibration will take a minimum of 24
 *          milliseconds at 32 MHz PCLKB and ADCLK. If ADC interrupts are enabled, a notification is provided via callback
 *          when calibration is complete. Otherwise, if the ADC interrupts are disabled then no notification will be
 *          provided and the application must check calibration status using infoGet() to determine if the calibration
 *          is complete before using the ADC API.Interrupts are enabled in adc_api_t::scanStatusGet().
 *
 * @param[in]  p_api_ctrl    Pointer to control handle structure
 * @param[in]  p_extend  Unused argument. Pass NULL.
 *
 * @retval  SSP_SUCCESS                         Calibration successfully initiated.
 * @retval  SSP_ERR_INVALID_HW_CONDITION        Hardware is in invalid state to perform calibration due to ongoing scan
 *                                              or scan trigger is enabled.
 * @retval  SSP_ERR_UNSUPPORTED                 Calibration not supported on this MCU.
 * @retval  SSP_ERR_ASSERTION                   The parameter p_api_ctrl is NULL.
***********************************************************************************************************************/
ssp_err_t R_ADC_Calibrate(adc_ctrl_t * const p_api_ctrl, void * const p_extend)
{

    /**  Perform parameter checking */
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    /** Verify the pointers are valid */
    SSP_ASSERT (NULL != p_api_ctrl);
#endif
    SSP_PARAMETER_NOT_USED (p_extend);

    bsp_feature_adc_t adc_feature = {0U};
    R_BSP_FeatureAdcGet(&adc_feature);
    ssp_err_t err = SSP_SUCCESS;
    if (adc_feature.calibration_reg_available)
    {
        adc_instance_ctrl_t * p_ctrl = (adc_instance_ctrl_t *) p_api_ctrl;

        /** ADC Calibration can only happen if there is no ongoing scan and if the scan trigger is disabled */
        if ((HW_ADC_ADCSR_ADST_Get(p_ctrl->p_reg) == 0U) && (HW_ADC_ADCSR_TRGE_Get(p_ctrl->p_reg) == 0U))
        {
            /** Set the normal mode interrupt request to occur when calibration is complete */
            HW_ADC_ADICR_Set (p_ctrl->p_reg, ADC_ADICR_CALIBRATION_INTERRUPT_ENABLED);
            /** Initiate calibration */
            HW_ADC_ADCALEXE_Set (p_ctrl->p_reg, ADC_ADCALEXE_SET_CALEXE);
        }
        else
        {
            err = SSP_ERR_INVALID_HW_CONDITION;
        }
    }
    else
    {
        err = SSP_ERR_UNSUPPORTED;
    }

    /** Return the unsupported error. */
    return err;
}

/*******************************************************************************************************************//**
 * adc_api_t::offsetSet is not supported on the ADC.
***********************************************************************************************************************/
ssp_err_t R_ADC_OffsetSet(adc_ctrl_t * const p_api_ctrl, adc_register_t const reg_id, int32_t offset)
{
    SSP_PARAMETER_NOT_USED(p_api_ctrl);
    SSP_PARAMETER_NOT_USED(reg_id);
    SSP_PARAMETER_NOT_USED(offset);

    /** Return the unsupported error. */
    return SSP_ERR_UNSUPPORTED;
}


/*******************************************************************************************************************//**
 * @} (end addtogroup ADC)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 Private Functions
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @brief   r_adc_interrupts_initialize
 *
 * This function initializes ADC interrupts.  It stores IRQ numbers and sets priorities in the NVIC.
 *
 * @param[in]  p_ctrl          :  ADC control structure.
 * @param[in]  p_cfg           :  Pointer to configuration structure
 * @param[in]  p_feature       :  Pointer to ADC feature
 * @return                        See @ref Common_Error_Codes or functions called by this function for other possible
 *                                return codes. This function calls:
 *                                        * fmi_api_t::eventInfoGet
***********************************************************************************************************************/
static ssp_err_t r_adc_interrupts_initialize(adc_instance_ctrl_t * const p_ctrl,
                                             adc_cfg_t     const * const p_cfg,
                                             ssp_feature_t       * const p_feature)
{
    /** Set the interrupt priorities. */
    ssp_vector_info_t * p_vector_info;
    fmi_event_info_t event_info = {(IRQn_Type) 0U};
    g_fmi_on_fmi.eventInfoGet(p_feature, SSP_SIGNAL_ADC_SCAN_END, &event_info);
    p_ctrl->scan_end_irq = event_info.irq;

    /** If a callback is used, then make sure the scan end interrupt is enabled */
    if (NULL != p_cfg->p_callback)
    {
        ADC_ERROR_RETURN(SSP_INVALID_VECTOR != p_ctrl->scan_end_irq, SSP_ERR_IRQ_BSP_DISABLED);
    }
    if (SSP_INVALID_VECTOR != p_ctrl->scan_end_irq)
    {
        R_SSP_VectorInfoGet(p_ctrl->scan_end_irq, &p_vector_info);
        NVIC_SetPriority(p_ctrl->scan_end_irq, p_cfg->scan_end_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;

        /** Clear any pending interrupt requests in the NVIC or the peripheral*/
        NVIC_DisableIRQ (p_ctrl->scan_end_irq);         /*** Disable interrupts in ICU    */
        NVIC_ClearPendingIRQ (p_ctrl->scan_end_irq);    /*** Clear interrupt flag         */
        R_BSP_IrqStatusClear (p_ctrl->scan_end_irq);
    }
    g_fmi_on_fmi.eventInfoGet(p_feature, SSP_SIGNAL_ADC_SCAN_END_B, &event_info);
    p_ctrl->scan_end_b_irq = event_info.irq;

    /** If group mode is enabled and a callback is used, then make sure group mode interrupts
     * are enabled */
    if ((p_cfg->mode == ADC_MODE_GROUP_SCAN) && (NULL != p_cfg->p_callback))
    {
        ADC_ERROR_RETURN(SSP_INVALID_VECTOR != p_ctrl->scan_end_b_irq, SSP_ERR_IRQ_BSP_DISABLED);
    }
    if (SSP_INVALID_VECTOR != p_ctrl->scan_end_b_irq)
    {
        R_SSP_VectorInfoGet(p_ctrl->scan_end_b_irq, &p_vector_info);
        NVIC_SetPriority(p_ctrl->scan_end_b_irq, p_cfg->scan_end_b_ipl);
        *(p_vector_info->pp_ctrl) = p_ctrl;

        /** Clear any pending interrupt requests in the NVIC or the peripheral*/
        NVIC_DisableIRQ (p_ctrl->scan_end_b_irq);         /*** Disable interrupts in ICU    */
        NVIC_ClearPendingIRQ (p_ctrl->scan_end_b_irq);    /*** Clear interrupt flag         */
        R_BSP_IrqStatusClear (p_ctrl->scan_end_b_irq);
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   r_adc_fmi_query
 *
 * This function queries the factory flash to ensure the unit exists and record the valid channel information.
 *
 * @param[in]  p_ctrl          :  ADC control structure.
 * @param[in]  p_cfg           :  Pointer to configuration structure
 * @param[in]  p_feature       :  Pointer to ADC feature
 * @return                        See @ref Common_Error_Codes or functions called by this function for other possible
 *                                return codes. This function calls:
 *                                   * fmi_api_t::productFeatureGet
***********************************************************************************************************************/
static ssp_err_t r_adc_fmi_query(adc_instance_ctrl_t * const p_ctrl,
                                 adc_cfg_t     const * const p_cfg,
                                 ssp_feature_t       * const p_feature)
{
    /** Confirm the requested unit exists on this MCU. */
    fmi_feature_info_t info = {0U};
    ssp_err_t err = g_fmi_on_fmi.productFeatureGet(p_feature, &info);
    ADC_ERROR_RETURN(SSP_SUCCESS == err, err);
    p_ctrl->p_reg = info.ptr;
    uint8_t max_resolution_bits = (uint8_t) ((info.variant_data & ADC_VARIANT_RESOLUTION_MASK)
            >> ADC_VARIANT_RESOLUTION_SHIFT);
    p_ctrl->max_resolution = (uint8_t) ((max_resolution_bits * 2U) + 8U);
    /** Determine if PGA is present on MCU on not */
    p_ctrl->pga_available = (uint8_t) ((info.variant_data & ADC_VARIANT_PGA_MASK) >> ADC_VARIANT_PGA_SHIFT);
    /** Set the valid channel mask based on variant data. */
    if (0U == g_adc_valid_channels[p_cfg->unit])
    {
        uint32_t * p_extended_data = (uint32_t *) info.ptr_extended_data;
        /* ADC_FMI_EXTENDED_DATA_COUNT_WORDS == info.extended_data_count, but this is not available in all
         * factory flash versions. */
        for (uint32_t j = 0U; j < ADC_FMI_EXTENDED_DATA_COUNT_WORDS; j += 1U)
        {
            if (0U == j)
            {
                /* The first word of extended data has a valid channel bitmask for the first 16 channels in the upper
                 * 16 bits. */
                g_adc_valid_channels[p_cfg->unit] = (p_extended_data[j] >> 16) & 0xFFFFU;
            }
            else
            {
                /* The second word of extended data has a valid channel bitmask for the any channels above channel 16
                 * in the upper 16 bits. */
                g_adc_valid_channels[p_cfg->unit] |= (p_extended_data[j] & 0xFFFF0000UL);
            }
        }
    }

    return SSP_SUCCESS;
}
/*******************************************************************************************************************//**
 * @brief   Handle PGA configuration for the supported MCU's
 *
 * This function enables/disables the ADC PGA feature on supported MCUs.
 *
 * @param[in]  p_ctrl          :  ADC control structure.
***********************************************************************************************************************/
void r_adc_open_configure_pga_settings(adc_instance_ctrl_t * const p_ctrl,adc_cfg_t     const * const p_cfg)
{
    /** If PGA is supported, configures corresponding register */
    if (1U == p_ctrl->pga_available)
    {
        /** PGA registers configuration */
        HW_ADC_ADPGAGS0_Set(p_ctrl->p_reg,p_cfg);
        HW_ADC_ADPGACR_Set(p_ctrl->p_reg,p_cfg);
        HW_ADC_ADPGADCR0_Set(p_ctrl->p_reg,p_cfg);
    }
}

/*******************************************************************************************************************//**
 * @brief   r_adc_infoget_param_check : ADC check the infoGet function parameters
 *
 * This function validates the configuration arguments for illegal combinations or options.
 *
 * @param[in]  p_ctrl     :  Control Structure
 * @param[in]  p_adc_info :  User defined structure into which the main infoGet call will populate data
 *
 * @retval  SSP_SUCCESS -       Successful
 * @retval  SSP_ERR_INVALID_ARGUMENT -  ADC is configured for Group mode which infoGet does not support
 * @retval  SSP_ERR_ASSERTION -        The parameter p_ctrl or p_adc_info or p_ctrl->p_reg is NULL.
 * @retval  SSP_ERR_NOT_OPEN -         The driver is not initialized.
***********************************************************************************************************************/
static ssp_err_t r_adc_infoget_param_check(adc_instance_ctrl_t * p_ctrl, adc_info_t * p_adc_info)
{
    ssp_err_t err = SSP_SUCCESS;

#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    /**Ensure that the pointers are valid */
    SSP_ASSERT (NULL != p_ctrl);
    SSP_ASSERT (NULL != p_adc_info);
#else
    SSP_PARAMETER_NOT_USED(p_adc_info);
#endif

    /** Ensure ADC Unit is already open  */
    if (ADC_OPEN != p_ctrl->opened)
    {
        return SSP_ERR_NOT_OPEN;
    }

#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)

    /** Return an error if mode is Group Mode since that is not
     * supported currently */
    if (ADC_MODE_GROUP_SCAN == p_ctrl->mode)
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }
    SSP_ASSERT (NULL != p_ctrl->p_reg);
#endif

    return err;
}


#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
/*******************************************************************************************************************//**
 * @brief   r_adc_sample_state_cfg_check
 *
 * This function checks the Sample and Hold arguments
 *
 * @param[in]  p_ctrl          :  ADC control structure.
 * @param[in]  p_sample        :  Sample State Configuration
 * @retval  SSP_SUCCESS -       Successful
 * @retval  SSP_ERR_INVALID_ARGUMENT -  ADC is configured for invalid sample states count or channels selected for sample state modification are not supported on the MCU.
***********************************************************************************************************************/
static ssp_err_t r_adc_sample_state_cfg_check(adc_instance_ctrl_t * p_ctrl, adc_sample_state_t * p_sample)
{
    /** Used to prevent compiler warning */
    SSP_PARAMETER_NOT_USED(p_ctrl);

    adc_sample_state_reg_t reg_id = p_sample->reg_id;
    if ((reg_id > ADC_SAMPLE_STATE_CHANNEL_15) || (reg_id < ADC_SAMPLE_STATE_CHANNEL_16_TO_27))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }
    if (reg_id >= ADC_SAMPLE_STATE_CHANNEL_0)
    {
        uint32_t requested_channel_mask = (1U << reg_id);
        if (0 == (requested_channel_mask & g_adc_valid_channels[p_ctrl->unit]))
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }
    }

    if (ADC_SAMPLE_STATE_COUNT_MIN > p_sample->num_states)
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }
    else
    {
        return SSP_SUCCESS;
    }
}
/*******************************************************************************************************************//**
 * @brief   r_adc_open_cfg_check : ADC check open function configuration
 *
 * This function validates the configuration arguments for illegal combinations or options.
 *
 * @param[in]  mode  :  Operational mode (see enumeration below)
 * @param[in]  p_cfg :  Pointer to configuration structure (see below)
 *
 * @retval  SSP_SUCCESS -       Successful
 * @retval  SSP_ERR_INVALID_ARGUMENT -  Mode or element of p_cfg structure has invalid value or is invalid based on mode
 * @retval  SSP_ERR_IRQ_BSP_DISABLED -  IRQ is disabled in the BSP when a callback function is passed
***********************************************************************************************************************/
static ssp_err_t r_adc_open_cfg_check(adc_mode_t const  mode, adc_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;
    /** Check for valid argument values for alignment, add_average_count, clearing, trigger, and mode. */
    err = r_adc_open_cfg_align_add_clear_check(p_cfg);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    err = r_adc_open_cfg_trigger_mode_check(p_cfg);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** If 16 time addition is used only 12 bit accuracy can be selected*/
    if ((ADC_RESOLUTION_12_BIT != p_cfg->resolution) && (ADC_ADD_SIXTEEN == p_cfg->add_average_count))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    /** If the mode is continuous, then the callback has to be NULL */
    if (p_cfg->p_callback != NULL)
    {
        if (p_cfg->mode == ADC_MODE_CONTINUOUS_SCAN)
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }
    }

    /** Group checking; only synchronous triggers (ELC) allowed.  */
    if ((ADC_MODE_GROUP_SCAN == mode))
    {
        if ((ADC_TRIGGER_SYNC_ELC  != p_cfg->trigger)
                ||(ADC_TRIGGER_SYNC_ELC != p_cfg->trigger_group_b))
        {
            err = SSP_ERR_INVALID_ARGUMENT;
        }
    }


    return err;
}
/*******************************************************************************************************************//**
 * @brief   r_adc_open_cfg_align_add_clear_check : ADC check open function configuration
 *
 * This function validates the configuration arguments for illegal combinations or options.
 *
 * @param[in]  p_cfg       :  Pointer to configuration structure
 *
 * @retval  SSP_SUCCESS -               Successful
 * @retval  SSP_ERR_INVALID_ARGUMENT -  Mode or element of p_cfg structure has invalid value or is invalid based on mode
***********************************************************************************************************************/
static ssp_err_t r_adc_open_cfg_align_add_clear_check(adc_cfg_t const * const p_cfg)
{
    ssp_err_t err;
    /** Check for valid argument values for alignment, add_average_count, and clearing. */
    if ((ADC_ALIGNMENT_RIGHT != p_cfg->alignment) && (ADC_ALIGNMENT_LEFT != p_cfg->alignment))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    err = r_adc_check_addition_supported(p_cfg);
    ADC_ERROR_RETURN(SSP_SUCCESS == err, err);

    if ((ADC_CLEAR_AFTER_READ_OFF != p_cfg->clearing) && (ADC_CLEAR_AFTER_READ_ON != p_cfg->clearing))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    return SSP_SUCCESS;
}

static ssp_err_t r_adc_check_addition_supported(adc_cfg_t const * const p_cfg)
{
    bsp_feature_adc_t adc_feature = {0U};
    R_BSP_FeatureAdcGet(&adc_feature);

    if(ADC_ADD_OFF == p_cfg->add_average_count)
    {
        return SSP_SUCCESS;
    }
    
    /** If addition is supported in the MCU, then Average 8 or Average 16 cannot be selected */

    if(adc_feature.addition_supported) 
    {
        if((ADC_ADD_AVERAGE_SIXTEEN == p_cfg->add_average_count)
                || (ADC_ADD_AVERAGE_EIGHT == p_cfg->add_average_count))
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }

    }
    else
    {
        if(0U == (ADC_ADADC_AVEE_BIT & p_cfg->add_average_count))
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }
    }
    
    return SSP_SUCCESS;
    
}

/*******************************************************************************************************************//**
 * @brief   r_adc_open_cfg_trigger_mode_check : ADC check open function configuration
 *
 * This function validates the configuration arguments for illegal combinations or options.
 *
 * @param[in]  p_cfg       :  Pointer to configuration structure
 *
 * @retval  SSP_SUCCESS -               Successful
 * @retval  SSP_ERR_INVALID_ARGUMENT -  Mode or element of p_cfg structure has invalid value or is invalid based on mode
***********************************************************************************************************************/
static ssp_err_t r_adc_open_cfg_trigger_mode_check(adc_cfg_t const * const p_cfg)
{
    /** Check for valid argument values for trigger and mode. */
    if ((ADC_TRIGGER_ASYNC_EXT_TRG0 != p_cfg->trigger) &&
            (ADC_TRIGGER_SYNC_ELC != p_cfg->trigger)
            && (ADC_TRIGGER_SOFTWARE != p_cfg->trigger))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }
    if ((ADC_MODE_SINGLE_SCAN != p_cfg->mode) &&
            (ADC_MODE_CONTINUOUS_SCAN != p_cfg->mode)
            && (ADC_MODE_GROUP_SCAN != p_cfg->mode))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   r_adc_open_cfg_resolution_check : ADC check open function configuration
 *
 * This function validates the configuration arguments for illegal combinations or options.
 *
 * @param[in]  p_cfg       :  Pointer to configuration structure
 * @param[in]  resolution  :  ADC maximum resolution
 *
 * @retval  SSP_SUCCESS -       Successful
 * @retval  SSP_ERR_INVALID_ARGUMENT -  Mode or element of p_cfg structure has invalid value or is invalid based on mode
***********************************************************************************************************************/
static ssp_err_t r_adc_open_cfg_resolution_check(adc_cfg_t const * const p_cfg, uint8_t resolution)
{
    if (12U == resolution)
    {
        if ((ADC_RESOLUTION_12_BIT != p_cfg->resolution)
             && (ADC_RESOLUTION_10_BIT != p_cfg->resolution)
             && (ADC_RESOLUTION_8_BIT != p_cfg->resolution))
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }
    }
    if (14U == resolution)
    {
        if ((ADC_RESOLUTION_14_BIT != p_cfg->resolution)
             && (ADC_RESOLUTION_12_BIT != p_cfg->resolution))
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }
    }
    if (16U == resolution)
    {
        if (ADC_RESOLUTION_16_BIT != p_cfg->resolution)
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   r_adc_scan_cfg_check_sample_hold_group
 *
 * This function checks the Sample and Hold arguments to verify group restrictions are not violated.
 *
 * @param[in]  p_regs         : Pointer to base ADC register
 * @param[in]  p_cfg          : The configuration argument passed to R_ADC_ScanConfigure.
 **********************************************************************************************************************/
static ssp_err_t r_adc_scan_cfg_check_sample_hold_group(ADC_BASE_PTR              const p_regs,
                                                        adc_channel_cfg_t const * const p_cfg)
{
    uint32_t a_mask;
    uint32_t b_mask;

    /** Sample and Hold  channels cannot not be split across groups  */
    a_mask = p_cfg->sample_hold_mask & p_cfg->scan_mask;
    b_mask = p_cfg->sample_hold_mask & p_cfg->scan_mask_group_b;
    if (ADC_MODE_GROUP_SCAN == HW_ADC_ScanModeGet(p_regs))
    {
        if ((a_mask != p_cfg->sample_hold_mask) && (b_mask != p_cfg->sample_hold_mask))
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }
    }
    else if (a_mask != p_cfg->sample_hold_mask)
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }
    else
    {
        /** Will not get here: Branch created to meet coding standard */
    }

    /** Sample and Hold channels cannot be a double trigger channel (can be in group B) */
    if ((HW_ADC_DOUBLE_TRIGGER_SELECTED == HW_ADC_DoubleTriggerGet(p_regs)) && (0 != a_mask))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    /** Sample and Hold  channels cannot be in GroupB if GroupA priority enabled     */
    if ((0 != b_mask) && (ADC_GROUP_A_PRIORITY_OFF != p_cfg->priority_group_a))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   r_adc_scan_cfg_check_sample_hold
 *
 * This function checks the Sample and Hold arguments
 *
 * @param[in]  p_regs         : Pointer to ADC base register
 * @param[in]  p_cfg          : The configuration argument passed to R_ADC_ScanConfigure.
 **********************************************************************************************************************/
static ssp_err_t r_adc_scan_cfg_check_sample_hold(ADC_BASE_PTR              const p_regs,
                                                  adc_channel_cfg_t const * const p_cfg)
{
    /** If a valid value is set in the mask. */
    if (0U != p_cfg->sample_hold_mask)
    {
        /** If sample and hold is not supported, the mask should be 0. */
        bsp_feature_adc_t adc_feature = {0U};
        R_BSP_FeatureAdcGet(&adc_feature);
        if (!adc_feature.has_sample_hold_reg)
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }

        /** Sample and Hold channels can only be 0,1,2 and must have at least minimum state count specified */
        if ((ADC_SAMPLE_HOLD_CHANNELS < p_cfg->sample_hold_mask)
                || (ADC_SAMPLE_STATE_HOLD_COUNT_MIN > p_cfg->sample_hold_states))
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }

        return r_adc_scan_cfg_check_sample_hold_group(p_regs, p_cfg);
    }
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   r_adc_scan_cfg_check_sensors_exclusive
 *
 * This function checks the Temperature and Voltage sensor arguments for restrictions that apply to MCUs that do not
 * allow both sensors to be used at once.
 *
 * @param[in]  p_ctrl         : The ADC instance control block.
 * @param[in]  p_cfg          : The configuration argument passed to R_ADC_ScanConfigure.
 * @retval  SSP_SUCCESS -       Successful
 * @retval  SSP_ERR_INVALID_ARGUMENT - both voltage and temperature sensor are used in same configuration or a sensor is used at the same time as regular channel or sensors are used in any mode other than single scan mode
 **********************************************************************************************************************/
static ssp_err_t r_adc_scan_cfg_check_sensors_exclusive(adc_instance_ctrl_t     * const p_ctrl,
                                                        adc_channel_cfg_t const * const p_cfg)
{
    /** Both sensors cannot be used at the same time on some MCUs. */
    if ((p_cfg->scan_mask & ADC_MASK_TEMPERATURE) && (p_cfg->scan_mask & ADC_MASK_VOLT))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    /** If any of the sensors are being used, then none of the channels can be used at the same time on some MCUs. */
    if ((p_cfg->scan_mask & ADC_MASK_SENSORS) && (p_cfg->scan_mask &
                          (ADC_MASK_CHANNEL_0 | ADC_MASK_CHANNEL_1 | ADC_MASK_CHANNEL_2 | ADC_MASK_CHANNEL_3 |
                           ADC_MASK_CHANNEL_4 | ADC_MASK_CHANNEL_5 | ADC_MASK_CHANNEL_6 | ADC_MASK_CHANNEL_7 |
                           ADC_MASK_CHANNEL_8 | ADC_MASK_CHANNEL_9 | ADC_MASK_CHANNEL_10 | ADC_MASK_CHANNEL_11 |
                           ADC_MASK_CHANNEL_12 | ADC_MASK_CHANNEL_13 | ADC_MASK_CHANNEL_14 | ADC_MASK_CHANNEL_15 |
                           ADC_MASK_CHANNEL_16 | ADC_MASK_CHANNEL_17 | ADC_MASK_CHANNEL_18 | ADC_MASK_CHANNEL_19 |
                           ADC_MASK_CHANNEL_20 | ADC_MASK_CHANNEL_21 | ADC_MASK_CHANNEL_22 | ADC_MASK_CHANNEL_23 |
                           ADC_MASK_CHANNEL_24 | ADC_MASK_CHANNEL_25 | ADC_MASK_CHANNEL_26 | ADC_MASK_CHANNEL_27 )))
    {
                return SSP_ERR_INVALID_ARGUMENT;
    }

    /** Sensors cannot be used in any mode other than single scan mode on some MCUs. */
    if ((p_cfg->scan_mask & ADC_MASK_SENSORS) && (ADC_MODE_SINGLE_SCAN != p_ctrl->mode))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   r_adc_scan_cfg_check_sensors
 *
 * This function checks the Temperature and Voltage sensor arguments
 *
 * @param[in]  p_ctrl         : The ADC instance control block.
 * @param[in]  p_cfg          : The configuration argument passed to R_ADC_ScanConfigure.
 * @retval  SSP_SUCCESS -       Successful
 * @retval  SSP_ERR_INVALID_ARGUMENT - Sensor configuration has been selected for Group B on an MCU which does not allow Group B configuration or sensor is used in Normal/Group A with the double trigger not enabled or MCU does not allow both the sensors to be used simultaneously
 **********************************************************************************************************************/
static ssp_err_t r_adc_scan_cfg_check_sensors(adc_instance_ctrl_t     * const p_ctrl,
                                              adc_channel_cfg_t const * const p_cfg)
{
    ADC_BASE_PTR p_regs = (ADC_BASE_PTR) p_ctrl->p_reg;

    bsp_feature_adc_t adc_feature = {0U};
    R_BSP_FeatureAdcGet(&adc_feature);
    if (!adc_feature.group_b_sensors_allowed)
    {
        /** Sensors are not supported in Group B in some MCUs. */
        if (p_cfg->scan_mask_group_b & ADC_MASK_SENSORS)
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }
    }

    /** If sensors specified in Normal/Group A, verify in legal configuration
     * If sensors are used, then Double Trigger and Disconnect Detection cannot be used */
    if ((p_cfg->scan_mask & ADC_MASK_SENSORS))
    {
        if (HW_ADC_DOUBLE_TRIGGER_SELECTED == HW_ADC_DoubleTriggerGet(p_regs))  /*** Double trigger mode          */
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }
        if (!adc_feature.sensors_exclusive)
        {
            /** Disconnect detection register must be 0 to use sensors on some MCUs.  */
            if (0U != HW_ADC_DisconnectDetectGet(p_regs))
            {
                return SSP_ERR_INVALID_ARGUMENT;
            }
        }
    }

    if (adc_feature.sensors_exclusive)
    {
        return r_adc_scan_cfg_check_sensors_exclusive(p_ctrl, p_cfg);
    }

    return SSP_SUCCESS;
}
#endif

/*******************************************************************************************************************//**
 * @brief   r_adc_retrieve_temp_sensor_type
 *
 * This function retrieves information about the temperature sensor including availability of the control and
 * calibration registers
 *
 * @param[in]  p_ctrl         : Pointer to the ADC control block
***********************************************************************************************************************/
static ssp_err_t r_adc_retrieve_temp_sensor_type(adc_instance_ctrl_t     * const p_ctrl)
{
    bsp_feature_adc_t adc_feature = {0U};
    /** Clear the control/calibration flags and registers */
    p_ctrl->tsn_ctrl_available = 0U;
    p_ctrl->tsn_calib_available = 0U;
    p_ctrl->p_tsn_calib_regs = NULL;
    p_ctrl->p_tsn_ctrl_regs = NULL;
    /** Determine if the TSN peripheral on this MCU supports Calibration and or Control capability */
    R_BSP_FeatureAdcGet(&adc_feature);

    /** Get temperature sensor control register. */
    ssp_feature_t ssp_feature = {{(ssp_ip_t) 0U}};
    ssp_feature.id = SSP_IP_TSN;
    ssp_feature.channel = 0U;

    if (1U == adc_feature.tsn_control_available)
    {
        /** TSN "Unit 0" contains information about the Control Register */
        ssp_feature.unit = 0U;
        fmi_feature_info_t feature_info = {0U};
        /** Retrieve the TSN Control register address */
        ssp_err_t err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &feature_info);
        if (SSP_SUCCESS != err)
        {
            return err;
        }
        /** Set control block flag indicating that control register is available */
        p_ctrl->tsn_ctrl_available = 1U;
        /** Save the control register address to the control block */
        p_ctrl->p_tsn_ctrl_regs = (R_TSN_Control_Type *) feature_info.ptr;
    }
    if (1U == adc_feature.tsn_calibration_available)
    {
        /** TSN "Unit 1" contains information about the Calibration Register */
        ssp_feature.unit = 1U;
        fmi_feature_info_t feature_info = {0U};
        /** Retrieve the TSN Calibration register address */
        ssp_err_t err = g_fmi_on_fmi.productFeatureGet(&ssp_feature, &feature_info);
        if (SSP_SUCCESS != err)
        {
            return err;
        }
        /** Set control block flag indicating that calibration register is available */
        p_ctrl->tsn_calib_available = 1U;
        /** Save the calibration register address to the control block */
        p_ctrl->p_tsn_calib_regs = (R_TSN_Calibration_Type *) feature_info.ptr;
    }
    /** Save the slope value to the control block */
    p_ctrl->slope_microvolts = adc_feature.tsn_slope;
    return SSP_SUCCESS;

}

/*******************************************************************************************************************//**
 * @brief   r_adc_sensor_cfg_temperature
 *
 * This function set the sensor bits to enable the temperature sensor for group A/normal mode.
 * This function requires the control block initialization to be complete prior to being used.
 *
 * @param[in]  p_ctrl         : Pointer to the ADC control block
 * @param[in]  p_regs         : Pointer to ADC base register
 * @param[in]  p_cfg          : The configuration argument passed to R_ADC_ScanConfigure.
 * @return                      See @ref Common_Error_Codes or functions called by this function for other possible
 *                              return codes. This function calls:
 *                                * fmi_api_t::productFeatureGet
 **********************************************************************************************************************/
static ssp_err_t r_adc_sensor_cfg_temperature(adc_instance_ctrl_t     * const p_ctrl,
                                              ADC_BASE_PTR              const p_regs,
                                              adc_channel_cfg_t const * const p_cfg)
{
    if (1U == p_ctrl->tsn_ctrl_available)
    {
        /** Get temperature sensor control register. */
        ssp_feature_t ssp_feature_control = {{(ssp_ip_t) 0U}};
        ssp_feature_control.id = SSP_IP_TSN;
        /** TSN "Channel 0" contains information about the Control Register */
        ssp_feature_control.channel = 0U;
        ssp_feature_control.unit = 0U;
        fmi_feature_info_t feature_info = {0U};
        /** Retrieve the TSN Control register address */
        ssp_err_t err = g_fmi_on_fmi.productFeatureGet(&ssp_feature_control, &feature_info);
        if (SSP_SUCCESS != err)
        {
            return err;
        }

        /** Power on the temperature sensor. This is only needed for TSNs that have the control register */
        R_BSP_ModuleStart(&ssp_feature_control);

        /** Enable the temperature sensor output to the ADC */
        HW_ADC_TemperatureSensorCfg(p_ctrl->p_tsn_ctrl_regs);
    }

    if (p_cfg->scan_mask & ADC_MASK_TEMPERATURE)
    {
        HW_ADC_SensorCfgTemperatureGroupA(p_regs);      /** Select temperature output GroupA */
    }
    else
    {
        HW_ADC_SensorCfgTemperatureGroupB(p_regs);
    }

    /** Enable temperature addition mode if set */
    uint16_t temperature_add = 0U;
    if ((p_cfg->add_mask & ADC_MASK_TEMPERATURE) > 0U)
    {
        temperature_add = 1U;
    }
    /** Configure the addition mode for the temperature sensor */
    HW_ADC_TemperatureSensorAddCfg(p_regs, temperature_add);


    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief r_adc_close_sub
 *
 * The Close function stops power and clocking to the A/D peripheral, disables interrupts and turns off the channels and
 * sensors.
 *
 * @param[in]  p_regs : Pointer to ADC base register
 * @param[in]  p_ctrl : ADC instance control block
 **********************************************************************************************************************/
static void r_adc_close_sub(ADC_BASE_PTR          const p_regs,
                            adc_instance_ctrl_t * const p_ctrl)
{
    /** Disable group priority. */
    HW_ADC_GroupPrioritySet(p_regs, HW_ADC_GROUP_PRIORITY_DISABLED);

    /** Disable the triggers for both groups */
    if (ADC_MODE_GROUP_SCAN == HW_ADC_ScanModeGet(p_regs))
    {
        HW_ADC_ElcTriggerGroupASet(p_regs, ADC_ELC_TRIGGER_DISABLED);
        HW_ADC_ElcTriggerGroupBSet(p_regs, ADC_ELC_TRIGGER_DISABLED);
    }
    /** If Group Mode was not enabled, then disable the trigger for normal mode */
    else
    {
        HW_ADC_ElcTriggerGroupASet(p_regs, ADC_ELC_TRIGGER_DISABLED);
    }

    /** Clear all configurations*/
    HW_ADC_RegisterReset(p_regs);
    HW_ADC_ADICR_Set (p_ctrl->p_reg, ADC_ADICR_CALIBRATION_INTERRUPT_DISABLED);

    /** Power down peripheral   */
    ssp_feature_t adc_feature = {{(ssp_ip_t) 0U}};
    adc_feature.id = SSP_IP_ADC;
    adc_feature.channel = p_ctrl->unit;
    adc_feature.unit = 0U;
    R_BSP_ModuleStop(&adc_feature);
    /** If the temperature sensor was used, then disable the sensor */
    if (0 != (p_ctrl->scan_mask & ADC_MASK_TEMPERATURE))
    {
        if (NULL != p_ctrl->p_tsn_ctrl_regs)
        {
            /*** Disable temperature sensor      */
            HW_ADC_TempSensorDisable(p_ctrl->p_tsn_ctrl_regs);
        }
        ssp_feature_t tsn_feature = {{(ssp_ip_t) 0U}};
        tsn_feature.id = SSP_IP_TSN;
        tsn_feature.channel = 0U;
        tsn_feature.unit = 0U;
        R_BSP_ModuleStop(&tsn_feature);
    }
}

/*******************************************************************************************************************//**
 * @brief r_adc_open_sub
 *
 * The Open function applies power to the A/D peripheral, sets the operational mode, trigger sources, and
 * configurations common to all channels and sensors.
 *
 * @param[in]  p_ctrl : Pointer to ADC base register
 * @param[in]  p_cfg  : Pointer to configuration structure
 *
 * @retval  SSP_SUCCESS          Hardware initialization successful.
 * @retval  SSP_ERR_TIMEOUT      Calibration timed out.
***********************************************************************************************************************/
static ssp_err_t r_adc_open_sub(adc_instance_ctrl_t * const p_ctrl,
                           adc_cfg_t const * const p_cfg)
{
    /** Apply power to peripheral   */
    bsp_feature_adc_t adc_bsp_feature = {0U};
    ssp_feature_t adc_feature = {{(ssp_ip_t) 0U}};
    uint32_t one_millisecond_delays = 0U;
    ssp_err_t err = SSP_SUCCESS;
    adc_feature.id = SSP_IP_ADC;
    adc_feature.channel = p_cfg->unit;
    adc_feature.unit = 0U;
    R_BSP_ModuleStart(&adc_feature);

    /** Clear all settings including disabling interrupts*/
    HW_ADC_RegisterReset(p_ctrl->p_reg);

    /** Set mode related register fields    */
    HW_ADC_ScanModeSet(p_ctrl->p_reg, p_cfg->mode);

    /** Disable double trigger since this is not currently supported */
    HW_ADC_DoubleTriggerSet(p_ctrl->p_reg, HW_ADC_DOUBLE_TRIGGER_DESELECTED);

    /** Configure trigger if its a hardware trigger*/
    if (ADC_TRIGGER_SYNC_ELC == p_cfg->trigger)
    {
        /** Set the ELC value for normal/groupA trigger*/
        HW_ADC_ElcTriggerGroupASet(p_ctrl->p_reg, ADC_ELC_TRIGGER);
    }
    /** Configure external trigger for async trigger*/
    else if (ADC_TRIGGER_ASYNC_EXT_TRG0 == p_cfg->trigger)
    {
        /** Enable Asynchronous external trigger*/
        HW_ADC_AsyncTriggerEnable(p_ctrl->p_reg);
        /** Set TRSA to 0x00 if using Asynchronous external trigger*/
        HW_ADC_ElcTriggerGroupASet(p_ctrl->p_reg, ADC_ELC_TRIGGER_EXTERNAL);
    }
    else
    {
        /** Do Nothing: Branch created to meet coding standard*/
    }

    /** Configure ADC resolution, alignment, clearing, and addition/averaging. */
    HW_ADC_Init(p_ctrl->p_reg, p_cfg);
    HW_ADC_Deselect_data_inversion(p_ctrl->p_reg);

    /** Retrieve information about the availability of ADC calibration register availability*/
    R_BSP_FeatureAdcGet(&adc_bsp_feature);
    /** Save calibration register availability status to control block */
    p_ctrl->adc_calib_available = adc_bsp_feature.calibration_reg_available;

    /** Set ADC internal reference voltage before calibration to avoid wrong calibrated reference value. */
    if (1U == adc_bsp_feature.reference_voltage)
    {
        HW_ADC_VREFAMPCNT_Set(p_ctrl->p_reg, p_ctrl->voltage_ref);
        R_BSP_SoftwareDelay(150U, BSP_DELAY_UNITS_MICROSECONDS);
        HW_ADC_OVERCURRENT_Set(p_ctrl->p_reg, p_ctrl->over_current);
        HW_ADC_VREFAMPCNT_Set_Enabled(p_ctrl->p_reg, p_ctrl->voltage_ref);
        R_BSP_SoftwareDelay(1500U, BSP_DELAY_UNITS_MICROSECONDS);
    }

    if (true == (bool)(p_ctrl->adc_calib_available))
    {
        /**If the user has chosen to not skip calibration, perform calibration */
        if (false == p_cfg->calib_adc_skip)
        {
            err = R_ADC_Calibrate(p_ctrl, NULL);
            if (SSP_SUCCESS != err)
            {
                return err;
            }

            /**ADC calibration time is : 15 PCLKB + 775,000 ADCLK.
             * Lowest supported ADCLK is 1MHz for low power ADC usage
             * The timeout value is thus approximately 780 milliseconds for a 1 MHz PCLKD (ADCLK)
             * considering worst case ratio of PCLKB:ADCLK of 1000 */

            /**Set calibration delay as 780 milliseconds for worst case. */
            one_millisecond_delays = (uint32_t)(ADC_MAX_CALIBRATION_CLOCKS_MILLISECS);

            /** Every millisecond check calibration status and count down if calibration not complete until the total calculated
             * delay is complete or calibration is complete */
            while ((ADC_ADCALEXE_CALIBRATION_STATUS & (HW_ADC_ADCALEXE_Get(p_ctrl->p_reg))) && (one_millisecond_delays > 0U))
            {
                R_BSP_SoftwareDelay(1U, BSP_DELAY_UNITS_MILLISECONDS);
                one_millisecond_delays--;
            }
            /** Configure the interrupt source to disable calibration interrupt after calibration is done and clear interrupt
             * request in the ICU.
             * This is done here since interrupts are not yet enabled (done in scanCfg) and so
             * the pending calibration IRQ is cleared to prevent the ISR when the scanCfg is eventually called.  */
            R_BSP_IrqStatusClear (p_ctrl->scan_end_irq) ;    /*** Clear flag in ICU     */
            NVIC_ClearPendingIRQ (p_ctrl->scan_end_irq) ;    /*** Clear flag in NVIC    */
            HW_ADC_ADICR_Set (p_ctrl->p_reg, ADC_ADICR_CALIBRATION_INTERRUPT_DISABLED);
            /** If the timeout counter expired without hardware status changing, then return timeout error */
            if (one_millisecond_delays == 0U)
            {
                return SSP_ERR_TIMEOUT;
            }
        }

    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief   r_adc_sensor_cfg
 *
 * This function set the sensor bits taking into account group inclusion and addition/average mode.
 * This function must only be called if it has been verified that sensors are used in this configuration
 *
 * @param[in]  p_ctrl         : Pointer to the ADC control block
 * @param[in]  p_regs         : Pointer to the ADC base register
 * @param[in]  p_cfg          : The configuration argument passed to R_ADC_ScanConfigure.
***********************************************************************************************************************/
static ssp_err_t r_adc_sensor_cfg(adc_instance_ctrl_t * const p_ctrl,
                                  ADC_BASE_PTR          const p_regs,
                                  adc_channel_cfg_t const * const p_cfg)
{
    /** Temperature sensor  */
    ssp_err_t err;

    uint32_t sample_states = 0U;

    /** Calculate sample state values such that the sample time for the temperature and voltage sensor is the
     * minimum defined by the hardware manual. The minimum is 4.15 microseconds for MF3 devices and
     * 5 microseconds for RV40. The sample states will be calculated to allow sampling for this duration*/
    /**Retrieve the clock source and frequency used by the ADC peripheral and sampling time required for the sensor*/
    err = r_adc_sensor_sample_state_calculation(&sample_states);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Check if the Temperature sensor channel is enabled */
    if ((p_cfg->scan_mask & ADC_MASK_TEMPERATURE) || (p_cfg->scan_mask_group_b & ADC_MASK_TEMPERATURE))
    {
        /**sample state registers are set to the calculated value */
        HW_ADC_TemperatureSampleStateSet(p_regs, (uint8_t)sample_states);
        err = r_adc_sensor_cfg_temperature(p_ctrl, p_regs, p_cfg);
        if (SSP_SUCCESS != err)
        {
            return err;
        }
    }

    /** Check if the voltage sensor channel is enabled */
    if ((p_cfg->scan_mask & ADC_MASK_VOLT) || (p_cfg->scan_mask_group_b & ADC_MASK_VOLT))
    {
        /**sample state registers are set to the calculated value */
        HW_ADC_VoltageSampleStateSet(p_regs, (uint8_t)sample_states);
        if (p_cfg->scan_mask & ADC_MASK_VOLT)
        {
            HW_ADC_SensorCfgVoltageGroupA(p_regs);    /** Select voltage output to GroupA  */
        }
        else
        {
            HW_ADC_SensorCfgVoltageGroupB(p_regs);      /** Select voltage output to GroupB  */
        }

        /** Enable voltage addition mode if set */
        uint16_t voltage_add = 0U;
        if (p_cfg->add_mask & ADC_MASK_VOLT)
        {
            voltage_add = 1U;
        }
        HW_ADC_VoltageSensorAddSet(p_regs, voltage_add);
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   r_adc_sensor_sample_state_calculation
 *
 * This function calculates the sample states value for the internal sensors and returns an error if the calculated value
 * is outside the limit supported by the hardware
 *
 * @param[out] p_sample_states: The calculates sample state count.
 * @return                       See @ref Common_Error_Codes or functions called by this function for other possible
 *                               return codes. This function calls:
 *                                 * cgc_api_t::systemClockFreqGet
***********************************************************************************************************************/
static ssp_err_t r_adc_sensor_sample_state_calculation(uint32_t * const p_sample_states)
{
    /** Temperature sensor  */
    ssp_err_t err;
    bsp_feature_adc_t adc_feature = {0U};
    uint32_t       freq_hz = 0U;
    *p_sample_states = 0U;

    /** Calculate sample state values such that the sample time for the temperature and voltage sensor is the
     * minimum defined by the hardware manual. The minimum is 4.15 microseconds for MF3 devices and
     * 5 microseconds for RV40. The sample states will be calculated to allow sampling for this duration*/
    /**Retrieve the clock source and frequency used by the ADC peripheral and sampling time required for the sensor*/
    R_BSP_FeatureAdcGet(&adc_feature);
    err = g_cgc_on_cgc.systemClockFreqGet((cgc_system_clocks_t) adc_feature.clock_source, &freq_hz);
    /** If the peripheral clock is less than 1 MHz, according to the hardware module, the ADC module cannot be used.
     * Upper limit of the peripheral clock is not checked since that will be limited by the hardware to 64MHz */
    if ((freq_hz < ADC_1MHZ_CLOCK) || (err != SSP_SUCCESS))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    /** Calculate sample states required for the current ADC conversion clock */
    *p_sample_states = ((freq_hz / ADC_1MHZ_CLOCK) * adc_feature.sensor_min_sampling_time) / ADC_SAMPLE_STATE_MULTIPLIER;

    /** If the calculated sample state value is higher than 255, then return an error since the sample state register is only 8 bits.
     * If it is less than the min number of states required, then return an error. */
    if ((*p_sample_states > ADC_SAMPLE_STATE_COUNT_MAX) || (*p_sample_states < ADC_SAMPLE_STATE_COUNT_MIN))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

     return SSP_SUCCESS;
}

#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
/*******************************************************************************************************************//**
 * @brief   r_adc_scan_cfg_check_addition
 *
 * This function checks the addition arguments
 *
 * @param[in]  p_regs         : Pointer to ADC registers for this unit
 * @param[in]  p_cfg          : The configuration argument passed to R_ADC_ScanConfigure.
 **********************************************************************************************************************/
static ssp_err_t r_adc_scan_cfg_check_addition(ADC_BASE_PTR              const p_regs,
                                               adc_channel_cfg_t const * const p_cfg)
{
    /** Verify that if addition is enabled, then at least one proper channel is selected*/
    if (ADC_ADD_OFF != HW_ADC_AddModeGet(p_regs))
    {
        /** Addition mask should not include bits from inactive channels.
         * This also serves as a check for valid channels in the addition mask */
        uint32_t tmp_mask = 0;
        tmp_mask = p_cfg->scan_mask_group_b;
        tmp_mask |= p_cfg->scan_mask;   /*** tmp_mask is Group A and B combined   */
        /*** Bit-AND with 1s-complement   */
        if ((0U != (p_cfg->add_mask & ~tmp_mask)) || (0U == p_cfg->add_mask))
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }
    }
    /** Channels are selected for addition despite addition being disabled.
        WARNING! Other features messed up if add_mask is non-zero when addition is turned off! */
    else
    {
        if (ADC_MASK_ADD_OFF != p_cfg->add_mask)
        {
             return SSP_ERR_INVALID_ARGUMENT;
        }
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   r_adc_scan_cfg_check
 *
 * This function does extensive checking on channel mask settings based upon operational mode.
 *
 * @param[in]  p_ctrl            :  ADC unit control block
 * @param[in]  p_cfg             :  Pointer to channel configuration structure containing masks
 * @param[in]  p_valid_channels  :  Pointer to bitmask of valid channels on this MCU
 *
 * @retval  SSP_SUCCESS -               Successful
 * @retval  SSP_ERR_INVALID_ARGUMENT -  Parameter has invalid/illegal value.
***********************************************************************************************************************/
static ssp_err_t r_adc_scan_cfg_check(adc_instance_ctrl_t     * const p_ctrl,
                                      adc_channel_cfg_t const * const p_cfg,
                                      uint32_t          const * const p_valid_channels)
{
    ssp_err_t err;
    uint16_t unit = p_ctrl->unit;
    ADC_BASE_PTR p_regs = (ADC_BASE_PTR) p_ctrl->p_reg;

    /** Verify at least one bonded channel is selected  */
    uint32_t valid_channels = p_valid_channels[unit] | ADC_MASK_TEMPERATURE | ADC_MASK_VOLT;
    if ((0U == p_cfg->scan_mask)
     || (0U != (p_cfg->scan_mask & (~valid_channels))))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

 /** Verify at least one unique bonded channel is selected for Group B
     * and GroupA priority setting is valid. */
    if (ADC_MODE_GROUP_SCAN == HW_ADC_ScanModeGet(p_regs))
    {
        if ((0U == p_cfg->scan_mask_group_b)
         || (0U != (p_cfg->scan_mask_group_b & (~valid_channels))))
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }
        else if (0 != (p_cfg->scan_mask & p_cfg->scan_mask_group_b))
        {
            return SSP_ERR_INVALID_ARGUMENT;         /*** Same channel in both groups */
        }
        else
        {
            /** Settings are valid, do nothing. */
        }
    }
    /*** If Group mode was not enabled, but channels were selected for group B*/
    else
    {
        if (ADC_MASK_GROUP_B_OFF != p_cfg->scan_mask_group_b)
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }
    }

    /** Verify sensors are in legal combination */
    err = r_adc_scan_cfg_check_sensors(p_ctrl, p_cfg);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Check addition settings. */
    err = r_adc_scan_cfg_check_addition(p_regs, p_cfg);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Check sample and hold settings. */
    err = r_adc_scan_cfg_check_sample_hold(p_regs, p_cfg);

    return err;
}
#endif

/*******************************************************************************************************************//**
 * @brief   r_adc_interrupts_cfg
 *
 * Configures interrupts based on user settings.
 *
 * @param[in]  p_ctrl : ADC instance control block
 * @param[in]  p_cfg  : Pointer to channel configuration structure containing masks
***********************************************************************************************************************/
static void r_adc_interrupts_cfg(adc_instance_ctrl_t     * const p_ctrl,
                                 adc_channel_cfg_t const * const p_cfg)
{
    ADC_BASE_PTR p_regs = (ADC_BASE_PTR) p_ctrl->p_reg;

    /** For all modes other than continuous scan mode, enable the scan completion interrupt*/
    if (ADC_MODE_CONTINUOUS_SCAN != p_ctrl->mode)
    {
        /** Enable Group B scan completion interrupt */
        if (ADC_MODE_GROUP_SCAN == HW_ADC_ScanModeGet(p_regs))
        {
            /** If the group priority is not set to group B continuous scan mode, then enable the group B interrupt*/
            if (ADC_GROUP_A_GROUP_B_CONTINUOUS_SCAN !=  p_cfg->priority_group_a)
            {
                if (SSP_INVALID_VECTOR != p_ctrl->scan_end_b_irq)
                {
                    R_BSP_IrqStatusClear (p_ctrl->scan_end_b_irq) ;    /*** Clear flag in ICU     */
                    NVIC_ClearPendingIRQ (p_ctrl->scan_end_b_irq) ;    /*** Clear flag in NVIC    */
                    NVIC_EnableIRQ (p_ctrl->scan_end_b_irq) ;          /*** Enable in NVIC        */
                }
                HW_ADC_S12gbadiEnable(p_regs);

                /** Set the ELC value for groupB trigger*/
                HW_ADC_ElcTriggerGroupBSet(p_regs, ADC_ELC_TRIGGER_GROUP_B);
            }
            /** If the group priority mode is group B in continuous scan mode, then disable the group B
             * synchronous trigger and disable the group B interrupt to prevent the ISR from triggering continuously */
            else
            {
                HW_ADC_ElcTriggerGroupBSet(p_regs, ADC_ELC_TRIGGER_DISABLED);
                if (SSP_INVALID_VECTOR != p_ctrl->scan_end_b_irq)
                {
                    NVIC_DisableIRQ (p_ctrl->scan_end_b_irq) ;         /*** Disable in NVIC        */
                    NVIC_ClearPendingIRQ (p_ctrl->scan_end_b_irq) ;    /*** Clear flag in NVIC    */
                    R_BSP_IrqStatusClear (p_ctrl->scan_end_b_irq) ;    /*** Clear flag in ICU     */
                }
                HW_ADC_S12gbadiDisable(p_regs);
            }
        }
        /** Enable the Normal Mode/Group A scan completion interrupt */
        if (SSP_INVALID_VECTOR != p_ctrl->scan_end_irq)
        {
            R_BSP_IrqStatusClear (p_ctrl->scan_end_irq) ;    /*** Clear flag in ICU     */
            NVIC_ClearPendingIRQ (p_ctrl->scan_end_irq) ;    /*** Clear flag in NVIC    */
            NVIC_EnableIRQ (p_ctrl->scan_end_irq) ;          /*** Enable in NVIC        */
        }
    }
}

/*******************************************************************************************************************//**
 * @brief   r_adc_scan_cfg
 *
 * This function does extensive checking on channel mask settings based upon operational mode. Mask registers are
 * initialized and interrupts enabled in peripheral. Interrupts are also enabled in ICU if corresponding priority
 * is not 0.
 *
 * @param[in]  p_ctrl            : ADC instance control block
 * @param[in]  p_cfg             : Pointer to channel configuration structure
 * @param[in]  p_valid_channels  : Pointer to valid channel bitmask
 *
 * @retval  SSP_SUCCESS -               Successful
 * @retval  SSP_ERR_INVALID_ARGUMENT -  Parameter has invalid/illegal value.
***********************************************************************************************************************/
static ssp_err_t r_adc_scan_cfg(adc_instance_ctrl_t     * const p_ctrl,
                                adc_channel_cfg_t const * const p_cfg,
                                uint32_t          const * const p_valid_channels)
{
    ADC_BASE_PTR p_regs = (ADC_BASE_PTR) p_ctrl->p_reg;
    ssp_err_t err = SSP_SUCCESS;

    /** parameter checking  */
#if (1 == ADC_CFG_PARAM_CHECKING_ENABLE)
    err = r_adc_scan_cfg_check(p_ctrl, p_cfg, p_valid_channels);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
#else
    SSP_PARAMETER_NOT_USED(p_valid_channels);
#endif

    /** Set mask for channels and sensors.
     * At this point, the mask values in scan_mask, scan_mask_group_b and add_mask
     * have already been determined to be valid for this MCU by the HW_ADC_ScanCfgCheck()
     * function so the values in those variables can be directly written to the
     * registers. */
    HW_ADC_ScanMaskSet(p_regs, p_cfg);

    /** If either voltage or temperature sensor are used, configure them.
     * Also check to see if the conversion clock frequency is valid. */
    if ((p_cfg->scan_mask & ADC_MASK_SENSORS) || (p_cfg->scan_mask_group_b & ADC_MASK_SENSORS))
    {
        err = r_adc_sensor_cfg(p_ctrl, p_regs, p_cfg);
        if (SSP_SUCCESS != err)
        {
            return err;
        }
    }

    /** NOTE: S&H adds to scan time because normal state machine still runs.
     adds 12 or more sample_hold_states ADCLKS to scan time */
    uint16_t adshcr = p_cfg->sample_hold_states;
    adshcr = (uint16_t) (adshcr | (uint16_t) ((uint16_t) (p_cfg->sample_hold_mask & ADC_MASK_SAMPLE_HOLD_BYPASS_CHANNELS) << ADC_MASK_SAMPLE_HOLD_BYPASS_SHIFT));
    HW_ADC_SampleHoldSet(p_regs, adshcr);

    /** Set group A priority action (not interrupt priority!)
     * This must be set prior to configuring the interrupts for all modes other than ADC_GROUP_A_GROUP_B_CONTINUOUS_SCAN */
    if (ADC_GROUP_A_GROUP_B_CONTINUOUS_SCAN != p_cfg->priority_group_a)
    {
        HW_ADC_GroupAPrioritySet(p_regs, p_cfg->priority_group_a);
    }

    /** Configure ADC interrupts. */
    r_adc_interrupts_cfg(p_ctrl, p_cfg);

    /** Set group A priority action (not interrupt priority!)
     * This will also start the Group B scans if configured for ADC_GROUP_A_GROUP_B_CONTINUOUS_SCAN.
     * This must be configured after the interrupts/triggers have been setup for the ADC_GROUP_A_GROUP_B_CONTINUOUS_SCAN */
    if (ADC_GROUP_A_GROUP_B_CONTINUOUS_SCAN == p_cfg->priority_group_a)
    {
        HW_ADC_GroupAPrioritySet(p_regs, p_cfg->priority_group_a);
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief ADC Channel 0 ISR routine
 *
 * This function implements the unit 0 interrupt handler for normal/Group A/double trigger scan complete.
***********************************************************************************************************************/
void adc_scan_end_isr(void)
{
    /** Save context if RTOS is used */
    SF_CONTEXT_SAVE;

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    adc_instance_ctrl_t * p_ctrl = (adc_ctrl_t *) *(p_vector_info->pp_ctrl);
    adc_callback_args_t   args;

    /** Clear the BSP IRQ Flag     */
    R_BSP_IrqStatusClear (R_SSP_CurrentIrqGet());

    /** Check that the pointer is not NULL prior to using it */
    if (NULL != p_ctrl)
    {
        /** Store the correct event into the callback argument */
        if( ADC_ADICR_CALIBRATION_INTERRUPT_DISABLED == HW_ADC_ADICR_Get(p_ctrl->p_reg))
        {
            args.event = ADC_EVENT_SCAN_COMPLETE;
        }
        else
        {
            args.event = ADC_EVENT_CALIBRATION_COMPLETE;
            /** Restore the interrupt source to disable interrupts after calibration is done. */
            HW_ADC_ADICR_Set (p_ctrl->p_reg, ADC_ADICR_CALIBRATION_INTERRUPT_DISABLED);
        }
        /** Store the Unit number into the callback argument */
        args.unit = p_ctrl->unit;
        /** Populate the context field*/
        args.p_context = p_ctrl->p_context;
        /** If a callback was provided, call it with the argument */
        if (NULL != p_ctrl->callback)
        {
            p_ctrl->callback(&args);
        }

    }

    /** Restore context if RTOS is used */
    SF_CONTEXT_RESTORE;
}

/*******************************************************************************************************************//**
 * @brief   ADC Scan End Group B ISR routine
 *
 * This function implements the interrupt handler for Group B scan complete.
***********************************************************************************************************************/
void adc_scan_end_b_isr(void)
{
    /** Save context if RTOS is used */
    SF_CONTEXT_SAVE;

    ssp_vector_info_t * p_vector_info = NULL;
    R_SSP_VectorInfoGet(R_SSP_CurrentIrqGet(), &p_vector_info);
    adc_instance_ctrl_t * p_ctrl = (adc_ctrl_t *) *(p_vector_info->pp_ctrl);
    adc_callback_args_t   args;

    /** Clear the BSP IRQ Flag     */
    R_BSP_IrqStatusClear (R_SSP_CurrentIrqGet());

    /** Check that the pointer is not NULL prior to using it */
    if (NULL != p_ctrl)
    {
        /** Store the event into the callback argument */
        args.event = ADC_EVENT_SCAN_COMPLETE_GROUP_B;
        /** Store the Unit number into the callback argument */
        args.unit = p_ctrl->unit;
        /** Populate the context field*/
        args.p_context = p_ctrl->p_context;
        /** If a callback was provided, call it with the argument */
        if (NULL != p_ctrl->callback)
        {
            p_ctrl->callback(&args);
        }
    }

    /** Restore context if RTOS is used */
    SF_CONTEXT_RESTORE;
}

/*******************************************************************************************************************//**
 * @brief  This function returns the lowest channel index that is configured  in order to read the results of the
 *         configured channels.
 * @param[in]  adc_mask  scan mask of active channels retrieved from the control structure
 * @retval  adc_mask_count  index value of lowest channel
 **********************************************************************************************************************/

static int32_t r_adc_lowest_channel_get(uint32_t adc_mask)
{
    /** Initialize the mask result */
    uint32_t adc_mask_result = 0U;
    int32_t adc_mask_count = -1;

    while (0U == adc_mask_result)
    {
        adc_mask_count++;
        adc_mask_result = (uint32_t)(adc_mask & (1U << adc_mask_count));
    }
    if ((uint32_t)(1U << adc_mask_count) == ADC_MASK_TEMPERATURE)
    {
        adc_mask_count = ADC_REG_TEMPERATURE;
    }
    else if ((uint32_t)(1U << adc_mask_count) == ADC_MASK_VOLT)
    {
        adc_mask_count = ADC_REG_VOLT;
    }
    return adc_mask_count;
}

/*******************************************************************************************************************//**
 * @brief  This function returns the highest channel index that is configured  in order to read the results of the
 *         configured channels.
 * @param[in]  adc_mask  scan mask of active channels retrieved from the control structure
 * @retval  adc_mask_count  index value of highest channel
 **********************************************************************************************************************/

static int32_t r_adc_highest_channel_get(uint32_t adc_mask)
{
    /** Set the mask count so that we start with the highest bit of the 32 bit mask */
    /** Initialize the mask result */
    uint32_t adc_mask_result = 0U;
    int32_t adc_mask_count = 32;
    while (0U == adc_mask_result)
    {
        adc_mask_count--;
        adc_mask_result = (uint32_t)(adc_mask & (1U << adc_mask_count));
    }
    if ((uint32_t)(1U << adc_mask_count) == ADC_MASK_TEMPERATURE)
    {
        adc_mask_count = ADC_REG_TEMPERATURE;
    }
    else if ((uint32_t)(1U << adc_mask_count) == ADC_MASK_VOLT)
    {
        adc_mask_count = ADC_REG_VOLT;
    }

    return adc_mask_count;
}

