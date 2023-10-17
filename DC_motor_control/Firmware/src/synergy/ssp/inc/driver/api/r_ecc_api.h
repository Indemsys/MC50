/*****************-******************************************************************************************************
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
 * File Name    : r_ecc_api.h
 * Description  : ECC Interface
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup Crypto_API
 * @defgroup ECC_API ECC Interface
 *
 * @brief ECC cryptographic functions for scalar multiplication, generate key,
 *        generate sign, verify sign and version get.
 *
 * @{
 **********************************************************************************************************************/

#ifndef DRV_ECC_API_H
#define DRV_ECC_API_H

/** Register definitions, common services and error codes. */
#include "bsp_api.h"
#include "r_crypto_api.h"

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define ECC_API_VERSION_MAJOR (1U)
#define ECC_API_VERSION_MINOR (0U)

/* ECC P-192 */
#define ECC_192_DOMAIN_PARAMETER_WITH_ORDER_LENGTH_WORDS        (24U)
#define ECC_192_DOMAIN_PARAMETER_WITHOUT_ORDER_LENGTH_WORDS     (18U)
#define ECC_192_POINT_ON_CURVE_LENGTH_WORDS                     (12U)
#define ECC_192_GENERATOR_POINT_LENGTH_WORDS                    (12U)
#define ECC_192_PUBLIC_KEY_LENGTH_WORDS                         (12U)
#define ECC_192_PRIVATE_KEY_LENGTH_WORDS                        (6U)
#define ECC_192_MESSAGE_DIGEST_LENGTH_WORDS                     (6U)
#define ECC_192_SIGNATURE_R_LENGTH_WORDS                        (6U)
#define ECC_192_SIGNATURE_S_LENGTH_WORDS                        (6U)
#define ECC_192_PRIVATE_KEY_HRK_LENGTH_WORDS                    (13U)

/* ECC P-224 */
#define ECC_224_DOMAIN_PARAMETER_WITH_ORDER_LENGTH_WORDS        (28U)
#define ECC_224_DOMAIN_PARAMETER_WITHOUT_ORDER_LENGTH_WORDS     (21U)
#define ECC_224_POINT_ON_CURVE_LENGTH_WORDS                     (14U)
#define ECC_224_GENERATOR_POINT_LENGTH_WORDS                    (14U)
#define ECC_224_PUBLIC_KEY_LENGTH_WORDS                         (14U)
#define ECC_224_PRIVATE_KEY_LENGTH_WORDS                        (7U)
#define ECC_224_MESSAGE_DIGEST_LENGTH_WORDS                     (7U)
#define ECC_224_SIGNATURE_R_LENGTH_WORDS                        (7U)
#define ECC_224_SIGNATURE_S_LENGTH_WORDS                        (7U)
#define ECC_224_PRIVATE_KEY_HRK_LENGTH_WORDS                    (13U)

/* ECC P-256 */
#define ECC_256_DOMAIN_PARAMETER_WITH_ORDER_LENGTH_WORDS        (32U)
#define ECC_256_DOMAIN_PARAMETER_WITHOUT_ORDER_LENGTH_WORDS     (24U)
#define ECC_256_POINT_ON_CURVE_LENGTH_WORDS                     (16U)
#define ECC_256_GENERATOR_POINT_LENGTH_WORDS                    (16U)
#define ECC_256_PUBLIC_KEY_LENGTH_WORDS                         (16U)
#define ECC_256_PRIVATE_KEY_LENGTH_WORDS                        (8U)
#define ECC_256_MESSAGE_DIGEST_LENGTH_WORDS                     (8U)
#define ECC_256_SIGNATURE_R_LENGTH_WORDS                        (8U)
#define ECC_256_SIGNATURE_S_LENGTH_WORDS                        (8U)
#define ECC_256_PRIVATE_KEY_HRK_LENGTH_WORDS                    (13U)

