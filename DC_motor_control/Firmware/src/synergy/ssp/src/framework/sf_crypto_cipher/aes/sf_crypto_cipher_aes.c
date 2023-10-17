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
 * File Name    : sf_crypto_cipher_aes.c
 * Description  : SSP Crypto Cipher Framework Module specific for AES keys.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @file
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include <string.h>
#include "sf_crypto_cipher.h"
#include "../../sf_crypto/sf_crypto_private_api.h"
#include "../sf_crypto_cipher_private_api.h"
#include "sf_crypto_cipher_aes_private.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes. DO NOT USE! These functions are for internal usage.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_validate_init_params(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                           sf_crypto_cipher_op_mode_t cipher_operation_mode,
                                                           sf_crypto_key_t const * const p_key,
                                                           sf_crypto_cipher_aes_init_params_t * p_aes_params);

static ssp_err_t sf_crypto_cipher_aes_instance_gcm_memory_allocate(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);

static ssp_err_t sf_crypto_cipher_instance_aes_gcm_memory_release (sf_crypto_cipher_instance_ctrl_t * const p_ctrl);

static void sf_crypto_cipher_aes_fill_interface_key_type(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                     crypto_interface_get_param_t * param);

static void sf_crypto_cipher_aes_fill_interface_key_size(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                     crypto_interface_get_param_t * param);

static void sf_crypto_cipher_aes_fill_interface_chaining_mode(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                          crypto_interface_get_param_t * param);

static void sf_crypto_cipher_aes_fill_partial_block(uint8_t * p_fill_buffer,
                                                    uint32_t * p_num_bytes_in_buffer,
                                                    uint32_t * fill_bytes,
                                                    sf_crypto_data_handle_t const * const p_data_in);

static void sf_crypto_cipher_aes_check_fill_partial_block(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                      uint32_t * fill_bytes,
                                                      sf_crypto_data_handle_t const * const p_data_in);

static uint32_t sf_crypto_cipher_aes_process_aad_partial_block(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);

static ssp_err_t sf_crypto_cipher_aes_process_partial_block(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                           sf_crypto_data_handle_t * const p_data_out);

static ssp_err_t sf_crypto_cipher_aes_handle_encrypt_padding(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                             sf_crypto_data_handle_t * const p_data_out);

static ssp_err_t sf_crypto_cipher_aes_get_gcm_tag(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);

static ssp_err_t sf_crypto_cipher_aes_set_gcm_tag(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);

static ssp_err_t sf_crypto_cipher_aes_compute_verify_gcm_tag(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);

static ssp_err_t  sf_crypto_cipher_aes_helper_validate_final_params(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                             sf_crypto_data_handle_t const * const p_data_in);

static ssp_err_t  sf_crypto_cipher_aes_validate_encrypt_final_params(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                     sf_crypto_data_handle_t const * const p_data_in,
                                                                     sf_crypto_data_handle_t * const p_data_out);

static ssp_err_t  sf_crypto_cipher_aes_validate_decrypt_final_params(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                     sf_crypto_data_handle_t const * const p_data_in,
                                                                     sf_crypto_data_handle_t * const p_data_out);

static ssp_err_t sf_crypto_cipher_aes_helper_decrypt_final(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                           sf_crypto_data_handle_t const * const p_data_in,
                                                           sf_crypto_data_handle_t * const p_data_out);
static ssp_err_t sf_crypto_cipher_aes_decrypt_last_block(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                         uint8_t * p_input_data,
                                                         uint8_t * p_output_buffer,
                                                         uint32_t * num_data_bytes);
static bool sf_crypto_cipher_aes_is_last_decrypt_block(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                       uint32_t remaining_length);

static ssp_err_t sf_crypto_cipher_aes_hal_encrypt_decrypt(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                          uint32_t *p_key,
                                                          uint32_t *p_iv,
                                                          uint32_t num_words,
                                                          uint32_t *p_input,
                                                          uint32_t *p_output
                                                          );

static uint32_t sf_crypto_cipher_aes_gcm_process_partial_block (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                sf_crypto_data_handle_t * const p_data_out);

static void sf_crypto_cipher_aes_zeroise_context_buffer(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);
static ssp_err_t sf_crypto_cipher_aes_get_plain_text_key_size_bytes(sf_crypto_key_size_t key_size,
                                                                    uint32_t *key_size_bytes);
static ssp_err_t sf_crypto_cipher_aes_get_wrapped_key_size_bytes(sf_crypto_key_size_t key_size,
                                                                 uint32_t *key_size_bytes);
static ssp_err_t sf_crypto_cipher_aes_validate_init_gcm_params (sf_crypto_cipher_aes_init_params_t * p_aes_params);
static ssp_err_t sf_crypto_cipher_aes_update_helper_gcm_aad_update(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);
static ssp_err_t sf_crypto_cipher_aes_validate_init_padding_scheme (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                    sf_crypto_cipher_padding_scheme_t padding_scheme);
static ssp_err_t sf_crypto_cipher_aes_gcm_helper_decrypt_final(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                               sf_crypto_data_handle_t const * const p_data_in,
                                                               sf_crypto_data_handle_t * const p_data_out);
static ssp_err_t sf_crypto_cipher_aes_helper_decrypt_last_block(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                               uint32_t input_data_length,
                                                                uint8_t * p_input_data,
                                                                sf_crypto_data_handle_t * const p_data_out,
                                                                uint32_t data_out_offset);

static void sf_crypto_cipher_aes_context_buffer_pointer_data_zeroise(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);
static ssp_err_t sf_crypto_cipher_aes_validate_update_params_context(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);

static ssp_err_t sf_crypto_cipher_aes_check_update_params(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                          sf_crypto_data_handle_t const * const p_data_in,
                                                          sf_crypto_data_handle_t * const p_data_out);

static bool sf_crypto_cipher_is_last_block_with_padding(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                        uint32_t remaining_input_length);
static ssp_err_t sf_crypto_cipher_aes_init_gcm_params(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                      sf_crypto_cipher_op_mode_t cipher_operation_mode,
                                                      sf_crypto_key_t const * const p_key,
                                                      sf_crypto_cipher_algorithm_init_params_t * p_algorithm_specific_params);

static ssp_err_t sf_crypto_cipher_aes_hal_open(sf_crypto_cipher_instance_ctrl_t * p_ctrl,
                                               sf_crypto_cipher_cfg_t const * const p_cfg);

static ssp_err_t sf_crypto_cipher_aes_hal_close(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);

static ssp_err_t sf_crypto_cipher_instance_aes_memory_allocate(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);
static ssp_err_t sf_crypto_cipher_instance_aes_memory_deallocate(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);
static ssp_err_t  sf_crypto_cipher_aes_get_key_size_bytes(sf_crypto_key_type_t key_type,
                                                          sf_crypto_key_size_t key_size,
                                                          uint32_t *key_size_bytes);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
static const uint8_t gcm_iv_padding[SF_CRYPTO_CIPHER_AES_GCM_IV_PAD_4_BYTES] = {0x00, 0x00, 0x00, 0x01};

/***********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
* @ingroup SF_Library
* @addtogroup SF_CRYPTO_CIPHER
* @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
Functions
***********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @brief          Subroutine to open a Crypto AES HAL module. This function is called by SF_CRYPTO_CIPHER_Open().
 *
 * @param[in,out]  p_ctrl                   Pointer to the cipher framework module control block used and set
 * @param[in]      p_cfg                    Pointer to the cipher framework module configuration parameters.
 *                                          in the open() call.
 * @retval         SSP_SUCCESS              AES HAL module is successfully opened.
 * @retval         SSP_ERR_OUT_OF_MEMORY    Failed to allocate memory to store AES HAL module control block.
 * @retval         SSP_ERR_INTERNAL         RTOS service returned a unexpected error.
 * @return                                  See @ref Common_Error_Codes or HAL driver for other possible
 *                                          return codes or causes. This function calls:aes_api_t::open
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_hal_open (sf_crypto_cipher_instance_ctrl_t * p_ctrl,
                                                sf_crypto_cipher_cfg_t const * const p_cfg)
{
    SSP_PARAMETER_NOT_USED(p_cfg);
    ssp_err_t   err = SSP_SUCCESS;
    uint32_t    err_crypto;
    sf_crypto_instance_ctrl_t * p_fwk_common_ctrl;
    aes_cfg_t   hal_cfg;

    /** Get a Crypto common control block and the interface. */
    p_fwk_common_ctrl = p_ctrl->p_lower_lvl_fwk_common_ctrl;

    /** Allocate memory for a Crypto HAL control block in the byte pool. */
    err = (ssp_err_t) sf_crypto_memory_allocate (&p_fwk_common_ctrl->byte_pool,
                                                 &p_ctrl->p_hal_ctrl,
                                                 sizeof(aes_ctrl_t),
                                                 p_fwk_common_ctrl->wait_option);
    if (SSP_SUCCESS == err)
    {
        /** Get a AES interface instance. */
        sf_crypto_cipher_aes_interface_get(p_ctrl);

        if (NULL == p_ctrl->p_hal_api)
        {
            sf_crypto_memory_release(p_ctrl->p_hal_ctrl, sizeof(aes_ctrl_t));
            return SSP_ERR_UNSUPPORTED;
        }

        /** Set Crypto HAL API instance with the control common hardware api. */
        hal_cfg.p_crypto_api = ((crypto_instance_t *)p_fwk_common_ctrl->p_lower_lvl_crypto)->p_api;

        /** Open the Crypto HAL AES module. */
        err_crypto = ((aes_api_t *)p_ctrl->p_hal_api)->open ((aes_ctrl_t *)(p_ctrl->p_hal_ctrl), &hal_cfg);
        if ((uint32_t)SSP_SUCCESS != err_crypto)
        {
            err = (ssp_err_t)err_crypto;
        }
    }

    return err;
} /* End of function sf_crypto_cipher_aes_hal_open */

/*******************************************************************************************************************//**
 * @brief      Subroutine to close a Crypto AES HAL module. This function is called by SF_CRYPTO_CIPHER_Close().
 *
 * @param[in]   p_ctrl                  Pointer to the cipher framework module control block used and set
 *                                      in the open() call.
 * @retval      SSP_SUCCESS             AES HAL module is successfully closed.
 * @return                              See @ref Common_Error_Codes or HAL driver for other possible
 *                                      return codes or causes. This function calls:aes_api_t::close
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_hal_close (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t   err = (ssp_err_t) ((aes_api_t *)p_ctrl->p_hal_api)->close ((aes_ctrl_t *)p_ctrl->p_hal_ctrl);
    if (SSP_SUCCESS == err)
    {
        /** Release the control block memory back to byte pool */
        err = sf_crypto_memory_release(p_ctrl->p_hal_ctrl, sizeof(aes_ctrl_t));
    }
    return err;
} /* End of function sf_crypto_cipher_aes_hal_close */

/*******************************************************************************************************************//**
 * @brief      Subroutine to get a pointer to the AES HAL API instance.
 *
 * @param[in,out]  p_ctrl   Pointer to a Cipher framework control block, whose p_hal_api is filled with HAL AES
 *                          interface. This will be NULL, for MCUs /feature/ configuration parameters not supported
 **********************************************************************************************************************/
void sf_crypto_cipher_aes_interface_get (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    sf_crypto_instance_ctrl_t * p_lower_level_crypto_ctrl;
    crypto_instance_t         * p_crypto;
    crypto_interface_get_param_t param;
    p_ctrl->p_hal_api = NULL;

    /** Get a Crypto common control block and the HAL instance. */
    p_lower_level_crypto_ctrl = (sf_crypto_instance_ctrl_t *)(p_ctrl->p_lower_lvl_fwk_common_ctrl);
    p_crypto = (crypto_instance_t *)(p_lower_level_crypto_ctrl->p_lower_lvl_crypto);

    /** Check the AES key type and size and get an appropriate API instance. */

    sf_crypto_cipher_aes_fill_interface_key_type(p_ctrl, &param);
    sf_crypto_cipher_aes_fill_interface_key_size(p_ctrl, &param);
    sf_crypto_cipher_aes_fill_interface_chaining_mode(p_ctrl, &param);

    /** Get the HAL API instance for a selected algorithm type. */
    param.algorithm_type = CRYPTO_ALGORITHM_TYPE_AES;
    p_crypto->p_api->interfaceGet(&param, &p_ctrl->p_hal_api);
} /* End of function sf_crypto_cipher_aes_interface_get */

/*******************************************************************************************************************//**
 * @brief   Subroutine to fill the key type parameter in the crypto_interface_get_param_t structure.
 *
 * @param[in,out]   p_ctrl      Pointer to the cipher framework module control block used and set
 *                              in the open() call. *
 * @param[in]       param       Pointer to a crypto_interface_get_param_t structure to get the lower level interface.
 **********************************************************************************************************************/
static void sf_crypto_cipher_aes_fill_interface_key_type (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                      crypto_interface_get_param_t * param)
{
    switch (p_ctrl->key_type)
    {
        case SF_CRYPTO_KEY_TYPE_AES_PLAIN_TEXT:
            param->key_type = CRYPTO_KEY_TYPE_AES_PLAIN_TEXT;
        break;
        case SF_CRYPTO_KEY_TYPE_AES_WRAPPED:
            param->key_type = CRYPTO_KEY_TYPE_AES_WRAPPED;
        break;
        default:
            param->key_type = CRYPTO_KEY_TYPE_MAX;
        break;
    }
}

