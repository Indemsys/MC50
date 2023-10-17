/***********************************************************************************************************************
 * Copyright [2015-2023] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * Unless otherwise expressly agreed in writing between Renesas and you: 1) you may not use, copy, modify, distribute,
 * display, or perform the contents; 2) you may not use any name or mark of Renesas for advertising or publicity
 * purposes or in connection with your use of the contents; 3) RENESAS MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE
 * SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED "AS IS" WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 * NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF CONTRACT OR TORT, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents included in this file may
 * be subject to different terms.
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * File Name    : r_arc4_api.h
 * Description  : ARC4 interface file
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Crypto_API
 * @defgroup ARC4_API ARC4 Interface
 *
 * @brief ARC4 encryption and decryption APIs
 *
 * @{
 **********************************************************************************************************************/

#ifndef DRV_ARC4_API_H
#define DRV_ARC4_API_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
/** Register definitions, common services and error codes. */
#include "bsp_api.h"
#include "r_crypto_api.h"


/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define ARC4_API_VERSION_MAJOR (1U)
#define ARC4_API_VERSION_MINOR (0U)

#define DRV_ARC4_CONTEXT_BUFFER_SIZE (66U)


/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** ARC4 Interface control structure */
typedef struct st_arc4_ctrl
{
    crypto_ctrl_t       * p_crypto_ctrl;  //!< pointer to crypto engine control structure
    crypto_api_t const  * p_crypto_api;   //!< pointer to crypto engine API
    uint32_t              state;          //!< used to identify state of the ARC4 control block
    uint32_t              work_buffer[DRV_ARC4_CONTEXT_BUFFER_SIZE];  //! used for storing context of the cipher
                                          //!< ARC4 uses this for storing the sbox results for the next encrypt/
                                          //!< decrypt operations
    bsp_lock_t            open;           //!< indicates whether driver is opened with this control block
} arc4_ctrl_t;

/** ARC4 Interface configuration structure. User must fill in these values before invoking the open() function */
typedef struct st_arc4_cfg
{
    crypto_api_t const  * p_crypto_api;   //!< pointer to crypto engine api
    uint32_t              length;         //!< Length of p_key in bytes
    uint8_t const       * p_key;          //!< ARC4 key to use for encrypt or decrypto operations
} arc4_cfg_t;

/** ARC4_Interface SCE functions implemented at the HAL layer will follow this API. */
typedef struct st_arc4_api
{
    /** ARC4 module open function. Must be called before performing any encrypt/decrypt operations.
     *  Initializes the context for the encrypt or decrypt operations using the chosen Cipher interface.
     *
     * @param[in,out] p_ctrl pointer to control structure for the ARC4 interface. Must be declared by user.
     *                       Elements are set here.
     * @param[in]     p_cfg  pointer to control structure for the ARC4 configuration. All elements of this structure
     *                       must be set by user.
     *
     */
    uint32_t (* open)(arc4_ctrl_t * const p_ctrl, arc4_cfg_t const * const p_cfg);

    /** ARC4 module key set function. Resets the state of the ARC4 encryption block.
     *
     * @param[in,out] p_ctrl pointer to control structure for the ARC4 interface.
     * @param[in]     length length of the `p_key` key material in bytes
     * @param[in]     p_key  pointer to the key material to use for encryption operations.
     */
    uint32_t (* keySet)(arc4_ctrl_t * const p_ctrl, uint32_t length, uint8_t const * p_key);


    /** @brief Encrypt or decrypt source data `p_source` of length `num_bytes` and write the results to destination buffer `p_dest`
     *
     * @param[in,out] p_ctrl     pointer to control structure for ARC4 interface.
     * @param[in]     num_bytes  number of bytes to encrypt or decrypt, the value must be a multiple of 16
     * @param[in]     p_source   pointer to source data buffer
     * @param[out]    p_dest     pointer to destination data buffer
     */
    uint32_t (* arc4Process)(arc4_ctrl_t  * const p_ctrl,
                         uint32_t             num_bytes,
                         uint8_t            * p_source,
                         uint8_t            * p_dest);

    /** Close the ARC4 module.
     *  @param[in,out] p_ctrl pointer to the control structure
     */
    uint32_t (* close)(arc4_ctrl_t * const p_ctrl);

    /** Gets version and stores it in provided pointer p_version.
     * @param[out]    p_version  Code and API version used.
     */
    uint32_t (* versionGet)(ssp_version_t * const p_version);

} arc4_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_arc4_instance
{
    arc4_ctrl_t              * p_ctrl;         ///< Pointer to the control structure for this instance
    arc4_cfg_t         const * p_cfg;          ///< Pointer to the configuration structure for this instance
    arc4_api_t         const * p_api;          ///< Pointer to the API structure for this instance
} arc4_instance_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
extern const crypto_api_t g_sce_crypto_api;
/** ARC4 interface is only available on S7G2, S5D9 and S5D5. */
#if defined(BSP_MCU_GROUP_S7G2) || defined(BSP_MCU_GROUP_S5D9) || defined(BSP_MCU_GROUP_S5D5) || defined(BSP_MCU_GROUP_S5D3)
extern const arc4_api_t g_arc4_on_sce;
#endif

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* DRV_ARC4_API_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup ARC4_API)
 **********************************************************************************************************************/
