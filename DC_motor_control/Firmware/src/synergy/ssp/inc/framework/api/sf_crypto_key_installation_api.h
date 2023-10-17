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
 * File Name    : sf_crypto_key_installation_api.h
 * Description  : Interface definition for SSP Crypto Key Installation Framework.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup SF_Interface_Library
 * @defgroup SF_CRYPTO_KEY_INSTALLATION_API SSP Crypto Key Installation Framework Interface
 * @brief Interface definition for Synergy Crypto Key Installation Framework module.
 *
 * @section SF_CRYPTO_KEY_INSTALLATION_API_SUMMARY Summary
 * This is the Interface of SF_CRYPTO_KEY_INSTALLATION Framework module.
 * The Key Installation framework module is a ThreadX aware Framework Interface which provides key installation
 * services. This sits between the user application and HAL layer.
 *
 * Crypto Key Installation Framework Interface description: @ref FrameworkCrypto
 *
 * @{
 **********************************************************************************************************************/

#ifndef SF_CRYPTO_KEY_INSTALLATION_API_H
#define SF_CRYPTO_KEY_INSTALLATION_API_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "sf_crypto_api.h"
#include "r_crypto_api.h"
#include "r_key_installation_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** The API version of SSP Crypto Key Installation Framework */
#define SF_CRYPTO_KEY_INSTALLATION_API_VERSION_MAJOR       (2U)
#define SF_CRYPTO_KEY_INSTALLATION_API_VERSION_MINOR       (0U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** State codes for the SSP Crypto Key installation framework module. Once the module is opened successfully,
 * then the state is transition to OPENED state. After Key Installation operations, the Key installation framework
 * module must be closed with CLOSED state. */
typedef enum e_sf_crypto_key_installation_state
{
    SF_CRYPTO_KEY_INSTALLATION_CLOSED          = 0,              ///< The Key Installation module is closed.
    SF_CRYPTO_KEY_INSTALLATION_OPENED          = 1,              ///< The Key Installation module is opened.
} sf_crypto_key_installation_state_t;

/** SSP Crypto Key installation framework control block.
 * Allocate an instance specific control block to pass into the SSP Crypto Key Installation framework API calls.
 * @par Implemented as
 * - sf_crypto_key_installation_ctrl_t
 */
typedef void sf_crypto_key_installation_ctrl_t;

/** Configuration structure for the SSP SSP Crypto Key Installation framework */
typedef struct st_sf_crypto_key_installation_cfg
{
    sf_crypto_key_type_t           key_type;                  ///< Type of key to be installed.
    sf_crypto_key_size_t           key_size;                  ///< Size of key to be installed.
    sf_crypto_instance_t         * p_lower_lvl_common;        ///< Pointer to a Crypto Framework common instance.
    key_installation_instance_t  * p_lower_lvl_instance;      ///< Pointer to Crypto Key Install HAL instance
    void const                   * p_extend;                  ///< Extension parameter for hardware specific settings (Future purpose).
} sf_crypto_key_installation_cfg_t;

/** Supported shared key index values */
typedef enum sf_crypto_key_installation_shared_index
{
    SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_0,   ///< Shared Key Index 0
    SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_1,   ///< Shared Key Index 1
    SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_2,   ///< Shared Key Index 2
    SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_3,   ///< Shared Key Index 3
    SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_4,   ///< Shared Key Index 4
    SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_5,   ///< Shared Key Index 5
    SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_6,   ///< Shared Key Index 6
    SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_7,   ///< Shared Key Index 7
    SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_8,   ///< Shared Key Index 8
    SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_9,   ///< Shared Key Index 9
    SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_A,   ///< Shared Key Index 10
    SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_B,   ///< Shared Key Index 11
    SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_C,   ///< Shared Key Index 12
    SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_D,   ///< Shared Key Index 13
    SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_E,   ///< Shared Key Index 14
    SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_F,   ///< Shared Key Index 15
} sf_crypto_key_installation_shared_index_t;

/** Shared Interface definition for the SSP Crypto Key Installation Interface framework */
typedef struct st_sf_crypto_key_installation_api
{
/** Open SSP Crypto Key Installation framework for subsequent call / Key installation.
 * @par Implemented as
 * - SF_CRYPTO_KEY_INSTALLATION_Open()
 * @param[in,out]  p_ctrl       Pointer to Crypto Key Installation Framework control block structure.
 * @param[in]      p_cfg        Pointer to sf_crypto_key_installation_cfg_t configuration structure.
 *                              All elements of this structure must be set by user.
 */
    ssp_err_t (* open)(sf_crypto_key_installation_ctrl_t * const p_ctrl,
                       sf_crypto_key_installation_cfg_t const * const p_cfg);

/** Close SSP Crypto Key Installation framework.
 * @par Implemented as
 * - SF_CRYPTO_KEY_INSTALLATION_Close()
 * @param[in,out]  p_ctrl       Pointer to Crypto Key Installation Framework control block structure.
 */
    ssp_err_t (* close)(sf_crypto_key_installation_ctrl_t * const p_ctrl);

/** Get version of SSP Crypto Key Installation framework.
 * @par Implemented as
 * - SF_CRYPTO_KEY_INSTALLATION_VersionGet()
 * @param[out]      p_version     Pointer to the memory to store the module version.
 */
   ssp_err_t (* versionGet)(ssp_version_t * const p_version);

/** Install a key from the user's encrypted key, a shared index, session key, and an IV generated using a scheme
 *  designed to maintain plaintext source key isolation. This returns a wrapped key (sometimes called a key index)
 *  that can be used in other crypto APIs in place of the associated plaintext key (stored offline).
 *  @par Implemented as
 * - SF_CRYPTO_KEY_INSTALLATION_KeyInstall()
 *  @param[in]     p_ctrl                   Pointer to Crypto Key Installation Framework control block structure.
 *                                          Caller should not modify any elements of this structure at any time.
 *  @param[in]     p_user_key_rsa_modulus   Pointer to sf_crypto_key_handle_t structure which includes a pointer to
 *                                          the WORD aligned buffer which holds the RSA modulus portion of the
 *                                          encrypted user RSA private key and the modulus length.
 *                                          This is only applicable when a RSA standard key is being installed.
 *                                          To be set to NULL otherwise.
 *  @param[in]     p_user_key_input         Pointer to sf_crypto_key_handle_t structure which includes a pointer to
 *                                          the WORD aligned buffer which holds the encrypted user key and length.
 *                                          This is the key to be installed in encrypted format.
 *  @param[in]     shared_index_input       An enumerated type that reflects the shared key index returned by the
 *                                          DLM Service, accompanied by the session key that follows.
 *  @param[in]     p_session_key_input      Pointer to sf_crypto_key_handle_t structure which includes a pointer to
 *                                          the WORD aligned buffer which holds the session key and length returned
 *                                          by the DLM Service, accompanied by the shared index key, above.
 *  @param[in]     p_iv_input               Pointer to the 128-bit IV array used to encrypt p_user_key_input.
 *  @param[in,out] p_key_data_out           Pointer to sf_crypto_key_handle_t structure which includes a pointer to
 *                                          the WORD aligned buffer to hold the wrapped key and the buffer length.
 *                                          This is the wrapped key returned after key installation.
 *  @note  It is the user's responsibility to ensure all the above input/output buffers are WORD aligned.
 *  @note  Caller must assign appropriate length to data_length field for all buffers before calling this API.
 */
   ssp_err_t (* keyInstall)(sf_crypto_key_installation_ctrl_t * const p_ctrl,
                            sf_crypto_data_handle_t const * const p_user_key_rsa_modulus,
                            sf_crypto_data_handle_t const * const p_user_key_input,
							sf_crypto_key_installation_shared_index_t const shared_index_input,
                            sf_crypto_data_handle_t const * const p_session_key_input,
                            uint32_t const * const p_iv_input,
                            sf_crypto_data_handle_t * const p_key_data_out);

} sf_crypto_key_installation_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_sf_crypto_key_installation_instance
{
    sf_crypto_key_installation_ctrl_t      * p_ctrl;    ///< Pointer to the control structure for this instance
    sf_crypto_key_installation_cfg_t       * p_cfg;     ///< Pointer to the configuration structure for this instance
    sf_crypto_key_installation_api_t const * p_api;     ///< Pointer to the API structure for this instance
} sf_crypto_key_installation_instance_t;

/** @} (end defgroup SF_CRYPTO_KEY_INSTALLATION_API) */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_CRYPTO_KEY_INSTALLATION_API_H */
