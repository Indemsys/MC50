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
 * File Name    : sf_crypto_signature_api.h
 * Description  : Interface definition for SSP Crypto Signature Framework.
 **********************************************************************************************************************/
#ifndef SF_CRYPTO_SIGNATURE_API_H
#define SF_CRYPTO_SIGNATURE_API_H
/*******************************************************************************************************************//**
 * @ingroup SF_Interface_Library
 * @defgroup SF_CRYPTO_SIGNATURE_API SSP Crypto Signature Framework Interface
 * @brief Interface definition for Synergy Crypto Signature Framework module.
 *
 * @section SF_CRYPTO_SIGNATURE_API_SUMMARY Summary
 * The Signature framework module is a ThreadX aware module which provides sign and sign-verify services.
 * They Key type and Key size provided in the configuration parameter determine the cryptography algorithm type
 * and uses the appropriate Driver API interface to provide requested functionality.
 * User can change the operation mode (Sign / Verify), message format and key data input multiple times after opening
 * this module using the Open API. There is no need to close the module using Close API and then re-open the module
 * if the intent is to just change operation mode, message format and/or key data input parameters.
 *
 * Crypto Signature Framework Interface description: @ref FrameworkCrypto
 *
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "sf_crypto_api.h"
#include "sf_crypto_hash_api.h"
#include "sf_crypto_key_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** The API version of SSP Crypto Signature Framework */
#define SF_CRYPTO_SIGNATURE_API_VERSION_MAJOR       (2U)
#define SF_CRYPTO_SIGNATURE_API_VERSION_MINOR       (0U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** Signature mode, sign or verify  */
typedef enum e_sf_crypto_signature_mode
{
    SF_CRYPTO_SIGNATURE_MODE_SIGN,                ///< Perform Sign Operation.
    SF_CRYPTO_SIGNATURE_MODE_VERIFY               ///< Perform Verify Operation.
} sf_crypto_signature_mode_t;

/** Signature message operation  */
typedef enum e_sf_crypto_signature_message_operation
{
    /** Input message is pre-formatted using appropriate format.
     *  Sign/verify operation is performed on the input message.*/
    SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_NONE,

    /** Generates a 20-byte (SHA-1) digest and signs/verifies the
     * digest using RSASSA-PKCS1 v1.5 padding scheme. */
    SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_RSA_SHA1_PKCS1_1_5,

    /** Generates a 28-byte (SHA-224) digest and signs/verifies the
     * digest using RSASSA-PKCS1 v1.5 padding scheme. */
    SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_RSA_SHA224_PKCS1_1_5,

    /** Generates a 32-byte (SHA-256) digest and signs/verifies the
     *  digest using RSASSA-PKCS1 v1.5 padding scheme. */
    SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_RSA_SHA256_PKCS1_1_5,
} sf_crypto_signature_message_operation_t;

/** RSA Algorithm specific parameters for signature operations  */
typedef struct st_sf_crypto_signature_rsa_specific_params
{
    /** Message format enumeration option. */
    sf_crypto_signature_message_operation_t         message_format;
} sf_crypto_signature_rsa_specific_params_t;

/** Algorithm specific parameters.  Allocate an algorithm specific block to pass into the contextInit API call.
 * @par Implemented as
 * - sf_crypto_signature_rsa_specific_params_t for RSA
 */
typedef void sf_crypto_signature_algorithm_init_params_t;

/** SSP Crypto Signature framework control block.  Allocate an instance specific control block to pass into the
 *  SSP Crypto Signature framework API calls.
 * @par Implemented as
 * - sf_crypto_signature_instance_ctrl_t
 */
typedef void sf_crypto_signature_ctrl_t;

