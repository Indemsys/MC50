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
 * File Name    : hw_rspi_common.h
 * Description  : Serial Peripheral Interface (RSPI) hardware common header file.
 **********************************************************************************************************************/



#ifndef HW_RSPI_COMMON_H
#define HW_RSPI_COMMON_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"

/* Public interface header file for this module. */
#include "r_rspi.h"
#include "../../r_rspi_private.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER


/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/** RSPI register access or macro  */
#define RSPI_REG(channel) ((R_RSPI0_Type *) ((uint32_t) R_RSPI0 + ((uint32_t) R_RSPI1 - (uint32_t) R_RSPI0) * (channel)))

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
/*****************************************************************************************************************//**
 * @brief     Sets BRR for baud rate generator register
 * @param[in] p_rspi_reg RSPI Registers
 * @param[in] brr        SPBR register setting  value
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_BitRateSet (R_RSPI0_Type * p_rspi_reg, const uint8_t brr)
{
    /* Setting the BRR bits for the bit rate register */
    p_rspi_reg->SPBR = brr;
}  /* End of function HW_RSPI_BitRateSet(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Clears the SPI command register 0
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_CommandClear (R_RSPI0_Type * p_rspi_reg)
{
    /* Clears the SPI command register SPCMdn */
    p_rspi_reg->SPCMDn[0] = 0;
}  /* End of function HW_RSPI_CommandClear(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Clears the RSPI Data Control register
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_DataControlClear (R_RSPI0_Type * p_rspi_reg)
{
    /* Clears data control register */
    p_rspi_reg->SPDCR = 0;
}  /* End of function HW_RSPI_DataControlClear(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the number of frames to used for SPI communication
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_DefaultDataFrame (R_RSPI0_Type * p_rspi_reg)
{
    /* Sets the frames number in data control register SPDCR */
     p_rspi_reg->SPDCR_b.SPFC = 0;
}  /* End of function HW_RSPI_DefaultDataFrame(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the default data access length to use longword
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_DefaultDataBitLength (R_RSPI0_Type * p_rspi_reg)
{
    /* Sets the default data access to use long word */
    /* 0 = word; 1 = long word */
    p_rspi_reg->SPDCR_b.SPLW = 1;
}  /* End of function HW_RSPI_DefaultDataBitLength(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the data access length to use halfword
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_DataBitLength_HalfWord (R_RSPI0_Type * p_rspi_reg)
{
    /* Sets the data access to use half word */
    /* 0 = word; 1 = long word */
     p_rspi_reg->SPDCR_b.SPLW = 0;
}  /* End of function HW_RSPI_DataBitLength_HalfWord(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the default Bit Rate Divisor in the SPI command register 0
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_DefaultBRDV (R_RSPI0_Type * p_rspi_reg)
{
    /* Sets the default Bit Rate Divisor for the BRR register */
    p_rspi_reg->SPCMDn_b[0].BRDV = 0;
}  /* End of function HW_RSPI_DefaultBRDVSet(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the calculated Bit Rate Divisor in the SPI command register 0
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_BRDVSet (R_RSPI0_Type * p_rspi_reg, const uint16_t brdv)
{
    /* Sets the default Bit Rate Divisor for the BRR register */
    p_rspi_reg->SPCMDn[0] = (uint16_t)((p_rspi_reg->SPCMDn[0] & 0xfff3) | ((brdv & 0x03)<<2));
}  /* End of function HW_RSPI_BRDVSet(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the default sequence length for transmission the Sequence Control register SPSCR
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_DefaultSequence (R_RSPI0_Type * p_rspi_reg)
{
    /* Sets the default sequence for the data frames */
    p_rspi_reg->SPSCR_b.SPSLN = 0;
}  /* End of function HW_RSPI_DefaultSequence(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the operating mode in master or slave in SPCR register
 * @param[in] p_rspi_reg RSPI Registers
 * @param[in] master     true-- master; false--slave
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_OperatingModeMaster (R_RSPI0_Type * p_rspi_reg, bool const master)
{
    /* Sets the operating mode to master or slave */
    p_rspi_reg->SPCR_b.MSTR = master;
}  /* End of function HW_RSPI_OperatingModeMaster(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the clock phase in the SPI command register (SPCMD0)
 * @param[in] p_rspi_reg RSPI Registers
 * @param[in] even       true--phase shift(even); false--no phase shift(odd)
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_ClockPhaseEven (R_RSPI0_Type * p_rspi_reg, bool const even)
{
    /* Sets the clock phase */
    p_rspi_reg->SPCMDn_b[0].CPHA = even;
}  /* End of function HW_RSPI_ClockPhaseEven(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the clock polarity in the SPI command register (SPCMD0)
 * @param[in] p_rspi_reg RSPI Registers
 * @param[in] high       true--clock high at idle; false--clock low at idle
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_ClockPolarityHigh (R_RSPI0_Type * p_rspi_reg, bool const high)
{
    /* Sets the clock polarity */
    p_rspi_reg->SPCMDn_b[0].CPOL = high;
}  /* End of function HW_RSPI_ClockPolarityHigh(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Enables/disables the mode fault detection in the SPCR register
 * @param[in] p_rspi_reg RSPI Registers
 * @param[in] on         true--detection 0n; false--detection off
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_ModeFaultDetectionOn (R_RSPI0_Type * p_rspi_reg, bool const on)
{
    /* Enable/disable the mode fault detection */
    p_rspi_reg->SPCR_b.MODFEN = on;
}  /* End of function HW_RSPI_ModeFaultDetectionOn(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the bit order to MSB or LSB in the SPI command register
 * @param[in] p_rspi_reg RSPI Registers
 * @param[in] lsb        true--LSB first; false--MSB first
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_BitOrderLSB (R_RSPI0_Type * p_rspi_reg, bool const lsb)
{
    /* Sets the bit order to LSB or MSB */
    p_rspi_reg->SPCMDn_b[0].LSBF = lsb;
}  /* End of function HW_RSPI_BitOrderLSB(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the communication to SPI mode or clock synchronous mode in the SPCR register.
 * @param[in] p_rspi_reg RSPI Registers
 * @param[in] clk_syn    true--clock synchronous mode; false-- SPI mode
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE bool HW_RSPI_OperationClkSyn (R_RSPI0_Type * p_rspi_reg, bool const clk_syn)
{
    /* Sets the communication to SPI or Clock Synchronous mode */
    return (p_rspi_reg->SPCR_b.SPMS = clk_syn);
} /* End of function HW_RSPI_OperationClkSyn(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the communication mode to full duplex or transmit only in the SPCR register.
 * @param[in] p_rspi_reg     RSPI Registers
 * @param[in] transmit_only  The mode the spi module should operate in
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_CommunicationTransmitOnly (R_RSPI0_Type * p_rspi_reg, rspi_communication_t const transmit_only)
{
    /* Sets the communication to transmit only or full duplex mode */
    p_rspi_reg->SPCR_b.TXMD = transmit_only;
}  /* End of function HW_RSPI_CommunicationTransmitOnly(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the slave select polarity in the SSLP register.
 * @param[in] p_rspi_reg RSPI Registers
 * @param[in] ssl        SSL0P, SSL1P, SSL2P, SSL3P
 * @param[in] high       true--high; false--low
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_SlaveSelectPolarity (R_RSPI0_Type * p_rspi_reg, rspi_ssl_select_t const ssl, bool const high)
{
    /* Sets the slave select polarity to high or low. True--high; false--low */
    switch(ssl)
    {
        case RSPI_SSL_SELECT_SSL0:
            p_rspi_reg->SSLP_b.SSL0P = high;
            break;
        case RSPI_SSL_SELECT_SSL1:
            p_rspi_reg->SSLP_b.SSL1P = high;
            break;
        case RSPI_SSL_SELECT_SSL2:
            p_rspi_reg->SSLP_b.SSL2P = high;
            break;
        case RSPI_SSL_SELECT_SSL3:
            p_rspi_reg->SSLP_b.SSL3P = high;
            break;
        default:
            break;
    }
}  /* End of function HW_RSPI_SlaveSelectPolarity(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets loopback1 mode in pin control (SPPCR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @param[in] loopback1   true--loopback1 mode with data inverted; false--normal mode
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_loopback1 (R_RSPI0_Type * p_rspi_reg, bool const loopback1)
{
    /* Sets the data pin to loopback1 or normal mode. True--loopback1 mode; false--normal mode */
    p_rspi_reg->SPPCR_b.SPLP = loopback1;
}  /* End of function HW_RSPI_loopback1(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets loopback2 mode in pin control (SPPCR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @param[in] loopback2   true--loopback2 mode with data not inverted; false--normal mode
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_loopback2 (R_RSPI0_Type * p_rspi_reg, bool const loopback2)
{
    /* Sets the data pin to loopback2 or normal mode. True--loopback2 mode; false--normal mode */
    p_rspi_reg->SPPCR_b.SPLP2 = loopback2;
}  /* End of function HW_RSPI_loopback2(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the MOSI idle fixed value (MOIFV) in the pin control (SPPCR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @param[in] high   true--high; false--low
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_MOSIIdleLevelHigh (R_RSPI0_Type * p_rspi_reg, bool const high)
{
    /* Sets the MOSI idle fixed value. True--high; false--low */
    p_rspi_reg->SPPCR_b.MOIFV = high;
}  /* End of function HW_RSPI_MOSIIdleLevelHigh(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Enables/disables the MOSI idle fixed value (MOIFE) in the pin control (SPPCR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @param[in] enable     true--enable; false--disable
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_MOSIIdleEnable (R_RSPI0_Type * p_rspi_reg, bool const enable)
{
    /* Enables/disables the MOSI idle fixed value. True--enable; false--disable */
    p_rspi_reg->SPPCR_b.MOIFE = enable;
}  /* End of function HW_RSPI_MOSIIdleEnable(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Enables/disables the parity in the control register 2 (SPCR2).
 * @param[in] p_rspi_reg RSPI Registers
 * @param[in] enable     true--enable parity; false--disable parity
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_ParityEnable (R_RSPI0_Type * p_rspi_reg, bool const enable)
{
    /* Enables/disables the parity. True--enables parity; false--disable parity */
    p_rspi_reg->SPCR2_b.SPPE = enable;
}  /* End of function HW_RSPI_ParityEnable(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the parity type in the control register 2 (SPCR2).
 * @param[in] p_rspi_reg RSPI Registers
 * @param[in] odd        true--odd parity; false--even parity
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_ParityOdd (R_RSPI0_Type * p_rspi_reg, bool const odd)
{
    /* Sets the parity type to odd or even. True--odd parity; false--even parity */
    p_rspi_reg->SPCR2_b.SPOE = odd;
}  /* End of function HW_RSPI_ParityOdd(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the RSPCK auto stop function.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_RSPCKAutoStop(R_RSPI0_Type * p_rspi_reg)
{
    /* Enable RSPCK auto-stop function. */
    p_rspi_reg->SPCR2_b.SCKASE = 1U;
}  /* End of function HW_RSPI_SPCKAutoStop(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets slave selection in the SPI command register(SPCMD0).
 * @param[in] p_rspi_reg RSPI Registers
 * @param[in] ssl        SSL0, SSL1, SSL2, SSL3
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_SlaveSelect (R_RSPI0_Type * p_rspi_reg, rspi_ssl_select_t const ssl)
{
    /* Selects the slave. */
    p_rspi_reg->SPCMDn_b[0].SSLA = ssl;
}  /* End of function HW_RSPI_SlaveSelect(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the slave select signal level in the SPI command register (SPCMD0).
 * @param[in] p_rspi_reg RSPI Registers
 * @param[in] keep       true--keep the signal level; false--negate the signal level
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_SlaveSelectLevelKeep (R_RSPI0_Type * p_rspi_reg, bool const keep)
{
    /* Select to keep or negate the slave select signal at the end of transfer. True--keep; false--negate */
    p_rspi_reg->SPCMDn_b[0].SSLKP = keep;
}  /* End of function HW_RSPI_SlaveSelectLevelKeep(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Sets the data bit length for the SPI communication in the SPI command register (SPCMD0).
 * @param[in] channel   RSPI channel number
 * @param[in] length    8, 16, 32 bits
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_DataBitLength (R_RSPI0_Type * p_rspi_reg, uint32_t const length)
{
    /* Sets the data bit length */
    p_rspi_reg->SPCMDn_b[0].SPB = (uint8_t) (length & 0x0F);
}  /* End of function HW_RSPI_DataBitLength(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Enables/disables swapping using BYSW bit fields in SPDCR2 register.(Applicable only for S5 series MCU's)
 * @param[in] RSPI low level register base address pointer
 * @param[in] true--enable byte swap; false--disable byte swap
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_Byte_Swap_Enable (R_RSPI0_Type * p_rspi_reg, bool const enable)
{
    /* Sets the data register swapping for 16/32 bits. */
     p_rspi_reg->SPDCR2_b.BYSW = enable;

}   /*End of function HW_RSPI_Byte_Swap_Enable (R_RSPI0_Type * p_rspi_reg, bool const enable) */

