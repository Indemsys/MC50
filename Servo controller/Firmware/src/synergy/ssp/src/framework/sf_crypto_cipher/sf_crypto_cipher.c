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
 * File Name    : sf_crypto_cipher.c
 * Description  : SSP Crypto Cipher Framework Module
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @file
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto_cipher.h"
#include "../sf_crypto/sf_crypto_private_api.h"
#include "aes/sf_crypto_cipher_aes_private.h"
#include "rsa/sf_crypto_cipher_rsa_private.h"
#include "sf_crypto_cipher_private_api.h"


/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

#ifndef SF_CRYPTO_CIPHER_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define SF_CRYPTO_CIPHER_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &sf_crypto_cipher_version)
#endif

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
#if SF_CRYPTO_CIPHER_CFG_PARAM_CHECKING_ENABLE
static ssp_err_t sf_crypto_cipher_open_param_check(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                 sf_crypto_cipher_cfg_t const * const p_cfg);
#endif

static void sf_crypto_cipher_determine_algorithm(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);

static ssp_err_t sf_crypto_cipher_validate_cfg_parameters(sf_crypto_cipher_cfg_t const * const p_cfg);
static bool sf_crypto_cipher_is_state_to_init(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);
static ssp_err_t sf_crypto_cipher_check_state_for_update(sf_crypto_cipher_state_t status);
static ssp_err_t sf_crypto_cipher_check_state_for_aad_update(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                              sf_crypto_cipher_state_t status);
static ssp_err_t sf_crypto_cipher_check_state_for_final(sf_crypto_cipher_state_t status);
static bool sf_crypto_cipher_is_module_opened(sf_crypto_cipher_state_t status);

static ssp_err_t sf_crypto_cipher_process_init(sf_crypto_cipher_instance_ctrl_t * p_ctrl,
                                               sf_crypto_cipher_op_mode_t cipher_operation_mode,
                                               sf_crypto_key_t const * const p_key,
                                               sf_crypto_cipher_algorithm_init_params_t * p_algorithm_specific_params);
static ssp_err_t sf_crypto_cipher_process_update(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                 sf_crypto_data_handle_t const * const p_data_in,
                                                 sf_crypto_data_handle_t * const p_data_out);
static ssp_err_t sf_crypto_cipher_process_final(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                sf_crypto_data_handle_t const * const p_data_in,
                                                sf_crypto_data_handle_t * const p_data_out);
