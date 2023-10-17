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
* File Name    : hw_sci_i2c_private.h
* Description  : SCI HAL driver for Simple I2C interface hardware definitions and declarations
***********************************************************************************************************************/

#ifndef HW_SCI_SIIC_PRIVATE_H
#define HW_SCI_SIIC_PRIVATE_H

/**********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "bsp_api.h"
#include "r_sci_i2c.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define    SCI_SIIC_SIMR3_STARTREQ    (0x51U)
#define    SCI_SIIC_SIMR3_RESTARTREQ  (0x52U)
#define    SCI_SIIC_SIMR3_STOPREQ     (0x54U)
#define    SCI_SIIC_SIMR3_CLEARREQ    (0x00U)
#define    SCI_SIIC_SIMR3_HALTPREQ    (0xF0U)

/* Number of cpu cycles representing a half clock period when using highest possible CPU CLK
 * and lowest possible I2C clock
 * CPU CLK = 240 MHz , I2C CLK = 100 KHz => 1 I2C CLK = 2400 CPU CLK => 0.5 I2C CLK = 1200 CPU CLK */
#define    SCI_IIC_WORST_CASE_TEND_DELAY     (1200U)

/**********************************************************************************************************************
Typedef definitions
**********************************************************************************************************************/
/** SCI SIIC Simple Mode select */
typedef enum e_sci_siic_mode
{
    SCI_SIIC_MODE_ASYNCHRONOUS = 0,  /**< SCI SIIC asynchronous mode, multiprocessor, clock synchronous mode */
    SCI_SIIC_MODE_I2C,               /**< SCI simple I2C mode */
    SCI_SIIC_MODE_SMART_CARD,        /**< SCI smart card mode */
    SCI_SIIC_MODE_PROHIBITED,        /**< SCI prohibited mode */
} sci_siic_mode_t;

/** SCI SIIC Output pin state for SDA/SCL */
typedef enum e_sci_siic_sda_scl_output
{
    SCI_SIIC_OUTPUT_NORMAL = 0,      /**< SCI SIIC SDA/SCL pins operate in serial data mode mode */
    SCI_SIIC_OUTPUT_COMMAND,         /**< SCI SIIC SDA/SCL pins operate in command (start, restart, stop) mode */
    SCI_SIIC_OUTPUT_LOW,             /**< SCI SIIC SDA/SCL pins set to low level output */
    SCI_SIIC_OUTPUT_OFF,             /**< SCI SIIC SDA/SCL pins set to high impedance mode  */
} sci_siic_sda_scl_output_t;

/** SCI SIIC Interrupt Source select */
typedef enum e_sci_siic_interrupt_source
{
    SCI_SIIC_INTERRUPT_SOURCE_ACK_NACK = 0,            /**< SCI SIIC uses ACK/NACK interrupts */
    SCI_SIIC_INTERRUPT_SOURCE_RECEPTION_TRANSMISSION,   /**< SCI SIIC uses reception and transmission interrupts */
} sci_siic_interrupt_source_t;

/*******************************************************************************************************************//**
* This function sets the I2C mode in I2C Mode Register 1 (SIMR1).
* @param[in] channel    SCI Channel number
* @param[in] sci_siic_mode    set mode as Asynchronous/Multiprocessor/Synchronous, Simple I2C, smart card
* @note      Parameter check is not held in this function.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_ModeSet (R_SCI0_Type * p_sci_reg, sci_siic_mode_t sci_siic_mode)
{
    p_sci_reg->SIMR1_b.IICM = sci_siic_mode;

}  /* End of function HW_SCI_SIICModeSet() */

/*******************************************************************************************************************//**
* This function initiates a start condition in the I2C Mode Register 3 (SIMR3).
* @param[in] channel    SCI Channel number
* @note      Channel number is not checked in this function, caller function must check it.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_SendStart (R_SCI0_Type * p_sci_reg)
{
		/* The IICRSTARREQ, IICSDAS, IICSCLS bits must be set simultaneously */
	p_sci_reg->SIMR3 = SCI_SIIC_SIMR3_STARTREQ;
}  /* End of function HW_SCI_SIIC_SendStart() */

