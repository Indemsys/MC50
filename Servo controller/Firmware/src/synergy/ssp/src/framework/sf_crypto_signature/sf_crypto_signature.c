/***********************************************************************************************************************
 * Copyright [2018-2021] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : sf_crypto_signature.c
 * Description  : Crypto Signature Framework Module.
***********************************************************************************************************************/

/***********************************************************************************************************************
 * @file
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto_api.h"
#include "sf_crypto_signature_api.h"
#include "sf_crypto_signature.h"
#include "sf_crypto_signature_private.h"
#include "rsa/sf_crypto_signature_private_rsa.h"
#include "sf_crypto_signature_cfg.h"
#include "../sf_crypto/sf_crypto_private_api.h"
#include "sf_crypto_signature_common.h"
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
#if SF_CRYPTO_SIGNATURE_CFG_PARAM_CHECKING_ENABLE
static
ssp_err_t
sf_crypto_signature_validate_params_open(sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                         sf_crypto_signature_cfg_t const * const p_cfg);
static
ssp_err_t
sf_crypto_signature_validate_params_context_init(sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                                 sf_crypto_key_t const * const p_key);
static
ssp_err_t
sf_crypto_signature_validate_params_sign_update(sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                                sf_crypto_data_handle_t const * const p_message);
static
ssp_err_t
sf_crypto_signature_validate_params_verify_update(sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                                  sf_crypto_data_handle_t const * const p_message);
static
ssp_err_t
sf_crypto_signature_validate_params_sign_final(sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                               sf_crypto_data_handle_t * const p_dest);
static
ssp_err_t
sf_crypto_signature_validate_params_verify_final(sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                                 sf_crypto_data_handle_t const * const p_signature);
#endif

static
ssp_err_t
sf_crypto_signature_validate_config(sf_crypto_signature_cfg_t const * const p_cfg);
static
ssp_err_t
sf_crypto_signature_hal_open(sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                             sf_crypto_signature_cfg_t const * const p_cfg);
static
ssp_err_t
sf_crypto_signature_hal_close(sf_crypto_signature_instance_ctrl_t * const  p_ctrl);
static
ssp_err_t
sf_crypto_signature_hal_context_init(sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                     sf_crypto_signature_mode_t operation_mode,
                                     sf_crypto_signature_algorithm_init_params_t * const p_algorithm_specific_params,
                                     sf_crypto_key_t const * const p_key);
static
ssp_err_t
sf_crypto_signature_hal_sign_update(sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                    sf_crypto_data_handle_t const * const p_message);
static
ssp_err_t
sf_crypto_signature_hal_verify_update(sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                      sf_crypto_data_handle_t const * const p_message);
static
ssp_err_t
sf_crypto_signature_hal_sign_final(sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                   sf_crypto_data_handle_t const * const p_message,
                                   sf_crypto_data_handle_t * const p_dest);
static
ssp_err_t
sf_crypto_signature_hal_verify_final(sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                     sf_crypto_data_handle_t const * const p_signature,
                                     sf_crypto_data_handle_t const * const p_message);

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
static const ssp_version_t sf_crypto_signature_version =
{
    .api_version_major  = SF_CRYPTO_SIGNATURE_API_VERSION_MAJOR,
    .api_version_minor  = SF_CRYPTO_SIGNATURE_API_VERSION_MINOR,
    .code_version_major = SF_CRYPTO_SIGNATURE_CODE_VERSION_MAJOR,
    .code_version_minor = SF_CRYPTO_SIGNATURE_CODE_VERSION_MINOR
};

/***********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/*LDRA_NOANALYSIS LDRA_INSPECTED below not working. */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const sf_crypto_signature_api_t         g_sf_crypto_signature_on_sf_crypto_signature =
{
    .open           = SF_CRYPTO_SIGNATURE_Open,
    .close          = SF_CRYPTO_SIGNATURE_Close,
    .contextInit    = SF_CRYPTO_SIGNATURE_ContextInit,
    .signUpdate     = SF_CRYPTO_SIGNATURE_SignUpdate,
    .verifyUpdate   = SF_CRYPTO_SIGNATURE_VerifyUpdate,
    .signFinal      = SF_CRYPTO_SIGNATURE_SignFinal,
    .verifyFinal    = SF_CRYPTO_SIGNATURE_VerifyFinal,
    .versionGet     = SF_CRYPTO_SIGNATURE_VersionGet
};

/*LDRA_ANALYSIS */

