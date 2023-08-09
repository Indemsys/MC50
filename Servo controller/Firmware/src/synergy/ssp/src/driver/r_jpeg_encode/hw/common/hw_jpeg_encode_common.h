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
 * File Name    : hw_jpeg_encode_common.h
 * Description  : JPEG Encode (JPEG_ENCODE) Module hardware common header file.
 **********************************************************************************************************************/

#ifndef HW_JPEG_ENCODE_COMMON_H
#define HW_JPEG_ENCODE_COMMON_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

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

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/* Register bit definition for BUS arbitration control */

#define MSTP_JPEG (1U << 5)
#define YCBCR_BYTES_PER_PIXEL (2U)
#define YCBCR422 (0x1U)
/* define jpeg interrupt enable register 1 bits.  */
#define JPEG_INTE1_DINLEN  (0x00000020U)                ///< Enable input line number available in encode.
#define JPEG_INTE1_CBTEN   (0x00000040U)                ///< Enable data transfer completed in encode.

/* define jpeg interrupt status register 0 bits.  */
#define JPEG_INTE0_INS6 (0x40U)                         ///< complete encoding process or decoding process.

/* define jpeg interrupt status register 1 bits.  */
#define JPEG_INTE1_DINLF  (0x00000020U)                 ///< the number of input data lines is available in encode.
#define JPEG_INTE1_CBTF   (0x00000040U)                 ///< the last output coded data is written in encode.
#define JPEG_OPERATION_ENCODE (0x00U)

/* Huffman and Quantization table base address */
#define R_JPEG_QUANT_TABLE_BASE                     0x400E6100UL
#define R_JPEG_HUFFM_TABLE_BASE                     0x400E6200UL
#define QUANT_TABLE_SIZE                            (64)
#define HUFFM_DC_TABLE_SIZE                         (28)
#define HUFFM_AC_TABLE_SIZE                         (178)
/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
typedef struct {                                    /*!< R_JPEG_QUANT_TABLE_BASE_Type Structure */
     __IO uint8_t   JCQTBL0[64];
     __IO uint8_t   JCQTBL1[64];
     __IO uint8_t   JCQTBL2[64];
     __IO uint8_t   JCQTBL3[64];
} R_JPEG_QUANT_TABLE_BASE_Type;

typedef struct {                                    /*!< R_JPEG_HUFFM_TABLE_BASE_Type Structure */
    __IO uint8_t    JCHTBD0[28];
    __I  uint8_t    reserved_1[4];
    __IO uint8_t    JCHTBA0[178];
    __I  uint8_t    reserved_2[46];
    __IO uint8_t    JCHTBD1[28];
    __I  uint8_t    reserved_3[4];
    __IO uint8_t    JCHTBA1[178];
} R_JPEG_HUFFM_TABLE_BASE_Type;

#define R_JPEG_QUANT_TABLE                          ((R_JPEG_QUANT_TABLE_BASE_Type   *) R_JPEG_QUANT_TABLE_BASE)
#define R_JPEG_HUFFM_TABLE                          ((R_JPEG_HUFFM_TABLE_BASE_Type   *) R_JPEG_HUFFM_TABLE_BASE)

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * Function name : HW_JPEG_SetProcess
 * Description   : Set the JPEG work mode: encode or encode
 * @param     process         JPEG Encode or Encode
 **********************************************************************************************************************/
__STATIC_INLINE void HW_JPEG_SetProcess (R_JPEG_Type * p_jpeg_reg, uint8_t process)//CHECK
{
    p_jpeg_reg->JCMOD_b.DSP = process & 0x01U;
}  /* End of function HW_JPEG_SetProcess(R_JPEG_Type * p_jpeg_reg, ) */

/*******************************************************************************************************************//**
 * General setting, resst bus.
 * @param     none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_JPEG_BusReset (R_JPEG_Type * p_jpeg_reg) //CHECK
{
    p_jpeg_reg->JCCMD_b.BRST = 0x01;
}  /* End of function HW_JPEG_BusRest(R_JPEG_Type * p_jpeg_reg, ) */