/*******************************************************************************************************************//**
* This function initiates a restart condition in the I2C Mode Register 3 (SIMR3).
* @param[in] channel    SCI Channel number
* @note      Channel number is not checked in this function, caller function must check it.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_SendRestart(R_SCI0_Type * p_sci_reg)
{
	/* The IICRSTARREQ, IICSDAS, IICSCLS bits must be set simultaneously */
    p_sci_reg->SIMR3 = SCI_SIIC_SIMR3_RESTARTREQ;
}  /* End of function HW_SCI_SIIC_SendRestart() */

/*******************************************************************************************************************//**
* This function initiates a stop condition in the I2C Mode Register 3 (SIMR3).
* @param[in] channel    SCI Channel number
* @note      Channel number is not checked in this function, caller function must check it.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_SendStop(R_SCI0_Type * p_sci_reg)
{
	/* The IICRSTARREQ, IICSDAS, IICSCLS bits must be set simultaneously */
    p_sci_reg->SIMR3 = SCI_SIIC_SIMR3_STOPREQ;
}  /* End of function HW_SCI_SIIC_SendStop() */

/*******************************************************************************************************************//**
* Waits for Start/Restart/Stop condition to be completed and clears IICSTIF and sets SDA and SCL to data output.
* @param[in] channel    SCI Channel number
* @note      Channel number is not checked in this function, caller function must check it.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_Wait_to_ClearLine(R_SCI0_Type * p_sci_reg)
{
	/* Wait for Start/Restart/Stop condition to be completed */
	while (p_sci_reg->SIMR3_b.IICSTIF == 0U)
	{
		/* Do nothing. */
	}

	/* Clear I2C mode register command issuing flag and enable SDA/SCL for serial data output mode */
    p_sci_reg->SIMR3 = SCI_SIIC_SIMR3_CLEARREQ;

}  /* End of function HW_SCI_SIIC_Wait_to_ClearLine() */

/*******************************************************************************************************************//**
* Waits for Start/Restart/Stop condition to be completed and clears IICSTIF and sets SDA and SCL to data output.
* @param[in] channel    SCI Channel number
* @note      Channel number is not checked in this function, caller function must check it.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_Wait_to_ShutLine(R_SCI0_Type * p_sci_reg)
{
	/* Wait for Start/Restart/Stop condition to be completed */
	while (p_sci_reg->SIMR3_b.IICSTIF == 0U)
	{
		/* Do nothing. */
	}

	/* Clear I2C mode register command issuing flag and disable SDA/SCL output mode */
    p_sci_reg->SIMR3 = SCI_SIIC_SIMR3_HALTPREQ;

}  /* End of function HW_SCI_SIIC_Wait_to_ShutLine() */

/*******************************************************************************************************************//**
* This function clears the command issuing flag and sets SDA/SCL to serial output in the I2C Mode Register 3 (SIMR3).
* @param[in] channel    SCI Channel number
* @note      Channel number is not checked in this function, caller function must check it.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_LineClear(R_SCI0_Type * p_sci_reg)
{
    /* Clear I2C mode register command issuing flag and enable SDA/SCL for serial data output mode */
    p_sci_reg->SIMR3 = SCI_SIIC_SIMR3_CLEARREQ;
}

/*******************************************************************************************************************//**
* This function clears the STI interrupt flag
* @param[in] channel    SCI Channel number
* @note      Channel number is not checked in this function, caller function must check it.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_ClearSTIF(R_SCI0_Type * p_sci_reg)
{
    /* Clear the interrupt STI flag */
    p_sci_reg->SIMR3_b.IICSTIF = 0U;
    volatile uint8_t dummy;
    dummy = p_sci_reg->SIMR3_b.IICSTIF;
    dummy = p_sci_reg->SIMR3_b.IICSTIF;
    dummy = p_sci_reg->SIMR3_b.IICSTIF;
    SSP_PARAMETER_NOT_USED(dummy);
}

/*******************************************************************************************************************//**
* This function clears the command issuing flag and sets SDA/SCL to high-impedance state in the I2C Mode Register 3 (SIMR3).
* @param[in] channel    SCI Channel number
* @note      Channel number is not checked in this function, caller function must check it.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_LineHalt(R_SCI0_Type * p_sci_reg)
{
    /* Clear I2C mode register command issuing flag and sets SDA/SCL to high-impedance state */
    p_sci_reg->SIMR3 = SCI_SIIC_SIMR3_HALTPREQ;
}

