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
* File Name    : r_sce.h
* Description  : Secure crypto engine control APIs
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @file
***********************************************************************************************************************/


#ifndef R_SCE_H
#define R_SCE_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "bsp_api.h"

#include "r_sce_cfg.h"
#include "r_crypto_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define SCE_CODE_VERSION_MAJOR   (1U)
#define SCE_CODE_VERSION_MINOR   (0U)

#define SCE_API_VERSION_MAJOR    (1U)
#define SCE_API_VERSION_MINOR    (0U)

#define SCE_MIN(x, y) (((x) < (y)) ? (x) : (y))
#define SCE_MAX(x, y) (((x) > (y)) ? (x) : (y))

/**********************************************************************************************************************
Exported global variables
***********************************************************************************************************************/
/** Crypto API interface on SCE */
/** @cond INC_HEADER_DEFS_SEC */
extern const crypto_api_t g_sce_crypto_api;
/** @endcond  */

/*******************************************************************************************************************//**
 * @addtogroup SCE
 * @{
***********************************************************************************************************************/

/**********************************************************************************************************************
Function Prototypes
***********************************************************************************************************************/

extern uint32_t R_SCE_Open(crypto_ctrl_t      * const p_ctrl,
                            crypto_cfg_t const * const p_cfg);

extern uint32_t R_SCE_VersionGet(ssp_version_t * const p_version);

extern uint32_t R_SCE_StatusGet(uint32_t *p_status);

extern uint32_t R_SCE_Close(crypto_ctrl_t * const p_ctrl);

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
#if defined(BSP_FEATURE_HAS_SCE_ON_S1)  /* Crypto on S1 */
typedef struct st_sce_interface_get_api_interfaces
{
    const uint32_t aes128ecb_on_sce;
    const uint32_t aes128cbc_on_sce;
    const uint32_t aes128ctr_on_sce;
    const uint32_t aes256ecb_on_sce;
    const uint32_t aes256cbc_on_sce;
    const uint32_t aes256ctr_on_sce;
    const uint32_t trng_on_sce;
}sce_interface_get_api_interfaces_t;
#elif defined(BSP_FEATURE_HAS_SCE5) /* SCE5: S3 */
typedef struct st_sce_interface_get_api_interfaces
{
    const uint32_t aes128ecb_on_sce;
    const uint32_t aes128cbc_on_sce;
    const uint32_t aes128ctr_on_sce;
    const uint32_t aes128gcm_on_sce;
    const uint32_t aes128xts_on_sce;
    const uint32_t aes256ecb_on_sce;
    const uint32_t aes256cbc_on_sce;
    const uint32_t aes256ctr_on_sce;
    const uint32_t aes256gcm_on_sce;
    const uint32_t aes256xts_on_sce;
    const uint32_t aes128ecb_on_sceHrk;
    const uint32_t aes128cbc_on_sceHrk;
    const uint32_t aes128ctr_on_sceHrk;
    const uint32_t aes128gcm_on_sceHrk;
    const uint32_t aes128xts_on_sceHrk;
    const uint32_t aes192ecb_on_sceHrk;
    const uint32_t aes192cbc_on_sceHrk;
    const uint32_t aes192ctr_on_sceHrk;
    const uint32_t aes192gcm_on_sceHrk;
    const uint32_t aes256ecb_on_sceHrk;
    const uint32_t aes256cbc_on_sceHrk;
    const uint32_t aes256ctr_on_sceHrk;
    const uint32_t aes256gcm_on_sceHrk;
    const uint32_t aes256xts_on_sceHrk;
    const uint32_t trng_on_sce;
}sce_interface_get_api_interfaces_t;
#else /* SCE7: For S5 and S7 */
typedef struct st_sce_interface_get_api_interfaces
{
    const uint32_t aes128ecb_on_sce;
    const uint32_t aes128cbc_on_sce;
    const uint32_t aes128ctr_on_sce;
    const uint32_t aes128gcm_on_sce;
    const uint32_t aes128xts_on_sce;
    const uint32_t aes192ecb_on_sce;
    const uint32_t aes192cbc_on_sce;
    const uint32_t aes192ctr_on_sce;
    const uint32_t aes192gcm_on_sce;
    const uint32_t aes256ecb_on_sce;
    const uint32_t aes256cbc_on_sce;
    const uint32_t aes256ctr_on_sce;
    const uint32_t aes256gcm_on_sce;
    const uint32_t aes256xts_on_sce;
    const uint32_t aes128ecb_on_sceHrk;
    const uint32_t aes128cbc_on_sceHrk;
    const uint32_t aes128ctr_on_sceHrk;
    const uint32_t aes128gcm_on_sceHrk;
    const uint32_t aes128xts_on_sceHrk;
    const uint32_t aes192ecb_on_sceHrk;
    const uint32_t aes192cbc_on_sceHrk;
    const uint32_t aes192ctr_on_sceHrk;
    const uint32_t aes192gcm_on_sceHrk;
    const uint32_t aes256ecb_on_sceHrk;
    const uint32_t aes256cbc_on_sceHrk;
    const uint32_t aes256ctr_on_sceHrk;
    const uint32_t aes256gcm_on_sceHrk;
    const uint32_t aes256xts_on_sceHrk;
    const uint32_t rsa1024_on_sce;
    const uint32_t rsa2048_on_sce;
    const uint32_t rsa1024_on_sce_hrk;
    const uint32_t rsa2048_on_sce_hrk;
    const uint32_t md5_hash_on_sce;
    const uint32_t sha1_hash_on_sce;
    const uint32_t sha256_hash_on_sce;
    const uint32_t ecc192_on_sce;
    const uint32_t ecc224_on_sce;
    const uint32_t ecc256_on_sce;
    const uint32_t ecc384_on_sce;
    const uint32_t ecc192_on_sce_hrk;
    const uint32_t ecc224_on_sce_hrk;
    const uint32_t ecc256_on_sce_hrk;
    const uint32_t ecc384_on_sce_hrk;
    const uint32_t trng_on_sce;
}sce_interface_get_api_interfaces_t;
#endif
/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // R_SCE_H

/*******************************************************************************************************************//**
 * @} (end addtogroup SCE)
***********************************************************************************************************************/
