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
* File Name    : hw_sci_common.h
* @brief    SCI LLD definitions, common portion
***********************************************************************************************************************/

#ifndef HW_SCI_COMMON_H
#define HW_SCI_COMMON_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
/* Includes board and MCU related header files. */
#include "bsp_api.h"

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/** SCI SCR register bit masks */
#define SCI_SCR_TE_MASK         (0x20U)     ///< transmitter enable
#define SCI_SCR_RE_MASK         (0x10U)     ///< receiver enable
#define SCI_SCR_TE_RE_MASK      (0x30U)     ///< transmitter & receiver enable
#define SCI_SCR_CKE_VALUE_MASK  (0x03U)     ///< CKE: 2 bits

/** SCI SEMR register bit masks */
#define SCI_SEMR_BGDM_VALUE_MASK    (0x01U)     ///< BGDM: 1 bit
#define SCI_SEMR_ABCS_VALUE_MASK    (0x01U)     ///< ABCS: 1 bit
#define SCI_SEMR_ABCSE_VALUE_MASK   (0x01U)     ///< ABCSE: 1 bit

/** SCI SMR register bit masks */
#define SCI_SMR_CKS_VALUE_MASK      (0x03U)     ///< CKS: 2 bits

/** SCI SSR register receiver error bit masks */
#define SCI_SSR_ORER_MASK   (0x20U)     ///< overflow error
#define SCI_SSR_FER_MASK    (0x10U)     ///< framing error
#define SCI_SSR_PER_MASK    (0x08U)     ///< parity err
#define SCI_RCVR_ERR_MASK   (SCI_SSR_ORER_MASK | SCI_SSR_FER_MASK | SCI_SSR_PER_MASK)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/** Baud rate divisor information
 * BRR(N) = (PCLK / (divisor * Baudrate(B))) - 1
 * when ABCSE=1,                         divisor = 12*pow(2,2N-1)
 * when ABCSE=1, BGDM=1&&ABCS=1,         divisor = 16*pow(2,2N-1)
 * when ABCSE=0, one of BGDM or ABCS =1, divisor = 32*pow(2,2N-1)
 * when ABCSE=0, BGDM=0&&ABCS=0,         divisor = 64*pow(2,2N-1)
 */
typedef struct st_baud_setting_t
{
    uint16_t    div_coefficient;   /**< Divisor coefficient */
    uint8_t     bgdm;               /**< BGDM value to get divisor */
    uint8_t     abcs;               /**< ABCS value to get divisor */
    uint8_t     abcse;              /**< ABCSE value to get divisor */
    uint8_t     cks;                /**< CKS  value to get divisor (CKS = N) */
} baud_setting_t;

/** Noise filter setting definition */
typedef enum e_noise_cancel_lvl
{
    NOISE_CANCEL_LVL1,          /**< Noise filter level 1(weak) */
    NOISE_CANCEL_LVL2,          /**< Noise filter level 2 */
    NOISE_CANCEL_LVL3,          /**< Noise filter level 3 */
    NOISE_CANCEL_LVL4           /**< Noise filter level 4(strong) */
} noise_cancel_lvl_t;

/***********************************************************************************************************************
Private function prototypes
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables
***********************************************************************************************************************/

/***********************************************************************************************************************
Private Functions
***********************************************************************************************************************/

/*******************************************************************************************************************//**
* Enables reception and transmission for the specified SCI channel
* @param[in] channel  SCI channel
* @retval    void
* @note      Channel number is not checked in this function, caller function must check it.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_TransmitterReceiverEnable (R_SCI0_Type * p_reg)
{
    p_reg->SCR |= SCI_SCR_TE_RE_MASK;
}  /* End of function HW_SCI_TransmitterReceiverEnable() */

/*******************************************************************************************************************//**
* Disables reception  and transmission for the specified SCI channel
* @param[in] channel  SCI channel
* @retval    void
* @note      Channel number is not checked in this function, caller function must check it.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_TransmitterReceiverDisable (R_SCI0_Type * p_reg)
{
    p_reg->SCR &= (uint8_t)(~SCI_SCR_TE_RE_MASK);
}  /* End of function HW_SCI_TransmitterReceiverDisable() */