/*******************************************************************************************************************//**
* This function sets the SDA/SCL pin output state in the I2C Mode Register 3 (SIMR3).
* @param   channel The hardware channel to access
* @param   state  state for the SDA/SCL pin output
* @note     Parameter check is not held in this function.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_Output (R_SCI0_Type * p_sci_reg, sci_siic_sda_scl_output_t const state)
{
   p_sci_reg->SIMR3_b.IICSDAS = state;    /**< set SDA output state */
   p_sci_reg->SIMR3_b.IICSCLS = state;    /**< set SCL output state */

   /* Give time for value to be updated */
   volatile uint8_t dummy;
   /** read the registers*/
   dummy = p_sci_reg->SIMR3_b.IICSDAS;
   dummy = p_sci_reg->SIMR3_b.IICSCLS;
   dummy = p_sci_reg->SIMR3_b.IICSDAS;
   dummy = p_sci_reg->SIMR3_b.IICSCLS;
   /** Do this to remove compile warnings */
   SSP_PARAMETER_NOT_USED(dummy);
}  /* End of function HW_SCI_SIIC_Output() */

/*******************************************************************************************************************//**
 * This function returns true when the SDA line is being held low.
 * @param   channel     The hardware channel to access
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_SCI_SIIC_SDALow(R_SCI0_Type * p_sci_reg)
{
    return (SCI_SIIC_OUTPUT_LOW ==  p_sci_reg->SIMR3_b.IICSDAS);  /**< return true if SDA pin stays at low level */
}  /* End of function HW_SCI_SIIC_SDALow() */

/*******************************************************************************************************************//**
 * Returns true when a requested stop condition has not yet been issued.
 * @param   channel     The hardware channel to access
 **********************************************************************************************************************/
__STATIC_INLINE bool HW_SCI_SIIC_StopPending(R_SCI0_Type * p_sci_reg)
{
    return (1U == p_sci_reg->SIMR3_b.IICSTPREQ);
}  /* End of function HW_SCI_SIIC_StopPending() */

/*******************************************************************************************************************//**
* This function sets the SDA output delay in the I2C Mode Register 1 (SIMR1).
* @param   channel The hardware channel to access
* @param   delay   add delay to the SDA output up to 31 cycles
* @note    Parameter check is not held in this function.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_DataOutputDelay (R_SCI0_Type * p_sci_reg, uint32_t const delay)
{
   p_sci_reg->SIMR1_b.IICDL = (uint8_t)(delay & 0x1F);    /**< set SDA output delay */
}  /* End of function HW_SCI_SIIC_DataOutputDelay() */

/*******************************************************************************************************************//**
* This function enables/disables the ACK/NACK transmission and reception (IICACKT bit) in the I2C Mode Register 2 (SIMR2).
* @param   channel The hardware channel to access
* @param   enable  true when ACK and NACK bits are received, false, otherwise.
* @note    Parameter check is not held in this function.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_ACKNACKEnable (R_SCI0_Type * p_sci_reg, bool const enable)
{
   p_sci_reg->SIMR2_b.IICACKT = (uint8_t) (enable & 1U);    /**< Set ACK/NACK transmission */
}  /* End of function HW_SCI_SIIC_ACKNACK_Enable() */

/*******************************************************************************************************************//**
* This function enables/disables the ACK/NACK transmission and reception (IICACKT bit) in the I2C Mode Register 2 (SIMR2).
* @param   channel The hardware channel to access
* @param   enable  true when ACK and NACK bits are received, false, otherwise.
* @note    Parameter check is not held in this function.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_FIFODisable(R_SCI0_Type * p_sci_reg)
{
   p_sci_reg->FCR_b.FM = 0U;    /**< Set ACK/NACK transmission */
}  /* End of function HW_SCI_SIIC_ACKNACK_Enable() */

/*******************************************************************************************************************//**
* This function enables/disables the clock synchronization (IICCSC bit) in the I2C Mode Register 2 (SIMR2).
* @param   channel The hardware channel to access
* @param   enable  true selects clock synchronization, false, otherwise.
* @note    Parameter check is not held in this function.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_ClockSynchronizationEnable (R_SCI0_Type * p_sci_reg, bool const enable)
{
   p_sci_reg->SIMR2_b.IICCSC = (uint8_t) (enable & 1U);    /**< enable/disable clock synchronization */
}  /* End of function HW_SCI_SIIC_ClockSynchronizationEnable() */