static ssp_err_t sf_crypto_cipher_initialize_instance (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                       sf_crypto_cipher_cfg_t const * const p_cfg);
static ssp_err_t sf_crypto_cipher_deinitialize_instance (sf_crypto_cipher_instance_ctrl_t * const p_ctrl);

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
static const ssp_version_t sf_crypto_cipher_version =
{
    .api_version_major  = SF_CRYPTO_CIPHER_API_VERSION_MAJOR,
    .api_version_minor  = SF_CRYPTO_CIPHER_API_VERSION_MINOR,
    .code_version_major = SF_CRYPTO_CIPHER_CODE_VERSION_MAJOR,
    .code_version_minor = SF_CRYPTO_CIPHER_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif
/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "sf_crypto_cipher";
#endif

/*******************************************************************************************************************//**
 * Implementation of Interface
 **********************************************************************************************************************/
/*LDRA_NOANALYSIS LDRA_INSPECTED below not working. */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const sf_crypto_cipher_api_t    g_sf_crypto_cipher_on_sf_crypto_cipher =
{
    .open               = SF_CRYPTO_CIPHER_Open,
    .close              = SF_CRYPTO_CIPHER_Close,
    .cipherInit         = SF_CRYPTO_CIPHER_CipherInit,
    .cipherAadUpdate    = SF_CRYPTO_CIPHER_CipherAadUpdate,
    .cipherUpdate       = SF_CRYPTO_CIPHER_CipherUpdate,
    .cipherFinal        = SF_CRYPTO_CIPHER_CipherFinal,
    .versionGet         = SF_CRYPTO_CIPHER_VersionGet
};

/*LDRA_ANALYSIS */

/*******************************************************************************************************************//**
* @ingroup SF_Library
* @addtogroup SF_CRYPTO_CIPHER
* @{
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Cipher Framework Open operation.
 *
 * The SF_CRYTO_CIPHER_Open function:
 * Allocates memory required for the cipher operations based on the configuration parameters.
 * Acquires lock for the shared crypto resources.
 * Gets the interface to the HAL driver based on the config parameters.
 * Calls the .open function of the HAL API.
 * On successful open, the module status is updated as such.
 * The shared resources are unlocked before exit.
 *
 * @retval SSP_SUCCESS                      The module was successfully opened.
 * @retval SSP_ERR_ASSERTION                One or more input parameters maybe NULL.
 * @retval SSP_ERR_INVALID_ARGUMENT         An invalid argument is used.
 * @retval SSP_ERR_CRYPTO_COMMON_NOT_OPENED Crypto Framework Common Module has yet been opened.
 * @retval SSP_ERR_ALREADY_OPEN             The module has been already opened.
 * @retval SSP_ERR_UNSUPPORTED              The module does not support the key type specified by user.
 * @retval SSP_ERR_INTERNAL                 An internal ThreadX error has occurred. This is typically a failure to
 *                                          create/use a mutex or to create an internal thread.
 * @return                                  See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_CIPHER_Open (sf_crypto_cipher_ctrl_t * const p_api_ctrl,
                                 sf_crypto_cipher_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_instance_ctrl_t * p_ctrl = (sf_crypto_cipher_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t * p_fwk_common_ctrl;
    sf_crypto_api_t * p_lower_lvl_fwk_common_api;
    sf_crypto_state_t common_module_status = SF_CRYPTO_CLOSED;

#if SF_CRYPTO_CIPHER_CFG_PARAM_CHECKING_ENABLE
    /** Verify if parameters are valid */
    err = sf_crypto_cipher_open_param_check(p_ctrl, p_cfg);
    SF_CRYPTO_CIPHER_ERROR_RETURN(SSP_SUCCESS == err, err);
#endif

    /** Get a Crypto common control block and the interface. */
    p_ctrl->p_lower_lvl_fwk_common_ctrl = p_cfg->p_lower_lvl_crypto_common->p_ctrl;
    p_fwk_common_ctrl = p_ctrl->p_lower_lvl_fwk_common_ctrl;

    p_ctrl->p_lower_lvl_fwk_common_api  = (sf_crypto_api_t *)p_cfg->p_lower_lvl_crypto_common->p_api;
    p_lower_lvl_fwk_common_api  = p_ctrl->p_lower_lvl_fwk_common_api;

    /** Get the pointers to TRNG control and API structures from the instance pointed to by the config structure. */
    p_ctrl->p_lower_lvl_sf_crypto_trng_ctrl = (sf_crypto_trng_ctrl_t *)p_cfg->p_lower_lvl_crypto_trng->p_ctrl;
    p_ctrl->p_sf_crypto_trng_api = (sf_crypto_trng_api_t *)p_cfg->p_lower_lvl_crypto_trng->p_api;


    /** Check if the Crypto Framework has been opened. If not yet opened, return an error. */
    err = p_lower_lvl_fwk_common_api->statusGet(p_fwk_common_ctrl, &common_module_status);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    if (SF_CRYPTO_OPENED != common_module_status)
    {
         return SSP_ERR_CRYPTO_COMMON_NOT_OPENED;
    }

    /** Check if the Cipher module has been already opened. If opened, return an error. */
    if (sf_crypto_cipher_is_module_opened(p_ctrl->status))
    {
        return SSP_ERR_ALREADY_OPEN;
    }

    /** Validate the cfg parameters */
    err = sf_crypto_cipher_validate_cfg_parameters(p_cfg);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

     /** Fill the control block with the validated cfg parameters */
    p_ctrl->key_type = p_cfg->key_type;
    p_ctrl->key_size = p_cfg->key_size;
    p_ctrl->cipher_chaining_mode = p_cfg->cipher_chaining_mode;

    /** Determine and fill the algorithm type in the control block. */
    sf_crypto_cipher_determine_algorithm(p_ctrl);

    /** Note: sf_crypto.lock() is not called because the HAL driver does not access HW during open.
     * The HAL interfaceGet API is also called which does not access HW either.
     * If HW will be accessed through the HAL driver, lock should be acquired. */

    /** Allocate memory resources used by this framework instance and open HAL driver */
    err = sf_crypto_cipher_initialize_instance(p_ctrl, p_cfg);
    if (SSP_SUCCESS == err)
    {
       p_ctrl->status = SF_CRYPTO_CIPHER_STATE_OPENED;
       sf_crypto_open_counter_increment(p_fwk_common_ctrl);
    }

    return (err);

} /* End of function SF_CRYPTO_CIPHER_Open */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Cipher Framework Close operation.
 *
 * @retval SSP_SUCCESS          The module was successfully closed.
 * @retval SSP_ERR_ASSERTION    One or more input parameters maybe NULL.
 * @retval SSP_ERR_NOT_OPEN     The module has yet been opened. Call Open API first.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_CIPHER_Close (sf_crypto_cipher_ctrl_t * const p_api_ctrl)
{
    ssp_err_t   err = SSP_SUCCESS;
    sf_crypto_cipher_instance_ctrl_t * p_ctrl = (sf_crypto_cipher_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t    * p_fwk_common_ctrl;

#if SF_CRYPTO_CIPHER_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid. */
    SSP_ASSERT(p_ctrl);
#endif

    /** Get a Crypto common control block and the interface. */
    p_fwk_common_ctrl = p_ctrl->p_lower_lvl_fwk_common_ctrl;

    /** Check if the Crypto Framework Cipher  module has been opened. If not yet opened, return an error. */

    if (!sf_crypto_cipher_is_module_opened(p_ctrl->status))
    {
         return SSP_ERR_NOT_OPEN;
    }

    /** Note: sf_crypto.lock() is not called because the HAL driver does not access HW during close.
     * If HW will be accessed through the HAL driver, lock should be acquired. */


    /** Free the memory allocated for this instance and close the HAL driver. */
    err = sf_crypto_cipher_deinitialize_instance(p_ctrl);

    if (SSP_SUCCESS == err)
    {
        p_ctrl->status = SF_CRYPTO_CIPHER_STATE_CLOSED;
        sf_crypto_open_counter_decrement(p_fwk_common_ctrl);
    }

    return (err);

} /* End of function SF_CRYPTO_CIPHER_Close */

/*******************************************************************************************************************//**
 * @brief      Gets driver version based on compile time macros.
 *
 * @retval     SSP_SUCCESS          The version is returned successfully.
 * @retval     SSP_ERR_ASSERTION    The parameter p_version is NULL.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_CIPHER_VersionGet (ssp_version_t * const p_version)
{
#if SF_CRYPTO_CIPHER_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid. */
    SSP_ASSERT(p_version);
#endif

    p_version->version_id = sf_crypto_cipher_version.version_id;

    return SSP_SUCCESS;
} /* End of function SF_CRYPTO_KEY_VersionGet */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Framework Cipher Init operation. The input parameters initialize the cipher operation.
 * Some of the parameters are algorithm specific. The input parameters are validated and then copied into the
 * context buffer.
 * Refer to sf_crypto_cipher_aes_init_params_t or sf_crypto_cipher_rsa_init_params_t.
 *
 * @retval SSP_SUCCESS              The module is initialized for cipher operation successfully.
 * @retval SSP_ERR_ASSERTION        NULL is passed through an argument.
 * @retval SSP_ERR_NOT_OPEN         The module has yet been opened. Call Open API first.
 * @retval SSP_ERR_INVALID_ARGUMENT One of the input parameters is invalid.
 * @retval SSP_ERR_UNSUPPORTED      The module does not support the key type specified by user.
 * @return                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_CIPHER_CipherInit (sf_crypto_cipher_ctrl_t * const p_api_ctrl,
                                       sf_crypto_cipher_op_mode_t cipher_operation_mode,
                                       sf_crypto_key_t const * const p_key,
                                       sf_crypto_cipher_algorithm_init_params_t * const p_algorithm_specific_params)
{
    ssp_err_t   err = SSP_SUCCESS;
    sf_crypto_cipher_instance_ctrl_t * p_ctrl = (sf_crypto_cipher_instance_ctrl_t *) p_api_ctrl;

#if SF_CRYPTO_CIPHER_CFG_PARAM_CHECKING_ENABLE
    /** Verify if parameters are valid */
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_key);
    SSP_ASSERT(p_algorithm_specific_params);
