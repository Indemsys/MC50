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
* File Name    : hw_riic_slave_common.h
* Description  : LLD implementation of the RIIC hardware
***********************************************************************************************************************/


#ifndef HW_RIIC_SLAVE_COMMON_H
#define HW_RIIC_SLAVE_COMMON_H

/**********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_riic_slave.h"
#include "bsp_api.h"
#include "r_riic_slave_cfg.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/** The timeout interrupt enable bit */
#define RIIC_TMO_EN_BIT     (0x01U)
/** The arbitration loss detection interrupt enable bit */
#define RIIC_ALD_EN_BIT     (0x02U)
/** The start condition detection interrupt enable bit */
#define RIIC_STR_EN_BIT     (0x04U)
/** The stop condition detection interrupt enable bit */
#define RIIC_STP_EN_BIT     (0x08U)
/** The NAK reception interrupt enable bit */
#define RIIC_NAK_EN_BIT     (0x10U)
/** The receive data full interrupt enable bit */
#define RIIC_RXI_EN_BIT     (0x20U)
/** The transmit end interrupt enable bit */
#define RIIC_TEI_EN_BIT     (0x40U)
/** The transmit data empty interrupt enable bit */
#define RIIC_TXI_EN_BIT     (0x80U)

/** Bit position for Timeout function (TMOF) detection flag in ICSR2 */
#define ICSR2_TMOF_BIT      (0x01U)

/** Bit position for Arbitration loss (AL) detection flag in ICSR2 */
#define ICSR2_AL_BIT        (0x02U)

/** Bit position for the START condition detection flag in ICSR2 */
#define ICSR2_START_BIT     (0x04U)

/** Bit position for STOP condition flag in ICSR2 */
#define ICSR2_STOP_BIT      (0x08U)

/** Bit position for No Acknowledgment (NACKF) flag in ICSR2 */
#define ICSR2_NACKF_BIT     (0x10U)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

typedef enum e_timeout_option
{
    RIIC_TIMEOUT_OFF,
    RIIC_TIMEOUT_ON_SCL_HIGH,
    RIIC_TIMEOUT_ON_SCL_LOW,
    RIIC_TIMEOUT_ON_BOTH
}riic_timeout_t;

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
 * Enable or disable output of SDA/SCL.
 * @param   p_riic_reg      Base address of the hardware registers
 * @param   enable  True when enabling SDA/SCL pins, otherwise disable SDA/SCL pin output
 **********************************************************************************************************************/
__STATIC_INLINE void    HW_RIIC_Output          (R_IIC0_Type * p_riic_reg, bool     const   enable)
{
    p_riic_reg->ICCR1_b.ICE = enable;

    /* Give time for value to be updated */
    volatile uint8_t dummy;
    dummy  = p_riic_reg->ICCR1_b.ICE;
    dummy  = p_riic_reg->ICCR1_b.ICE;
    dummy  = p_riic_reg->ICCR1_b.ICE;
    SSP_PARAMETER_NOT_USED(dummy);
}

/*******************************************************************************************************************//**
 * Set whether the peripheral is in the reset state.
 * @param   p_riic_reg      Base address of the hardware registers
 * @param   enable  True when enabling SDA/SCL pins, otherwise disable SDA/SCL pin output
 **********************************************************************************************************************/
__STATIC_INLINE void    HW_RIIC_Reset           (R_IIC0_Type * p_riic_reg, bool     const   enable)
{
    p_riic_reg->ICCR1_b.IICRST = enable;

    /* Give time for value to be updated */
    volatile uint8_t dummy;
    dummy  = p_riic_reg->ICCR1_b.IICRST;
    dummy  = p_riic_reg->ICCR1_b.IICRST;
    dummy  = p_riic_reg->ICCR1_b.IICRST;
    SSP_PARAMETER_NOT_USED(dummy);
}

/*******************************************************************************************************************//**
 * Enable/disable slope control circuit
 * @param   p_riic_reg      Base address of the hardware registers
 * @param   timeout         timeout type
 * @param   short_mode      Counting mode is short
 **********************************************************************************************************************/
__STATIC_INLINE void    HW_RIIC_TimeoutCfg (R_IIC0_Type * p_riic_reg, riic_timeout_t timeout, bool short_mode)
{
   if(RIIC_TIMEOUT_OFF != timeout)
   {
       if(RIIC_TIMEOUT_ON_SCL_HIGH == timeout)
       {
           p_riic_reg->ICMR2_b.TMOH = 1;
           p_riic_reg->ICMR2_b.TMOL = 0;
       }
       else if(RIIC_TIMEOUT_ON_SCL_LOW == timeout)
       {
           p_riic_reg->ICMR2_b.TMOL = 1;
           p_riic_reg->ICMR2_b.TMOH = 0;
       }
       else
       {
           p_riic_reg->ICMR2_b.TMOH = 1;
           p_riic_reg->ICMR2_b.TMOL = 1;
       }

       p_riic_reg->ICMR2_b.TMOS = short_mode;

       /* Enable timeout function */
       p_riic_reg->ICFER_b.TMOE = 1;
   }
   else
   {
       /* Disable timeout function */
       p_riic_reg->ICFER_b.TMOE = 0;
   }
}


