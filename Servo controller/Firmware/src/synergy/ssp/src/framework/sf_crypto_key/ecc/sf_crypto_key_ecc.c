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
 * File Name    : sf_crypto_key_ecc.c
 * Description  : SSP Crypto Key Framework Module specific for ECC key types.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @file
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto_key.h"
#include "../../sf_crypto/sf_crypto_private_api.h"
#include "sf_crypto_key_ecc_private.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define BYTES_IN_WORD   (4U)
/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes. DO NOT USE! These functions are for internal usage.
 **********************************************************************************************************************/
static void sf_crypto_key_ecc_set_returned_key_length(sf_crypto_key_type_t key_type,
                                                      sf_crypto_key_size_t key_size,
                                                      uint32_t * p_secret_key_data_length,
                                                      uint32_t * p_public_key_data_length);

static ssp_err_t sf_crypto_key_ecc_verify_output_wrapped_key_buffer_size(sf_crypto_key_size_t key_size,
                                                                        uint32_t secret_key_buffer_length);
static ssp_err_t sf_crypto_key_ecc_verify_output_plain_key_buffer_size(sf_crypto_key_size_t key_size,
                                                                       uint32_t secret_key_buffer_length);
static ssp_err_t  sf_crypto_key_ecc_verify_key_size (sf_crypto_key_size_t key_size);
static ssp_err_t sf_crypto_key_ecc_interface_get (sf_crypto_key_instance_ctrl_t * const p_ctrl);

static ssp_err_t sf_crypto_key_ecc_verify_input_domain_buffer_size(sf_crypto_key_size_t key_size,
                                                                   uint32_t domain_params_buffer_length);
static ssp_err_t sf_crypto_key_ecc_verify_input_generator_point_buffer_size(sf_crypto_key_size_t key_size,
                                                                            uint32_t generator_point_buffer_length);
static ssp_err_t sf_crypto_key_ecc_verify_output_public_key_buffer_size(sf_crypto_key_size_t key_size,
                                                                        uint32_t public_key_buffer_length);
static ssp_err_t sf_crypto_key_ecc_verify_output_secret_key_buffer_size(sf_crypto_key_type_t key_type,
                                                                        sf_crypto_key_size_t key_size,
                                                                        uint32_t secret_key_buffer_length);
static ssp_err_t sf_crypto_key_ecc_verify_output_curve_data_point_buffer_size(sf_crypto_key_size_t key_size,
                                                                              uint32_t curve_data_buffer_length);
static void sf_crypto_key_ecc_set_returned_wrapped_key_length(sf_crypto_key_size_t key_size,
                                                              uint32_t * p_secret_key_data_length,
                                                              uint32_t * p_public_key_data_length);
static void sf_crypto_key_ecc_set_returned_plain_text_key_length(sf_crypto_key_size_t key_size,
                                                                 uint32_t * p_secret_key_data_length,
                                                                 uint32_t * p_public_key_data_length);