#endif

    /** Validate cipher operation mode */
    if ((cipher_operation_mode != SF_CRYPTO_CIPHER_OP_MODE_ENCRYPT) &&
        (cipher_operation_mode != SF_CRYPTO_CIPHER_OP_MODE_DECRYPT))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }


    /** Check if the module can transition to the initialized state. */
     if (!sf_crypto_cipher_is_state_to_init(p_ctrl))
    {
        return SSP_ERR_NOT_OPEN;
    }

    /** Note: sf_crypto.lock() is not called because the HAL driver does not access HW during init.
     * If HW will be accessed through the HAL driver, lock should be acquired. */

    /** Do algorithm specific init */
    err = sf_crypto_cipher_process_init(p_ctrl,
                                        cipher_operation_mode,
                                        p_key,
                                        p_algorithm_specific_params);
    if(SSP_SUCCESS == err)
    {
        /** Mark the module status as 'Initialized'. */
        p_ctrl->status = SF_CRYPTO_CIPHER_STATE_INITIALIZED;
    }

        return err;
} /* End of function SF_CRYPTO_CIPHER_CipherInit */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Cipher Framework - Encrypts / Decrypts the input data and writes the cipher-text or plain-text to
 * the output buffer.  Can be called  multiple times for chunks of data.
 *
 * @retval SSP_SUCCESS          The function updated a cipher operation successfully.
 * @retval SSP_ERR_ASSERTION    NULL is passed through an argument.
 * @retval SSP_ERR_NOT_OPEN     The module has yet been opened. Call Open API first.
 * @retval SSP_ERR_UNSUPPORTED  Key types / algorithms are not supported for the MCU part.
 * @retval SSP_ERR_INVALID_CALL Function call was made if the module state had not yet transitioned to
 *                              SF_CRYPTO_CIPHER_STATE_INITIALIZED.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 *
 * @note 1. This is a blocking call.
 * @note 2. The input and output buffers have to be WORD aligned.
 * @note 3. On encryption and decryption operations, block alignment considerations may require that the number of
 *          bytes written into output buffer be larger or smaller than input data length or even 0.
 * @note 4. The output data area must not partially overlap the input data area such that the input data is modified before
 *          it is used else incorrect output may result.
 * @note 5. If the length of input data is 0 this method does nothing.
 * @note 6. For all Cipher operations cipherFinal() must be called to finalize the operation.
 * @note 7. For RSA Operations, no data is output unless cipherFinal() is called.
  **********************************************************************************************************************/
