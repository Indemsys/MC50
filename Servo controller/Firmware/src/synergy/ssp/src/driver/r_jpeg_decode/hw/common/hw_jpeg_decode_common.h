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
 * File Name    : hw_jpeg_decode_common.h
 * Description  : JPEG Decode (JPEG_DECODE) Module hardware common header file.
 **********************************************************************************************************************/

#ifndef HW_JPEG_DECODE_COMMON_H
#define HW_JPEG_DECODE_COMMON_H

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

/* define jpeg interrupt enable register 0 bits.  */
#define JPEG_INTE0_INT3 (0x08U)                         ///< Enable image size available in decode.
#define JPEG_INTE0_INT5 (0x20U)                         ///< Enable the final number error in decode.
#define JPEG_INTE0_INT6 (0x40U)                         ///< Enable the total number error in decode.
#define JPEG_INTE0_INT7 (0x80U)                         ///< Enable the number error of restart interval in decode.

/* define jpeg interrupt enable register 1 bits.  */
#define JPEG_INTE1_DOUTLEN (0x00000001U)                ///< Enable output lines number available in decode.
#define JPEG_INTE1_JINEN   (0x00000002U)                ///< Enable input amount available in decode.
#define JPEG_INTE1_DBTEN   (0x00000004U)                ///< Enable data transfer completed in decode.
#define JPEG_INTE1_DINLEN  (0x00000020U)                ///< Enable input line number available in encode.
#define JPEG_INTE1_CBTEN   (0x00000040U)                ///< Enable data transfer completed in encode.

/* define jpeg interrupt status register 0 bits.  */
#define JPEG_INTE0_INS3 (0x08U)                         ///< image size is available in decode.
#define JPEG_INTE0_INS5 (0x20U)                         ///< encounter a encoded data error in decode.
#define JPEG_INTE0_INS6 (0x40U)                         ///< complete encoding process or decoding process.

/* define jpeg interrupt status register 1 bits.  */
#define JPEG_INTE1_DOUTLF (0x00000001U)                 ///< the number of lines of output image data is available in decode.
#define JPEG_INTE1_JINF   (0x00000002U)                 ///< the amount of input coded data is available in decode.
#define JPEG_INTE1_DBTF   (0x00000004U)                 ///< the last output image data is written in decode.
#define JPEG_INTE1_DINLF  (0x00000020U)                 ///< the number of input data lines is available in encode.
#define JPEG_INTE1_CBTF   (0x00000040U)                 ///< the last output coded data is written in encode.

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
#define JPEG_OPERATION_ENCODE (0x00U)
#define JPEG_OPERATION_DECODE (0x01U)

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
 * Description   : Set the JPEG work mode: decode or encode
 * @param     p_jpeg_reg      JPEG register
 * @param     process         JPEG Encode or Decode
 **********************************************************************************************************************/
__STATIC_INLINE void HW_JPEG_SetProcess (R_JPEG_Type * p_jpeg_reg, uint8_t process)
{
    p_jpeg_reg->JCMOD_b.DSP = process & 0x01U;
}  /* End of function HW_JPEG_SetProcess(R_JPEG_Type * p_jpeg_reg, ) */

/*******************************************************************************************************************//**
 * General setting, resst bus.
 * @param     p_jpeg_reg      JPEG register
 **********************************************************************************************************************/
__STATIC_INLINE void HW_JPEG_BusReset (R_JPEG_Type * p_jpeg_reg)
{
    p_jpeg_reg->JCCMD_b.BRST = 0x01;
}  /* End of function HW_JPEG_BusRest(R_JPEG_Type * p_jpeg_reg, ) */

/*******************************************************************************************************************//**
 * Start JPEG core.                  
 * Description   : start the JPEG Core
 * @param     p_jpeg_reg      JPEG register
**********************************************************************************************************************/
__STATIC_INLINE void HW_JPEG_CoreStart (R_JPEG_Type * p_jpeg_reg)
{
    p_jpeg_reg->JCCMD_b.JSRT = 0x1;
}

/*******************************************************************************************************************//**
 * General setting, clear CoreStop
 * @param[in] none
 **********************************************************************************************************************/
__STATIC_INLINE void HW_JPEG_CoreStopClear (R_JPEG_Type * p_jpeg_reg)
{
    p_jpeg_reg->JCCMD_b.JRST = 0x1;
}

