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
 * File Name    : r_jpeg_decode.h
 * Description  : JPEG Decoder (JPEG_DECODE) Module instance header file.
 **********************************************************************************************************************/

/*****************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup JPEG_DECODE JPEG CODEC
 * @brief Driver for the JPEG CODEC.
 *
 * @{
 **********************************************************************************************************************/

#ifndef R_JPEG_DECODE_H
#define R_JPEG_DECODE_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_jpeg_decode_api.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define JPEG_DECODE_CODE_VERSION_MAJOR (2U)
#define JPEG_DECODE_CODE_VERSION_MINOR (0U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** JPEG Codec module control block.  DO NOT INITIALIZE.  Initialization occurs when jpep_api_t::open is called. */
typedef struct st_jpeg_decode_instance_ctrl
{
    jpeg_decode_status_t        status;                             ///< JPEG Codec module status
    ssp_err_t                   error_code;                         ///< JPEG Codec error code (if any).
    void (* p_callback)(jpeg_decode_callback_args_t *p_args);       ///< User-supplied callback functions.
    /* Pointer to JPEG codec peripheral specific configuration */
    void const  * p_extend;       ///< JPEG Codec hardware dependent configuration */
    void const  * p_context;      ///< Placeholder for user data.  Passed to user callback in ::jpeg_decode_callback_args_t.
    R_JPEG_Type * p_reg;          ///< Pointer to register base address

    jpeg_decode_pixel_format_t  pixel_format;        ///< Pixel format
    uint32_t                    horizontal_stride;   ///< Horizontal Stride settings.
    uint32_t                    outbuffer_size;      ///< out buffer size
    uint16_t                    total_lines_decoded; ///< Track the number of lines decoded so far.
    jpeg_decode_subsample_t     horizontal_subsample;    ///< Horizontal sub-sample setting.

} jpeg_decode_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Function Prototypes
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
extern const jpeg_decode_api_t g_jpeg_decode_on_jpeg_decode;
/** @endcond */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif
/*******************************************************************************************************************//**
 * @} (end defgroup JPEG_DECODE)
 **********************************************************************************************************************/
