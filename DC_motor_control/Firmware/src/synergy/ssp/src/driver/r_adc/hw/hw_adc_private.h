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
* File Name    : hw_adc_private.h
* Description  : ADC LLD implementation
***********************************************************************************************************************/

#ifndef HW_ADC_PRIVATE_H
#define HW_ADC_PRIVATE_H

/**********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "bsp_api.h"
#include "r_adc.h"


/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#ifdef R_S12ADC0_BASE
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define ADC_BASE_PTR  R_S12ADC0_Type *
#elif defined(R_S14ADC_BASE)
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define ADC_BASE_PTR  R_S14ADC_Type *
#elif defined(R_S16ADC_BASE)
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define ADC_BASE_PTR  R_S16ADC_Type *
#endif

/** Value for disabling AVEE bit in ADADC */
#define ADC_ADADC_DISABLE_AVEE (0x07U)

/** Define pointer type for the TSN Control Register Base */
#define TSN_BASE_PTR R_TSN_Control_Type *

/** Define pointer type for the TSN Calibration Register Base */
#define TSN_CALIB_BASE_PTR R_TSN_Calibration_Type *

/** Sample and hold bypass applies to these channels. */
#define ADC_MASK_SAMPLE_HOLD_BYPASS_CHANNELS  (0x3U)

/** Sample and hold bypass starts at bit 8. */
#define ADC_MASK_SAMPLE_HOLD_BYPASS_SHIFT     (9U)

/** Value for ADPGADCR0 to disable PGA */
#define ADC_ADPGADCR0_DISABLE_PGA (0x0000)
/** Value for ADPGACR to disable PGA */
#define ADC_ADPGACR_DISABLE_PGA   (0x9999)
/** Position of ADCALEXE to start calibration-CALEXE bit */
#define ADC_ADCALEXE_SET_CALEXE   (0x80U)
/** Position of ADCALEXE calibration-CALMON bit */
#define ADC_ADCALEXE_CALIBRATION_STATUS   (0x40U)

/** Value of ADICR with interrupt at end of calibration */
#define ADC_ADICR_CALIBRATION_INTERRUPT_ENABLED   (0x03U)
/** Value of ADICR with no interrupt at end of calibration */
#define ADC_ADICR_CALIBRATION_INTERRUPT_DISABLED   (0x00U)
/**********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/** Defines the registers settings for the ADC synchronous ELC trigger*/
typedef enum e_adc_elc_trigger
{
    ADC_ELC_TRIGGER_EXTERNAL = (0x00U),
    ADC_ELC_TRIGGER          = (0x09U),
    ADC_ELC_TRIGGER_GROUP_B  = (0x0AU),
    ADC_ELC_TRIGGER_DISABLED = (0x3FU)
} adc_elc_trigger_t;

/** ADCSR register ADCS field */
typedef enum e_adc_adcs
{
    ADC_ADCS_SINGLE_SCAN=0U,
    ADC_ADCS_GROUP_SCAN=1U,
    ADC_ADCS_CONT_SCAN=2U,
    ADC_ADCS_MAX
} adc_adcs_t;

/** State of ADST bit*/
typedef enum e_hw_adc_adcsr_adst
{
    HW_ADC_ADCSR_ADCST_NOT_SET= (0x0),
    HW_ADC_ADCSR_ADCST_SET    = (0x1),
    HW_ADC_ADCSR_ADCST_MAX
} hw_adc_adcsr_adst_t;

/** State of TRGE bit*/
typedef enum e_hw_adc_adcsr_trge
{
    HW_ADC_ADCSR_TRGE_NOT_SET= (0x0),
    HW_ADC_ADCSR_TRGE_SET    = (0x1),
    HW_ADC_ADCSR_TRGE_MAX
} hw_adc_adcsr_trge_t;

/** State of ADIE bit*/
typedef enum e_hw_adc_adcsr_adie
{
    HW_ADC_ADCSR_ADIE_NOT_SET= (0x0),
    HW_ADC_ADCSR_ADIE_SET    = (0x1),
    HW_ADC_ADCSR_ADIE_MAX
} hw_adc_adcsr_adie_t;

