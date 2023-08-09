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
 * File Name    : sf_crypto_cipher_api.h
 * Description  : Interface definition for SSP Crypto Cipher Framework.
 **********************************************************************************************************************/

#ifndef SF_CRYPTO_CIPHER_API_H
#define SF_CRYPTO_CIPHER_API_H

/*******************************************************************************************************************//**
 * @ingroup SF_Interface_Library
 * @defgroup SF_CRYPTO_CIPHER_API SSP Crypto Cipher Framework Interface
 * @brief Interface definition for Synergy Crypto Cipher Framework module.
 *
 * @section SF_CRYPTO_CIPHER_API_SUMMARY Summary
 * This is a ThreadX aware Interface of SF_CRYPTO_CIPHER Framework module which provides encryption and decryption
 * operations for AES and RSA algorithms.
 *
 * Crypto Cipher Framework Interface description: @ref FrameworkCrypto
 *
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "sf_crypto_api.h"
#include "sf_crypto_key_api.h"
#include "sf_crypto_trng_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** AES modes for the SSP Crypto Cipher Framework */
typedef enum e_sf_crypto_cipher_mode
{
    SF_CRYPTO_CIPHER_MODE_ECB,              ///< Electronic Code Book chaining mode, default for RSA
    SF_CRYPTO_CIPHER_MODE_CBC,              ///< Cipher Block Chaining.
    SF_CRYPTO_CIPHER_MODE_CTR,              ///< Counter Mode.
    SF_CRYPTO_CIPHER_MODE_XTS,              ///< XEX-based tweaked-codebook mode with ciphertext stealing
    SF_CRYPTO_CIPHER_MODE_GCM,              ///< Galois Counter Mode.
} sf_crypto_cipher_mode_t;

/** Operating mode for Cipher APIs */
typedef enum e_sf_crypto_cipher_op_mode
{
    SF_CRYPTO_CIPHER_OP_MODE_ENCRYPT,       ///< The operating mode is set to encryption.
    SF_CRYPTO_CIPHER_OP_MODE_DECRYPT,       ///< The operating mode is set to decryption.
} sf_crypto_cipher_op_mode_t;

/** Padding mode to be used for Cipher operation - encrypting/ decrypting input data   */
typedef enum e_sf_crypto_cipher_padding_scheme
{
    SF_CRYPTO_CIPHER_PADDING_SCHEME_NO_PADDING,     ///< No padding scheme
    SF_CRYPTO_CIPHER_PADDING_SCHEME_PKCS7,          ///< PKCS#7 padding scheme - applicable only for AES operations.
    SF_CRYPTO_CIPHER_PADDING_SCHEME_PKCS1_1_5,      ///< PKCS#1 v1.5 padding scheme - applicable only for RSA operations.
} sf_crypto_cipher_padding_scheme_t;

/** AES Algorithm specific parameters for cipher operations  */
typedef struct st_sf_crypto_cipher_aes_init_params
{
    sf_crypto_cipher_padding_scheme_t       padding_scheme;
    sf_crypto_data_handle_t               * p_iv;               ///< pointer to IV for the AES operation.
    sf_crypto_data_handle_t               * p_auth_tag;         ///< Pointer to the GCM Authentication Tag.
                                                                ///< Only tag length of SF_CRYPTO_CIPHER_AES_GCM_TAG_LENGTH_16_BYTES is supported.
} sf_crypto_cipher_aes_init_params_t;

/** RSA Algorithm specific parameters for cipher operations  */
typedef struct st_sf_crypto_cipher_rsa_init_params
{
    sf_crypto_cipher_padding_scheme_t   padding_scheme;
} sf_crypto_cipher_rsa_init_params_t;


/** Algorithm specific parameters.  Allocate an algorithm specific block to pass into the cipherInit API call.
 * @par Implemented as
 * - sf_crypto_cipher_aes_init_params_t for AES
 * - sf_crypto_cipher_rsa_init_params_t for RSA.
 */
typedef void sf_crypto_cipher_algorithm_init_params_t;

