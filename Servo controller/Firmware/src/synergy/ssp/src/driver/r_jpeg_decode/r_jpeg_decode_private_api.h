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

#ifndef R_JPEG_DECODE_PRIVATE_API_H
#define R_JPEG_DECODE_PRIVATE_API_H

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Private Instance API Functions. DO NOT USE! Use functions through Interface API structure instead.
 **********************************************************************************************************************/
ssp_err_t R_JPEG_Decode_Open (jpeg_decode_ctrl_t * const p_ctrl, jpeg_decode_cfg_t const * const p_cfg);
ssp_err_t R_JPEG_Decode_OutputBufferSet (jpeg_decode_ctrl_t * p_ctrl, void * output_buffer,
                                         uint32_t output_buffer_size);
ssp_err_t R_JPEG_Decode_LinesDecodedGet (jpeg_decode_ctrl_t * const p_ctrl, uint32_t * const p_lines);
ssp_err_t R_JPEG_Decode_HorizontalStrideSet (jpeg_decode_ctrl_t * p_ctrl, uint32_t horizontal_stride);
ssp_err_t R_JPEG_Decode_InputBufferSet (jpeg_decode_ctrl_t * constp_ctrl,
                                        void               * p_data_buffer,
                                        uint32_t           data_buffer_size);
ssp_err_t R_JPEG_Decode_Close (jpeg_decode_ctrl_t * p_ctrl);
ssp_err_t R_JPEG_Decode_ImageSizeGet (jpeg_decode_ctrl_t * p_ctrl,
                                      uint16_t           * p_horizontal_size,
                                      uint16_t           * p_vertical_size);
ssp_err_t R_JPEG_Decode_StatusGet (jpeg_decode_ctrl_t * p_ctrl, jpeg_decode_status_t * p_status);
ssp_err_t R_JPEG_Decode_ImageSubsampleSet (jpeg_decode_ctrl_t * const p_ctrl,
                                           jpeg_decode_subsample_t    horizontal_subsample,
                                           jpeg_decode_subsample_t    vertical_subsample);
ssp_err_t R_JPEG_Decode_PixelFormatGet (jpeg_decode_ctrl_t * p_ctrl, jpeg_decode_color_space_t * p_color_space);
ssp_err_t R_JPEG_Decode_VersionGet (ssp_version_t * p_version);

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_JPEG_DECODE_PRIVATE_API_H */
