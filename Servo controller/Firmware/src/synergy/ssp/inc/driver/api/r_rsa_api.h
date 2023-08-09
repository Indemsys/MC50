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
 * File Name    : r_rsa_api.h
 * Description  : RSA_Interface
 ***********************************************************************************************************************
 * History : DD.MM.YYYY Version Description
 *           29.01.2015 RSA_Interface  1.00    Initial Release.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Crypto_API
 * @defgroup RSA_API RSA Interface
 *
 * @brief RSA cryptographic functions for signature generation, verification, encryption and decryption
 *
 * @{
 **********************************************************************************************************************/

#ifndef DRV_RSA_API_H
#define DRV_RSA_API_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
/** Register definitions, common services and error codes. */
#include "bsp_api.h"
#include "r_crypto_api.h"
#include "r_rsa_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define RSA_API_VERSION_MAJOR (1U)
#define RSA_API_VERSION_MINOR (1U)

/** Return RSA modulus size in bytes from the specified RSA modulus size in bits */
#define RSA_MODULUS_SIZE_BYTES(RSA_SIZE_BITS)                    ((RSA_SIZE_BITS)/8U)

/** Return RSA public key size in bytes from the specified RSA modulus size in bits */
#define RSA_PLAIN_TEXT_PUBLIC_KEY_SIZE_BYTES(RSA_SIZE_BITS)      (((uint32_t)32+(uint32_t)RSA_SIZE_BITS)/8U)

/** Return RSA private key size in bytes from the specified RSA modulus size in bits */
#define RSA_PLAIN_TEXT_PRIVATE_KEY_SIZE_BYTES(RSA_SIZE_BITS)     (((uint32_t)2*(uint32_t)RSA_SIZE_BITS)/8U)

/** Return RSA CRT private key size in bytes from the specified RSA modulus size in bits */
#define RSA_PLAIN_TEXT_CRT_KEY_SIZE_BYTES(RSA_SIZE_BITS)         (((uint32_t)5*((uint32_t)RSA_SIZE_BITS))/16U)

/** Return RSA wrapped private key size in bytes from the specified RSA modulus size in bits */
#define RSA_WRAPPPED_PRIVATE_KEY_SIZE_BYTES(RSA_SIZE_BITS)       ((((uint32_t)2 * (uint32_t)RSA_SIZE_BITS)+(uint32_t)160)/8U)

/** Return RSA wrapped private CRT key size in bytes from the specified RSA modulus size in bits */
#define RSA_WRAPPPED_PRIVATE_CRT_KEY_SIZE_BYTES(RSA_SIZE_BITS)   ((RSA_PLAIN_TEXT_CRT_KEY_SIZE_BYTES(RSA_SIZE_BITS))+20U)

/** RSA key format definitions */
typedef enum e_rsa_key_format
{
    RSA_KEY_FORMAT_PLAIN_TEXT_PUBLIC_KEY,    //!< RSA public key in plain text format
    RSA_KEY_FORMAT_PLAIN_TEXT_PRIVATE_KEY,   //!< RSA private key in plain text format
    RSA_KEY_FORMAT_PLAIN_TEXT_CRT_KEY,       //!< RSA CRT Key in plain text format
    RSA_KEY_FORMAT_WRAPPED_PRIVATE_KEY,       //!< RSA private Key wrapped using device specific key
    RSA_KEY_FORMAT_WRAPPED_PRIVATE_CRT_KEY    //!< RSA private CRT Key wrapped using device specific key
} rsa_key_format_t;

/** RSA key data structure */
typedef struct st_rsa_key
{
    rsa_key_format_t key_format;       //!< Indicates if the key is in plain-text format or encrypted using device unique key
    uint32_t         length;           //!< Length in bytes of the p_data buffer
    uint8_t        * p_data;           //!< Buffer where the private key is stored
} rsa_key_t;

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** RSA Interface control structure */
typedef struct st_rsa_ctrl
{
    crypto_ctrl_t       * p_crypto_ctrl;    //!< pointer to crypto engine control structure
    crypto_api_t const  * p_crypto_api;     //!< pointer to crypto engine API
    uint32_t              stage_num;        //!< processing stage
} rsa_ctrl_t;

/** RSA Interface configuration structure. User must fill in these values before invoking the open() function */
typedef struct st_rsa_cfg
{
    crypto_api_t const  * p_crypto_api; //!< pointer to crypto engine api
} rsa_cfg_t;

/** RSA_Interface SCE functions implemented at the HAL layer will follow this API. */
typedef struct st_rsa_api
{
    /** RSA module open function. Must be called before performing any encrypt/decrypt or
     *  sign/verify operations.
     *
     * @param[in,out] p_ctrl pointer to control structure for the RSA interface. Must be declared by user.
     *                       Elements are set here.
     * @param[in]     p_cfg  pointer to control structure for the RSA configuration. All elements of this structure
     *                       must be set by user.
     *
     */
    uint32_t (* open)(rsa_ctrl_t * const p_ctrl, rsa_cfg_t const * const p_cfg);

    /** @brief Encrypt source data from `p_source` using an RSA public key from `p_key` and write the results
     *  to destination buffer `p_dest`.
     *
     * @param[in]  *p_ctrl     pointer to control structure for RSA interface
     * @param[in]  *p_key      pointer to the RSA plain-text public key consisting of 32-bit public exponent
     *                         and RSA public modulus of size either 1024-bits or 2048-bits.
     * @param[in]  *p_domain   unused parameter for RSA encryption. NULL value is acceptable.
     * @param[in]   num_words  data buffer size in words. Each word is 4-bytes.
     * @param[in]  *p_source   source data buffer to be encrypted.
     * @param[out] *p_dest     destination data buffer, encryption result will be stored here.
     */
    uint32_t (* encrypt)(rsa_ctrl_t * const p_ctrl, const uint32_t * p_key, const uint32_t * p_domain, uint32_t num_words,
                         uint32_t * p_source, uint32_t * p_dest);

    /** @brief Decrypt source data from `p_source` using an RSA private key from `p_key` and write the results
     *  to destination buffer `p_dest`. The RSA private key data `p_key` is specified in the standard format that consists
     *  of private exponent and the RSA modulus. The size of the private exponent and the RSA modulus is 1024-bits for
     *  the `g_rsa1024_on_sce` implementation and 2048-bits for the `g_rsa2048_on_sce` implementation.
     *
     * @param[in]  *p_ctrl     pointer to control structure for RSA interface
     * @param[in]  *p_key      pointer to RSA plain-text private key consisting of private exponent and the RSA modulus.
     * @param[in]  *p_domain   unused parameter for RSA decryption. NULL value is acceptable.
     * @param[in]  num_words   data buffer size in words. Each word is 4-bytes.
     * @param[in]  *p_source   input data buffer to be decrypted.
     * @param[out] *p_dest     output destination data buffer, decryption result will be stored here.
     */
    uint32_t (* decrypt)(rsa_ctrl_t * const p_ctrl, const uint32_t * p_key, const uint32_t * p_domain, uint32_t num_words,
                         uint32_t * p_source, uint32_t * p_dest);

    /** @brief Decrypt source data from `p_source` using an RSA private key from `p_key` and write the results
     *  to destination buffer `p_dest`. RSA private key data is specified in CRT format. The RSA CRT key consists
     *  of the exponent2 || prime2 || exponent1 || prime1 || coefficient, starting with exponent2 at index 0.
     *  The size of each of these parameter is 512-bits for the `g_rsa1024_on_sce` implementation
     *  and 1024-bits for the `g_rsa2048_on_sce` implementation.
     *
     * @param[in]  *p_key      pointer to RSA private key in CRT format.
     * @param[in]  *p_domain   unused parameter for RSA decryption. NULL value is acceptable.
     * @param[in]   num_words  data buffer size in words. Each word is 4-bytes.
     * @param[in]  *p_source   input data buffer to be decrypted.
     * @param[out] *p_dest     output destination data buffer, decryption result will be stored here.
     */
    uint32_t (* decryptCrt)(rsa_ctrl_t * const p_ctrl, const uint32_t * p_key, const uint32_t * p_domain, uint32_t num_words,
                            uint32_t * p_source, uint32_t * p_dest);

    /** @brief Verify signature given in buffer `p_signature` using the RSA public key `p_key` for the given
     *  padded message hash from buffer `p_padded_hash`.
     *
     * @param[in] *p_key        pointer to the RSA plain-text public key consisting of 32-bit public exponent
     *                          and RSA public modulus of size either 1024-bits or 2048-bits.
     * @param[in] *p_domain     unused parameter. NULL value is acceptable.
     * @param[in] num_words     data buffer size in words. Each word is 4-bytes.
     * @param[in] *p_signature  signature data that needs to be verified
     * @param[in] *p_paddedHash padded hash value of the input message buffer
     */
    uint32_t (* verify)(rsa_ctrl_t * const p_ctrl, const uint32_t * p_key, const uint32_t * p_domain, uint32_t num_words,
                        uint32_t * p_signature, uint32_t * p_padded_hash);

    /** @brief Generate signature for the given padded hash buffer `p_padded_hash` using the RSA private key
     *  `p_key`. Write the results to the buffer `p_dest`.
     *
     * @param[in] *p_ctrl           pointer to control structure for RSA interface
     * @param[in]  *p_key           pointer to RSA private key consisting of private exponent and the RSA modulus.
     * @param[in]  *p_domain        unused parameter. NULL value is acceptable.
     * @param[in]  num_words        data buffer size in words. Each word is 4-bytes. multiples of 4
     * @param[in]  *p_padded_hash   padded hash for the input message for which an RSA signature is desired
     * @param[out] *p_dest          generated signature data will be written here.
     */
    uint32_t (* sign)(rsa_ctrl_t * const p_ctrl, const uint32_t * p_key, const uint32_t * p_domain, uint32_t num_words,
                      uint32_t * p_padded_hash, uint32_t * p_dest);

    /** @brief Generate signature for the given padded hash buffer `p_padded_hash` using the RSA private key
     *  `p_key`. RSA private key `p_key` is assumed to be in CRT format. Write the results to the buffer `p_dest`.
     *  The RSA CRT key consists of the exponent2 || prime2 || exponent1 || prime1 || coefficient,
     *  starting with exponent2 at index 0.
     *  The size of each of these parameter is 512-bits for the `g_rsa1024_on_sce` implementation
     *  and 1024-bits for the `g_rsa2048_on_sce` implementation.
     *
     * @param[in] *p_ctrl           pointer to control structure for RSA interface
     * @param[in]  *p_key           pointer to RSA private key in CRT format.
     * @param[in]  *p_domain        unused parameter. NULL value is acceptable.
     * @param[in]  num_words        data buffer size in words. Each word is 4-bytes. multiples of 4
     * @param[in]  *p_padded_hash   padded hash for the input message for which an RSA signature is desired
     * @param[out] *p_dest          generated signature data will be written here.
     */
    uint32_t (* signCrt)(rsa_ctrl_t * const p_ctrl, const uint32_t * p_key, const uint32_t * p_domain, uint32_t num_words,
                         uint32_t * p_padded_hash, uint32_t * p_dest);

    /** Close the RSA module.
     *  @param[in] p_ctrl pointer to the control structure
     */
    uint32_t (* close)(rsa_ctrl_t * const p_ctrl);

    /** Gets version and stores it in provided pointer p_version.
     * @param[out]    p_version  Code and API version used.
     */
    uint32_t (* versionGet)(ssp_version_t * const p_version);

    /** Generates an RSA key. This is a blocking call
     *  @param[in]       *p_ctrl        pointer to control structure for RSA interface
     *  @param[in,out]   *p_private_key pointer to a private key structure
     *  @param[in,out]   *p_public_key  pointer to a public key structure
     *
     *  \code
     *  {
     *    // The following code snippet gives an example for generating and using a 1024-bit RSA key.
     *    // For simplicity, the below code snippet does not check return values.
     *
     *       rsa_ctrl_t rsa_ctrl;
     *       rsa_cfg_t  rsa_cfg;
     *       rsa_key_t  rsa_secret_key;
     *       rsa_key_t  rsa_public_key;
     *       uint8_t    rsa_secret_key_data[RSA_PLAIN_TEXT_PRIVATE_KEY_SIZE_BYTES(1024)];
     *       uint8_t    rsa_public_key_data[RSA_PLAIN_TEXT_PUBLIC_KEY_SIZE_BYTES(1024)];
     *
     *       // This example shows generation of an RSA private key in standard format.
     *       // To generate a CRT key, the key_format field should be set to RSA_KEY_FORMAT_PLAIN_TEXT_CRT_KEY
     *       // and define rsa_secret_key_data buffer to be of size RSA_KEY_FORMAT_PLAIN_TEXT_CRT_KEY
     *       rsa_secret_key.key_format = RSA_KEY_FORMAT_PLAIN_TEXT_PRIVATE_KEY;
     *       rsa_secret_key.length     = sizeof(rsa_secret_key_data);
     *       rsa_secret_key.p_data     = rsa_secret_key_data;
     *
     *       rsa_public_key.key_format = RSA_KEY_FORMAT_PLAIN_TEXT_PUBLIC_KEY;
     *       rsa_public_key.length     = sizeof(rsa_public_key_data);
     *       rsa_publi_key.p_data      = rsa_public_key_data;
     *
     *       g_rsa1024_on_sce.open(&rsa_ctrl, &rsa_cfg);
     *
     *       g_rsa1024_on_sce.keyCreate(p_ctrl, &rsa_secret_key, &rsa_public_key);
     *
     *       // p_source is a pointer to a padded hash data. The computed signature will be stored at p_dest
     *       // The example below uses an RSA private key in standard format.
     *       // To compute signature using an RSA CRT private key, use the signCrt() interface function.
     *       g_rsa1024_on_sce.sign(p_ctrl, num_words, rsa_secret_key.p_data, p_source, p_dest)
     *
     *       g_rsa1024_on_sce.verify(p_ctrl, num_words, rsa_public_key.p_data, p_dest, p_source);
     *  }
     *  \endcode
     */
    uint32_t (* keyCreate)(rsa_ctrl_t * const p_ctrl, rsa_key_t * p_private_key, rsa_key_t * p_public_key);

} rsa_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_rsa_instance {
    rsa_ctrl_t   * p_ctrl ;         ///< Pointer to the control structure for this instance
    rsa_cfg_t    const * p_cfg  ;   ///< Pointer to the configuration structure for this instance
    rsa_api_t    const * p_api  ;   ///< Pointer to the API structure for this instance
} rsa_instance_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
extern const crypto_api_t g_sce_crypto_api;

/** RSA interface is only available on S7G2, S5D9 and S5D5. */
#if defined(BSP_MCU_GROUP_S7G2) || defined(BSP_MCU_GROUP_S5D9) || defined(BSP_MCU_GROUP_S5D5) || defined(BSP_MCU_GROUP_S5D3)
extern const rsa_api_t    g_rsa1024_on_sce;
extern const rsa_api_t    g_rsa2048_on_sce;
extern const rsa_api_t    g_rsa1024_on_sce_hrk;
extern const rsa_api_t    g_rsa2048_on_sce_hrk;
#endif /* defined(BSP_MCU_GROUP_S7G2) || defined(BSP_MCU_GROUP_S5D9) || defined(BSP_MCU_GROUP_S5D5) || defined(BSP_MCU_GROUP_S5D3) */


/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* DRV_RSA_API_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup RSA_API)
 **********************************************************************************************************************/