/* ECC P-384 */
#define ECC_384_DOMAIN_PARAMETER_WITH_ORDER_LENGTH_WORDS        (48U)
#define ECC_384_DOMAIN_PARAMETER_WITHOUT_ORDER_LENGTH_WORDS     (36U)
#define ECC_384_POINT_ON_CURVE_LENGTH_WORDS                     (24U)
#define ECC_384_GENERATOR_POINT_LENGTH_WORDS                    (24U)
#define ECC_384_PUBLIC_KEY_LENGTH_WORDS                         (24U)
#define ECC_384_PRIVATE_KEY_LENGTH_WORDS                        (12U)
#define ECC_384_MESSAGE_DIGEST_LENGTH_WORDS                     (12U)
#define ECC_384_SIGNATURE_R_LENGTH_WORDS                        (12U)
#define ECC_384_SIGNATURE_S_LENGTH_WORDS                        (12U)
#define ECC_384_PRIVATE_KEY_HRK_LENGTH_WORDS                    (17U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** ECC Interface control structure. */
typedef struct st_ecc_ctrl
{
    crypto_ctrl_t       * p_crypto_ctrl;    ///< Pointer to crypto engine control structure
    crypto_api_t const  * p_crypto_api;     ///< Pointer to crypto engine API
}ecc_ctrl_t;

/** ECC Interface configuration structure. */
typedef struct st_ecc_cfg
{
    crypto_api_t const    * p_crypto_api;    ///< Pointer to crypto engine API
}ecc_cfg_t;