/** SSP Crypto Cipher framework control block.
 * Allocate an instance specific control block to pass into the SSP Crypto framework Cipher API calls.
 * @par Implemented as
 * - sf_crypto_cipher_instance_ctrl_t
 */
typedef void sf_crypto_cipher_ctrl_t;

/** Configuration structure for the SSP  Crypto Cipher framework 
 * Cipher chaining mode for RSA operations is not applicable and can be set to ECB
*/
typedef struct st_sf_crypto_cipher_cfg
{
    sf_crypto_key_type_t            key_type;                       ///< Key type for cipher operation.
    sf_crypto_key_size_t            key_size;                       ///< Key size for cipher operation.
    sf_crypto_cipher_mode_t         cipher_chaining_mode;           ///< Chaining mode specified for the cipher operation.
    sf_crypto_instance_t          * p_lower_lvl_crypto_common;      ///< Pointer to a Crypto Framework common instance.
    sf_crypto_trng_instance_t     * p_lower_lvl_crypto_trng;        ///< Pointer to a Crypto Framework TRNG instance.
    void const                    * p_extend;                       ///< Future extension for hardware specific settings.
} sf_crypto_cipher_cfg_t;

/** Shared Interface definition for the SSP Crypto Cipher framework module */
typedef struct st_sf_crypto_cipher_api
{
    /** @brief Opens SSP Crypto Cipher framework. This function initializes a control block of the framework module based
     *  on the configuration parameters such as the key type, key size and chaining mode. The module allows users to
     *  have multiple instances with different control blocks, if required.
     * @par Implemented as
     * - SF_CRYPTO_CIPHER_Open()
     * @param[in,out]  p_ctrl       Pointer to Crypto Cipher Framework control block structure.
     *                              Caller only needs to allocate sf_crypto_cipher_instance_ctrl_t and
     *                              not fill any parameters.
     * @param[in]      p_cfg        Pointer to sf_crypto_cipher_cfg_t configuration structure.
     *                              All elements of this structure must be filled by caller.
     */
    ssp_err_t (* open)(sf_crypto_cipher_ctrl_t * const p_ctrl,
                       sf_crypto_cipher_cfg_t const * const p_cfg);

    /** @brief Initializes a  cipher operation. Must be called after open() or cipherFinal() is called, to initialize a
     *  new cipher operation. Unless a different key type or key size or chaining mode is used, users do not need
     *  to close the module for a new cipher operation but can call this function to restart another cipher operation.
     * @par Implemented as
     * - SF_CRYPTO_CIPHER_CipherInit()
     *
     *  @param[in, out] p_ctrl                      Pointer to Crypto Cipher Framework control block structure.
     *  @param[in]      cipher_operation_mode       Specifies encrypt or decrypt operation.
     *  @param[in]      p_key                       The key to be used for the cipher operation.
     *  @param[in]      p_algorithm_specific_params Algorithm specific parameters.
     *                                              Allocate and fill parameters specific to the algorithm for the
     *                                              key type configured at open().
     */
    ssp_err_t (* cipherInit)(sf_crypto_cipher_ctrl_t * const p_ctrl,
                             sf_crypto_cipher_op_mode_t cipher_operation_mode,
                             sf_crypto_key_t const * const p_key,
                             sf_crypto_cipher_algorithm_init_params_t * const p_algorithm_specific_params);

    /** @brief Encrypts / decrypts input data and writes it to the output buffer. Can be called multiple times for additional
     *  blocks of data.
     *  If input length is 0 this method does nothing.
     *  There may be 0 to (input length+block size - 1) bytes of data for AES operations.
     *  For RSA operation there will be no output until cipherFinal() is called.
     *  RSA Encryption is only supported with the RSA Public Key.
     *  RSA Decryption is only supported with the RSA Private Key.
     * @par Implemented as
     * - SF_CRYPTO_CIPHER_CipherUpdate()
     *
     *  @param[in, out] p_ctrl      Pointer to Crypto Cipher Framework control block structure.
     *  @param[in]      p_data_in   Pointer to an input data buffer and the input data length.
     *  @param[in, out] p_data_out  Pointer to an output data buffer and the buffer size on input.
     *                              If there is data to be output, buffer is filled and the length is updated.
     *
     *  @note Data buffers must be WORD aligned.
     */
    ssp_err_t (* cipherUpdate)(sf_crypto_cipher_ctrl_t * const p_ctrl,
                               sf_crypto_data_handle_t const * const p_data_in,
                               sf_crypto_data_handle_t * const p_data_out);

    /** @brief Encrypts/decrypts all/last block of data and writes to the output buffer. Once cipherFinal() is called,
     *  no additional call of cipherUpdate() is allowed but cipherInit() can be called to initialize a new cipher
     *  operation unless another key type / key size/chaining mode is needed. In such a case a call to
     *  close() and open() is required.
     *  For AES operations, the number of bytes output into output data buffer may be larger or
     *  smaller than input length or even 0.
     *  RSA Encryption is only supported with the RSA Public Key.
     *  RSA Decryption is only supported with the RSA Private Key.
     * @par Implemented as
     * - SF_CRYPTO_CIPHER_CipherFinal()
     *
     *  @param[in, out] p_ctrl      Pointer to Crypto Cipher Framework control block structure.
     *  @param[in]      p_data_in   Pointer to an input data buffer and the input data length.
     *  @param[in, out] p_data_out  Pointer to the output data buffer and the buffer size on input.
     *                              If there is data to be output, buffer is filled and the length is updated.
     *
     *   @note Data buffers must be WORD aligned.
     */
    ssp_err_t (* cipherFinal)(sf_crypto_cipher_ctrl_t * const p_ctrl,
                              sf_crypto_data_handle_t const * const p_data_in,
                              sf_crypto_data_handle_t * const p_data_out);

    /** @brief Updates AAD (Additional Authenticated Data) for AES GCM operation.
     * Can be called multiple times for additional blocks of data.
     * This is ONLY to provide AAD for AES GCM operation. Not applicable to any other algorithms or modes.
     * This has to be called prior to processing any plain text / cipher text data. In other words, before any call to
     * cipherUpdate() or cipherFinal() is made.
     * @par Implemented as
     * - SF_CRYPTO_CIPHER_CipherAadUpdate()
     *
     *  @param[in]      p_ctrl      Pointer to Crypto Cipher Framework control block structure.
     *  @param[in]      p_aad       Pointer to an input data buffer containing AAD and the AAD length.
     *
     *  @note Data buffer must be WORD aligned.
     */
    ssp_err_t (* cipherAadUpdate)(sf_crypto_cipher_ctrl_t * const p_ctrl,
                                  sf_crypto_data_handle_t const * const p_aad);

    /** @brief Closes SSP Crypto Cipher framework. This function resets a control block of the framework module and
      *  allows users to re-configure the module differently. For instance, users can close the module and re-open it
      *  with different key type or key size or chaining mode / algorithm for a new cipher operation.
      * @par Implemented as
      * - SF_CRYPTO_CIPHER_Close()
      *
      * @param[in,out]  p_ctrl       Pointer to Crypto Cipher Framework control block structure.
      */
     ssp_err_t (* close)(sf_crypto_cipher_ctrl_t * const p_ctrl);

    /** @brief Get version of SSP Crypto Cipher framework.
    * @par Implemented as
    * - SF_CRYPTO_CIPHER_VersionGet()
    *
    * @param[in]      p_version     Pointer to the memory to store the module version.
    */
   ssp_err_t (* versionGet)(ssp_version_t * const p_version);

} sf_crypto_cipher_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_sf_crypto_cipher_instance
{
    sf_crypto_cipher_ctrl_t      * p_ctrl;    ///< Pointer to the control structure for this instance
    sf_crypto_cipher_cfg_t const * p_cfg;     ///< Pointer to the configuration structure for this instance
    sf_crypto_cipher_api_t const * p_api;     ///< Pointer to the API structure for this instance
} sf_crypto_cipher_instance_t;

/** @} (end defgroup SF_CRYPTO_CIPHER_API) */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_CRYPTO_CIPHER_API_H */