/*****************************************************************************************************************//**
 * @brief     Enables/disables the clock delay and sets the delay value in SPCMD0 and the clock delay register (SPCKD).
 * @param[in] channel   RSPI channel number
 * @param[in] enable    true--enable; false--disable
 * @param[in] count     number of clock cycles
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_ClockDelay (R_RSPI0_Type * p_rspi_reg, bool const enable, rspi_clock_delay_count_t const count)
{
    /* Enables/disables clock delay and sets the clock delay value */
    p_rspi_reg->SPCMDn_b[0].SCKDEN = enable;
    p_rspi_reg->SPCKD = count;
}  /* End of function HW_RSPI_ClockDelay(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Enables/disables the slave select negation delay and sets the value in SPCMD0 and the SSLND register.
 * @param[in] channel   RSPI channel number
 * @param[in] enable    true--enable; false--disable
 * @param[in] count     number of clock cycles
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_SlaveSelectNegationDelay (R_RSPI0_Type * p_rspi_reg, bool const enable,
                                                   rspi_ssl_negation_delay_count_t const count)
{
    /* Enables/disables the slave select negation delay and sets the delay value */
    p_rspi_reg->SPCMDn_b[0].SLNDEN = enable;
    p_rspi_reg->SSLND = count;
}  /* End of function HW_RSPI_SlaveSelectNegationDelay(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Enables/disables the next access delay and sets the value in SPCMD0 and the SPND register.
 * @param[in] channel   RSPI channel number
 * @param[in] enable    true--enable; false--disable
 * @param[in] count     number of clock cycles
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_NextAccessDelay (R_RSPI0_Type * p_rspi_reg, bool const enable,
                                          rspi_next_access_delay_count_t const count)
{
    /* Enables/disables next access delay and sets the delay value */
    p_rspi_reg->SPCMDn_b[0].SPNDEN = enable;
    p_rspi_reg->SPND = count;
}  /* End of function HW_RSPI_NextAccessDelay(R_RSPI0_Type * p_rspi_reg) */

