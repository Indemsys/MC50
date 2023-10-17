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
 * File Name    : r_tdes_api.h
 * Description  : TDES_Interface
 ***********************************************************************************************************************
 * History : DD.MM.YYYY Version Description
 *           TDES_Interface  1.00    Initial Release.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Crypto_API
 * @defgroup TDES_API TDES Interface
 *
 * @brief TDES encryption and decryption APIs
 *
 * @{
 **********************************************************************************************************************/

#ifndef DRV_TDES_API_H
#define DRV_TDES_API_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
/** Register definitions, common services and error codes. */
#include "bsp_api.h"
#include "r_crypto_api.h"
#include "r_tdes_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define TDES_API_VERSION_MAJOR (1U)
#define TDES_API_VERSION_MINOR (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** TDES Interface control structure */
typedef struct st_tdes_ctrl
{
    crypto_ctrl_t       crypto_ctrl;    //!< pointer to crypto control structure
    crypto_api_t const  * p_crypto_api; //!< pointer to crypto engine API
} tdes_ctrl_t;

/** TDES Interface configuration structure. User must fill in these values before invoking the open() function */
typedef struct st_tdes_cfg
{
    crypto_api_t const  * p_crypto_api; //!< pointer to crypto engine api
} tdes_cfg_t;

/** TDES_Interface SCE functions implemented at the HAL layer will follow this API. */
typedef struct st_tdes_api
{
    /** TDES module open function. Must be called before performing any encrypt/decrypt operations.
     *
     * @param[in,out] p_ctrl pointer to control structure for the TDES interface. Must be declared by user.
     *                       Elements are set here.
     * @param[in]     p_cfg  pointer to control structure for the TDES configuration. All elements of this structure
     *                       must be set by user.
     *
     */
    uint32_t (* open)(tdes_ctrl_t * const p_ctrl, tdes_cfg_t const * const p_cfg);

    /** @brief TDES encryption.
     *
     * Encrypt input data with a 192-bit TDES key and the chaining mode specified.
     *
     * @param[in]       *p_key      pointer to the TDES plain-text key.
     * @param[in, out]  *p_iv       pointer to initialization vector. Should be 8 bytes long. Unused for ECB mode.
     *                              Next IV value is returned on successful completion.
     * @param[in]       num_words   Specifies the size of the input data in words. Should be multiples of 2.
     *                              Note: 1 word is 4-bytes long.
     * @param[in]       *p_source   input data buffer - should be at least num_words long.
     * @param[out]      *p_dest     output data buffer - should be at least num_words long.
     */
    uint32_t (* encrypt)(tdes_ctrl_t * const p_ctrl, const uint32_t * p_key, uint32_t * p_iv,
                              uint32_t num_words, uint32_t * p_source, uint32_t * p_dest);

    /** @brief TDES decryption.
     *
     * Decrypt input data with a 192-bit TDES key and the chaining mode specified.
     * @param[in]       *p_key      pointer to the 192-bit plain-text key
     * @param[in, out]  *p_iv       pointer to initialization vector. Should be 8 bytes long. Unused for ECB mode.
     *                              Next IV value is returned on successful completion.
     * @param[in]       num_words   Specifies the size of the input data in words. Should be multiples of 2.
     *                              Note: 1 word is 4-bytes long.
     * @param[in]       *p_source   input data buffer - should be at least num_words long.
     * @param[out]      *p_dest     output data buffer - should be at least num_words long.
     */
    uint32_t (* decrypt)(tdes_ctrl_t * const p_ctrl, const uint32_t * p_key, uint32_t * p_iv,
                             uint32_t num_words, uint32_t * p_source, uint32_t * p_dest);

    /** Close the TDES module.
     *  @param[in] p_ctrl pointer to the control structure
     */
    uint32_t (* close)(tdes_ctrl_t * const p_ctrl);

    /** Gets version and stores it in provided pointer p_version.
     * @param[out]    p_version  Code and API version used.
     */
    uint32_t (* versionGet)(ssp_version_t * const p_version);
} tdes_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_tdes_instance {
    tdes_ctrl_t   * p_ctrl ;         ///< Pointer to the control structure for this instance
    tdes_cfg_t    const * p_cfg  ;   ///< Pointer to the configuration structure for this instance
    tdes_api_t    const * p_api  ;   ///< Pointer to the API structure for this instance
} tdes_instance_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
extern const crypto_api_t g_sce_crypto_api;

/** TDES interface is only available on S7G2 and S5D9. */
#if defined(BSP_MCU_GROUP_S7G2) || defined(BSP_MCU_GROUP_S5D9) || defined(BSP_MCU_GROUP_S5D5) || defined(BSP_MCU_GROUP_S5D3)
extern const tdes_api_t   g_tdes192ecb_on_sce;
extern const tdes_api_t   g_tdes192cbc_on_sce;
extern const tdes_api_t   g_tdes192ctr_on_sce;
#endif // defined(BSP_MCU_GROUP_S7G2) || defined(BSP_MCU_GROUP_S5D9) || defined(BSP_MCU_GROUP_S5D5) || defined(BSP_MCU_GROUP_S5D3)

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* DRV_TDES_API_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup TDES_API)
 **********************************************************************************************************************/