/** ECC_Interface SCE functions implemented at the HAL layer will follow this API. */
typedef struct st_ecc_api
{
    /**
     * @brief Open the ECC driver. This API must be called before performing any ECC operations.
     * @param[in]  p_ctrl    Pointer to control structure. Must be allocated by user
     *                       before calling the API. Elements are set here.
     * @param[in]  p_cfg     Pointer to configuration structure. All elements of this structure
     *                       must be set by user before calling this API.
     */
    ssp_err_t (* open)(ecc_ctrl_t * const p_ctrl,
                       ecc_cfg_t const * const p_cfg);

    /**
     * @brief Close the ECC module.
     * @param[in] p_ctrl    Pointer to the control structure.
     */
    ssp_err_t (* close)(ecc_ctrl_t * const p_ctrl);

    /**
     * @brief scalarMultiplication: This API calculates R=kP.
     * @param[in]      p_ctrl    Pointer to control structure for the ECC interface.
     * @param[in]      p_domain  a||b||p - These are domain parameters for ECC as defined in IEEE1363.
     * @param[in]      p_k       Scalar.
     * @param[in]      p_p       Px||Py, where, Px and Py are x and y coordinates respectively.
     * @param[in, out] p_r       Rx||Ry (R=kP), where, Rx and Ry are x and y coordinates respectively.
     *                           Data length in words is updated in the output buffer data handle.
     */
    ssp_err_t (* scalarMultiplication)(ecc_ctrl_t * const p_ctrl,
                                       r_crypto_data_handle_t * const p_domain,
                                       r_crypto_data_handle_t * const p_k,
                                       r_crypto_data_handle_t * const p_p,
                                       r_crypto_data_handle_t * const p_r);
    /**
     * @brief keyCreate: This API generates key pair for ECC.
     * @param[in]       p_ctrl             Pointer to control structure for the ECC interface.
     * @param[in]       p_domain           a||b||p||n - These are domain parameters for ECC as defined in FIPS186-3.
     * @param[in]       p_generator_point  Gx||Gy - Base point of the curve,
     *                                     where, Gx and Gy are x and y coordinates respectively.
     *                                     This parameter is one of the domain parameters.
     * @param[in, out]  p_key_private      Private Key generated.
     *                                     Data length in words is updated in the output buffer data handle.
     * @param[in, out]  p_key_public       Public Key generated.
     *                                     Data length in words is updated in the output buffer data handle.
     */
    ssp_err_t (* keyCreate)(ecc_ctrl_t * const p_ctrl,
                            r_crypto_data_handle_t * const p_domain,
                            r_crypto_data_handle_t * const p_generator_point,
                            r_crypto_data_handle_t * const p_key_private,
                            r_crypto_data_handle_t * const p_key_public);

    /**
     * @brief sign: This API generates signature of ECDSA
     * @param[in]       p_ctrl             Pointer to control structure for the ECC interface.
     * @param[in]       p_domain           a||b||p||n - These are domain parameters for ECC as defined in IEEE1363.
     * @param[in]       p_generator_point  Gx||Gy - Base point of the curve,
     *                                     where, Gx and Gy are x and y coordinates respectively.
     *                                     This parameter is one of the domain parameters.
     * @param[in]       p_key_private     Private Key to process signature generation.
     * @param[in]       msg_digest        Message Digest. Length of this buffer must be equal to the
     *                                    ECC curve size in words.
     * @param[in, out]  signature_r       Signature r generated.
     *                                    Data length in words is updated in the output buffer data handle.
     * @param[in, out]  signature_s       Signature s generated.
     *                                    Data length in words is updated in the output buffer data handle.
     */
    ssp_err_t (* sign)(ecc_ctrl_t * const p_ctrl,
                       r_crypto_data_handle_t * const p_domain,
                       r_crypto_data_handle_t * const p_generator_point,
                       r_crypto_data_handle_t * const p_key_private,
                       r_crypto_data_handle_t * const msg_digest,
                       r_crypto_data_handle_t * const signature_r,
                       r_crypto_data_handle_t * const signature_s);

    /**
     * @brief verify: This is a procedure for signature verification of ECDSA.
     * @param[in]  p_ctrl             Pointer to control structure for the ECC interface.
     * @param[in]  p_domain           a||b||p||n - These are domain parameters for ECC as defined in IEEE1363.
     * @param[in]  p_generator_point  Gx||Gy - Base point of the curve,
     *                                where, Gx and Gy are x and y coordinates respectively.
     *                                This parameter is one of the domain parameters.
     * @param[in]  p_key_public       Public Key for signature verification.
     * @param[in]  msg_digest         Padded Message Digest. Length of this buffer must be equal to the
     *                                ECC curve size in words.
     * @param[in]  signature_r        Signature r.
     * @param[in]  signature_s        Signature s.
     */
    ssp_err_t (* verify)(ecc_ctrl_t * const p_ctrl,
                         r_crypto_data_handle_t * const p_domain,
                         r_crypto_data_handle_t * const p_generator_point,
                         r_crypto_data_handle_t * const p_key_public,
                         r_crypto_data_handle_t * const msg_digest,
                         r_crypto_data_handle_t *const signature_r,
                         r_crypto_data_handle_t * const signature_s);

    /**
     * @brief versionGet: Gets version and stores it in provided pointer p_version.
     * @param[out]  p_version   Code and API version used.
     */
    ssp_err_t (* versionGet)(ssp_version_t * const p_version);
}ecc_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_ecc_instance {
    ecc_ctrl_t   * p_ctrl ;         ///< Pointer to the control structure for this instance
    ecc_cfg_t    const * p_cfg  ;   ///< Pointer to the configuration structure for this instance
    ecc_api_t    const * p_api  ;   ///< Pointer to the API structure for this instance
} ecc_instance_t;

/** ECC interface is only available on S7G2, S5D9 and S5D5. */
#if defined(BSP_MCU_GROUP_S7G2) || defined(BSP_MCU_GROUP_S5D9) || defined(BSP_MCU_GROUP_S5D5) || defined(BSP_MCU_GROUP_S5D3)
extern const ecc_api_t g_ecc192_on_sce;
extern const ecc_api_t g_ecc192_on_sce_hrk;
extern const ecc_api_t g_ecc224_on_sce;
extern const ecc_api_t g_ecc224_on_sce_hrk;
extern const ecc_api_t g_ecc256_on_sce;
extern const ecc_api_t g_ecc256_on_sce_hrk;
extern const ecc_api_t g_ecc384_on_sce;
extern const ecc_api_t g_ecc384_on_sce_hrk;
#endif /* defined(BSP_MCU_GROUP_S7G2) || defined(BSP_MCU_GROUP_S5D9) || defined(BSP_MCU_GROUP_S5D5) || defined(BSP_MCU_GROUP_S5D3) */
#endif /* DRV_ECC_API_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup ECC_API)
 **********************************************************************************************************************/