/*******************************************************************************************************************//**
* @brief     Checks if Master mode is set or not
* @param[in] channel RSPI channel number
* @retval    true  : master mode
* @retval    false : other mode
* @note      the parameter check must be held by HLD
***********************************************************************************************************************/
__STATIC_INLINE bool HW_RSPI_MasterModeCheck (R_RSPI0_Type * p_rspi_reg)
{
    return (1 == p_rspi_reg->SPCR_b.MSTR);
}  /* End of function HW_RSPI_MasterModeCheck(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Enables the idle interrupt in the control register 2 (SPCR2).
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_IdleInterruptEnable (R_RSPI0_Type * p_rspi_reg)
{
    /* Disable the Idle interrupt. */
    p_rspi_reg->SPCR2_b.SPIIE = 1;
}  /* End of function HW_RSPI_IdleInterruptEnable(R_RSPI0_Type * p_rspi_reg) */


/*****************************************************************************************************************//**
 * @brief     Disables the idle interrupt in the control register 2 (SPCR2).
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_IdleInterruptDisable (R_RSPI0_Type * p_rspi_reg)
{
    /* Disable the Idle interrupt. */
    p_rspi_reg->SPCR2_b.SPIIE = 0;
}  /* End of function HW_RSPI_IdleInterruptDisable(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Check the idle flag status in status register (SPSR).
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    true       Not Idle
 * @retval    false      Idle
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE bool HW_RSPI_IdleCheck (R_RSPI0_Type * p_rspi_reg)
{
    /* returns the idle flag status */
    return (p_rspi_reg->SPSR_b.IDLNF);
} /* End of function HW_RSPI_IdleCheck(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Gets the status of the status register (SPSR).
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    uint8_t    Status register
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE uint8_t HW_RSPI_Status (R_RSPI0_Type * p_rspi_reg)
{
    /* returns the status of the status register */
    return (p_rspi_reg->SPSR);
} /* End of function HW_RSPI_Status(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Clears all the error source in the status (SPSR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    bool       Error condition exists
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE bool HW_RSPI_ErrorsClear (R_RSPI0_Type * p_rspi_reg)
{
    /* Clears all the error condition */
    return (p_rspi_reg->SPSR & (RSPI_SPSR_OVRF | RSPI_SPSR_MODF | RSPI_SPSR_PERF));
}  /* End of function HW_RSPI_ErrorsClear */

