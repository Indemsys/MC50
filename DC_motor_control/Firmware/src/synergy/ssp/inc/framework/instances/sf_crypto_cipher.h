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
 * File Name    : sf_crypto_cipher.h
 * Description  : Header file for SSP Crypto Cipher Framework module instance.
 **********************************************************************************************************************/

#ifndef SF_CRYPTO_CIPHER_H
#define SF_CRYPTO_CIPHER_H

/*******************************************************************************************************************//**
 * @ingroup SF_Library
 * @defgroup SF_CRYPTO_CIPHER SSP Crypto Cipher Framework
 * @brief RTOS-integrated Crypto Cipher Framework Module.
 *
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto_cipher_api.h"
#include "sf_crypto_cipher_cfg.h"
#include "sf_crypto.h"
#include "sf_crypto_trng.h"
#include "r_rsa_api.h"
#include "r_aes_api.h"
#include "r_crypto_api.h"


/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** The API version of SSP Crypto Cipher Framework */
#define SF_CRYPTO_CIPHER_CODE_VERSION_MAJOR       (2U)
#define SF_CRYPTO_CIPHER_CODE_VERSION_MINOR       (0U)


#define NUM_BYTES_IN_WORD (4U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** The API version of SSP Crypto CIPHER Framework */
#define SF_CRYPTO_CIPHER_API_VERSION_MAJOR          (1U)
#define SF_CRYPTO_CIPHER_API_VERSION_MINOR          (0U)

/** Macros for AES operations */
#define SF_CRYPTO_CIPHER_AES_128_XTS_KEY_SIZE           (2 * (AES128_SECRET_KEY_SIZE_BYTES))
#define SF_CRYPTO_CIPHER_AES_256_XTS_KEY_SIZE           (2 * (AES256_SECRET_KEY_SIZE_BYTES))
#define SF_CRYPTO_CIPHER_AES_GCM_TAG_LENGTH_16_BYTES    (16U)   ///< AES GCM tag of length 16 bytes.
#define SF_CRYPTO_CIPHER_AES_IV_LENGTH_12_BYTES         (12U)   ///< IV for AES operations - 16 bytes.
#define SF_CRYPTO_CIPHER_AES_GCM_IV_PAD_4_BYTES         (4U)    ///< 4 byte padding for 96-bit IV.
#define SF_CRYPTO_CIPHER_AES_IV_LENGTH_16_BYTES         (16U)   ///< IV for AES operations - 16 bytes.
#define SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES           (16U)   ///< AES block size = 16 bytes
#define SF_CRYPTO_CIPHER_BYTES_PER_WORD                 (4U)    ///< number of bytes in a WORD.
#define SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_IN_WORDS    (SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES/SF_CRYPTO_CIPHER_BYTES_PER_WORD)

/** Macros for RSA operations */
/** EB = ENCRYPTION_BLOCK
 * PS = Padding String
 * PKCS_1_5 = RSAES-PKCS1-v1_5
 */
#define SF_CRYPTO_CIPHER_RSA_1024_MODULUS_BITS      (1024U) ///< Modulus size of RSA 1024-bit key
#define SF_CRYPTO_CIPHER_RSA_2048_MODULUS_BITS      (2048U) ///< Modulus size of RSA 2048-bit key
#define SF_CRYPTO_PKCS_1_5_EB_START_BYTE            (0U)    ///< Encryption Block start byte = 00
#define SF_CRYPTO_PKCS_1_5_BT_00                    (0U)    ///< Encryption Block Type (BT) = 00
#define SF_CRYPTO_PKCS_1_5_BT_01                    (1U)    ///< Encryption Block Type (BT) = 01
#define SF_CRYPTO_PKCS_1_5_BT_02                    (2U)    ///< Encryption Block Type (BT) = 02
#define SF_CRYPTO_PKCS_1_5_EB_DATA_SEPARATOR        (0U)    ///< EB Data separator (between PS and Data).

#define SF_CRYPTO_PKCS_1_5_EB_START_BYTE_LENGTH     (1U)    ///< Encryption Block Start Byte length
#define SF_CRYPTO_PKCS_1_5_EB_BT_BYTE_LENGTH        (1U)    ///< Encryption Block Block Type field  length
#define SF_CRYPTO_PKCS_1_5_EB_PS_MIN_LENGTH         (8U)    ///< Encryption Block Padding String (PS) min length.
#define SF_CRYPTO_PKCS_1_5_EB_DATA_SEPARATOR_LENGTH (1U)    ///< EB Data separator (between PS and Data)length.
/** Overhead for formatting the Encryption Block, in number of bytes */
#define SF_CRYPTO_PKCS_1_5_EB_OVERHEAD  (SF_CRYPTO_PKCS_1_5_EB_START_BYTE_LENGTH +  \
                                         SF_CRYPTO_PKCS_1_5_EB_BT_BYTE_LENGTH +     \
                                         SF_CRYPTO_PKCS_1_5_EB_PS_MIN_LENGTH  +     \
                                         SF_CRYPTO_PKCS_1_5_EB_DATA_SEPARATOR_LENGTH)

/** States the SSP Crypto Cipher Framework module can go through. */
typedef enum e_sf_crypto_cipher_state
{
    SF_CRYPTO_CIPHER_STATE_CLOSED,          ///< The Cipher module is closed.
    SF_CRYPTO_CIPHER_STATE_OPENED,          ///< The Cipher module is opened.
    SF_CRYPTO_CIPHER_STATE_INITIALIZED,     ///< The cipher operation is initialized.
    SF_CRYPTO_CIPHER_STATE_UPDATED,         ///< The cipher operation is updated.
    SF_CRYPTO_CIPHER_STATE_FINALIZED        ///< The cipher operation is finalized.
} sf_crypto_cipher_state_t;


/** SSP Crypto Cipher Framework instance control block.
 * DO NOT INITIALIZE.
 * Initialization occurs when SF_CRYPTO_CIPHER_Open is called */
typedef struct st_sf_crypto_cipher_instance_ctrl
{
    sf_crypto_key_type_t        key_type;                   ///< Key type.
    sf_crypto_key_size_t        key_size;                   ///< Key size.
    sf_crypto_cipher_mode_t     cipher_chaining_mode;       ///< Chaining mode specified for the cipher operation.
    sf_crypto_cipher_state_t    status;                     ///< Module status.
    crypto_algorithm_type_t     cipher_algorithm_type;      ///< Cipher algorithm for the keys selected.
    sf_crypto_instance_ctrl_t * p_lower_lvl_fwk_common_ctrl;///< Pointer to the Crypto Framework Common instance.
    sf_crypto_api_t           * p_lower_lvl_fwk_common_api; ///< Pointer to the Crypto Framework Common API.
    sf_crypto_trng_instance_ctrl_t  * p_lower_lvl_sf_crypto_trng_ctrl;     ///< Pointer to the Crypto TRNG API.
    sf_crypto_trng_api_t      * p_sf_crypto_trng_api;       ///< Pointer to the Crypto TRNG API.
    void                      * p_hal_ctrl;             ///< Pointer to HAL control structure for the Cipher operation.
    void                      * p_hal_api;                  ///< Pointer to HAL API structure for the cipher algorithm.
    void                      * p_cipher_context_buffer;      ///< Cipher context buffer after DWORD alignment.

} sf_crypto_cipher_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const sf_crypto_cipher_api_t    g_sf_crypto_cipher_on_sf_crypto_cipher;
/** @endcond */

/** @} (end defgroup SF_CRYPTO_CIPHER) */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_CRYPTO_CIPHER_H */
