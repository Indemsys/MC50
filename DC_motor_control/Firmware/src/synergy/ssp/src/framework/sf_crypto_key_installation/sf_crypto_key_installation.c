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
 * File Name    : sf_crypto_key_installation.c
 * Description  : SSP Crypto Key Installation Framework Module
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @file
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <string.h>
#include "sf_crypto_key_installation_cfg.h"
#include "sf_crypto_key_installation.h"
#include "sf_crypto_key_installation_private_api.h"
#include "../sf_crypto/sf_crypto_private_api.h"
#include "r_key_installation_api.h"
#include "r_crypto_api.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/**  Definition of key lengths */
#define BITS_IN_BYTE                                    (8U)
#define BYTES_IN_WORD                                   (4U)
#define RSA_1024_KEY_LENGTH_BITS                        (1024U)
#define RSA_2048_KEY_LENGTH_BITS                        (2048U)
#define AES_128_ENCRYPTED_KEY_LENGTH_BITS               (256U)
#define AES_192_ENCRYPTED_KEY_LENGTH_BITS               (384U)
#define AES_256_ENCRYPTED_KEY_LENGTH_BITS               (384U)
#define AES_128_ENCRYPTED_XTS_KEY_LENGTH_BITS           (384U)
#define AES_256_ENCRYPTED_XTS_KEY_LENGTH_BITS           (640U)
#define RSA_1024_KEY_LENGTH_BYTES                       ((RSA_1024_KEY_LENGTH_BITS) / (BITS_IN_BYTE))
#define RSA_2048_KEY_LENGTH_BYTES                       ((RSA_2048_KEY_LENGTH_BITS) / (BITS_IN_BYTE))
#define RSA_1024_ENCRYPTED_KEY_LENGTH_BYTES             (144U)
#define RSA_2048_ENCRYPTED_KEY_LENGTH_BYTES             (272U)
#define AES_128_ENCRYPTED_KEY_LENGTH_BYTES              ((AES_128_ENCRYPTED_KEY_LENGTH_BITS) / (BITS_IN_BYTE))
#define AES_192_ENCRYPTED_KEY_LENGTH_BYTES              ((AES_192_ENCRYPTED_KEY_LENGTH_BITS) / (BITS_IN_BYTE))
#define AES_256_ENCRYPTED_KEY_LENGTH_BYTES              ((AES_256_ENCRYPTED_KEY_LENGTH_BITS) / (BITS_IN_BYTE))
#define ECC_192_ENCRYPTED_KEY_LENGTH_BYTES              (48U)
#define ECC_224_ENCRYPTED_KEY_LENGTH_BYTES              (48U)
#define ECC_256_ENCRYPTED_KEY_LENGTH_BYTES              (48U)
#define ECC_384_ENCRYPTED_KEY_LENGTH_BYTES              (64U)
#define AES_128_ENCRYPTED_XTS_KEY_LENGTH_BYTES          ((AES_128_ENCRYPTED_XTS_KEY_LENGTH_BITS) / (BITS_IN_BYTE))
#define AES_256_ENCRYPTED_XTS_KEY_LENGTH_BYTES          ((AES_256_ENCRYPTED_XTS_KEY_LENGTH_BITS) / (BITS_IN_BYTE))

#define SESSION_KEY_SIZE                                (32U)

#define AES_128_INSTALLED_WRAPPED_KEY_SIZE              (36U)
#define AES_192_INSTALLED_WRAPPED_KEY_SIZE              (52U)
#define AES_256_INSTALLED_WRAPPED_KEY_SIZE              (52U)
#define AES_128_XTS_INSTALLED_WRAPPED_KEY_SIZE          (52U)
#define AES_256_XTS_INSTALLED_WRAPPED_KEY_SIZE          (84U)
#define RSA_1024_INSTALLED_WRAPPED_KEY_SIZE             (148U)
#define RSA_2048_INSTALLED_WRAPPED_KEY_SIZE             (276U)
#define ECC_192_INSTALLED_WRAPPED_KEY_SIZE              (52U)
#define ECC_224_INSTALLED_WRAPPED_KEY_SIZE              (52U)
#define ECC_256_INSTALLED_WRAPPED_KEY_SIZE              (52U)
#define ECC_384_INSTALLED_WRAPPED_KEY_SIZE              (68U)

#define RSA_1024_MODULUS_LENGTH_BYTES                   (128U)
#define RSA_2048_MODULUS_LENGTH_BYTES                   (256U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
#if SF_CRYPTO_KEY_INSTALLATION_CFG_PARAM_CHECKING_ENABLE
static ssp_err_t sf_crypto_key_installation_open_param_check(sf_crypto_key_installation_instance_ctrl_t * const p_ctrl,
                                                             sf_crypto_key_installation_cfg_t const * const p_cfg);
static
ssp_err_t
sf_crypto_key_installation_key_install_buffer_param_check(sf_crypto_data_handle_t const * const p_user_key_input,
                                                          sf_crypto_data_handle_t const * const p_session_key_input,
                                                          uint32_t const * const p_iv_input,
                                                          sf_crypto_data_handle_t const * const p_key_data_out);
#endif /* SF_CRYPTO_KEY_INSTALLATION_CFG_PARAM_CHECKING_ENABLE */

static ssp_err_t sf_crypto_validate_interface(sf_crypto_instance_ctrl_t * const p_ctrl,
                                              sf_crypto_key_installation_cfg_t const * const p_cfg);
static ssp_err_t sf_crypto_validate_interface_set_param_aes(sf_crypto_key_installation_cfg_t const * const p_cfg,
                                                            crypto_interface_get_param_t * p_param);
static ssp_err_t sf_crypto_validate_interface_set_param_ecc(sf_crypto_key_installation_cfg_t const * const p_cfg,
                                                            crypto_interface_get_param_t * p_param);
static ssp_err_t sf_crypto_validate_interface_set_param_rsa(sf_crypto_key_installation_cfg_t const * const p_cfg,
                                                            crypto_interface_get_param_t * p_param);
static ssp_err_t sf_crypto_set_buffer_params(sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                             key_installation_key_t * p_user_key_input,
                                             key_installation_key_t * p_session_key_input,
                                             key_installation_key_t * p_key_data_out);
static ssp_err_t sf_crypto_set_user_key_buffer_params_aes(sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                                          key_installation_key_t * p_user_key_input,
                                                          key_installation_key_t * p_session_key_input);
static ssp_err_t sf_crypto_set_user_key_buffer_params_ecc(sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                                          key_installation_key_t * p_user_key_input,
                                                          key_installation_key_t * p_session_key_input);
static ssp_err_t sf_crypto_set_user_key_buffer_params_rsa(sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                                          key_installation_key_t * p_user_key_input,
                                                          key_installation_key_t * p_session_key_input);
static ssp_err_t sf_crypto_get_key_install_config(sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                                  sf_crypto_key_installation_cfg_t const * const p_cfg);
static ssp_err_t sf_crypto_get_key_install_config_aes(sf_crypto_key_installation_cfg_t const * const p_cfg);
static ssp_err_t sf_crypto_get_key_install_config_rsa(sf_crypto_key_installation_cfg_t const * const p_cfg);
static ssp_err_t sf_crypto_get_key_install_config_ecc(sf_crypto_key_installation_cfg_t const * const p_cfg);
static ssp_err_t sf_crypto_validate_shared_index(uint32_t const * const p_shared_index_input);
static ssp_err_t sf_crypto_validate_key_buffer_data_length(sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                                           sf_crypto_data_handle_t const * p_user_key_input,
                                                           sf_crypto_data_handle_t const * p_session_key_input,
                                                           sf_crypto_data_handle_t const * p_key_data_out);
static ssp_err_t sf_crypto_validate_key_buffer_data_length_rsa(sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                                               sf_crypto_data_handle_t const * p_user_key_input,
                                                               sf_crypto_data_handle_t const * p_key_data_out);
static ssp_err_t sf_crypto_validate_key_buffer_data_length_aes(sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                                               sf_crypto_data_handle_t const * p_user_key_input,
                                                               sf_crypto_data_handle_t const * p_key_data_out);
static ssp_err_t sf_crypto_validate_key_buffer_data_length_ecc(sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                                               sf_crypto_data_handle_t const * p_user_key_input,
                                                               sf_crypto_data_handle_t const * p_key_data_out);
static void sf_crypto_key_installation_get_rsa_key_sizes(sf_crypto_key_size_t rsa_key_size,
                                                         uint32_t * rsa_modulus_size,
                                                         uint32_t * rsa_wrapped_key_size);