/*******************************************************************************************************************//**
 * @brief   Subroutine to fill the key size parameter in the crypto_interface_get_param_t structure.
 *          This function is called by SF_CRYPTO_CIPHER_Open().
 *
 * @param[in,out]   p_ctrl      Pointer to a Crypto Key Framework module control block.
 * @param[in]       param       Pointer to a crypto_interface_get_param_t structure to get the lower level interface.
 **********************************************************************************************************************/
static void sf_crypto_cipher_aes_fill_interface_key_size (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                      crypto_interface_get_param_t * param)
{
    switch (p_ctrl->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_AES_128:
            param->key_size = CRYPTO_KEY_SIZE_AES_128;
        break;
        case SF_CRYPTO_KEY_SIZE_AES_XTS_128:
            param->key_size = CRYPTO_KEY_SIZE_AES_128;
        break;
        case SF_CRYPTO_KEY_SIZE_AES_192:
            param->key_size = CRYPTO_KEY_SIZE_AES_192;
        break;
        case SF_CRYPTO_KEY_SIZE_AES_256:
            param->key_size = CRYPTO_KEY_SIZE_AES_256;
        break;
        case SF_CRYPTO_KEY_SIZE_AES_XTS_256:
            param->key_size = CRYPTO_KEY_SIZE_AES_256;
        break;
        default:
            param->key_size = CRYPTO_KEY_SIZE_MAX;
        break;
    }
}

/*******************************************************************************************************************//**
 * @brief   Subroutine to fill the chaining mode parameter in the crypto_interface_get_param_t structure.
 *          This function is called by SF_CRYPTO_CIPHER_Open().
 *
 * @param[in,out]   p_ctrl      Pointer to a Crypto Key Framework module control block.
 * @param[in]       param       Pointer to a crypto_interface_get_param_t structure to get the lower level interface.
 **********************************************************************************************************************/
static void sf_crypto_cipher_aes_fill_interface_chaining_mode (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                           crypto_interface_get_param_t * param)
{
    switch (p_ctrl->cipher_chaining_mode)
    {

        case SF_CRYPTO_CIPHER_MODE_ECB:
            param->chaining_mode = CRYPTO_ECB_MODE;
        break;
        case SF_CRYPTO_CIPHER_MODE_CBC:
            param->chaining_mode = CRYPTO_CBC_MODE;
        break;
        case SF_CRYPTO_CIPHER_MODE_CTR:
            param->chaining_mode = CRYPTO_CTR_MODE;
        break;
        case SF_CRYPTO_CIPHER_MODE_XTS:
            param->chaining_mode = CRYPTO_XTS_MODE;
        break;
        case SF_CRYPTO_CIPHER_MODE_GCM:
            param->chaining_mode = CRYPTO_GCM_MODE;
        break;
        default:
            param->chaining_mode = CRYPTO_CHAINING_MODE_MAX;
        break;
    }
}

/*******************************************************************************************************************//**
 * @brief   Subroutine to allocate memory required for the context buffer for AES cipher operation.
 * A contiguous block for the entire memory requirement for AES instance may not be available in the byte pool if it is
 * fragmented. Hence memory is allocated in chunks to get the most out of the byte pool.
 *
 * @param[in,out]   p_ctrl                  Pointer to a Crypto Key Framework module control block.
 * @retval          SSP_SUCCESS             Memory allocation was successful.
 * @retval          SSP_ERR_TIMEOUT         Was unable to allocate the memory within the specified time to wait.
 * @retval          SSP_ERR_OUT_OF_MEMORY   Requested size is zero or larger than the pool.
 * @retval          SSP_ERR_INTERNAL        RTOS service returned a unexpected error. *
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_instance_aes_memory_allocate (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_instance_ctrl_t * p_ctrl_common =  p_ctrl->p_lower_lvl_fwk_common_ctrl;
    sf_crypto_cipher_aes_context_t * p_context = NULL;

    /** Allocate memory for the AES context buffer. Extra 3 bytes are allocated to get the buffer WORD aligned*/
    err = sf_crypto_memory_allocate (&p_ctrl_common->byte_pool,
                                     (void **)&p_ctrl->p_cipher_context_buffer,
                                     sizeof (sf_crypto_cipher_aes_context_t),
                                     p_ctrl_common->wait_option);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    /* Zeroise the context buffer and set all pointers to NULL */
    sf_crypto_cipher_aes_zeroise_context_buffer(p_ctrl);

    err = sf_crypto_cipher_aes_get_key_size_bytes(p_ctrl->key_type,
                                                  p_ctrl->key_size,
                                                  &(p_context->cipher_key_buffer_size));
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    p_context->working_iv_buffer_size = SF_CRYPTO_CIPHER_AES_IV_LENGTH_16_BYTES;
    p_context->partial_block_size = (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES;

    /** Allocate memory for IV in the context buffer Extra 3 bytes are allocated to get the buffer WORD aligned*/
    err = sf_crypto_memory_allocate (&p_ctrl_common->byte_pool,
                                     (void **)&p_context->working_iv,
                                     p_context->working_iv_buffer_size,
                                     p_ctrl_common->wait_option);
    if (SSP_SUCCESS != err)
    {
        sf_crypto_memory_release (p_ctrl->p_cipher_context_buffer,
                                  sizeof (sf_crypto_cipher_aes_context_t));
        return err;
    }

    /** Allocate memory for the key in the context buffer Extra 3 bytes are allocated to get the buffer WORD aligned*/
    err = sf_crypto_memory_allocate (&p_ctrl_common->byte_pool,
                                     (void **)&p_context->cipher_key,
                                     p_context->cipher_key_buffer_size,
                                     p_ctrl_common->wait_option);
    if (SSP_SUCCESS != err)
    {
        sf_crypto_memory_release (p_context->working_iv,
                                  p_context->working_iv_buffer_size);
        sf_crypto_memory_release (p_ctrl->p_cipher_context_buffer,
                                  sizeof (sf_crypto_cipher_aes_context_t));
        return err;
    }

    /** Allocate memory for the partial block in the context buffer
     * Extra 3 bytes are allocated to get the buffer WORD aligned.
     */
    err = sf_crypto_memory_allocate (&p_ctrl_common->byte_pool,
                                     (void **)&p_context->partial_block,
                                     p_context->partial_block_size,
                                     p_ctrl_common->wait_option);
    if (SSP_SUCCESS != err)
    {
        sf_crypto_memory_release(p_context->cipher_key,
                                 p_context->cipher_key_buffer_size);
        sf_crypto_memory_release(p_context->working_iv,
                                 p_context->working_iv_buffer_size);
        sf_crypto_memory_release(p_ctrl->p_cipher_context_buffer,
                                 sizeof (sf_crypto_cipher_aes_context_t));
        return err;
    }

    if (p_ctrl->cipher_chaining_mode == SF_CRYPTO_CIPHER_MODE_GCM)
    {
        /** Allocate memory required for GCM related elements in the context buffer  */
        err = sf_crypto_cipher_aes_instance_gcm_memory_allocate(p_ctrl);
    }
    if (SSP_SUCCESS != err)
    {
        sf_crypto_memory_release(p_context->partial_block,
                                 p_context->partial_block_size);
        sf_crypto_memory_release(p_context->cipher_key,
                                 p_context->cipher_key_buffer_size);
        sf_crypto_memory_release(p_context->working_iv,
                                 p_context->working_iv_buffer_size);
        sf_crypto_memory_release(p_ctrl->p_cipher_context_buffer,
                                 sizeof (sf_crypto_cipher_aes_context_t));
        return err;
    }
    return (err);

} /* End of function sf_crypto_cipher_instance_aes_memory_allocate */

/*******************************************************************************************************************//**
 * @brief   Subroutine to check if the key type enum provided is a valid AES key type for cipher operation.
 *
 * @param[in]   key_type    The key type to be tested.
 * @retval      true        The key is a valid AES key type.
 * @retval      false       The key is NOT a valid AES key type.
 **********************************************************************************************************************/
bool sf_crypto_cipher_is_key_type_aes (sf_crypto_key_type_t key_type)
{
    bool return_value = false;

    /** Check the key type and and determine the algorithm. */
    if ((key_type == SF_CRYPTO_KEY_TYPE_AES_PLAIN_TEXT) ||
        (key_type == SF_CRYPTO_KEY_TYPE_AES_WRAPPED))
      {
        return_value = true;
     }
    return return_value;
} /* End of function sf_crypto_cipher_is_key_type_aes */

/*******************************************************************************************************************//**
 * @brief   Subroutine to get AES plain text key size in bytes given the key size enums.
 *
 * @param[in]       key_size            The key size enum to be used in calculating the key size in bytes.
 * @param[in, out]  key_size_bytes      The AES key size in bytes.
 * @retval          SSP_ERR_INVALID_ARGUMENT    At least one of the inputs is invalid.
 * @retval          SSP_SUCCESS                 The function executed successfully.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_get_plain_text_key_size_bytes (sf_crypto_key_size_t key_size,
                                                                     uint32_t *key_size_bytes)
{
    ssp_err_t   err = SSP_SUCCESS;
    *key_size_bytes = 0U;

    switch (key_size)
    {
        case SF_CRYPTO_KEY_SIZE_AES_128:
            *key_size_bytes = (uint32_t)AES128_SECRET_KEY_SIZE_BYTES;
        break;

        case SF_CRYPTO_KEY_SIZE_AES_XTS_128:
            *key_size_bytes = ((uint32_t)SF_CRYPTO_CIPHER_AES_128_XTS_KEY_SIZE);
        break;

        case SF_CRYPTO_KEY_SIZE_AES_192:
            *key_size_bytes = (uint32_t)AES192_SECRET_KEY_SIZE_BYTES;
        break;

        case SF_CRYPTO_KEY_SIZE_AES_256:
            *key_size_bytes = (uint32_t)AES256_SECRET_KEY_SIZE_BYTES;
        break;

        case SF_CRYPTO_KEY_SIZE_AES_XTS_256:
            *key_size_bytes = ((uint32_t)SF_CRYPTO_CIPHER_AES_256_XTS_KEY_SIZE);
        break;

        default:
            err = SSP_ERR_INVALID_ARGUMENT;
        break;
    }
    return err;
}   /* End of function sf_crypto_cipher_aes_get_plain_text_key_size_bytes */

/*******************************************************************************************************************//**
 * @brief   Subroutine to get AES wrapped key size in bytes given the key size enums.
 *
 * @param[in]       key_size            The key size enum to be used in calculating the key size in bytes.
 * @param[in, out]  key_size_bytes      The AES key size in bytes.
 * @retval          SSP_ERR_INVALID_ARGUMENT    At least one of the inputs is invalid.
 * @retval          SSP_SUCCESS                 The function executed successfully.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_get_wrapped_key_size_bytes (sf_crypto_key_size_t key_size,
                                                                  uint32_t *key_size_bytes)
{
    ssp_err_t err = SSP_SUCCESS;
    *key_size_bytes = 0x0U;

    switch (key_size)
    {
        case SF_CRYPTO_KEY_SIZE_AES_128:
            *key_size_bytes = (uint32_t)AES128_WRAPPPED_SECRET_KEY_SIZE_BYTES;
        break;

        case SF_CRYPTO_KEY_SIZE_AES_XTS_128:
            *key_size_bytes = (uint32_t)AES_XTS_128_WRAPPPED_SECRET_KEY_SIZE_BYTES;
        break;

        case SF_CRYPTO_KEY_SIZE_AES_192:
            *key_size_bytes = (uint32_t)AES192_WRAPPPED_SECRET_KEY_SIZE_BYTES;
        break;

        case SF_CRYPTO_KEY_SIZE_AES_256:
            *key_size_bytes = (uint32_t)AES256_WRAPPPED_SECRET_KEY_SIZE_BYTES;
        break;

        case SF_CRYPTO_KEY_SIZE_AES_XTS_256:
            *key_size_bytes = (uint32_t)AES_XTS_256_WRAPPPED_SECRET_KEY_SIZE_BYTES;
        break;

        default:
            err = SSP_ERR_INVALID_ARGUMENT;
        break;
    }
    return err;
} /* End of function sf_crypto_cipher_aes_get_wrapped_key_size_bytes */

/*******************************************************************************************************************//**
 * @brief   Subroutine to get AES key size in bytes given the key type and key size enums.
 *
 * @param[in]   key_type            The key type enum to be used in calculating the key size in bytes.
 * @param[in]   key_size            The key size enum to be used in calculating the key size in bytes.
 * @param[in, out]   key_size_bytes The key size in bytes.
 * @retval      SSP_ERR_INVALID_ARGUMENT    At least one of the inputs is invalid.
 * @retval      SSP_SUCCESS                  The function executed successfully.
 **********************************************************************************************************************/
 static ssp_err_t sf_crypto_cipher_aes_get_key_size_bytes(sf_crypto_key_type_t key_type,
                                                          sf_crypto_key_size_t key_size,
                                                          uint32_t *key_size_bytes)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_key_type_t local_key_type = key_type;
    *key_size_bytes = 0x0U;

    switch (local_key_type)
    {
        case SF_CRYPTO_KEY_TYPE_AES_PLAIN_TEXT:
            err   = sf_crypto_cipher_aes_get_plain_text_key_size_bytes(key_size, key_size_bytes);
        break;

        case SF_CRYPTO_KEY_TYPE_AES_WRAPPED:
            err = sf_crypto_cipher_aes_get_wrapped_key_size_bytes(key_size, key_size_bytes);
        break;

        default:
            err = SSP_ERR_INVALID_ARGUMENT;
        break;
    }
    return err;
} /* End of function sf_crypto_cipher_aes_get_key_size_bytes */