/*******************************************************************************
 * Set source data size.                           
 * @param          size     Source data size in bytes
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_SourceDataSizeSet (R_JPEG_Type * p_jpeg_reg, uint32_t size)
{
    p_jpeg_reg->JIFDSDC_b.JDATAS = (size & 0x0000FFF8);
}

/*******************************************************************************
 * Clear the JPEG Core stop command
 * @param     p_jpeg_reg      JPEG register
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_ClearRequest (R_JPEG_Type * p_jpeg_reg)
{
    p_jpeg_reg->JCCMD_b.JEND = 0x1;
}

/*******************************************************************************
 * Set Pixel Format
 * Arguments     pixel_format    Input data pixel format to be configured
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_InputPixelFormatSet (R_JPEG_Type * p_jpeg_reg, uint8_t pixel_format)
{
    p_jpeg_reg->JCMOD_b.REDU = pixel_format & 0x07U;
}

/*******************************************************************************
 * Get Pixel Format
 * @param     p_jpeg_reg      JPEG register
 * @return       Pixel Format
 *******************************************************************************/
__STATIC_INLINE jpeg_decode_color_space_t HW_JPEG_InputPixelFormatGet (R_JPEG_Type * p_jpeg_reg)
{
    return (jpeg_decode_color_space_t) (p_jpeg_reg->JCMOD_b.REDU);
}

/*******************************************************************************
 * Enable decode Input Count Mode
 * @param     p_jpeg_reg      JPEG register
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_DecodeInCountModeEnable (R_JPEG_Type * p_jpeg_reg)
{
    p_jpeg_reg->JIFDCNT_b.JINC = 0x01;
}

/*******************************************************************************
 * Disable decode Input Count Mode
 * @param     p_jpeg_reg      JPEG register
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_DecodeInCountModeDisable (R_JPEG_Type * p_jpeg_reg)
{
    p_jpeg_reg->JIFDCNT_b.JINC = 0x00;
}

/*******************************************************************************
 * Configure decode Input Count Mode
 * @param        resume_mode     Set the input resume mode
 * @param        num_of_data     Input data size in bytes
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_DecodeInCountModeConfig (R_JPEG_Type * p_jpeg_reg, uint8_t resume_mode, uint16_t num_of_data)
{
    p_jpeg_reg->JIFDCNT_b.JINRINI = resume_mode & 0x01U; //0x0:Not Reset Address, 0x1:Reset Address
    p_jpeg_reg->JIFDSDC_b.JDATAS  = num_of_data; //Number of data is transfer when count-mode is on (in 8-bytes uint)
}

/*******************************************************************************
 * Enable decode Output Count Mode
 * @param     p_jpeg_reg      JPEG register
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_DecodeOutCountModeEnable (R_JPEG_Type * p_jpeg_reg)
{
    p_jpeg_reg->JIFDCNT_b.DOUTLC = 0x01;
}

/*******************************************************************************
 * Disable decode Output Count Mode
 * @param        None
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_DecodeOutCountModeDisable (R_JPEG_Type * p_jpeg_reg)
{
    p_jpeg_reg->JIFDCNT_b.DOUTLC = 0x00;
}

/*******************************************************************************
 * Configure decode Output Count Mode
 * @param        resume_mode      Set the output resume mode
 * @param        num_of_lines     Lines to decode
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_DecodeOutCountModeConfig (R_JPEG_Type * p_jpeg_reg, uint8_t resume_mode, uint16_t num_of_lines)
{
    p_jpeg_reg->JIFDCNT_b.DOUTRINI = resume_mode & 0x01U;  //0x0:Not Reset Address, 0x1:Reset Address
    p_jpeg_reg->JIFDDLC_b.LINES    = num_of_lines; //Number of lines to decode
}

/*******************************************************************************
 * Decode input count mode Resume Command
 * @param        resume_enb          Resume mode
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_DecodeInCountModeResume (R_JPEG_Type * p_jpeg_reg, uint8_t resume_enb)
{
    p_jpeg_reg->JIFDCNT_b.JINRCMD = (uint32_t)resume_enb & 0x01U;
}

/*******************************************************************************
 * Decode output count mode Resume Command
 * @param     p_jpeg_reg      JPEG register
 * @param        resume_enb          Resume mode
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_DecodeOutCountModeResume (R_JPEG_Type * p_jpeg_reg, uint8_t resume_enb)
{
    p_jpeg_reg->JIFDCNT_b.DOUTRCMD = (uint32_t)resume_enb & 0x01U;
}

/*******************************************************************************
 * Configure decode source buffer address
 * @param        src_jpeg             source jpeg data
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_DecodeSourceAddressSet (R_JPEG_Type * p_jpeg_reg, void * src_jpeg)
{
    p_jpeg_reg->JIFDSA = (uint32_t) src_jpeg;
}

/*******************************************************************************
 * Conifugre decode destination address
 * @address        Output buffer address
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_DecodeDestinationAddressSet (R_JPEG_Type * p_jpeg_reg, uint32_t address)
{
    p_jpeg_reg->JIFDDA = address;
}

/*******************************************************************************
 * Retrieves the address of the decode output buffer
 * @param     p_jpeg_reg      JPEG register
 * @return        Destination Address
 *******************************************************************************/