/*******************************************************************************************************************//**
* This function writes data to transmit data register.
* @param[in] channel  SCI channel
* @param[in] data     Data to be sent
* @retval    void
* @note      All the parameter check must be held by HLD
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_Write (R_SCI0_Type * p_reg, uint8_t const data)
{
    while (0U == p_reg->SSR_b.TDRE)
    {
        /* Wait until TDRE is cleared */
    }

    /* Write 1byte data to data register */
    p_reg->TDR = data;
}  /* End of function HW_SCI_Write() */

/*******************************************************************************************************************//**
* This function reads data from receive data register
* @param[in] channel    SCI Channel number
* @note      Channel number is not checked in this function, caller function must check it.
***********************************************************************************************************************/
__STATIC_INLINE uint8_t HW_SCI_Read (R_SCI0_Type * p_reg)
{
    return p_reg->RDR;
}  /* End of function HW_SCI_Read() */

/*******************************************************************************************************************//**
* This function initializes all SCI registers.
* @param[in] channel    SCI Channel number
* @retval    none
* @note      All the parameter check must be held by HLD
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_RegisterReset (R_SCI0_Type * p_reg)
{
    p_reg->SMR = 0U;
    p_reg->SCR = 0U;
    p_reg->SSR = 0U;
    p_reg->SCMR = 0xF2U;
    p_reg->BRR = 0xFFU;
    p_reg->MDDR = 0xFFU;
    p_reg->SEMR = 0U;
    p_reg->SNFR = 0U;
    p_reg->SIMR1 = 0U;
    p_reg->SIMR2 = 0U;
    p_reg->SIMR3 = 0U;
    p_reg->SISR = 0U;
    p_reg->SPMR = 0U;
    p_reg->SISR = 0U;
    p_reg->FCR = 0xF800U;
    p_reg->CDR = 0U;
    p_reg->DCCR = 0x40U;
    p_reg->SPTR = 0x03U;
}  /* End of function HW_SCI_RegisterReset() */


/*******************************************************************************************************************//**
* Selects internal clock for baud rate generator
* @param[in] channel    SCI Channel number
* @retval    void
* @note      the parameter check must be held by HLD
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_BaudRateGenInternalClkSelect (R_SCI0_Type * p_reg)
{
    p_reg->SCR_b.CKE    = 0U;             /* Internal clock */
}  /* End of function HW_SCI_BaudRateGenInternalClkSelect() */

/*****************************************************************************************************************//**
 * @brief     Sets BRR for baud rate generator register
 * @param[in] channel    SCI Channel number
 * @param[in] brr        BRR register setting  value
 *            clk_div    Clock divisor (n value)
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_BitRateBRRSet (R_SCI0_Type * p_reg, const uint8_t brr, const uint8_t clk_div)
{
    /* Setting the BRR bits for the baud rate register */
    p_reg->BRR       = brr;
    p_reg->SMR_b.CKS = (uint8_t)(SCI_SMR_CKS_VALUE_MASK & clk_div);
}  /* End of function HW_SCI_BitRateBRRSet() */

/*******************************************************************************************************************//**
* Sets Noise cancel filter
* @param[in] channel   SCI Channel number
* @param[in] level  Noise cancel level
* @retval    void
* @note      Channel number and argument check is omitted, must be checked by SCI HLD
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_NoiseFilterSet (R_SCI0_Type * p_reg, noise_cancel_lvl_t level)
{
    p_reg->SEMR_b.NFEN = 1U;      /* enable noise filter */
    p_reg->SNFR = level;
}  /* End of function HW_SCI_NoiseFilterSet() */

/*****************************************************************************************************************//**
 * @brief     This function selects transfer direction (MSB first or LSB first)
 * @param[in] channel   SCI channel
 * @retval    void
 * @note      Channel number is not checked in this function, caller function must check it.
 ********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_TransferDirection (R_SCI0_Type * p_reg, bool const direction)
{
    /* Set the SCMR.SDIR to 0 or 1 (0: transfer with LSB first; 1: transfer with MSB first) */
    if (direction)
    {
        p_reg->SCMR_b.SDIR = 1U;
    }
    else
    {
        p_reg->SCMR_b.SDIR = 0U;
    }
}  /* End of function HW_SCI_TransferDirection() */