/** Configuration structure for the SSP Crypto Signature framework */
typedef struct st_sf_crypto_signature_cfg
{
    /** Key Type. */
    sf_crypto_key_type_t                            key_type;

    /** Key Size. */
    sf_crypto_key_size_t                            key_size;

    /** Pointer to Hash framework instance. */
    sf_crypto_hash_instance_t                     * p_lower_lvl_sf_crypto_hash;

    /** Pointer to a Crypto Framework common instance. */
    sf_crypto_instance_t                          * p_lower_lvl_crypto_common;

    /** Extension parameter for hardware specific settings (Future purpose). */
    void const                                    * p_extend;
} sf_crypto_signature_cfg_t;

/** Shared Interface definition for the SSP Crypto Signature framework */
typedef struct st_sf_crypto_signature_api
{
    /** Open SSP Crypto Signature framework.
     * This function sets up a control block of the framework module based
     * on the configuration parameters such as the key type, key size and domain parameters.
     * The module allows users to have multiple instances with different control blocks, if required.
     * This API will allocate memory internally according to the cryptography algorithm selected through
     * key_type and key_size parameters.
     * @par Implemented as
     * - SF_CRYPTO_SIGNATURE_Open()
     * @param[in,out]  p_ctrl       Pointer to Crypto Signature Framework control block structure.
     * @param[in]      p_cfg        Pointer to sf_crypto_signature_cfg_t configuration structure.
     *                              All elements of this structure must be set by user.
     */
    ssp_err_t (* open)(sf_crypto_signature_ctrl_t * const p_ctrl,
                       sf_crypto_signature_cfg_t const * const p_cfg);

    /** Close SSP Crypto Signature framework module. This API will free any memory allocated when
     * the signature framework module was opened.
     * @par Implemented as
     * - SF_CRYPTO_SIGNATURE_Close()
     * @param[in,out]  p_ctrl       Pointer to Crypto Signature Framework control block structure.
     */
    ssp_err_t (* close)(sf_crypto_signature_ctrl_t * const p_ctrl);

    /** Perform Signature Module Context Initialization operation.
     *  This API initializes the signature module by setting operating mode, the message padding scheme and
     *  appropriate key for subsequent calls to signUpdate, verifyUpdate, signFinal and verifyFinal APIs.
     *  This API can be called only after this module has be opened using the open API.
     *  This API can be called after signFinal or verifyFinal API to initialize context for a new operation.
     *  This API sets up the internal context for sign/verify operation.
     * @par Implemented as
     * - SF_CRYPTO_SIGNATURE_ContextInit()
     *  @param[in]     p_ctrl                           Pointer to Crypto Signature Framework control block structure.
     *  @param[in]     operation_mode                   Selects Sign or Verify Operation enumeration.
     *  @param[in]     p_algorithm_specific_params      Algorithm specific parameters.
     *  @param[in]     p_key                            Pointer to a private key for Sign operation OR
     *                                                  Pointer to a public key for Verify operation.
     *
     *  @note                               p_key should be WORD aligned.
     */
    ssp_err_t (* contextInit)(sf_crypto_signature_ctrl_t * const p_ctrl,
                             sf_crypto_signature_mode_t operation_mode,
                             sf_crypto_signature_algorithm_init_params_t * const p_algorithm_specific_params,
                             sf_crypto_key_t const * const p_key);

    /** Perform Signature Module Signature-Update operation.
     * This API can be called multiple times to accumulate the message to be signed. This API can be used
     * when the input message to be signed is not available all at once in a byte array.
     * @par Implemented as
     * - SF_CRYPTO_SIGNATURE_SignUpdate()
     *  @param[in]     p_ctrl        Pointer to Crypto Signature Framework control block structure.
     *  @param[in]     p_message     Pointer to input message to be signed.
     *
     *  @note                        p_message should be WORD aligned.
     *  @note                        In case SF_CRYPTO_SIGNATURE_NO_PADDING is chosen as the padding scheme
     *                               ensure p_message is a valid message digest in appropriate format.
     */
    ssp_err_t (* signUpdate)(sf_crypto_signature_ctrl_t * const p_ctrl,
                             sf_crypto_data_handle_t const * const p_message);

    /** Perform Signature Module Signature-Verification-Update operation.
     * This API can be called multiple times to accumulate the message whose signature is to be verified.
     * This API can be used when the input message to be verified against a signature is not available
     *  all at once in a byte array.
     * @par Implemented as
     * - SF_CRYPTO_SIGNATURE_VerifyUpdate()
     *  @param[in]     p_ctrl        Pointer to Crypto Signature Framework control block structure.
     *  @param[in]     p_message     Pointer to message whose signature is to be verified.
     *
     *  @note                        p_message should be WORD aligned.
     */
    ssp_err_t (* verifyUpdate)(sf_crypto_signature_ctrl_t * const p_ctrl,
                               sf_crypto_data_handle_t const * const p_message);

    /** Perform Signature Module Signature-Final Operation.
     * Call to this API generates signature and writes it to p_dest. p_message can be the pointer to last block
     * of input message to be signed or can be passed as NULL if all of the input is passed through one or more
     * signUpdate API call(s).
     * @par Implemented as
     * - SF_CRYPTO_SIGNATURE_SignFinal()
     *  @param[in]     p_ctrl        Pointer to Crypto Signature Framework control block structure.
     *  @param[in]     p_message     Pointer to data handle containing last block of data and its length.
     *                               If there is no more data to be passed this param can be set to NULL.
     *
     *  @param[in,out] p_dest        Pointer to data handle containing pointer to a buffer for storing signature.
     *                               The data_length of this handle must be populated with the buffer length.
     *                               Upon successful return this data_length will be updated with the number
     *                               of bytes written to this buffer.
     *
     *  @note                        p_message should be WORD aligned.
     *  @note                        p_dest should be WORD aligned.
     *  @note                        p_message can be set to NULL.
     *  @note                        In case SF_CRYPTO_SIGNATURE_NO_PADDING is chosen as the padding scheme
     *                               ensure p_message is a valid message digest in appropriate format.
     */
    ssp_err_t (* signFinal)(sf_crypto_signature_ctrl_t * const p_ctrl,
                            sf_crypto_data_handle_t const * const p_message,
                            sf_crypto_data_handle_t * const p_dest);

    /** Perform Signature Module Signature-Verification-Final Operation.
     *  Call to this API performs signature verification operation. p_message can be the pointer to last block of
     *  message or can be passed as NULL if all of the message whose signature is to be verified
     *  is passed through one or more verifyUpdate API call(s).
     * @par Implemented as
     * - SF_CRYPTO_SIGNATURE_VerifyFinal()
     *  @param[in]     p_ctrl        Pointer to Crypto Signature Framework control block structure.
     *  @param[in]     p_signature   Pointer to Signature buffer to be verified.
     *  @param[in]     p_message     Pointer to last block of message whose signature is being verified.
     *                               If there is no more data to be passed this param can be set to NULL.
     *
     *  @note                        p_message should be WORD aligned.
     *  @note                        p_message can be set to NULL.
     */
    ssp_err_t (* verifyFinal)(sf_crypto_signature_ctrl_t * const p_ctrl,
                              sf_crypto_data_handle_t const * const p_signature,
                              sf_crypto_data_handle_t const * const p_message);

    /** Get version of SSP Crypto Signature framework.
    * @par Implemented as
    * - SF_CRYPTO_SIGNATURE_VersionGet()
    * @param[out]      p_version     Pointer to the memory to store the module version.
    */
   ssp_err_t (* versionGet)(ssp_version_t * const p_version);

} sf_crypto_signature_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_sf_crypto_signature_instance
{
    sf_crypto_signature_ctrl_t      * p_ctrl;    ///< Pointer to the control structure for this instance
    sf_crypto_signature_cfg_t       * p_cfg;     ///< Pointer to the configuration structure for this instance
    sf_crypto_signature_api_t const * p_api;     ///< Pointer to the API structure for this instance
} sf_crypto_signature_instance_t;

/** @} (end defgroup SF_CRYPTO_SIGNATURE_API) */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_CRYPTO_SIGNATURE_API_H */
