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
 * File Name    : sf_crypto_cipher_rsa_private.h
 * Description  : SSP Crypto Cipher Framework module private API definition
 **********************************************************************************************************************/

#ifndef SF_CRYPTO_CIPHER_RSA_PRIVATE_H
#define SF_CRYPTO_CIPHER_RSA_PRIVATE_H

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/** Cipher module internal context structure for RSA cipher operations */
typedef struct st_sf_crypto_cipher_rsa_context
{
    sf_crypto_cipher_op_mode_t          operation_mode;         ///< Encrypt / Decrypt operation.
    sf_crypto_cipher_padding_scheme_t   padding_scheme;         ///< Padding scheme used for the cipher operation.
    uint8_t                            *cipher_key;             ///< Key used for the encrypt / decrypt operation.
    uint32_t                            cipher_key_buffer_size; ///< key buffer size.
    uint32_t                            cipher_key_length;      ///< Length of the key used for the cipher operation.
    uint8_t                           * partial_block;          ///< Pointer to partial block.
    uint32_t                            partial_block_size;     ///< Partial block size.
    uint32_t                            partial_bytes_buffered; ///< Number of bytes in the partial block.
}sf_crypto_cipher_rsa_context_t;

/***********************************************************************************************************************
 * Private Functions - DO NOT USE!
 * These functions are for the private use of Crypto Cipher framework module.
 * RSA key type specific private functions.
  **********************************************************************************************************************/

void      sf_crypto_cipher_rsa_interface_get(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);

bool sf_crypto_cipher_is_key_type_rsa(sf_crypto_key_type_t key_type);




ssp_err_t sf_crypto_cipher_rsa_init(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                    sf_crypto_cipher_op_mode_t cipher_operation_mode,
                                    sf_crypto_key_t const * const p_key,
                                    sf_crypto_cipher_algorithm_init_params_t * p_algorithm_specific_params);

ssp_err_t sf_crypto_cipher_rsa_update(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                       sf_crypto_data_handle_t const * const p_data_in,
                                       sf_crypto_data_handle_t * const p_data_out);


ssp_err_t sf_crypto_cipher_rsa_final(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                            sf_crypto_data_handle_t const * const p_data_in,
                                            sf_crypto_data_handle_t * const p_data_out);

ssp_err_t sf_crypto_cipher_initialize_rsa_instance(sf_crypto_cipher_instance_ctrl_t * p_ctrl,
                                                   sf_crypto_cipher_cfg_t const * const p_cfg);

ssp_err_t sf_crypto_cipher_deinitialize_rsa_instance(sf_crypto_cipher_instance_ctrl_t * p_ctrl);

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_CRYPTO_CIPHER_RSA_PRIVATE_H */