/*******************************************************************************************************************//**
 * Function for enabling/disabling the data inversion in the Smart card mode register (SCMR).
 * @param   channel The hardware channel to access
 * @param   enable  true or false
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_DataInversionEnable(R_SCI0_Type * p_reg, bool const enable)
{
    /* enable/disable bit */
    p_reg->SCMR_b.SINV = (uint8_t) (enable & 1U);
}  /* End of function HW_SCI_SIIC_DataDirectionEnable() */

/*******************************************************************************************************************//**
 * Function for enabling/disabling bit rate modulation function in serial extended mode register (SEMR).
 * @param   channel The hardware channel to access
 * @param   enable  true or false
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_BitRateModulationEnable(R_SCI0_Type * p_reg, bool const enable)
{
    /* enable/disable bit */
    p_reg->SEMR_b.BRME = (uint8_t) (enable & 1U);
}  /* End of function HW_SCI_BitRateModulationEnable() */

/*******************************************************************************************************************//**
 * Function for enabling/disabling smart card function in the Smart card mode register (SCMR).
 * @param   channel The hardware channel to access
 * @param   enable  true or false
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SmartCardEnable(R_SCI0_Type * p_reg, bool const enable)
{
    /* enable/disable bit */
    p_reg->SCMR_b.SMIF = (uint8_t) (enable & 1U);
}  /* End of function HW_SCI_SmartCardEnable() */

/*******************************************************************************************************************//**
 * Function for clearing the SPI mode register (SPMR).
 * @param   channel The hardware channel to access
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SPIModeClear(R_SCI0_Type * p_reg)
{
    /* Clear register */
    p_reg->SPMR = 0U;

}  /* End of function HW_SCI_SIIC_SPIModeClear() */

/*******************************************************************************************************************//**
 * Function for setting the communication mode to be be clock synchronous mode in the SMR register.
 * @param   channel The hardware channel to access
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_ClockSynModeClear(R_SCI0_Type * p_reg)
{
    /* Clear register */
    p_reg->SMR_b.CM = 0U;

}  /* End of function HW_SCI_ClockSynModeClear() */

/*******************************************************************************************************************//**
 * Function for clearing the serial mode register (SMR).
 * @param   channel The hardware channel to access
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SerialModeClear(R_SCI0_Type * p_reg)
{
    /*  Clear register */
    p_reg->SMR = 0U;
}  /* End of function HW_SCI_SIIC_SerialModeClear() */

/*******************************************************************************************************************//**
 * Function for clearing the smart card mode register (SCMR).
 * @param   channel The hardware channel to access
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SmartCardModeClear(R_SCI0_Type * p_reg)
{
    /* Clear register */
    p_reg->SCMR = 0U;
}  /* End of function HW_SCI_SmartCardModeClear() */

/*******************************************************************************************************************//**
 * Function for clearing the serial control register (SCR).
 * @param   channel The hardware channel to access
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SerialControlClear(R_SCI0_Type * p_reg)
{
    /* Clear register */
    p_reg->SCR = 0U;
}  /* End of function HW_SCI_SerialControlClear() */

/*******************************************************************************************************************//**
 * Function for clearing the Serial Status register (SSR).
 * @param   channel The hardware channel to access
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SerialStatusClear(R_SCI0_Type * p_reg)
{
    /* Clear SSR register */
    p_reg->SSR = 0U;
}  /* End of function HW_SCI_SerialStatusClear() */

/*******************************************************************************************************************//**
* Check BRME register valuse
* @param[in] channel    SCI Channel number
* @retval    bool       BRME bit value
* @note      the parameter check must be held by HLD
***********************************************************************************************************************/
__STATIC_INLINE bool HW_SCI_BitRateModulationCheck (R_SCI0_Type * p_reg)
{
    /* Read BRME value */
    return (1U == p_reg->SEMR_b.BRME);
}  /* End of function HW_SCI_BitRateModulationCheck() */

/*******************************************************************************************************************//**
* Sets MDDR register as calculated
* @param[in] channel    SCI Channel number
* @param[in] mddr        BRR register setting  value
* @retval    void
* @note      the parameter check must be held by HLD
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_UartBitRateModulationSet (R_SCI0_Type * p_reg, const uint8_t mddr)
{
    /* Set MBBR register value*/
    p_reg->MDDR          = mddr;

}  /* End of function HW_SCI_UartBitRateModulationSet() */

#endif // HW_SCI_COMMON_H

/* End of file */