__STATIC_INLINE uint32_t HW_JPEG_DecodeSourceAddressGet (R_JPEG_Type * p_jpeg_reg)
{
    return (p_jpeg_reg->JIFDSA);
}

/*******************************************************************************
 * Retrieves decode destination address
 * @param     p_jpeg_reg      JPEG register
 * @return         Destination Address
 *******************************************************************************/
__STATIC_INLINE uint32_t HW_JPEG_DecodeDestinationAddressGet (R_JPEG_Type * p_jpeg_reg)
{
    return (p_jpeg_reg->JIFDDA);
}

/*******************************************************************************
 * Setting Data Swap
 * @param          input_swap_mode    Input Data Swap Mode 
 * @param          output_swap_mode   Output Data Swap Mode
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_DecodeDataSwap (R_JPEG_Type * p_jpeg_reg, uint8_t input_swap_mode, uint8_t output_swap_mode)
{
    p_jpeg_reg->JIFDCNT_b.JINSWAP  = input_swap_mode & 0x07U;
    p_jpeg_reg->JIFDCNT_b.DOUTSWAP = output_swap_mode & 0x07U;
}

/*******************************************************************************
 * Get the number of lines decoded into the output buffer
 * @param     p_jpeg_reg      JPEG register
 * @return          The number of lines
 *******************************************************************************/
__STATIC_INLINE uint32_t HW_JPEG_LinesDecodedGet (R_JPEG_Type * p_jpeg_reg)
{
    return p_jpeg_reg->JIFDDLC_b.LINES;
}

/*******************************************************************************
 * Get the Error number
 * @param          None
 * @return         the number of JPEG IRQ
 *******************************************************************************/
__STATIC_INLINE uint8_t HW_JPEG_ErrorGet (R_JPEG_Type * p_jpeg_reg)
{
    /*if(1==p_jpeg_reg->JINTS0_b.INS5)*/ //whether to do this.
    return (p_jpeg_reg->JCDERR_b.ERR);
}

/*******************************************************************************
 * Select Pixel Format
 * @param        format    Output image format
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_DecodeOutputImageFormatSet (R_JPEG_Type * p_jpeg_reg, uint8_t format)
{
    p_jpeg_reg->JIFDCNT_b.OPF = format & 0x03U;
}

/*******************************************************************************
 * Get Pixel Format
 * @param     p_jpeg_reg      JPEG register
 * @return         Pixel Format
 *******************************************************************************/
__STATIC_INLINE int HW_JPEG_DecodeOutputImageFormatGet (R_JPEG_Type * p_jpeg_reg)
{
    return (p_jpeg_reg->JIFDCNT_b.OPF);
}

/*******************************************************************************
 * Set the decoding output sub sample
 * @param     p_jpeg_reg      JPEG register
 * @param                V_subsampling  Vertical Subsample value
 * @param                H_subsampling  Horizontal Subsample value
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_OutputSubsampleSet (R_JPEG_Type * p_jpeg_reg, uint8_t V_subsampling, uint8_t H_subsampling)
{
    p_jpeg_reg->JIFDCNT_b.VINTER = V_subsampling & 0x03U;
    p_jpeg_reg->JIFDCNT_b.HINTER = H_subsampling & 0x03U;
}

/*******************************************************************************
 * Set output horizontal stride
 * @param     p_jpeg_reg      JPEG register
 * @param            horizontal_stride   Horizontal stride value, in bytes
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_HorizontalStrideSet (R_JPEG_Type * p_jpeg_reg, uint32_t horizontal_stride)
{
    p_jpeg_reg->JIFDDOFST = horizontal_stride;
}

/*******************************************************************************
 * Get output horizontal stride
 * @param     p_jpeg_reg      JPEG register
 * @return         Horizontal stride value, in bytes
 *******************************************************************************/
