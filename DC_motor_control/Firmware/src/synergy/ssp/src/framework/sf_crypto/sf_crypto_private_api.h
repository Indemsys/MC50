/***********************************************************************************************************************
 * Copyright [2017-2021] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : sf_crypto_private_api.h
 * Description  : SSP Crypto Framework Common module private definition. Any functions defined in this file is not for
 *                user applications.
 **********************************************************************************************************************/

#ifndef SF_CRYPTO_PRIVATE_API_H
#define SF_CRYPTO_PRIVATE_API_H

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Private Instance API Functions. DO NOT USE! Use functions through Interface API structure instead.
 **********************************************************************************************************************/
ssp_err_t   SF_CRYPTO_Open (sf_crypto_ctrl_t * const p_api_ctrl, sf_crypto_cfg_t const * const p_cfg);

ssp_err_t   SF_CRYPTO_Close (sf_crypto_ctrl_t * const p_api_ctrl);

ssp_err_t   SF_CRYPTO_Lock (sf_crypto_ctrl_t * const p_api_ctrl);

ssp_err_t   SF_CRYPTO_Unlock (sf_crypto_ctrl_t * const p_api_ctrl);

ssp_err_t   SF_CRYPTO_StatusGet (sf_crypto_ctrl_t * const p_ctrl, sf_crypto_state_t * p_status);

ssp_err_t   SF_CRYPTO_VersionGet (ssp_version_t * const p_version);

/***********************************************************************************************************************
 * Crypto Framework common functions. DO NOT USE! These functions are not for users but for Crypto framework modules.
 **********************************************************************************************************************/
ssp_err_t   sf_crypto_memory_allocate ( TX_BYTE_POOL * p_byte_pool, void ** pp_memory, uint32_t memory_size,
                                                                                              uint32_t wait_option);

ssp_err_t   sf_crypto_memory_release (void * pp_memory, uint32_t memory_size);

void        sf_crypto_open_counter_increment (sf_crypto_ctrl_t * const p_api_ctrl);

void        sf_crypto_open_counter_decrement (sf_crypto_ctrl_t * const p_api_ctrl);

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_CRYPTO_PRIVATE_API_H */
