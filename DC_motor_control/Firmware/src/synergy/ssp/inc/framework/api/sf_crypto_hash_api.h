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
 * File Name    : sf_crypto_hash_api.h
 * Description  : Interface definition for SSP Crypto HASH Framework.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup SF_Interface_Library
 * @defgroup SF_CRYPTO_HASH_API SSP Crypto HASH Framework Interface
 * @brief Interface definition for Synergy Crypto HASH Framework module.
 *
 * @section SF_CRYPTO_HASH_API_SUMMARY Summary
 * This is the Interface of SF_CRYPTO_HASH Framework module.
 *
 * Crypto HASH Framework Interface description: @ref FrameworkCrypto
 *
 * @{
 **********************************************************************************************************************/

#ifndef SF_CRYPTO_HASH_API_H
#define SF_CRYPTO_HASH_API_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "sf_crypto_api.h"
#include "r_hash_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** The API version of SSP Crypto HASH Framework */
#define SF_CRYPTO_HASH_API_VERSION_MAJOR       (2U)
#define SF_CRYPTO_HASH_API_VERSION_MINOR       (0U)

/** Message Digest size for each HASH algorithm in bytes */
#define SF_CRYPTO_HASH_MESSAGE_DIGEST_SIZE_MD5      (16U)       ///< Message Digest size for SHA1
#define SF_CRYPTO_HASH_MESSAGE_DIGEST_SIZE_SHA1     (20U)       ///< Message Digest size for SHA1
#define SF_CRYPTO_HASH_MESSAGE_DIGEST_SIZE_SHA224   (28U)       ///< Message Digest size for SHA224
#define SF_CRYPTO_HASH_MESSAGE_DIGEST_SIZE_SHA256   (32U)       ///< Message Digest size for SHA256

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** State codes for the SSP SSP Crypto HASH Framework */
typedef enum e_sf_crypto_hash_state
{
    SF_CRYPTO_HASH_CLOSED,                  ///< The module is closed.
    SF_CRYPTO_HASH_OPENED,                  ///< The module is opened. The initial message digest is not yet generated.
    SF_CRYPTO_HASH_DIGEST_INITIALIZED,      ///< Message digest is initialized.
    SF_CRYPTO_HASH_DIGEST_UPDATED           ///< Message digest is updated.
} sf_crypto_hash_state_t;

/** HASH algorithm types for the SSP SSP Crypto HASH Framework */
typedef enum e_sf_crypto_hash_type
{
    SF_CRYPTO_HASH_ALGORITHM_MD5,           ///< MD5 algorithm type
    SF_CRYPTO_HASH_ALGORITHM_SHA1,          ///< SHA-1 algorithm type
    SF_CRYPTO_HASH_ALGORITHM_SHA224,        ///< SHA-224 algorithm type
    SF_CRYPTO_HASH_ALGORITHM_SHA256         ///< SHA-256 algorithm type
} sf_crypto_hash_type_t;

typedef  sf_crypto_data_handle_t  sf_crypto_hash_t;

/** HASH internal context structure for a message digest */
typedef struct st_sf_crypto_hash_context
{
    uint8_t  * p_message_digest;                ///< Intermediate digest stored buffer - WORD aligned
    uint8_t  * p_message_digest_org;            ///< Originally allocated buffer - may not be WORD aligned.
    uint8_t  * p_message_buffer;                ///< Intermediate message data stored buffer - - WORD aligned.
    uint8_t  * p_message_buffer_org;            ///< IOriginally allocated buffer - may not be WORD aligned.
    uint64_t   message_bytes;                   ///< Number of bytes from user data processed.
    uint32_t   message_bytes_buffered;          ///< Number of bytes buffered in the message data stored buffer.
} sf_crypto_hash_context_t;

/** Callback arguments for the SSP Crypto HASH framework */
typedef struct st_sf_crypto_hash_callback_args
{
    ssp_err_t      error;                       ///< Error code if SF_CRYPTO_EVENT_ERROR
} sf_crypto_hash_callback_args_t;

/** SSP Crypto framework control block.  Allocate an instance specific control block to pass into the SSP Crypto
 *  framework API calls.
 * @par Implemented as
 * - sf_crypto_instance_ctrl_t
 */
typedef void sf_crypto_hash_ctrl_t;

/** Configuration structure for the SSP SSP Crypto HASH framework */
typedef struct st_sf_crypto_hash_cfg
{
    sf_crypto_hash_type_t       hash_type;                  ///< HASH algorithm type.
    sf_crypto_instance_t      * p_lower_lvl_crypto_common;  ///< Pointer to a Crypto Framework common instance.
    hash_instance_t           * p_lower_lvl_instance;       ///< pointer to HASH lower-level module instance
    void                      * p_extend;	    ///< Pointer to an optional configuration for Crypto HAL module.
} sf_crypto_hash_cfg_t;

/** Shared Interface definition for the SSP SSP Crypto framework */
typedef struct st_sf_crypto_hash_api
{
    /** Opens SSP Crypto HASH framework. This function initializes a control block of the framework module based
     *  on the configuration parameters such as the HASH algorithm type. The module allows users to have multiple
     *  instances with different control blocks, if required.
     * @par Implemented as
     * - SF_CRYPTO_HASH_Open()
     * @param[in,out]  p_ctrl       Pointer to Crypto HASH Framework control block structure.
     * @param[in]      p_cfg        Pointer to sf_crypto_hash_cfg_t configuration structure. All elements of this
     *                               structure must be set by user.
     */
    ssp_err_t (* open) (sf_crypto_hash_ctrl_t * const p_ctrl, sf_crypto_hash_cfg_t const * const p_cfg);

    /** Closes SSP Crypto HASH framework. This function de-initializes a control block of the framework module and
     *  allow users to re-configure the module differently. For instance, users can close the module and re-open it
     *  with different HASH algorithm for a new digest operation.
     * @par Implemented as
     * - SF_CRYPTO_HASH_Close()
     * @param[in,out]  p_ctrl       Pointer to Crypto HASH Framework control block structure.
     */
    ssp_err_t (* close) (sf_crypto_hash_ctrl_t * const p_ctrl);

    /** Initializes a message digest operation. Must be called once open() or hashFinal() is called to initialize a
     *  new digest operation. Unless a different HASH type is used, users do not need to close the module for a new
     *  digest operation but can call this function to restart another digest operation. This is a blocking call.
     * @par Implemented as
     * - SF_CRYPTO_HASH_MessageDigestInit()
     *  @param[in]      p_ctrl      Pointer to Crypto HASH Framework control block structure.
     */
    ssp_err_t (* hashInit) (sf_crypto_hash_ctrl_t * const p_ctrl);

    /** Hashes input data and saves it in an internal context buffer. Can be called multiple times for additional
     *  blocks of data. This is a blocking call.
     * @par Implemented as
     * - SF_CRYPTO_HASH_MessageDigestUpdate()
     *  @param[in]      p_ctrl      Pointer to Crypto HASH Framework control block structure.
     *  @param[in]      p_data_in   Pointer to an input data buffer and the data length.
     */
    ssp_err_t (* hashUpdate) (sf_crypto_hash_ctrl_t * const p_ctrl, sf_crypto_data_handle_t const * const p_data_in);

    /** Hashes the last block of data and returns a message digest in the output buffer. Once hashFinal() is called,
     *  no additional call of hashUpdate() is allowed but hashInit() can be called to initialize a new digest
     *  operation unless the other HASH algorithm type needed. If the other HASH algorithm is required for a new
     *  digest operation, call close() and open(). This is a blocking call.
     * @par Implemented as
     * - SF_CRYPTO_HASH_MessageDigestUpdate()
     *  @param[in]      p_ctrl          Pointer to Crypto HASH Framework control block structure.
     *  @param[in,out]  p_msg_digest    Pointer to an output data buffer and the buffer size. Message digest will be
     *                                  generated in the buffer. Data buffer must be aligned to word alignment and
     *                                  the size must be sufficient to store the message digest.
     *  @param[out]     p_size          Pointer to the 32-bit memory space to store the size of message digest.
     */
    ssp_err_t (* hashFinal) (sf_crypto_hash_ctrl_t * const p_ctrl, sf_crypto_data_handle_t * const p_msg_digest,
                                                                   uint32_t * p_size);

    /** Get version of SSP Crypto HASH framework.
    * @par Implemented as
    * - SF_CRYPTO_HASH_VersionGet()
    * @param[in]      p_version     Pointer to the memory to store the module version.
    */
   ssp_err_t (* versionGet) (ssp_version_t * const p_version);

} sf_crypto_hash_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_sf_crypto_hash_instance
{
    sf_crypto_hash_ctrl_t      * p_ctrl;    ///< Pointer to the control structure for this instance
    sf_crypto_hash_cfg_t       * p_cfg;     ///< Pointer to the configuration structure for this instance
    sf_crypto_hash_api_t const * p_api;     ///< Pointer to the API structure for this instance
} sf_crypto_hash_instance_t;

/** @} (end defgroup SF_CRYPTO_HASH_API) */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_CRYPTO_HASH_API_H */