__STATIC_INLINE uint32_t HW_JPEG_HorizontalStrideGet (R_JPEG_Type * p_jpeg_reg)
{
    return (p_jpeg_reg->JIFDDOFST);
}

/*******************************************************************************
 * Set the output image alpha
 * @param     p_jpeg_reg      JPEG register
 * @param      Alpha  Alpha value to be applied to ARGB8888 output format
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_DecodeOutputAlphaSet (R_JPEG_Type * p_jpeg_reg, uint8_t alpha)
{
    p_jpeg_reg->JIFDADT = alpha;
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
 * @param     p_jpeg_reg      JPEG register
 * @param      interrupts  Interrupt mask bits to program for InterruptEnable1
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_InterruptEnable1Set (R_JPEG_Type * p_jpeg_reg, uint32_t interrupts)
{
    p_jpeg_reg->JINTE1 = interrupts;
}

/*******************************************************************************
 * Retrieves InterruptEnable1 Value
 * @param     p_jpeg_reg      JPEG register
 * @return     INTE1 value
 *******************************************************************************/
__STATIC_INLINE uint32_t HW_JPEG_InterruptEnable1Get (R_JPEG_Type * p_jpeg_reg)
{
    return (p_jpeg_reg->JINTE1);
}

/*******************************************************************************
 * Retrieves IntStatus0 Value
 * @param     p_jpeg_reg      JPEG register
 * @return     INT0 status value
 *******************************************************************************/
__STATIC_INLINE uint8_t HW_JPEG_InterruptStatus0Get (R_JPEG_Type * p_jpeg_reg)
{
    return (p_jpeg_reg->JINTS0);
}

/*******************************************************************************
 * Retrieves IntStatus1 Value
 * @param     p_jpeg_reg      JPEG register
 * @return     INT1 status value
 *******************************************************************************/
__STATIC_INLINE uint32_t HW_JPEG_InterruptStatus1Get (R_JPEG_Type * p_jpeg_reg)
{
    return (p_jpeg_reg->JINTS1);
}

/*******************************************************************************
 * Set InterruptStatus0 Value
 * @param     p_jpeg_reg      JPEG register
 * @param      value  InterruptStatus0 value to be configured
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_InterruptStatus0Set (R_JPEG_Type * p_jpeg_reg, uint8_t value)
{
    p_jpeg_reg->JINTS0 = value;
}

/*******************************************************************************
 * Set InterruptStatus1 Value
 * @param     p_jpeg_reg      JPEG register
 * @param      value  InterruptStatus1 value to be configured
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_InterruptStatus1Set (R_JPEG_Type * p_jpeg_reg, uint8_t value)
{
    p_jpeg_reg->JINTS1 = value;
}

/*******************************************************************************
 * Get JPEG Decode Error
 * @param     p_jpeg_reg      JPEG register
 * @return     JPEG error code
 *******************************************************************************/
__STATIC_INLINE uint8_t HW_JPEG_DecodeErrorGet (R_JPEG_Type * p_jpeg_reg)
{
    return (p_jpeg_reg->JCDERR_b.ERR);
}

/*******************************************************************************
 * Get JPEG image size (horizontal and vertial)
 * @param     p_jpeg_reg      JPEG register
 * @param      p_horizontal  Pointer to the storage space for the horizontal value
 * @param      p_vertical    Pointer to the storage space for the vertical value
 *******************************************************************************/
__STATIC_INLINE void HW_JPEG_ImageSizeGet (R_JPEG_Type * p_jpeg_reg, uint16_t * p_horizontal, uint16_t * p_vertical)
{
    uint16_t upper;
    uint16_t lower;
    upper = (uint16_t) (p_jpeg_reg->JCHSZU << 8);
    lower = (uint16_t) (p_jpeg_reg->JCHSZD);
    *p_horizontal = (upper | lower);
    upper = (uint16_t) (p_jpeg_reg->JCVSZU << 8);
    lower = (uint16_t) (p_jpeg_reg->JCVSZD);
    *p_vertical   = (upper | lower);
}

/*******************************************************************************
 * Set JPEG image size (horizontal and vertical)
 * @param	p_jpeg_reg      JPEG register
 * @param	p_horizontal    Horizontal image size to set
 * @param	p_vertical      Vertical image size to set
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

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* HW_JPEG_DECODE_COMMON_H */