ssp_err_t   SF_CRYPTO_CIPHER_CipherUpdate (sf_crypto_cipher_ctrl_t * const p_api_ctrl,
                                           sf_crypto_data_handle_t const * const p_data_in,
                                           sf_crypto_data_handle_t  * const p_data_out)
{
    ssp_err_t   err = SSP_SUCCESS;
    sf_crypto_cipher_instance_ctrl_t * p_ctrl = (sf_crypto_cipher_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t * p_fwk_common_ctrl;
    sf_crypto_api_t * p_lower_lvl_fwk_common_api;

#if SF_CRYPTO_CIPHER_CFG_PARAM_CHECKING_ENABLE
    /** Verify if parameters are valid */
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_data_in);
    SSP_ASSERT(p_data_out);
#endif

    /** If input data length is 0 - do nothing return success. */
    /** The status will be set to 'updated' only when data is processed. */

    if (0U == p_data_in->data_length)
    {
        return err;
    }

    /** Get a Crypto Framework common control block and the interface. */
    p_fwk_common_ctrl = p_ctrl->p_lower_lvl_fwk_common_ctrl;
    p_lower_lvl_fwk_common_api  = (sf_crypto_api_t *)p_ctrl->p_lower_lvl_fwk_common_api;

     /** Check if the Crypto Cipher Framework is in the correct state to transition to do the update operation.
     * If module is not opened or initialized, return an error.
     */
    err = sf_crypto_cipher_check_state_for_update (p_ctrl->status);
    if (SSP_SUCCESS == err)
    {

        /** Acquire the lock from the common module to access the Crypto HAL driver. */
        err = p_lower_lvl_fwk_common_api->lock(p_fwk_common_ctrl);
    }

     if (SSP_SUCCESS == err)
    {
        err = sf_crypto_cipher_process_update(p_ctrl, p_data_in, p_data_out);
        if (SSP_SUCCESS == err)
        {
            /* Mark the module status as 'Updated'. */
            p_ctrl->status = SF_CRYPTO_CIPHER_STATE_UPDATED;
        }

        /* Unlock the module. */
        ssp_err_t err_unlock = p_lower_lvl_fwk_common_api->unlock(p_fwk_common_ctrl);
        if (SSP_SUCCESS == err)
        {
            /** If update processing has succeeded return the error from unlock. */
            err = err_unlock;
        }
    }

    return (err);
} /* End of function SF_CRYPTO_CIPHER_CipherUpdate */

/*******************************************************************************************************************//**
 * @brief Updates Additional Authenticated Data. This is applicable only to AES GCM.
 * Can be called  multiple times for chunks of data.
 *
 * @retval SSP_SUCCESS          The function updated the AAD successfully.
 * @retval SSP_ERR_ASSERTION    NULL is passed through an argument.
 * @retval SSP_ERR_NOT_OPEN     The module has yet been opened. Call Open API first.
 * @retval SSP_ERR_UNSUPPORTED  Key types / algorithms are not supported for the MCU part.
 * @retval SSP_ERR_INVALID_CALL Function call was made if the module state had not yet transitioned to
 *                              SF_CRYPTO_CIPHER_STATE_INITIALIZED.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 *
 * @note 1. This is a blocking call.
 * @note 2. This function has to be called before any call to cipherUpdate / cipherFinal is made.
 * @note 3. The data buffer must be WORD aligned.
  **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_CIPHER_CipherAadUpdate (sf_crypto_cipher_ctrl_t * const p_api_ctrl,
                                            sf_crypto_data_handle_t const * const p_aad)
{
    ssp_err_t  err = SSP_SUCCESS;
    sf_crypto_cipher_instance_ctrl_t * p_ctrl = (sf_crypto_cipher_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t * p_fwk_common_ctrl;
    sf_crypto_api_t * p_lower_lvl_fwk_common_api;

#if SF_CRYPTO_CIPHER_CFG_PARAM_CHECKING_ENABLE
    /** Verify if parameters are valid */
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_aad);
    SSP_ASSERT (p_aad->p_data != NULL);
#endif

    if (0U == p_aad->data_length)
    {
        /** do nothing - similar to the cipherUpdate API */
        return SSP_SUCCESS;
    }

    /** Get a Crypto Framework common control block and the interface. */
    p_fwk_common_ctrl = p_ctrl->p_lower_lvl_fwk_common_ctrl;
    p_lower_lvl_fwk_common_api  = (sf_crypto_api_t *)p_ctrl->p_lower_lvl_fwk_common_api;

    /** Check if the Crypto Cipher Framework is in the correct state to execute this operation.
     * If module is not opened or initialized, return an error.
     */
    err = sf_crypto_cipher_check_state_for_aad_update (p_ctrl, p_ctrl->status);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    err = sf_crypto_cipher_aes_validate_aad_update_params_context(p_ctrl);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Acquire the lock from the common module to access the Crypto HAL driver. */
    err = p_lower_lvl_fwk_common_api->lock(p_fwk_common_ctrl);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    err = sf_crypto_cipher_aes_aad_update(p_ctrl, p_aad);
    /** AAD is optional so no need to track if it is updated.*/

    /** Unlock the module. */
    ssp_err_t err_unlock = p_lower_lvl_fwk_common_api->unlock(p_fwk_common_ctrl);
    if (SSP_SUCCESS == err)
    {
        /** if update processing has succeeded return the error from unlock. */
        err = err_unlock;
    }
    return err;
} /* End of function SF_CRYPTO_CIPHER_CipherAadUpdate */