/*****************************************************************************************************************//**
 * @brief    Sets receive buffer full and under-run flag in status (SPSR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_RxBufferFull_Underrun_Set (R_RSPI0_Type * p_rspi_reg)
{
    /* Sets the Rx buffer full and under-run flags */
    p_rspi_reg->SPSR = RSPI_SPSR_MASK;
}  /* End of function HW_RSPI_RxBufferFull_Underrun_Set */

/*****************************************************************************************************************//**
 * @brief     Enable TX buffer empty/Error Interrupt and SPI function in the control (SPCR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_InterruptEnableTx (R_RSPI0_Type * p_rspi_reg)
{
    /* Enable TX buffer empty/Error Interrupt and SPI function bits */
    p_rspi_reg->SPCR |= (RSPI_SPCR_SPTIE | RSPI_SPCR_SPEIE | RSPI_SPCR_SPE);
}  /* End of function HW__RSPI_InterruptEnableTx */

/*****************************************************************************************************************//**
 * @brief     Enable RX buffer full/TX buffer empty/Error Interrupt and SPI function in the control (SPCR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_InterruptEnable (R_RSPI0_Type * p_rspi_reg)
{
    /* Enable RX buffer full/TX buffer empty/Error Interrupt and SPI function bits */
    p_rspi_reg->SPCR |= (RSPI_SPCR_SPTIE | RSPI_SPCR_SPRIE | RSPI_SPCR_SPEIE | RSPI_SPCR_SPE);
}  /* End of function HW__RSPI_InterruptEnable */