/*******************************************************************************************************************//**
 * @ingroup SF_Library
 * @addtogroup SF_CRYPTO_SIGNATURE
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
Functions
***********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @brief   SSP Crypto Signature Framework Open operation.
 *
 * @retval SSP_SUCCESS                          The module was successfully opened.
 * @retval SSP_ERR_ASSERTION                    NULL is passed through an argument.
 * @retval SSP_ERR_INTERNAL                     Critical internal error.
 * @retval SSP_ERR_CRYPTO_COMMON_NOT_OPENED     Crypto Framework Common Module has yet been opened.
 * @retval SSP_ERR_ALREADY_OPEN                 The module has been already opened.
 * @retval SSP_ERR_UNSUPPORTED                  The module does not support the key type specified by user.
 * @return                                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_SIGNATURE_Open (sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                    sf_crypto_signature_cfg_t const * const p_cfg)
{
    ssp_err_t                               err                 = SSP_SUCCESS;
#if SF_CRYPTO_SIGNATURE_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid. */
    err = sf_crypto_signature_validate_params_open(p_api_ctrl, p_cfg);
    if (SSP_SUCCESS != err)
    {
        return (err);
    }
#endif
    sf_crypto_signature_instance_ctrl_t     * p_ctrl            = (sf_crypto_signature_instance_ctrl_t *)p_api_ctrl;
    sf_crypto_instance_ctrl_t               * p_ctrl_common;
    sf_crypto_api_t                         * p_api_common;
    sf_crypto_state_t                         common_module_status = SF_CRYPTO_CLOSED;

    /** Setup control block with common framework module (instance control and API). */
    p_ctrl->p_lower_lvl_common_ctrl = p_cfg->p_lower_lvl_crypto_common->p_ctrl;
    p_ctrl->p_lower_lvl_common_api = (sf_crypto_api_t *)p_cfg->p_lower_lvl_crypto_common->p_api;

    /** Setup control block with Hash framework module (instance). */
    p_ctrl->p_lower_lvl_sf_crypto_hash = p_cfg->p_lower_lvl_sf_crypto_hash;

    p_ctrl_common = p_ctrl->p_lower_lvl_common_ctrl;
    p_api_common = p_ctrl->p_lower_lvl_common_api;

    /** Check if Crypto Common module is opened before calling this API. */
    err = p_api_common->statusGet(p_ctrl_common, &common_module_status);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    if (SF_CRYPTO_OPENED != common_module_status)
    {
        return SSP_ERR_CRYPTO_COMMON_NOT_OPENED;
    }

    /** Check if SF Crypto Signature is already opened. */
    if (SF_CRYPTO_SIGNATURE_OPENED == p_ctrl->status)
    {
        return SSP_ERR_ALREADY_OPEN;
    }

    /** Validate configuration parameters. */
    err =  sf_crypto_signature_validate_config(p_cfg);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    /** Copy configuration parameters to control block. */
    p_ctrl->key_type = p_cfg->key_type;
    p_ctrl->key_size = p_cfg->key_size;

    /* Note: sf_crypto.lock() is not called because the HAL driver does not access HW during open.
     * The HAL interfaceGet API is also called which does not access HW either.
     * If HW will be accessed through the HAL driver, lock should be acquired. */

    /** Open Crypto HAL module. */
    err = sf_crypto_signature_hal_open(p_ctrl, p_cfg);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    /** Set Signature Framework module internal operation state to Open. */
    p_ctrl->operation_state = SF_CRYPTO_SIGNATURE_OPERATION_STATE_OPEN;
    /** Set Signature Framework module status to Open. */
    p_ctrl->status = SF_CRYPTO_SIGNATURE_OPENED;

    /** Increment Open counter to indicate a (this) module is open. */
    sf_crypto_open_counter_increment(p_ctrl_common);

    return (err);
} /* End of function SF_CRYPTO_SIGNATURE_Open */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Signature Framework Close operation.
 *
 * @retval SSP_SUCCESS          The module was successfully closed.
 * @retval SSP_ERR_ASSERTION    NULL is passed through the argument.
 * @retval SSP_ERR_INTERNAL     Critical internal error.
 * @retval SSP_ERR_NOT_OPEN     The module has yet been opened. Call Open API first.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_SIGNATURE_Close (sf_crypto_signature_ctrl_t * const p_api_ctrl)
{
#if SF_CRYPTO_SIGNATURE_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid. */
    SSP_ASSERT(p_api_ctrl);
#endif
    ssp_err_t                                       err = SSP_SUCCESS;
    sf_crypto_signature_instance_ctrl_t             * p_ctrl = (sf_crypto_signature_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t                       * p_ctrl_common;

    p_ctrl_common = p_ctrl->p_lower_lvl_common_ctrl;

    /** Check if the Crypto Framework has been opened. If not yet opened, return an error. */
    if (SF_CRYPTO_SIGNATURE_OPENED != p_ctrl->status)
    {
        return SSP_ERR_NOT_OPEN;
    }

    /* Note: sf_crypto.lock() is not called because the HAL driver does not access HW during close.
     * If HW will be accessed through the HAL driver, lock should be acquired. */

    /** Close Crypto HAL module. */
    err = sf_crypto_signature_hal_close(p_ctrl);

    if (SSP_SUCCESS == err)
    {
        p_ctrl->status = SF_CRYPTO_SIGNATURE_CLOSED;
    }
    else
    {
        return err;
    }

    /** Decrement Open counter to indicate a (this) module is closed. */
    sf_crypto_open_counter_decrement(p_ctrl_common);

    return (err);
} /* End of function SF_CRYPTO_SIGNATURE_Close */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Signature Framework Context Initialization operation.
 *
 * @retval SSP_SUCCESS                              The module context was successfully initialized.
 * @retval SSP_ERR_ASSERTION                        NULL is passed through the argument.
 * @retval SSP_ERR_INTERNAL                         Critical internal error.
 * @retval SSP_ERR_INVALID_CALL                     Invalid call to this API.
 * @retval SSP_ERR_UNSUPPORTED                      Invalid Hash module request.
 * @retval SSP_ERR_CRYPTO_INVALID_OPERATION_MODE    Invalid operation mode requested.
 * @return                                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_SIGNATURE_ContextInit (sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                          sf_crypto_signature_mode_t operation_mode,
                                          sf_crypto_signature_algorithm_init_params_t * const p_algorithm_specific_params,
                                          sf_crypto_key_t const * const p_key)
{
    ssp_err_t                               err                 = SSP_SUCCESS;
#if SF_CRYPTO_SIGNATURE_CFG_PARAM_CHECKING_ENABLE
    err = sf_crypto_signature_validate_params_context_init(p_api_ctrl, p_key);
    /** Verify parameters are valid. */
    if (SSP_SUCCESS != err)
    {
        return (err);
    }
#endif

    sf_crypto_signature_instance_ctrl_t             * p_ctrl = (sf_crypto_signature_instance_ctrl_t *) p_api_ctrl;

    /** Check if the Crypto Framework has been opened. If not yet opened, return an error. */
    if (SF_CRYPTO_SIGNATURE_OPENED != p_ctrl->status)
    {
        return SSP_ERR_NOT_OPEN;
    }

    /* Note: sf_crypto.lock() is not called because the HAL driver does not access HW during init.
     * If HW will be accessed through the HAL driver, lock should be acquired. */

    /** Setup Context for HAL. */
    err = sf_crypto_signature_hal_context_init(p_ctrl,
                                               operation_mode,
                                               p_algorithm_specific_params,
                                               p_key);

    return (err);
} /* End of function SF_CRYPTO_SIGNATURE_ContextInit */


/*******************************************************************************************************************//**
 * @brief   SSP Crypto Signature Framework Signature Update operation.
 *
 * @retval SSP_SUCCESS                              Sign update was performed successfully.
 * @retval SSP_ERR_ASSERTION                        NULL is passed through the argument.
 * @retval SSP_ERR_INTERNAL                         Critical internal error.
 * @retval SSP_ERR_INVALID_CALL                     Invalid call to this API.
 * @retval SSP_ERR_UNSUPPORTED                      Invalid Hash module request.
 * @retval SSP_ERR_CRYPTO_BUF_OVERFLOW              Update data exceeded the block size.
 * @retval SSP_ERR_CRYPTO_INVALID_OPERATION_MODE    Invalid operation mode requested.
 * @return                                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_SIGNATURE_SignUpdate (sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                          sf_crypto_data_handle_t const * const p_message)
{
    ssp_err_t                               err                 = SSP_SUCCESS;
#if SF_CRYPTO_SIGNATURE_CFG_PARAM_CHECKING_ENABLE
    err = sf_crypto_signature_validate_params_sign_update(p_api_ctrl, p_message);
    /** Verify parameters are valid. */
    if (SSP_SUCCESS != err)
    {
        return (err);
    }
#endif
    ssp_err_t                               err_lock            = SSP_SUCCESS;
    sf_crypto_signature_instance_ctrl_t             * p_ctrl = (sf_crypto_signature_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t                       * p_ctrl_common;
    sf_crypto_api_t                                 * p_api_common;

    p_ctrl_common = p_ctrl->p_lower_lvl_common_ctrl;
    p_api_common = p_ctrl->p_lower_lvl_common_api;


    /** Check if the Crypto Framework has been opened. If not yet opened, return an error. */
    if (SF_CRYPTO_SIGNATURE_OPENED != p_ctrl->status)
    {
        return SSP_ERR_NOT_OPEN;
    }

    /** Validate state transition. */
    err = sf_crypto_signature_validate_sign_operation_state_transition(p_ctrl,
                                                                 SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_UPDATED);
    if (SSP_SUCCESS != err)
    {
        return (err);
    }

    /** Lock the module to access to Crypto HAL module. */
    err = p_api_common->lock (p_ctrl_common);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Perform Sign Update. */
    err = sf_crypto_signature_hal_sign_update(p_ctrl, p_message);

    if (SSP_SUCCESS == err)
    {
        /** Set Signature Framework internal state. */
        p_ctrl->operation_state = SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_UPDATED;
    }
    /** Unlock the module. */
    err_lock = p_api_common->unlock (p_ctrl_common);
    if (SSP_SUCCESS != err)
    {
        return (err);
    }
    return (err_lock);
} /* End of function SF_CRYPTO_SIGNATURE_SignUpdate */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Signature Framework Signature-Verification Update operation.
 *
 * @retval SSP_SUCCESS                              Verify update operation was performed successfully.
 * @retval SSP_ERR_ASSERTION                        NULL is passed through the argument.
 * @retval SSP_ERR_INTERNAL                         Critical internal error.
 * @retval SSP_ERR_INVALID_CALL                     Invalid call to this API.
 * @retval SSP_ERR_UNSUPPORTED                      Invalid Hash module request.
 * @retval SSP_ERR_CRYPTO_BUF_OVERFLOW              Update data exceeded the block size.
 * @retval SSP_ERR_CRYPTO_INVALID_OPERATION_MODE    Invalid operation mode requested.
 * @return                                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_SIGNATURE_VerifyUpdate (sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                            sf_crypto_data_handle_t const * const p_message)
{
    ssp_err_t                               err                 = SSP_SUCCESS;
#if SF_CRYPTO_SIGNATURE_CFG_PARAM_CHECKING_ENABLE
    err = sf_crypto_signature_validate_params_verify_update(p_api_ctrl, p_message);
    /** Verify parameters are valid. */
    if (SSP_SUCCESS != err)
    {
        return (err);
    }
#endif
    ssp_err_t                               err_lock            = SSP_SUCCESS;
    sf_crypto_signature_instance_ctrl_t             * p_ctrl = (sf_crypto_signature_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t                       * p_ctrl_common;
    sf_crypto_api_t                                 * p_api_common;

    p_ctrl_common = p_ctrl->p_lower_lvl_common_ctrl;
    p_api_common = p_ctrl->p_lower_lvl_common_api;

    /** Check if the Crypto Framework has been opened. If not yet opened, return an error. */
    if (SF_CRYPTO_SIGNATURE_OPENED != p_ctrl->status)
    {
        return SSP_ERR_NOT_OPEN;
    }

    /** Validate state transition. */
    err = sf_crypto_signature_validate_verify_operation_state_transition(p_ctrl,
                                                                 SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_UPDATED);
    if (SSP_SUCCESS != err)
    {
        return (err);
    }

    /** Lock the module to access to Crypto HAL module. */
    err = p_api_common->lock (p_ctrl_common);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Perform Verify Update. */
    err = sf_crypto_signature_hal_verify_update(p_ctrl, p_message);

    if (SSP_SUCCESS == err)
    {
        /** Set Signature Framework internal state. */
        p_ctrl->operation_state = SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_UPDATED;
    }

    /** Unlock the module. */
    err_lock = p_api_common->unlock (p_ctrl_common);
    if (SSP_SUCCESS != err)
    {
        return (err);
    }
    return (err_lock);
} /* End of function SF_CRYPTO_SIGNATURE_VerifyUpdate */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Signature Framework Signature Final operation.
 *
 * @retval SSP_SUCCESS                              Sign Final operation was performed successfully.
 * @retval SSP_ERR_ASSERTION                        NULL is passed through the argument.
 * @retval SSP_ERR_INTERNAL                         Critical internal error.
 * @retval SSP_ERR_INVALID_CALL                     Invalid call to this API.
 * @retval SSP_ERR_CRYPTO_INVALID_SIZE              Not enough space to store signature.
 * @retval SSP_ERR_UNSUPPORTED                      Invalid Hash module request.
 * @retval SSP_ERR_CRYPTO_BUF_OVERFLOW              Update data exceeded the block size.
 * @return                                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_SIGNATURE_SignFinal (sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                         sf_crypto_data_handle_t const * const p_message,
                                         sf_crypto_data_handle_t * const p_dest)
{
    ssp_err_t                               err                 = SSP_SUCCESS;
#if SF_CRYPTO_SIGNATURE_CFG_PARAM_CHECKING_ENABLE
    err = sf_crypto_signature_validate_params_sign_final(p_api_ctrl, p_dest);
    /** Verify parameters are valid. */
    if (SSP_SUCCESS != err)
    {
        return (err);
    }
#endif
    ssp_err_t                               err_lock            = SSP_SUCCESS;
    sf_crypto_signature_instance_ctrl_t             * p_ctrl = (sf_crypto_signature_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t                       * p_ctrl_common;
    sf_crypto_api_t                                 * p_api_common;

    p_ctrl_common = p_ctrl->p_lower_lvl_common_ctrl;
    p_api_common = p_ctrl->p_lower_lvl_common_api;

    /** Check if the Crypto Framework has been opened. If not yet opened, return an error. */
    if (SF_CRYPTO_SIGNATURE_OPENED != p_ctrl->status)
    {
        return SSP_ERR_NOT_OPEN;
    }

    /** Validate state transition. */
    err = sf_crypto_signature_validate_sign_operation_state_transition(p_ctrl,
                                                                 SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_FINALIZED);
    if (SSP_SUCCESS != err)
    {
        return (err);
    }

    /** Lock the module to access to Crypto HAL module. */
    err = p_api_common->lock (p_ctrl_common);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Perform Sign Final. */
    err = sf_crypto_signature_hal_sign_final(p_ctrl, p_message, p_dest);

    if (SSP_SUCCESS == err)
    {
        /** Set Signature Framework internal state. */
        p_ctrl->operation_state = SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_FINALIZED;
    }

    /** Unlock the module. */
    err_lock = p_api_common->unlock (p_ctrl_common);

    if (SSP_SUCCESS != err)
    {
        return (err);
    }
    return (err_lock);
} /* End of function SF_CRYPTO_SIGNATURE_SignFinal */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Signature Framework Signature-Verification Update operation.
 *
 * @retval SSP_SUCCESS                              Verify final operation is performed successfully.
 * @retval SSP_ERR_ASSERTION                        NULL is passed through the argument.
 * @retval SSP_ERR_INTERNAL                         Critical internal error.
 * @retval SSP_ERR_INVALID_CALL                     Invalid call to this API.
 * @retval SSP_ERR_CRYPTO_INVALID_SIZE              Invalid signature length.
 * @retval SSP_ERR_UNSUPPORTED                      Invalid Hash module request.
 * @retval SSP_ERR_CRYPTO_BUF_OVERFLOW              Update data exceeded the block size.
 * @return                                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_SIGNATURE_VerifyFinal (sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                           sf_crypto_data_handle_t const * const p_signature,
                                           sf_crypto_data_handle_t const * const p_message)
{
    ssp_err_t                               err                 = SSP_SUCCESS;
#if SF_CRYPTO_SIGNATURE_CFG_PARAM_CHECKING_ENABLE
    err = sf_crypto_signature_validate_params_verify_final(p_api_ctrl, p_signature);
    /** Verify parameters are valid. */
    if (SSP_SUCCESS != err)
    {
        return (err);
    }
#endif
    ssp_err_t                               err_lock            = SSP_SUCCESS;
    sf_crypto_signature_instance_ctrl_t             * p_ctrl = (sf_crypto_signature_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t                       * p_ctrl_common;
    sf_crypto_api_t                                 * p_api_common;

    p_ctrl_common = p_ctrl->p_lower_lvl_common_ctrl;
    p_api_common = p_ctrl->p_lower_lvl_common_api;

    /** Check if the Crypto Framework has been opened. If not yet opened, return an error. */
    if (SF_CRYPTO_SIGNATURE_OPENED != p_ctrl->status)
    {
        return SSP_ERR_NOT_OPEN;
    }

    /** Validate state transition. */
    err = sf_crypto_signature_validate_verify_operation_state_transition(p_ctrl,
                                                                 SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_FINALIZED);
    if (SSP_SUCCESS != err)
    {
        return (err);
    }

    /** Lock the module to access to Crypto HAL module. */
    err = p_api_common->lock (p_ctrl_common);
    if (SSP_SUCCESS != err)
    {
        return (err);
    }

    /** Perform Verify Final. */
    err = sf_crypto_signature_hal_verify_final(p_ctrl, p_signature, p_message);

    if (SSP_SUCCESS == err)
    {
        /** Set Signature Framework internal state. */
        p_ctrl->operation_state = SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_FINALIZED;
    }

    /** Unlock the module. */
    err_lock = p_api_common->unlock (p_ctrl_common);

    if (SSP_SUCCESS != err)
    {
        return (err);
    }
    return (err_lock);
} /* End of function SF_CRYPTO_SIGNATURE_VerifyFinal */

/*******************************************************************************************************************//**
 * @brief      Gets driver version based on compile time macros.
 *
 * @retval     SSP_SUCCESS          Function returned successfully.
 * @retval     SSP_ERR_ASSERTION    The parameter p_version is NULL.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_SIGNATURE_VersionGet (ssp_version_t * const p_version)
{
#if SF_CRYPTO_SIGNATURE_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid. */
    SSP_ASSERT(p_version);
#endif

    p_version->version_id = sf_crypto_signature_version.version_id;

    return SSP_SUCCESS;
} /* End of function SF_CRYPTO_SIGNATURE_VersionGet */

/*******************************************************************************************************************//**
 * @} (end defgroup SF_CRYPTO_SIGNATURE)
 **********************************************************************************************************************/
/* Private Helper Functions */

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to check configuration params.
 * This helper function should be called before copying configuration params to control block.
 * This function is called by SF_CRYPTO_SIGNATURE_Open().
 *
 * @param[in]      p_cfg            Pointer to sf_crypto_signature_cfg_t configuration structure.
 *
 * @retval SSP_SUCCESS              Valid Key type and Key size pair.
 * @retval SSP_ERR_INVALID_ARGUMENT Invalid Key type and Key size pair.
 * @retval SSP_ERR_ASSERTION        Missing domain_params if applicable for crypto algorithm requested.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_validate_config (sf_crypto_signature_cfg_t const * const p_cfg)
{
    /* Validate Key Size and Key Type enumeration Pair. Check for domain_param buffer if applicable.*/
    ssp_err_t iret = SSP_ERR_INVALID_ARGUMENT;
    switch(p_cfg->key_type)
    {
        /* Fall through is deliberate */
        case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
        {
            /* Validate RSA Key Size enumeration */
            iret = sf_crypto_signature_key_size_config_rsa(p_cfg);
        break;
        }
        default:
        {
            return SSP_ERR_INVALID_ARGUMENT;
        break;
        }
    }
    return (iret);
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to open the requested HAL driver.
 * This function is called by SF_CRYPTO_SIGNATURE_Open().
 *
 * @param[in,out]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]      p_cfg            Pointer to sf_crypto_signature_cfg_t configuration structure.
 *
 * @retval SSP_SUCCESS              The module was successfully opened.
 * @retval SSP_ERR_ASSERTION        Critical internal error.
 * @retval SSP_ERR_UNSUPPORTED      Key Type specified is not supported.
 * @return                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_hal_open (sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                              sf_crypto_signature_cfg_t const * const p_cfg)
{
    ssp_err_t iret = SSP_ERR_UNSUPPORTED;
    switch(p_cfg->key_type)
    {
        /* Fall through is deliberate */
        case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
        {
            iret = sf_crypto_signature_open_rsa(p_ctrl, p_cfg);
        break;
        }
        default:
        {
            /* Must never land here as sf_crypto_signature_validate_config is always called prior to this function. */
            return SSP_ERR_UNSUPPORTED;
        break;
        }
    }
    return (iret);
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to close the requested HAL driver.
 * This function is called by SF_CRYPTO_SIGNATURE_Close().
 *
 * @param[in,out]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 *
 * @retval SSP_SUCCESS              The module was successfully closed.
 * @retval SSP_ERR_ASSERTION        Critical internal error.
 * @retval SSP_ERR_UNSUPPORTED      Key Type specified is not supported.
 * @return                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_hal_close (sf_crypto_signature_instance_ctrl_t * const  p_ctrl)
{
    ssp_err_t iret = SSP_ERR_UNSUPPORTED;
    switch(p_ctrl->key_type)
    {
        /* Fall through is deliberate */
        case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
        {
            iret = sf_crypto_signature_close_rsa(p_ctrl);
        break;
        }
        default:
        {
            return SSP_ERR_UNSUPPORTED;
        break;
        }
    }
    return (iret);
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework context to be initialized for sign/verify operations.
 * This function is called by SF_CRYPTO_SIGNATURE_ContextInit().
 *
 * @param[in]  p_ctrl                           Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]  operation_mode                   Operation mode - Sign / Verify.
 * @param[in]  p_algorithm_specific_params      Algorithm specific params.
 * @param[in]  p_key                            Pointer to a key structure.
 *
 * @retval SSP_SUCCESS                              The module context was successfully initialized.
 * @retval SSP_ERR_ASSERTION                        Critical internal error.
 * @retval SSP_ERR_INVALID_CALL                     Invalid call to this API.
 * @retval SSP_ERR_UNSUPPORTED                      Invalid Hash module request.
 * @retval SSP_ERR_CRYPTO_INVALID_OPERATION_MODE    Invalid operation mode requested.
 * @return                                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_hal_context_init (sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                      sf_crypto_signature_mode_t operation_mode,
                                      sf_crypto_signature_algorithm_init_params_t * const p_algorithm_specific_params,
                                      sf_crypto_key_t const * const p_key)
{
    ssp_err_t iret = SSP_ERR_UNSUPPORTED;
    switch(p_ctrl->key_type)
    {
        /* Fall through is deliberate */
        case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
        {
            iret = sf_crypto_signature_context_init_rsa(p_ctrl,
                                                        operation_mode,
                                                        p_algorithm_specific_params,
                                                        p_key);
        break;
        }
        default:
        {
            return SSP_ERR_UNSUPPORTED;
        break;
        }
    }
    return (iret);
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to be initialized for sign operations.
 * This function is called by SF_CRYPTO_SIGNATURE_SignUpdate().
 *
 * @param[in]  p_ctrl               Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]  p_message            Message chunk in appropriate format set at the time of initialization.
 *
 * @retval SSP_SUCCESS                              Sign update was performed successfully.
 * @retval SSP_ERR_ASSERTION                        Critical internal error.
 * @retval SSP_ERR_UNSUPPORTED                      Invalid Hash module request.
 * @retval SSP_ERR_CRYPTO_BUF_OVERFLOW              Update data exceeded the block size.
 * @retval SSP_ERR_CRYPTO_INVALID_OPERATION_MODE    Invalid operation mode requested.
 * @return                                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_hal_sign_update (sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                     sf_crypto_data_handle_t const * const p_message)
{
    ssp_err_t iret = SSP_ERR_UNSUPPORTED;
    switch(p_ctrl->key_type)
    {
        /* Fall through is deliberate */
        case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
        {
            iret = sf_crypto_signature_sign_update_rsa(p_ctrl, p_message);
        break;
        }
        default:
        {
            return SSP_ERR_UNSUPPORTED;
        break;
        }
    }
    return (iret);
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to be initialized for sign operations.
 * This function is called by SF_CRYPTO_SIGNATURE_VerifyUpdate().
 *
 * @param[in]  p_ctrl               Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]  p_message            Message chunk in appropriate format set at the time of initialization.
 *
 * @retval SSP_SUCCESS                              Verify update operation was performed successfully.
 * @retval SSP_ERR_ASSERTION                        Critical internal error.
 * @retval SSP_ERR_UNSUPPORTED                      Invalid Hash module request.
 * @retval SSP_ERR_CRYPTO_BUF_OVERFLOW              Update data exceeded the block size.
 * @retval SSP_ERR_CRYPTO_INVALID_OPERATION_MODE    Invalid operation mode requested.
 * @return                                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_hal_verify_update (sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                       sf_crypto_data_handle_t const * const p_message)
{
    ssp_err_t iret = SSP_ERR_UNSUPPORTED;
    switch(p_ctrl->key_type)
    {
        /* Fall through is deliberate */
        case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
        {
            iret = sf_crypto_signature_verify_update_rsa(p_ctrl, p_message);
        break;
        }
        default:
        {
            return SSP_ERR_UNSUPPORTED;
        break;
        }
    }
    return (iret);
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to be initialized for sign operations.
 * This function is called by SF_CRYPTO_SIGNATURE_SignFinal().
 *
 * @param[in]  p_ctrl              Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]  p_message           Last block of message in appropriate format set at the time of initialization.
 * @param[out] p_dest              Signature output.
 *
 * @retval SSP_SUCCESS                              Sign Final operation was performed successfully.
 * @retval SSP_ERR_ASSERTION                        Critical internal error.
 * @retval SSP_ERR_CRYPTO_INVALID_SIZE              Not enough space to store signature.
 * @retval SSP_ERR_UNSUPPORTED                      Invalid Hash module request.
 * @retval SSP_ERR_CRYPTO_BUF_OVERFLOW              Update data exceeded the block size.
 * @return                                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_hal_sign_final (sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                    sf_crypto_data_handle_t const * const p_message,
                                    sf_crypto_data_handle_t * const p_dest)
{
    ssp_err_t iret = SSP_ERR_UNSUPPORTED;
    switch(p_ctrl->key_type)
    {
        /* Fall through is deliberate */
        case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
        {
            iret = sf_crypto_signature_sign_final_rsa(p_ctrl, p_message, p_dest);
        break;
        }
        default:
        {
            return SSP_ERR_UNSUPPORTED;
        break;
        }
    }
    return (iret);
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to be initialized for sign operations.
 * This function is called by SF_CRYPTO_SIGNATURE_VerifyFinal().
 *
 * @param[in]  p_ctrl              Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]  p_message           Last block of message in appropriate format set at the time of initialization.
 * @param[in]  p_signature         Signature Input for verification.
 *
 *
 * @retval SSP_SUCCESS                              Verify final operation is performed successfully.
 * @retval SSP_ERR_ASSERTION                        Critical internal error.
 * @retval SSP_ERR_CRYPTO_INVALID_SIZE              Invalid signature length.
 * @retval SSP_ERR_UNSUPPORTED                      Invalid Hash module request.
 * @retval SSP_ERR_CRYPTO_BUF_OVERFLOW              Update data exceeded the block size.
 * @return                                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_hal_verify_final (sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                      sf_crypto_data_handle_t const * const p_signature,
                                      sf_crypto_data_handle_t const * const p_message)
{
    ssp_err_t iret = SSP_ERR_UNSUPPORTED;
    switch(p_ctrl->key_type)
    {
        /* Fall through is deliberate */
        case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
        {
            iret = sf_crypto_signature_verify_final_rsa(p_ctrl, p_signature, p_message);
        break;
        }
        default:
        {
            return SSP_ERR_UNSUPPORTED;
        break;
        }
    }
    return (iret);
}

#if SF_CRYPTO_SIGNATURE_CFG_PARAM_CHECKING_ENABLE

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to check open API parameters.
 * This function is called by SF_CRYPTO_SIGNATURE_Open().
 *
 * @param[in]  p_api_ctrl           Pointer to Crypto Signature Framework control block structure.
 * @param[in]  p_cfg                Pointer to sf_crypto_signature_cfg_t configuration structure.
 *
 * @retval SSP_SUCCESS              API parameters are successfully validated.
 * @retval SSP_ERR_ASSERTION        NULL is passed through an argument.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_validate_params_open (sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                          sf_crypto_signature_cfg_t const * const p_cfg)
{
    SSP_ASSERT(p_api_ctrl);
    SSP_ASSERT(p_cfg);
    return (SSP_SUCCESS);
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to check context initialization API parameters.
 * This function is called by SF_CRYPTO_SIGNATURE_ContextInit().
 *
 * @param[in]  p_api_ctrl           Pointer to Crypto Signature Framework control block structure.
 * @param[in]  p_key                Pointer to key data handle
 *
 * @retval SSP_SUCCESS              API parameters are successfully validated.
 * @retval SSP_ERR_ASSERTION        NULL is passed through an argument.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_validate_params_context_init (sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                                  sf_crypto_key_t const * const p_key)
{
    SSP_ASSERT(p_api_ctrl);
    SSP_ASSERT(p_key);
    SSP_ASSERT(p_key->p_data);
    SSP_ASSERT(p_key->data_length);
    return (SSP_SUCCESS);
}


/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to check sign update API parameters.
 * This function is called by SF_CRYPTO_SIGNATURE_signUpdate().
 *
 * @param[in]  p_api_ctrl           Pointer to Crypto Signature Framework control block structure.
 * @param[in]  p_message            Pointer to message data handle.
 *
 * @retval SSP_SUCCESS              API parameters are successfully validated.
 * @retval SSP_ERR_ASSERTION        NULL is passed through an argument.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_validate_params_sign_update (sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                                 sf_crypto_data_handle_t const * const p_message)
{
    SSP_ASSERT(p_api_ctrl);
    SSP_ASSERT(p_message);
    SSP_ASSERT(p_message->p_data);
    SSP_ASSERT(p_message->data_length);
    return (SSP_SUCCESS);
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to check verify update API parameters.
 * This function is called by SF_CRYPTO_SIGNATURE_verifyUpdate().
 *
 * @param[in]  p_api_ctrl           Pointer to Crypto Signature Framework control block structure.
 * @param[in]  p_message            Pointer to message data handle.
 *
 * @retval SSP_SUCCESS              API parameters are successfully validated.
 * @retval SSP_ERR_ASSERTION        NULL is passed through an argument.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_validate_params_verify_update (sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                                   sf_crypto_data_handle_t const * const p_message)
{
    SSP_ASSERT(p_api_ctrl);
    SSP_ASSERT(p_message);
    SSP_ASSERT(p_message->p_data);
    SSP_ASSERT(p_message->data_length);
    return (SSP_SUCCESS);
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to check sign final API parameters.
 * This function is called by SF_CRYPTO_SIGNATURE_signFinal().
 *
 * @param[in]  p_api_ctrl           Pointer to Crypto Signature Framework control block structure.
 * @param[in]  p_dest               Pointer to destination data handle to return signature.
 *
 * @retval SSP_SUCCESS              API parameters are successfully validated.
 * @retval SSP_ERR_ASSERTION        NULL is passed through an argument.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_validate_params_sign_final (sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                                sf_crypto_data_handle_t * const p_dest)
{
    SSP_ASSERT(p_api_ctrl);
    SSP_ASSERT(p_dest);
    SSP_ASSERT(p_dest->p_data);
    SSP_ASSERT(p_dest->data_length);
    return (SSP_SUCCESS);
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to check verify final API parameters.
 * This function is called by SF_CRYPTO_SIGNATURE_verifyFinal().
 *
 * @param[in]  p_api_ctrl           Pointer to Crypto Signature Framework control block structure.
 * @param[in]  p_signature          Pointer to signature data handle.
 *
 * @retval SSP_SUCCESS              API parameters are successfully validated.
 * @retval SSP_ERR_ASSERTION        NULL is passed through an argument.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_validate_params_verify_final (sf_crypto_signature_ctrl_t * const p_api_ctrl,
                                                  sf_crypto_data_handle_t const * const p_signature)
{
    SSP_ASSERT(p_api_ctrl);
    SSP_ASSERT(p_signature);
    SSP_ASSERT(p_signature->p_data);
    SSP_ASSERT(p_signature->data_length);
    return (SSP_SUCCESS);
}
#endif