/*******************************************************************************************************************//**
 * @brief   Subroutine to validate the input parameters for the AES cipher Init operation.
 *
 * @param[in]       p_aes_params                Pointer to the AES algorithm specific parameter struct.
 *
 * @retval          SSP_SUCCESS                 All of the input parameters are validated successfully.
 * @retval          SSP_ERR_INVALID_ARGUMENT    An input for the required cipher operation is invalid.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_validate_init_gcm_params (sf_crypto_cipher_aes_init_params_t * p_aes_params)
{

    /** GCM supports 96-bit IV and 128-bit IV */
    if (((uint32_t)SF_CRYPTO_CIPHER_AES_IV_LENGTH_16_BYTES != (uint32_t)(p_aes_params->p_iv->data_length)) &&
        ((uint32_t)SF_CRYPTO_CIPHER_AES_IV_LENGTH_12_BYTES != (uint32_t)(p_aes_params->p_iv->data_length)))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    /** Tag is necessary for both encrypt and decrypt operations*/
    if ((NULL == p_aes_params->p_auth_tag) ||
        (NULL == p_aes_params->p_auth_tag->p_data) ||
        (p_aes_params->p_auth_tag->data_length != SF_CRYPTO_CIPHER_AES_GCM_TAG_LENGTH_16_BYTES))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    return SSP_SUCCESS;
}  /* End of function sf_crypto_cipher_aes_validate_init_gcm_params */

/*******************************************************************************************************************//**
 * @brief   Subroutine to validate the IV for the AES cipher Init operation for modes other than ECB and GCM.
 *
 *
 * @param[in, out]  p_ctrl                      Pointer to the cipher framework module control block used
 *                                              in the open() call.
  * @param[in]      p_aes_params                Pointer to the algorithm specific parameters.
 * @retval          SSP_SUCCESS                 All of the input parameters are validated successfully.
 * @retval          SSP_ERR_INVALID_ARGUMENT    An input for the required cipher operation is invalid.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_validate_init_params_iv (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                               sf_crypto_cipher_aes_init_params_t * p_aes_params)
{
    if ((SF_CRYPTO_CIPHER_MODE_ECB != p_ctrl->cipher_chaining_mode) &&
        (SF_CRYPTO_CIPHER_MODE_GCM != p_ctrl->cipher_chaining_mode) &&
        ((NULL == p_aes_params->p_iv) ||
        (NULL == p_aes_params->p_iv->p_data) ||
        ((uint32_t)SF_CRYPTO_CIPHER_AES_IV_LENGTH_16_BYTES != (uint32_t)(p_aes_params->p_iv->data_length))))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }
    return SSP_SUCCESS;

} /* End of function sf_crypto_cipher_aes_validate_init_params_iv */

/*******************************************************************************************************************//**
 * @brief   Subroutine to validate the padding scheme for AES cipher Init operation.
 *
 * @param[in, out]  p_ctrl                      Pointer to the cipher framework module control block used
 *                                              in the open() call.
 * @param[in]       padding_scheme              The padding scheme to be used for the cipher operations.

 * @retval          SSP_SUCCESS                 All of the input parameters are validated successfully.
 * @retval          SSP_ERR_INVALID_ARGUMENT    An input for the required cipher operation is invalid.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_validate_init_padding_scheme (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                    sf_crypto_cipher_padding_scheme_t padding_scheme)
{
    if ((SF_CRYPTO_CIPHER_PADDING_SCHEME_NO_PADDING != padding_scheme) &&
        (SF_CRYPTO_CIPHER_PADDING_SCHEME_PKCS7 != padding_scheme))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    /** Expect NO_PADDING for CTR and XTS - we expect multiple of block size
     * For GCM it will be internally padded with 0, so user has to choose no padding.
     */

    if (((SF_CRYPTO_CIPHER_MODE_CTR == (p_ctrl->cipher_chaining_mode)) ||
         (SF_CRYPTO_CIPHER_MODE_XTS == (p_ctrl->cipher_chaining_mode)) ||
         (SF_CRYPTO_CIPHER_MODE_GCM == (p_ctrl->cipher_chaining_mode)))   &&
        (SF_CRYPTO_CIPHER_PADDING_SCHEME_NO_PADDING != padding_scheme))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }
    return SSP_SUCCESS;
} /* End of function sf_crypto_cipher_aes_validate_init_padding_scheme  */


/*******************************************************************************************************************//**
 * @brief   Subroutine to validate the input parameters for the AES cipher Init operation.
 *
 * @param[in, out]  p_ctrl                      Pointer to the cipher framework module control block used
 *                                              in the open() call.
 * @param[in]       cipher_operation_mode       The cipher operation mode - encrypt / decrypt.
 * @param[in]       p_key                       Pointer to the key to be used for the cipher operations.
 * @param[in]       p_aes_params                Pointer to the algorithm specific parameters.
 * @retval          SSP_SUCCESS                 All of the input parameters are validated successfully.
 * @retval          SSP_ERR_INVALID_ARGUMENT    An input for the required cipher operation is invalid.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_validate_init_params (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                            sf_crypto_cipher_op_mode_t cipher_operation_mode,
                                                            sf_crypto_key_t const * const p_key,
                                                            sf_crypto_cipher_aes_init_params_t * p_aes_params)
{
    SSP_PARAMETER_NOT_USED(cipher_operation_mode);

    ssp_err_t err = SSP_SUCCESS;
    uint32_t key_size_bytes = 0;

    /** Cipher operation mode is already validated before this function is called. */

    /** Validate key length */
    err = sf_crypto_cipher_aes_get_key_size_bytes(p_ctrl->key_type,
                                                  p_ctrl->key_size,
                                                  &key_size_bytes);
    if (SSP_SUCCESS != err)
    {
       return err;
    }
    if ((p_key->data_length != key_size_bytes) ||
            (NULL == p_key->p_data))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    /** Validate padding scheme*/
    err = sf_crypto_cipher_aes_validate_init_padding_scheme(p_ctrl, p_aes_params->padding_scheme);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Validate algorithm specific parameters - IV for all modes except ECB and GCM
     * ECB does not need IV and IV length for GCM will be tested in the next call to a GCM specific test.
     */
    err = sf_crypto_cipher_aes_validate_init_params_iv(p_ctrl,
                                                       p_aes_params);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Validate parameters / lengths specific to GCM mode */
    if (SF_CRYPTO_CIPHER_MODE_GCM == (p_ctrl->cipher_chaining_mode))
    {
        err = sf_crypto_cipher_aes_validate_init_gcm_params(p_aes_params);
    }
    return err;
} /* End of function sf_crypto_cipher_validate_aes_init_params */

/*******************************************************************************************************************//**
 * @brief   Subroutine to init the AES GCM mode of operation with parameters from the AES cipherInit operation.
 *
 * @param[in, out]  p_ctrl                      Pointer to the cipher framework module control block used
 *                                              in the open() call.
 * @param[in]       cipher_operation_mode       The cipher operation mode - encrypt / decrypt.
 * @param[in]       p_key                       Pointer to the key to be used for the cipher operations.
 * @param[in]       p_algorithm_specific_params Pointer to the algorithm specific parameters.
 * @retval          SSP_SUCCESS                 All of the input parameters are validated successfully.
 * @return                                      See @ref Common_Error_Codes or HAL driver for other possible
 *                                              return codes or causes. This function calls:
 *                                              aes_api_t.open()
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_init_gcm_params (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                       sf_crypto_cipher_op_mode_t cipher_operation_mode,
                                                       sf_crypto_key_t const * const p_key,
                                                       sf_crypto_cipher_algorithm_init_params_t * p_algorithm_specific_params)
{
    /**  These unused common params are already handled in the calling routine*/
    SSP_PARAMETER_NOT_USED (cipher_operation_mode);
    SSP_PARAMETER_NOT_USED (p_key);

    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_aes_init_params_t * p_aes_params =
            (sf_crypto_cipher_aes_init_params_t *)p_algorithm_specific_params;
    aes_cfg_t   aes_hal_cfg;
    uint32_t err_hal_crypto;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)(p_ctrl->p_cipher_context_buffer);

    if ((uint32_t)SF_CRYPTO_CIPHER_AES_IV_LENGTH_12_BYTES == (uint32_t)(p_context->working_iv_length))
    {
        memcpy (((p_context->working_iv) + (p_context->working_iv_length)),
                gcm_iv_padding,
                sizeof (gcm_iv_padding));
        p_context->working_iv_length += sizeof (gcm_iv_padding);
    }

    if (SF_CRYPTO_CIPHER_OP_MODE_ENCRYPT == p_context->operation_mode)
    {
        /** Store the tag address supplied by the user, in the context buffer.  */
        p_context->user_gcm_auth_tag = (p_aes_params->p_auth_tag);
        p_context->gcm_auth_tag_length = SF_CRYPTO_CIPHER_AES_GCM_TAG_LENGTH_16_BYTES;
    }
    else
    {

        memcpy(p_context->gcm_auth_tag,
               p_aes_params->p_auth_tag->p_data,
               p_aes_params->p_auth_tag->data_length);
        p_context->gcm_auth_tag_length = p_aes_params->p_auth_tag->data_length;
    }
    /** For GCM, the work buffer in the HAL has to be initialized and
     * that is done through the open call because there is no HW
     * interaction and only the work buffer in the control block is cleaned up.
     */
    aes_hal_cfg.p_crypto_api = ((crypto_instance_t *)p_ctrl->p_lower_lvl_fwk_common_ctrl->p_lower_lvl_crypto)->p_api;
    err_hal_crypto = ((aes_api_t *)p_ctrl->p_hal_api)->open((aes_ctrl_t *)p_ctrl->p_hal_ctrl, &aes_hal_cfg);
    if (SSP_SUCCESS != err_hal_crypto)
    {
        err = (ssp_err_t)err_hal_crypto;
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief   Subroutine to initialize the AES cipher operation.
 *
 * @param[in, out]  p_ctrl                      Pointer to the cipher framework module control block used
 *                                              in the open() call.
 * @param[in]       cipher_operation_mode       The cipher operation mode - encrypt / decrypt.
 * @param[in]       p_key                       Pointer to the key to be used for the cipher operations.
 * @param[in]       p_algorithm_specific_params Pointer to the algorithm specific parameters.
 *
 * @retval          SSP_SUCCESS                 All of the input parameters are validated successfully.
 * @retval          SSP_ERR_INVALID_ARGUMENT    An input for the required cipher operation is invalid.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_cipher_aes_init (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                     sf_crypto_cipher_op_mode_t cipher_operation_mode,
                                     sf_crypto_key_t const * const p_key,
                                     sf_crypto_cipher_algorithm_init_params_t * p_algorithm_specific_params)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_aes_init_params_t * p_aes_params =
            (sf_crypto_cipher_aes_init_params_t *)p_algorithm_specific_params;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)(p_ctrl->p_cipher_context_buffer);


    err = sf_crypto_cipher_aes_validate_init_params(p_ctrl,
                                                    cipher_operation_mode,
                                                    p_key,
                                                    p_aes_params);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** All buffers within the context buffer are allocated at Open. Now just zeroise their contents. */
    sf_crypto_cipher_aes_context_buffer_pointer_data_zeroise(p_ctrl);

    /** Copy the init parameters to the context buffer */
    p_context->operation_mode = cipher_operation_mode;

    memcpy(p_context->cipher_key,
           p_key->p_data,
           p_key->data_length);
    p_context->cipher_key_length = p_key->data_length;

    p_context->padding_scheme = p_aes_params->padding_scheme;

    if ((NULL != p_aes_params) &&
        (NULL != p_aes_params->p_iv) &&
        (NULL != p_aes_params->p_iv->p_data) &&
        (p_aes_params->p_iv->data_length))
    {

        memcpy(p_context->working_iv,
               p_aes_params->p_iv->p_data,
               p_aes_params->p_iv->data_length);
        p_context->working_iv_length =  p_aes_params->p_iv->data_length;
    }


    if (SF_CRYPTO_CIPHER_MODE_GCM == p_ctrl->cipher_chaining_mode)
    {

        err = sf_crypto_cipher_aes_init_gcm_params(p_ctrl,
                                                   cipher_operation_mode,
                                                   p_key,
                                                   p_algorithm_specific_params);

    }

    return err;
} /* End of function sf_crypto_cipher_aes_init */




/*******************************************************************************************************************//**
 * @brief   Subroutine to fill partial data block in the context buffer for the AES cipher operation.
 * First checks if there is any data in the partial block. Only if there is any data, fills the partial block
 * with the data from the input.
 * @param[in, out]  p_ctrl          Pointer to the cipher framework module control block used
 *                                  in the open() call.
 * @param[in, out]  fill_bytes      Pointer to the number of bytes required to fill the buffer.
 *                                  On output, contains the number of bytes used.
 * @param[in]       p_data_in       Pointer to input data output from the cipher operation.
 **********************************************************************************************************************/
