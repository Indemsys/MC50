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
* File Name    : r_adc.h
* @brief       : Functions for configuring and using the ADC
***********************************************************************************************************************/

#ifndef R_ADC_H
#define R_ADC_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include <stdlib.h>
/* Fixed width integer support. */
#include <stdint.h>
/* bool support */
#include <stdbool.h>
#include "bsp_api.h"
#include "r_adc_cfg.h"
#include "r_adc_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup ADC ADC
 * @brief Driver for the 14-Bit A/D Converter (ADC14) and 12-bit A/D Converter (ADC12).
 *
 * This module supports the ADC14 and ADC12 peripherals.  It implements
 * the following interfaces:
 *   - @ref ADC_API
 * @{
***********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/** Version of code that implements the API defined in this file */
#define ADC_CODE_VERSION_MAJOR  (2U)
#define ADC_CODE_VERSION_MINOR  (0U)

/** Typical values that can be used to modify the sample states.
 * The minimum sample state count value is either 6 or 7 depending on the clock ratios.
 * It is fixed to 7 based on the fact that at the lowest ADC conversion clock supported (1 MHz)
 * this extra state will lead to at worst a "1 microsecond" increase in conversion time.
 * At 60 MHz the extra sample state will add 16.7 ns to the conversion time.
 */
#define ADC_SAMPLE_STATE_COUNT_MIN     (7U)
#define ADC_SAMPLE_STATE_COUNT_MAX     (255U)

/** Typical values that can be used for the sample and hold counts for the channels 0-2*/
/** Minimum sample and hold states */
#define ADC_SAMPLE_STATE_HOLD_COUNT_MIN      (4U)
/** Default sample and hold states */
#define ADC_SAMPLE_STATE_HOLD_COUNT_DEFAULT  (24U)

/** For ADC Scan configuration adc_channel_cfg_t::scan_mask, scan_mask_group_b,
 * add_mask and sample_hold_mask      */
/** Use bitwise OR to combine these masks for desired channels and sensors.    */
#define ADC_MASK_CHANNEL_0    (1U<<0U)
#define ADC_MASK_CHANNEL_1    (1U<<1U)
#define ADC_MASK_CHANNEL_2    (1U<<2U)
#define ADC_MASK_CHANNEL_3    (1U<<3U)
#define ADC_MASK_CHANNEL_4    (1U<<4U)
#define ADC_MASK_CHANNEL_5    (1U<<5U)
#define ADC_MASK_CHANNEL_6    (1U<<6U)
#define ADC_MASK_CHANNEL_7    (1U<<7U)
#define ADC_MASK_CHANNEL_8    (1U<<8U)
#define ADC_MASK_CHANNEL_9    (1U<<9U)
#define ADC_MASK_CHANNEL_10   (1U<<10U)
#define ADC_MASK_CHANNEL_11   (1U<<11U)
#define ADC_MASK_CHANNEL_12   (1U<<12U)
#define ADC_MASK_CHANNEL_13   (1U<<13U)
#define ADC_MASK_CHANNEL_14   (1U<<14U)
#define ADC_MASK_CHANNEL_15   (1U<<15U)
#define ADC_MASK_CHANNEL_16   (1U<<16U)
#define ADC_MASK_CHANNEL_17   (1U<<17U)
#define ADC_MASK_CHANNEL_18   (1U<<18U)
#define ADC_MASK_CHANNEL_19   (1U<<19U)
#define ADC_MASK_CHANNEL_20   (1U<<20U)
#define ADC_MASK_CHANNEL_21   (1U<<21U)
#define ADC_MASK_CHANNEL_22   (1U<<22U)
#define ADC_MASK_CHANNEL_23   (1U<<23U)
#define ADC_MASK_CHANNEL_24   (1U<<24U)
#define ADC_MASK_CHANNEL_25   (1U<<25U)
#define ADC_MASK_CHANNEL_26   (1U<<26U)
#define ADC_MASK_CHANNEL_27   (1U<<27U)
#define ADC_MASK_TEMPERATURE  (1U<<28UL)
#define ADC_MASK_VOLT         (1U<<29UL)
#define ADC_MASK_SENSORS (ADC_MASK_TEMPERATURE | ADC_MASK_VOLT)
#define ADC_MASK_GROUP_B_OFF  (0UL)
#define ADC_MASK_ADD_OFF      (0UL)
#define ADC_MASK_SAMPLE_HOLD_OFF (0U)

/** Sample and hold Channel mask. Sample and hold is only available for channel 0,1,2*/
#define ADC_SAMPLE_HOLD_CHANNELS     (0x07U)
/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/** ADC instance control block. DO NOT INITIALIZE.  Initialized in adc_api_t::open(). */
typedef struct
{
    uint16_t                 unit;               ///< ADC Unit in use
    int16_t                  slope_microvolts;   ///< Temperature sensor slope in microvolts/°C
    adc_mode_t               mode;               ///< operational mode
    adc_alignment_t          alignment;          ///< alignment
    uint8_t                  max_resolution;     ///< ADC max resolution: 8, 10, 12, or 14-bit
    uint8_t                  pga_available;      ///< PGA available or not on MCU
    uint8_t                  tsn_ctrl_available; ///< Availability of TSN control register
    uint8_t                  tsn_calib_available;///< Availability of TSn calibration register
    uint8_t                  adc_calib_available;///< Availability of ADC calibration feature
    R_TSN_Control_Type     * p_tsn_ctrl_regs; ///< Pointer to temperature control register
    R_TSN_Calibration_Type * p_tsn_calib_regs;///< Pointer to temperature calibration register
    void const             * p_context;       ///< Placeholder for user data
    void                   * p_reg;           ///< Base register for this unit
    void                  (*callback)(adc_callback_args_t *p_args); ///< User callback pointer
    adc_trigger_t            trigger;         ///< Trigger defined for normal mode
    uint32_t                 opened;          ///< Boolean to verify that the Unit has been initialized
    uint32_t                 scan_mask;       ///< Scan mask used for Normal scan.
    IRQn_Type                scan_end_irq;    ///< Scan end IRQ number
    IRQn_Type                scan_end_b_irq;  ///< Scan end group B IRQ number
    adc_voltage_reference_t  voltage_ref;     ///< ADC reference voltage selection. Default is VREF
    adc_over_current_t       over_current;    ///< ADC reference voltage selection. Default is Over current
    adc_pga_t                pga0;            ///< PGA0 setting
    adc_pga_t                pga1;            ///< PGA1 setting
    adc_pga_t                pga2;            ///< PGA2 setting
} adc_instance_ctrl_t;

/**********************************************************************************************************************
Exported global variables
***********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Interface Structure for user access */
extern const adc_api_t g_adc_on_adc;
/** @endcond */


/*******************************************************************************************************************//**
 * @} (end defgroup ADC)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_ADC_H */