/*******************************************************************************
 * Clear the JPEG Core End command
 * @param        : None
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_ClearRequest (R_JPEG_Type * p_jpeg_reg)
{
    p_jpeg_reg->JCCMD_b.JEND = 0x1;
}

/*******************************************************************************
 * Set Pixel Format
 * Arguments     pixel_format    Input data pixel format to be configured
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_EncodeInputPixelFormatSet (R_JPEG_Type * p_jpeg_reg, uint8_t pixel_format)
{
    p_jpeg_reg->JCMOD_b.REDU = pixel_format & 0x07U;
}

/*******************************************************************************
 * Enable encode Input Count Mode
 * @param        None
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_EncodeInCountModeEnable (R_JPEG_Type * p_jpeg_reg)
{
    p_jpeg_reg->JIFECNT_b.DINLC = 0x01;
}

/*******************************************************************************
 * Disable encode Input Count Mode
 * @param        None
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_EncodeInCountModeDisable (R_JPEG_Type * p_jpeg_reg)
{
    p_jpeg_reg->JIFECNT_b.DINLC = 0x00;
}

/*******************************************************************************
 * Disable encode Output Count Mode
 * @param        None
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_EncodeInputCountModeDisable (R_JPEG_Type * p_jpeg_reg)
{
    p_jpeg_reg->JIFECNT_b.DINRCMD = 0x00;
}

/*******************************************************************************
 * Configure encode Output Count Mode
 * @param        resume_mode      Set the output resume mode
 * @param        num_of_lines     Lines to encode
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_EncodeInputCountModeConfig (R_JPEG_Type * p_jpeg_reg, uint8_t resume_mode, uint16_t num_of_lines)
{
    p_jpeg_reg->JIFECNT_b.DINRINI = resume_mode & 0x01U;  //0x0:Not Reset Address, 0x1:Reset Address
    p_jpeg_reg->JIFESLC_b.LINES    = num_of_lines; //Number of lines to encode
}

/*******************************************************************************
 * Encode output count mode Resume Command
 * @param        resume_enb          Resume mode
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_EncodeInputCountModeResume (R_JPEG_Type * p_jpeg_reg, uint8_t resume_enb)
{
    p_jpeg_reg->JIFECNT_b.DINRCMD = (uint32_t)resume_enb & 0x01U;
}

/*******************************************************************************
 * Configure encode source buffer address
 * @param        src_jpeg             source jpeg data
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_EncodeSourceAddressSet (R_JPEG_Type * p_jpeg_reg, void * src_jpeg)
{
    p_jpeg_reg->JIFESA = (uint32_t) src_jpeg;
}

/*******************************************************************************
 * Conifugre encode destination address
 * @address        Output buffer address
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_EncodeDestinationAddressSet (R_JPEG_Type * p_jpeg_reg, uint32_t address)
{
    p_jpeg_reg->JIFEDA = ((uint32_t)address & 0xFFFFFFF8);
}

/*******************************************************************************
 * Retrieves encode destination address
 * @param          None
 * @return         Destination Address
 *******************************************************************************/
__STATIC_INLINE uint32_t HW_JPEG_EncodeDestinationAddressGet (R_JPEG_Type * p_jpeg_reg)
{
    return (p_jpeg_reg->JIFEDA);
}

/*******************************************************************************
 * Setting Data Swap
 * @param          input_swap_mode    Input Data Swap Mode 
 * @param          output_swap_mode   Output Data Swap Mode
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_EncodeDataSwap (R_JPEG_Type * p_jpeg_reg, uint8_t input_swap_mode, uint8_t output_swap_mode)//CHECK
{
    p_jpeg_reg->JIFECNT_b.JOUTSWAP  = output_swap_mode & 0x07U;
    p_jpeg_reg->JIFECNT_b.DINSWAP = input_swap_mode & 0x07U;
}

/*******************************************************************************
 * Get the number of lines encoded into the output buffer
 * @param           None
 * @return          The number of lines
 *******************************************************************************/
__STATIC_INLINE uint32_t HW_JPEG_LinesEncodedGet (R_JPEG_Type * p_jpeg_reg)
{
    return (uint16_t)p_jpeg_reg->JIFESLC_b.LINES;
}

/*******************************************************************************
 * Set output horizontal stride
 * @param            horizontal_stride   Horizontal stride value, in bytes
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_EncodeHorizontalStrideSet (R_JPEG_Type * p_jpeg_reg, uint32_t horizontal_stride)//CHECK
{
    p_jpeg_reg->JIFESOFST = ((horizontal_stride) & 0x00007FFF8);
}

/*******************************************************************************
 * Set InterruptEnable0 Value
 * @param      interrupts  Interrupt mask bits to program for InterruptEnable0
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_InterruptEnable0Set (R_JPEG_Type * p_jpeg_reg, uint8_t interrupts)
{
    p_jpeg_reg->JINTE0 = interrupts;
}

/*******************************************************************************
 * Set InterruptEnable1 Value
 * @param      interrupts  Interrupt mask bits to program for InterruptEnable1
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_InterruptEnable1Set (R_JPEG_Type * p_jpeg_reg, uint32_t interrupts)
{
    p_jpeg_reg->JINTE1 = interrupts;
}

/*******************************************************************************
 * Retrieves InterruptEnable1 Value
 * @param      None
 * @return     INTE1 value
 *******************************************************************************/
