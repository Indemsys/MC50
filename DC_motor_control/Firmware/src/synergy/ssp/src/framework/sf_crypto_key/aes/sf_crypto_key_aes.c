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
 * File Name    : sf_crypto_key_aes.c
 * Description  : SSP Crypto Key Framework Module specific for AES keys.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @file
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto_key.h"
#include "sf_crypto_key_aes_private.h"
#include "../../sf_crypto/sf_crypto_private_api.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes. DO NOT USE! These functions are for internal usage.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_aes_verify_wrapped_xtskey_buffersize(sf_crypto_key_size_t key_size, uint32_t buffer_length);
static void sf_crypto_key_aes_set_returned_key_length(sf_crypto_key_type_t key_type,
                                                      sf_crypto_key_size_t key_size,
                                                      uint32_t * p_secret_key_data_length);
/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
* @ingroup SF_Library
* @addtogroup SF_CRYPTO_KEY
* @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
Functions
***********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @brief          Subroutine to open a Crypto AES HAL module. This function is called by SF_CRYPTO_KEY_Open().
 *
 * @param[in,out]  p_ctrl                   Pointer to a Crypto Key Framework module control block
 * @param[in]      p_cfg                    Pointer to a Crypto Key Framework module configuration structure
 * @retval         SSP_SUCCESS              AES HAL module is successfully opened.
 * @retval         SSP_ERR_OUT_OF_MEMORY    Failed to allocate memory to store AES HAL module control block.
 * @retval         SSP_ERR_INTERNAL         RTOS service returned a unexpected error.
 * @return                                  See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_open_aes (sf_crypto_key_instance_ctrl_t * p_ctrl, sf_crypto_key_cfg_t const * const p_cfg)
{
    ssp_err_t   err = SSP_SUCCESS;
    uint32_t    err_crypto;
    sf_crypto_instance_ctrl_t * p_ctrl_common;
    aes_cfg_t   hal_cfg;

    /* Get a Crypto common control block and the interface. */
    p_ctrl->p_fwk_common_ctrl = (sf_crypto_instance_ctrl_t *)p_cfg->p_lower_lvl_crypto_common->p_ctrl;
    p_ctrl_common = p_ctrl->p_fwk_common_ctrl;

    /* Allocate memory for a Crypto HAL control block in the byte pool. */
    err = (ssp_err_t) sf_crypto_memory_allocate(&p_ctrl_common->byte_pool, &p_ctrl->p_hal_ctrl, sizeof(aes_ctrl_t),
                                                 p_ctrl_common->wait_option);
    if (SSP_SUCCESS == err)
    {
        /* Get a AES interface instance. */
        sf_crypto_key_aes_interface_get(p_ctrl);

        if (NULL == p_ctrl->p_hal_api)
        {
            sf_crypto_memory_release(p_ctrl->p_hal_ctrl, sizeof(aes_ctrl_t));
            return SSP_ERR_UNSUPPORTED;
        }

        /* Set Crypto HAL API instance with the control common hardware api. */
        hal_cfg.p_crypto_api = ((crypto_instance_t *)p_ctrl_common->p_lower_lvl_crypto)->p_api;

        /* Open the Crypto HAL module. */
        err_crypto = ((aes_api_t *)p_ctrl->p_hal_api)->open(p_ctrl->p_hal_ctrl, &hal_cfg);
        if ((uint32_t)SSP_SUCCESS != err_crypto)
        {
            err = (ssp_err_t)err_crypto;
        }
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to close a Crypto AES HAL module. This function is called by SF_CRYPTO_KEY_Close().
 *
 * @param[in]  p_ctrl                   Pointer to a Crypto Key Framework module control block
 * @retval     SSP_SUCCESS              AES HAL module is successfully closed.
 * @return                              See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_close_aes (sf_crypto_key_instance_ctrl_t * p_ctrl)
{
    void * p_tmp = p_ctrl->p_hal_ctrl;
    /* Close the Crypto HAL driver. */
    ssp_err_t   err = (ssp_err_t) ((aes_api_t *)p_ctrl->p_hal_api)->close(p_ctrl->p_hal_ctrl);
    if ((uint32_t)SSP_SUCCESS == err)
    {
        /* Release the control block memory back to byte pool */
        err = sf_crypto_memory_release(p_tmp, sizeof(aes_ctrl_t));
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to generate a AES key. This function is called by SF_CRYPTO_KEY_Generate().
 *
 * @param[in]  p_ctrl                Pointer to a Crypto Key Framework module control block.
 * @param[out] p_secret_key          Pointer to a secret key.
 * @retval     SSP_SUCCESS           The module created the AES Secret key successfully.
 * @retval     SSP_ERR_INVALID_SIZE  Failed, as the allocated key buffer length is not sufficient for the
 *                                   AES Key generation operation.
 * @retval     SSP_ERR_UNSUPPORTED   Procedure is not supported for the supplied parameters.
 * @return                           See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_generate_aes (sf_crypto_key_instance_ctrl_t * p_ctrl,
                                      sf_crypto_key_t * const p_secret_key)
{
    ssp_err_t    err_crypto;

    /* Verify the size of secret key before passing to Key operations */
    err_crypto = sf_crypto_key_aes_verify_wrappedkey_buffersize(p_ctrl->key_size, p_secret_key->data_length);
    if ((uint32_t)SSP_SUCCESS != err_crypto)
    {
    	return (ssp_err_t)err_crypto;
    }

    /* Call Crypto HAL driver. */
    err_crypto = (ssp_err_t) ((aes_api_t *)p_ctrl->p_hal_api)->createKey(p_ctrl->p_hal_ctrl,
                                                                         p_secret_key->data_length/4,
                                                                         (uint32_t *) p_secret_key->p_data);
    if( SSP_SUCCESS == (ssp_err_t)err_crypto)
    {
        sf_crypto_key_aes_set_returned_key_length(p_ctrl->key_type,
                                                  p_ctrl->key_size,
                                                  &p_secret_key->data_length);
    }

    return err_crypto;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to get a AES HAL API instance. This function is called by sf_crypto_key_open_aes().
 *             Key framework control block's p_hal_api field is filled with HAL AES interface.
 *             This will be set to NULL if the API interface is not found on the particular MCU.
 *
 * @param[in,out]  p_ctrl        Pointer to a Key framework control block.
 *                               interface. This indicates NULL, for not supported MCUs
 **********************************************************************************************************************/
void sf_crypto_key_aes_interface_get (sf_crypto_key_instance_ctrl_t * const p_ctrl)
{
    sf_crypto_instance_ctrl_t * p_lower_level_crypto_ctrl;
    crypto_instance_t         * p_crypto;
    crypto_interface_get_param_t param;

    /* Get a Crypto common control block and the HAL instance. */
    p_lower_level_crypto_ctrl = (sf_crypto_instance_ctrl_t *)(p_ctrl->p_fwk_common_ctrl);
    p_crypto = (crypto_instance_t *)(p_lower_level_crypto_ctrl->p_lower_lvl_crypto);
    /* Get the HAL API instance for a selected algorithm type. */
    param.algorithm_type = CRYPTO_ALGORITHM_TYPE_AES;
    /* Check the AES key type and size and get an appropriate API instance. */
    if (SF_CRYPTO_KEY_TYPE_AES_WRAPPED == p_ctrl->key_type)
    {
        switch (p_ctrl->key_size)
        {
        case SF_CRYPTO_KEY_SIZE_AES_128:
            param.key_type = CRYPTO_KEY_TYPE_AES_WRAPPED;
            param.key_size = CRYPTO_KEY_SIZE_AES_128;
            sf_crypto_key_aes_get_available_api_interface(p_ctrl, &param);
            break;
        case SF_CRYPTO_KEY_SIZE_AES_XTS_128:
            param.key_type = CRYPTO_KEY_TYPE_AES_WRAPPED;
            param.key_size = CRYPTO_KEY_SIZE_AES_128;
            param.chaining_mode = CRYPTO_XTS_MODE;
            p_crypto->p_api->interfaceGet(&param, &p_ctrl->p_hal_api);
            break;
        case SF_CRYPTO_KEY_SIZE_AES_192:
            param.key_type = CRYPTO_KEY_TYPE_AES_WRAPPED;
            param.key_size = CRYPTO_KEY_SIZE_AES_192;
            sf_crypto_key_aes_get_available_api_interface(p_ctrl, &param);
            break;
        case SF_CRYPTO_KEY_SIZE_AES_256:
            param.key_type = CRYPTO_KEY_TYPE_AES_WRAPPED;
            param.key_size = CRYPTO_KEY_SIZE_AES_256;
            sf_crypto_key_aes_get_available_api_interface(p_ctrl, &param);
            break;
        case SF_CRYPTO_KEY_SIZE_AES_XTS_256:
            param.key_type = CRYPTO_KEY_TYPE_AES_WRAPPED;
            param.key_size = CRYPTO_KEY_SIZE_AES_256;
            param.chaining_mode = CRYPTO_XTS_MODE;
            p_crypto->p_api->interfaceGet(&param, &p_ctrl->p_hal_api);
            break;
        default:
            param.key_type = CRYPTO_KEY_TYPE_MAX;
            param.key_size = CRYPTO_KEY_SIZE_MAX;
            param.chaining_mode = CRYPTO_CHAINING_MODE_MAX;
            p_crypto->p_api->interfaceGet(&param, &p_ctrl->p_hal_api);
            break;
        }
    }
    else
    {
        param.key_type = CRYPTO_KEY_TYPE_MAX;
        param.key_size = CRYPTO_KEY_SIZE_MAX;
        param.chaining_mode = CRYPTO_CHAINING_MODE_MAX;
        p_crypto->p_api->interfaceGet(&param, &p_ctrl->p_hal_api);
    }
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to get first available AES HAL API instance for ECB/CBC/CTR/GCM chaining modes.
 *             This function is called by sf_crypto_key_aes_interface_get(). Key framework control block's
 *             p_hal_api field is filled with HAL AES interface. This will be set to NULL if the API interface
 *             is not found on the particular MCU.
 *
 * @param[in,out]  p_ctrl        Pointer to a Key framework control block.
 * @param[in]      p_param       Interface get param. Algorithm type, key type and key size is set by the caller.
 **********************************************************************************************************************/
void sf_crypto_key_aes_get_available_api_interface (sf_crypto_key_instance_ctrl_t * const p_ctrl,
                                                    crypto_interface_get_param_t * p_param)
{
    sf_crypto_instance_ctrl_t * p_lower_level_crypto_ctrl;
    crypto_instance_t         * p_crypto;
    uint32_t                  ret = (uint32_t)SSP_ERR_INVALID_ARGUMENT;

    /* Get a Crypto common control block and the HAL instance. */
    p_lower_level_crypto_ctrl = (sf_crypto_instance_ctrl_t *)(p_ctrl->p_fwk_common_ctrl);
    p_crypto = (crypto_instance_t *)(p_lower_level_crypto_ctrl->p_lower_lvl_crypto);
    /* Check available interface */
    p_param->chaining_mode = CRYPTO_ECB_MODE;
    ret = p_crypto->p_api->interfaceGet(p_param, &p_ctrl->p_hal_api);
    if ((uint32_t)SSP_SUCCESS != ret)
    {
    	/* Interface with ECB is unavailable, try CBC chaining mode. */
        p_param->chaining_mode = CRYPTO_CBC_MODE;
        ret = p_crypto->p_api->interfaceGet(p_param, &p_ctrl->p_hal_api);
    }
    else
    {
        return; /* Interface with ECB chaining mode is available. */
    }

    if ((uint32_t)SSP_SUCCESS != ret)
    {
    	/* Interface with ECB and CBC is unavailable, try CTR chaining mode. */
        p_param->chaining_mode = CRYPTO_CTR_MODE;
        ret = p_crypto->p_api->interfaceGet(p_param, &p_ctrl->p_hal_api);
    }
    else
    {
        return; /* Interface with CBC chaining mode is available. */
    }

    if ((uint32_t)SSP_SUCCESS != ret)
    {
    	/* Interface with ECB, CBC and CTR is unavailable, try GCM chaining mode. */
        p_param->chaining_mode = CRYPTO_GCM_MODE;
        p_crypto->p_api->interfaceGet(p_param, &p_ctrl->p_hal_api); /* No need to capture return code here. */
    }
    /* Interface with CTR or GCM chaining mode is available Or None of the above interfaces are available. */
    return;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to verify that the provided `buffer_length` (indicates size of the secret AES Wrapped key) is
 * sufficient or not, before proceeding with key Generate function.
 * This function is called by AES Key Generate function before passing request to Crypto HAL Key generate API call.
 *
 * @param[in]  key_size                 Indicates AES key sizes - 128/192/256-bits, supported chaining modes - CBC, ECB,
 *                                      GCM, CTR
 * @param[in]  buffer_length            Length of the secret key which shall be filled by AES
 *                                      Key generation algorithm.
 * @retval     SSP_SUCCESS              Key length is successful, and proceed with AES Key Generation.
 * @retval     SSP_ERR_INVALID_SIZE     Failed, as the allocated key length is not sufficient for the Key generation
 *                                      operation of AES
 * @note       This function is not a user API but an internal function for SF_CRYPTO_KEY to verify whether passed buffer
 *             size is sufficient for Key to hold in Key Generate API call.
 *
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_aes_verify_wrappedkey_buffersize (sf_crypto_key_size_t key_size, uint32_t buffer_length)
{
    ssp_err_t    err_crypto = SSP_SUCCESS;

    if ((SF_CRYPTO_KEY_SIZE_AES_XTS_128 == key_size) || (SF_CRYPTO_KEY_SIZE_AES_XTS_256 == key_size))
    {
        err_crypto = sf_crypto_key_aes_verify_wrapped_xtskey_buffersize(key_size, buffer_length);
    }
    else if (((SF_CRYPTO_KEY_SIZE_AES_128 == key_size) && (buffer_length < (uint32_t)AES128_WRAPPPED_SECRET_KEY_SIZE_BYTES)) ||
             ((SF_CRYPTO_KEY_SIZE_AES_192 == key_size) && (buffer_length < (uint32_t)AES192_WRAPPPED_SECRET_KEY_SIZE_BYTES)) ||
             ((SF_CRYPTO_KEY_SIZE_AES_256 == key_size) && (buffer_length < (uint32_t)AES256_WRAPPPED_SECRET_KEY_SIZE_BYTES)))
    {
        err_crypto = SSP_ERR_INVALID_SIZE;
    }
    else
    {
        /* Do Nothing */
    }

    return err_crypto;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to verify that the provided `num_words` (indicates size of the secret AES Wrapped key for XTS
 * AES chaining mode) is sufficient or not, before proceeding with key operations.
 * This function is called by `sf_crypto_key_aes_verify_wrappedkey_buffersize` as an internal function call.
 *
 * @param[in]  key_size                 Indicates AES key sizes - 128/256-bits, supported chaining mode - XTS
 * @param[in]  buffer_length            Length of the secret key which shall be filled by AES
 *                                      Key generation algorithm.
 * @retval     SSP_SUCCESS              Key length is successful, and proceed with AES Key Generation.
 * @retval     SSP_ERR_INVALID_SIZE     Failed, as the allocated key length is not sufficient for the Key generation
 *                                      operation of AES
 * @note       This function is not a user API but an internal function for SF_CRYPTO_KEY to verify whether passed buffer
 *             size is sufficient for Key to hold in Key Generate API call.
 *
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_aes_verify_wrapped_xtskey_buffersize (sf_crypto_key_size_t key_size, uint32_t buffer_length)
{
    ssp_err_t    err_crypto = SSP_SUCCESS;

    if (((SF_CRYPTO_KEY_SIZE_AES_XTS_128 == key_size) && (buffer_length < (uint32_t)AES_XTS_128_WRAPPPED_SECRET_KEY_SIZE_BYTES)) ||
        ((SF_CRYPTO_KEY_SIZE_AES_XTS_256 == key_size) && (buffer_length < (uint32_t)AES_XTS_256_WRAPPPED_SECRET_KEY_SIZE_BYTES)))
    {
        err_crypto = SSP_ERR_INVALID_SIZE;
    }
    else
    {
        /* Do Nothing */
    }
    return err_crypto;
}

/*******************************************************************************************************************//**
 * @brief Private helper routine to set data_length element in the secret key data handle.
 * This function is called by `sf_crypto_key_generate_aes` as an internal function call.
 *
 * @param[in]   key_size                   Indicates AES key size.
 * @param[in]   key_type                   Indicates AES key type.
 * @param[out]  p_secret_key_data_length   Pointer to secret_key data handle's data_length element.
 *
 **********************************************************************************************************************/
static void sf_crypto_key_aes_set_returned_key_length (sf_crypto_key_type_t key_type,
                                                       sf_crypto_key_size_t key_size,
                                                       uint32_t * p_secret_key_data_length)
{
    if (SF_CRYPTO_KEY_TYPE_AES_WRAPPED == key_type)
    {
        switch (key_size)
        {
            case SF_CRYPTO_KEY_SIZE_AES_128:
            {
                *p_secret_key_data_length = (uint32_t)AES128_WRAPPPED_SECRET_KEY_SIZE_BYTES;
            break;
            }
            case SF_CRYPTO_KEY_SIZE_AES_XTS_128:
            {
                *p_secret_key_data_length = (uint32_t)AES_XTS_128_WRAPPPED_SECRET_KEY_SIZE_BYTES;
            break;
            }
            case SF_CRYPTO_KEY_SIZE_AES_192:
            {
                *p_secret_key_data_length = (uint32_t)AES192_WRAPPPED_SECRET_KEY_SIZE_BYTES;
            break;
            }
            case SF_CRYPTO_KEY_SIZE_AES_256:
            {
                *p_secret_key_data_length = (uint32_t)AES256_WRAPPPED_SECRET_KEY_SIZE_BYTES;
            break;
            }
            case SF_CRYPTO_KEY_SIZE_AES_XTS_256:
            {
                *p_secret_key_data_length = (uint32_t)AES_XTS_256_WRAPPPED_SECRET_KEY_SIZE_BYTES;
            break;
            }
            default:
            {
                /* Validated input params. Will never land here. */
            break;
            }
        }
    }
}
/*******************************************************************************************************************//**
 * @} (end defgroup SF_CRYPTO_KEY)
 **********************************************************************************************************************/