static void sf_crypto_key_ecc_set_returned_curve_point_length (sf_crypto_key_size_t key_size,
                                                               uint32_t * p_point_on_curve_data_length);

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
 * @brief          Subroutine to open a Crypto ECC HAL module. This function is called by SF_CRYPTO_KEY_Open().
 *                  The configuration parameters are validated before opening the HAL module.
 * @param[in,out]  p_ctrl                   Pointer to a Crypto Key Framework module control block
 * @param[in,out]  p_cfg                    Pointer to a Crypto Key Framework module configuration structure
 * @retval         SSP_SUCCESS              ECC HAL module is successfully opened.
 * @retval         SSP_ERR_OUT_OF_MEMORY    Failed to allocate memory to store ECC HAL module control block.
 * @retval         SSP_ERR_INTERNAL         RTOS service returned a unexpected error.
 * @return                                  See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_open_ecc (sf_crypto_key_instance_ctrl_t * p_ctrl, sf_crypto_key_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;
    ssp_err_t interface_get_err = SSP_SUCCESS;
    uint32_t err_crypto;
    sf_crypto_instance_ctrl_t * p_ctrl_common;
    ecc_cfg_t hal_cfg;

    /** Validate the cfg parameters. The key type is already validated.*/

    /** Verify the key size.*/
    err = sf_crypto_key_ecc_verify_key_size(p_ctrl->key_size);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Verify the domain parameter input buffer size. The length has to be exactly as specified */
    err = sf_crypto_key_ecc_verify_input_domain_buffer_size(p_ctrl->key_size,
                                                            p_ctrl->domain_params.data_length);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Verify the generator point input buffer size. The length has to be exactly as specified */
    err = sf_crypto_key_ecc_verify_input_generator_point_buffer_size(p_ctrl->key_size,
                                                                     p_ctrl->generator_point.data_length);

    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Get a Crypto common control block and the interface. */
    p_ctrl->p_fwk_common_ctrl = (sf_crypto_instance_ctrl_t *)p_cfg->p_lower_lvl_crypto_common->p_ctrl;
    p_ctrl_common = p_ctrl->p_fwk_common_ctrl;

    /** Allocate memory for a Crypto HAL control block in the byte pool. */
    err = sf_crypto_memory_allocate(&p_ctrl_common->byte_pool, &p_ctrl->p_hal_ctrl, (uint32_t)sizeof(ecc_ctrl_t),
                                     p_ctrl_common->wait_option);
    if (SSP_SUCCESS == err)
    {
        /** Get a ECC interface instance. */
        interface_get_err = sf_crypto_key_ecc_interface_get(p_ctrl);

        if ((interface_get_err != SSP_SUCCESS) || (NULL == p_ctrl->p_hal_api))
        {
            sf_crypto_memory_release(p_ctrl->p_hal_ctrl, (uint32_t)sizeof(ecc_ctrl_t));
            return SSP_ERR_UNSUPPORTED;
        }

        /** Set Crypto HAL API instance with the control common hardware api. */
        hal_cfg.p_crypto_api = ((crypto_instance_t *)p_ctrl_common->p_lower_lvl_crypto)->p_api;

        /** Open the Crypto HAL module. */
        err_crypto = ((ecc_api_t *)p_ctrl->p_hal_api)->open(p_ctrl->p_hal_ctrl, &hal_cfg);
        if ((uint32_t)SSP_SUCCESS != err_crypto)
        {
            err = (ssp_err_t)err_crypto;
        }
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to close a Crypto ECC HAL module. This function is called by SF_CRYPTO_KEY_Close().
 *
 * @param[in]  p_ctrl                   Pointer to a Crypto Key Framework module control block
 * @retval     SSP_SUCCESS              ECC HAL module is successfully closed.
 * @return                              See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_close_ecc (sf_crypto_key_instance_ctrl_t * p_ctrl)
{
    void * p_tmp = p_ctrl->p_hal_ctrl;

    /** Close the Crypto HAL driver. */
    ssp_err_t err = (ssp_err_t) ((ecc_api_t *)p_ctrl->p_hal_api)->close(p_ctrl->p_hal_ctrl);
    if ((uint32_t)SSP_SUCCESS == err)
    {
        /** Release the control block memory back to byte pool */
        err = sf_crypto_memory_release(p_tmp, sizeof(ecc_ctrl_t));
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to generate a ECC key. This function is called by SF_CRYPTO_KEY_Generate().
 *
 * @param[in]  p_ctrl               Pointer to a Crypto Key Framework module control block
 * @param[out] p_secret_key         Pointer to a secret key
 * @param[out] p_public_key         Pointer to a public key
 * @retval     SSP_SUCCESS          The module created a key successfully.
 * @retval     SSP_ERR_INVALID_SIZE Failed, as the allocated key buffer length is not sufficient for the
 *                                  ECC Key generation operation.
 * @retval     SSP_ERR_UNSUPPORTED  Procedure is not supported for the supplied parameters.
 * @return                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_generate_ecc (sf_crypto_key_instance_ctrl_t * p_ctrl,
                                      sf_crypto_key_t * const p_secret_key,
                                      sf_crypto_key_t * const p_public_key)
{
    ssp_err_t err_crypto = SSP_ERR_UNSUPPORTED;

    /** Verify the domain parameter input buffer size. The length has to be exactly as specified */
    err_crypto = sf_crypto_key_ecc_verify_input_domain_buffer_size(p_ctrl->key_size,
                                                                   p_ctrl->domain_params.data_length);
    if (SSP_SUCCESS != err_crypto)
    {
        return err_crypto;
    }

    /** Verify the public key buffer size that holds the generated public key. */
    err_crypto = sf_crypto_key_ecc_verify_output_public_key_buffer_size(p_ctrl->key_size,
                                                                        p_public_key->data_length);
    if (SSP_SUCCESS != err_crypto)
    {
        return err_crypto;
    }

    /** Verify the secret key buffer size that holds the generated private key. */
    err_crypto = sf_crypto_key_ecc_verify_output_secret_key_buffer_size(p_ctrl->key_type,
                                                                        p_ctrl->key_size,
                                                                        p_secret_key->data_length);
    if (SSP_SUCCESS != err_crypto)
    {
        return err_crypto;
    }

    /** Local variable created to call the HAL API. */
    r_crypto_data_handle_t   domain_params;
    r_crypto_data_handle_t   generator_point;
    r_crypto_data_handle_t   secret_key;
    r_crypto_data_handle_t   public_key;

    /** Note that the HAL driver requires data in WORDS */
    domain_params.data_length  = (p_ctrl->domain_params.data_length) / BYTES_IN_WORD;
    domain_params.p_data       = (uint32_t *) p_ctrl->domain_params.p_data;

    generator_point.data_length  = (p_ctrl->generator_point.data_length) / BYTES_IN_WORD;
    generator_point.p_data       = (uint32_t *) p_ctrl->generator_point.p_data;

    secret_key.data_length  =  p_secret_key->data_length / BYTES_IN_WORD;
    secret_key.p_data       = (uint32_t *) p_secret_key->p_data;

    public_key.data_length  =  p_public_key->data_length / BYTES_IN_WORD;
    public_key.p_data       = (uint32_t *) p_public_key->p_data;

    /** Call Crypto HAL driver. */
    err_crypto = (ssp_err_t) ((ecc_api_t *)p_ctrl->p_hal_api)->keyCreate(p_ctrl->p_hal_ctrl,
                                                                        &domain_params, &generator_point,
                                                                        &secret_key, &public_key);
    if (SSP_SUCCESS == (ssp_err_t)err_crypto)
    {
        sf_crypto_key_ecc_set_returned_key_length(p_ctrl->key_type,
                                                  p_ctrl->key_size,
                                                  &p_secret_key->data_length,
                                                  &p_public_key->data_length);
    }

    return err_crypto;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to get a ECC HAL API instance. This function is called by sf_crypto_key_open_ecc().
 *
 * @param[in,out]  p_ctrl        Pointer to a Key framework control block, whose p_hal_api filled with HAL ECC
 *                               interface. This indicates NULL, for not supported MCUs
 * @retval    SSP_SUCCESS        The HAL interface is returned successfully.
 * @return                       See @ref Common_Error_Codes or HAL driver for other possible return codes or causes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_key_ecc_interface_get (sf_crypto_key_instance_ctrl_t * const p_ctrl)
{
    sf_crypto_instance_ctrl_t * p_lower_level_crypto_ctrl;
    crypto_instance_t * p_crypto;
    crypto_interface_get_param_t param;
    ssp_err_t err_crypto = SSP_ERR_UNSUPPORTED;

    /** Get a Crypto common control block and the HAL instance. */
    p_lower_level_crypto_ctrl = (sf_crypto_instance_ctrl_t *)(p_ctrl->p_fwk_common_ctrl);
    p_crypto = (crypto_instance_t *)(p_lower_level_crypto_ctrl->p_lower_lvl_crypto);

    /** Populate param structure to get ECC HAL interface. */
    switch(p_ctrl->key_type)
    {
        case SF_CRYPTO_KEY_TYPE_ECC_WRAPPED:
            param.key_type = CRYPTO_KEY_TYPE_ECC_WRAPPED;
            break;
        case SF_CRYPTO_KEY_TYPE_ECC_PLAIN_TEXT:
            param.key_type = CRYPTO_KEY_TYPE_ECC_PLAIN_TEXT;
            break;
        default:
            param.key_type = CRYPTO_KEY_TYPE_MAX;
            break;
    }

    /** Check the ECC key type. */
    switch(p_ctrl->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_ECC_192:
            param.key_size = CRYPTO_KEY_SIZE_ECC_192;
            break;
        case SF_CRYPTO_KEY_SIZE_ECC_224:
            param.key_size = CRYPTO_KEY_SIZE_ECC_224;
            break;
        case SF_CRYPTO_KEY_SIZE_ECC_256:
            param.key_size = CRYPTO_KEY_SIZE_ECC_256;
            break;
        case SF_CRYPTO_KEY_SIZE_ECC_384:
            param.key_size = CRYPTO_KEY_SIZE_ECC_384;
            break;
        default:
            param.key_size = CRYPTO_KEY_SIZE_MAX;
            break;
    }

    /** Get the HAL API instance for a selected algorithm type. */
    param.algorithm_type = CRYPTO_ALGORITHM_TYPE_ECC;
    err_crypto = (ssp_err_t) p_crypto->p_api->interfaceGet(&param, &p_ctrl->p_hal_api);
    return err_crypto;
}

/*******************************************************************************************************************//**
 * @brief     Subroutine to verify that the provided input domain parameter `buffer_length` is as expected before
 *            making keyGenerate api call.
 * This function is called by ECC Key Generate function before passing request to Crypto HAL Key generate API call.
 *
 * @param[in] key_size                      Indicates ECC key size - 192/224/256/384-bits.
 * @param[in] domain_params_buffer_length   Length of the input domain parameter buffer length.
 * @retval    SSP_SUCCESS                   Input domain buffer length validation is successful, and proceed with ECC
 *                                          Key Generation.
 * @retval    SSP_ERR_INVALID_SIZE          Failed, as the allocated buffer length is not sufficient for the Key
 *                                          generation operation of ECC.
 * @note      This function is not a user API but an internal function for SF_CRYPTO_KEY to verify whether passed buffer
 *            size is sufficient for Key to hold in Key Generate API call.
 *
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_key_ecc_verify_input_domain_buffer_size (sf_crypto_key_size_t key_size,
                                                                    uint32_t domain_params_buffer_length)
{
    ssp_err_t err_crypto = SSP_SUCCESS;

    if (((SF_CRYPTO_KEY_SIZE_ECC_192 == key_size) &&
            (domain_params_buffer_length != ((uint32_t)ECC_192_DOMAIN_PARAMETER_WITH_ORDER_LENGTH_WORDS * BYTES_IN_WORD))) ||
        ((SF_CRYPTO_KEY_SIZE_ECC_224 == key_size) &&
            (domain_params_buffer_length != ((uint32_t)ECC_224_DOMAIN_PARAMETER_WITH_ORDER_LENGTH_WORDS * BYTES_IN_WORD))) ||
        ((SF_CRYPTO_KEY_SIZE_ECC_256 == key_size) &&
            (domain_params_buffer_length != ((uint32_t)ECC_256_DOMAIN_PARAMETER_WITH_ORDER_LENGTH_WORDS * BYTES_IN_WORD))) ||
        ((SF_CRYPTO_KEY_SIZE_ECC_384 == key_size) &&
            (domain_params_buffer_length != ((uint32_t)ECC_384_DOMAIN_PARAMETER_WITH_ORDER_LENGTH_WORDS * BYTES_IN_WORD))))
    {
        err_crypto =  SSP_ERR_INVALID_SIZE;
    }

    return err_crypto;
}

/*******************************************************************************************************************//**
 * @brief     Subroutine to verify that the provided input generator point `buffer_length` is sufficient before making
 *            keyGenerate api call.
 * This function is called by ECC Key Generate function before passing request to Crypto HAL Key generate API call.
 *
 * @param[in] key_size                      Indicates ECC key size - 192/224/256/384-bits.
 * @param[in] generator_point_buffer_length Length of the input generator point buffer length.
 * @retval    SSP_SUCCESS                   Input generator point buffer lengths validation is successful, and proceed
 *                                          with ECC Key Generation.
 * @retval    SSP_ERR_INVALID_SIZE          Failed, as the allocated buffer length is not sufficient for the Key
 *                                          generation operation of ECC.
 * @note      This function is not a user API but an internal function for SF_CRYPTO_KEY to verify whether passed buffer
 *            size is sufficient for Key to hold in Key Generate API call.
 *
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_key_ecc_verify_input_generator_point_buffer_size (sf_crypto_key_size_t key_size,
                                                                             uint32_t generator_point_buffer_length)
{
    ssp_err_t err_crypto = SSP_SUCCESS;

    if (((SF_CRYPTO_KEY_SIZE_ECC_192 == key_size) &&
             (generator_point_buffer_length != ((uint32_t)ECC_192_GENERATOR_POINT_LENGTH_WORDS * BYTES_IN_WORD))) ||
        ((SF_CRYPTO_KEY_SIZE_ECC_224 == key_size) &&
             (generator_point_buffer_length != ((uint32_t)ECC_224_GENERATOR_POINT_LENGTH_WORDS * BYTES_IN_WORD))) ||
        ((SF_CRYPTO_KEY_SIZE_ECC_256 == key_size) &&
             (generator_point_buffer_length != ((uint32_t)ECC_256_GENERATOR_POINT_LENGTH_WORDS * BYTES_IN_WORD))) ||
        ((SF_CRYPTO_KEY_SIZE_ECC_384 == key_size) &&
             (generator_point_buffer_length != ((uint32_t)ECC_384_GENERATOR_POINT_LENGTH_WORDS * BYTES_IN_WORD))))
    {
        err_crypto =  SSP_ERR_INVALID_SIZE;
    }

    return err_crypto;
}

/*******************************************************************************************************************//**
 * @brief     Subroutine to verify that the provided public key output buffer length is sufficient before making
 *            keyGenerate api call.
 * This function is called by ECC Key Generate function before passing request to Crypto HAL Key generate API call.
 *
 * @param[in] key_size                      Indicates ECC key size - 192/224/256/384-bits.
 * @param[in] public_key_buffer_length      Length of the output public key buffer length.
 * @retval    SSP_SUCCESS                   Buffer length validation is successful, and proceed with ECC Key Generation
 * @retval    SSP_ERR_INVALID_SIZE          Failed, as the allocated key length is not sufficient for the Key
 *                                          generation operation of ECC.
 * @note      This function is not a user API but an internal function for SF_CRYPTO_KEY to verify whether passed
 *            buffer size is sufficient for Key to hold in Key Generate API call.
 *
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_key_ecc_verify_output_public_key_buffer_size (sf_crypto_key_size_t key_size,
                                                                         uint32_t public_key_buffer_length)
{
    ssp_err_t err_crypto = SSP_SUCCESS;

    /** Verification of Public Key buffer length */
    if (((SF_CRYPTO_KEY_SIZE_ECC_192 == key_size) &&
             (public_key_buffer_length < ((uint32_t)ECC_192_PUBLIC_KEY_LENGTH_WORDS * BYTES_IN_WORD))) ||
        ((SF_CRYPTO_KEY_SIZE_ECC_224 == key_size) &&
             (public_key_buffer_length < ((uint32_t)ECC_224_PUBLIC_KEY_LENGTH_WORDS * BYTES_IN_WORD))) ||
        ((SF_CRYPTO_KEY_SIZE_ECC_256 == key_size) &&
             (public_key_buffer_length < ((uint32_t)ECC_256_PUBLIC_KEY_LENGTH_WORDS * BYTES_IN_WORD))) ||
        ((SF_CRYPTO_KEY_SIZE_ECC_384 == key_size) &&
             (public_key_buffer_length < ((uint32_t)ECC_384_PUBLIC_KEY_LENGTH_WORDS * BYTES_IN_WORD))))
    {
        err_crypto =  SSP_ERR_INVALID_SIZE;
    }
    return err_crypto;
}

/*******************************************************************************************************************//**
 * @brief     Subroutine to verify that the provided secret key output buffer length is sufficient before making
 *            keyGenerate api call.
 * This function is called by ECC Key Generate and scalar multiplication functions before passing request to Crypto
 *  HAL Key generate API call.
 *
 * @param[in] key_type                      Indicates ECC key type - Plain text/ Wrapped key.
 * @param[in] key_size                      Indicates ECC key size - 192/256-bits.
 * @param[in] secret_key_buffer_length      Length of the output secret key buffer length.
 * @retval    SSP_SUCCESS                   Buffer length validation is successful, and proceed with ECC Key Generation
 * @retval    SSP_ERR_INVALID_SIZE          Failed, as the allocated key length is not sufficient for the Key
 *                                          generation operation of ECC.
 * @note      This function is not a user API but an internal function for SF_CRYPTO_KEY to verify whether passed buffer
 *            size is sufficient for Key to hold in Key Generate API call.
 *
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_key_ecc_verify_output_secret_key_buffer_size (sf_crypto_key_type_t key_type,
                                                                         sf_crypto_key_size_t key_size,
                                                                         uint32_t secret_key_buffer_length)
{
    ssp_err_t err_crypto = SSP_ERR_INVALID_SIZE;

    if (SF_CRYPTO_KEY_TYPE_ECC_WRAPPED == key_type)
    {
        err_crypto = sf_crypto_key_ecc_verify_output_wrapped_key_buffer_size(key_size,
                                                                             secret_key_buffer_length);
    }
    else
    {
        /* key_type is SF_CRYPTO_KEY_TYPE_ECC_PLAIN_TEXT */
        err_crypto = sf_crypto_key_ecc_verify_output_plain_key_buffer_size(key_size,
                                                                           secret_key_buffer_length);
    }
    return err_crypto;
}

/*******************************************************************************************************************//**
 * @brief     Subroutine to verify that the output buffer length provided is sufficient to hold
 *            the generated wrapped secret key.
 * This function is called by ECC Key Generate function before passing request to Crypto HAL Key generate API call.
 *
 * @param[in] key_size                      Indicates ECC key size - 192/256-bits.
 * @param[in] secret_key_buffer_length      Length of the output wrapped secret key buffer length.
 * @retval    SSP_SUCCESS                   Buffer length validation is successful, and proceed with ECC Key Generation
 * @retval    SSP_ERR_INVALID_SIZE          Failed, as the allocated key length is not sufficient for the Key
 *                                          generation operation of ECC.
 * @note      This function is not a user API but an internal function for SF_CRYPTO_KEY to verify whether passed buffer
 *            size is sufficient for Key to hold in Key Generate API call.
 *
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_key_ecc_verify_output_wrapped_key_buffer_size (sf_crypto_key_size_t key_size,
                                                                          uint32_t secret_key_buffer_length)
{
    ssp_err_t err_crypto = SSP_ERR_INVALID_SIZE;
    uint32_t required_buffer_size = UINT32_MAX;

    switch(key_size)
    {
        case SF_CRYPTO_KEY_SIZE_ECC_192:
    	    required_buffer_size = ((uint32_t)ECC_192_PRIVATE_KEY_HRK_LENGTH_WORDS * BYTES_IN_WORD);
    	    break;
        case SF_CRYPTO_KEY_SIZE_ECC_224:
    	    required_buffer_size = ((uint32_t)ECC_224_PRIVATE_KEY_HRK_LENGTH_WORDS * BYTES_IN_WORD);
    	    break;
        case SF_CRYPTO_KEY_SIZE_ECC_256:
    	    required_buffer_size = ((uint32_t)ECC_256_PRIVATE_KEY_HRK_LENGTH_WORDS * BYTES_IN_WORD);
    	    break;
        case SF_CRYPTO_KEY_SIZE_ECC_384:
    	    required_buffer_size = ((uint32_t)ECC_384_PRIVATE_KEY_HRK_LENGTH_WORDS * BYTES_IN_WORD);
    	    break;
        default:
    	    break;
    }

    if (secret_key_buffer_length >= required_buffer_size)
    {
        err_crypto = SSP_SUCCESS;
    }

    return err_crypto;
}

/*******************************************************************************************************************//**
 * @brief     Subroutine to verify that the output buffer length provided is sufficient to hold
 *            the generated plain-text secret key.
 * This function is called by ECC Key Generate function before passing request to Crypto HAL Key generate API call.
 *
 * @param[in] key_size                      Indicates ECC key size - 192/256-bits.
 * @param[in] secret_key_buffer_length      Length of the output secret key buffer length.
 * @retval    SSP_SUCCESS                   Buffer length validation is successful, and proceed with ECC Key Generation
 * @retval    SSP_ERR_INVALID_SIZE          Failed, as the allocated key length is not sufficient for the Key
 *                                          generation operation of ECC.
 * @note      This function is not a user API but an internal function for SF_CRYPTO_KEY to verify whether passed buffer
 *            size is sufficient for Key to hold in Key Generate API call.
 *
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_key_ecc_verify_output_plain_key_buffer_size (sf_crypto_key_size_t key_size,
                                                                         uint32_t secret_key_buffer_length)
{
    ssp_err_t err = SSP_ERR_INVALID_SIZE;
    uint32_t required_buffer_size = UINT32_MAX;

    switch(key_size)
    {
        case SF_CRYPTO_KEY_SIZE_ECC_192:
    	    required_buffer_size = ((uint32_t)ECC_192_PRIVATE_KEY_LENGTH_WORDS * BYTES_IN_WORD);
    	    break;
        case SF_CRYPTO_KEY_SIZE_ECC_224:
    	    required_buffer_size = ((uint32_t)ECC_224_PRIVATE_KEY_LENGTH_WORDS * BYTES_IN_WORD);
    	    break;
        case SF_CRYPTO_KEY_SIZE_ECC_256:
    	    required_buffer_size = ((uint32_t)ECC_256_PRIVATE_KEY_LENGTH_WORDS * BYTES_IN_WORD);
    	    break;
        case SF_CRYPTO_KEY_SIZE_ECC_384:
    	    required_buffer_size = ((uint32_t)ECC_384_PRIVATE_KEY_LENGTH_WORDS * BYTES_IN_WORD);
    	    break;
        default:
    	    break;
    }

    if (secret_key_buffer_length >= required_buffer_size)
    {
        err = SSP_SUCCESS;
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief     Subroutine to validate the input and output buffer length for scalar multiplication.
 *
 * This function is called by ECC ECDH Compute function before passing request to Crypto HAL Key generate
 * API call.
 *
 * @param[in] key_size                      Indicates ECC key size - 192/224/256/384-bits.
 * @param[in] curve_data_buffer_length      Length of the input/output buffer length holding .
 * @retval    SSP_SUCCESS                   Buffer length validation is successful, and proceed with ECC Key Generation
 * @retval    SSP_ERR_INVALID_SIZE          Failed, as the allocated key length is not sufficient for the Key
 *                                          generation operation of ECC.
 * @note      This function is not a user API but an internal function for SF_CRYPTO_KEY to verify whether passed buffer
 *            size is sufficient to hold input and output data representing points on curve.
 *
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_key_ecc_verify_output_curve_data_point_buffer_size (sf_crypto_key_size_t key_size,
		                                                                       uint32_t curve_data_buffer_length)
{
    ssp_err_t err = SSP_ERR_INVALID_SIZE;
    uint32_t required_buffer_size = UINT32_MAX;

    switch(key_size)
    {
        case SF_CRYPTO_KEY_SIZE_ECC_192:
    	    required_buffer_size = ((uint32_t)ECC_192_POINT_ON_CURVE_LENGTH_WORDS * BYTES_IN_WORD);
    	    break;
        case SF_CRYPTO_KEY_SIZE_ECC_224:
    	    required_buffer_size = ((uint32_t)ECC_224_POINT_ON_CURVE_LENGTH_WORDS * BYTES_IN_WORD);
    	    break;
        case SF_CRYPTO_KEY_SIZE_ECC_256:
    	    required_buffer_size = ((uint32_t)ECC_256_POINT_ON_CURVE_LENGTH_WORDS * BYTES_IN_WORD);
    	    break;
        case SF_CRYPTO_KEY_SIZE_ECC_384:
    	    required_buffer_size = ((uint32_t)ECC_384_POINT_ON_CURVE_LENGTH_WORDS * BYTES_IN_WORD);
    	    break;
        default:
    	    break;
    }

    if (curve_data_buffer_length >= required_buffer_size)
    {
        err = SSP_SUCCESS;
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief     Subroutine to verify that the ECC key size is supported.
 * @param[in] key_size                      Indicates ECC key size.
 * @retval    SSP_SUCCESS                   Key length validation is successful.
 * @retval    SSP_ERR_UNSUPPORTED           Key size supplied is unsupported.
 *
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_key_ecc_verify_key_size (sf_crypto_key_size_t key_size)
{
    ssp_err_t err = SSP_SUCCESS;

    switch(key_size)
    {
        case SF_CRYPTO_KEY_SIZE_ECC_192:
            break;
        case SF_CRYPTO_KEY_SIZE_ECC_224:
            break;
        case SF_CRYPTO_KEY_SIZE_ECC_256:
            break;
        case SF_CRYPTO_KEY_SIZE_ECC_384:
            break;
        default:
            err = SSP_ERR_UNSUPPORTED;
        break;
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to set data_length element of public and secret key data handles.
 * This function is called by `sf_crypto_key_generate_ecc` as an internal function call.
 *
 * @param[in]   key_size                   Indicates ECC key size.
 * @param[in]   key_type                   Indicates ECC key type.
 * @param[out]  p_secret_key_data_length   Pointer to secret_key data handle's data_length element.
 * @param[out]  p_public_key_data_length   Pointer to public_key data handle's data_length element.
 *
 **********************************************************************************************************************/

static void sf_crypto_key_ecc_set_returned_key_length (sf_crypto_key_type_t key_type,
                                                       sf_crypto_key_size_t key_size,
                                                       uint32_t * p_secret_key_data_length,
                                                       uint32_t * p_public_key_data_length)
{
    if (SF_CRYPTO_KEY_TYPE_ECC_PLAIN_TEXT == key_type)
    {
        sf_crypto_key_ecc_set_returned_plain_text_key_length(key_size,
                                                             p_secret_key_data_length,
                                                             p_public_key_data_length);
    }
    else
    {
        /* key_type is SF_CRYPTO_KEY_TYPE_ECC_WRAPPED */
        sf_crypto_key_ecc_set_returned_wrapped_key_length(key_size,
                                                          p_secret_key_data_length,
                                                          p_public_key_data_length);
    }
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to set data_length element of public and wrapped secret key data handles.
 * This function is called by `sf_crypto_key_ecc_set_returned_key_length` as an internal function call.
 *
 * @param[in]   key_size                   Indicates ECC key size.
 * @param[out]  p_secret_key_data_length   Pointer to wrapped secret_key data handle's data_length element.
 * @param[out]  p_public_key_data_length   Pointer to public_key data handle's data_length element.
 *
 **********************************************************************************************************************/

static void sf_crypto_key_ecc_set_returned_wrapped_key_length (sf_crypto_key_size_t key_size,
                                                               uint32_t * p_secret_key_data_length,
                                                               uint32_t * p_public_key_data_length)
{
    switch(key_size)
    {
        case SF_CRYPTO_KEY_SIZE_ECC_192:
    	    /* key_size is SF_CRYPTO_KEY_SIZE_ECC_192 */
            *p_secret_key_data_length = ECC_192_PRIVATE_KEY_HRK_LENGTH_WORDS * BYTES_IN_WORD;
            *p_public_key_data_length = ECC_192_PUBLIC_KEY_LENGTH_WORDS * BYTES_IN_WORD;
    	    break;
        case SF_CRYPTO_KEY_SIZE_ECC_224:
    	    /* key_size is SF_CRYPTO_KEY_SIZE_ECC_224 */
            *p_secret_key_data_length = ECC_224_PRIVATE_KEY_HRK_LENGTH_WORDS * BYTES_IN_WORD;
            *p_public_key_data_length = ECC_224_PUBLIC_KEY_LENGTH_WORDS * BYTES_IN_WORD;
    	    break;
        case SF_CRYPTO_KEY_SIZE_ECC_256:
            /* key_size is SF_CRYPTO_KEY_SIZE_ECC_256 */
            *p_secret_key_data_length = ECC_256_PRIVATE_KEY_HRK_LENGTH_WORDS * BYTES_IN_WORD;
            *p_public_key_data_length = ECC_256_PUBLIC_KEY_LENGTH_WORDS * BYTES_IN_WORD;
    	    break;
        case SF_CRYPTO_KEY_SIZE_ECC_384:
    	    /* key_size is SF_CRYPTO_KEY_SIZE_ECC_384 */
            *p_secret_key_data_length = ECC_384_PRIVATE_KEY_HRK_LENGTH_WORDS * BYTES_IN_WORD;
            *p_public_key_data_length = ECC_384_PUBLIC_KEY_LENGTH_WORDS * BYTES_IN_WORD;
    	    break;
        default:
    	    break;
    }
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to set data_length element of public and plain-text secret key data handles.
 * This function is called by `sf_crypto_key_ecc_set_returned_key_length` as an internal function call.
 *
 * @param[in]   key_size                   Indicates ECC key size.
 * @param[out]  p_secret_key_data_length   Pointer to plain-text secret_key data handle's data_length element.
 * @param[out]  p_public_key_data_length   Pointer to public_key data handle's data_length element.
 *
 **********************************************************************************************************************/

static void sf_crypto_key_ecc_set_returned_plain_text_key_length (sf_crypto_key_size_t key_size,
                                                                  uint32_t * p_secret_key_data_length,
                                                                  uint32_t * p_public_key_data_length)
{
    switch(key_size)
    {
        case SF_CRYPTO_KEY_SIZE_ECC_192:
            /* key_size is SF_CRYPTO_KEY_SIZE_ECC_192 */
            *p_secret_key_data_length = ECC_192_PRIVATE_KEY_LENGTH_WORDS * BYTES_IN_WORD;
            *p_public_key_data_length = ECC_192_PUBLIC_KEY_LENGTH_WORDS * BYTES_IN_WORD;
            break;
        case SF_CRYPTO_KEY_SIZE_ECC_224:
            /* key_size is SF_CRYPTO_KEY_SIZE_ECC_224 */
            *p_secret_key_data_length = ECC_224_PRIVATE_KEY_LENGTH_WORDS * BYTES_IN_WORD;
            *p_public_key_data_length = ECC_224_PUBLIC_KEY_LENGTH_WORDS * BYTES_IN_WORD;
            break;
        case SF_CRYPTO_KEY_SIZE_ECC_256:
            /* key_size is SF_CRYPTO_KEY_SIZE_ECC_256 */
            *p_secret_key_data_length = ECC_256_PRIVATE_KEY_LENGTH_WORDS * BYTES_IN_WORD;
            *p_public_key_data_length = ECC_256_PUBLIC_KEY_LENGTH_WORDS * BYTES_IN_WORD;
            break;
        case SF_CRYPTO_KEY_SIZE_ECC_384:
            /* key_size is SF_CRYPTO_KEY_SIZE_ECC_384 */
            *p_secret_key_data_length = ECC_384_PRIVATE_KEY_LENGTH_WORDS * BYTES_IN_WORD;
            *p_public_key_data_length = ECC_384_PUBLIC_KEY_LENGTH_WORDS * BYTES_IN_WORD;
            break;
        default:
            break;
    }
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to perform a ECC scalar multiplication.
 * This function is called by SF_CRYPTO_KEY_ECDHCompute().
 *
 * @param[in]  p_ctrl               Pointer to a Crypto Key Framework module control block
 * @param[in]  p_secret_key         Pointer to a secret key
 * @param[in]  p_point_on_curve     Pointer to a point specified on the curve.
 * @param[in]  p_resultant_vector   Pointer to the resultant point on the curve.
 * @retval     SSP_SUCCESS          The module created a key successfully.
 * @retval     SSP_ERR_INVALID_SIZE Failed, as the allocated key buffer length is not sufficient for the
 *                                  ECC Key generation operation.
 * @retval     SSP_ERR_UNSUPPORTED  Procedure is not supported for the supplied parameters.
 * @return                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_key_scalar_multiplication_ecc (sf_crypto_key_instance_ctrl_t * p_ctrl,
                                                   sf_crypto_key_t * const p_secret_key,
                                                   sf_crypto_key_t * const p_point_on_curve,
                                                   sf_crypto_key_t * const p_resultant_vector)
{
    ssp_err_t err_crypto = SSP_ERR_UNSUPPORTED;

    /** Verify the secret key buffer size that holds the generated private key. */
    err_crypto = sf_crypto_key_ecc_verify_output_secret_key_buffer_size(p_ctrl->key_type,
                                                                        p_ctrl->key_size,
                                                                        p_secret_key->data_length);
    if (SSP_SUCCESS != err_crypto)
    {
        return err_crypto;
    }

    /** Verify the input buffer size for holding the data, representing point on the curve data. */
    err_crypto = sf_crypto_key_ecc_verify_output_curve_data_point_buffer_size(p_ctrl->key_size,
                                                                              p_point_on_curve->data_length);
    if (SSP_SUCCESS != err_crypto)
    {
        return err_crypto;
    }

    /** Verify the output buffer size to hold the resultant data, representing point on the curve. */
    err_crypto = sf_crypto_key_ecc_verify_output_curve_data_point_buffer_size(p_ctrl->key_size,
                                                                              p_resultant_vector->data_length);
    if (SSP_SUCCESS != err_crypto)
    {
        return err_crypto;
    }

    /** Local variable created to call the HAL API. */
    r_crypto_data_handle_t   domain_params;
    r_crypto_data_handle_t   secret_key;
    r_crypto_data_handle_t   point_on_curve;
    r_crypto_data_handle_t   resultant_vector;
    uint32_t domain_length = (p_ctrl->domain_params.data_length) / BYTES_IN_WORD;

    /** Note that the HAL driver requires data in WORDS */
    /* Scalar multiplication in HAL expects domain length without order length. Each domain parameter
     * a||b||p||n will have 1/4th the size of the domain parameter length with order details    */
    domain_params.data_length  = domain_length - (domain_length >> 2U);
    domain_params.p_data       = (uint32_t *) p_ctrl->domain_params.p_data;

    secret_key.data_length  =  p_secret_key->data_length / BYTES_IN_WORD;
    secret_key.p_data       =  (uint32_t *) p_secret_key->p_data;

    point_on_curve.data_length  =  p_point_on_curve->data_length / BYTES_IN_WORD;
    point_on_curve.p_data       =  (uint32_t *) p_point_on_curve->p_data;

    resultant_vector.data_length  =  p_resultant_vector->data_length / BYTES_IN_WORD;
    resultant_vector.p_data       =  (uint32_t *) p_resultant_vector->p_data;

    /** Call Crypto HAL driver. */
    err_crypto = (ssp_err_t) ((ecc_api_t *)p_ctrl->p_hal_api)->scalarMultiplication(p_ctrl->p_hal_ctrl,
                                                                                    &domain_params,
                                                                                    &secret_key,
                                                                                    &point_on_curve,
                                                                                    &resultant_vector);
    if (SSP_SUCCESS == (ssp_err_t)err_crypto)
    {
        sf_crypto_key_ecc_set_returned_curve_point_length(p_ctrl->key_size, &p_resultant_vector->data_length);
    }

    return err_crypto;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to set data_length element of resulting point data handles of scalar multiplication.
 * This function is called by `sf_crypto_key_scalar_multiplication_ecc` as an internal function call.
 *
 * @param[in]   key_size                       Indicates ECC key size.
 * @param[out]  p_point_on_curve_data_length   Pointer to plain-text secret_key data handle's data_length element.
 *
 **********************************************************************************************************************/
static void sf_crypto_key_ecc_set_returned_curve_point_length (sf_crypto_key_size_t key_size,
                                                               uint32_t * p_point_on_curve_data_length)
{
    switch(key_size)
    {
        case SF_CRYPTO_KEY_SIZE_ECC_192:
    	    /* key_size is SF_CRYPTO_KEY_SIZE_ECC_192 */
            *p_point_on_curve_data_length = ECC_192_POINT_ON_CURVE_LENGTH_WORDS * BYTES_IN_WORD;
    	    break;
        case SF_CRYPTO_KEY_SIZE_ECC_224:
            /* key_size is SF_CRYPTO_KEY_SIZE_ECC_224 */
            *p_point_on_curve_data_length = ECC_224_POINT_ON_CURVE_LENGTH_WORDS * BYTES_IN_WORD;
    	    break;
        case SF_CRYPTO_KEY_SIZE_ECC_256:
            /* key_size is SF_CRYPTO_KEY_SIZE_ECC_256 */
            *p_point_on_curve_data_length = ECC_256_POINT_ON_CURVE_LENGTH_WORDS * BYTES_IN_WORD;
    	    break;
        case SF_CRYPTO_KEY_SIZE_ECC_384:
            /* key_size is SF_CRYPTO_KEY_SIZE_ECC_384 */
            *p_point_on_curve_data_length = ECC_384_POINT_ON_CURVE_LENGTH_WORDS * BYTES_IN_WORD;
    	    break;
        default:
    	    break;
    }
}

/*******************************************************************************************************************//**
 * @} (end defgroup SF_CRYPTO_KEY)
 **********************************************************************************************************************/
