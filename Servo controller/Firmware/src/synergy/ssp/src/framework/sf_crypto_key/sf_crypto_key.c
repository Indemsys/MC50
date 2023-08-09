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
 * File Name    : sf_crypto_key.c
 * Description  : SSP Crypto Key Framework Module
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @file
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto_key.h"
#include "sf_crypto_key_private_api.h"
#include "../sf_crypto/sf_crypto_private_api.h"
#include "aes/sf_crypto_key_aes_private.h"
#include "rsa/sf_crypto_key_rsa_private.h"
#include "ecc/sf_crypto_key_ecc_private.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** "CRPT" in ASCII, used to identify Crypto Framework opened.  */
#define OPEN (0x43525054U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
#if SF_CRYPTO_KEY_CFG_PARAM_CHECKING_ENABLE
static ssp_err_t sf_crypto_key_open_param_check(sf_crypto_key_instance_ctrl_t * const p_ctrl,
                                                sf_crypto_key_cfg_t const * const p_cfg);
#endif
static ssp_err_t sf_crypto_key_hal_open(sf_crypto_key_instance_ctrl_t * const p_ctrl,
                                        sf_crypto_key_cfg_t const * const p_cfg);

static ssp_err_t sf_crypto_key_hal_close(sf_crypto_key_instance_ctrl_t * const p_ctrl);

static ssp_err_t sf_crypto_key_hal_generate(sf_crypto_key_instance_ctrl_t * const p_ctrl,
                                            sf_crypto_key_t * const p_secret_key,
                                            sf_crypto_key_t * const p_public_key);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug. This pragma suppresses the warnings in this
 * structure only.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure */
static const ssp_version_t sf_crypto_key_version =
{
    .api_version_major  = SF_CRYPTO_KEY_API_VERSION_MAJOR,
    .api_version_minor  = SF_CRYPTO_KEY_API_VERSION_MINOR,
    .code_version_major = SF_CRYPTO_KEY_CODE_VERSION_MAJOR,
    .code_version_minor = SF_CRYPTO_KEY_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/***********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/*LDRA_NOANALYSIS LDRA_INSPECTED below not working. */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const sf_crypto_key_api_t         g_sf_crypto_key_api =
{
    .open                      = SF_CRYPTO_KEY_Open,
    .close                     = SF_CRYPTO_KEY_Close,
    .keyGenerate               = SF_CRYPTO_KEY_Generate,
    .EcdhSharedSecretCompute   = SF_CRYPTO_KEY_EcdhSharedSecretCompute,
    .versionGet                = SF_CRYPTO_KEY_VersionGet
};

/*LDRA_ANALYSIS */

/*******************************************************************************************************************//**
* @ingroup SF_Library
* @addtogroup SF_CRYPTO_KEY
* @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
Functions
***********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @brief   SSP Crypto KeyFramework Open operation.
 *
 * @retval SSP_SUCCESS          The module was successfully opened.
 * @retval SSP_ERR_ASSERTION    NULL is passed through an argument.
 * @retval SSP_ERR_CRYPTO_COMMON_NOT_OPENED     Crypto Framework Common Module has yet been opened.
 * @retval SSP_ERR_ALREADY_OPEN The module has been already opened.
 * @retval SSP_ERR_UNSUPPORTED  The module does not support the key type specified by user.
 * @retval SSP_ERR_INVALID_SIZE The buffer length of one of the configuration parameters is invalid.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_KEY_Open(sf_crypto_key_ctrl_t * const p_api_ctrl, sf_crypto_key_cfg_t const * const p_cfg)
{
    ssp_err_t                         err = SSP_SUCCESS;
    sf_crypto_key_instance_ctrl_t   * p_ctrl = (sf_crypto_key_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t       * p_ctrl_common;
    sf_crypto_state_t                 common_module_status = SF_CRYPTO_CLOSED;

#if SF_CRYPTO_KEY_CFG_PARAM_CHECKING_ENABLE
    /** Verify if parameters are valid. */
    err = sf_crypto_key_open_param_check(p_ctrl, p_cfg);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
#endif

    /** Get a Crypto common control block and the interface. */
    p_ctrl->p_fwk_common_ctrl = p_cfg->p_lower_lvl_crypto_common->p_ctrl;
    p_ctrl_common = p_ctrl->p_fwk_common_ctrl;

    p_ctrl->p_fwk_common_api  = (sf_crypto_api_t *)p_cfg->p_lower_lvl_crypto_common->p_api;

    /** Check if the Crypto Framework has been opened. If not yet opened, return an error. */
    err = p_ctrl->p_fwk_common_api->statusGet(p_ctrl_common, &common_module_status);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    if (SF_CRYPTO_OPENED != common_module_status)
    {
        return SSP_ERR_CRYPTO_COMMON_NOT_OPENED;
    }

    /** Check if the Crypto key framework module has been already opened. If yes, return an error. */
    if (SF_CRYPTO_KEY_OPENED == p_ctrl->status)
    {
        return SSP_ERR_ALREADY_OPEN;
    }

    /* Note: sf_crypto.lock() is not called because the HAL driver does not access HW during open.
     * The HAL interfaceGet API is also called which does not access HW either.
     * In the future, if HW will be accessed through the HAL driver; lock should be acquired. */

    /** Set a key type, size, domain and generator point.
     * Generator point and domain are UNUSED for RSA and AES algorithms, applicable only for ECC.
    */
    p_ctrl->key_type    = p_cfg->key_type;
    p_ctrl->key_size    = p_cfg->key_size;
    p_ctrl->domain_params.p_data        = p_cfg->domain_params.p_data;
    p_ctrl->domain_params.data_length   = p_cfg->domain_params.data_length;

    p_ctrl->generator_point.p_data      = p_cfg->generator_point.p_data;
    p_ctrl->generator_point.data_length = p_cfg->generator_point.data_length;

    /** Open Crypto HAL module. */
    err = sf_crypto_key_hal_open(p_ctrl, p_cfg);
    if (SSP_SUCCESS == err)
    {
        p_ctrl->status = SF_CRYPTO_KEY_OPENED;
        sf_crypto_open_counter_increment(p_ctrl_common);
    }

    return err;

} /* End of function SF_CRYPTO_KEY_Open */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Key Framework Close operation.
 *
 * @retval SSP_SUCCESS          The module was successfully closed.
 * @retval SSP_ERR_ASSERTION    NULL is passed through the argument.
 * @retval SSP_ERR_NOT_OPEN     The module has yet been opened. Call Open API first.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_KEY_Close(sf_crypto_key_ctrl_t * const p_api_ctrl)
{
    ssp_err_t                             err = SSP_SUCCESS;
    sf_crypto_key_instance_ctrl_t       * p_ctrl = (sf_crypto_key_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t    * p_ctrl_common;

#if SF_CRYPTO_KEY_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid. */
    SSP_ASSERT(p_ctrl);
#endif

    /** Get a Crypto common control block and the interface. */
    p_ctrl_common = p_ctrl->p_fwk_common_ctrl;

    /** Check if the Crypto Framework has been opened. If not yet opened, return an error. */
    if (SF_CRYPTO_KEY_OPENED != p_ctrl->status)
    {
        return SSP_ERR_NOT_OPEN;
    }

    /* Note: sf_crypto.lock() is not called because the HAL driver does not access HW during close.
     * In the future, if HW will be accessed through the HAL driver; lock should be acquired. */

    /** Close Crypto HAL module. */
    err = sf_crypto_key_hal_close(p_ctrl);

    if (SSP_SUCCESS == err)
    {
        p_ctrl->status = SF_CRYPTO_KEY_CLOSED;
    }

    /** Decrement the open counter to enable users to close SF_CRYPTO module. */
    sf_crypto_open_counter_decrement(p_ctrl_common);
    return err;

} /* End of function SF_CRYPTO_KEY_Close */


/*******************************************************************************************************************//**
 * @brief   SSP Crypto Framework Key Generate operation.
 *
 * @retval SSP_SUCCESS          The module created a key successfully.
 * @retval SSP_ERR_ASSERTION    NULL is passed through an argument.
 * @retval SSP_ERR_NOT_OPEN     The module has yet been opened. Call Open API first.
 * @retval SSP_ERR_UNSUPPORTED  The module does not support the key type specified by user.
 * @retval SSP_ERR_INVALID_SIZE The length of the buffer supplied for the key to be generated, is insufficient.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_KEY_Generate(sf_crypto_key_ctrl_t * const p_api_ctrl,
                                 sf_crypto_key_t * const p_secret_key,
                                 sf_crypto_key_t * const p_public_key)
{
    ssp_err_t                             err = SSP_SUCCESS;
    sf_crypto_key_instance_ctrl_t       * p_ctrl = (sf_crypto_key_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t    * p_ctrl_common;
    sf_crypto_api_t              * p_api_common;

#if SF_CRYPTO_KEY_CFG_PARAM_CHECKING_ENABLE
    /** Verify if parameters are valid. */
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_secret_key);
    SSP_ASSERT(0U == ((uint32_t)p_secret_key->p_data % sizeof(uint32_t)));

    /** Verify Public Key parameter only for RSA. */
    if (p_ctrl->key_type != SF_CRYPTO_KEY_TYPE_AES_WRAPPED)
    {
        SSP_ASSERT(p_public_key);
        SSP_ASSERT(0U == ((uint32_t)p_public_key->p_data % sizeof(uint32_t)));
    }