/*******************************************************************************************************************//**
* This function selects the source on interrupt requests (IICINTM bit) in simple I2C mode in the I2C Mode Register 2 (SIMR2).
* @param   channel The hardware channel to access
* @param   enable  true selects clock synchronization, false, otherwise.
* @note    Parameter check is not held in this function.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_InterruptSource (R_SCI0_Type * p_sci_reg, sci_siic_interrupt_source_t const source)
{
   p_sci_reg->SIMR2_b.IICINTM = source;    /**< set the source for interrupts */
}  /* End of function HW_SCI_SIIC_InterruptSource() */

/*******************************************************************************************************************//**
* This function checks wether the interrupt source is ACK/NACK or not.
* @param[in]  channel The hardware channel to access
* @param[out] boolean indicating the interrupt source is ACK/NACK or not
* @note    Parameter check is not held in this function.
***********************************************************************************************************************/
__STATIC_INLINE bool HW_SCI_SIIC_InterruptSourceIsACKNACK(R_SCI0_Type * p_reg)
{
	return (0U == p_reg->SIMR2_b.IICINTM);
}

/*******************************************************************************************************************//**
 * Sets the acknowledge bit when getting ready to signal the slave to stop transmitting.
 * @param   channel     The hardware channel to access
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_AckSet (R_SCI0_Type * p_sci_reg)
{
   p_sci_reg->SIMR2_b.IICACKT = 0U;    /**< set ACK transmission data */
}

/*******************************************************************************************************************//**
* Checks if ACK has been received or not
* @param[in] channel    SCI Channel number
* @note      Parameter check is not held in this function.
***********************************************************************************************************************/
__STATIC_INLINE bool HW_SCI_SIIC_AckReceived (R_SCI0_Type * p_sci_reg)
{
  	return (0U == p_sci_reg->SISR_b.IICACKR);
}  /* End of function HW_SCI_SIIC_AckReceived() */

/*******************************************************************************************************************//**
* Checks if NACK has been received or not
* @param[in] channel    SCI Channel number
* @note      Parameter check is not held in this function.
***********************************************************************************************************************/
__STATIC_INLINE bool HW_SCI_SIIC_NAckReceived (R_SCI0_Type * p_sci_reg)
{
  	return (1U == p_sci_reg->SISR_b.IICACKR);
}  /* End of function HW_SCI_SIIC_NAckReceived() */

/*******************************************************************************************************************//**
* Waits for the TX buffer to be ready to transfer
* * @param[in] channel    SCI Channel number
* @note      Parameter check is not held in this function.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_Wait_TXReady (R_SCI0_Type * p_sci_reg)
{
  	while (!p_sci_reg->SSR_b.TDRE)
	{
		/* Do nothing. */
	}
}  /* End of function HW_SCI_SIIC__Wait_TXReady() */

/*******************************************************************************************************************//**
* Waits for the RX buffer to be buffer (received data ready)
* * @param[in] channel    SCI Channel number
* @note      Parameter check is not held in this function.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_Wait_RXReady (R_SCI0_Type * p_sci_reg)
{
  	while (!p_sci_reg->SSR_b.RDRF)
	{
		/* Do nothing. */
	}
}  /* End of function HW_SCI_SIIC__Wait_RXReady() */

/*******************************************************************************************************************//**
* Waits for an ACK to be received
* @param[in] channel    SCI Channel number
* @note      Parameter check is not held in this function.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_SIIC_Wait_Ack (R_SCI0_Type * p_sci_reg)
{
  	while (1U == p_sci_reg->SISR_b.IICACKR)
	{
		/* Do nothing. */
	}
}  /* End of function HW_SCI_SIIC_Wait_Ack() */

/*******************************************************************************************************************//**
* @brief     This function returns the address of the write register
* @param[in] channel     SCI channel
* @note      Channel number is not checked in this function, caller function must check it.
***********************************************************************************************************************/
__STATIC_INLINE uint8_t volatile const * HW_SCI_SIICWriteAddrGet (R_SCI0_Type * p_sci_reg)
{
        return (uint8_t volatile const *) (&(p_sci_reg->TDR));
}  /* End of function HW_SCI_SIICWriteAddrGet() */

/*******************************************************************************************************************//**
* @brief     This function returns the address of the read register
* @param[in] channel     SCI channel
* @note      Channel number is not checked in this function, caller function must check it.
***********************************************************************************************************************/
__STATIC_INLINE uint8_t volatile const * HW_SCI_SIICReadAddrGet (R_SCI0_Type * p_sci_reg)
{
        return (uint8_t volatile const *) (&(p_sci_reg->RDR));
}  /* End of function HW_SCI_SIICReadAddrGet() */


