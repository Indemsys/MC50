/***********************************************************************************************************************
 * Copyright [2015-2021] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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

#ifndef R_ADC_PRIVATE_API_H
#define R_ADC_PRIVATE_API_H

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Private Instance API Functions. DO NOT USE! Use functions through Interface API structure instead.
 **********************************************************************************************************************/
ssp_err_t R_ADC_Open(adc_ctrl_t * p_ctrl, adc_cfg_t const * const p_cfg);
ssp_err_t R_ADC_ScanConfigure(adc_ctrl_t * p_ctrl, adc_channel_cfg_t const * const p_channel_cfg);
ssp_err_t R_ADC_InfoGet(adc_ctrl_t * p_ctrl, adc_info_t * p_adc_info);
ssp_err_t R_ADC_ScanStart(adc_ctrl_t * p_ctrl);
ssp_err_t R_ADC_ScanStop(adc_ctrl_t * p_ctrl);
ssp_err_t R_ADC_CheckScanDone(adc_ctrl_t * p_ctrl);
ssp_err_t R_ADC_Read(adc_ctrl_t * p_ctrl, adc_register_t const reg_id, adc_data_size_t * const p_data);
ssp_err_t R_ADC_Read32(adc_ctrl_t * p_ctrl, adc_register_t const reg_id, uint32_t * const p_data);
ssp_err_t R_ADC_SetSampleStateCount(adc_ctrl_t * p_ctrl, adc_sample_state_t  * p_sample);
ssp_err_t R_ADC_Close(adc_ctrl_t * p_ctrl);
ssp_err_t R_ADC_OffsetSet(adc_ctrl_t * const p_api_ctrl, adc_register_t const reg_id, int32_t offset);
ssp_err_t R_ADC_Calibrate(adc_ctrl_t * const p_api_ctrl, void * const p_extend);
ssp_err_t R_ADC_VersionGet(ssp_version_t * const p_version);

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_ADC_PRIVATE_API_H */