static
ssp_err_t
sf_crypto_key_installation_validate_rsa_buffers(sf_crypto_key_size_t key_size,
                                                sf_crypto_data_handle_t const * const p_user_key_rsa_modulus,
                                                sf_crypto_data_handle_t const * const p_key_data_out);
static ssp_err_t sf_crypto_key_buffer_data_length_check(ssp_err_t user_key_in_and_key_out_rsa_validity,
                                                        ssp_err_t user_key_in_and_key_out_aes_validity,
                                                        ssp_err_t user_key_in_and_key_out_ecc_validity,
                                                        ssp_err_t session_key_input_validity);

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
static const ssp_version_t sf_crypto_key_installation_version =
{
    .api_version_major  = SF_CRYPTO_KEY_INSTALLATION_API_VERSION_MAJOR,
    .api_version_minor  = SF_CRYPTO_KEY_INSTALLATION_API_VERSION_MINOR,
    .code_version_major = SF_CRYPTO_KEY_INSTALLATION_CODE_VERSION_MAJOR,
    .code_version_minor = SF_CRYPTO_KEY_INSTALLATION_CODE_VERSION_MINOR
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
const sf_crypto_key_installation_api_t         g_sf_crypto_key_installation_api =
{
    .open           = SF_CRYPTO_KEY_INSTALLATION_Open,
    .close          = SF_CRYPTO_KEY_INSTALLATION_Close,
    .keyInstall     = SF_CRYPTO_KEY_INSTALLATION_KeyInstall,
    .versionGet     = SF_CRYPTO_KEY_INSTALLATION_VersionGet
};

/*LDRA_ANALYSIS */

/*******************************************************************************************************************//**
 * @addtogroup SF_CRYPTO_KEY_INSTALLATION
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
Functions
***********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @brief   SSP Crypto Key Installation Framework Open operation.
 *
 * @retval SSP_SUCCESS          The module was successfully opened.
 * @retval SSP_ERR_ASSERTION    NULL is passed through an argument.
 * @retval SSP_ERR_CRYPTO_COMMON_NOT_OPENED     Crypto Framework Common Module has yet been opened.
 * @retval SSP_ERR_ALREADY_OPEN The module has been already opened.
 * @retval SSP_ERR_INVALID_ARGUMENT The module does not support the key type specified by user.
 * @return                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_KEY_INSTALLATION_Open (sf_crypto_key_installation_ctrl_t * const p_api_ctrl,
                                           sf_crypto_key_installation_cfg_t const * const p_cfg)
{
    ssp_err_t                                   ret_val;
    uint32_t                                    ret_val_crypto;
    sf_crypto_key_installation_instance_ctrl_t  * p_ctrl = (sf_crypto_key_installation_instance_ctrl_t *)p_api_ctrl;
    sf_crypto_instance_ctrl_t                   * p_ctrl_common;
    key_installation_ctrl_t                     * p_ctrl_key_install;
    key_installation_cfg_t const                * p_cfg_key_install;
    key_installation_api_t const                * p_api_key_install;
    sf_crypto_state_t                             common_module_status = SF_CRYPTO_CLOSED;

#if SF_CRYPTO_KEY_INSTALLATION_CFG_PARAM_CHECKING_ENABLE
    /** Verify if parameters are valid */
    ret_val = sf_crypto_key_installation_open_param_check(p_ctrl, p_cfg);
    if (SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }
#endif

    ret_val = SSP_SUCCESS;
    ret_val_crypto = (uint32_t)SSP_SUCCESS;

    p_ctrl->p_lower_lvl_common_ctrl = p_cfg->p_lower_lvl_common->p_ctrl;
    p_ctrl->p_lower_lvl_common_api = (sf_crypto_api_t *)p_cfg->p_lower_lvl_common->p_api;

    p_ctrl_common = p_ctrl->p_lower_lvl_common_ctrl;

    /** Check if the Crypto Framework has been opened. If not yet opened, return an error. */
    ret_val = p_ctrl->p_lower_lvl_common_api->statusGet(p_ctrl_common, &common_module_status);
    if (SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }

    if (SF_CRYPTO_OPENED != common_module_status)
    {
        return SSP_ERR_CRYPTO_COMMON_NOT_OPENED;
    }

    /** Check if SF Crypto Key Installation is already opened */
    if (SF_CRYPTO_KEY_INSTALLATION_OPENED == p_ctrl->status)
    {
        return SSP_ERR_ALREADY_OPEN;
    }

    /** Validate and copy configuration parameters to control block */
    ret_val =  sf_crypto_get_key_install_config(p_ctrl, p_cfg);
    if (SSP_SUCCESS != ret_val)
    {
        /* Incorrect configuration request ... */
        return ret_val;
    }

    /* Note: sf_crypto.lock() is not called because the HAL driver does not access HW during open.
     * The HAL interfaceGet API is also called which does not access HW either.
     * In the future, if HW will be accessed through the HAL driver; lock should be acquired. */

    /* Setup HAL API */
    p_ctrl->p_lower_lvl_instance = (void *)p_cfg->p_lower_lvl_instance;

    ret_val_crypto = sf_crypto_validate_interface(p_ctrl_common, p_cfg);
    if ((uint32_t)SSP_SUCCESS != ret_val_crypto)
    {
        /* Incorrect key installation request ... */
        ret_val = (ssp_err_t)ret_val_crypto;
        return ret_val;
    }
    p_ctrl_key_install = ((key_installation_instance_t *)(p_ctrl->p_lower_lvl_instance))->p_ctrl;
    p_cfg_key_install  = ((key_installation_instance_t *)(p_ctrl->p_lower_lvl_instance))->p_cfg;
    p_api_key_install  = ((key_installation_instance_t *)(p_ctrl->p_lower_lvl_instance))->p_api;

    /** Call HAL API to Open Key Installation HAL Driver */
    ret_val_crypto = p_api_key_install->open(p_ctrl_key_install, p_cfg_key_install);

    if ((uint32_t)SSP_SUCCESS != ret_val_crypto)
    {
        ret_val = (ssp_err_t)ret_val_crypto;
        return ret_val;
    }

    /** Set Key Installation Framework module status to Open */
    p_ctrl->status = SF_CRYPTO_KEY_INSTALLATION_OPENED;

    /* Increment Open counter to indicate a (this) module is open */
    sf_crypto_open_counter_increment(p_ctrl_common);

    return ret_val;

} /* End of function SF_CRYPTO_KEY_INSTALLATION_Open */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Key Installation Framework Close operation.
 *
 * @retval SSP_SUCCESS          The module was successfully closed.
 * @retval SSP_ERR_ASSERTION    NULL is passed through the argument.
 * @retval SSP_ERR_NOT_OPEN     The module has yet been opened. Call Open API first.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_KEY_INSTALLATION_Close (sf_crypto_key_installation_ctrl_t * const p_api_ctrl)
{
    ssp_err_t                                       ret_val;
    sf_crypto_key_installation_instance_ctrl_t    * p_ctrl = (sf_crypto_key_installation_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t                     * p_ctrl_common;
    key_installation_ctrl_t                       * p_ctrl_key_install;
    key_installation_api_t const                  * p_api_key_install;

#if SF_CRYPTO_KEY_INSTALLATION_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid. */
    SSP_ASSERT(p_ctrl);
#endif

    ret_val = SSP_SUCCESS;

    p_ctrl_common = p_ctrl->p_lower_lvl_common_ctrl;

    /** Check if the Crypto Framework has been opened. If not yet opened, return an error. */
    if (SF_CRYPTO_KEY_INSTALLATION_OPENED != p_ctrl->status)
    {
        return SSP_ERR_NOT_OPEN;
    }

    /* Note: sf_crypto.lock() is not called because the HAL driver does not access HW during close.
     * In the future, if HW will be accessed through the HAL driver; lock should be acquired. */

    p_ctrl_key_install = ((key_installation_instance_t *)(p_ctrl->p_lower_lvl_instance))->p_ctrl;
    p_api_key_install  = ((key_installation_instance_t *)(p_ctrl->p_lower_lvl_instance))->p_api;

    /** Call HAL API to Close Key Installation HAL Driver */
    ret_val = p_api_key_install->close(p_ctrl_key_install);

    if (SSP_SUCCESS == ret_val)
    {
        p_ctrl->status = SF_CRYPTO_KEY_INSTALLATION_CLOSED;
    }

    /* Decrement Open counter to indicate a (this) module is closed */
    sf_crypto_open_counter_decrement(p_ctrl_common);

    return (ret_val);

} /* End of function SF_CRYPTO_KEY_INSTALLATION_Close */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Framework Key Installation operation.
 *
 * @retval SSP_SUCCESS              The module created a key successfully.
 * @retval SSP_ERR_ASSERTION        NULL is passed through an argument.
 * @retval SSP_ERR_NOT_OPEN         The module has yet been opened. Call Open API first.
 * @retval SSP_ERR_INVALID_ARGUMENT At least one of the input arguments is invalid.
 * @retval SSP_ERR_INVALID_SIZE     At least one of the buffers provided is of invalid size.
 * @return                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_KEY_INSTALLATION_KeyInstall (sf_crypto_key_installation_ctrl_t * const p_api_ctrl,
                                                 sf_crypto_data_handle_t const * const p_user_key_rsa_modulus,
                                                 sf_crypto_data_handle_t const * const p_user_key_input,
												 sf_crypto_key_installation_shared_index_t const shared_index_input,
                                                 sf_crypto_data_handle_t const * const p_session_key_input,
                                                 uint32_t const * const p_iv_input,
                                                 sf_crypto_data_handle_t * const p_key_data_out)
{
    ssp_err_t                                     ret_val;
    sf_crypto_key_installation_instance_ctrl_t    * p_ctrl = (sf_crypto_key_installation_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t                     * p_ctrl_common;
    sf_crypto_api_t                               * p_api_common;
    key_installation_ctrl_t                       * p_ctrl_key_install;
    key_installation_api_t const                  * p_api_key_install;
    r_crypto_data_handle_t                        * p_user_modulus_input;
    r_crypto_data_handle_t                        user_key_rsa_modulus;
    key_installation_key_t                        user_key_input;
    key_installation_key_t                        session_key_input;
    key_installation_key_t                        key_data_out;

#if SF_CRYPTO_KEY_INSTALLATION_CFG_PARAM_CHECKING_ENABLE
    /** Verify if parameters are valid */
    SSP_ASSERT(p_ctrl);

    ret_val = sf_crypto_key_installation_key_install_buffer_param_check (p_user_key_input, p_session_key_input, 
                                                                         p_iv_input, p_key_data_out);
    if (SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }
#endif

    ret_val = SSP_SUCCESS;

    p_ctrl_common = p_ctrl->p_lower_lvl_common_ctrl;
    p_api_common = p_ctrl->p_lower_lvl_common_api;

    /** Validate modulus and output buffers only in case of RSA key type. */
    if (SF_CRYPTO_KEY_TYPE_ENCRYPTED_RSA_PRIVATE_KEY == p_ctrl->key_type)
    {
        ret_val = sf_crypto_key_installation_validate_rsa_buffers(p_ctrl->key_size,
                                                                  p_user_key_rsa_modulus,
                                                                  p_key_data_out);
        if (SSP_SUCCESS != ret_val)
        {
            /* Incorrect buffer(s) provided ... */
            return ret_val;
        }
    }

    /** Check if the Crypto Key Installation Framework has been opened. If not yet opened, return an error. */
    if (SF_CRYPTO_KEY_INSTALLATION_CLOSED == p_ctrl->status)
    {
        return SSP_ERR_NOT_OPEN;
    }

    /* Lock the module to access to Crypto HAL module. */
    ret_val = p_api_common->lock (p_ctrl_common);
    if (SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }

    p_ctrl_key_install = ((key_installation_instance_t *)(p_ctrl->p_lower_lvl_instance))->p_ctrl;
    p_api_key_install  = ((key_installation_instance_t *)(p_ctrl->p_lower_lvl_instance))->p_api;

    /* Convert shared_index_input enum to a pointer for the PRC */
    uint32_t shared_input[1] = { 0 };
    shared_input[0] = (uint32_t)shared_index_input;

    /* Validate the shared index is in the valid range */
    ret_val = sf_crypto_validate_shared_index(shared_input);

    if (SSP_SUCCESS != ret_val)
    {
        /* Incorrect shared index provided ... */
        /* Unlock before returning. */
        p_api_common->unlock(p_ctrl_common);
        return ret_val;
    }

    /* Map shared index input to HAL call parameter as key_installation_key_shared_index_t */
    key_installation_key_shared_index_t shared_index = (key_installation_key_shared_index_t)shared_index_input;

    /* Validate all input/output key buffers */
    ret_val = sf_crypto_validate_key_buffer_data_length(p_ctrl, p_user_key_input, 
                                                        p_session_key_input, p_key_data_out);
    if (SSP_SUCCESS != ret_val)
    {
        /* Incorrect buffer(s) provided ... */
        /* Unlock before returning. */
        p_api_common->unlock (p_ctrl_common);
        return ret_val;
    }

    /* This function sets key_format and key_size for user_key_input
     * and key_format for key_data_out before passing it to HAL */
    ret_val = sf_crypto_set_buffer_params(p_ctrl, &user_key_input, &session_key_input, &key_data_out);
    if (SSP_SUCCESS != ret_val)
    {
        /* Invalid mapping - Unlock Crypto Common module */
        p_api_common->unlock (p_ctrl_common);
        return (ret_val);
    }

    /* This is just a place holder. key_size of key_data_out currently has no meaning. */
    key_data_out.key_size = KEY_INSTALLATION_KEY_SIZE_AES_128;

    /*
     * Map user key rsa modulus (sf_crypto_data_handle_t) to HAL call parameter as key_installation_key_t
     */
    p_user_modulus_input = (r_crypto_data_handle_t *)0;
    if (SF_CRYPTO_KEY_TYPE_ENCRYPTED_RSA_PRIVATE_KEY == p_ctrl->key_type)
    {
        user_key_rsa_modulus.data_length = (p_user_key_rsa_modulus->data_length) / BYTES_IN_WORD;
        user_key_rsa_modulus.p_data = ((uint32_t *)p_user_key_rsa_modulus->p_data);
        p_user_modulus_input = &user_key_rsa_modulus;
    }

    /*
     * Map user key input (sf_crypto_data_handle_t) to HAL call parameter as key_installation_key_t
     */
    user_key_input.data_length = (p_user_key_input->data_length) / BYTES_IN_WORD;
    user_key_input.p_data = ((uint32_t *)p_user_key_input->p_data);

    /*
     * Map session key input (sf_crypto_data_handle_t) to HAL call parameter as key_installation_key_t
     */
    session_key_input.data_length = (p_session_key_input->data_length) / BYTES_IN_WORD;
    session_key_input.p_data = ((uint32_t *)p_session_key_input->p_data);

    /*
     * Map key output (sf_crypto_data_handle_t)to HAL call parameter as key_installation_key_t
     */
    key_data_out.data_length = (p_key_data_out->data_length) / BYTES_IN_WORD;
    key_data_out.p_data = ((uint32_t *)p_key_data_out->p_data);

    /** Call HAL API to perform Key Installation operation */
    ret_val = p_api_key_install->keyInstall(p_ctrl_key_install,
                                            p_user_modulus_input,
                                            &user_key_input,
                                            shared_index,
                                            &session_key_input,
                                            p_iv_input,
                                            &key_data_out);

    /* Unlock the module. */
    ssp_err_t err_unlock = p_api_common->unlock (p_ctrl_common);

    if (SSP_SUCCESS == ret_val)
    {
        ret_val = err_unlock;
        p_key_data_out->data_length = ((key_data_out.data_length) * BYTES_IN_WORD);
    }

    return (ret_val);
} /* End of function SF_CRYPTO_KEY_INSTALLATION_KeyInstall */

/*******************************************************************************************************************//**
 * @brief      Gets driver version based on compile time macros.
 *
 * @retval     SSP_SUCCESS          Successful close.
 * @retval     SSP_ERR_ASSERTION    The parameter p_version is NULL.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_KEY_INSTALLATION_VersionGet (ssp_version_t * const p_version)
{
#if SF_CRYPTO_KEY_INSTALLATION_CFG_PARAM_CHECKING_ENABLE
    /* Verify parameters are valid. */
    SSP_ASSERT(p_version);
#endif

    p_version->version_id = sf_crypto_key_installation_version.version_id;

    return SSP_SUCCESS;
} /* End of function SF_CRYPTO_KEY_INSTALLATION_VersionGet */

/*******************************************************************************************************************//**
 * @} (end defgroup SF_CRYPTO_KEY_INSTALLATION)
 **********************************************************************************************************************/

#if SF_CRYPTO_KEY_INSTALLATION_CFG_PARAM_CHECKING_ENABLE
/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to check open API parameters.
 * This function is called by SF_CRYPTO_KEY_INSTALLATION_Open().
 *
 * @param[in,out]  p_ctrl           Pointer to Crypto Key Installation Framework control block structure.
 * @param[in]      p_cfg            Pointer to sf_crypto_key_installation_cfg_t configuration structure.
 *
 * @retval SSP_SUCCESS              The module was successfully opened.
 * @retval SSP_ERR_ASSERTION        NULL is passed through an argument.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_key_installation_open_param_check (sf_crypto_key_installation_instance_ctrl_t * const p_ctrl,
                                                              sf_crypto_key_installation_cfg_t const * const p_cfg)
{
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_cfg);
    SSP_ASSERT(p_cfg->p_lower_lvl_common);

    return SSP_SUCCESS;
} /* End of function sf_crypto_key_installation_open_param_check */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to check keyInstall API parameters.
 * This function is called by SF_CRYPTO_KEY_INSTALLATION_KeyInstall().
 *
 *  @param[in]     p_user_key_input     Pointer to sf_crypto_key_handle_t structure which includes a pointer to the
 *                                      WORD aligned buffer which holds the encrypted user key and the key length.
 *  @param[in]     p_session_key_input  Pointer to sf_crypto_key_handle_t structure which includes a pointer to the
 *                                      WORD aligned buffer which holds the session key and key length.
 *  @param[in]     p_iv_input           Pointer to array of words which holds the IV used to encrypt the user key.
 *  @param[in]     p_key_data_out       Pointer to sf_crypto_key_handle_t structure which includes a pointer to the
 *                                      WORD aligned buffer to hold the wrapped key and the buffer length.
 *
 * @retval SSP_SUCCESS                  The module was successfully opened.
 * @retval SSP_ERR_ASSERTION            NULL is passed through an argument. One or more buffers are not WORD aligned.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_key_installation_key_install_buffer_param_check (sf_crypto_data_handle_t const * const p_user_key_input,
                                                           sf_crypto_data_handle_t const * const p_session_key_input,
                                                           uint32_t const * const p_iv_input,
                                                           sf_crypto_data_handle_t const * const p_key_data_out)
{
    SSP_ASSERT(p_user_key_input);
    SSP_ASSERT(p_user_key_input->p_data);
    SSP_ASSERT(0U == ((uint32_t)p_user_key_input->p_data % sizeof(uint32_t)));
    SSP_ASSERT(p_session_key_input);
    SSP_ASSERT(p_session_key_input->p_data);
    SSP_ASSERT(0U == ((uint32_t)p_session_key_input->p_data % sizeof(uint32_t)));
    SSP_ASSERT(p_iv_input);
    SSP_ASSERT(0U == ((uint32_t)p_iv_input % sizeof(uint32_t)));
    SSP_ASSERT(p_key_data_out);
    SSP_ASSERT(p_key_data_out->p_data);
    SSP_ASSERT(0U == ((uint32_t)p_key_data_out->p_data % sizeof(uint32_t)));

    return SSP_SUCCESS;
} /* End of function sf_crypto_key_installation_key_install_buffer_param_check */