/*******************************************************************************************************************//**
 * Enables transmit interrupt.
 * @param   channel     The hardware channel to access
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_TxIrqEnable(R_SCI0_Type * p_reg, sci_i2c_instance_ctrl_t * const p_ctrl)
{
    NVIC_EnableIRQ(p_ctrl->txi_irq);

    /* Give time for value to be updated */
    volatile uint8_t dummy;
    dummy = p_reg->SCR;
    dummy = p_reg->SCR;
    dummy = p_reg->SCR;
    SSP_PARAMETER_NOT_USED(dummy);

    p_reg->SCR_b.TIE = 1U;
}

/*******************************************************************************************************************//**
 * Enables receive interrupt.
 * @param   channel     The hardware channel to access
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_RxIrqEnable(R_SCI0_Type * p_reg, sci_i2c_instance_ctrl_t * const p_ctrl)
{
    NVIC_EnableIRQ(p_ctrl->rxi_irq);

    /* Give time for value to be updated */
    volatile uint8_t dummy;
    dummy = p_reg->SCR;
    dummy = p_reg->SCR;
    dummy = p_reg->SCR;
    SSP_PARAMETER_NOT_USED(dummy);

    p_reg->SCR_b.RIE = 1U;
}

/*******************************************************************************************************************//**
 * Enables transmit end interrupt.
 * @param   channel     The hardware channel to access
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_TeIrqEnable(R_SCI0_Type * p_reg, sci_i2c_instance_ctrl_t * const p_ctrl)
{
    NVIC_EnableIRQ(p_ctrl->tei_irq);

    /* Give time for value to be updated */
    volatile uint8_t dummy;
    dummy = p_reg->SCR;
    dummy = p_reg->SCR;
    dummy = p_reg->SCR;
    SSP_PARAMETER_NOT_USED(dummy);

    p_reg->SCR_b.TEIE = 1U;
}

/*******************************************************************************************************************//**
 * Disables transmit interrupt.
 * @param   channel     The hardware channel to access
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_TxIrqDisable(R_SCI0_Type * p_reg, sci_i2c_instance_ctrl_t * const p_ctrl)
{
    NVIC_DisableIRQ(p_ctrl->txi_irq);

    /* Give time for value to be updated */
    volatile uint8_t dummy;
    dummy = p_reg->SCR;
    dummy = p_reg->SCR;
    dummy = p_reg->SCR;
    SSP_PARAMETER_NOT_USED(dummy);

    p_reg->SCR_b.TIE = 0U;
}

/*******************************************************************************************************************//**
 * Disables receive interrupt.
 * @param   channel     The hardware channel to access
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_RxIrqDisable(R_SCI0_Type * p_reg, sci_i2c_instance_ctrl_t * const p_ctrl)
{
    NVIC_DisableIRQ(p_ctrl->rxi_irq);

    /* Give time for value to be updated */
    volatile uint8_t dummy;
    dummy = p_reg->SCR;
    dummy = p_reg->SCR;
    dummy = p_reg->SCR;
    SSP_PARAMETER_NOT_USED(dummy);

    p_reg->SCR_b.RIE = 0U;
}

/*******************************************************************************************************************//**
 * Disables transmit end interrupt.
 * @param   channel     The hardware channel to access
 **********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_TeIrqDisable(R_SCI0_Type * p_reg, sci_i2c_instance_ctrl_t * const p_ctrl)
{
    NVIC_DisableIRQ(p_ctrl->tei_irq);

    /* Give time for value to be updated */
    volatile uint8_t dummy;
    dummy = p_reg->SCR;
    dummy = p_reg->SCR;
    dummy = p_reg->SCR;
    SSP_PARAMETER_NOT_USED(dummy);

    p_reg->SCR_b.TEIE = 0U;
}

/*******************************************************************************************************************//**
* @brief     Perform a timed wait for transmission end flag to be set
* @param[in] channel     SCI channel
* @note      Channel number is not checked in this function, caller function must check it.
***********************************************************************************************************************/
__STATIC_INLINE void HW_SCI_TimedWaitForTEND (R_SCI0_Type * p_sci_reg)
{
   uint32_t timeout = SCI_IIC_WORST_CASE_TEND_DELAY;
   while((0 == p_sci_reg->SSR_b.TEND) && (0 != timeout))
   {
       timeout --;
   }
}

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_SCI_SIIC_PRIVATE_H */
