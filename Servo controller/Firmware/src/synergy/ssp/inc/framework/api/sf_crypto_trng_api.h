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
/***********************************************************************************************************************
 * File Name    : sf_crypto_trng_api.h
 * Description  : Crypto TRNG Framework API.
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup SF_Interface_Library
 * @defgroup SF_CRYPTO_TRNG_API SSP Crypto TRNG Framework Interface
 * @brief Interface definition for Synergy Crypto TRNG Framework module.
 *
 * @section SF_CRYPTO_TRNG_API_SUMMARY Summary
 * This is the Interface of SF_CRYPTO_TRNG Framework module.
 *
 * Crypto TRNG Framework Interface description: @ref FrameworkCrypto
 *
 * @{
 **********************************************************************************************************************/

#ifndef SF_CRYPTO_TRNG_API_H
#define SF_CRYPTO_TRNG_API_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "sf_crypto_api.h"
#include "r_trng_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** The API version of SSP Crypto Framework */
#define SF_CRYPTO_TRNG_API_VERSION_MAJOR       (2U)
#define SF_CRYPTO_TRNG_API_VERSION_MINOR       (0U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** State codes for the SSP Crypto TRNG framework */
typedef enum e_sf_crypto_trng_state
{
    SF_CRYPTO_TRNG_CLOSED          = 0,     ///< Crypto TRNG Framework Module is closed
    SF_CRYPTO_TRNG_OPENED          = 1,     ///< Crypto TRNG Framework Module is opened
} sf_crypto_trng_state_t;

/** SSP Crypto TRNG framework control block.
* @par Implemented as
* - sf_crypto_trng_ctrl_t
*/
typedef void sf_crypto_trng_ctrl_t;

/** Configuration structure for the SSP Crypto TRNG framework */
typedef struct st_sf_crypto_trng_cfg
{
    sf_crypto_instance_t * p_lower_lvl_common;      ///< Pointer to a Crypto Framework common instance
    trng_instance_t      * p_lower_lvl_instance;    ///< Pointer to Crypto TRNG HAL instance
    void                 * p_extend;                ///< Pointer to an optional configuration for HW specific settings.
}sf_crypto_trng_cfg_t;

/** Shared Interface definition for the SSP Crypto framework */
typedef struct st_sf_crypto_trng_api
{
    /** Open SSP Crypto TRNG framework for true random number generation.
     * @par Implemented as
     * - SF_CRYPTO_TRNG_Open()
     * @param[in,out]  p_ctrl_api   Pointer to Crypto TRNG Framework control block structure.
     * @param[in]      p_cfg        Pointer to sf_crypto_trng_cfg_t configuration structure. All elements of this
     *                              structure must be set by user.
     */
    ssp_err_t (*open)(sf_crypto_trng_ctrl_t * const p_ctrl, sf_crypto_trng_cfg_t const * const p_cfg);

    /** Close SSP Crypto TRNG framework. This API should be called once TRNG services are no longer needed.
     * @par Implemented as
     * - SF_CRYPTO_TRNG_Close()
     * @param[in,out]  p_ctrl_api   Pointer to Crypto TRNG Framework control block structure.
     */
    ssp_err_t (*close)(sf_crypto_trng_ctrl_t * const p_ctrl);

    /** Generate a True Random Number of specified size
     * @par Implemented as
     * - SF_CRYPTO_TRNG_RandomNumberGenerate()
     *  @param[in]     p_ctrl_api              Pointer to Crypto TRNG Framework control block structure.
     *  @param[in,out] p_random_number_buff    Pointer to sf_crypto_data_handle_t structure storing pointer
     *                                         to buffer and its size where true random number will be returned.
     *  @note Size value specified under p_random_number_buff must be specified in Bytes.
     *  @note Size value specified under p_random_number_buff must not be 0 Bytes. Its minimum
     *        value is 1.
     *  @note Pointer to data buffer specified under p_random_number_buff must not be NULL.
     *  @note Data buffer must be WORD aligned. The memory allocation to store the true random number is user's
     *        responsibility.
     */
    ssp_err_t (*randomNumberGenerate)(sf_crypto_trng_ctrl_t * const p_ctrl,
                                      sf_crypto_data_handle_t * const p_random_number_buff);

    /** Get version of SSP Crypto TRNG Framework Module.
    * @par Implemented as
    * - SF_CRYPTO_TRNG_VersionGet()
    * @param[out]      p_version     Pointer to the memory to store the module version.
    */
    ssp_err_t (*versionGet)(ssp_version_t * const p_version);
}sf_crypto_trng_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct
{
    sf_crypto_trng_ctrl_t         *p_ctrl;  ///< Pointer to the control structure for this instance
    sf_crypto_trng_cfg_t          *p_cfg;   ///< Pointer to the configuration structure for this instance
    sf_crypto_trng_api_t const    *p_api;   ///< Pointer to the API structure for this instance
}sf_crypto_trng_instance_t;

/** @} (end defgroup SF_CRYPTO_TRNG_API) */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /*SF_CRYPTO_TRNG_API_H*/