/*******************************************************************************************************************//**
 * @brief   SSP Crypto Cipher Framework - Generates encrypted / decrypted output from all/last input data.
 * This function processes any remaining input data buffered by one or more calls to the cipherUpdate()API
 * as well as input data supplied in the input parameter.
 * This function must be invoked to complete a cipher operation.
 *
 * @retval SSP_SUCCESS          The module updated a message Digest successfully.
 * @retval SSP_ERR_ASSERTION    NULL is passed through an argument.
 * @retval SSP_ERR_NOT_OPEN     The module has yet been opened. Call Open API first.
 * @retval SSP_ERR_UNSUPPORTED  Key types / algorithms are not supported for the MCU part.
 * @retval SSP_ERR_INVALID_CALL Function call was made if the module state had not yet transitioned to
 *                              SF_CRYPTO_CIPHER_STATE_INITIALIZED or SF_CRYPTO_CIPHER_STATE_UPDATED.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 *
 * @note 1. This is a blocking call.
 * @note 2. On encryption and decryption operations, block alignment considerations may require that the number of
 *          bytes written into output buffer be larger or smaller than input data length or even 0.
 * @note 3. The output data area must not partially overlap the input data area such that the input data is modified before
 *          it is used else incorrect output may result.
 * @note 4. Except for GCM, if the length of input data is 0 and no input was provided through update, an error is returned.
 * @note 5. On decryption operations the padding bytes are not written to p_data_out.p_data.
 * @note 6. The input and output buffers must be WORD aligned.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_CIPHER_CipherFinal (sf_crypto_cipher_ctrl_t * const p_api_ctrl,
                                        sf_crypto_data_handle_t const * const p_data_in,
                                        sf_crypto_data_handle_t  * const p_data_out)
{
    ssp_err_t  err = SSP_SUCCESS;
    sf_crypto_cipher_instance_ctrl_t * p_ctrl = (sf_crypto_cipher_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t * p_fwk_common_ctrl = NULL;
    sf_crypto_api_t * p_lower_lvl_fwk_common_api = NULL;

#if SF_CRYPTO_CIPHER_CFG_PARAM_CHECKING_ENABLE
    /** Verify if parameters are valid */
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_data_out);
#endif

    /** Get a Crypto Framework common control block and the interface. */
    p_fwk_common_ctrl = p_ctrl->p_lower_lvl_fwk_common_ctrl;
    p_lower_lvl_fwk_common_api  = (sf_crypto_api_t *)p_ctrl->p_lower_lvl_fwk_common_api;


    /** Check if the Crypto Cipher Framework is in the correct state to execute this operation.
     * If module is not opened or initialized, return an error.
     */
    err = sf_crypto_cipher_check_state_for_final (p_ctrl->status);
    if (SSP_SUCCESS != err)
    {
         return err;
    }

    /** Acquire the lock from the common module to access to Crypto HAL driver. */
    err = p_lower_lvl_fwk_common_api->lock(p_fwk_common_ctrl);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    err = sf_crypto_cipher_process_final(p_ctrl, p_data_in, p_data_out);
    if (SSP_SUCCESS == err)
    {
        /** Mark the module status as 'Finalized'. */
        p_ctrl->status = SF_CRYPTO_CIPHER_STATE_FINALIZED;
    }

    /** Unlock the module. */
    ssp_err_t err_unlock = p_lower_lvl_fwk_common_api->unlock(p_fwk_common_ctrl);
    if (SSP_SUCCESS == err)
    {
         /** if final processing has succeeded return the error from unlock. */
        err = err_unlock;
    }
    return err;
} /* End of function SF_CRYPTO_CIPHER_CipherFinal */

/*******************************************************************************************************************//**
 * @} (end defgroup SF_CRYPTO_CIPHER)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/


#if SF_CRYPTO_CIPHER_CFG_PARAM_CHECKING_ENABLE
/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto KeyFramework to check open API parameters.
 * This function is called by SF_CRYPTO_KEY_Open().
 *
 * @retval SSP_SUCCESS          The module was successfully opened.
 * @retval SSP_ERR_ASSERTION    NULL is passed through an argument.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_open_param_check (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                    sf_crypto_cipher_cfg_t const * const p_cfg)
{
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_cfg);
    SSP_ASSERT(p_cfg->p_lower_lvl_crypto_common);
    SSP_ASSERT(p_cfg->p_lower_lvl_crypto_trng);
    return SSP_SUCCESS;
}
#endif


/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto  Cipher Framework to validate the cfg parameters supplied at Open()
 *          This function is called by SF_CRYPTO_CIPHER_Open().
 *
 * @param[in]  p_cfg            Pointer to the configuration structure for the Cipher module.

 * @retval SSP_SUCCESS          The key type /size and chaining mode is successfully determined.
 * @retval SSP_ERR_UNSUPPORTED  The module does not support the configuration parameters specified by user.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_validate_cfg_parameters (sf_crypto_cipher_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;

    switch (p_cfg->key_type)
    {
        case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
           switch(p_cfg->key_size)
           {
               case SF_CRYPTO_KEY_SIZE_RSA_1024:
               case SF_CRYPTO_KEY_SIZE_RSA_2048:
               break;
               default:
                   err = SSP_ERR_INVALID_ARGUMENT;
               break;
           }
        break;

        case SF_CRYPTO_KEY_TYPE_AES_PLAIN_TEXT:
        case SF_CRYPTO_KEY_TYPE_AES_WRAPPED:
           switch(p_cfg->key_size)
           {
               case SF_CRYPTO_KEY_SIZE_AES_128:
               case SF_CRYPTO_KEY_SIZE_AES_256:
               case SF_CRYPTO_KEY_SIZE_AES_192:
                   switch(p_cfg->cipher_chaining_mode)
                   {
                       case SF_CRYPTO_CIPHER_MODE_ECB:
                       case SF_CRYPTO_CIPHER_MODE_CBC:
                       case SF_CRYPTO_CIPHER_MODE_CTR:
                       case SF_CRYPTO_CIPHER_MODE_GCM:
                       break;

                       default:
                           err = SSP_ERR_INVALID_ARGUMENT;
                       break;
                   }
               break;
               case SF_CRYPTO_KEY_SIZE_AES_XTS_128:
               case SF_CRYPTO_KEY_SIZE_AES_XTS_256:
                   switch(p_cfg->cipher_chaining_mode)
                   {
                       case SF_CRYPTO_CIPHER_MODE_XTS:
                       break;
                       default:
                           err = SSP_ERR_INVALID_ARGUMENT;
                       break;
                   }
               break;
               default:
                   err = SSP_ERR_INVALID_ARGUMENT;
               break;
           }
        break; /** For AES keys. */
        default:
            /** The other key types are currently not supported. */
            err = SSP_ERR_UNSUPPORTED;
        break;
    }
    return err;
} /* End of function sf_crypto_cipher_validate_cfg_parameters */


