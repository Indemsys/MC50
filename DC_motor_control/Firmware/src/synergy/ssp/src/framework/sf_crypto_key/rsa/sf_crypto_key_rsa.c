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
 * File Name    : sf_crypto_key_rsa.c
 * Description  : SSP Crypto Key Framework Module specific for RSA key types.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @file
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto_key.h"
#include "../../sf_crypto/sf_crypto_private_api.h"
#include "sf_crypto_key_rsa_private.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes. DO NOT USE! These functions are for internal usage.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_rsa_verify_plaintext_buffersize(sf_crypto_key_size_t key_size, uint32_t buffer_length);
ssp_err_t sf_crypto_key_rsa_verify_crtplaintext_buffersize(sf_crypto_key_size_t key_size, uint32_t buffer_length);
ssp_err_t sf_crypto_key_rsa_verify_wrapped_buffersize(sf_crypto_key_size_t key_size, uint32_t buffer_length);
static void sf_crypto_key_rsa_set_returned_key_length(sf_crypto_key_type_t key_type,
                                                      sf_crypto_key_size_t key_size,
                                                      uint32_t * p_secret_key_data_length,
                                                      uint32_t * p_public_key_data_length);
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
 * Private Functions.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @brief          Subroutine to open a Crypto RSA HAL module. This function is called by SF_CRYPTO_KEY_Open().
 *
 * @param[in,out]  p_ctrl                   Pointer to a Crypto Key Framework module control block
 * @param[in,out]  p_cfg                    Pointer to a Crypto Key Framework module configuration structure
 * @retval         SSP_SUCCESS              RSA HAL module is successfully opened.
 * @retval         SSP_ERR_OUT_OF_MEMORY    Failed to allocate memory to store RSA HAL module control block.
 * @retval         SSP_ERR_INTERNAL         RTOS service returned a unexpected error.
 * @return                                  See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_open_rsa (sf_crypto_key_instance_ctrl_t * p_ctrl, sf_crypto_key_cfg_t const * const p_cfg)
{
    ssp_err_t   err = SSP_SUCCESS;
    uint32_t    err_crypto;
    sf_crypto_instance_ctrl_t * p_ctrl_common;
    rsa_cfg_t   hal_cfg;

    /* Get a Crypto common control block and the interface. */
    p_ctrl->p_fwk_common_ctrl = (sf_crypto_instance_ctrl_t *)p_cfg->p_lower_lvl_crypto_common->p_ctrl;
    p_ctrl_common = p_ctrl->p_fwk_common_ctrl;

    /* Allocate memory for a Crypto HAL control block in the byte pool. */
    err = sf_crypto_memory_allocate(&p_ctrl_common->byte_pool, &p_ctrl->p_hal_ctrl, sizeof(rsa_ctrl_t),
                                                                                    p_ctrl_common->wait_option);
    if (SSP_SUCCESS == err)
    {
        /* Get a RSA interface instance. */
        sf_crypto_key_rsa_interface_get(p_ctrl);

        if (NULL == p_ctrl->p_hal_api)
        {
            sf_crypto_memory_release(p_ctrl->p_hal_ctrl, sizeof(rsa_ctrl_t));
            return SSP_ERR_UNSUPPORTED;
        }

        /* Set Crypto HAL API instance with the control common hardware api. */
        hal_cfg.p_crypto_api = ((crypto_instance_t *)p_ctrl_common->p_lower_lvl_crypto)->p_api;

        /* Open the Crypto HAL module. */
        err_crypto = ((rsa_api_t *)p_ctrl->p_hal_api)->open(p_ctrl->p_hal_ctrl, &hal_cfg);
        if ((uint32_t)SSP_SUCCESS != err_crypto)
        {
            err = (ssp_err_t)err_crypto;
        }
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to close a Crypto RSA HAL module. This function is called by SF_CRYPTO_KEY_Close().
 *
 * @param[in]  p_ctrl                   Pointer to a Crypto Key Framework module control block
 * @retval     SSP_SUCCESS              RSA HAL module is successfully closed.
 * @return                              See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_close_rsa (sf_crypto_key_instance_ctrl_t * p_ctrl)
{
    void * p_tmp = p_ctrl->p_hal_ctrl;

    /* Close the Crypto HAL driver. */
    ssp_err_t   err = (ssp_err_t) ((rsa_api_t *)p_ctrl->p_hal_api)->close(p_ctrl->p_hal_ctrl);
    if ((uint32_t)SSP_SUCCESS == err)
    {
        /* Release the control block memory back to byte pool */
        err = sf_crypto_memory_release(p_tmp, sizeof(rsa_ctrl_t));
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to generate a RSA key. This function is called by SF_CRYPTO_KEY_Generate().
 *
 * @param[in]  p_ctrl               Pointer to a Crypto Key Framework module control block
 * @param[out] p_secret_key         Pointer to a secret key
 * @param[out] p_public_key         Pointer to a public key
 * @retval     SSP_SUCCESS          The module created a key successfully.
 * @retval     SSP_ERR_INVALID_SIZE Failed, as the allocated key buffer length is not sufficient for the
 *                                  RSA Key generation operation.
 * @retval     SSP_ERR_UNSUPPORTED  Procedure is not supported for the supplied parameters.
 * @return                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_generate_rsa (sf_crypto_key_instance_ctrl_t * p_ctrl,
                                      sf_crypto_key_t * const p_secret_key, sf_crypto_key_t * const p_public_key)
{
    uint32_t    err_crypto;
    rsa_key_t   secret_key;
    rsa_key_t   public_key;

    if (SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT == p_ctrl->key_type)
    {
        secret_key.key_format   = RSA_KEY_FORMAT_PLAIN_TEXT_PRIVATE_KEY;
    }
    else if (SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT == p_ctrl->key_type)
    {
        secret_key.key_format   = RSA_KEY_FORMAT_PLAIN_TEXT_CRT_KEY;
    }
    else
    {
        secret_key.key_format   = RSA_KEY_FORMAT_WRAPPED_PRIVATE_KEY;
    }

    secret_key.length       = p_secret_key->data_length;
    secret_key.p_data       = p_secret_key->p_data;

    public_key.key_format   = RSA_KEY_FORMAT_PLAIN_TEXT_PUBLIC_KEY;
    public_key.length       = p_public_key->data_length;
    public_key.p_data       = p_public_key->p_data;

    /* Verify the Buffer size that holds the generated secret key */
    err_crypto = (uint32_t)sf_crypto_key_rsa_verify_privatekey_buffersize(p_ctrl->key_type, p_ctrl->key_size,
                                                                           p_secret_key->data_length);
    if (SSP_SUCCESS != err_crypto)
    {
    	return (ssp_err_t)err_crypto;
    }

    /* Verify the Buffer size that holds the generated public key */
    err_crypto = (uint32_t)sf_crypto_key_rsa_verify_publickey_buffersize(p_ctrl->key_type, p_ctrl->key_size,
                                                                         p_public_key->data_length);
    if (SSP_SUCCESS != err_crypto)
    {
    	return (ssp_err_t)err_crypto;
    }

    /* Call Crypto HAL driver. */
    err_crypto = ((rsa_api_t *)p_ctrl->p_hal_api)->keyCreate(p_ctrl->p_hal_ctrl, &secret_key, &public_key);
    if (SSP_SUCCESS == (ssp_err_t)err_crypto)
    {
        sf_crypto_key_rsa_set_returned_key_length(p_ctrl->key_type,
                                                  p_ctrl->key_size,
                                                  &p_secret_key->data_length,
                                                  &p_public_key->data_length);

    }
    return (ssp_err_t)err_crypto;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to get a RSA HAL API instance. This function is called by sf_crypto_key_open_rsa().
 *
 * @param[in,out]  p_ctrl        Pointer to a Key framework control block, whose p_hal_api filled with HAL RSA
 *                               interface. This indicates NULL, for not supported MCUs
 **********************************************************************************************************************/
void sf_crypto_key_rsa_interface_get (sf_crypto_key_instance_ctrl_t * const p_ctrl)
{
    sf_crypto_instance_ctrl_t * p_lower_level_crypto_ctrl;
    crypto_instance_t         * p_crypto;
    crypto_interface_get_param_t param;

    /* Get a Crypto common control block and the HAL instance. */
    p_lower_level_crypto_ctrl = (sf_crypto_instance_ctrl_t *)(p_ctrl->p_fwk_common_ctrl);
    p_crypto = (crypto_instance_t *)(p_lower_level_crypto_ctrl->p_lower_lvl_crypto);

    /* Check the RSA key type. */
    switch(p_ctrl->key_type)
    {
        case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
            param.key_type = CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT;
            break;
        case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
            param.key_type = CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT;
            break;
        case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
            param.key_type = CRYPTO_KEY_TYPE_RSA_WRAPPED;
            break;
        default:
            param.key_type = CRYPTO_KEY_TYPE_MAX;
            break;
    }

    /* Check the RSA key type. */
    switch(p_ctrl->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_RSA_1024:
            param.key_size = CRYPTO_KEY_SIZE_RSA_1024;
            break;
        case SF_CRYPTO_KEY_SIZE_RSA_2048:
            param.key_size = CRYPTO_KEY_SIZE_RSA_2048;
            break;
        default:
            param.key_size = CRYPTO_KEY_SIZE_MAX;
            break;
    }

    /* Get the HAL API instance for a selected algorithm type. */
    param.algorithm_type = CRYPTO_ALGORITHM_TYPE_RSA;
    p_crypto->p_api->interfaceGet(&param, &p_ctrl->p_hal_api);

}

/*******************************************************************************************************************//**
 * @brief      Subroutine to verify that the provided `buffer_length` (buffer to hold the secret RSA private key)
 * is sufficient or not, before proceeding with key Generate function.
 * This function is called by RSA Key Generate function before passing request to Crypto HAL Key generate API call.
 *
 * @param[in]  key_type                 Indicates RSA key type - Plain text/CRT Plain Text/ Wrapped key.
 * @param[in]  key_size                 Indicates RSA key size - 1024/2048-bits.
 * @param[in]  buffer_length            Length of the secret key which shall be filled by RSA Key generation algorithm.
 * @retval     SSP_SUCCESS              Key length is successful, and proceed with RSA Key Generation.
 * @retval     SSP_ERR_INVALID_SIZE     Failed, as the allocated key length is not sufficient for the Key generation
 *                                      operation of RSA.
 * @note       This function is not a user API but an internal function for SF_CRYPTO_KEY to verify whether passed buffer
 *             size is sufficient for Key to hold in Key Generate API call.
 *
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_rsa_verify_privatekey_buffersize (sf_crypto_key_type_t key_type, sf_crypto_key_size_t key_size,
                                                          uint32_t buffer_length)
{
    ssp_err_t    err_crypto = SSP_SUCCESS;

    if (SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT == key_type)
    {
        err_crypto = sf_crypto_key_rsa_verify_plaintext_buffersize(key_size, buffer_length);
    }
    else if (SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT == key_type)
    {
        err_crypto = sf_crypto_key_rsa_verify_crtplaintext_buffersize(key_size, buffer_length);
    }
    else if (SF_CRYPTO_KEY_TYPE_RSA_WRAPPED == key_type)
    {
        err_crypto = sf_crypto_key_rsa_verify_wrapped_buffersize(key_size, buffer_length);
    }
    else
    {
        /* Do Nothing */
    }

    return err_crypto;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to verify that the provided `buffer_length` (buffer to hold the secret RSA Public key)
 * is sufficient or not, before proceeding with key Generate function.
 * This function is called by RSA Key Generate function before passing request to Crypto HAL Key generate API call.
 *
 * @param[in]  key_type                 Indicates RSA key type - Plain text/CRT Plain Text/ Wrapped key.
 * @param[in]  key_size                 Indicates RSA key size - 1024/2048-bits.
 * @param[in]  buffer_length            Length of the secret key which shall be filled by RSA Key generation algorithm.
 * @retval     SSP_SUCCESS              Key length is successful, and proceed with RSA Key Generation.
 * @retval     SSP_ERR_INVALID_SIZE     Failed, as the allocated key length is not sufficient for the Key generation
 *                                      operation of RSA.
 * @note       This function is not a user API but an internal function for SF_CRYPTO_KEY to verify whether passed buffer
 *             size is sufficient for Key to hold in Key Generate API call.
 *
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_rsa_verify_publickey_buffersize (sf_crypto_key_type_t key_type, sf_crypto_key_size_t key_size,
                                                         uint32_t buffer_length)
{
    ssp_err_t    err_crypto = SSP_SUCCESS;

    if (SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT == key_type)
    {
        if (((SF_CRYPTO_KEY_SIZE_RSA_1024 == key_size) && (buffer_length < (uint32_t)RSA_PLAIN_TEXT_PUBLIC_KEY_SIZE_BYTES(1024U))) ||
            ((SF_CRYPTO_KEY_SIZE_RSA_2048 == key_size) && (buffer_length < (uint32_t)RSA_PLAIN_TEXT_PUBLIC_KEY_SIZE_BYTES(2048U))))
        {
            err_crypto =  SSP_ERR_INVALID_SIZE;
        }
        else
        {
            /* Do Nothing */
        }
    }
    return err_crypto;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to verify that the provided `buffer_length` (buffer to hold the secret RSA Plain text key)
 * is sufficient or not, before proceeding with key Generate function.
 * This function is called by `sf_crypto_key_rsa_verify_privatekey_buffersize` as an internal function call.
 *
 * @param[in]  key_size                 Indicates RSA key size - 1024/2048-bits.
 * @param[in]  buffer_length            Length of the secret key which shall be filled by RSA Key generation algorithm.
 * @retval     SSP_SUCCESS              Key length is successful, and proceed with RSA Key Generation.
 * @retval     SSP_ERR_INVALID_SIZE     Failed, as the allocated key length is not sufficient for the Key generation
 *                                      operation of RSA.
 * @note       This function is not a user API but an internal function for SF_CRYPTO_KEY to verify whether passed buffer
 *             size is sufficient for Key to hold in Key Generate API call.
 *
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_rsa_verify_plaintext_buffersize (sf_crypto_key_size_t key_size, uint32_t buffer_length)
{
    ssp_err_t    err_crypto = SSP_SUCCESS;

    if (((SF_CRYPTO_KEY_SIZE_RSA_1024 == key_size) && (buffer_length < (uint32_t)RSA_PLAIN_TEXT_PRIVATE_KEY_SIZE_BYTES(1024U))) ||
        ((SF_CRYPTO_KEY_SIZE_RSA_2048 == key_size) && (buffer_length < (uint32_t)RSA_PLAIN_TEXT_PRIVATE_KEY_SIZE_BYTES(2048U))))
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
 * @brief      Subroutine to verify that the provided `buffer_length` (buffer to hold the secret RSA CRT Plain text key)
 * is sufficient or not, before proceeding with key Generate function.
 * This function is called by `sf_crypto_key_rsa_verify_privatekey_buffersize` as an internal function call.
 *
 * @param[in]  key_size                 Indicates RSA key size - 1024/2048-bits.
 * @param[in]  buffer_length            Length of the secret key which shall be filled by RSA Key generation algorithm.
 * @retval     SSP_SUCCESS              Key length is successful, and proceed with RSA Key Generation.
 * @retval     SSP_ERR_INVALID_SIZE     Failed, as the allocated key length is not sufficient for the Key generation
 *                                      operation of RSA.
 * @note       This function is not a user API but an internal function for SF_CRYPTO_KEY to verify whether passed buffer
 *             size is sufficient for Key to hold in Key Generate API call.
 *
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_rsa_verify_crtplaintext_buffersize (sf_crypto_key_size_t key_size, uint32_t buffer_length)
{
    ssp_err_t    err_crypto = SSP_SUCCESS;

    if (((SF_CRYPTO_KEY_SIZE_RSA_1024 == key_size) && (buffer_length < (uint32_t)RSA_PLAIN_TEXT_CRT_KEY_SIZE_BYTES(1024U))) ||
        ((SF_CRYPTO_KEY_SIZE_RSA_2048 == key_size) && (buffer_length < (uint32_t)RSA_PLAIN_TEXT_CRT_KEY_SIZE_BYTES(2048U))))
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
 * @brief      Subroutine to verify that the provided `buffer_length` (buffer to hold the secret RSA Wrapped key) is
 * sufficient or not, before proceeding with key Generate function.
 * This function is called by `sf_crypto_key_rsa_verify_privatekey_buffersize` as an internal function call.
 *
 * @param[in]  key_size                 Indicates RSA key size - 1024/2048-bits.
 * @param[in]  buffer_length            Length of the secret key which shall be filled by RSA Key generation algorithm.
 * @retval     SSP_SUCCESS              Key length is successful, and proceed with RSA Key Generation.
 * @retval     SSP_ERR_INVALID_SIZE     Failed, as the allocated key length is not sufficient for the Key generation
 *                                      operation of RSA.
 * @note       This function is not a user API but an internal function for SF_CRYPTO_KEY to verify whether passed buffer
 *             size is sufficient for Key to hold in Key Generate API call.
 *
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_rsa_verify_wrapped_buffersize (sf_crypto_key_size_t key_size, uint32_t buffer_length)
{
    ssp_err_t    err_crypto = SSP_SUCCESS;

    if (((SF_CRYPTO_KEY_SIZE_RSA_1024 == key_size) && (buffer_length < (uint32_t)RSA_WRAPPPED_PRIVATE_KEY_SIZE_BYTES(1024U))) ||
        ((SF_CRYPTO_KEY_SIZE_RSA_2048 == key_size) && (buffer_length < (uint32_t)RSA_WRAPPPED_PRIVATE_KEY_SIZE_BYTES(2048U))))
    {
        err_crypto =  SSP_ERR_INVALID_SIZE;
    }
    else
    {
        /* Do Nothing */
    }
    return err_crypto;
}

/*******************************************************************************************************************//**
 * @brief Private helper routine to set data_length element of public and secret key data handles.
 * This function is called by `sf_crypto_key_generate_rsa` as an internal function call.
 *
 * @param[in]   key_type                   Indicates RSA key type.
 * @param[in]   key_size                   Indicates RSA key size - 1024/2048-bits.
 * @param[out]  p_secret_key_data_length   Pointer to secret_key data handle's data_length element.
 * @param[out]  p_public_key_data_length   Pointer to public_key data handle's data_length element.
 *
 **********************************************************************************************************************/
static void sf_crypto_key_rsa_set_returned_key_length (sf_crypto_key_type_t key_type,
                                                       sf_crypto_key_size_t key_size,
                                                       uint32_t * p_secret_key_data_length,
                                                       uint32_t * p_public_key_data_length)
{
    switch (key_type)
    {
        case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
        {
            if(SF_CRYPTO_KEY_SIZE_RSA_1024 == key_size)
            {
                *p_secret_key_data_length = RSA_PLAIN_TEXT_PRIVATE_KEY_SIZE_BYTES(1024U);
                *p_public_key_data_length = RSA_PLAIN_TEXT_PUBLIC_KEY_SIZE_BYTES(1024U);
                break;
            }
            if(SF_CRYPTO_KEY_SIZE_RSA_2048 == key_size)
            {
                *p_secret_key_data_length = RSA_PLAIN_TEXT_PRIVATE_KEY_SIZE_BYTES(2048U);
                *p_public_key_data_length = RSA_PLAIN_TEXT_PUBLIC_KEY_SIZE_BYTES(2048U);
                break;
            }
            break;
        }
        case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
        {
            if(SF_CRYPTO_KEY_SIZE_RSA_1024 == key_size)
            {
                *p_secret_key_data_length = RSA_PLAIN_TEXT_CRT_KEY_SIZE_BYTES(1024U);
                *p_public_key_data_length = RSA_PLAIN_TEXT_PUBLIC_KEY_SIZE_BYTES(1024U);
                break;
            }
            if(SF_CRYPTO_KEY_SIZE_RSA_2048 == key_size)
            {
                *p_secret_key_data_length = RSA_PLAIN_TEXT_CRT_KEY_SIZE_BYTES(2048U);
                *p_public_key_data_length = RSA_PLAIN_TEXT_PUBLIC_KEY_SIZE_BYTES(2048U);
                break;
            }
            break;
        }
        case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
        {
            if(SF_CRYPTO_KEY_SIZE_RSA_1024 == key_size)
            {
                *p_secret_key_data_length = RSA_WRAPPPED_PRIVATE_KEY_SIZE_BYTES(1024U);
                *p_public_key_data_length = RSA_PLAIN_TEXT_PUBLIC_KEY_SIZE_BYTES(1024U);
                break;
            }
            if(SF_CRYPTO_KEY_SIZE_RSA_2048 == key_size)
            {
                *p_secret_key_data_length = RSA_WRAPPPED_PRIVATE_KEY_SIZE_BYTES(2048U);
                *p_public_key_data_length = RSA_PLAIN_TEXT_PUBLIC_KEY_SIZE_BYTES(2048U);
                break;
            }
            break;
        }
        default:
        {
            /* Validated params. Must never land here. */
        break;
        }
    }
}

/*******************************************************************************************************************//**
 * @} (end defgroup SF_CRYPTO_KEY)
 **********************************************************************************************************************/