#endif

    p_ctrl_common = p_ctrl->p_fwk_common_ctrl;
    p_api_common  = p_ctrl->p_fwk_common_api;

    /** Check if the Crypto Framework has been opened. If not yet opened, return an error. */
    if (SF_CRYPTO_KEY_OPENED != p_ctrl->status)
    {
        return SSP_ERR_NOT_OPEN;
    }

    /** Lock the module to access to Crypto HAL module. */
    err = p_api_common->lock(p_ctrl_common);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Generate a key. */
    err = sf_crypto_key_hal_generate(p_ctrl, p_secret_key, p_public_key);

    /** Unlock the module. */
    ssp_err_t err_unlock = p_api_common->unlock(p_ctrl_common);

    if (SSP_SUCCESS == err)
    {
        err = err_unlock;
    }

    return (err);
} /* End of function SF_CRYPTO_KEY_Generate */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Framework ECDH Shared Secret Computation operation.
 *
 * @retval SSP_SUCCESS          The module created a key successfully.
 * @retval SSP_ERR_ASSERTION    NULL is passed through an argument.
 * @retval SSP_ERR_NOT_OPEN     The module has yet been opened. Call Open API first.
 * @retval SSP_ERR_UNSUPPORTED  The module does not support scalar multiplication operation.
 * @retval SSP_ERR_INVALID_SIZE The length of the buffer supplied for the key to be generated, is insufficient.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_KEY_EcdhSharedSecretCompute(sf_crypto_key_ctrl_t * const p_api_ctrl,
                                                sf_crypto_key_t * const p_local_secret_key ,
                                                sf_crypto_key_t * const p_remote_public_key,
                                                sf_crypto_key_t * const p_shared_secret)
{
    ssp_err_t                             err = SSP_SUCCESS;
    sf_crypto_key_instance_ctrl_t       * p_ctrl = (sf_crypto_key_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t    * p_ctrl_common;
    sf_crypto_api_t              * p_api_common;

#if SF_CRYPTO_KEY_CFG_PARAM_CHECKING_ENABLE
    /** Verify if parameters are valid. */
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_local_secret_key);
    SSP_ASSERT(p_remote_public_key);
    SSP_ASSERT(p_shared_secret);
    SSP_ASSERT(0U == ((uint32_t)p_local_secret_key->p_data % sizeof(uint32_t)));
    SSP_ASSERT(0U == ((uint32_t)p_remote_public_key->p_data % sizeof(uint32_t)));
    SSP_ASSERT(0U == ((uint32_t)p_shared_secret->p_data % sizeof(uint32_t)));
#endif

    /** Check if ECDH computation is used with ECC, otherwise return an error. */
    if ((p_ctrl->key_type != SF_CRYPTO_KEY_TYPE_ECC_PLAIN_TEXT) &&
        (p_ctrl->key_type != SF_CRYPTO_KEY_TYPE_ECC_WRAPPED))
    {
        return SSP_ERR_UNSUPPORTED;
    }

    p_ctrl_common = p_ctrl->p_fwk_common_ctrl;
    p_api_common  = p_ctrl->p_fwk_common_api;

    /** Check if the Crypto Framework has been opened. If not yet opened, return an error. */
    if (SF_CRYPTO_KEY_OPENED != p_ctrl->status)
    {
        return SSP_ERR_NOT_OPEN;
    }

    /** Lock the module to access to Crypto HAL module. */
    err = p_api_common->lock(p_ctrl_common);

    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Perform the scalar multiplication. */
    err = sf_crypto_key_scalar_multiplication_ecc(p_ctrl, p_local_secret_key, p_remote_public_key, p_shared_secret);

    /** Unlock the module. */
    ssp_err_t err_unlock = p_api_common->unlock(p_ctrl_common);

    if (SSP_SUCCESS == err)
    {
        err = err_unlock;
    }

    return (err);
}

/*******************************************************************************************************************//**
 * @brief      Gets driver version based on compile time macros.
 *
 * @retval     SSP_SUCCESS          Successful close.
 * @retval     SSP_ERR_ASSERTION    The parameter p_version is NULL.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_KEY_VersionGet(ssp_version_t * const p_version)
{
#if SF_CRYPTO_KEY_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid. */
    SSP_ASSERT(p_version);