static void sf_crypto_cipher_aes_check_fill_partial_block (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                           uint32_t * fill_bytes,
                                                           sf_crypto_data_handle_t const * const p_data_in)
{
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    *fill_bytes = 0U;

    /** Check if there is data in the partial block buffer. If so fill it from the input data */
    if (p_context->partial_bytes_buffered)
    {
       /** Calculate the number of bytes to fill the partial block. */
        *fill_bytes = (uint32_t)(uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES - p_context->partial_bytes_buffered;

       if (p_data_in->data_length <= (*fill_bytes))
       {
           *fill_bytes = (uint32_t)p_data_in->data_length;
       }

       memcpy((p_context->partial_block + (uint32_t)p_context->partial_bytes_buffered),
              p_data_in->p_data,
              *fill_bytes);

       /** Update the size of the data in the partial block. */
       p_context->partial_bytes_buffered += *fill_bytes;
    }
}

/*******************************************************************************************************************//**
 * @brief   Subroutine to fill any AES partial block for the AES cipher operation. Fills the buffer only if there is
 *          any data present in the buffer.
 *
 * @param[in]       p_fill_buffer           Pointer to the cbuffer to be filled.
 * @param[in, out]  p_num_bytes_in_buffer   Pointer to the number of bytes already present in the buffer.
 * @param[in, out]  fill_bytes              Pointer to the number of bytes required to fill the buffer.
 * @param[in]       p_data_in               Pointer to data handle of the input data from the user.
 **********************************************************************************************************************/
static void sf_crypto_cipher_aes_fill_partial_block(uint8_t * p_fill_buffer,
                                                    uint32_t * p_num_bytes_in_buffer,
                                                    uint32_t * fill_bytes,
                                                    sf_crypto_data_handle_t const * const p_data_in)
{
    /* Check if there is data in the partial block buffer. If so fill it from the input data */
    if (*p_num_bytes_in_buffer)
    {
       /** Calculate the number of bytes to fill the buffer. */
        *fill_bytes = (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES - (*p_num_bytes_in_buffer);

       if (p_data_in->data_length < *fill_bytes)
       {
           *fill_bytes = p_data_in->data_length;
       }

       memcpy(p_fill_buffer + *p_num_bytes_in_buffer,
              p_data_in->p_data,
              *fill_bytes);

       /** Update the size of the data in the partial block. */
       *p_num_bytes_in_buffer += *fill_bytes;
    }
}

/*******************************************************************************************************************//**
 * @brief   Subroutine to allocate memory for GCM parameters in the context buffer.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 *
 * @retval          SSP_SUCCESS             Memory was alocated successfully.
 * @retval          SSP_ERR_TIMEOUT         Was unable to allocate the memory within the specified time to wait.
 * @retval          SSP_ERR_OUT_OF_MEMORY   Requested size is zero or larger than the pool.
 * @retval          SSP_ERR_INTERNAL        RTOS service returned a unexpected error.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_instance_gcm_memory_allocate (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_instance_ctrl_t * p_ctrl_common = p_ctrl->p_lower_lvl_fwk_common_ctrl;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t * )p_ctrl->p_cipher_context_buffer;

    p_context->gcm_auth_tag_buffer_size = SF_CRYPTO_CIPHER_AES_GCM_TAG_LENGTH_16_BYTES;
    p_context->gcm_auth_tag_length = SF_CRYPTO_CIPHER_AES_GCM_TAG_LENGTH_16_BYTES;
    p_context->partial_aad_buffer_size = (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES;

    /** Allocate memory for GCM tag in the context buffer.
     * Extra 3 bytes are allocated to get the buffer WORD aligned
     */
    err = sf_crypto_memory_allocate(&p_ctrl_common->byte_pool,
                                    (void **)&p_context->gcm_auth_tag,
                                    p_context->gcm_auth_tag_buffer_size,
                                    p_ctrl_common->wait_option);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Allocate memory for partial AAD in the context buffer.
     * Extra 3 bytes are allocated to get the buffer WORD aligned
     */
    err = sf_crypto_memory_allocate(&p_ctrl_common->byte_pool,
                                     (void **)&p_context->partial_aad_block,
                                     p_context->partial_aad_buffer_size,
                                     p_ctrl_common->wait_option);
    if (SSP_SUCCESS != err)
    {
        /** Release the memory allocated for GCM Tag */
        sf_crypto_memory_release (p_context->gcm_auth_tag,
                                  p_context->gcm_auth_tag_buffer_size);
        return err;
    }
    return err;
}


/*******************************************************************************************************************//**
 * @brief   Subroutine to update check and update AAD  for cipher AES operation.
 * If the mode is GCM and there is any partial AAD not processed yet, it will be processed.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.

 * @retval          SSP_SUCCESS             AAD check / operation was completed successfully.
 * @return                                  See @ref Common_Error_Codes or HAL driver for other possible return codes
 *                                          or causes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_update_helper_gcm_aad_update (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t hal_err = (uint32_t)SSP_SUCCESS;
    sf_crypto_cipher_aes_context_t *p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    if ((SF_CRYPTO_CIPHER_MODE_GCM == (sf_crypto_cipher_mode_t)p_ctrl->cipher_chaining_mode) &&
        (SF_CRYPTO_CIPHER_STATE_INITIALIZED == p_ctrl->status) &&
        (p_context->partial_aad_length > 0U))
    {
        hal_err = sf_crypto_cipher_aes_process_aad_partial_block(p_ctrl);
        if ((uint32_t)SSP_SUCCESS != hal_err)
        {
            return (ssp_err_t)hal_err;
        }
        /** Now that all the data in the partial block is processed, clean it up and set the size to 0 */
        memset(p_context->partial_aad_block, 0U, (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES);
        p_context->partial_aad_length = 0U;
    }

    return err;
}
/*******************************************************************************************************************//**
 * @brief   Subroutine to update the cipher AES operation.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 * @param[in, out]  p_data_in               Pointer to the input data structure - has the pointer to
 *                                          input data and the data length
 **@param[in, out]  p_data_out              Pointer to the output data structure - has the pointer to
 *                                          output data and the data length.

 * @retval          SSP_SUCCESS             Cipher operation was updated successfully.
 * @retval          SSP_ERR_INVALID_SIZE    The out buffer is inadequate to hold the output data.
 * @return                                  See @ref Common_Error_Codes or HAL driver for other possible return codes
 *                                          or causes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_cipher_aes_update (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                       sf_crypto_data_handle_t const * const p_data_in,
                                       sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t input_length = p_data_in->data_length;
    uint32_t remaining_length = 0U;
    uint32_t fill_bytes_needed = 0U;
    uint32_t part_out_bytes = 0U;        // Partial output written to output buffer.
    uint32_t num_blocks = 0U;

    sf_crypto_cipher_aes_context_t *p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;
    err = sf_crypto_cipher_aes_check_update_params(p_ctrl, p_data_in, p_data_out);
    if (SSP_SUCCESS != err)
    {
        return err;
    }


    /** If GCM check if there is any partial AAD remaining.
     * This is to be processed only if the cipher is in initialized state.
     * Once the update is done AAD calculations should not be done.
     * if so call to update AAD */

    err = sf_crypto_cipher_aes_update_helper_gcm_aad_update(p_ctrl);
    if (SSP_SUCCESS != err)
    {
        return err;
    }


    /** Set the data out length before starting cipher operation. */
    p_data_out->data_length = 0U;
    remaining_length = input_length;

    /* Check if there is data in the partial block buffer. If so first fill it from the input data */
    sf_crypto_cipher_aes_check_fill_partial_block(p_ctrl,
                                                  &fill_bytes_needed,
                                                  p_data_in);

    /** Update the number of bytes remaining in the input buffer to be processed. */
     remaining_length -= fill_bytes_needed;

    /** If data is block length, process it. That is encrypt / decrypt it. */
    if ((uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES == (uint32_t)(p_context->partial_bytes_buffered))
    {
        err = sf_crypto_cipher_aes_process_partial_block(p_ctrl, p_data_out);
        if (SSP_SUCCESS != err)
        {
            return err;
        }

        /* Since the partial block was processed successfully, it means that the block sized data was output.
         * update the number of bytes written to the output buffer.
         * */
         part_out_bytes = (uint32_t)(uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES;
         p_data_out->data_length += part_out_bytes;

         /* Now that all the data in the partial block is processed, clean it up and set the size to 0 */
         memset(p_context->partial_block, 0U, (uint32_t)(uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES);
         p_context->partial_bytes_buffered = 0U;
    }

    /** Check if the remaining input data is a multiple of block size */
    num_blocks = remaining_length / (uint32_t)(uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES;

    if (num_blocks)
    {
        err = sf_crypto_cipher_aes_hal_encrypt_decrypt(p_ctrl,
                                                       (uint32_t *)p_context->cipher_key,
                                                       (uint32_t *)p_context->working_iv,
                                                       (uint32_t)(num_blocks *
                                                                 (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_IN_WORDS),
                                                       (uint32_t *)p_data_in->p_data + fill_bytes_needed,
                                                       (uint32_t *)p_data_out->p_data + part_out_bytes);
        if (SSP_SUCCESS != err)
        {
            return err;
        }

        /** Update the number of bytes written to the output buffer. */
        part_out_bytes += (num_blocks * (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES);
        p_data_out->data_length += part_out_bytes;

        /** Update the number of bytes remaining in the input buffer to be processed. */
        remaining_length -= (num_blocks * (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES);
    }

    /** If any bytes remain that is less than the block size, copy to the partial buffer */
    if (remaining_length)
    {
        memcpy(p_context->partial_block,
               p_data_in->p_data + (p_data_in->data_length - remaining_length),
               remaining_length);

        /** Update the size of the data in the partial block */
        p_context->partial_bytes_buffered += remaining_length;
    }
    return err;
} /* End of function sf_crypto_cipher_aes_update */

/*******************************************************************************************************************//**
 * @brief   Subroutine to check the parameters for the cipherUpdate operation.
 *
 * @param[in, out]  p_ctrl                      Pointer to the cipher framework module control block used
 *                                              in the open() call.
 * @param[in]       p_data_in                   Pointer to the data handle of the input data
 * @param[in]       p_data_out                  Pointer to the data handle of the output data
 * @retval          SSP_SUCCESS                 Parameters were validated successfully.
 * @retval          SSP_ERR_INVALID_ARGUMENT    At least one of the buffer is set to NULL.
 * @retval          SSP_ERR_INVALID_SIZE        The output buffer sizes is invalid.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_check_update_params (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                           sf_crypto_data_handle_t const * const p_data_in,
                                                           sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t possible_output_bytes = 0U;
    uint32_t input_length = p_data_in->data_length;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    /** Check if the output buffer length is sufficient to hold the cipher text. */
    possible_output_bytes = ((((uint32_t)p_context->partial_bytes_buffered + input_length) /
                               (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES) *
                               (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES);

    if (p_data_out->data_length < possible_output_bytes)
    {
        return SSP_ERR_INVALID_SIZE;
    }

    if ((possible_output_bytes) && (NULL == p_data_out->p_data))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    /** validate the context buffer params too. */
    err = sf_crypto_cipher_aes_validate_update_params_context(p_ctrl);
    return err;
}

/*******************************************************************************************************************//**
 * @brief   Subroutine to validate the context buffer parameters for the cipherUpdate operation.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.

 * @retval          SSP_SUCCESS             Parameters were validated successfully.
 * @retval          SSP_ERR_ASSERTION       At least one of the buffer is set to NULL.
 * @return                                  See @ref Common_Error_Codes or HAL driver for other possible return codes
 *                                          or causes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_validate_update_params_context (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    if ((SF_CRYPTO_CIPHER_MODE_ECB != p_ctrl->cipher_chaining_mode) &&
        ((NULL == p_context->working_iv) ||
        ((uint32_t)SF_CRYPTO_CIPHER_AES_IV_LENGTH_16_BYTES >  (uint32_t)(p_context->working_iv_length))))
    {
        return SSP_ERR_INTERNAL;
    }

    if (SF_CRYPTO_CIPHER_MODE_GCM == p_ctrl->cipher_chaining_mode)
    {
        if ((NULL == p_context->gcm_auth_tag) ||
           ((uint32_t)SF_CRYPTO_CIPHER_AES_GCM_TAG_LENGTH_16_BYTES >  (uint32_t)(p_context->gcm_auth_tag_length)))
        {
            return SSP_ERR_INTERNAL;
        }
        if (SF_CRYPTO_CIPHER_OP_MODE_ENCRYPT == p_context->operation_mode)
        {
            if (NULL == p_context->user_gcm_auth_tag)
            {
                return SSP_ERR_INTERNAL;
            }
        }
    }
    return err;
} /* End of function sf_crypto_cipher_aes_validate_update_params */

/*******************************************************************************************************************//**
 * @brief   Subroutine to validate the context buffer parameters for the cipherAadUpdate operation.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.

 * @retval          SSP_SUCCESS             Parameters were validated successfully.
 * @retval          SSP_ERR_ASSERTION       At least one of the buffer is set to NULL.
 * @return                                  See @ref Common_Error_Codes or HAL driver for other possible return codes
 *                                          or causes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_cipher_aes_validate_aad_update_params_context (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    if ((NULL == p_context->partial_aad_block) ||
         (0U == p_context->partial_aad_buffer_size))
     {
         return SSP_ERR_INTERNAL;
     }
    return err;
} /* End of function sf_crypto_cipher_aes_validate_aad_update_params_context */

/*******************************************************************************************************************//**
 * @brief   Subroutine to update the AAD for the cipher operation.
 *
 * @param[in, out]  p_ctrl      Pointer to the cipher framework module control block used
 *                              in the open() call.
 * @param[in]       p_aad       Pointer to the input data structure - has the pointer to
 *                              input AAD and the data length
 *
 * @retval          SSP_SUCCESS AAD was updated successfully.
 * @return                      See @ref Common_Error_Codes or HAL driver for other possible return codes or causes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_cipher_aes_aad_update (sf_crypto_cipher_instance_ctrl_t * p_ctrl,
                                           sf_crypto_data_handle_t const * const p_aad)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;
    uint32_t hal_err = (uint32_t)SSP_SUCCESS;
    uint32_t remaining_length = p_aad->data_length;
    uint32_t fill_bytes_needed = 0;
    uint32_t num_blocks = 0;
    aes_api_t * aes_hal_api = (aes_api_t *)(p_ctrl->p_hal_api);

    /** Check if there is data in the aad block buffer. If so first fill it from the input data */
    sf_crypto_cipher_aes_fill_partial_block(p_context->partial_aad_block,
                                            &p_context->partial_aad_length,
                                            &fill_bytes_needed,
                                            p_aad);

    /** Update the number of bytes remaining in the input buffer to be processed. */
    remaining_length -= fill_bytes_needed;

    /** If data is block length, encrypt it. */
    if ((uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES == (uint32_t)(p_context->partial_aad_length))
    {
        hal_err = sf_crypto_cipher_aes_process_aad_partial_block(p_ctrl);
        if (hal_err)
        {
            return (ssp_err_t)(hal_err);
        }

        /* Now that all the data in the partial block is processed, clean it up and set the size to 0 */
        memset(p_context->partial_aad_block, 0U, (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES);
        p_context->partial_aad_length = 0U;
    }

    /** Check if the input data is a multiple of block size */
    num_blocks = remaining_length / (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES;

    if (num_blocks)
    {
        if (SF_CRYPTO_CIPHER_OP_MODE_ENCRYPT == p_context->operation_mode)
        {
            hal_err = aes_hal_api->zeroPaddingEncrypt(((aes_ctrl_t *)(p_ctrl->p_hal_ctrl)),
                                                       (uint32_t *)p_context->cipher_key,
                                                       (uint32_t *)p_context->working_iv,
                                                       (uint32_t)num_blocks *
                                                       (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES,
                                                       (uint32_t *)p_aad->p_data + fill_bytes_needed,
                                                       NULL);
        }
        else
        {
            hal_err = aes_hal_api->zeroPaddingDecrypt((aes_ctrl_t *)(p_ctrl->p_hal_ctrl),
                                                       (uint32_t *)p_context->cipher_key,
                                                       (uint32_t *)p_context->working_iv,
                                                       num_blocks *
                                                       (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES,
                                                       (uint32_t *)p_aad->p_data + fill_bytes_needed,
                                                       NULL);
        }

        if ((uint32_t)SSP_SUCCESS != hal_err)
        {
            return (ssp_err_t)hal_err;
        }

        /** Update the number of bytes remaining in the input buffer to be processed. */
        remaining_length -= (num_blocks * (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES);
    }

    /** If any bytes remain that is less than the block size, copy to the partial buffer */
    if (remaining_length)
    {
        memcpy(p_context->partial_aad_block,
               p_aad->p_data + (p_aad->data_length - remaining_length),
               remaining_length);

        /** Update the size of the data in the partial block */
        p_context->partial_aad_length = remaining_length;
    }

    return err;
} /* End of function sf_crypto_cipher_aes_aad_update */

/*******************************************************************************************************************//**
 * @brief   Subroutine to update the partial block of AAD for the cipher operation.
 * Any partial data in the context buffer will be zero padded internally and updated.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.

 * @retval          SSP_SUCCESS             AAD was updated successfully.
 * @return                                  See @ref Common_Error_Codes or HAL driver for other possible return codes
 *                                          or causes.
 **********************************************************************************************************************/
static uint32_t sf_crypto_cipher_aes_process_aad_partial_block (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    uint32_t hal_err = (uint32_t)SSP_SUCCESS;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    if (SF_CRYPTO_CIPHER_OP_MODE_ENCRYPT == p_context->operation_mode)
    {
        hal_err = ((aes_api_t *)p_ctrl->p_hal_api)->zeroPaddingEncrypt((aes_ctrl_t *)(p_ctrl->p_hal_ctrl),
                                                                       (uint32_t *)p_context->cipher_key,
                                                                       (uint32_t *)p_context->working_iv,
                                                                       p_context->partial_aad_length,
                                                                       (uint32_t *)p_context->partial_aad_block,
                                                                       NULL);
    }
    else
    {
        hal_err = ((aes_api_t *)p_ctrl->p_hal_api)->zeroPaddingDecrypt((aes_ctrl_t *)(p_ctrl->p_hal_ctrl),
                                                                       (uint32_t *)p_context->cipher_key,
                                                                       (uint32_t *)p_context->working_iv,
                                                                       p_context->partial_aad_length,
                                                                       (uint32_t *)p_context->partial_aad_block,
                                                                       NULL);

    }
    return hal_err;
} /* End of function sf_crypto_cipher_process_aes_aad_partial_block */

/*******************************************************************************************************************//**
 * @brief   Subroutine to update the partial block of input data for the cipher operation.
 * This calls the lower level HAL APIs to process the data.
 *
 * @param[in, out]  p_ctrl                      Pointer to the cipher framework module control block used
 *                                              in the open() call.
 * @param[in]       p_data_out                  Pointer to data handle of the output data.
 * @retval          SSP_SUCCESS                 Data was processed successfully.
 * @return                                      See @ref Common_Error_Codes or HAL driver for other possible return
 *                                              codes or causes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_process_partial_block (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                             sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    err = sf_crypto_cipher_aes_hal_encrypt_decrypt(p_ctrl,
                                                   (uint32_t *)p_context->cipher_key,
                                                   (uint32_t *)p_context->working_iv,
                                                   (uint32_t)p_context->partial_bytes_buffered / NUM_BYTES_IN_WORD,
                                                   (uint32_t *)p_context->partial_block,
                                                   (uint32_t *)p_data_out->p_data);

    return err;
} /* End of function sf_crypto_cipher_process_aes_partial_block */

/*******************************************************************************************************************//**
 * @brief   Subroutine to process the partial block of input data for the AES GCM cipher operation.
 * This calls the lower level HAL APIs to process the data.
 *
 * @param[in, out]  p_ctrl                      Pointer to the cipher framework module control block used
 *                                              in the open() call.
 * @param[in]       p_data_out                  Pointer to data handle of the output data.
 * @retval          SSP_SUCCESS                 Data was processed successfully.
 * @return                                      See @ref Common_Error_Codes or HAL driver for other possible return
 *                                              codes or causes.
 **********************************************************************************************************************/
static uint32_t sf_crypto_cipher_aes_gcm_process_partial_block (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                sf_crypto_data_handle_t * const p_data_out)
{
    uint32_t hal_err = (uint32_t)SSP_SUCCESS;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;
    uint8_t temp_block_output[SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES] = {0};

    if (SF_CRYPTO_CIPHER_OP_MODE_ENCRYPT == (sf_crypto_cipher_op_mode_t)p_context->operation_mode)
    {
        hal_err = ((aes_api_t *)p_ctrl->p_hal_api)->zeroPaddingEncrypt((aes_ctrl_t *)(p_ctrl->p_hal_ctrl),
                                                                      (uint32_t *)p_context->cipher_key,
                                                                      (uint32_t *)p_context->working_iv,
                                                                      (uint32_t)p_context->partial_bytes_buffered,
                                                                      (uint32_t *)p_context->partial_block,
                                                                      (uint32_t *)&temp_block_output[0]);
    }
    else
    {
        hal_err = ((aes_api_t *)p_ctrl->p_hal_api)->zeroPaddingDecrypt((aes_ctrl_t *)(p_ctrl->p_hal_ctrl),
                                                                       (uint32_t *)p_context->cipher_key,
                                                                       (uint32_t *)p_context->working_iv,
                                                                       (uint32_t)p_context->partial_bytes_buffered,
                                                                       (uint32_t *)p_context->partial_block,
                                                                       (uint32_t *)&temp_block_output[0]);
    }


    if (SSP_SUCCESS == hal_err)
    {
        /* The source memory (temp_block_output) and the destination memory(p_data_out) are not overlapped but
         * LDRA reports that they are overlapped.
         * It is clear that there is no overlap so suppress the rule 480 S for the following code.
         * */
        /*LDRA_INSPECTED 480 S */
        memcpy((uint8_t *)p_data_out->p_data + p_data_out->data_length,
               (uint8_t *)&temp_block_output[0],
               (uint32_t)p_context->partial_bytes_buffered);
        p_data_out->data_length += (uint32_t)p_context->partial_bytes_buffered;
    }
    return hal_err;
} /* End of function sf_crypto_cipher_aes_gcm_process_partial_block */


/*******************************************************************************************************************//**
 * @brief   Subroutine to finalize the cipher encrypt operation.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 * @param[in, out]  p_data_in               Pointer to the input data structure - has the pointer to
 *                                          input data and the data length
 **@param[in, out]  p_data_out              Pointer to the output data structure - has the pointer to
 *                                          output data and the data length.

 * @retval          SSP_SUCCESS                 Cipher operation was finalized successfully.
 * @retval          SSP_ERR_INVALID_ARGUMENT    Input data is invalid.
 * @retval          SSP_ERR_INVALID_SIZE        The out buffer is inadequate to hold the output data.
 * @return                                      See @ref Common_Error_Codes or HAL driver for other possible return
 *                                              codes or causes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_cipher_aes_encrypt_final (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                              sf_crypto_data_handle_t const * const p_data_in,
                                              sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t hal_err = (uint32_t)SSP_SUCCESS;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    /** Validate the input parameters */
    err = sf_crypto_cipher_aes_validate_encrypt_final_params(p_ctrl, p_data_in, p_data_out);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Process the input data */
    err = sf_crypto_cipher_aes_update(p_ctrl, p_data_in, p_data_out);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

     /** Handle padding for ECB, CBC  modes */
    if ((SF_CRYPTO_CIPHER_MODE_GCM != p_ctrl->cipher_chaining_mode) &&
        (SF_CRYPTO_CIPHER_MODE_CTR != p_ctrl->cipher_chaining_mode) &&
        (SF_CRYPTO_CIPHER_MODE_XTS != p_ctrl->cipher_chaining_mode) &&
        (SF_CRYPTO_CIPHER_PADDING_SCHEME_NO_PADDING != p_context->padding_scheme))
    {
        /** Add padding and encrypt the last block. */
        err = sf_crypto_cipher_aes_handle_encrypt_padding(p_ctrl, p_data_out);
    }

    if (SF_CRYPTO_CIPHER_MODE_GCM == p_ctrl->cipher_chaining_mode)
    {
        /** If there is a remaining partial block process it. For GCM zero padding is automatic  */
        if  (p_context->partial_bytes_buffered)
        {
            hal_err = sf_crypto_cipher_aes_gcm_process_partial_block(p_ctrl, p_data_out);
            if ((uint32_t)SSP_SUCCESS != hal_err)
            {
                return (ssp_err_t)hal_err;
            }
        }

        /** Now that the possible AAD update and plain text encryption is done, get the GCM tag  */
        err = sf_crypto_cipher_aes_get_gcm_tag (p_ctrl);
    }

    return err;
} /* End of function sf_crypto_cipher_aes_encrypt_final */

/*******************************************************************************************************************//**
 * @brief   Subroutine to check if the total data length to be decrypted is a multiple of the AES block size.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 * @param[in, out]  p_data_in               Pointer to the input data structure - has the pointer to
 *                                          input data and the data length
 * @retval          SSP_SUCCESS                 Cipher operation was finalized successfully.
 * @retval          SSP_ERR_INVALID_ARGUMENT    Input data is invalid.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_decrypt_final_check_input_length (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                        sf_crypto_data_handle_t const * const p_data_in)
{
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    if (((p_context->partial_bytes_buffered) + (p_data_in->data_length)) %
         (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES)
    {
        /** If remaining length is not a multiple of block size, return error */
        return SSP_ERR_INVALID_ARGUMENT;
    }
    return SSP_SUCCESS;
}
/*******************************************************************************************************************//**
 * @brief   Subroutine to finalize the cipher decrypt operation.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 * @param[in, out]  p_data_in               Pointer to the input data structure - has the pointer to
 *                                          input data and the data length
 **@param[in, out]  p_data_out              Pointer to the output data structure - has the pointer to
 *                                          output data and the data length.

 * @retval          SSP_SUCCESS                 Cipher operation was finalized successfully.
 * @retval          SSP_ERR_INVALID_ARGUMENT    Input data is invalid.
 * @retval          SSP_ERR_INVALID_SIZE        The out buffer is inadequate to hold the output data.
 * @return                                      See @ref Common_Error_Codes or HAL driver for other possible return
 *                                              codes or causes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_cipher_aes_decrypt_final (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                              sf_crypto_data_handle_t const * const p_data_in,
                                              sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;

    /** Validate the input parameters */
    err = sf_crypto_cipher_aes_validate_decrypt_final_params(p_ctrl, p_data_in, p_data_out);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Process the input data */
    if (SF_CRYPTO_CIPHER_MODE_GCM != (sf_crypto_cipher_mode_t)(p_ctrl->cipher_chaining_mode))
    {
        err = sf_crypto_cipher_aes_decrypt_final_check_input_length(p_ctrl, p_data_in);
        if (SSP_SUCCESS != err)
        {
          return err;
        }
        err = sf_crypto_cipher_aes_helper_decrypt_final(p_ctrl,
                                                        p_data_in,
                                                        p_data_out);
    }
    else
    {
        /** If GCM  check if there is any partial AAD remaining, if so call to update AAD */
        err = sf_crypto_cipher_aes_update_helper_gcm_aad_update(p_ctrl);
        if (SSP_SUCCESS != err)
        {
            return err;
        }

        /** If GCM  set the tag */
        err = sf_crypto_cipher_aes_set_gcm_tag(p_ctrl);
        if (SSP_SUCCESS != err)
        {
            return err;
        }

        err =  sf_crypto_cipher_aes_gcm_helper_decrypt_final (p_ctrl,
                                                              p_data_in,
                                                              p_data_out);
        if (SSP_SUCCESS != err)
        {
            return err;
        }

        /** For GCM  compute and verify the tag - return error to indicate if the data is valid or not. */
        err = sf_crypto_cipher_aes_compute_verify_gcm_tag(p_ctrl);
    }

    return err;
} /* End of function sf_crypto_cipher_aes_decrypt_final */

/*******************************************************************************************************************//**
 * @brief   Subroutine to check whether the the input is the last block for the cipher decrypt operation.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 * @param[in]       remaining_length        Remaining bytes in the input data to be decrypted.
 *
 * @retval          true                    Yes, it is the last block.
 * @retval          false                   No, it is not the last block.
 **********************************************************************************************************************/
static bool sf_crypto_cipher_aes_is_last_decrypt_block (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                        uint32_t remaining_length)
{
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;
    bool last_block = false;
    if ((remaining_length + p_context->partial_bytes_buffered) <= (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES)
    {
        last_block = true;
    }
    return last_block;
}

/*******************************************************************************************************************//**
 * @brief   Subroutine to decrypt the last block of the input data for the cipher decrypt operation.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 * @param[in]       p_input_data            Pointer to the the input data buffer.
 * @param[in, out]  p_output_buffer         Pointer to the output data buffer.
 * @param[in, out]  num_data_bytes          Length of the actual data (block minus the padding) is populated on
 *                                          successful output.
 * @retval          SSP_SUCCESS                 Cipher operation was finalized successfully.
 * @retval          SSP_ERR_INVALID_ARGUMENT    Input data is invalid.
 * @retval          SSP_ERR_INVALID_SIZE        The out buffer is inadequate to hold the output data.
 * @return                                      See @ref Common_Error_Codes or HAL driver for other possible return
 *                                              codes or causes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_decrypt_last_block (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                           uint8_t * p_input_data,
                                                          uint8_t * p_output_buffer,
                                                          uint32_t * num_data_bytes)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t hal_err = SSP_SUCCESS;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    /** Decrypt the last block into the buffer provided.*/

    hal_err = ((aes_api_t *)p_ctrl->p_hal_api)->decrypt((aes_ctrl_t *)(p_ctrl->p_hal_ctrl),
                                                         (uint32_t *)p_context->cipher_key,
                                                         (uint32_t *)p_context->working_iv,
                                                         (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_IN_WORDS,
                                                         (uint32_t *)p_input_data,
                                                         (uint32_t *)p_output_buffer);
    if ((uint32_t)SSP_SUCCESS != hal_err)
    {
        return (ssp_err_t)hal_err;
    }

    /** Strip the padding bytes */
    /** Pick the last byte which is the pad byte*/
    uint8_t pad_byte = (uint8_t)(*(p_output_buffer + ((uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES - 0x1U)));
    uint8_t * buffer_end = (uint8_t *)(((uint32_t)p_output_buffer + (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES) - 0x1U);

    /** Check backwards to ensure the number of pad_bytes is the real padding  */
    for (uint8_t i = 0U; i < pad_byte;  i++)
    {
        if (((uint8_t)(*(buffer_end - (uint32_t)i))) != pad_byte)
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }
    }

    *num_data_bytes = (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES - (uint32_t)pad_byte;

    return err;
} /* End of function sf_crypto_cipher_aes_decrypt_last_block */

/*******************************************************************************************************************//**
 * @brief   helper function to finalize the cipher decrypt operation. It decrypts the last block of data
 *          strips the padding bytes and writes the actual data to the output data buffer at the given offset.
 *          Also updates the data length on output.
 *
 * @param[in, out]  p_ctrl              Pointer to the cipher framework module instance control block.
 * @param[in]       input_data_length   Length of the input data
 * @param[in]       p_input_data        Pointer to the input data buffer.
 * @param[in, out]  p_data_out          Pointer to the output data structure - has the pointer to
 *                                      output data and the data length on input.
 * @param[in]       data_out_offset     Offset at which the data has to be output into p_data_out.p_data.
 *
 * @retval          SSP_SUCCESS                 Cipher operation was finalized successfully.
 * @retval          SSP_ERR_INVALID_ARGUMENT    Input data is invalid.
 * @retval          SSP_ERR_INVALID_SIZE        The out buffer is inadequate to hold the output data.
 * @return                                      See @ref Common_Error_Codes or HAL driver for other possible return
 *                                              codes or causes.
 **********************************************************************************************************************/

static ssp_err_t sf_crypto_cipher_aes_helper_decrypt_last_block (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                 uint32_t input_data_length,
                                                                 uint8_t * p_input_data,
                                                                 sf_crypto_data_handle_t * const p_data_out,
                                                                 uint32_t data_out_offset)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t num_data_bytes = 0U;
    uint8_t temp_buffer[(uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES] ={0};

    if (SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES != input_data_length)
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    err = sf_crypto_cipher_aes_decrypt_last_block(p_ctrl,
                                                  p_input_data,
                                                  &temp_buffer[0],
                                                  &num_data_bytes);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    memcpy (p_data_out->p_data + data_out_offset,
            &temp_buffer[0],
            num_data_bytes);
    p_data_out->data_length += num_data_bytes;

    return err;
}

/*******************************************************************************************************************//**
 * @brief   helper function to finalize the cipher decrypt operation.
 * This checks if it is the last block to be decrypted and if padding is enabled.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 * @param[in]       remaining_input_length  The number of bytes remaining to be processed in the input data.
 * @retval          padded_block            true if the remaining length is the last block and is padded
 *                                          false otherwise.
 **********************************************************************************************************************/
static bool sf_crypto_cipher_is_last_block_with_padding (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                         uint32_t remaining_input_length)
{
    bool padded_block = false;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    if ((sf_crypto_cipher_aes_is_last_decrypt_block(p_ctrl,remaining_input_length)) &&
        ((sf_crypto_cipher_padding_scheme_t)SF_CRYPTO_CIPHER_PADDING_SCHEME_NO_PADDING !=
         (sf_crypto_cipher_padding_scheme_t)(p_context->padding_scheme)))
    {
        padded_block = true;
    }

    return padded_block;
}

/*******************************************************************************************************************//**
 * @brief   helper function to finalize the cipher decrypt operation.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 * @param[in, out]  p_data_in               Pointer to the input data structure - has the pointer to
 *                                          input data and the data length
 **@param[in, out]  p_data_out              Pointer to the output data structure - has the pointer to
 *                                          output data and the data length.

 * @retval          SSP_SUCCESS                 Cipher operation was finalized successfully.
 * @retval          SSP_ERR_INVALID_ARGUMENT    Input data is invalid.
 * @retval          SSP_ERR_INVALID_SIZE        The out buffer is inadequate to hold the output data.
 * @return                                      See @ref Common_Error_Codes or HAL driver for other possible return
 *                                              codes or causes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_helper_decrypt_final (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                            sf_crypto_data_handle_t const * const p_data_in,
                                                            sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t hal_err = (uint32_t)SSP_SUCCESS;
    uint32_t remaining_length = (uint32_t)(p_data_in->data_length);
    uint32_t fill_bytes_needed = 0U;
    uint32_t part_out_bytes = 0U;                    // Partial block written to output buffer.
    uint32_t num_blocks = 0U;
    uint32_t advance_in_data = 0U;
    uint32_t operation_data_length = 0U;

    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    /** This will be updated once the data is written to the buffer. */
    p_data_out->data_length = 0U;

    /** Check if there is data in the partial block buffer. If so first fill it from the input data */
    sf_crypto_cipher_aes_check_fill_partial_block(p_ctrl,
                                                  &fill_bytes_needed,
                                                  p_data_in);

    /** Update the number of bytes remaining in the input buffer to be processed. */
     remaining_length -= fill_bytes_needed;
     advance_in_data = fill_bytes_needed;

    /** If data is block length within the partial block in the context buffer, then decrypt it. */
    if ((uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES == (uint32_t)(p_context->partial_bytes_buffered))
    {
        if (sf_crypto_cipher_is_last_block_with_padding(p_ctrl, remaining_length))
        {
            err = sf_crypto_cipher_aes_helper_decrypt_last_block(p_ctrl,
                                                                 (p_context->partial_bytes_buffered),
                                                                 p_context->partial_block,
                                                                 p_data_out,
                                                                 part_out_bytes);
            return err;  /** This is last block with padding buffered in the context buffer and we are done,so return */

        }
        else
        {
            err = sf_crypto_cipher_aes_process_partial_block(p_ctrl, p_data_out);
            if (SSP_SUCCESS != err)
            {
                return err;
            }

            /** update the number of bytes written to the output buffer. */
            part_out_bytes = (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES;
            p_data_out->data_length += (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES;

            /** Now that all the data in the partial block is processed, clean it up and set the size to 0 */
            memset(p_context->partial_block, 0U, (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES);
            p_context->partial_bytes_buffered = 0U;
        }
    }

    /** Check if there is remaining input data */
    if (remaining_length == 0U)
    {
        return err;     /** no more data to process, so return .*/
    }

    num_blocks = remaining_length / (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES;
    operation_data_length = num_blocks * (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES;
    if (SF_CRYPTO_CIPHER_PADDING_SCHEME_NO_PADDING != (sf_crypto_cipher_padding_scheme_t)p_context->padding_scheme)
    {
        if (sf_crypto_cipher_is_last_block_with_padding(p_ctrl, remaining_length))
        {
            err = sf_crypto_cipher_aes_helper_decrypt_last_block(p_ctrl,
                                                                 operation_data_length,
                                                                 p_data_in->p_data + advance_in_data,
                                                                 p_data_out,
                                                                 part_out_bytes);
            return err; /** This is last block with padding from the input data stream and we are done,so return */

        }
        /** There are multiple blocks to be decrypted but the last one is padded.
         * So decrypt all but the last one. */
        operation_data_length = (num_blocks- (1U)) * (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES;
    }

    /** Process the input data stream with num blocks */
    hal_err = ((aes_api_t *)p_ctrl->p_hal_api)->decrypt((aes_ctrl_t *)(p_ctrl->p_hal_ctrl),
                                                        (uint32_t *)p_context->cipher_key,
                                                        (uint32_t *)p_context->working_iv,
                                                        (uint32_t )operation_data_length / NUM_BYTES_IN_WORD,
                                                        (uint32_t *)p_data_in->p_data + advance_in_data,
                                                        (uint32_t *)p_data_out->p_data + part_out_bytes);
    if ((uint32_t)SSP_SUCCESS != hal_err)
    {
        return (ssp_err_t)hal_err;
    }

    advance_in_data += operation_data_length;

    /** update the number of bytes written to the output buffer. */
    part_out_bytes += operation_data_length;
    p_data_out->data_length += part_out_bytes;

    /** Update the number of bytes remaining in the input buffer to be processed. */
    remaining_length -= operation_data_length;

    if (sf_crypto_cipher_is_last_block_with_padding(p_ctrl, remaining_length))
    {
        err = sf_crypto_cipher_aes_helper_decrypt_last_block(p_ctrl,
                                                             SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES,
                                                             p_data_in->p_data + advance_in_data,
                                                             p_data_out,
                                                             part_out_bytes);
        return err; /** This is last block with padding from the input data stream and we are done,so return */
    }

    return err;
} /* End of function sf_crypto_cipher_aes_helper_decrypt_final */



/*******************************************************************************************************************//**
 * @brief   helper function to finalize the AES cipher decrypt operation for GCM .
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 * @param[in, out]  p_data_in               Pointer to the input data structure - has the pointer to
 *                                          input data and the data length
 **@param[in, out]  p_data_out              Pointer to the output data structure - has the pointer to
 *                                          output data and the data length.

 * @retval          SSP_SUCCESS                 Cipher operation was finalized successfully.
 * @retval          SSP_ERR_INVALID_ARGUMENT    Input data is invalid.
 * @retval          SSP_ERR_INVALID_SIZE        The out buffer is inadequate to hold the output data.
 * @return                                      See @ref Common_Error_Codes or HAL driver for other possible return
 *                                              codes or causes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_gcm_helper_decrypt_final (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                sf_crypto_data_handle_t const * const p_data_in,
                                                                sf_crypto_data_handle_t * const p_data_out)
{
    uint32_t hal_err = (uint32_t)SSP_SUCCESS;
    uint32_t remaining_length = (uint32_t)(p_data_in->data_length);
    uint32_t fill_bytes_needed = 0U;
    uint32_t part_out_bytes = 0U;                    // Partial block written to output buffer.
    uint32_t advance_in_data = 0U;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    /** This will be updated once the data is written to the buffer. */
    p_data_out->data_length = 0U;

    /** Check if there is data in the partial block buffer. If so first fill it from the input data */
    sf_crypto_cipher_aes_check_fill_partial_block(p_ctrl,
                                                  &fill_bytes_needed,
                                                  p_data_in);

    /** Update the number of bytes remaining in the input buffer to be processed. */
     remaining_length -= fill_bytes_needed;
     advance_in_data = fill_bytes_needed;

     /** If there is a remaining partial block process it. For GCM zero padding is automatic  */
    if  (p_context->partial_bytes_buffered)
    {
        hal_err = sf_crypto_cipher_aes_gcm_process_partial_block(p_ctrl, p_data_out);
        if ((uint32_t)SSP_SUCCESS != hal_err)
        {
            return (ssp_err_t)hal_err;
        }
        /** update the number of bytes written to the output buffer. */
        part_out_bytes = (p_context->partial_bytes_buffered);
        p_data_out->data_length += part_out_bytes;

        /** Now that all the data in the partial block is processed, clean it up and set the size to 0 */
        memset(p_context->partial_block, 0U, (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES);
        p_context->partial_bytes_buffered = 0U;
    }

    /** If remaining length is 0, we are done else we have more data to process*/
    if (remaining_length == 0U)
    {
        return SSP_SUCCESS; ;
    }

    hal_err = ((aes_api_t *)p_ctrl->p_hal_api)->zeroPaddingDecrypt((aes_ctrl_t *)(p_ctrl->p_hal_ctrl),
                                                                   (uint32_t *)p_context->cipher_key,
                                                                   (uint32_t *)p_context->working_iv,
                                                                   (uint32_t)remaining_length,
                                                                   (uint32_t *)p_data_in->p_data + advance_in_data,
                                                                   (uint32_t *)p_data_out->p_data + part_out_bytes);

    /** update the number of bytes written to the output buffer. */
      p_data_out->data_length += remaining_length;

    return (ssp_err_t)hal_err;
} /* End of function sf_crypto_cipher_aes_gcm_helper_decrypt_final */

/*******************************************************************************************************************//**
 * @brief   Subroutine to handle the PKCS#7 padding scheme for the cipher encrypt operation.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 * @param[in, out]  p_data_out              Pointer to the output data structure - has the pointer to
 *                                          output data and the data length.

 * @retval          SSP_SUCCESS             Cipher operation was finalized successfully.
 * @return                                  See @ref Common_Error_Codes or HAL driver for other possible return codes
 *                                          or causes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_handle_encrypt_padding (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                              sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t num_pad_bytes = 0;
    uint8_t temp_buffer[SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES] = {0};
    sf_crypto_data_handle_t temp_data_out;

    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    temp_data_out.p_data = temp_buffer;
    temp_data_out.data_length = sizeof (temp_buffer);

    /** Add PKCS#7 padding */
    if (p_context->partial_bytes_buffered)
    {
        num_pad_bytes = (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES - p_context->partial_bytes_buffered;
    }
    else
    {
        num_pad_bytes = (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES;
    }

    memset((p_context->partial_block + (p_context->partial_bytes_buffered)),
           (uint8_t)num_pad_bytes,
           num_pad_bytes);

    p_context->partial_bytes_buffered = (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES;
    err = sf_crypto_cipher_aes_process_partial_block(p_ctrl, &temp_data_out);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    memcpy((p_data_out->p_data + (p_data_out->data_length)),
            temp_buffer,
            (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES);

    (p_data_out->data_length) += (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES;

    return err;
} /* End of function sf_crypto_cipher_aes_handle_encrypt_padding */

/*******************************************************************************************************************//**
 * @brief   Subroutine to get the GCM tag after the the cipher encrypt operation.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used

 * @retval          SSP_SUCCESS             The GCM tag was created successfully.
 * @return                                  See @ref Common_Error_Codes or HAL driver for other possible return codes
 *                                          or causes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_get_gcm_tag (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    uint32_t hal_err = (uint32_t)SSP_SUCCESS;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    hal_err = ((aes_api_t *)p_ctrl->p_hal_api)->encrypt((aes_ctrl_t *)(p_ctrl->p_hal_ctrl),
                                                        (uint32_t *)p_context->cipher_key,
                                                        (uint32_t *)p_context->working_iv,
                                                        SF_CRYPTO_CIPHER_AES_GCM_TAG_LENGTH_16_BYTES,
                                                        NULL,
                                                        (uint32_t *)p_context->user_gcm_auth_tag->p_data);
    if (SSP_SUCCESS != hal_err)
    {
        return (ssp_err_t)hal_err;
    }

    p_context->user_gcm_auth_tag->data_length = SF_CRYPTO_CIPHER_AES_GCM_TAG_LENGTH_16_BYTES;
    return (ssp_err_t)hal_err;
}  /* End of function sf_crypto_cipher_aes_get_gcm_tag */

/*******************************************************************************************************************//**
 * @brief   Subroutine to set the GCM tag for the the cipher decrypt operation.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.

 * @retval          SSP_SUCCESS             The GCM tag was created successfully.
 * @return                                  See @ref Common_Error_Codes or HAL driver for other possible return codes
 *                                          or causes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_set_gcm_tag (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    uint32_t hal_err = (uint32_t)SSP_SUCCESS;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    hal_err = ((aes_api_t *)p_ctrl->p_hal_api)->setGcmTag((aes_ctrl_t *)(p_ctrl->p_hal_ctrl),
                                                        (uint32_t)((p_context->gcm_auth_tag_length) /(uint32_t)SF_CRYPTO_CIPHER_BYTES_PER_WORD),
                                                        (uint32_t *)p_context->gcm_auth_tag);


    return (ssp_err_t)hal_err;
} /* End of function sf_crypto_cipher_aes_set_gcm_tag */

/*******************************************************************************************************************//**
 * @brief   Subroutine to compute and verify the GCM tag for after the cipher decrypt operation.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.

 * @retval          SSP_SUCCESS             The GCM tag was computed and verified successfully.
 * @return                                  See @ref Common_Error_Codes or HAL driver for other possible return codes
 *                                          or causes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_compute_verify_gcm_tag (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    uint32_t hal_err = (uint32_t)SSP_SUCCESS;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    hal_err = ((aes_api_t *)p_ctrl->p_hal_api)->decrypt((aes_ctrl_t *)(p_ctrl->p_hal_ctrl),
                                                        (uint32_t *)p_context->cipher_key,
                                                        (uint32_t *)p_context->working_iv,
                                                        0,
                                                        NULL,
                                                        NULL);

    return (ssp_err_t)hal_err;
} /* End of function sf_crypto_cipher_aes_compute_verify_gcm_tag */

/*******************************************************************************************************************//**
 * @brief   Subroutine to validate the parameters for the AES cipher final operation.
 * This function only checks if any input data is received for the final operation.
 *  *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 *                                          in the open() call.
 * @param[in, out]  p_data_in               Pointer to the input data structure - has the pointer to
 *                                          input data and the data length
  *
 * @retval          SSP_SUCCESS             The parameters were verified successfully.
 * @retval          SSP_ERR_INVALID_ARGUMENT No input data was provided either to the cipherUpdate()
 *                                           or the cipherFinal operation.
 * @return                                  See @ref Common_Error_Codes or HAL driver for other possible return codes
 *                                          or causes.
 **********************************************************************************************************************/
static ssp_err_t  sf_crypto_cipher_aes_helper_validate_final_params (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                     sf_crypto_data_handle_t const * const p_data_in)
{
    ssp_err_t err = SSP_SUCCESS;

    /** GCM is the only mode that can take no input data but just create a tag
     * For other modes: If no input data is provided in update operation and none in final, return error.
     * If the status is SF_CRYPTO_CIPHER_STATE_INITIALIZED, it means this is a one shot final operation and
     * no data was provided through the Update call.
     */
    if (SF_CRYPTO_CIPHER_MODE_GCM != p_ctrl->cipher_chaining_mode)
    {
        if ((SF_CRYPTO_CIPHER_STATE_INITIALIZED == p_ctrl->status) &&
            ((NULL == p_data_in) ||
              (NULL == p_data_in->p_data) ||
              (0U == p_data_in->data_length)))
        {
            return SSP_ERR_INVALID_ARGUMENT;
        }
    }

    return err;
} /* End of function sf_crypto_cipher_validate_aes_final_params */


static ssp_err_t sf_crypto_cipher_aes_validate_encrypt_final_out_buffer (sf_crypto_data_handle_t * const p_data_out,
                                                                         uint32_t required_length)
{
    ssp_err_t err = SSP_SUCCESS;

    if (p_data_out->data_length < required_length)
    {
        return SSP_ERR_INVALID_SIZE;
    }

    if ((required_length) && (NULL == p_data_out->p_data))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    return err;
}
/*******************************************************************************************************************//**
 * @brief   Subroutine to validate the parameters for the final encrypt operation.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 * @param[in, out]  p_data_in               Pointer to the input data structure - has the pointer to
 *                                          input data and the data length
 * @param[in, out]  p_data_out              Pointer to the output data structure - has the pointer to
 *                                          output data and the data length
 *
 * @retval          SSP_SUCCESS             The parameters were verified successfully.
 * @retval          SSP_ERR_INVALID_ARGUMENT  Input data is not a multiple of block size.
 * @retval          SSP_ERR_INVALID_SIZE    Output data buffer is inadequate to hold the output data.
 * @return                                  See @ref Common_Error_Codes or HAL driver for other possible return codes
 *                                          or causes.
 **********************************************************************************************************************/
static ssp_err_t  sf_crypto_cipher_aes_validate_encrypt_final_params (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                      sf_crypto_data_handle_t const * const p_data_in,
                                                                      sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t possible_output_bytes = 0;
    uint32_t partial_block_bytes = 0;       // bytes left over when data is not block size.
    uint32_t input_length = 0;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    if ((p_data_in) && (NULL != p_data_in->p_data))
    {
        input_length = p_data_in->data_length;
    }

    partial_block_bytes = (p_context->partial_bytes_buffered + input_length)%
                           (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES;

    /** GCM is the only mode that gets zero padded internally.
     * Unless a padding scheme is selected, for rest of the modes the data has to be a multiple of the block size.
     */
    if ((SF_CRYPTO_CIPHER_MODE_GCM != p_ctrl->cipher_chaining_mode) &&
        (SF_CRYPTO_CIPHER_PADDING_SCHEME_NO_PADDING == p_context->padding_scheme) &&
        (partial_block_bytes))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    /** Check if the output buffer is big enough */
    possible_output_bytes = p_context->partial_bytes_buffered + input_length;

    /** If a padding scheme is selected and mode is not GCM,
     * whether or not there are partial bytes remaining, padding will be applied.
     */
    if ((SF_CRYPTO_CIPHER_MODE_GCM != p_ctrl->cipher_chaining_mode) &&
        (SF_CRYPTO_CIPHER_PADDING_SCHEME_NO_PADDING != p_context->padding_scheme))
    {
        possible_output_bytes += ((uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES - partial_block_bytes);
    }

    err = sf_crypto_cipher_aes_validate_encrypt_final_out_buffer(p_data_out, possible_output_bytes);

    return err;
} /* End of function sf_crypto_cipher_validate_aes_encrypt_final_params */

/*******************************************************************************************************************//**
 * @brief   Subroutine to validate the parameters for the final decrypt operation.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 * @param[in, out]  p_data_in               Pointer to the input data structure - has the pointer to
 *                                          input data and the data length
 * @param[in, out]  p_data_out              Pointer to the output data structure - has the pointer to
 *                                          output data and the data length
 *
 * @retval          SSP_SUCCESS               The parameters were verified successfully.
 * @retval          SSP_ERR_INVALID_ARGUMENT  input data is not a multiple of block size.
 * @return                                    See @ref Common_Error_Codes or HAL driver for other possible return
 *                                            codes or causes.
 **********************************************************************************************************************/
static ssp_err_t  sf_crypto_cipher_aes_validate_decrypt_final_params (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                      sf_crypto_data_handle_t const * const p_data_in,
                                                                      sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t possible_output_bytes = 0;
    uint32_t partial_block_bytes = 0;       // bytes left over when data is not block size.
    uint32_t input_length = 0;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    if ((p_data_in) && (NULL != p_data_in->p_data))
    {
        input_length = p_data_in->data_length;
    }

    partial_block_bytes = (p_context->partial_bytes_buffered + input_length)%
                          (uint32_t)SF_CRYPTO_CIPHER_AES_BLOCK_SIZE_BYTES;

    /** Except for GCM - For decryption, the data must be a multiple of block size */
    if ((partial_block_bytes) && (SF_CRYPTO_CIPHER_MODE_GCM != p_ctrl->cipher_chaining_mode))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    /** Check if the output buffer is big enough.
     * Note: If there is partial data in GCM, it will be zero padded internally for calculations but
     * the data returned will be the same size as the input data.
     * */
    possible_output_bytes = p_context->partial_bytes_buffered + input_length;

    if (p_data_out->data_length < possible_output_bytes)
    {
        return SSP_ERR_INVALID_SIZE;
    }

    if ((possible_output_bytes) && (NULL == p_data_out->p_data))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    return err;
} /* End of function sf_crypto_cipher_validate_aes_decrypt_final_params */

/*******************************************************************************************************************//**
 * @brief   Subroutine calls the HAL encrypt / decrypt APIs as pointed to by the HAL interface in the control block.
 * It is assumed that the control block is already populated with the interface for the HAL AES APIs. *
 *
 * @param[in,out]   p_ctrl      Pointer to the Cipher framework module instance control block.
 * @param[in]       p_key       Pointer to the key to be used for the cipher operation.
 * @param[in]       p_iv        Pointer to the IV to be used for the cipher operation.
 * @param[in]       num_words   Number of words of input data for the cipher operation.
 * @param[in]       p_input     Pointer to the input data for the cipher operation.
 * @param[in]       p_output    Pointer to the IV to output data for the cipher operation.
 *
 * @retval SSP_SUCCESS          The cipher update operation was successful.
 * @retval SSP_ERR_UNSUPPORTED  The module does not support the algorithm bbased on the key type specified by the user.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_aes_hal_encrypt_decrypt(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                          uint32_t *p_key,
                                                          uint32_t *p_iv,
                                                          uint32_t num_words,
                                                          uint32_t *p_input,
                                                          uint32_t *p_output
                                                          )
{
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;
    aes_ctrl_t * p_hal_aes_ctrl = (aes_ctrl_t *)(p_ctrl->p_hal_ctrl);
    uint32_t hal_err = (uint32_t)SSP_SUCCESS;

    if (SF_CRYPTO_CIPHER_OP_MODE_ENCRYPT == (sf_crypto_cipher_op_mode_t)p_context->operation_mode)
    {
        hal_err = ((aes_api_t *)p_ctrl->p_hal_api)->encrypt((aes_ctrl_t *)p_hal_aes_ctrl,
                                                            (uint32_t *)p_key,
                                                            (uint32_t *)p_iv,
                                                            (uint32_t)num_words,
                                                            (uint32_t *)p_input,
                                                            (uint32_t *)p_output);
    }
    else
    {
        hal_err = ((aes_api_t *)p_ctrl->p_hal_api)->decrypt((aes_ctrl_t *)(p_ctrl->p_hal_ctrl),
                                                            (uint32_t *)p_key,
                                                            (uint32_t *)p_iv,
                                                            (uint32_t)num_words,
                                                            (uint32_t *)p_input,
                                                            (uint32_t *)p_output);
    }

    return (ssp_err_t)hal_err;
} /* End of function sf_crypto_cipher_aes_hal_encrypt_decrypt */

/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto  Cipher Framework to call the appropriate routine for final encryption based on
 * the algorithm for the cipherFinal operation.
 * This function is called by SF_CRYPTO_CIPHER_CipherFinal().
 *
 * @param[in,out]   p_ctrl      Pointer to the Cipher framework module instance control block.
 * @param[in]       p_data_in   Pointer to the input data buffer and the length of the input data.
 * @param[in, out]  p_data_out  Pointer to the output data buffer and the buffer length on input.
 *                              If data is output, the length of the data will be updated.
 *
 * @retval SSP_SUCCESS          The cipher update operation was successful.
 * @retval SSP_ERR_UNSUPPORTED  The module does not support the algorithm bbased on the key type specified by the user.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_cipher_aes_final (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                      sf_crypto_data_handle_t const * const p_data_in,
                                      sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    /** Validate that there is some input data provided / processed before the finalizing operation
     * Applies to both encrypt and decrypt operations.
     * AES GCM is the only exception. */
    err = sf_crypto_cipher_aes_helper_validate_final_params(p_ctrl,
                                                            p_data_in);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    switch (p_context->operation_mode)
    {
        case SF_CRYPTO_CIPHER_OP_MODE_ENCRYPT:
            err = sf_crypto_cipher_aes_encrypt_final(p_ctrl, p_data_in, p_data_out);
        break;
        case SF_CRYPTO_CIPHER_OP_MODE_DECRYPT:
            err = sf_crypto_cipher_aes_decrypt_final(p_ctrl, p_data_in, p_data_out);
        break;
        default:
            err = SSP_ERR_UNSUPPORTED;
        break;
    }
    /* Clean up the context buffer before exiting.*/
    sf_crypto_cipher_aes_context_buffer_pointer_data_zeroise(p_ctrl);

    return err;
} /* End of function sf_crypto_cipher_aes_final */

/*******************************************************************************************************************//**
 * @brief   A subroutine to zeroise the memory pointed to by the fields within the context buffer.
 * NOTE: the buffer sizes pointed to by the original allocation should NOT be cleared.
 * Those sizes will be used when freeing up the memory.
 *
 * @param[in,out]   p_ctrl      Pointer to the Cipher framework module instance control block.
 **********************************************************************************************************************/
static void sf_crypto_cipher_aes_context_buffer_pointer_data_zeroise (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    if (p_context->cipher_key)
    {
        memset(p_context->cipher_key, 0, p_context->cipher_key_buffer_size);
    }
    if (p_context->working_iv)
    {
        memset(p_context->working_iv, 0, p_context->working_iv_buffer_size);
    }
    if (p_context->gcm_auth_tag)
    {
        memset(p_context->gcm_auth_tag, 0, p_context->gcm_auth_tag_buffer_size);
    }
    if(p_context->partial_aad_block)
    {
        memset(p_context->partial_aad_block, 0, p_context->partial_aad_buffer_size);
    }
    if(p_context->partial_block)
    {
        memset(p_context->partial_block, 0, p_context->partial_block_size);
    }

    p_context->gcm_auth_tag_length = 0U;
    p_context->partial_aad_length = 0U;
    p_context->partial_bytes_buffered = 0U;
    p_context->working_iv_length = 0U;
    return;
} /* End of function sf_crypto_cipher_aes_context_buffer_pointer_data_zeroise */

/*******************************************************************************************************************//**
 * @brief   A subroutine to release the memory allocated for context buffer.
 *          All the data pointed to by the context buffer is first zeroised and freed.
 *          Next all elements of the context buffer are zeroised and pointers are set to NULL.
 *          Lastly the context buffer itself is freed.
 *
 * @param[in,out]   p_ctrl      The instance control block for the Cipher module.
 *
 * @retval          SSP_SUCCESS             The memory for this instance was freed successfully.
 * @retval          SSP_ERR_INTERNAL        RTOS service returned a unexpected error.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_instance_aes_memory_deallocate (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_aes_context_t * p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    /** First zeroise the data pointed to from the context buffer
     * and then release the memory resources used for this instance
     */
    sf_crypto_cipher_aes_context_buffer_pointer_data_zeroise(p_ctrl);

    /** Release the memory allocated for Partial Block */
    err = sf_crypto_memory_release (p_context->partial_block,
                                    p_context->partial_block_size);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    /** Release the memory allocated for Key */
    err = sf_crypto_memory_release (p_context->cipher_key,
                                    p_context->cipher_key_buffer_size);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Release the memory allocated for IV */
    err = sf_crypto_memory_release (p_context->working_iv,
                                    p_context->working_iv_buffer_size);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    if (p_ctrl->cipher_chaining_mode == SF_CRYPTO_CIPHER_MODE_GCM)
    {
        /** Release the memory allocated for AAD partial Block and tag */
        err = sf_crypto_cipher_instance_aes_gcm_memory_release(p_ctrl);
    }

    /** Zeroise the context buffer and set all pointers to NULL */
    sf_crypto_cipher_aes_zeroise_context_buffer(p_ctrl);

    /** Release the memory allocated for the context buffer. */
    err =  sf_crypto_memory_release(p_ctrl->p_cipher_context_buffer,
                                    sizeof (sf_crypto_cipher_aes_context_t));

    return err;
}  /* End of function sf_crypto_cipher_instance_aes_memory_deallocate */


/*******************************************************************************************************************//**
 * @brief   A subroutine to init / reset the context buffer.
 * All elements are zeroised and pointers are set to NULL.
 *
 * @param[in,out]   p_ctrl      The instance control block for the Cipher module.
 *
 **********************************************************************************************************************/
static void sf_crypto_cipher_aes_zeroise_context_buffer (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    sf_crypto_cipher_aes_context_t *p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;
    memset (p_ctrl->p_cipher_context_buffer, 0, sizeof(sf_crypto_cipher_aes_context_t));

    p_context->cipher_key = NULL;
    p_context->partial_block = NULL;
    p_context->working_iv = NULL;
    p_context->partial_aad_block = NULL;
    p_context->gcm_auth_tag = NULL;
    p_context->user_gcm_auth_tag = NULL;

} /* End of function sf_crypto_cipher_aes_zeroise_context_buffer */


/*******************************************************************************************************************//**
 * @brief   A subroutine to free the memory allocated for the GCM data in the context buffer.
 * All elements are zeroised and pointers are set to NULL.
 *
 * @param[in,out]   p_ctrl      The instance control block for the Cipher module.
 *
 * @retval          SSP_SUCCESS         The GCM data in the context buffer for this instance was freed successfully.
 * @retval          SSP_ERR_INTERNAL    RTOS service returned a unexpected error.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_instance_aes_gcm_memory_release (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_aes_context_t *p_context = (sf_crypto_cipher_aes_context_t *)p_ctrl->p_cipher_context_buffer;

    err = sf_crypto_memory_release (p_context->gcm_auth_tag,
                                    p_context->gcm_auth_tag_buffer_size);
    if (SSP_SUCCESS == err)
    {
        err = sf_crypto_memory_release(p_context->partial_aad_block,
                                        p_context->partial_aad_length);
    }

    return err;
} /* End of function sf_crypto_cipher_instance_aes_gcm_memory_release */
/*******************************************************************************************************************//**
 * @brief   Allocates memory for the instance and opens the underlying HAL instance.
 *
 * @param   [in,out]   p_ctrl       Pointer to Crypto Cipher Framework instance control block structure.
 * @param   [in]       p_cfg        Pointer to the configuration structure for Cipher module .
 *
 * @retval  SSP_SUCCESS             The module instantiated successfully.
 * @retval  SSP_ERR_TIMEOUT         Was unable to allocate the memory within the specified time to wait.
 * @retval  SP_ERR_OUT_OF_MEMORY    Requested size is zero or larger than the pool.
 * @retval  SSP_ERR_INTERNAL        RTOS service returned a unexpected error.
 * @return                          See @ref Common_Error_Codes or HAL driver for other possible
 *                                  return codes or causes. This function calls:
 *                                  sf_crypto_cipher_instance_memory_allocate
 *                                  sf_crypto_cipher_hal_open
**********************************************************************************************************************/
ssp_err_t sf_crypto_cipher_initialize_aes_instance(sf_crypto_cipher_instance_ctrl_t * p_ctrl,
                                                   sf_crypto_cipher_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;

    err = sf_crypto_cipher_instance_aes_memory_allocate(p_ctrl);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    err = sf_crypto_cipher_aes_hal_open(p_ctrl, p_cfg);
    if (SSP_SUCCESS != err)
    {
        sf_crypto_cipher_instance_aes_memory_deallocate(p_ctrl);
    }

    return err;
} /* End of function sf_crypto_cipher_initialize_aes_instance */

/*******************************************************************************************************************//**
 * @brief   closes the underlying HAL instance and releases the memory for the instance.
 *
 * @param   [in,out]   p_ctrl       Pointer to Crypto Cipher Framework instance control block structure.
 *
 * @retval  SSP_SUCCESS             The module instantiated successfully.
 * @retval  SSP_ERR_TIMEOUT         Was unable to allocate the memory within the specified time to wait.
 * @retval  SP_ERR_OUT_OF_MEMORY    Requested size is zero or larger than the pool.
 * @retval  SSP_ERR_INTERNAL        RTOS service returned a unexpected error.
 * @return                          See @ref Common_Error_Codes or HAL driver for other possible
 *                                  return codes or causes. This function calls:
 *                                  sf_crypto_cipher_instance_memory_allocate
 *                                  sf_crypto_cipher_hal_open
**********************************************************************************************************************/
ssp_err_t sf_crypto_cipher_deinitialize_aes_instance(sf_crypto_cipher_instance_ctrl_t * p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;

    err = sf_crypto_cipher_aes_hal_close(p_ctrl);
    if (SSP_SUCCESS == err)
    {
        err = sf_crypto_cipher_instance_aes_memory_deallocate(p_ctrl);
    }

    return err;
}  /* End of function sf_crypto_cipher_deinitialize_aes_instance */

/*******************************************************************************************************************//**
 * @} (end defgroup SF_CRYPTO_CIPHER)
 **********************************************************************************************************************/