/*******************************************************************************************************************//**
 * Read one byte from the specified RIIC block.
 * @param   p_riic_reg      Base address of the hardware registers
 **********************************************************************************************************************/
__STATIC_INLINE uint8_t HW_RIIC_ReadByte        (R_IIC0_Type * p_riic_reg)
{
    return p_riic_reg->ICDRR;
}

/*******************************************************************************************************************//**
 * Write one byte to the specified RIIC block.
 * @param   p_riic_reg      Base address of the hardware registers
 * @param   byte            Data need to be written in data register
 **********************************************************************************************************************/
__STATIC_INLINE void    HW_RIIC_WriteByte       (R_IIC0_Type * p_riic_reg, uint8_t  const   byte)
{
    p_riic_reg->ICDRT = byte;
}

/*******************************************************************************************************************//**
 * Sets up slave address 'n' Address and format on the specified channel.
 * @param   p_riic_reg      Base address of the hardware registers
 * @param   sar_y       Index for Slave Address Register 'n' Enable
 * @param   addr_mode   Slave address mode (7/10 bit)
 * @param   addr        Slave address 'n'
 **********************************************************************************************************************/
__STATIC_INLINE void    HW_RIIC_SetupSlaveAddress ( R_IIC0_Type * p_riic_reg,
                                                    uint8_t    const sar_y,
                                                    i2c_addr_mode_t    const addr_mode,
                                                    uint16_t   const addr)
{
    /*7 bit mode selected, clear SARU and set SARL */
    if (I2C_ADDR_MODE_7BIT== addr_mode)
    {
    	p_riic_reg->SARLnRC0[sar_y].SARUn = 0;
    	p_riic_reg->SARLnRC0[sar_y].SARLn = (uint8_t)(addr << 1);
    }
    /* 10 bit mode selected, set SARU and SARL */
    else
    {
        p_riic_reg->SARLnRC0[sar_y].SARUn = (((addr >> 7) | 0x01) & 0x07);
        p_riic_reg->SARLnRC0[sar_y].SARLn = (uint8_t)addr;
    }

    //* Enable the slave address */
    p_riic_reg->ICSER = (uint8_t)( 1 << sar_y);
}

/*******************************************************************************************************************//**
 * Clear the slave address
 * @param   p_riic_reg      Base address of the hardware registers
 * @param   sar_y       Index for Slave Address Register 'n' 
 **********************************************************************************************************************/
__STATIC_INLINE void    HW_RIIC_ClearSlaveAddress ( R_IIC0_Type * p_riic_reg,
                                                    uint8_t    const sar_y)
{
    p_riic_reg->SARLnRC0[sar_y].SARUn = 0x00U;
    p_riic_reg->SARLnRC0[sar_y].SARLn = 0x00U;
}

/*******************************************************************************************************************//**
 * Returns ICDRR buffer used for dummy read.
 * @param   p_riic_reg      Base address of the hardware registers
 **********************************************************************************************************************/
__STATIC_INLINE uint8_t   HW_RIIC_DummyRead       (R_IIC0_Type * p_riic_reg)
{
    return p_riic_reg->ICDRR;
}

/*******************************************************************************************************************//**
 * Sets the acknowledge bit when getting ready to signal the slave to stop transmitting.
 * @param   p_riic_reg      Base address of the hardware registers
 **********************************************************************************************************************/
__STATIC_INLINE void    HW_RIIC_SetNACKTransmission   (R_IIC0_Type * p_riic_reg)
{
    p_riic_reg->ICMR3_b.ACKWP = 1;

    /* Give time for value to be updated */
    volatile uint8_t dummy;
    dummy  = p_riic_reg->ICMR3_b.ACKWP;
    dummy  = p_riic_reg->ICMR3_b.ACKWP;
    dummy  = p_riic_reg->ICMR3_b.ACKWP;
    SSP_PARAMETER_NOT_USED(dummy);

    p_riic_reg->ICMR3_b.ACKBT = 1;
    p_riic_reg->ICMR3_b.ACKWP = 0;
}