#endif /* SF_CRYPTO_KEY_INSTALLATION_CFG_PARAM_CHECKING_ENABLE */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to validate and set control block
 *          from configuration block.
 * This function is called by SF_CRYPTO_KEY_INSTALLATION_Open().
 *
 * @param[in,out]  p_ctrl           Pointer to Crypto Key Installation Framework control block structure.
 * @param[in]      p_cfg            Pointer to sf_crypto_key_installation_cfg_t configuration structure.
 *
 * @note                            Parameter validity check is the responsibility of
 *                                  the caller of this static function.
 *
 * @retval SSP_SUCCESS              The module was successfully opened.
 * @retval SSP_ERR_INVALID_ARGUMENT Invalid request
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_get_key_install_config (sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                                   sf_crypto_key_installation_cfg_t const * const p_cfg)
{
    ssp_err_t iret = SSP_SUCCESS;
    switch(p_cfg->key_type)
    {
        case SF_CRYPTO_KEY_TYPE_ENCRYPTED_RSA_PRIVATE_KEY:
        {
            iret = sf_crypto_get_key_install_config_rsa(p_cfg);
            break;
        }
        case SF_CRYPTO_KEY_TYPE_ENCRYPTED_AES_KEY:
        {
            iret = sf_crypto_get_key_install_config_aes(p_cfg);
            break;
        }
        case SF_CRYPTO_KEY_TYPE_ENCRYPTED_ECC_PRIVATE_KEY:
        {
            iret = sf_crypto_get_key_install_config_ecc(p_cfg);
            break;
        }
        default:
            return SSP_ERR_INVALID_ARGUMENT;
    }
    if (SSP_SUCCESS == iret)
    {
        p_ctrl->key_type = p_cfg->key_type;
        p_ctrl->key_size = p_cfg->key_size;
    }
    return (iret);
} /* End of function sf_crypto_get_key_install_config */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to validate RSA related configuration
 * This function is called by sf_crypto_get_key_install_config().
 *
 * @param[in]      p_cfg            Pointer to sf_crypto_key_installation_cfg_t configuration structure.
 *
 * @note                            Parameter validity check is the responsibility of
 *                                  the caller of this static function.
 *
 * @retval SSP_SUCCESS              The module was successfully opened.
 * @retval SSP_ERR_INVALID_ARGUMENT Invalid request
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_get_key_install_config_rsa (sf_crypto_key_installation_cfg_t const * const p_cfg)
{
    switch(p_cfg->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_RSA_1024:
        case SF_CRYPTO_KEY_SIZE_RSA_2048:
            break;
        default:
            return SSP_ERR_INVALID_ARGUMENT;
    }
    return SSP_SUCCESS;
} /* End of function sf_crypto_get_key_install_config_rsa */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to validate AES related configuration
 * This function is called by sf_crypto_get_key_install_config().
 *
 * @param[in]      p_cfg            Pointer to sf_crypto_key_installation_cfg_t configuration structure.
 *
 * @note                            Parameter validity check is the responsibility of
 *                                  the caller of this static function.
 *
 * @retval SSP_SUCCESS              The module was successfully opened.
 * @retval SSP_ERR_INVALID_ARGUMENT Invalid request
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_get_key_install_config_aes (sf_crypto_key_installation_cfg_t const * const p_cfg)
{
    switch(p_cfg->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_AES_128:
        case SF_CRYPTO_KEY_SIZE_AES_XTS_128:
        case SF_CRYPTO_KEY_SIZE_AES_192:
        case SF_CRYPTO_KEY_SIZE_AES_256:
        case SF_CRYPTO_KEY_SIZE_AES_XTS_256:
            break;
        default:
            return SSP_ERR_INVALID_ARGUMENT;
    }
    return SSP_SUCCESS;
} /* End of function sf_crypto_get_key_install_config_aes */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to validate ECC related configuration
 * This function is called by sf_crypto_get_key_install_config().
 *
 * @param[in]      p_cfg            Pointer to sf_crypto_key_installation_cfg_t configuration structure.
 *
 * @note                            Parameter validity check is the responsibility of
 *                                  the caller of this static function.
 *
 * @retval SSP_SUCCESS              The module was successfully opened.
 * @retval SSP_ERR_INVALID_ARGUMENT Invalid request
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_get_key_install_config_ecc (sf_crypto_key_installation_cfg_t const * const p_cfg)
{
    switch(p_cfg->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_ECC_192:
        case SF_CRYPTO_KEY_SIZE_ECC_256:
            break;
        default:
            return SSP_ERR_INVALID_ARGUMENT;
    }
    return SSP_SUCCESS;
} /* End of function sf_crypto_get_key_install_config_ecc */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to check keyInstall request is valid.
 * This function is called by SF_CRYPTO_KEY_INSTALLATION_Open().
 *
 * @param[in,out]  p_ctrl           Pointer to Crypto Key Installation Framework control block structure.
 * @param[in]      p_cfg            Pointer to sf_crypto_key_installation_cfg_t configuration structure.
 *
 * @note                            Parameter validity check is the responsibility of
 *                                  the caller of this static function.
 *
 * @retval SSP_SUCCESS              The module was successfully opened.
 * @retval SSP_ERR_INVALID_ARGUMENT Invalid request
 * @retval SSP_ERR_ASSERTION        Error in handling the request.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_validate_interface (sf_crypto_instance_ctrl_t * const p_ctrl,
                                               sf_crypto_key_installation_cfg_t const * const p_cfg)
{
    crypto_instance_t                   * p_crypto;
    crypto_interface_get_param_t        param;
    void                                * requested_interface = NULL;
    ssp_err_t                           ret_val        = SSP_SUCCESS;

    /* Get a Crypto common control block and the HAL instance. */
    p_crypto = (crypto_instance_t *)(p_ctrl->p_lower_lvl_crypto);

    switch(p_cfg->key_type)
    {
        case SF_CRYPTO_KEY_TYPE_ENCRYPTED_RSA_PRIVATE_KEY:
        {
            param.algorithm_type = CRYPTO_ALGORITHM_TYPE_RSA;
            param.key_type = CRYPTO_KEY_TYPE_RSA_WRAPPED;
            ret_val = sf_crypto_validate_interface_set_param_rsa(p_cfg, &param);
            break;
        }
        case SF_CRYPTO_KEY_TYPE_ENCRYPTED_AES_KEY:
        {
            param.algorithm_type = CRYPTO_ALGORITHM_TYPE_AES;
            param.key_type = CRYPTO_KEY_TYPE_AES_WRAPPED;
            ret_val = sf_crypto_validate_interface_set_param_aes(p_cfg, &param);
            break;
        }
        case SF_CRYPTO_KEY_TYPE_ENCRYPTED_ECC_PRIVATE_KEY:
        {
            param.algorithm_type = CRYPTO_ALGORITHM_TYPE_ECC;
            param.key_type = CRYPTO_KEY_TYPE_ECC_WRAPPED;
            ret_val = sf_crypto_validate_interface_set_param_ecc(p_cfg, &param);
            break;
        }
        default:
            return SSP_ERR_INVALID_ARGUMENT;
    }
    /* Verify requested key installation request is valid for the MCU under consideration. */
    ret_val = (ssp_err_t)p_crypto->p_api->interfaceGet(&param, &requested_interface);
    return ret_val;
} /* End of function sf_crypto_validate_interface */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to set param which is used to validate interface by caller.
 * This function is called by sf_crypto_validate_interface().
 * For Key type - SF_CRYPTO_KEY_TYPE_ENCRYPTED_AES_KEY.
 *
 * @param[in]      p_cfg            Pointer to sf_crypto_key_installation_cfg_t configuration structure.
 * @param[out]     p_param          Pointer to param.
 *
 * @note                            Parameter validity check is the responsibility of
 *                                  the caller of this static function.
 *
 * @retval SSP_SUCCESS              The module was successfully opened.
 * @retval SSP_ERR_INVALID_ARGUMENT Invalid request
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_validate_interface_set_param_aes (sf_crypto_key_installation_cfg_t const * const p_cfg,
                                                             crypto_interface_get_param_t * p_param)
{
    switch(p_cfg->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_AES_128:
        {
            p_param->key_size = CRYPTO_KEY_SIZE_AES_128;
            /* Needs to be a valid value.
             * CRYPTO_ECB_MODE, CRYPTO_CBC_MODE, CRYPTO_CTR_MODE, CRYPTO_GCM_MODE are valid values
             * Same Key installation procedure is used for the above chaining modes. */
            p_param->chaining_mode = CRYPTO_ECB_MODE;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_AES_192:
        {
            p_param->key_size = CRYPTO_KEY_SIZE_AES_192;
            /* Needs to be a valid value.
             * CRYPTO_ECB_MODE, CRYPTO_CBC_MODE, CRYPTO_CTR_MODE, CRYPTO_GCM_MODE are valid values
             * Same Key installation procedure is used for the above chaining modes. */
            p_param->chaining_mode = CRYPTO_ECB_MODE;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_AES_256:
        {
            p_param->key_size = CRYPTO_KEY_SIZE_AES_256;
            /* Needs to be a valid value.
             * CRYPTO_ECB_MODE, CRYPTO_CBC_MODE, CRYPTO_CTR_MODE, CRYPTO_GCM_MODE are valid values
             * Same Key installation procedure is used for the above chaining modes. */
            p_param->chaining_mode = CRYPTO_ECB_MODE;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_AES_XTS_128:
        {
            p_param->key_size = CRYPTO_KEY_SIZE_AES_128;
            p_param->chaining_mode = CRYPTO_XTS_MODE;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_AES_XTS_256:
        {
            p_param->key_size = CRYPTO_KEY_SIZE_AES_256;
            p_param->chaining_mode = CRYPTO_XTS_MODE;
            break;
        }
        default:
            return SSP_ERR_INVALID_ARGUMENT;
    }
    return (SSP_SUCCESS);
} /* End of function sf_crypto_validate_interface_set_param_aes */


/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to set param which is used to validate interface by caller.
 * This function is called by sf_crypto_validate_interface().
 * For Key type - SF_CRYPTO_KEY_TYPE_ENCRYPTED_ECC_PRIVATE_KEY
 *
 * @param[in]      p_cfg            Pointer to sf_crypto_key_installation_cfg_t configuration structure.
 * @param[out]     p_param          Pointer to param.
 *
 * @note                            Parameter validity check is the responsibility of
 *                                  the caller of this static function.
 *
 * @retval SSP_SUCCESS              The module was successfully opened.
 * @retval SSP_ERR_INVALID_ARGUMENT Invalid request
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_validate_interface_set_param_ecc (sf_crypto_key_installation_cfg_t const * const p_cfg,
                                                             crypto_interface_get_param_t * p_param)
{
    switch(p_cfg->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_ECC_192:
        {
            p_param->key_size = CRYPTO_KEY_SIZE_ECC_192;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_ECC_256:
        {
            p_param->key_size = CRYPTO_KEY_SIZE_ECC_256;
            break;
        }
        default:
            return SSP_ERR_INVALID_ARGUMENT;
    }
    return (SSP_SUCCESS);
} /* End of function sf_crypto_validate_interface_set_param_ecc */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to set param which is used to validate interface by caller.
 * This function is called by sf_crypto_validate_interface().
 * For Key type - SF_CRYPTO_KEY_TYPE_ENCRYPTED_RSA_PRIVATE_KEY
 *
 * @param[in]      p_cfg            Pointer to sf_crypto_key_installation_cfg_t configuration structure.
 * @param[out]     p_param          Pointer to param.
 *
 * @note                            Parameter validity check is the responsibility of
 *                                  the caller of this static function.
 *
 * @retval SSP_SUCCESS              The module was successfully opened.
 * @retval SSP_ERR_INVALID_ARGUMENT Invalid request
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_validate_interface_set_param_rsa (sf_crypto_key_installation_cfg_t const * const p_cfg,
                                                             crypto_interface_get_param_t * p_param)
{
    switch(p_cfg->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_RSA_1024:
        {
            p_param->key_size = CRYPTO_KEY_SIZE_RSA_1024;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_RSA_2048:
        {
            p_param->key_size = CRYPTO_KEY_SIZE_RSA_2048;
            break;
        }
        default:
            return SSP_ERR_INVALID_ARGUMENT;
    }
    return (SSP_SUCCESS);
} /* End of function sf_crypto_validate_interface_set_param_rsa */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to validate the shared index input.
 * This function is called by SF_CRYPTO_KEY_INSTALLATION_KeyInstall().
 *
 * @param[in,out]  p_ctrl                   Pointer to Crypto Key Installation Framework control block structure.
 * @param[in]      p_shared_index_input     Pointer to enum shared index for the SCE to interpret p_user_key_input.
 *
 * @retval SSP_SUCCESS                      Shared index is successfully validated.
 * @retval SSP_ERR_INVALID_ARGUMENT         Invalid request
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_validate_shared_index (uint32_t const * const p_shared_index_input)
{
#if SF_CRYPTO_KEY_INSTALLATION_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_shared_index_input);
#endif

    ssp_err_t ret_val = SSP_ERR_INVALID_ARGUMENT;

    uint32_t shared_index_input = (uint32_t)(*p_shared_index_input);
    if (shared_index_input <= (uint32_t)SF_CRYPTO_KEY_INSTALLATION_SHARED_INDEX_F)
    {
        ret_val = SSP_SUCCESS;
    }

    return(ret_val);
} /* End of function sf_crypto_validate_shared_index */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to validate input and output key length.
 * This function is called by SF_CRYPTO_KEY_INSTALLATION_KeyInstall().
 *
 * @param[in,out]  p_ctrl                   Pointer to Crypto Key Installation Framework control block structure.
 * @param[in]      p_user_key_input         Pointer to sf_crypto_key_handle_t structure which includes a pointer
 *                                          to the WORD aligned buffer which holds the encrypted user key and length.
 * @param[in]      p_session_key_input      Pointer to sf_crypto_key_handle_t structure which includes a pointer
 *                                          to the WORD aligned buffer which holds the session key and length.
 * @param[in,out]  p_key_data_out           Pointer to sf_crypto_key_handle_t structure which includes a pointer
 *                                          to the WORD aligned buffer to hold the wrapped key and the buffer length.
 *
 * @note                                    Parameter validity check is the responsibility of
 *                                          the caller of this static function.
 *
 * @retval SSP_SUCCESS                      Buffer length is successfully validated.
 * @retval SSP_ERR_INVALID_SIZE             Incorrect data length is set.
 * @retval SSP_ERR_INVALID_ARGUMENT         Invalid request
 * @reval  SSP_ERR_CRYPTO_NOT_IMPLEMENTED   RSA/ ECC not yet available for keyInstall
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_validate_key_buffer_data_length (sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                                            sf_crypto_data_handle_t const * p_user_key_input,
                                                            sf_crypto_data_handle_t const * p_session_key_input,
                                                            sf_crypto_data_handle_t const * p_key_data_out)
{
    ssp_err_t user_key_in_and_key_out_rsa_validity = SSP_SUCCESS;
    ssp_err_t user_key_in_and_key_out_aes_validity = SSP_SUCCESS;
    ssp_err_t user_key_in_and_key_out_ecc_validity = SSP_SUCCESS;
    ssp_err_t session_key_input_validity = SSP_SUCCESS;
    ssp_err_t ret_val = SSP_ERR_INVALID_SIZE;

    switch(p_ctrl->key_type)
        {
            case SF_CRYPTO_KEY_TYPE_ENCRYPTED_RSA_PRIVATE_KEY:
            {
                user_key_in_and_key_out_rsa_validity = sf_crypto_validate_key_buffer_data_length_rsa(p_ctrl,
                                                                                                     p_user_key_input,
                                                                                                     p_key_data_out);
                break;
            }
            case SF_CRYPTO_KEY_TYPE_ENCRYPTED_AES_KEY:
            {
                user_key_in_and_key_out_aes_validity = sf_crypto_validate_key_buffer_data_length_aes(p_ctrl,
                                                                                                     p_user_key_input,
                                                                                                     p_key_data_out);
                break;
            }
            case SF_CRYPTO_KEY_TYPE_ENCRYPTED_ECC_PRIVATE_KEY:
            {
                user_key_in_and_key_out_ecc_validity = sf_crypto_validate_key_buffer_data_length_ecc(p_ctrl,
                                                                                                     p_user_key_input,
                                                                                                     p_key_data_out);
                break;
            }
            default:
                return SSP_ERR_INVALID_ARGUMENT;    /* Validated at the time of Open. Must ideally never land here.. */
        }

        if ((SSP_ERR_INVALID_ARGUMENT == user_key_in_and_key_out_rsa_validity) ||
            (SSP_ERR_INVALID_ARGUMENT == user_key_in_and_key_out_aes_validity) ||
            (SSP_ERR_INVALID_ARGUMENT == user_key_in_and_key_out_ecc_validity))
        {
            /* Validated at the time of Open. Must ideally never land here.. */
            return (SSP_ERR_INVALID_ARGUMENT);
        }

        if (((uint32_t)SESSION_KEY_SIZE) != (p_session_key_input->data_length))
        {
            session_key_input_validity = SSP_ERR_INVALID_SIZE;
        }

        ret_val = sf_crypto_key_buffer_data_length_check(user_key_in_and_key_out_rsa_validity,
                                                         user_key_in_and_key_out_aes_validity,
                                                         user_key_in_and_key_out_ecc_validity,
                                                         session_key_input_validity);
        return(ret_val);
} /* End of function sf_crypto_validate_key_buffer_data_length */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to deduce the final return status of buffer validation.
 * This function is called by sf_crypto_validate_key_buffer_data_length().
 *
 * @param[in]   user_key_in_and_key_out_rsa_validity     Validation status of user key in and key out buffers
 *                                                       for RSA Algorithm
 * @param[in]   user_key_in_and_key_out_aes_validity     Validation status of user key in and key out buffers
 *                                                       for AES Algorithm
 * @param[in]   user_key_in_and_key_out_ecc_validity     Validation status of user key in and key out buffers
 *                                                       for ECC Algorithm
 * @param[in]   session_key_input_validity               Validation status of session key buffer
 *
 * @retval SSP_SUCCESS                  Buffer length is successfully validated.
 * @retval SSP_ERR_INVALID_SIZE         Invalid buffer size.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_key_buffer_data_length_check (ssp_err_t user_key_in_and_key_out_rsa_validity,
                                                         ssp_err_t user_key_in_and_key_out_aes_validity,
                                                         ssp_err_t user_key_in_and_key_out_ecc_validity,
                                                         ssp_err_t session_key_input_validity)
{
    if ((SSP_SUCCESS == user_key_in_and_key_out_rsa_validity) &&
        (SSP_SUCCESS == user_key_in_and_key_out_aes_validity) &&
        (SSP_SUCCESS == user_key_in_and_key_out_ecc_validity) &&
        (SSP_SUCCESS == session_key_input_validity))
    {
        return (SSP_SUCCESS);
    }
    return (SSP_ERR_INVALID_SIZE);
} /* End of function sf_crypto_key_buffer_data_length_check */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to validate input and output
 *          key length for RSA key installation.
 *          This function is called by sf_crypto_validate_key_buffer_data_length().
 *
 * @param[in,out]  p_ctrl               Pointer to Crypto Key Installation Framework control block structure.
 * @param[in]      p_user_key_input     Pointer to sf_crypto_key_handle_t structure which includes a pointer to the
 *                                      WORD aligned buffer which holds the encrypted user key and the key length.
 * @param[in,out]  p_key_data_out       Pointer to sf_crypto_key_handle_t structure which includes a pointer to the
 *                                      WORD aligned buffer to hold the wrapped key and the buffer length.
 *
 * @note                                Parameter validity check is the responsibility of
 *                                      the caller of this static function.
 *
 * @retval SSP_SUCCESS                  The module was successfully opened.
 * @retval SSP_ERR_INVALID_SIZE         Incorrect data length is set.
 * @retval SSP_ERR_INVALID_ARGUMENT     Invalid request
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_validate_key_buffer_data_length_rsa (sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                                                sf_crypto_data_handle_t const * p_user_key_input,
                                                                sf_crypto_data_handle_t const * p_key_data_out)
{
    ssp_err_t user_key_input_validity = SSP_SUCCESS;
    ssp_err_t key_data_output_validity = SSP_SUCCESS;
    uint32_t key_data_output_size = 0;
    uint32_t user_key_input_size = 0;

    switch(p_ctrl->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_RSA_1024:
        {
            user_key_input_size = (uint32_t)RSA_1024_ENCRYPTED_KEY_LENGTH_BYTES;
            key_data_output_size = (uint32_t)RSA_1024_INSTALLED_WRAPPED_KEY_SIZE;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_RSA_2048:
        {
            user_key_input_size = (uint32_t)RSA_2048_ENCRYPTED_KEY_LENGTH_BYTES;
            key_data_output_size = (uint32_t)RSA_2048_INSTALLED_WRAPPED_KEY_SIZE;
            break;
        }
            break;
        default:
            return SSP_ERR_INVALID_ARGUMENT;
    }
    user_key_input_validity =
           (user_key_input_size == (p_user_key_input->data_length)) ? SSP_SUCCESS : SSP_ERR_INVALID_SIZE;

    key_data_output_validity =
           (key_data_output_size <= (p_key_data_out->data_length)) ? SSP_SUCCESS : SSP_ERR_INVALID_SIZE;

    if ((SSP_SUCCESS == user_key_input_validity) &&
            (SSP_SUCCESS == key_data_output_validity))
    {
        return (SSP_SUCCESS);
    }
    else
    {
        return (SSP_ERR_INVALID_SIZE);
    }
} /* End of function sf_crypto_validate_key_buffer_data_length_rsa */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to validate input
 *          key length for AES key installation.
 *          This function is called by sf_crypto_validate_key_buffer_data_length().
 *
 * @param[in,out]  p_ctrl               Pointer to Crypto Key Installation Framework control block structure.
 * @param[in]      p_user_key_input     Pointer to sf_crypto_key_handle_t structure which includes a pointer to the
 *                                      WORD aligned buffer which holds the encrypted user key and the key length.
 * @param[in,out]  p_key_data_out       Pointer to sf_crypto_key_handle_t structure which includes a pointer to the
 *                                      WORD aligned buffer to hold the wrapped key and the buffer length.
 *
 * @note                                Parameter validity check is the responsibility of
 *                                      the caller of this static function.
 *
 * @retval SSP_SUCCESS                  The module was successfully opened.
 * @retval SSP_ERR_INVALID_SIZE         Incorrect data length is set.
 * @retval SSP_ERR_INVALID_ARGUMENT     Invalid request
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_validate_key_buffer_data_length_aes (sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                                                sf_crypto_data_handle_t const * p_user_key_input,
                                                                sf_crypto_data_handle_t const * p_key_data_out)
{
    ssp_err_t user_key_input_validity = SSP_ERR_INVALID_ARGUMENT;
    ssp_err_t key_data_output_validity = SSP_ERR_INVALID_ARGUMENT;
    uint32_t user_key_input_size = 0;
    uint32_t key_data_output_size = 0;
    switch(p_ctrl->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_AES_128:
        {
            user_key_input_size = (uint32_t)AES_128_ENCRYPTED_KEY_LENGTH_BYTES;
            key_data_output_size = (uint32_t)AES_128_INSTALLED_WRAPPED_KEY_SIZE;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_AES_XTS_128:
        {
            /*LDRA_INSPECTED 330 S This is not an implicit conversion */
            user_key_input_size = (uint32_t)AES_128_ENCRYPTED_XTS_KEY_LENGTH_BYTES;
            /*LDRA_INSPECTED 330 S This is not an implicit conversion */
            key_data_output_size = (uint32_t)AES_128_XTS_INSTALLED_WRAPPED_KEY_SIZE;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_AES_192:
        {
            user_key_input_size = (uint32_t)AES_192_ENCRYPTED_KEY_LENGTH_BYTES;
            key_data_output_size = (uint32_t)AES_192_INSTALLED_WRAPPED_KEY_SIZE;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_AES_256:
        {
            user_key_input_size = (uint32_t)AES_256_ENCRYPTED_KEY_LENGTH_BYTES;
            key_data_output_size = (uint32_t)AES_256_INSTALLED_WRAPPED_KEY_SIZE;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_AES_XTS_256:
        {
            /*LDRA_INSPECTED 330 S This is not an implicit conversion */
            /*LDRA_INSPECTED 96 S This is not using mixed mode arithmetic */
            user_key_input_size = (uint32_t)AES_256_ENCRYPTED_XTS_KEY_LENGTH_BYTES;
            /*LDRA_INSPECTED 330 S This is not an implicit conversion */
            /*LDRA_INSPECTED 96 S This is not using mixed mode arithmetic */
            key_data_output_size = (uint32_t)AES_256_XTS_INSTALLED_WRAPPED_KEY_SIZE;
            break;
        }
        default:
            return SSP_ERR_INVALID_ARGUMENT;
    }
    user_key_input_validity =
            (user_key_input_size == (p_user_key_input->data_length)) ? SSP_SUCCESS : SSP_ERR_INVALID_SIZE;

    key_data_output_validity =
           (key_data_output_size <= (p_key_data_out->data_length)) ? SSP_SUCCESS : SSP_ERR_INVALID_SIZE;

    if ((SSP_SUCCESS == user_key_input_validity) &&
        (SSP_SUCCESS == key_data_output_validity))
    {
        return (SSP_SUCCESS);
    }
    else
    {
        return (SSP_ERR_INVALID_SIZE);
    }
} /* End of function sf_crypto_validate_key_buffer_data_length_aes */


/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to validate input and output
 *          key length for ECC key installation.
 *          This function is called by sf_crypto_validate_key_buffer_data_length().
 *
 * @param[in,out]  p_ctrl               Pointer to Crypto Key Installation Framework control block structure.
 * @param[in]      p_user_key_input     Pointer to sf_crypto_key_handle_t structure which includes a pointer to the
 *                                      WORD aligned buffer which holds the encrypted user key and the key length.
 * @param[in,out]  p_key_data_out       Pointer to sf_crypto_key_handle_t structure which includes a pointer to the
 *                                      WORD aligned buffer to hold the wrapped key and the buffer length.
 *
 * @note                                Parameter validity check is the responsibility of
 *                                      the caller of this static function.
 *
 * @retval SSP_SUCCESS                  The module was successfully opened.
 * @retval SSP_ERR_INVALID_SIZE         Incorrect data length is set.
 * @retval SSP_ERR_INVALID_ARGUMENT     Invalid request
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_validate_key_buffer_data_length_ecc (sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                                                sf_crypto_data_handle_t const * p_user_key_input,
                                                                sf_crypto_data_handle_t const * p_key_data_out)
{
    ssp_err_t user_key_input_validity = SSP_SUCCESS;
    ssp_err_t key_data_output_validity = SSP_SUCCESS;
    uint32_t key_data_output_size = 0;
    uint32_t user_key_input_size = 0;

    switch(p_ctrl->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_ECC_192:
        {
            user_key_input_size = (uint32_t)ECC_192_ENCRYPTED_KEY_LENGTH_BYTES;
            key_data_output_size = (uint32_t)ECC_192_INSTALLED_WRAPPED_KEY_SIZE;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_ECC_256:
        {
            user_key_input_size = (uint32_t)ECC_256_ENCRYPTED_KEY_LENGTH_BYTES;
            key_data_output_size = (uint32_t)ECC_256_INSTALLED_WRAPPED_KEY_SIZE;
            break;
        }
            break;
        default:
            return SSP_ERR_INVALID_ARGUMENT;
    }
    user_key_input_validity =
           (user_key_input_size == (p_user_key_input->data_length)) ? SSP_SUCCESS : SSP_ERR_INVALID_SIZE;

    key_data_output_validity =
           (key_data_output_size <= (p_key_data_out->data_length)) ? SSP_SUCCESS : SSP_ERR_INVALID_SIZE;

    if ((SSP_SUCCESS == user_key_input_validity) &&
            (SSP_SUCCESS == key_data_output_validity))
    {
        return (SSP_SUCCESS);
    }
    else
    {
        return (SSP_ERR_INVALID_SIZE);
    }

} /* End of function sf_crypto_validate_key_buffer_data_length_ecc */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to set appropriate key buffer parameters
 *          before passing it to the HAL.
 * This function is called by SF_CRYPTO_KEY_INSTALLATION_KeyInstall().
 *
 * @param[in,out]  p_ctrl               Pointer to Crypto Key Installation Framework control block structure.
 * @param[in]      p_user_key_input     Pointer to User key input buffer.
 * @param[in]      p_session_key_input  Pointer to Session key input buffer.
 * @param[in]      p_key_data_out       Pointer to key data out buffer.
 *
 * @note                                Parameter validity check is the responsibility of
 *                                      the caller of this static function.
 *
 * @retval SSP_SUCCESS                  The module was successfully opened.
 * @retval SSP_ERR_INVALID_ARGUMENT     Invalid request
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_set_buffer_params (sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                              key_installation_key_t * p_user_key_input,
                                              key_installation_key_t * p_session_key_input,
                                              key_installation_key_t * p_key_data_out)
{
    ssp_err_t ret_val = SSP_SUCCESS;

    switch(p_ctrl->key_type)
    {
        case SF_CRYPTO_KEY_TYPE_ENCRYPTED_RSA_PRIVATE_KEY:
        {
            p_session_key_input->key_format = KEY_INSTALLATION_KEY_FORMAT_SESSION_KEY;
            p_key_data_out->key_format = KEY_INSTALLATION_KEY_FORMAT_WRAPPED_RSA_PRIVATE_KEY;
            ret_val = sf_crypto_set_user_key_buffer_params_rsa(p_ctrl, p_user_key_input, p_session_key_input);
            break;
        }
        case SF_CRYPTO_KEY_TYPE_ENCRYPTED_AES_KEY:
        {
            p_session_key_input->key_format = KEY_INSTALLATION_KEY_FORMAT_SESSION_KEY;
            p_key_data_out->key_format = KEY_INSTALLATION_KEY_FORMAT_WRAPPED_AES_KEY;
            ret_val = sf_crypto_set_user_key_buffer_params_aes(p_ctrl, p_user_key_input, p_session_key_input);
            break;
        }
        case SF_CRYPTO_KEY_TYPE_ENCRYPTED_ECC_PRIVATE_KEY:
        {
            p_session_key_input->key_format = KEY_INSTALLATION_KEY_FORMAT_SESSION_KEY;
            p_key_data_out->key_format = KEY_INSTALLATION_KEY_FORMAT_WRAPPED_ECC_PRIVATE_KEY;
            ret_val = sf_crypto_set_user_key_buffer_params_ecc(p_ctrl, p_user_key_input, p_session_key_input);
            break;
        }
        default:
            return SSP_ERR_INVALID_ARGUMENT;
    }
    return ret_val;
} /* End of function sf_crypto_set_buffer_params */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to set appropriate key buffer parameters for user key
 *          before passing it to the HAL. For Key type - SF_CRYPTO_KEY_TYPE_ENCRYPTED_AES_KEY
 * This function is called by sf_crypto_set_buffer_params().
 *
 * @param[in,out]  p_ctrl               Pointer to Crypto Key Installation Framework control block structure.
 * @param[in]      p_user_key_input     Pointer to User key input buffer.
 * @param[in]      p_session_key_input  Pointer to Session key input buffer.
 *
 * @note                                Parameter validity check is the responsibility of
 *                                      the caller of this static function.
 *
 * @retval SSP_SUCCESS                  The module was successfully opened.
 * @retval SSP_ERR_INVALID_ARGUMENT     Invalid request
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_set_user_key_buffer_params_aes (sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                                           key_installation_key_t * p_user_key_input,
                                                           key_installation_key_t * p_session_key_input)
{
    p_session_key_input->key_size = KEY_INSTALLATION_KEY_SIZE_SESSION;
    p_user_key_input->key_format = KEY_INSTALLATION_KEY_FORMAT_ENCRYPTED_AES_KEY;

    switch(p_ctrl->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_AES_128:
        {
            p_user_key_input->key_size = KEY_INSTALLATION_KEY_SIZE_AES_128;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_AES_XTS_128:
        {
            p_user_key_input->key_size = KEY_INSTALLATION_KEY_SIZE_AES_XTS_128;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_AES_192:
        {
            p_user_key_input->key_size = KEY_INSTALLATION_KEY_SIZE_AES_192;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_AES_256:
        {
            p_user_key_input->key_size = KEY_INSTALLATION_KEY_SIZE_AES_256;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_AES_XTS_256:
        {
            p_user_key_input->key_size = KEY_INSTALLATION_KEY_SIZE_AES_XTS_256;
            break;
        }
        default:
            return SSP_ERR_INVALID_ARGUMENT;
    }
    return (SSP_SUCCESS);
} /* End of function sf_crypto_set_user_key_buffer_params_aes */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to set appropriate key buffer parameters for user key
 *          before passing it to the HAL. For key type - SF_CRYPTO_KEY_TYPE_ENCRYPTED_ECC_PRIVATE_KEY
 * This function is called by sf_crypto_set_buffer_params().
 *
 * @param[in,out]  p_ctrl               Pointer to Crypto Key Installation Framework control block structure.
 * @param[in]      p_user_key_input     Pointer to User key input buffer.
 * @param[in]      p_session_key_input  Pointer to Session key input buffer.
 *
 * @note                                Parameter validity check is the responsibility of
 *                                      the caller of this static function.
 *
 * @retval SSP_SUCCESS                  The module was successfully opened.
 * @retval SSP_ERR_INVALID_ARGUMENT     Invalid request
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_set_user_key_buffer_params_ecc (sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                                           key_installation_key_t * p_user_key_input,
                                                           key_installation_key_t * p_session_key_input)
{
    p_session_key_input->key_size = KEY_INSTALLATION_KEY_SIZE_SESSION;
    p_user_key_input->key_format = KEY_INSTALLATION_KEY_FORMAT_ENCRYPTED_ECC_PRIVATE_KEY;

    switch(p_ctrl->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_ECC_192:
        {
            p_user_key_input->key_size = KEY_INSTALLATION_KEY_SIZE_ECC_192;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_ECC_256:
        {
            p_user_key_input->key_size = KEY_INSTALLATION_KEY_SIZE_ECC_256;
            break;
        }
        default:
            return SSP_ERR_INVALID_ARGUMENT;
    }
    return (SSP_SUCCESS);
} /* End of function sf_crypto_set_user_key_buffer_params_ecc */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Key Installation Framework to set appropriate key buffer parameters for user key
 *          before passing it to the HAL. For Key Type - SF_CRYPTO_KEY_TYPE_ENCRYPTED_RSA_PRIVATE_KEY
 * This function is called by sf_crypto_set_buffer_params().
 *
 * @param[in,out]  p_ctrl               Pointer to Crypto Key Installation Framework control block structure.
 * @param[in]      p_user_key_input     Pointer to User key input buffer.
 * @param[in]      p_session_key_input  Pointer to Session key input buffer.
 *
 * @note                                Parameter validity check is the responsibility of
 *                                      the caller of this static function.
 *
 * @retval SSP_SUCCESS                  The module was successfully opened.
 * @retval SSP_ERR_INVALID_ARGUMENT     Invalid request
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_set_user_key_buffer_params_rsa (sf_crypto_key_installation_instance_ctrl_t * p_ctrl,
                                                           key_installation_key_t * p_user_key_input,
                                                           key_installation_key_t * p_session_key_input)
{
    p_session_key_input->key_size = KEY_INSTALLATION_KEY_SIZE_SESSION;
    p_user_key_input->key_format = KEY_INSTALLATION_KEY_FORMAT_ENCRYPTED_RSA_PRIVATE_KEY;

    switch(p_ctrl->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_RSA_1024:
        {
            p_user_key_input->key_size = KEY_INSTALLATION_KEY_SIZE_RSA_1024;
            break;
        }
        case SF_CRYPTO_KEY_SIZE_RSA_2048:
        {
            p_user_key_input->key_size = KEY_INSTALLATION_KEY_SIZE_RSA_2048;
            break;
        }
        default:
            return SSP_ERR_INVALID_ARGUMENT;
    }
    return (SSP_SUCCESS);
} /* End of function sf_crypto_set_user_key_buffer_params_rsa */

/*******************************************************************************************************************//**
 * @brief   Private function returns the RSA modulus size given the RSA key size.
 *
 * @param[in]       rsa_key_size            RSA key size.
 * @param[in,out]   rsa_modulus_size        The RSA modulus size is returned.
 * @param[in,out]   rsa_wrapped_key_size    The RSA wrapped exponent size is returned.
 * @retval          SSP_SUCCESS             The RSA modulus was returned successfully.
 *
 * @note rsa_key_size - validity check is the responsibility of the caller of this static function.
 *
  **********************************************************************************************************************/
static void sf_crypto_key_installation_get_rsa_key_sizes (sf_crypto_key_size_t rsa_key_size,
                                                          uint32_t * rsa_modulus_size,
                                                          uint32_t * rsa_wrapped_key_size)
{
    if (SF_CRYPTO_KEY_SIZE_RSA_1024 == rsa_key_size)
    {
        *rsa_modulus_size = RSA_1024_MODULUS_LENGTH_BYTES;
        *rsa_wrapped_key_size = RSA_1024_INSTALLED_WRAPPED_KEY_SIZE;
    }
    else
    {
        /*  key_size is SF_CRYPTO_KEY_SIZE_RSA_2048 */
        *rsa_modulus_size = RSA_2048_MODULUS_LENGTH_BYTES;
        *rsa_wrapped_key_size = RSA_2048_INSTALLED_WRAPPED_KEY_SIZE;
    }
} /* End of function sf_crypto_key_installation_get_rsa_key_sizes */

/*******************************************************************************************************************//**
 * @brief   This function validates the input parameters for RSA modulus and RSA wrapped key output.
 * This function is called by SF_CRYPTO_KEY_INSTALLATION_KeyInstall2() only for
 * key type - SF_CRYPTO_KEY_TYPE_ENCRYPTED_RSA_PRIVATE_KEY
 *
 * @param[in]   rsa_key_size            RSA key size.
 * @param[in]   p_user_key_rsa_modulus  Pointer to User key modulus data handle.
 * @param[in]   p_key_data_out          Pointer to User key modulus data handle.
 *
 *
 * @retval SSP_SUCCESS              The RSA buffers are validated successfully..
 * @retval SSP_ERR_INVALID_ARGUMENT At least one of the arguments is invalid.
 * @retval SSP_ERR_INVALID_SIZE     At least one of the arguments is of invalid size.
 *
 * @note  It is the responsibility of the caller to ensure that the input parameters are valid.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_key_installation_validate_rsa_buffers (sf_crypto_key_size_t rsa_key_size,
                                                 sf_crypto_data_handle_t const * const p_user_key_rsa_modulus,
                                                 sf_crypto_data_handle_t const * const p_key_data_out)
{
    uint32_t modulus_size = 0;
    uint32_t wrapped_key_size = 0;

    sf_crypto_key_installation_get_rsa_key_sizes(rsa_key_size, &modulus_size, &wrapped_key_size);

    /** Validate the modulus input parameter */
    if ((NULL == p_user_key_rsa_modulus) || (NULL == p_user_key_rsa_modulus->p_data))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    /** Validate the length of the modulus */
    if (modulus_size != p_user_key_rsa_modulus->data_length)
    {
        return SSP_ERR_INVALID_SIZE;
    }

    /** Validate the length of the output buffer provided to hold the RSA wrapped key */
    if (wrapped_key_size > p_key_data_out->data_length)
    {
        return SSP_ERR_INVALID_SIZE;
    }

    return SSP_SUCCESS;
} /* End of function sf_crypto_key_installation_validate_rsa_buffers */
