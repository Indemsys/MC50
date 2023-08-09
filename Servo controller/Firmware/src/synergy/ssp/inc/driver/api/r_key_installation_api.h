/***********************************************************************************************************************
 * Copyright [2017-2023] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : r_key_installation_api.h
 * Description  : Key Installation driver interface to install the user key (generated outside of the Synergy platform)
 *                on to the Synergy platform.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Crypto_API
 * @defgroup KEY_INSTALLATION_API KEY_INSTALLATION Interface
 *
 * @brief Key Installation functions for Key Installation procedure
 *
 * @{
 **********************************************************************************************************************/

#ifndef DRV_KEY_INSTALLATION_API_H
#define DRV_KEY_INSTALLATION_API_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
/** Register definitions, common services and error codes. */
#include "bsp_api.h"
#include "r_crypto_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

#define SCE_KEY_INSTALLATION_API_VERSION_MAJOR                      (2U)
#define SCE_KEY_INSTALLATION_API_VERSION_MINOR                      (0U)

/** Macro definitions for universal KeyInstall Session/ IV Key sizes */
#define KEY_INSTALLATION_SESSION_KEY_SIZE_IN_WORDS                  (8U)
#define KEY_INSTALLATION_IV_SIZE_IN_WORDS                           (4U)

/** Macro definitions for RSA Key sizes */
/** RSA Modulus size in words for a 1024-bit RSA Key */
#define KEY_INSTALLATION_RSA1024_MODULUS_SIZE_IN_WORDS              (32U)

/** RSA Modulus size in words for a 2048-bit RSA Key */
#define KEY_INSTALLATION_RSA2048_MODULUS_SIZE_IN_WORDS              (64U)

/** RSA Encrypted key size in words for a 1024-bit RSA (private) Key */
#define KEY_INSTALLATION_RSA1024_ENCRYPTED_KEY_SIZE_IN_WORDS        (36U)

/** RSA Encrypted key size in words for a 2048-bit RSA (private) Key */
#define KEY_INSTALLATION_RSA2048_ENCRYPTED_KEY_SIZE_IN_WORDS        (68U)

/** RSA Wrapped (output) private key size in words for a 1024-bit RSA Key */
#define KEY_INSTALLATION_RSA1024_WRAPPED_PRIVKEY_SIZE_IN_WORDS      (37U)

/** RSA Wrapped (output) private key size in words for a 2048-bit RSA Key */
#define KEY_INSTALLATION_RSA2048_WRAPPED_PRIVKEY_SIZE_IN_WORDS      (69U)

/** Macro definitions for AES Key sizes */
/** AES Encrypted key size in words for a 128-bit AES Key */
#define KEY_INSTALLATION_AES128_ENCRYPTED_KEY_SIZE_IN_WORDS         (8U)

/** AES Encrypted key size in words for a 192-bit AES Key */
#define KEY_INSTALLATION_AES192_ENCRYPTED_KEY_SIZE_IN_WORDS         (12U)

/** AES Encrypted key size in words for a 256-bit AES Key */
#define KEY_INSTALLATION_AES256_ENCRYPTED_KEY_SIZE_IN_WORDS         (12U)

/** AES Encrypted key size in words for a 128-bit AES Key in XTS chaining mode */
#define KEY_INSTALLATION_AES128_ENCRYPTED_XTS_KEY_SIZE_IN_WORDS     (12U)

/** AES Encrypted key size in words for a 256-bit AES Key in XTS chaining mode*/
#define KEY_INSTALLATION_AES256_ENCRYPTED_XTS_KEY_SIZE_IN_WORDS     (20U)

/** AES Wrapped (output) key size in words for a 128-bit AES Key */
#define KEY_INSTALLATION_AES128_WRAPPED_KEY_SIZE_IN_WORDS           (9U)

/** AES Wrapped (output) key size in words for a 192-bit AES Key */
#define KEY_INSTALLATION_AES192_WRAPPED_KEY_SIZE_IN_WORDS           (13U)