/*******************************************************************************************************************//**
 * Sets the acknowledge bit when getting ready to signal the slave to stop transmitting.
 * @param   p_riic_reg      Base address of the hardware registers
 **********************************************************************************************************************/
__STATIC_INLINE void    HW_RIIC_SetACKTransmission    (R_IIC0_Type * p_riic_reg)
{
    p_riic_reg->ICMR3_b.ACKWP = 1;

    /* Give time for value to be updated */
    volatile uint8_t dummy;
    dummy  = p_riic_reg->ICMR3_b.ACKWP;
    dummy  = p_riic_reg->ICMR3_b.ACKWP;
    dummy  = p_riic_reg->ICMR3_b.ACKWP;
    SSP_PARAMETER_NOT_USED(dummy);

    p_riic_reg->ICMR3_b.ACKBT = 0;
    p_riic_reg->ICMR3_b.ACKWP = 0;
}

/*******************************************************************************************************************//**
 *Set the BRCL value
 * @param   p_riic_reg      Base address of the hardware registers
 * @param   brcl_val        bit rate counter low value to be set
 **********************************************************************************************************************/
__STATIC_INLINE void  HW_RIIC_SetBRCL      (R_IIC0_Type * p_riic_reg, uint8_t brcl_val)
{
    /* Set the number of counts that the clock remains low */
    p_riic_reg->ICBRL_b.BRL = (uint8_t)(0x1F & brcl_val);
}

/*******************************************************************************************************************//**
 * Set internal reference clock
 * @param   p_riic_reg      Base address of the hardware registers
 * @param   clock_source    Internal reference clock source
 **********************************************************************************************************************/
__STATIC_INLINE void   HW_RIIC_SetCKS      (R_IIC0_Type * p_riic_reg, uint8_t clock_source)
{
    /* Set the internal reference clock source for generating IIC clock */
    p_riic_reg->ICMR1_b.CKS = (uint8_t)(clock_source & 0x7U);
}

/*******************************************************************************************************************//**
 * Enable/disable Master arbitration loss detection
 * @param   p_riic_reg      Base address of the hardware registers
 * @param   enable          Enable or disable
 **********************************************************************************************************************/
__STATIC_INLINE void   HW_RIIC_EnableMALE (R_IIC0_Type * p_riic_reg, bool enable)
{
    p_riic_reg->ICFER_b.MALE = enable;
}

/*******************************************************************************************************************//**
 * Enable/disable NACK arbitration loss detection
 * @param   p_riic_reg      Base address of the hardware registers
 * @param   enable          Enable or disable
 **********************************************************************************************************************/
__STATIC_INLINE void   HW_RIIC_EnableNALE (R_IIC0_Type * p_riic_reg, bool enable)
{
    p_riic_reg->ICFER_b.NALE = enable;
}

/*******************************************************************************************************************//**
 * Enable/disable Slave arbitration loss detection
 * @param   p_riic_reg      Base address of the hardware registers
 * @param   enable          Enable or disable
 **********************************************************************************************************************/
__STATIC_INLINE void   HW_RIIC_EnableSALE (R_IIC0_Type * p_riic_reg, bool enable)
{
    p_riic_reg->ICFER_b.SALE = enable;
}

/*******************************************************************************************************************//**
 * Enable/disable NACK Receptions suspend
 * @param   p_riic_reg      Base address of the hardware registers
 * @param   enable          Enable or disable
 **********************************************************************************************************************/
__STATIC_INLINE void   HW_RIIC_EnableNACKE (R_IIC0_Type * p_riic_reg, bool enable)
{
    p_riic_reg->ICFER_b.NACKE = enable;
}

/*******************************************************************************************************************//**
 * Enable/disable Digital noise filter
 * @param   p_riic_reg          Base address of the hardware registers
 * @param   num_filter_stages   Number of filter stages
 **********************************************************************************************************************/
__STATIC_INLINE void   HW_RIIC_EnableNFE (R_IIC0_Type * p_riic_reg, uint8_t num_filter_stages)
{
   if(0 < num_filter_stages)
   {
       p_riic_reg->ICMR3_b.NF = (num_filter_stages & 0x03U);
       p_riic_reg->ICFER_b.NFE = 0x01U;
   }
}

/*******************************************************************************************************************//**
 * Get status flags
 * @param   p_riic_reg      Base address of the hardware registers
 **********************************************************************************************************************/
__STATIC_INLINE uint8_t   HW_RIIC_GetICSR2 (R_IIC0_Type * p_riic_reg)
{
    return p_riic_reg->ICSR2;
}

/*******************************************************************************************************************//**
 * Clear status flags
 * @param   p_riic_reg      Base address of the hardware registers
 **********************************************************************************************************************/
