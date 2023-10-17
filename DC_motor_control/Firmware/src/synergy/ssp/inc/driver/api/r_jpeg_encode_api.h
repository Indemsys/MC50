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
 * File Name    : r_jpeg_encode_api.h
 * Description  : JPEG_ENCODE interface layer API
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup JPEG_ENCODE_API JPEG Encode Interface
 * @brief Interface for JPEG encode functions.
 *
 * @section JPEG_ENCODE_API_SUMMARY Summary
 * The JPEG ENCODE interface provides JPEG encoder functionality.  It allows application to convert a JPEG image into
 * bitmap data suitable for display frame buffer.
 *
 * Related SSP architecture topics:
 *  - @ref ssp-interfaces
 *  - @ref ssp-predefined-layers
 *  - @ref using-ssp-modules
 *
 * JPEG ENCODE Interface description: @ref HALJPEGEncodeInterface
 *
 * @{
 **********************************************************************************************************************/

#ifndef DRV_JPEG_ENCODE_API_H
#define DRV_JPEG_ENCODE_API_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
/** Register definitions, common services and error codes. */
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define JPEG_ENCODE_API_VERSION_MAJOR (2U)
#define JPEG_ENCODE_API_VERSION_MINOR (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** Multi-byte Data Format */
typedef enum e_jpeg_encode_data_format
{
    JPEG_ENCODE_DATA_FORMAT_NORMAL = 0,                 ///< (1)(2)(3)(4)(5)(6)(7)(8) Normal byte order
    JPEG_ENCODE_DATA_FORMAT_BYTE_SWAP,                  ///< (2)(1)(4)(3)(6)(5)(8)(7) Byte Swap
    JPEG_ENCODE_DATA_FORMAT_WORD_SWAP,                  ///< (3)(4)(1)(2)(7)(8)(5)(6) Word Swap
    JPEG_ENCODE_DATA_FORMAT_WORD_BYTE_SWAP,             ///< (4)(3)(2)(1)(8)(7)(6)(5) Word-Byte Swap
    JPEG_ENCODE_DATA_FORMAT_LONGWORD_SWAP,              ///< (5)(6)(7)(8)(1)(2)(3)(4) Longword Swap
    JPEG_ENCODE_DATA_FORMAT_LONGWORD_BYTE_SWAP,         ///< (6)(5)(8)(7)(2)(1)(4)(3) Longword Byte Swap
    JPEG_ENCODE_DATA_FORMAT_LONGWORD_WORD_SWAP,         ///< (7)(8)(5)(6)(3)(4)(1)(2) Longword Word Swap
    JPEG_ENCODE_DATA_FORMAT_LONGWORD_WORD_BYTE_SWAP,    ///< (8)(7)(6)(5)(4)(3)(2)(1) Longword Word Byte Swap
    JPEG_ENCODE_DATA_FORMAT_MAX                         ///< Maximum value of data format
} jpeg_encode_data_format_t;

/** JPEG HLD driver internal status information.
 * The driver can simultaneously be in more than any one status at the same time.
 * Parse the status bit-fields using the definitions in this enum to determine driver status */
typedef enum e_jpeg_encode_status
{
    JPEG_ENCODE_STATUS_FREE                  = 0x0,     ///< JPEG codec module is not yet open.
    JPEG_ENCODE_STATUS_IDLE                  = 0x1,     ///< JPEG Codec module is open, and is not operational.
    JPEG_ENCODE_STATUS_RUNNING               = 0x2,     ///< JPEG Codec is running.
    JPEG_ENCODE_STATUS_DONE                  = 0x4,     ///< JPEG Codec has successfully finished the operation.
    JPEG_ENCODE_STATUS_INPUT_PAUSE           = 0x8,     ///< JPEG Codec paused waiting for more input data.
} jpeg_encode_status_t;

/** Data type for encoding count mode enable. */
typedef enum e_jpeg_encode_count
{
    JPEG_ENCODE_COUNT_DISABLE = 0,                  ///< Count mode disable.
    JPEG_ENCODE_COUNT_ENABLE                        ///< Count mode enable.
} jpeg_encode_count_t;

/** Data type for encoding resume mode */
typedef enum e_jpeg_encode_resume_mode
{
    JPEG_ENCODE_COUNT_MODE_ADDRESS_CONTINUE = 0,    ///< The data buffer address will not be initialized when resuming image data lines.
    JPEG_ENCODE_COUNT_MODE_ADDRESS_REINITIALIZE     ///< The data buffer address will be initialized when resuming image data lines.
} jpeg_encode_resume_mode_t;

/** Image parameter structure */
typedef struct st_jpeg_encode_raw_image_parameter
{
    uint16_t horizontal_stride;     ///< Horizontal stride
    uint16_t horizontal_resolution; ///< Horizontal Resolution in pixel
    uint16_t vertical_resolution;   ///< Vertical Resolution in pixel
} jpeg_encode_raw_image_parameters;

/** Callback status structure */
typedef struct st_jpeg_encode_callback_args
{
    volatile jpeg_encode_status_t status;    ///< JPEG status
    uint32_t image_size;                     ///< JPEG image size
    void const * p_context;                  ///< Pointer to user-provided context
} jpeg_encode_callback_args_t;

/** User configuration structure, used in open function. */
typedef struct st_jpeg_encode_cfg
{
    jpeg_encode_data_format_t   input_data_format;           ///< Input data stream byte order
    jpeg_encode_data_format_t   output_data_format;          ///< Output data stream byte order
    uint16_t                    dri_marker;                  ///< DRI Marker setting 0 :- No DRI and RST marker
    uint8_t                     jdti_ipl;                    ///< Data transfer interrupt priority
    uint8_t                     jedi_ipl;                    ///< Decompression interrupt priority
    uint8_t                     quality_factor;              ///< JPEG image quality
    uint16_t                    vertical_resolution;         ///< vertical resolution of input image
    uint16_t                    horizontal_resolution;       ///< horizontal resolution of input image
    uint8_t const             * p_quant_luma_table;          ///< Luma table
    uint8_t const             * p_quant_croma_table;         ///< croma table
    uint8_t const             * p_huffman_luma_ac_table;     ///< Huffman AC table for luma
    uint8_t const             * p_huffman_luma_dc_table;     ///< Huffman DC table for luma
    uint8_t const             * p_huffman_croma_ac_table;    ///< Huffman AC table for croma
    uint8_t const             * p_huffman_croma_dc_table;    ///< Huffman DC table for croma
    void (* p_callback)(jpeg_encode_callback_args_t *p_args);///< User-supplied callback functions.
    void const * p_context;                                  ///< Placeholder for user data.  Passed to user callback in ::jpeg_encode_callback_args_t.
} jpeg_encode_cfg_t;

/** JPEG encode control block.  Allocate an instance specific control block to pass into the JPEG encode API calls.
 * @par Implemented as
 * - jpeg_encode_instance_ctrl_t
 */
typedef void jpeg_encode_ctrl_t;

/** JPEG functions implemented at the HAL layer will follow this API. */
typedef struct st_jpeg_encode_api
{
    /** Initial configuration
     * @par Implemented as
     * - R_JPEG_Encode_Open()
     *
     * @pre none
     *
     * @param[in,out] p_ctrl    Pointer to control block.  Must be declared by user. Elements set here.
     * @param[in]     p_cfg     Pointer to configuration structure. All elements of this structure  must be set by user.
     */
    ssp_err_t (* open)(jpeg_encode_ctrl_t        * const p_ctrl,
                       jpeg_encode_cfg_t const   * const   p_cfg);

    /** Set image parameters to JPEG Codec
     * @par Implemented as
     * - R_JPEG_Encode_ImageParameterSet()
     *
     * @pre The JPEG codec module must have been opened properly.
     *
     * @param[in,out] p_ctrl                 Pointer to control block.  Must be declared by user. Elements set here.
     * @param[in]     p_raw_image_parameters Pointer to the RAW image parameters
     */
    ssp_err_t (* imageParameterSet)(jpeg_encode_ctrl_t               * const p_ctrl,
                                    jpeg_encode_raw_image_parameters * p_raw_image_parameters);

    /** Assign output buffer to JPEG codec for storing output data.
     * @par Implemented as
     * - R_JPEG_Encode_OutputBufferSet()
     *
     * @pre The JPEG codec module must have been opened properly.
     * @note The buffer starting address must be 8-byte aligned.
     * @param[in]     p_ctrl        Control block set in jpeg_encode_api_t::open call.
     * @param[in]     p_buffer      Pointer to the output buffer space
     */
    ssp_err_t (* outputBufferSet)(jpeg_encode_ctrl_t        * const p_ctrl,
                                  void                      * p_buffer);

    /** Assign input data buffer to JPEG codec.
     * @par Implemented as
     * - R_JPEG_Encode_InputBufferSet()
     *
     * @pre the JPEG codec module must have been opened properly, output buffer and image parameter must be set prior
     * to call this function.
     * @note The buffer starting address must be 8-byte aligned.
     * @param[in]     p_ctrl      Control block set in jpeg_encode_api_t::open call.
     * @param[in]     p_buffer    Pointer to the input buffer space
     * @param[in]     buffer_size Size of the input buffer
     */
    ssp_err_t (* inputBufferSet)(jpeg_encode_ctrl_t                * const p_ctrl,
                                 void                              * p_buffer,
                                 uint32_t                            buffer_size);

    /** Retrieve current status of the JPEG codec module.
     * @par Implemented as
     * - R_JPEG_Encode_StatusGet()
     *
     * @pre the JPEG codec module must have been opened properly.
     * @param[in]     p_ctrl               Control block set in jpeg_encode_api_t::open call.
     * @param[out]    p_status             JPEG module status
     */
    ssp_err_t (* statusGet)(jpeg_encode_ctrl_t                 * const p_ctrl,
                            volatile jpeg_encode_status_t      * p_status);

    /** Cancel an outstanding operation.
     * @par Implemented as
     * - R_JPEG_Encode_Close()
     *
     * @pre the JPEG codec module must have been opened properly.
     * @param[in]     p_ctrl      Control block set in jpeg_encode_api_t::Open call.
     */
    ssp_err_t (* close)(jpeg_encode_ctrl_t        * const p_ctrl);

    /** Get version and store it in provided pointer p_version.
     * @par Implemented as
     * - R_JPEG_Encode_VersionGet()
     *
     * @param[out]  p_version  Code and API version used.
     */
    ssp_err_t (* versionGet)(ssp_version_t * p_version);

} jpeg_encode_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_jpeg_encode_instance
{
    jpeg_encode_ctrl_t      * p_ctrl;    ///< Pointer to the control structure for this instance
    jpeg_encode_cfg_t const * p_cfg;     ///< Pointer to the configuration structure for this instance
    jpeg_encode_api_t const * p_api;     ///< Pointer to the API structure for this instance
} jpeg_encode_instance_t;

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* DRV_JPEG_ENCODE_API_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup JPEG_ENCODE_API)
 **********************************************************************************************************************/
