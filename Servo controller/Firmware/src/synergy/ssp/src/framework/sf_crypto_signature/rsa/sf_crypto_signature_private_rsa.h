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
 * File Name    : sf_crypto_signature_private_rsa.h
 * Description  : Crypto Signature Framework RSA specific private APIs.
***********************************************************************************************************************/

#ifndef SF_CRYPTO_SIGNATURE_RSA_PRIVATE_H
#define SF_CRYPTO_SIGNATURE_RSA_PRIVATE_H
/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto_signature_api.h"
#include "sf_crypto_signature.h"
#include "../../sf_crypto/sf_crypto_private_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define RSA_KEY_BITS_1024                                   (1024U)
#define RSA_KEY_BITS_2048                                   (2048U)
#define RSA_PLAIN_TEXT_1024_PRIVATE_KEY_SIZE_BYTES          RSA_PLAIN_TEXT_PRIVATE_KEY_SIZE_BYTES(RSA_KEY_BITS_1024)
#define RSA_PLAIN_TEXT_2048_PRIVATE_KEY_SIZE_BYTES          RSA_PLAIN_TEXT_PRIVATE_KEY_SIZE_BYTES(RSA_KEY_BITS_2048)
#define RSA_PLAIN_TEXT_1024_CRT_PRIVATE_KEY_SIZE_BYTES      RSA_PLAIN_TEXT_CRT_KEY_SIZE_BYTES(RSA_KEY_BITS_1024)
#define RSA_PLAIN_TEXT_2048_CRT_PRIVATE_KEY_SIZE_BYTES      RSA_PLAIN_TEXT_CRT_KEY_SIZE_BYTES(RSA_KEY_BITS_2048)
#define RSA_PLAIN_TEXT_1024_WRAPPED_PRIVATE_KEY_SIZE_BYTES  RSA_WRAPPPED_PRIVATE_KEY_SIZE_BYTES(RSA_KEY_BITS_1024)
#define RSA_PLAIN_TEXT_2048_WRAPPED_PRIVATE_KEY_SIZE_BYTES  RSA_WRAPPPED_PRIVATE_KEY_SIZE_BYTES(RSA_KEY_BITS_2048)
#define RSA_PLAIN_TEXT_1024_PUBLIC_KEY_SIZE_BYTES           RSA_PLAIN_TEXT_PUBLIC_KEY_SIZE_BYTES(RSA_KEY_BITS_1024)
#define RSA_PLAIN_TEXT_2048_PUBLIC_KEY_SIZE_BYTES           RSA_PLAIN_TEXT_PUBLIC_KEY_SIZE_BYTES(RSA_KEY_BITS_2048)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Instance API Functions. DO NOT USE!
 **********************************************************************************************************************/
ssp_err_t sf_crypto_signature_key_size_config_rsa(sf_crypto_signature_cfg_t const * const  p_cfg);

ssp_err_t sf_crypto_signature_open_rsa(sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                       sf_crypto_signature_cfg_t const * const p_cfg);
ssp_err_t sf_crypto_signature_close_rsa(sf_crypto_signature_instance_ctrl_t * p_ctrl);

ssp_err_t sf_crypto_signature_context_init_rsa(sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                               sf_crypto_signature_mode_t operation_mode,
                                               sf_crypto_signature_algorithm_init_params_t * const p_algorithm_specific_params,
                                               sf_crypto_key_t const * const p_key);

ssp_err_t sf_crypto_signature_sign_update_rsa(sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                              sf_crypto_data_handle_t const * const p_message);

ssp_err_t sf_crypto_signature_verify_update_rsa(sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                                sf_crypto_data_handle_t const * const p_message);

ssp_err_t sf_crypto_signature_sign_final_rsa(sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                             sf_crypto_data_handle_t const * const p_message,
                                             sf_crypto_data_handle_t * const p_dest);

ssp_err_t sf_crypto_signature_verify_final_rsa(sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                               sf_crypto_data_handle_t const * const p_signature,
                                               sf_crypto_data_handle_t const * const p_message);
/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_CRYPTO_SIGNATURE_RSA_PRIVATE_H */
