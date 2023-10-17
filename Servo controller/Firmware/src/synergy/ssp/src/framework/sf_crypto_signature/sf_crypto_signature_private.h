/***********************************************************************************************************************
 * Copyright [2018-2021] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
/***********************************************************************************************************************
 * File Name    : sf_crypto_signature_private.h
 * Description  : Crypto Signature Framework private APIs.
***********************************************************************************************************************/

#ifndef SF_CRYPTO_SIGNATURE_PRIVATE_H
#define SF_CRYPTO_SIGNATURE_PRIVATE_H
/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto_signature_api.h"
#include "sf_crypto_hash_api.h"
#include "sf_crypto_signature.h"
#include "ssp_common_api.h"
/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Instance API Functions. DO NOT USE! Use functions through Interface API structure instead.
 **********************************************************************************************************************/
ssp_err_t
SF_CRYPTO_SIGNATURE_Open(sf_crypto_signature_ctrl_t * const p_api_ctrl,
                         sf_crypto_signature_cfg_t const * const p_cfg);

ssp_err_t
SF_CRYPTO_SIGNATURE_Close(sf_crypto_signature_ctrl_t * const p_api_ctrl);

ssp_err_t
SF_CRYPTO_SIGNATURE_ContextInit(sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                sf_crypto_signature_mode_t operation_mode,
                                sf_crypto_signature_algorithm_init_params_t * const p_algorithm_specific_params,
                                sf_crypto_key_t const * const p_key);

ssp_err_t
SF_CRYPTO_SIGNATURE_SignUpdate(sf_crypto_signature_ctrl_t * const p_api_ctrl,
                               sf_crypto_data_handle_t const * const p_message);

ssp_err_t
SF_CRYPTO_SIGNATURE_VerifyUpdate(sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                 sf_crypto_data_handle_t const * const p_message);

ssp_err_t
SF_CRYPTO_SIGNATURE_SignFinal(sf_crypto_signature_ctrl_t * const p_api_ctrl,
                              sf_crypto_data_handle_t const * const p_message,
                              sf_crypto_data_handle_t * const p_dest);

ssp_err_t
SF_CRYPTO_SIGNATURE_VerifyFinal(sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                sf_crypto_data_handle_t const * const p_signature,
                                sf_crypto_data_handle_t const * const p_message);

ssp_err_t
SF_CRYPTO_SIGNATURE_VersionGet(ssp_version_t * const p_version);

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_CRYPTO_SIGNATURE_PRIVATE_H */