__STATIC_INLINE void   HW_RIIC_ClearICSR2 (R_IIC0_Type * p_riic_reg)
{
    p_riic_reg->ICSR2 = 0x00;
}

/*******************************************************************************************************************//**
 * Clear stop bit status flag
 * @param   p_riic_reg      Base address of the hardware registers
 **********************************************************************************************************************/
__STATIC_INLINE void   HW_RIIC_ClearStopBitFlag (R_IIC0_Type * p_riic_reg)
{
	 /* Clear the Stop condition flag */
	 p_riic_reg->ICSR2  &= (uint8_t)~(ICSR2_STOP_BIT);
}

/*******************************************************************************************************************//**
 * Enable interrupts
 * @param   p_riic_reg      Base address of the hardware registers
 * @param   bits_to_set     The bit combination to set
 **********************************************************************************************************************/
__STATIC_INLINE void   HW_RIIC_SetICIER (R_IIC0_Type * p_riic_reg, uint8_t bits_to_set)
{
    p_riic_reg->ICIER |=  bits_to_set;
}

/*******************************************************************************************************************//**
 * Disable interrupts
 * @param   p_riic_reg      Base address of the hardware registers
 * @param   bits_to_clear          Bit combination to clear
 **********************************************************************************************************************/
__STATIC_INLINE void   HW_RIIC_ClearICIER (R_IIC0_Type * p_riic_reg, uint8_t bits_to_clear)
{
    p_riic_reg->ICIER &= (uint8_t)~bits_to_clear;
}

/*******************************************************************************************************************//**
 * Clear START flag and enable START interrupt
 * @param   p_riic_reg      Base address of the hardware registers
 **********************************************************************************************************************/
__STATIC_INLINE void   HW_RIIC_EnableInterruptOnSTART(R_IIC0_Type * p_riic_reg)
{
    /* Clear the Start condition flag */
    p_riic_reg->ICSR2  &= (uint8_t)~(ICSR2_START_BIT);

    /* Give time for value to be updated */
    volatile uint8_t dummy;
    dummy  = p_riic_reg->ICSR2_b.START;
    dummy  = p_riic_reg->ICSR2_b.START;
    dummy  = p_riic_reg->ICSR2_b.START;
    SSP_PARAMETER_NOT_USED(dummy);
    /* Set the interrupt flag */
    p_riic_reg->ICIER  |= (uint8_t)(RIIC_STR_EN_BIT);
}

/*******************************************************************************************************************//**
 * Read slave address detection flag
 * @param   p_riic_reg      Base address of the hardware registers
 **********************************************************************************************************************/
__STATIC_INLINE bool   HW_RIIC_SlaveAddressMatchFlag(R_IIC0_Type * p_riic_reg)
{
    return p_riic_reg->ICSR1_b.AAS0;
}

/*******************************************************************************************************************//**
 * Enable fast mode plus slope circuit
 * @param   p_riic_reg      Base address of the hardware registers
 **********************************************************************************************************************/
__STATIC_INLINE void   HW_RIIC_FMPSlopeCircuit(R_IIC0_Type * p_riic_reg, bool enable)
{

    p_riic_reg->ICFER_b.FMPE = enable;
}

/*******************************************************************************************************************//**
 * Returns slave received ACK or NACK
 * @param   p_riic_reg      Base address of the hardware registers
 **********************************************************************************************************************/
__STATIC_INLINE bool    HW_RIIC_AckNackCheck (R_IIC0_Type * p_riic_reg)
{
    return (0 == p_riic_reg->ICMR3_b.ACKBR);
}

/*******************************************************************************************************************//**
 * Clears the transmit end interrupt.
 * @param   p_riic_reg      Base address of the hardware registers
 **********************************************************************************************************************/
__STATIC_INLINE void    HW_RIIC_ClearTxEnd      (R_IIC0_Type * p_riic_reg)
{
    p_riic_reg->ICSR2_b.TEND = 0;

    /* Give time for value to be updated */
    volatile uint8_t dummy;
    dummy  = p_riic_reg->ICSR2_b.TEND;
    dummy  = p_riic_reg->ICSR2_b.TEND;
    dummy  = p_riic_reg->ICSR2_b.TEND;
    SSP_PARAMETER_NOT_USED(dummy);
}

/*******************************************************************************************************************//**
 * Sets the RDRFS bit
 * @param   p_riic_reg      Base address of the hardware registers
 **********************************************************************************************************************/
__STATIC_INLINE void    HW_RIIC_RDRFSSet (R_IIC0_Type * p_riic_reg)
{
    p_riic_reg->ICMR3_b.RDRFS = 1U;
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_RIIC_COMMON_H */
