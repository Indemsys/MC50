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
 * File Name    : sf_crypto_api.h
 * Description  : Interface definition for SSP Crypto Framework Common Module.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup SF_Interface_Library
 * @defgroup SF_CRYPTO_API SSP Crypto Framework Common Module Interface
 * @brief Interface definition for Synergy Crypto Framework module.
 *
 * @section SF_CRYPTO_API_SUMMARY Summary
 * This is the Interface of SF_CRYPTO Framework module.
 *
 * Crypto Common Framework Interface description: @ref FrameworkCrypto
 *
 * @{
 **********************************************************************************************************************/

#ifndef SF_CRYPTO_API_H
#define SF_CRYPTO_API_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
/*LDRA_NOANALYSIS tx_api.h is not maintained by Renesas, so LDRA analysis is skipped for this file only. */
#include "tx_api.h"
/*LDRA_ANALYSIS */
#include "r_crypto_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** The API version of SSP Crypto Framework Common Module */
#define SF_CRYPTO_API_VERSION_MAJOR       (2U)
#define SF_CRYPTO_API_VERSION_MINOR       (0U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** A structure to handle data among Crypto Framework modules */
typedef  struct sf_crypto_data_handle
{
    uint8_t   * p_data;             ///< Pointer to data
    uint32_t    data_length;        ///< The length of data pointed by p_data
} sf_crypto_data_handle_t;

/** Supported key types */
typedef enum sf_crypto_key_type
{
    SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT,      ///< RSA Key pair in standard format and plain text
    SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT,  ///< RSA Key pair in CRT format and plain text
    SF_CRYPTO_KEY_TYPE_RSA_WRAPPED, ///< RSA Key pair public key in plain text and wrapped standard format private key.
    SF_CRYPTO_KEY_TYPE_AES_WRAPPED,         ///< Wrapped AES key
    SF_CRYPTO_KEY_TYPE_AES_PLAIN_TEXT,      ///< AES Plain text key
    SF_CRYPTO_KEY_TYPE_ECC_PLAIN_TEXT,      ///< ECC Key pair in standard format and plain text.
    SF_CRYPTO_KEY_TYPE_ECC_WRAPPED,         ///< ECC Key pair public key in plain text and wrapped standard format private key.
    SF_CRYPTO_KEY_TYPE_ENCRYPTED_RSA_PRIVATE_KEY,   ///< RSA private key in encrypted format for installation.
    SF_CRYPTO_KEY_TYPE_ENCRYPTED_AES_KEY,           ///< AES key in encrypted format for installation.
    SF_CRYPTO_KEY_TYPE_ENCRYPTED_ECC_PRIVATE_KEY,   ///< ECC private key in encrypted format for installation.
} sf_crypto_key_type_t;

/** Supported key sizes */
typedef enum sf_crypto_key_size
{
    SF_CRYPTO_KEY_SIZE_RSA_1024,    ///< RSA 1024-bit key
    SF_CRYPTO_KEY_SIZE_RSA_2048,    ///< RSA 2048-bit key
    SF_CRYPTO_KEY_SIZE_AES_128,     ///< AES 128-bit key for CBC, CTR, ECB, GCM chaining modes
    SF_CRYPTO_KEY_SIZE_AES_XTS_128, ///< AES 128-bit key for XTS chaining mode only
    SF_CRYPTO_KEY_SIZE_AES_192,     ///< AES 192-bit key for CBC, CTR, ECB, GCM chaining modes
    SF_CRYPTO_KEY_SIZE_AES_256,     ///< AES 256-bit key for CBC, CTR, ECB, GCM chaining modes
    SF_CRYPTO_KEY_SIZE_AES_XTS_256, ///< AES 256-bit key for XTS chaining mode only
    SF_CRYPTO_KEY_SIZE_ECC_192,     ///< ECC 192-bit key
    SF_CRYPTO_KEY_SIZE_ECC_224,     ///< ECC 224-bit key
    SF_CRYPTO_KEY_SIZE_ECC_256,     ///< ECC 256-bit key
    SF_CRYPTO_KEY_SIZE_ECC_384,     ///< ECC 384-bit key
} sf_crypto_key_size_t;

/** State codes for the SSP Crypto Framework Common Module */
typedef enum e_sf_crypto_state
{
    SF_CRYPTO_CLOSED,               ///< The module is closed.
    SF_CRYPTO_OPENED                ///< The module is opened.
} sf_crypto_state_t;

/** Event code for the SSP Crypto Framework Common Module. This event code is all reserved for the future use. */
typedef enum e_sf_crypto_event
{
    SF_CRYPTO_EVENT_PROCEDURE_DONE,         ///< Crypto hardware procedure done.
    SF_CRYPTO_EVENT_ERROR                   ///< Error occurred.
} sf_crypto_event_t;

/** SF_CRYPTO Close option.
 * The module executes close operation if any SF_CRYPTO_XXX modules have already closed if
 * SF_CRYPTO_CLOSE_OPTION_DEFAULT option is specified. The module performs close operation regardless of any
 * SF_CRYPTO_XXX module status if SF_CRYPTO_CLOSE_OPTION_FORCE_CLOSE is specified. */
typedef enum e_sf_crypto_close_option
{
    SF_CRYPTO_CLOSE_OPTION_DEFAULT,         ///< Close the module if no any SF_CRYPTO_XXX modules opened.
    SF_CRYPTO_CLOSE_OPTION_FORCE_CLOSE      ///< Close the module regardless of SF_CRYPTO_XXX modules status.
} sf_crypto_close_option_t;

/** Callback arguments for the SSP Crypto Framework Common Module */
typedef struct st_sf_crypto_callback_args
{
    sf_crypto_event_t   event;              ///< Event code of the low level hardware
    ssp_err_t           error;              ///< Error code if SF_CRYPTO_EVENT_ERROR
} sf_crypto_callback_args_t;

/** SSP Crypto Framework Common Module control block.  Allocate an instance specific control block to pass into the
 *  SSP Crypto Framework Common Module API calls.
 * @par Implemented as
 * - sf_crypto_instance_ctrl_t
 */
typedef void sf_crypto_ctrl_t;

/** Configuration structure for the SSP Crypto Framework Common Module */
typedef struct st_sf_crypto_cfg
{
    uint32_t            wait_option;        ///< Wait option for RTOS service calls
    crypto_instance_t * p_lower_lvl_crypto; ///< Pointer to a low-level Crypto engine HAL driver instance
    void const        * p_extend;           ///< Extension parameter for hardware specific settings
    void const        * p_context;          ///< Placeholder for user data
    void              * p_memory_pool;      ///< Byte pool address
    uint32_t            memory_pool_size;   ///< Byte pool size
    sf_crypto_close_option_t    close_option;   ///< Close option
} sf_crypto_cfg_t;

/** Shared Interface definition for the SSP Crypto Framework Common Module */
typedef struct st_sf_crypto_api
{
    /** Open SSP Crypto Framework Common Module.
     * This function is to be called only once to initialize the Crypto services.
     * @par Implemented as
     * - SF_CRYPTO_Open()
     * @param[in,out]  p_ctrl       Pointer to a Crypto framework control block. Must be declared by user.
     * @param[in]      p_cfg        Pointer to a Crypto framework configuration structure.
     *                              All elements of this structure must be set by user.
     */
    ssp_err_t (* open)(sf_crypto_ctrl_t * const p_ctrl, sf_crypto_cfg_t const * const p_cfg);

    /** Close SSP Crypto Framework Common Module.
     * This function is to be called only once when the the  Crypto services are no longer required.
     * @par Implemented as
     * - SF_CRYPTO_Close()
     * @param[in,out]  p_ctrl       Pointer to a Crypto framework control block.
     */
    ssp_err_t (* close)(sf_crypto_ctrl_t * const p_ctrl);

    /** Lock shared resources for Cryptography operations. This function is typically called by Crypto Framework
     *  modules (SF_CRYPTO_XXX) to protect shared software resources provided in Crypto Framework Common module
     *  or shared crypto hardware engine. Once lock() is called by a thread, any Crypto Framework services called
     *  by the other thread will be blocked until unlock() is called. The lock and unlock operations are
     *  managed by Crypto Framework modules so users do not need to call this function in typical use-cases.
     *  However, if this function is called by a user thread, users must be aware that any cryptography operations
     *  by the other threads will be locked out until unlock() is called by the thread which called lock().
     * @par Implemented as
     * - SF_CRYPTO_Lock()
     * @param[in,out]  p_ctrl       Pointer to a Crypto framework control block.
     */
    ssp_err_t (* lock)(sf_crypto_ctrl_t * const p_ctrl);

    /** Unlock shared resources for Cryptography operations. This function is typically called by Crypto Framework
     *  modules (SF_CRYPTO_XXX) to allow any other threads to access to shared software resources provided in
     *  Crypto Framework Common module or shared crypto hardware engine. This function must be called by a thread
     *  which called lock(). The lock and unlock operations are managed by Crypto Framework modules so users do not
     *  need to call this function in typical use-cases. However, this function must be called by a user thread if
     *  the thread has ever called lock().
     * @par Implemented as
     * - SF_CRYPTO_Unlock()
     * @param[in,out]  p_ctrl       Pointer to a Crypto framework control block.
     */
    ssp_err_t (* unlock)(sf_crypto_ctrl_t * const p_ctrl);

     /** Get version of SSP Crypto Framework Common Module.
     * @par Implemented as
     * - SF_CRYPTO_VersionGet()
     * @param[out]     p_version    Pointer to the memory to store the version information.
     */
    ssp_err_t (* versionGet)(ssp_version_t * const p_version);

    /** Get status of SSP Crypto Framework Common Module.
     * @par Implemented as
     * - SF_CRYPTO_StatusGet()
     * @param[in]      p_ctrl      Pointer to a Crypto framework control block.
     * @param[out]     p_status    Memory location to store module status.
     */
    ssp_err_t (* statusGet)(sf_crypto_ctrl_t * const p_ctrl, sf_crypto_state_t * p_status);

} sf_crypto_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_sf_crypto_instance
{
    sf_crypto_ctrl_t      * p_ctrl;    ///< Pointer to the control structure for this instance
    sf_crypto_cfg_t const * p_cfg;     ///< Pointer to the configuration structure for this instance
    sf_crypto_api_t const * p_api;     ///< Pointer to the API structure for this instance
} sf_crypto_instance_t;

/** @} (end defgroup SF_CRYPTO_API) */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_CRYPTO_API_H */