/*****************************************************************************************************************//**
 * @brief     Disable RX buffer full/TX buffer empty/Error Interrupt and SPI function in the control (SPCR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_InterruptDisable (R_RSPI0_Type * p_rspi_reg)
{
    /* Disable RX buffer full/TX buffer empty/Error Interrupt and SPI function bits */
    p_rspi_reg->SPCR &= (uint8_t) (~((RSPI_SPCR_SPTIE | RSPI_SPCR_SPRIE | RSPI_SPCR_SPEIE | RSPI_SPCR_SPE)));
}  /* End of function HW__RSPI_InterruptDisable */

/*****************************************************************************************************************//**
 * @brief     Disable TX buffer empty interrupt and the SPI function in the control (SPCR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_TxIntr_SPI_Disable (R_RSPI0_Type * p_rspi_reg)
{
    /* Disable the TX buffer empty interrupt and the SPI function */
    p_rspi_reg->SPCR &= (uint8_t) (~(RSPI_SPCR_SPTIE | RSPI_SPCR_SPE));
}  /* End of function HW_RSPI_TxIntr_SPI_Disable */

/*****************************************************************************************************************//**
 * @brief     Disable RX buffer full interrupt in the control (SPCR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_RxIntrDisable (R_RSPI0_Type * p_rspi_reg)
{
    /* Disable the RX buffer full interrupt */
    p_rspi_reg->SPCR_b.SPRIE = 0;
}  /* End of function HW_RSPI_RxIntrDisable(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Enable TX buffer empty interrupt in the control (SPCR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_TxIntrEnable (R_RSPI0_Type * p_rspi_reg)
{
    /* Disable the TX buffer empty interrupt */
    p_rspi_reg->SPCR_b.SPTIE = 1;
}  /* End of function HW_RSPI_TxIntrEnable(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Disable TX buffer empty interrupt in the control (SPCR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_TxIntrDisable (R_RSPI0_Type * p_rspi_reg)
{
    /* Disable the TX buffer empty interrupt */
    p_rspi_reg->SPCR_b.SPTIE = 0;
}  /* End of function HW_RSPI_TxIntrDisable(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Enable RSPI function in the control (SPCR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_RSPIEnable (R_RSPI0_Type * p_rspi_reg)
{
    /* Disable the RSPI function */
    p_rspi_reg->SPCR_b.SPE = 1;
}  /* End of function HW_RSPI_RSPIEnable(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Disable RSPI function in the control (SPCR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_RSPIDisable (R_RSPI0_Type * p_rspi_reg)
{
    /* Disable the RSPI function */
    p_rspi_reg->SPCR_b.SPE = 0;
}  /* End of function HW_RSPI_RSPIDisable(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Clears the RX buffer full error in the status (SPSR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_RxBufferFullClear (R_RSPI0_Type * p_rspi_reg)
{
    /* Clears the RX buffer full error */
    p_rspi_reg->SPSR = (uint8_t)(RSPI_SPSR_MASK_CLEAR & (uint8_t)(~RSPI_SPSR_SPRF));
}  /* End of function HW_RSPI_RxBufferFullClear(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Clears overrun error in the status (SPSR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_OverRunErrorClear (R_RSPI0_Type * p_rspi_reg)
{
    /* Clears the overrun error */
    p_rspi_reg->SPSR = (uint8_t)(RSPI_SPSR_MASK_CLEAR & (uint8_t)(~RSPI_SPSR_OVRF));
}  /* End of function HW_RSPI_OverRunErrorClear(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Clears mode fault error in the status (SPSR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_ModeFaultErrorClear (R_RSPI0_Type * p_rspi_reg)
{
    /* Clears the mode fault error */
    p_rspi_reg->SPSR = (uint8_t)(RSPI_SPSR_MASK_CLEAR & (uint8_t)(~RSPI_SPSR_MODF));
}  /* End of function HW_RSPI_ModeFaultErrorrClear(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Clears underflow error in the status (SPSR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_UnderflowErrorClear (R_RSPI0_Type * p_rspi_reg)
{
    /* Clears the underflow error */
    p_rspi_reg->SPSR = (uint8_t)(RSPI_SPSR_MASK_CLEAR & (uint8_t)(~RSPI_SPSR_UDRF));
}  /* End of function HW_RSPI_UnderflowErrorClear(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Transmit buffer empty in the status (SPSR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    uint8_t    Status of the transmit buffer empty flag
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE uint8_t HW_RSPI_TransmitBufferEmpty (R_RSPI0_Type * p_rspi_reg)
{
    return p_rspi_reg->SPSR_b.SPTEF;
}  /* End of function HW_RSPI_TransmitBufferEmpty(R_RSPI0_Type * p_rspi_reg) */