/** State of GBADIE bit*/
typedef enum e_hw_adc_adcsr_gbadie
{
    HW_ADC_ADCSR_GBADIE_NOT_SET= (0x0),
    HW_ADC_ADCSR_GBADIE_SET    = (0x1),
    HW_ADC_ADCSR_GBADIE_MAX
} hw_adc_adcsr_gbadie_t;

/** State of DBLE bit*/
typedef enum e_hw_adc_double_trigger
{
    HW_ADC_DOUBLE_TRIGGER_DESELECTED = (0x0),
    HW_ADC_DOUBLE_TRIGGER_SELECTED   = (0x1),
} hw_adc_double_trigger_t;

/** State of PGS bit*/
typedef enum e_hw_adc_group_priority
{
    HW_ADC_GROUP_PRIORITY_DISABLED          = (0x0),
    HW_ADC_GROUP_PRIORITY_GROUP_A_CONTROL   = (0x1),
} hw_adc_group_priority_t;

/***********************************************************************************************************************
 * Private Global Variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/


__STATIC_INLINE void HW_ADC_ADCSR_ADST_Set(ADC_BASE_PTR p_regs, hw_adc_adcsr_adst_t value)
{
    p_regs->ADCSR_b.ADST = value;
}

__STATIC_INLINE hw_adc_adcsr_adst_t HW_ADC_ADCSR_ADST_Get(ADC_BASE_PTR p_regs)
{
    return((hw_adc_adcsr_adst_t)p_regs->ADCSR_b.ADST);
}

__STATIC_INLINE void HW_ADC_ADCSR_TRGE_Set(ADC_BASE_PTR p_regs, hw_adc_adcsr_trge_t value)
{
    p_regs->ADCSR_b.TRGE = value;
}

__STATIC_INLINE hw_adc_adcsr_trge_t HW_ADC_ADCSR_TRGE_Get(ADC_BASE_PTR p_regs)
{
    return((hw_adc_adcsr_trge_t)p_regs->ADCSR_b.TRGE);
}

__STATIC_INLINE void HW_ADC_ADCSR_ADIE_Set(ADC_BASE_PTR p_regs, hw_adc_adcsr_adie_t value)
{
    SSP_PARAMETER_NOT_USED(p_regs);
    SSP_PARAMETER_NOT_USED(value);
}

__STATIC_INLINE void HW_ADC_ADCSR_GBADIE_Set(ADC_BASE_PTR p_regs, hw_adc_adcsr_gbadie_t value)
{
    p_regs->ADCSR_b.GBADIE = value;
}

__STATIC_INLINE void HW_ADC_SensorCfgTemperatureGroupA(ADC_BASE_PTR p_regs)
{
    p_regs->ADEXICR_b.TSSA = 1U;      /** Select temperature output GroupA */
    p_regs->ADEXICR_b.TSSB = 0U;
}

__STATIC_INLINE void HW_ADC_SensorCfgTemperatureGroupB(ADC_BASE_PTR p_regs)
{
    p_regs->ADEXICR_b.TSSA = 0U;
    p_regs->ADEXICR_b.TSSB = 1U;      /** Select temperature output GroupB */
}

__STATIC_INLINE void HW_ADC_SensorCfgVoltageGroupA(ADC_BASE_PTR p_regs)
{
    p_regs->ADEXICR_b.OCSA = 1U;      /** Select voltage output to GroupA  */
    p_regs->ADEXICR_b.OCSB = 0U;
}

__STATIC_INLINE void HW_ADC_SensorCfgVoltageGroupB(ADC_BASE_PTR p_regs)
{
    p_regs->ADEXICR_b.OCSA = 0U;
    p_regs->ADEXICR_b.OCSB = 1U;      /** Select voltage output to GroupB  */
}