/*******************************************************************************************************************//**
 * Determines the algorithm to be used for subsequent operations, based on the key type
 * configured by the user. The key type is validated and stored in the control block.
 *
 * @param[in,out]   p_ctrl  Control block for this instance of the Cipher framework.
 *
 **********************************************************************************************************************/
static void sf_crypto_cipher_determine_algorithm (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{

    /** Init the algorithm type with invalid value */
    p_ctrl->cipher_algorithm_type = CRYPTO_ALGORITHM_TYPE_MIN;

    /** Check the key type and and determine the algorithm. */
    if (sf_crypto_cipher_is_key_type_rsa(p_ctrl->key_type))
    {
        p_ctrl->cipher_algorithm_type = CRYPTO_ALGORITHM_TYPE_RSA;
    }
    else if (sf_crypto_cipher_is_key_type_aes(p_ctrl->key_type))
    {
        p_ctrl->cipher_algorithm_type = CRYPTO_ALGORITHM_TYPE_AES;
    }
    else
    {
        /*  Do nothing. Key type is already validated and algorithm is filled accordingly. */
    }
     return;
} /** End of function sf_crypto_cipher_determine_algorithm */

/*******************************************************************************************************************//**
 * Conveys if the module is in a state that allows a transition to the init state.
 *
 * @param[in,out]   p_ctrl      Control block for this instance of the Cipher framework.
 *
 * @retval          true        The state transition to init is allowed.
 * @retval          false       The module is not in a state to transition to the init state.
 **********************************************************************************************************************/
static bool sf_crypto_cipher_is_state_to_init (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    bool return_value = false;

    if ((SF_CRYPTO_CIPHER_STATE_OPENED == (sf_crypto_cipher_state_t)p_ctrl->status) ||
        (SF_CRYPTO_CIPHER_STATE_INITIALIZED == (sf_crypto_cipher_state_t)p_ctrl->status) ||
        (SF_CRYPTO_CIPHER_STATE_UPDATED == (sf_crypto_cipher_state_t)p_ctrl->status) ||
        (SF_CRYPTO_CIPHER_STATE_FINALIZED == (sf_crypto_cipher_state_t)p_ctrl->status))
    {
        return_value = true;
    }

    return return_value;
} /* End of function sf_crypto_cipher_is_state_to_init */

/*******************************************************************************************************************//**
 * Determines if the Cipher module is in a opened state.
 *
 * @param[in]       status      Cipher framework module status .
 *
 * @retval          true        Cipher framework module status reflects that it is opened.
 * @retval          false       Cipher framework module is not opened.
 **********************************************************************************************************************/
static bool sf_crypto_cipher_is_module_opened (sf_crypto_cipher_state_t status)
{
    bool opened = false;

    if (((sf_crypto_cipher_state_t)SF_CRYPTO_CIPHER_STATE_OPENED == status) ||
         (sf_crypto_cipher_state_t)(SF_CRYPTO_CIPHER_STATE_INITIALIZED == status) ||
         (sf_crypto_cipher_state_t)(SF_CRYPTO_CIPHER_STATE_UPDATED == status) ||
         (sf_crypto_cipher_state_t)(SF_CRYPTO_CIPHER_STATE_FINALIZED == status))
    {
        opened = true;
    }
    return opened;
} /* End of function sf_crypto_cipher_is_module_opened */

/*******************************************************************************************************************//**
 * Conveys if the module is in a state that allows a transition to the update state.
 *
 *  @param[in]       status      Cipher framework module status .
 *
 * @retval SSP_SUCCESS          The module is in a state to transition to the update state successfully.
 * @retval SSP_ERR_NOT_OPEN     The module has yet been opened. Call Open API first.
 * @retval SSP_ERR_INVALID_CALL Function call was made if the module state had not yet transitioned to
 *                              SF_CRYPTO_CIPHER_STATE_INITIALIZED or SF_CRYPTO_CIPHER_STATE_UPDATED.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_check_state_for_update (sf_crypto_cipher_state_t status)
{
    ssp_err_t err = SSP_ERR_INVALID_CALL;
    if (!sf_crypto_cipher_is_module_opened(status))
    {
        return SSP_ERR_NOT_OPEN;
    }
    switch (status)
    {
        case SF_CRYPTO_CIPHER_STATE_INITIALIZED:
        case SF_CRYPTO_CIPHER_STATE_UPDATED:
            err = SSP_SUCCESS;
        break;

        default:
            /*
             * SF_CRYPTO_CIPHER_STATE_OPENED and SF_CRYPTO_CIPHER_STATE_FINALIZED also fall into this path.
             */
             err = SSP_ERR_INVALID_CALL;
        break;
    }
    return err;
} /* End of function sf_crypto_cipher_is_state_to_update */