/*****************************************************************************************************************//**
 * @brief     Clears parity error in the status (SPSR) register.
 * @param[in] p_rspi_reg RSPI Registers
 * @retval    void
 * @note      the parameter check must be held by HLD
 *********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_ParityErrorrClear (R_RSPI0_Type * p_rspi_reg)
{
    /* Clears the parity error */
    p_rspi_reg->SPSR = (uint8_t)(RSPI_SPSR_MASK_CLEAR & (uint8_t)(~RSPI_SPSR_PERF));
}  /* End of function HW_RSPI_ParityErrorrClear(R_RSPI0_Type * p_rspi_reg) */

/*******************************************************************************************************************//**
* @brief  Writes long word data (32 bits) to transmit data register.
* @param[in] channel  RSPI channel number
* @param[in] data     Data to be sent
* @retval    void
* @note      All the parameter check must be held by HLD
***********************************************************************************************************************/
__STATIC_INLINE void HW_RSPI_Write (R_RSPI0_Type * p_rspi_reg, uint32_t const data)
{
    /* Write data to data register */
    p_rspi_reg->SPDR = data;
}  /* End of function HW_RSPI_Write(R_RSPI0_Type * p_rspi_reg) */

/*******************************************************************************************************************//**
* @brief  Reads data from receive data register
* @param[in] p_rspi_reg RSPI Registers
* @note      Channel number is not checked in this function, caller function must check it.
***********************************************************************************************************************/
__STATIC_INLINE uint32_t HW_RSPI_Read (R_RSPI0_Type * p_rspi_reg)
{
    return p_rspi_reg->SPDR;
}  /* End of function HW_RSPI_Read(R_RSPI0_Type * p_rspi_reg) */

/*******************************************************************************************************************//**
* @brief     This function returns the address of the RSPI write and read register
* @param[in] channel     RSPI channel
* @retval    uint32_t    Address of the RSPI write and read register
* @note      Channel number is not checked in this function, caller function must check it.
***********************************************************************************************************************/
__STATIC_INLINE uint32_t volatile const * HW_RSPI_WriteReadAddrGet (R_RSPI0_Type * p_rspi_reg)
{
    return (uint32_t volatile const *) (&(p_rspi_reg->SPDR));
}  /* End of function HW_SCI_SPIWriteAddrGet() */

/*******************************************************************************************************************//**
* Event lookup for RXI.
* @param[in] channel  RSPI channel
* @retval    ELC event
* @note      Parameter check is not held in this function
***********************************************************************************************************************/
__STATIC_INLINE elc_event_t rspi_rxi_event_lookup (uint32_t const channel)
{
    return rxi_event_list[channel];
}

/*******************************************************************************************************************//**
* Event lookup for TXI.
* @param[in] channel  RSPI channel
* @retval    ELC event
* @note      Parameter check is not held in this function
***********************************************************************************************************************/
__STATIC_INLINE elc_event_t rspi_txi_event_lookup (uint32_t const channel)
{
    return txi_event_list[channel];
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_RSPI_COMMON_H */
