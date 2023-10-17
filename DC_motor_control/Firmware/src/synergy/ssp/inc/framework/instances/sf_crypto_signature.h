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

/**********************************************************************************************************************
 * File Name    : sf_crypto_signature_api.h
 * Description  : Interface definition for SSP Crypto Signature Framework module instance.
 **********************************************************************************************************************/

#ifndef SF_CRYPTO_SIGNATURE_H
#define SF_CRYPTO_SIGNATURE_H
/*******************************************************************************************************************//**
 * @ingroup SF_Library
 * @defgroup SF_CRYPTO_SIGNATURE SSP Crypto Signature Framework
 * @brief RTOS-integrated Crypto Signature Framework Module.
 *
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto.h"
#include "sf_crypto_signature_api.h"
#include "r_crypto_api.h"
#include "r_rsa_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** The API version of SSP Crypto Signature Framework */
#define SF_CRYPTO_SIGNATURE_CODE_VERSION_MAJOR       (2U)
#define SF_CRYPTO_SIGNATURE_CODE_VERSION_MINOR       (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** State codes for the SSP Crypto Signature framework module. Once the module is opened successfully, then the state is
 * transition to OPENED state. After sign/verify operations, the Signature framework module must be closed with CLOSED
 * state. */
typedef enum e_sf_crypto_signature_state
{
    SF_CRYPTO_SIGNATURE_CLOSED,                            ///< The Signature module is closed.
    SF_CRYPTO_SIGNATURE_OPENED                             ///< The Signature module is opened.
} sf_crypto_signature_state_t;

/** Internal state codes for the SSP Crypto Signature framework module. */
typedef enum e_sf_crypto_signature_operation_state
{
    SF_CRYPTO_SIGNATURE_OPERATION_STATE_OPEN,                             ///< Module opened to perform
                                                                          ///< sign/verify operation.
    SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_INITIALIZED,                 ///< Context is initialized for Sign Operation.
    SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_UPDATED,                     ///< Sign operation is in progress.
    SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_FINALIZED,                   ///< Sign operation has been completed.
    SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_INITIALIZED,               ///< Context is initialized for Verify Operation.
    SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_UPDATED,                   ///< Verify operation is in progress.
    SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_FINALIZED                  ///< Verify operation has been completed.
} sf_crypto_signature_operation_state_t;

/** Internal SSP Crypto Signature framework module context. */
typedef struct sf_crypto_signature_context
{
    /**  Operating mode. (Sign / Verify operation) */
    sf_crypto_signature_mode_t                      operation_mode;

    /** Algorithm specific parameters. OR hold formatted input data. */
    sf_crypto_signature_algorithm_init_params_t    * p_aglorithm_specific_params;

    /** Internal buffer to format input data */
    sf_crypto_data_handle_t                         buffer;

    /** Buffer to hold private key in case of Sign Operations. OR.
     *  Buffer to hold public key in case of Verify Operations. */
    uint8_t                                       * p_key_data;

    /** Length of key data. */
    uint32_t                                        key_data_length;
} sf_crypto_signature_context_t;

/** SSP Crypto Signature Framework instance control block */
typedef struct st_sf_crypto_signature_instance_ctrl
{
    sf_crypto_signature_state_t                   status;                        ///< Module status
    sf_crypto_key_type_t                          key_type;                      ///< Key type
    sf_crypto_key_size_t                          key_size;                      ///< Key size
    sf_crypto_signature_operation_state_t         operation_state;               ///< Internal Operation state.
    sf_crypto_signature_context_t                 operation_context;             ///< Context for sign / verify
                                                                                 ///< operations.
    sf_crypto_instance_ctrl_t                     * p_lower_lvl_common_ctrl;     ///< Pointer to the Crypto
                                                                                 ///< Framework Common instance
    sf_crypto_api_t                               * p_lower_lvl_common_api;      ///< Pointer to the Crypto
                                                                                 ///< Framework API instance
    void                                          * p_hal_ctrl;                  ///< pointer to Crypto module
                                                                                 ///< control structure
    void                                          * p_hal_api;                   ///< pointer to Crypto module
                                                                                 ///< API structure
    sf_crypto_hash_instance_t                     * p_lower_lvl_sf_crypto_hash;  ///< pointer to Crypto Framework
                                                                                 ///< Hash instance
} sf_crypto_signature_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const sf_crypto_signature_api_t    g_sf_crypto_signature_on_sf_crypto_signature;
/** @endcond */


/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

/** @} (end defgroup SF_CRYPTO_SIGNATURE) */

#endif /* SF_CRYPTO_SIGNATURE_H */