__STATIC_INLINE void HW_ADC_S12gbadiEnable(ADC_BASE_PTR p_regs)
{
    p_regs->ADCSR_b.GBADIE = 1U;                /*** Enable in peripheral  */
}

__STATIC_INLINE void HW_ADC_S12gbadiDisable(ADC_BASE_PTR p_regs)
{
    p_regs->ADCSR_b.GBADIE = 0UL;                /*** Disable in peripheral  */
}

__STATIC_INLINE void HW_ADC_SampleStateRegSet(ADC_BASE_PTR p_regs, adc_sample_state_reg_t reg_id, uint8_t num_states)
{
    p_regs->ADSSTR0n[reg_id] = num_states;
}

__STATIC_INLINE uint16_t const volatile * HW_ADC_ResultRegAddrGet(ADC_BASE_PTR p_regs, int32_t index)
{
    return (&p_regs->ADDRn[index]);
}

__STATIC_INLINE adc_data_size_t HW_ADC_ResultGet(ADC_BASE_PTR p_regs, int32_t index)
{
    return (adc_data_size_t) p_regs->ADDRn[index];
}

__STATIC_INLINE adc_mode_t HW_ADC_ScanModeGet(ADC_BASE_PTR p_regs)
{
    return (adc_mode_t) p_regs->ADCSR_b.ADCS;
}

__STATIC_INLINE void HW_ADC_ScanModeSet(ADC_BASE_PTR p_regs, adc_mode_t mode)
{
    p_regs->ADCSR_b.ADCS = mode;
}

__STATIC_INLINE hw_adc_double_trigger_t HW_ADC_DoubleTriggerGet(ADC_BASE_PTR p_regs)
{
    return (hw_adc_double_trigger_t) p_regs->ADCSR_b.DBLE;
}

__STATIC_INLINE void HW_ADC_DoubleTriggerSet(ADC_BASE_PTR p_regs, hw_adc_double_trigger_t double_trigger)
{
    p_regs->ADCSR_b.DBLE = double_trigger;
}

__STATIC_INLINE void HW_ADC_RegisterReset(ADC_BASE_PTR p_regs)
{
    /** Clear all settings including disabling interrupts*/
    p_regs->ADCSR = 0UL;
    /** Clear the sensor settings*/
    p_regs->ADEXICR = 0UL;
}

__STATIC_INLINE void HW_ADC_SampleHoldSet(ADC_BASE_PTR p_regs, uint16_t value)
{
    p_regs->ADSHCR = value;
}

__STATIC_INLINE uint8_t HW_ADC_DisconnectDetectGet(ADC_BASE_PTR p_regs)
{
    return p_regs->ADDISCR;
}

__STATIC_INLINE void HW_ADC_TemperatureSensorCfg(TSN_BASE_PTR p_tsn_regs)
{
    p_tsn_regs->TSCR_b.TSEN = 1U;             /** Enable temperature sensor           */
    /** 30us delay  to stabilize the clock before an accurate reading can be taken.
     * This only applies when the sensor is powered on the first time */
    p_tsn_regs->TSCR_b.TSOE = 1U;             /** Enable temperature sensor output to ADC */
}
__STATIC_INLINE void HW_ADC_TemperatureSensorAddCfg(ADC_BASE_PTR p_regs, uint16_t temperature_add)
{
    /**Configure addition mode for the temperature sensor */
    p_regs->ADEXICR_b.TSSAD = (temperature_add & 1U);
}
__STATIC_INLINE void HW_ADC_CalibrationRegRead(TSN_CALIB_BASE_PTR p_regs, uint32_t * p_calibration_data)
{
    /** Read into memory to prevent compiler warning when performing "|" on volatile register data*/
    uint32_t data = p_regs -> TSCDR;
    /** Read the calibration data from ROM and mask the upper 20 bits as the lower 12 bits carry the data*/
    *p_calibration_data = (data & 0x00000FFF);

}
__STATIC_INLINE void HW_ADC_GroupPrioritySet(ADC_BASE_PTR p_regs, hw_adc_group_priority_t value)
{
    p_regs->ADGSPCR_b.PGS = value;
}