/*******************************************************************************************************************//**
 * Conveys if the module is in a state that allows a transition to the final state.
 *
 * @param[in]      status      Cipher framework module status .
 *
 * @retval SSP_SUCCESS          The module is in a state to transition to the final state successfully.
 * @retval SSP_ERR_NOT_OPEN     The module has yet been opened. Call Open API first.
 * @retval SSP_ERR_INVALID_CALL Function call was made if the module state had not yet transitioned to
 *                              SF_CRYPTO_CIPHER_STATE_INITIALIZED or SF_CRYPTO_CIPHER_STATE_UPDATED.
* **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_check_state_for_final (sf_crypto_cipher_state_t status)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_state_t local_status = status;
    /** The same check for update holds good for final too. */
    err = sf_crypto_cipher_check_state_for_update(local_status);
    return err;
} /* End of function sf_crypto_cipher_check_state_for_final */

/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto  Cipher Framework to call the appropriate update routine based on the algorithm for
 * the cipherUpdate operation.
 * This function is called by SF_CRYPTO_CIPHER_CipherUpdate().
 *
 * @param[in,out]   p_ctrl      Pointer to Crypto Cipher Framework instance control block structure.
 * @param[in]       p_data_in   Pointer to the input data buffer and the length of the input data.
 * @param[in, out]  p_data_out  Pointer to the output data buffer and the buffer length on input.
 *                              If data is output, the length of the data will be updated.
 *
 * @retval SSP_SUCCESS          The cipher update operation was successful.
 * @retval SSP_ERR_UNSUPPORTED  The module does not support the algorithm bbased on the key type specified by the user.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_process_update (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                  sf_crypto_data_handle_t const * const p_data_in,
                                                  sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_ERR_UNSUPPORTED;
    switch (p_ctrl->cipher_algorithm_type)
    {
        case CRYPTO_ALGORITHM_TYPE_AES:
            err = sf_crypto_cipher_aes_update(p_ctrl, p_data_in, p_data_out);
        break;
        case CRYPTO_ALGORITHM_TYPE_RSA:
            err = sf_crypto_cipher_rsa_update(p_ctrl, p_data_in, p_data_out);
        break;
        default:
            /* Do nothing. SSP_ERR_UNSUPPORTED will be returned automatically. */
        break;
    }
    return err;
}  /* End of function sf_crypto_cipher_process_update */

/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto  Cipher Framework to call the appropriate final routine based on the algorithm for
 * the cipherUpdate operation.
 * This function is called by SF_CRYPTO_CIPHER_CipherFinal().
 *
 * @param[in,out]   p_ctrl      Pointer to Crypto Cipher Framework instance control block structure.
 * @param[in]       p_data_in   Pointer to the input data buffer and the length of the input data.
 * @param[in, out]  p_data_out  Pointer to the output data buffer and the buffer length on input.
 *                              If data is output, the length of the data will be updated.
 *
 * @retval SSP_SUCCESS          The cipher update operation was successful.
 * @retval SSP_ERR_UNSUPPORTED  The module does not support the algorithm bbased on the key type specified by the user.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 *                              This function calls:
 *                              sf_crypto_cipher_aes_final
 *                              sf_crypto_cipher_rsa_final
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_process_final (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                 sf_crypto_data_handle_t const * const p_data_in,
                                                 sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_ERR_UNSUPPORTED;
    switch (p_ctrl->cipher_algorithm_type)
    {
        case CRYPTO_ALGORITHM_TYPE_AES:
            err = sf_crypto_cipher_aes_final(p_ctrl, p_data_in, p_data_out);
        break;
        case CRYPTO_ALGORITHM_TYPE_RSA:
            err = sf_crypto_cipher_rsa_final(p_ctrl, p_data_in, p_data_out);
        break;
        default:
            /* Do nothing as SSP_ERR_UNSUPPORTED will be returned automatically. */
        break;
    }
    return err;
} /* End of function sf_crypto_cipher_process_final */

/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto  Cipher Framework to call the appropriate init routine based on the algorithm for
 * the cipherInit operation.
 * This function is called by SF_CRYPTO_CIPHER_CipherInit().
 * @param[in,out]   p_ctrl                  Pointer to Crypto Cipher Framework instance control block structure.
 * @param[in]       cipher_operation_mode   Cipher operation mode - encryption / decryption.
 * @param[in]       p_key                   Pointer to the key that is to be used for the cipher operation.
 * @param[in]       p_algorithm_specific_params Pointer to the algorithm specific parameters.
 *
 * @retval SSP_SUCCESS                  The cipher update operation was successful.
 * @retval SSP_ERR_INVALID_ARGUMENT     At least one of the input parameters is invalid for the cipher operation.
 * @retval SSP_ERR_UNSUPPORTED          The module does not support the algorithm bbased on the key type specified by
 *                                      the user.
 * @return                              See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_process_init (sf_crypto_cipher_instance_ctrl_t * p_ctrl,
                                                sf_crypto_cipher_op_mode_t cipher_operation_mode,
                                                sf_crypto_key_t const * const p_key,
                                                sf_crypto_cipher_algorithm_init_params_t * p_algorithm_specific_params)
{
    ssp_err_t err = SSP_SUCCESS;

    switch (p_ctrl->cipher_algorithm_type)
    {
        case CRYPTO_ALGORITHM_TYPE_RSA:
            err = sf_crypto_cipher_rsa_init (p_ctrl,
                                             cipher_operation_mode,
                                             p_key,
                                             p_algorithm_specific_params);
        break;

        case CRYPTO_ALGORITHM_TYPE_AES:
            err = sf_crypto_cipher_aes_init (p_ctrl,
                                             cipher_operation_mode,
                                             p_key,
                                             p_algorithm_specific_params);
        break;

        default:
           /* The other algorithm types are currently not supported. */
           err = SSP_ERR_UNSUPPORTED;
        break;


    }
    return err;
} /* End of function sf_crypto_cipher_process_init */