#endif

    p_version->version_id = sf_crypto_key_version.version_id;

    return SSP_SUCCESS;
} /* End of function SF_CRYPTO_KEY_VersionGet */

/*******************************************************************************************************************//**
 * @} (end defgroup SF_CRYPTO_KEY)
 **********************************************************************************************************************/


#if SF_CRYPTO_KEY_CFG_PARAM_CHECKING_ENABLE
/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto KeyFramework to check open API parameters.
 * This function is called by SF_CRYPTO_KEY_Open().
 *
 * @retval SSP_SUCCESS          The module was successfully opened.
 * @retval SSP_ERR_ASSERTION    NULL is passed through an argument.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_key_open_param_check(sf_crypto_key_instance_ctrl_t * const p_ctrl,
                                                sf_crypto_key_cfg_t const * const p_cfg)
{
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_cfg);
    SSP_ASSERT(p_cfg->p_lower_lvl_crypto_common);

    return SSP_SUCCESS;
}
#endif

/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto KeyFramework to open a Crypto HAL module.
 * This function is called by SF_CRYPTO_KEY_Open().
 *
 * @retval SSP_SUCCESS          The module was successfully opened.
 * @retval SSP_ERR_UNSUPPORTED  The module does not support the key type specified by user.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_key_hal_open(sf_crypto_key_instance_ctrl_t * const p_ctrl,
                                        sf_crypto_key_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;

    /* Check the key type and get an API instance matching to it. */
    switch (p_ctrl->key_type)
    {
    case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
    case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
    case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
        err = sf_crypto_key_open_rsa(p_ctrl, p_cfg);
        break;

    case SF_CRYPTO_KEY_TYPE_AES_WRAPPED:
        err = sf_crypto_key_open_aes(p_ctrl, p_cfg);
        break;

    case SF_CRYPTO_KEY_TYPE_ECC_WRAPPED:
    case SF_CRYPTO_KEY_TYPE_ECC_PLAIN_TEXT:
        err = sf_crypto_key_open_ecc(p_ctrl, p_cfg);
        break;

    default:
        /* The other key types are currently not supported. */
        err = SSP_ERR_UNSUPPORTED;
        break;
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto KeyFramework to close a Crypto HAL module.
 * This function is called by SF_CRYPTO_KEY_Close().
 *
 * @retval SSP_SUCCESS          The module was successfully closed.
 * @retval SSP_ERR_UNSUPPORTED  The module does not support the key type specified by user.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_key_hal_close(sf_crypto_key_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;

    /* Check the key type and get an API instance matching to it. */
    switch (p_ctrl->key_type)
    {
    case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
    case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
    case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
        err = sf_crypto_key_close_rsa(p_ctrl);
        break;

    case SF_CRYPTO_KEY_TYPE_AES_WRAPPED:
        err = sf_crypto_key_close_aes(p_ctrl);
        break;

    case SF_CRYPTO_KEY_TYPE_ECC_WRAPPED:
    case SF_CRYPTO_KEY_TYPE_ECC_PLAIN_TEXT:
        err = sf_crypto_key_close_ecc(p_ctrl);
        break;

    default:
        /* The other key types are currently not supported. */
        err = SSP_ERR_UNSUPPORTED;
        break;
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto KeyFramework to let a Crypto HAL module generate a key.
 * This function is called by SF_CRYPTO_KEY_Close().
 *
 * @retval SSP_SUCCESS          The module generated a key successfully.
 * @retval SSP_ERR_UNSUPPORTED  The module does not support the key type specified by user.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_key_hal_generate(sf_crypto_key_instance_ctrl_t * const p_ctrl,
                                            sf_crypto_key_t * const p_secret_key,
                                            sf_crypto_key_t * const p_public_key)
{
    ssp_err_t err = SSP_SUCCESS;

    /* Check the key type. */
    switch (p_ctrl->key_type)
    {
    case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
    case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
    case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:

        err = sf_crypto_key_generate_rsa(p_ctrl, p_secret_key, p_public_key);
        break;

    case SF_CRYPTO_KEY_TYPE_AES_WRAPPED:
        err = sf_crypto_key_generate_aes(p_ctrl, p_secret_key);
        break;

    case SF_CRYPTO_KEY_TYPE_ECC_WRAPPED:
    case SF_CRYPTO_KEY_TYPE_ECC_PLAIN_TEXT:
        err = sf_crypto_key_generate_ecc(p_ctrl, p_secret_key, p_public_key);
        break;

    default:
        /* The other key types are currently not supported. */
        err = SSP_ERR_UNSUPPORTED;
        break;
    }

    return err;
}
