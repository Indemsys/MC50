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
 * File Name    : sf_crypto_key_api.h
 * Description  : Interface definition for SSP Crypto Key Framework.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup SF_Interface_Library
 * @defgroup SF_CRYPTO_KEY_API SSP Crypto Key Framework Interface
 * @brief Interface definition for Synergy Crypto Key Framework module.
 *
 * @section SF_CRYPTO_KEY_API_SUMMARY Summary
 * This is the Interface of SF_CRYPTO_KEY Framework module. The Key framework module is a ThreadX aware Key Framework
 * Interface which provides key generation services. This sits between the user application and HAL layer.
 *
 * Crypto Key Framework Interface description: @ref FrameworkCrypto
 * @{
 **********************************************************************************************************************/

#ifndef SF_CRYPTO_KEY_API_H
#define SF_CRYPTO_KEY_API_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "sf_crypto_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** The API version of SSP Crypto Framework */
#define SF_CRYPTO_KEY_API_VERSION_MAJOR       (2U)
#define SF_CRYPTO_KEY_API_VERSION_MINOR       (0U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** State codes for the SSP Crypto Key framework module. Once the module is opened successfully, then the state is
 * transition to OPENED state. After Key operations, the Key framework module must be closed with CLOSED state. */
typedef enum e_sf_crypto_key_state
{
    SF_CRYPTO_KEY_CLOSED          = 0,              ///< The Key module is closed.
    SF_CRYPTO_KEY_OPENED          = 0x4F50454EU,    ///< The Key module is opened The code means 'OPEN'.
} sf_crypto_key_state_t;

typedef sf_crypto_data_handle_t  sf_crypto_key_t;

/** SSP Crypto framework control block.  Allocate an instance specific control block to pass into the SSP Crypto
 *  framework API calls.
 * @par Implemented as
 * - sf_crypto_instance_ctrl_t
 */
typedef void sf_crypto_key_ctrl_t;

/** Configuration structure for the SSP SSP Crypto Key framework */
typedef struct st_sf_crypto_key_cfg
{
    sf_crypto_key_type_t          key_type;                   ///< Key type to be generated.
    sf_crypto_key_size_t          key_size;                   ///< Key size to be generated.
    sf_crypto_data_handle_t       domain_params;              ///< Pointer to domain parameters for the requested key type.
                                                              ///< Structure contains the domain data in the order
                                                              ///< a||b||p||n for ECC as defined in FIPS186-3 and data length.
                                                              ///< Length of the data to be in bytes.
                                                              ///< Should set to NULL for RSA and AES.
    sf_crypto_data_handle_t       generator_point;            ///< Pointer to the generator base point of curve in the
                                                              ///< order Gx||Gy for ECC (where Gx and Gy are x and y
                                                              /// coordinates respectively) and data length.
                                                              ///< Length of the data to be in bytes.
                                                              ///< This parameter applies only for ECC.
                                                              ///< For others (AES and RSA), this is UNUSED.
                                                              ///< Should set to NULL for RSA and AES.
    sf_crypto_instance_t        * p_lower_lvl_crypto_common;  ///< Pointer to a Crypto Framework common instance.
    void const                  * p_extend;                   ///< Extension parameter for hardware specific settings (Future purpose).
} sf_crypto_key_cfg_t;

/** Shared Interface definition for the SSP SSP Crypto framework */
typedef struct st_sf_crypto_key_api
{
    /** Open SSP Crypto Key framework for subsequent call / Key generation.
     * @par Implemented as
     * - SF_CRYPTO_KEY_Open()
     * @param[in,out]  p_ctrl       Pointer to Crypto Key Framework control block structure.
     * @param[in]      p_cfg        Pointer to sf_crypto_key_cfg_t configuration structure. All elements of this
     *                              structure must be set by user.
     */
    ssp_err_t (* open)(sf_crypto_key_ctrl_t * const p_ctrl, sf_crypto_key_cfg_t const * const p_cfg);

    /** Close SSP Crypto Key framework.
     * @par Implemented as
     * - SF_CRYPTO_KEY_Close()
     * @param[in,out]  p_ctrl       Pointer to Crypto Key Framework control block structure.
     */
    ssp_err_t (* close)(sf_crypto_key_ctrl_t * const p_ctrl);

    /** Generate a key. This is a blocking call.
     * @par Implemented as
     * - SF_CRYPTO_KEY_Generate()
     *  @param[in]     p_ctrl       Pointer to Crypto Key Framework control block structure.
     *  @param[in,out] p_secret_key Pointer to a secret key structure.
     *                              The pointer to the buffer and it's length in bytes, are to be populated on input.
     *                              On success the key and it's length in bytes, are returned.
     *                              Refer to r_rsa_api.h for RSA secret key sizes.
     *                              Refer to r_aes_api.h for AES key sizes.
     *                              Refer to r_ecc_api.h for ECC key sizes.
     *                              p_secret_key should be WORD aligned. The memory allocation to store the secret
     *                              key is user's responsibility.
     *  @param[in,out] p_public_key Pointer to a public key structure.
     *                              The pointer to the buffer and it's length in bytes, are to be populated on input.
     *                              On success the key and it's length in bytes, are returned.
     *                              Refer to r_rsa_api.h for RSA public key sizes.
     *                              Refer to r_ecc_api.h for ECC public key sizes.
     *                              Should set to NULL for AES.
     *                              p_public_key should be WORD aligned. The memory allocation to store the public
     *                              key is user's responsibility.
     */
    ssp_err_t (* keyGenerate)(sf_crypto_key_ctrl_t * const p_ctrl,
                              sf_crypto_key_t * const p_secret_key,
                              sf_crypto_key_t * const p_public_key);
    /** Perform scalar multiplication for ECC algorithms only. This is a blocking call.
     * @par Implemented as
     * - SF_CRYPTO_KEY_EcdhSharedSecretCompute
     * @param[in]     p_ctrl              Pointer to Crypto Key Framework control block structure.
     * @param[in]     p_local_secret_key  Pointer to a secret key structure.
     *                                    The pointer to the secret key and it's length in bytes, are to be populated on input.
     *                                    Refer to r_ecc_api.h for ECC key sizes.
     *                                    p_secret_key should be WORD aligned. The memory allocation to store the secret
     *                                    key is user's responsibility.
     * @param[in]     p_remote_public_key pointer to a point on the curve data.
     *                                    The pointer to the point on curve data and its length in bytes, are to be populated
     *                                    on input. Refer to r_ecc_api.h for ECC point on curve sizes.
     *                                    p_point_on_curve should be WORD aligned. The memory allocation to store the point
     *                                    on curve data is user's responsibility.
     * @param[in,out] p_shared_secret     The pointer to the buffer and it's length in bytes, are to be populated on input.
     *                                    On success the resultant point on curve data and it's length in bytes, are returned.
     *                                    Refer to r_ecc_api.h for ECC public key sizes.
     *                                    p_resultant_vector should be WORD aligned. The memory allocation to store the resultant
     *                                    point on curve data is user's responsibility.
     *
     */
    ssp_err_t (* EcdhSharedSecretCompute)(sf_crypto_key_ctrl_t * const p_ctrl,
                                          sf_crypto_key_t * const p_local_secret_key,
                                          sf_crypto_key_t * const p_remote_public_key,
                                          sf_crypto_key_t * const p_shared_secret);
    /** Get version of SSP Crypto Key framework.
    * @par Implemented as
    * - SF_CRYPTO_KEY_VersionGet()
    * @param[out]      p_version     Pointer to the memory to store the module version.
    */
    ssp_err_t (* versionGet)(ssp_version_t * const p_version);

} sf_crypto_key_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_sf_crypto_key_instance
{
    sf_crypto_key_ctrl_t      * p_ctrl;    ///< Pointer to the control structure for this instance
    sf_crypto_key_cfg_t       * p_cfg;     ///< Pointer to the configuration structure for this instance
    sf_crypto_key_api_t const * p_api;     ///< Pointer to the API structure for this instance
} sf_crypto_key_instance_t;

/*******************************************************************************************************************//**
 *  @} (end defgroup SF_CRYPTO_KEY_API)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_CRYPTO_KEY_API_H */