/** AES Wrapped (output) key size in words for a 256-bit AES Key */
#define KEY_INSTALLATION_AES256_WRAPPED_KEY_SIZE_IN_WORDS           (13U)

/** AES Wrapped (output) key size in words for a 128-bit AES Key in XTS chaining mode */
#define KEY_INSTALLATION_AES128_WRAPPED_XTS_KEY_SIZE_IN_WORDS       (13U)

/** AES Wrapped (output) key size in words for a 256-bit AES Key in XTS chaining mode*/
#define KEY_INSTALLATION_AES256_WRAPPED_XTS_KEY_SIZE_IN_WORDS       (21U)

/** Macro definitions for ECC Key sizes */
/** ECC Encrypted key size in words for a 192-bit ECC Key */
#define KEY_INSTALLATION_ECC192_ENCRYPTED_KEY_SIZE_IN_WORDS         (12U)

/** ECC Encrypted key size in words for a 224-bit ECC Key */
#define KEY_INSTALLATION_ECC224_ENCRYPTED_KEY_SIZE_IN_WORDS         (12U)

/** ECC Encrypted key size in words for 256-bit ECC Key */
#define KEY_INSTALLATION_ECC256_ENCRYPTED_KEY_SIZE_IN_WORDS         (12U)

/** ECC Encrypted key size in words for a 384-bit ECC Key */
#define KEY_INSTALLATION_ECC384_ENCRYPTED_KEY_SIZE_IN_WORDS         (16U)

/** ECC Wrapped (output) key size in words for a 192-bit ECC Key */
#define KEY_INSTALLATION_ECC192_WRAPPED_KEY_SIZE_IN_WORDS           (13U)

/** ECC Wrapped (output) key size in words for a 224-bit ECC Key */
#define KEY_INSTALLATION_ECC224_WRAPPED_KEY_SIZE_IN_WORDS           (13U)

/** ECC Wrapped (output) key size in words for a 256-bit ECC Key */
#define KEY_INSTALLATION_ECC256_WRAPPED_KEY_SIZE_IN_WORDS           (13U)

/** ECC Wrapped (output) key size in words for a 384-bit ECC Key */
#define KEY_INSTALLATION_ECC384_WRAPPED_KEY_SIZE_IN_WORDS           (17U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** A structure to handle data for Key Installation module operation */
/** Supported key format definitions */
typedef enum e_key_installation_key_format
{
    KEY_INSTALLATION_KEY_FORMAT_ENCRYPTED_RSA_PRIVATE_KEY,          ///< Encrypted RSA Private key
    KEY_INSTALLATION_KEY_FORMAT_WRAPPED_RSA_PRIVATE_KEY,            ///< RSA Private Key wrapped
    KEY_INSTALLATION_KEY_FORMAT_ENCRYPTED_AES_KEY,                  ///< Encrypted AES Private key
    KEY_INSTALLATION_KEY_FORMAT_WRAPPED_AES_KEY,                    ///< AES Private Key wrapped
	KEY_INSTALLATION_KEY_FORMAT_ENCRYPTED_INSTALL_KEY,              ///< TO BE REMOVED w/ RSA/ ECC MODS
    KEY_INSTALLATION_KEY_FORMAT_ENCRYPTED_ECC_PRIVATE_KEY,          ///< Encrypted ECC Private key
    KEY_INSTALLATION_KEY_FORMAT_WRAPPED_ECC_PRIVATE_KEY,            ///< ECC Private Key wrapped
    KEY_INSTALLATION_KEY_FORMAT_ENCRYPTED_RSA_PRIVATE_CRT_KEY,      ///< Encrypted RSA Private CRT key
    KEY_INSTALLATION_KEY_FORMAT_WRAPPED_RSA_PRIVATE_CRT_KEY,        ///< RSA Private CRT Key wrapped
    KEY_INSTALLATION_KEY_FORMAT_SESSION_KEY,                        ///< Session key for keyInstall API
} key_installation_key_format_t;