__STATIC_INLINE void HW_ADC_ElcTriggerGroupASet(ADC_BASE_PTR p_regs, adc_elc_trigger_t value)
{
    p_regs->ADSTRGR_b.TRSA = value;
}

__STATIC_INLINE void HW_ADC_ElcTriggerGroupBSet(ADC_BASE_PTR p_regs, adc_elc_trigger_t value)
{
    p_regs->ADSTRGR_b.TRSB = value;
}

__STATIC_INLINE void HW_ADC_ScanMaskSet(ADC_BASE_PTR p_regs, adc_channel_cfg_t const * const p_cfg)
{
    /** Set mask for channels and sensors for channels 0 to 15. */
    p_regs->ADANSA0 = (uint16_t) (p_cfg->scan_mask);
    p_regs->ADANSB0 = (uint16_t) (p_cfg->scan_mask_group_b);
    p_regs->ADADS0 = (uint16_t) (p_cfg->add_mask);

    /** Set mask for channels and sensors for higher channels   */
    p_regs->ADANSA1 = (uint16_t) ((p_cfg->scan_mask >> 16));
    p_regs->ADANSB1 = (uint16_t) ((p_cfg->scan_mask_group_b >> 16));
    p_regs->ADADS1 = (uint16_t) ((p_cfg->add_mask >> 16));
}

__STATIC_INLINE void HW_ADC_Init(ADC_BASE_PTR const p_regs, adc_cfg_t const * const p_cfg)
{
#if defined (R_S12ADC0_BASE) || defined (R_S14ADC_BASE)
    /** Configure ADC resolution for this unit */
    p_regs->ADCER_b.ADPRC = p_cfg->resolution;
    /** Configure ADC alignment for this unit */
    p_regs->ADCER_b.ADRFMT = (ADC_ALIGNMENT_LEFT == p_cfg->alignment) ? 1U : 0U;
#endif
    /** Configure is the result register should be cleared after reading for this unit */
    p_regs->ADCER_b.ACE = (ADC_CLEAR_AFTER_READ_ON == p_cfg->clearing) ? 1U : 0U;
    /** Configure the count for result addition or averaging*/
#if defined R_S16ADC_BASE
    p_regs->ADADC = (p_cfg->add_average_count & ADC_ADADC_DISABLE_AVEE);
#else
    p_regs->ADADC = p_cfg->add_average_count;
#endif
}

__STATIC_INLINE void HW_ADC_Deselect_data_inversion(ADC_BASE_PTR const p_regs)
{
#if defined R_S16ADC_BASE
    p_regs->ADCER_b.ADINV = 1U;
#else
    SSP_PARAMETER_NOT_USED(p_regs);
#endif
}

__STATIC_INLINE void HW_ADC_ADICR_Set (ADC_BASE_PTR const p_regs, uint8_t value)
{
#if defined R_S16ADC_BASE
    p_regs->ADICR_b.ADIC = (uint8_t) (value & 0x03);
#else
    SSP_PARAMETER_NOT_USED(p_regs);
    SSP_PARAMETER_NOT_USED(value);
#endif
}

__STATIC_INLINE uint8_t HW_ADC_ADICR_Get (ADC_BASE_PTR const p_regs)
{
#if defined R_S16ADC_BASE
    return p_regs->ADICR_b.ADIC;
#else
    SSP_PARAMETER_NOT_USED(p_regs);
    return 0U;
#endif
}

__STATIC_INLINE void HW_ADC_ADCALEXE_Set(ADC_BASE_PTR const p_regs, uint8_t value)
{
#if defined R_S16ADC_BASE
    p_regs->ADCALEXE = value;
#else
    SSP_PARAMETER_NOT_USED(p_regs);
    SSP_PARAMETER_NOT_USED(value);
#endif
}

