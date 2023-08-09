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
 * File Name    : r_jpeg_encode.h
 * Description  : JPEG Encoder (JPEG_ENCODE) Module instance header file.
 **********************************************************************************************************************/

/*****************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup JPEG_ENCODE JPEG ENCODE
 * @brief Driver for the JPEG CODEC.
 *
 * @{
 **********************************************************************************************************************/

#ifndef R_JPEG_ENCODE_H
#define R_JPEG_ENCODE_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_jpeg_encode_api.h"
/** Configuration for this module */
#include "r_jpeg_encode_cfg.h"
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define JPEG_ENCODE_CODE_VERSION_MAJOR (2U)
#define JPEG_ENCODE_CODE_VERSION_MINOR (0U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** JPEG Codec module control block.  DO NOT INITIALIZE.  Initialization occurs when jpep_api_t::open is called. */
typedef struct st_jpeg_encode_instance_ctrl
{
    volatile jpeg_encode_status_t        status;                    ///< JPEG Codec module status
    void (* p_callback)(jpeg_encode_callback_args_t *p_args);       ///< User-supplied callback functions.
    /* Pointer to JPEG codec peripheral specific configuration */
    void const  * p_extend;       ///< JPEG Codec hardware dependent configuration */
    void const  * p_context;      ///< Placeholder for user data.  Passed to user callback in ::jpeg_encode_callback_args_t.
    R_JPEG_Type * p_reg;          ///< Pointer to register base address
    uint32_t                    horizontal_stride;      ///< Horizontal Stride settings (Line offset).
    uint32_t                    output_buffer_size;     ///< out buffer size
    uint16_t                    lines_to_encoded;       ///< Number of lines to encode
    uint16_t                    vertical_resolution;    ///< vertical size
    uint16_t                    encoded_lines;          ///< Number of lines encoded
} jpeg_encode_instance_ctrl_t;

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
extern const jpeg_encode_api_t g_jpeg_encode_on_jpeg_encode;
/** @endcond */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif
/*******************************************************************************************************************//**
 * @} (end defgroup JPEG_ENCODE)
 **********************************************************************************************************************/