/*******************************************************************************************************************//**
 * @brief   Conveys if the module is configured for GCM and is in initialized state.
 *
 * @param[in,out]   p_ctrl      Pointer to Crypto Cipher Framework instance control block structure.
 * @param[in]       status      Cipher framework module status .
 *
 * @retval SSP_SUCCESS          The module updated a message Digest successfully.
 * @retval SSP_ERR_NOT_OPEN     The module has yet been opened. Call Open API first.
 * @retval SSP_ERR_INVALID_CALL Function call was made if the module state had not yet transitioned to
 *                              SF_CRYPTO_CIPHER_STATE_INITIALIZED or SF_CRYPTO_CIPHER_STATE_UPDATED.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
**********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_check_state_for_aad_update (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                              sf_crypto_cipher_state_t status)
{
    ssp_err_t err = SSP_SUCCESS;

    if (!sf_crypto_cipher_is_module_opened(status))
    {
        return SSP_ERR_NOT_OPEN;
    }
    /** ensure that the chaining mode configured is GCM and
     * no update or Final API has been executed already that is the state can only be init.
     */
    if (SF_CRYPTO_CIPHER_MODE_GCM != p_ctrl->cipher_chaining_mode)
    {
        return SSP_ERR_INVALID_CALL;
    }

    if (SF_CRYPTO_CIPHER_STATE_INITIALIZED != status)
    {
        err = SSP_ERR_INVALID_CALL;
    }
    return err;
} /* End of function sf_crypto_cipher_check_state_for_aad_update */

/*******************************************************************************************************************//**
 * @brief   Calls the algorithm specific routines to:
 * Allocate memory for the instance and open the underlying HAL instance.
 *
 * @param   [in,out]   p_ctrl       Pointer to Crypto Cipher Framework instance control block structure.
 * @param   [in]       p_cfg        Pointer to the configuration structure for Cipher module .
 *
 * @retval  SSP_SUCCESS             The module instantiated successfully.
 * @retval  SSP_ERR_TIMEOUT         Was unable to allocate the memory within the specified time to wait.
 * @retval  SP_ERR_OUT_OF_MEMORY    Requested size is zero or larger than the pool.
 * @retval  SSP_ERR_INTERNAL        RTOS service returned a unexpected error.
 * @retval SSP_ERR_UNSUPPORTED      The module does not support the algorithm bbased on the key type specified by the user.
 * @return                          See @ref Common_Error_Codes or HAL driver for other possible
 *                                  return codes or causes. This function calls:
 *                                  sf_crypto_cipher_instance_memory_allocate
 *                                  sf_crypto_cipher_hal_open
**********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_initialize_instance (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                       sf_crypto_cipher_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_ERR_UNSUPPORTED;

    switch (p_ctrl->cipher_algorithm_type)
     {
         case CRYPTO_ALGORITHM_TYPE_RSA:
             err = sf_crypto_cipher_initialize_rsa_instance(p_ctrl, p_cfg);
         break;

         case CRYPTO_ALGORITHM_TYPE_AES:
             err = sf_crypto_cipher_initialize_aes_instance(p_ctrl, p_cfg);
         break;

         default:
             /* Do nothing. SSP_ERR_UNSUPPORTED will be returned automatically. */
         break;
     }

    return err;
}

/*******************************************************************************************************************//**
 * @brief   Calls the algorithm specific routines to:
 *  close the underlying HAL instance and release the memory for the instance.
 *
 * @param   [in,out]   p_ctrl       Pointer to Crypto Cipher Framework instance control block structure.
 *
 * @retval  SSP_SUCCESS             The module instantiated successfully.
 * @retval  SSP_ERR_TIMEOUT         Was unable to allocate the memory within the specified time to wait.
 * @retval  SP_ERR_OUT_OF_MEMORY    Requested size is zero or larger than the pool.
 * @retval  SSP_ERR_INTERNAL        RTOS service returned a unexpected error.
 * @retval SSP_ERR_UNSUPPORTED      The module does not support the algorithm based on the key type specified by the user.
 * @return                          See @ref Common_Error_Codes or HAL driver for other possible
 *                                  return codes or causes. This function calls:
 *                                  sf_crypto_cipher_instance_memory_allocate
 *                                  sf_crypto_cipher_hal_open
**********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_deinitialize_instance (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_ERR_UNSUPPORTED;

    switch (p_ctrl->cipher_algorithm_type)
    {
        case CRYPTO_ALGORITHM_TYPE_RSA:
             err = sf_crypto_cipher_deinitialize_rsa_instance(p_ctrl);
        break;

        case CRYPTO_ALGORITHM_TYPE_AES:
             err = sf_crypto_cipher_deinitialize_aes_instance(p_ctrl);
        break;

        default:
             /* Unsupported key type / algorithm type. Do nothing. SSP_ERR_UNSUPPORTED will be returned.*/
        break;
     }

    return err;
}