__STATIC_INLINE uint8_t HW_ADC_ADCALEXE_Get(ADC_BASE_PTR const p_regs)
{
#if defined R_S16ADC_BASE
    return p_regs->ADCALEXE;
#else
    SSP_PARAMETER_NOT_USED(p_regs);
   return 0U;
#endif
}

__STATIC_INLINE void HW_ADC_GroupAPrioritySet(ADC_BASE_PTR p_regs, adc_group_a_t priority)
{
    p_regs->ADGSPCR = priority;
}

__STATIC_INLINE void HW_ADC_TemperatureSampleStateSet(ADC_BASE_PTR p_regs, uint8_t num_states)
{
    p_regs->ADSSTRT = num_states;
}

__STATIC_INLINE void HW_ADC_VoltageSampleStateSet(ADC_BASE_PTR p_regs, uint8_t num_states)
{
    p_regs->ADSSTRO = num_states;
}

__STATIC_INLINE void HW_ADC_VoltageSensorAddSet(ADC_BASE_PTR p_regs, uint16_t voltage_add)
{
    p_regs->ADEXICR_b.OCSAD = (voltage_add & 1U);
}

__STATIC_INLINE void HW_ADC_AsyncTriggerEnable(ADC_BASE_PTR p_regs)
{
    p_regs->ADCSR_b.EXTRG = 1U;
}

__STATIC_INLINE adc_add_t HW_ADC_AddModeGet(ADC_BASE_PTR p_regs)
{
    return (adc_add_t) p_regs->ADADC_b.ADC;
}

__STATIC_INLINE void HW_ADC_TempSensorDisable(R_TSN_Control_Type * p_tsn_regs)
{
    p_tsn_regs->TSCR = 0UL;
}

__STATIC_INLINE void HW_ADC_ADPGADCR0_Set(ADC_BASE_PTR p_regs, adc_cfg_t const * const p_cfg)
{

    if(( DIFFERENTIAL_INPUT_GAIN_1 <= p_cfg->pga0 ) && (DIFFERENTIAL_INPUT_GAIN_4 >= p_cfg->pga0))
    {
        p_regs->ADPGADCR0_b.P000DG = (0x03U & (p_cfg->pga0 >> 4U));  /*PGA-0 Differential input Gain setting(0,1,2,3).*/
        p_regs->ADPGADCR0_b.P000DEN = 1U;                            /*PGA-0 Differential input Enabled.*/
    }
    else
    {
        p_regs->ADPGADCR0_b.P000DEN = 0U;                           /* Single end mode Enabled */
    }

    if(( DIFFERENTIAL_INPUT_GAIN_1 <= p_cfg->pga1 ) && (DIFFERENTIAL_INPUT_GAIN_4 >= p_cfg->pga1))
    {
        p_regs->ADPGADCR0_b.P001DG = (0x03U & (p_cfg->pga1 >> 4U)); /* PGA-1 Differential input Gain setting(0,1,2,3).*/
        p_regs->ADPGADCR0_b.P001DEN = 1U;                           /* PGA-1 Differential input Enabled.*/
    }
    else
    {
        p_regs->ADPGADCR0_b.P001DEN = 0U;                           /* Single end mode Enabled.*/
    }

    if(( DIFFERENTIAL_INPUT_GAIN_1 <= p_cfg->pga2 ) && (DIFFERENTIAL_INPUT_GAIN_4 >= p_cfg->pga2))
    {
        p_regs->ADPGADCR0_b.P002DG = (0x03U & (p_cfg->pga2 >> 4U)); /* PGA-2 Differential input Gain setting(0,1,2,3).*/
        p_regs->ADPGADCR0_b.P002DEN = 1U;                           /* PGA-2 Differential input Enabled.*/
    }
    else
    {
        p_regs->ADPGADCR0_b.P002DEN = 0U;                           /* Single end mode Enabled.*/
    }
}