__STATIC_INLINE uint32_t HW_JPEG_InterruptEnable1Get (R_JPEG_Type * p_jpeg_reg)
{
    return (p_jpeg_reg->JINTE1);
}

/*******************************************************************************
 * Retrieves IntStatus0 Value
 * @param      None
 * @return     INT0 status value
 *******************************************************************************/
__STATIC_INLINE uint8_t HW_JPEG_InterruptStatus0Get (R_JPEG_Type * p_jpeg_reg)
{
    return (p_jpeg_reg->JINTS0);
}

/*******************************************************************************
 * Retrieves IntStatus1 Value
 * @param      None
 * @return     INT1 status value
 *******************************************************************************/
__STATIC_INLINE uint32_t HW_JPEG_InterruptStatus1Get (R_JPEG_Type * p_jpeg_reg)
{
    return (p_jpeg_reg->JINTS1);
}

/*******************************************************************************
 * Set InterruptStatus0 Value
 * @param      value  InterruptStatus0 value to be configured
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_InterruptStatus0Set (R_JPEG_Type * p_jpeg_reg, uint8_t value)
{
    p_jpeg_reg->JINTS0 = value;
}
/*******************************************************************************
 * Start encode process
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_Encoder_start (R_JPEG_Type * p_jpeg_reg)
{
    p_jpeg_reg->JCCMD_b.JSRT = 0x1;
}

/*******************************************************************************
 * Set InterruptStatus1 Value
 * @param      value  InterruptStatus1 value to be configured
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_InterruptStatus1Set (R_JPEG_Type * p_jpeg_reg, uint8_t value)
{
    p_jpeg_reg->JINTS1 = value;
}

/*******************************************************************************
 * Get JPEG image size (horizontal and vertial)
 * @param      p_horizontal  Pointer to the storage space for the horizontal value
 * @param      p_vertical    Pointer to the storage space for the vertical value
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_ImageSizeSet (R_JPEG_Type * p_jpeg_reg, uint16_t horizontal, uint16_t vertical)
{
	/* Vertical image size */
    p_jpeg_reg->JCVSZU_b.VSZU = (uint8_t)((vertical) >> 8);
    p_jpeg_reg->JCVSZD_b.VSZD = (uint8_t)((vertical) &  0xff);

	/* Horizontal image size */
    p_jpeg_reg->JCHSZU_b.HSZU = (uint8_t)((horizontal) >> 8);
    p_jpeg_reg->JCHSZD_b.HSZD = (uint8_t)((horizontal) &  0xff);
}

/*******************************************************************************
 * Quantization table setting
 * @param     table number
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_EncoderQunatizationTable (R_JPEG_Type * p_jpeg_reg, uint8_t q_table )
{
    p_jpeg_reg->JCQTN = q_table;
}
/*******************************************************************************
 * huffman table setting
 * @param     table number
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_EncoderHuffmanTable (R_JPEG_Type * p_jpeg_reg, uint8_t h_table )
{
    p_jpeg_reg->JCHTN = h_table;
}
/*** DRI setting Reset MARKER */
__STATIC_INLINE void HW_JPEG_EncoderResetMarker (R_JPEG_Type * p_jpeg_reg, uint16_t reset_marker )
{
    p_jpeg_reg->JCDRIU_b.DRIU = (uint8_t)(reset_marker >> 8);
    p_jpeg_reg->JCDRID_b.DRID = (uint8_t)(reset_marker & 0xff);
}
__STATIC_INLINE uint32_t HW_JPEG_EncoderCompressedLineGet(R_JPEG_Type * p_jpeg_reg)
{
    uint32_t compressed_size = 0;
    compressed_size = (p_jpeg_reg->JCDTCU_b.DCU);
    compressed_size = (compressed_size << 8)|(p_jpeg_reg->JCDTCM_b.DCM);
    compressed_size = (compressed_size << 8)|(p_jpeg_reg->JCDTCD_b.DCD);
    return compressed_size;
}/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_JPEG_ENCODE_COMMON_H */