/** Supported key sizes */
typedef enum e_key_installation_key_size
{
    KEY_INSTALLATION_KEY_SIZE_RSA_1024,                 ///< RSA 1024-bit key
    KEY_INSTALLATION_KEY_SIZE_RSA_2048,                 ///< RSA 2048-bit key
    KEY_INSTALLATION_KEY_SIZE_AES_128,                  ///< AES 128-bit key for CBC, CTR, ECB, GCM chaining modes
    KEY_INSTALLATION_KEY_SIZE_AES_XTS_128,              ///< AES 128-bit key for XTS chaining mode only
    KEY_INSTALLATION_KEY_SIZE_AES_192,                  ///< AES 192-bit key for CBC, CTR, ECB, GCM chaining modes
    KEY_INSTALLATION_KEY_SIZE_AES_256,                  ///< AES 256-bit key for CBC, CTR, ECB, GCM chaining modes
    KEY_INSTALLATION_KEY_SIZE_AES_XTS_256,              ///< AES 256-bit key for XTS chaining mode only
    KEY_INSTALLATION_KEY_SIZE_ENCRYPTED_INSTALL_416,    ///< Renesas provided install key size
    KEY_INSTALLATION_KEY_SIZE_ECC_192,                  ///< ECC 192-bit key
    KEY_INSTALLATION_KEY_SIZE_ECC_224,                  ///< ECC 224-bit key
    KEY_INSTALLATION_KEY_SIZE_ECC_256,                  ///< ECC 256-bit key
    KEY_INSTALLATION_KEY_SIZE_ECC_384,                  ///< ECC 384-bit key
    KEY_INSTALLATION_KEY_SIZE_SESSION,                  ///< Session Key size for all operations

} key_installation_key_size_t;

/** Supported Shared Key Index values (for keyInstall) */
typedef enum e_key_installation_key_shared_index
{
    KEY_INSTALLATION_KEY_SHARED_INDEX_0,                ///< Shared Key Index 0
    KEY_INSTALLATION_KEY_SHARED_INDEX_1,                ///< Shared Key Index 1
    KEY_INSTALLATION_KEY_SHARED_INDEX_2,                ///< Shared Key Index 2
    KEY_INSTALLATION_KEY_SHARED_INDEX_3,                ///< Shared Key Index 3
    KEY_INSTALLATION_KEY_SHARED_INDEX_4,                ///< Shared Key Index 4
    KEY_INSTALLATION_KEY_SHARED_INDEX_5,                ///< Shared Key Index 5
    KEY_INSTALLATION_KEY_SHARED_INDEX_6,                ///< Shared Key Index 6
    KEY_INSTALLATION_KEY_SHARED_INDEX_7,                ///< Shared Key Index 7
    KEY_INSTALLATION_KEY_SHARED_INDEX_8,                ///< Shared Key Index 8
    KEY_INSTALLATION_KEY_SHARED_INDEX_9,                ///< Shared Key Index 9
    KEY_INSTALLATION_KEY_SHARED_INDEX_A,                ///< Shared Key Index 10
    KEY_INSTALLATION_KEY_SHARED_INDEX_B,                ///< Shared Key Index 11
    KEY_INSTALLATION_KEY_SHARED_INDEX_C,                ///< Shared Key Index 12
    KEY_INSTALLATION_KEY_SHARED_INDEX_D,                ///< Shared Key Index 13
    KEY_INSTALLATION_KEY_SHARED_INDEX_E,                ///< Shared Key Index 14
    KEY_INSTALLATION_KEY_SHARED_INDEX_F,                ///< Shared Key Index 15
} key_installation_key_shared_index_t;

/** Definition for Key data structure for Key Installation API operations */
typedef struct st_key_installation_key
{
    key_installation_key_format_t   key_format;         ///< Indicates the key_format
    key_installation_key_size_t     key_size;           ///< Indicates the key_type
    uint32_t                      * p_data;             ///< Pointer to input (encrypted user key (OR) output data
                                                        ///< buffer to hold the wrapped key)
    uint32_t                        data_length;        ///< The length of data in WORDS(32-bits), pointed by p_data
} key_installation_key_t;

/** Key Installation control block. Allocate using driver instance control structure from driver instance header file.
 */
typedef void key_installation_ctrl_t;