__STATIC_INLINE void HW_ADC_ADPGAGS0_Set(ADC_BASE_PTR p_regs, adc_cfg_t const * const p_cfg)
{
    /* Gain level setting for PGA0,PGA1,PGA2 */
    p_regs->ADPGAGS0_b.P000GAIN = (uint16_t)(p_cfg->pga0 & 0x000F ); /* PGA-0 Single Ended(0-15) & Differential input(1,5,9,11) gain setting Register. */
    p_regs->ADPGAGS0_b.P001GAIN = (uint16_t)(p_cfg->pga1 & 0x000F ); /* PGA-1 Single Ended(0-15) & Differential input(1,5,9,11) gain setting Register. */
    p_regs->ADPGAGS0_b.P002GAIN = (uint16_t)(p_cfg->pga2 & 0x000F ); /* PGA-2 Single Ended(0-15) & Differential input(1,5,9,11) gain setting Register. */
}

__STATIC_INLINE void HW_ADC_ADPGACR_Set(ADC_BASE_PTR p_regs,adc_cfg_t const * const p_cfg)
{

    /* PGA-0, PGA-1, PGA-2 Enable  Gain Setting.*/
    p_regs->ADPGACR_b.P000GEN = 1U;
    p_regs->ADPGACR_b.P001GEN = 1U;
    p_regs->ADPGACR_b.P002GEN = 1U;

    /* PGA-0 Bypass/Enable/Disable */
    if (PGA_DISABLE == p_cfg->pga0)
    {
        p_regs->ADPGACR_b.P000SEL0 = 1U;
        p_regs->ADPGACR_b.P000SEL1 = 0U;
        p_regs->ADPGACR_b.P000ENAMP = 0U;
    }
    else
    {
        p_regs->ADPGACR_b.P000SEL0 = 0U;
        p_regs->ADPGACR_b.P000SEL1 = 1U;
        p_regs->ADPGACR_b.P000ENAMP = 1U;
    }

    /* PGA-1 Bypass/Enable/Disable */
    if (PGA_DISABLE == p_cfg->pga1)
    {
        p_regs->ADPGACR_b.P001SEL0 = 1U;
        p_regs->ADPGACR_b.P001SEL1 = 0U;
        p_regs->ADPGACR_b.P001ENAMP = 0U;
    }
    else
    {
        p_regs->ADPGACR_b.P001SEL0 = 0U;
        p_regs->ADPGACR_b.P001SEL1 = 1U;
        p_regs->ADPGACR_b.P001ENAMP = 1U;
    }
    /* PGA-2 Bypass/Enable/Disable */
    if (PGA_DISABLE == p_cfg->pga2)
    {
        p_regs->ADPGACR_b.P002SEL0 = 1U;
        p_regs->ADPGACR_b.P002SEL1 = 0U;
        p_regs->ADPGACR_b.P002ENAMP = 0U;
    }
    else
    {
        p_regs->ADPGACR_b.P002SEL0 = 0U;
        p_regs->ADPGACR_b.P002SEL1 = 1U;
        p_regs->ADPGACR_b.P002ENAMP = 1U;
    }
}

__STATIC_INLINE void HW_ADC_VREFAMPCNT_Set(ADC_BASE_PTR p_regs, uint8_t value)
{
    p_regs->VREFAMPCNT_b.VREFADCG = (value & 3U);
    p_regs->VREFAMPCNT_b.BGREN = 1U;
}
__STATIC_INLINE void HW_ADC_VREFAMPCNT_Set_Enabled(ADC_BASE_PTR p_regs, uint8_t value)
{
    p_regs->VREFAMPCNT_b.VREFADCEN = 0U;
    if(value >= 1U)
    {
        p_regs->VREFAMPCNT_b.VREFADCEN = 1U;
    }
}
__STATIC_INLINE void HW_ADC_OVERCURRENT_Set(ADC_BASE_PTR p_regs, adc_over_current_t value)
{
    p_regs->VREFAMPCNT_b.OLDETEN = value;
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file.*/
SSP_FOOTER

#endif /* HW_ADC_PRIVATE_H */