/** Key Installation Interface configuration structure. User must fill in these values before invoking the open()
 * function */
typedef struct st_key_installation_cfg
{
    crypto_api_t const          * p_lower_lvl_crypto_api;   ///< pointer to crypto engine api
    void const                  * p_extend;                 ///< Extension parameter for hardware specific settings
} key_installation_cfg_t;

/** KEY_INSTALLATION_Interface functions implemented at the HAL layer will follow this API. */
typedef struct st_key_installation_api
{
    /** Key Installation module open API function. Must be called before invoking Key Installation operation.
     *
     * @param[in,out] p_ctrl    Pointer to control structure for the KEY_INSTALLATION interface.
     *                          Must be declared by user.
     * @param[in]     p_cfg     Pointer to configuration structure for the KEY_INSTALLATION configuration.
     *                          All elements of this structure must be set by user.
     */
    ssp_err_t (* open)(key_installation_ctrl_t * const p_ctrl, key_installation_cfg_t const * const p_cfg);

    /** Key installation API function that takes the user's encrypted key, a shared index, session key, and an IV
     *  then returns wrapped key of the user's plain-text key.
     *
     * @param[in]     p_ctrl                    Pointer to the control structure.
     * @param[in]     p_user_key_rsa_modulus    Pointer to a user key's RSA modulus.
     *                                          Only applicable for Key Installation using RSA keys.
     *                                          For other crypto algorithms this parameter should be NULL.
     * @param[in]     p_user_key                Pointer to a user's encrypted key.
     *                                          In case of Key Installation using RSA keys,
     *                                          this will be the encrypted exponent.
     * @param[in]     shared_index              Shared Key Index that is returned by the DLM Service,
     *                                          accompanied by the Session Key that follows.
     * @param[in]     p_session_key             Pointer to the Session Key returned by the DLM Service,
     *                                          accompanied by the previous Shared Key Index parameter.
     * @param[in]     p_iv                      Pointer to the IV used to encrypt the User Key.
     *
     * @param[in,out] p_key_data                Pointer to output wrapped output key.
     *                                          'key_size' of p_key_data structure is UNUSED.
     *
     * @note For AES crypto algorithms, the buffer length required to hold the output wrapped key will be -
     *       'Plaintext key word length + 5'. Expected values are defined by macros for each supported key
     *       length/ mode, for example KEY_INSTALLATION_AES128_WRAPPED_KEY_SIZE_IN_WORDS for 128-bit AES.
     */
    ssp_err_t (* keyInstall)(key_installation_ctrl_t * const p_ctrl,
                             r_crypto_data_handle_t const * const p_user_key_rsa_modulus,
                             key_installation_key_t const * const p_user_key,
                             key_installation_key_shared_index_t const shared_index,
                             key_installation_key_t const * const p_session_key,
                             uint32_t const * const p_iv,
                             key_installation_key_t * const p_key_data);

    /** Close API function of Key Installation module.
     *  @param[in] p_ctrl Pointer to the control structure
     */
    ssp_err_t (* close)(key_installation_ctrl_t * const p_ctrl);

    /** Gets version and stores it in provided pointer p_version.
     * @param[out]    p_version  Code and API version used.
     */
    ssp_err_t (* versionGet)(ssp_version_t * const p_version);

} key_installation_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_key_installation_instance {
    key_installation_ctrl_t         * p_ctrl ;   ///< Pointer to the control structure for this instance
    key_installation_cfg_t    const * p_cfg  ;   ///< Pointer to the configuration structure for this instance
    key_installation_api_t    const * p_api  ;   ///< Pointer to the API structure for this instance
} key_installation_instance_t;

/***********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** Key Installation API Interface structure for this Instance. */
#if !(defined(BSP_MCU_GROUP_S124) || defined(BSP_MCU_GROUP_S128) || defined(BSP_MCU_GROUP_S1JA))
extern const key_installation_api_t    g_key_installation_on_sce;
#endif

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* DRV_KEY_INSTALLATION_API_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup KEY_INSTALLATION_API)
 **********************************************************************************************************************/
