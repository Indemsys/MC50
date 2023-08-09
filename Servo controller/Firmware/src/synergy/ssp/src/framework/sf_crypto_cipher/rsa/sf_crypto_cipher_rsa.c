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
 * File Name    : sf_crypto_cipher_rsa.c
 * Description  : SSP Crypto Cipher Framework Module specific for RSA key types.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @file
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto_cipher.h"
#include "../../sf_crypto/sf_crypto_private_api.h"
#include "../sf_crypto_cipher_private_api.h"
#include "sf_crypto_cipher_rsa_private.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes. DO NOT USE! These functions are for internal usage.
 **********************************************************************************************************************/
static void sf_crypto_cipher_rsa_zeroise_context_buffer(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);
static ssp_err_t sf_crypto_cipher_get_rsa_modulus_size_bits(sf_crypto_key_size_t key_size, uint32_t *key_size_bits);
static ssp_err_t sf_crypto_cipher_get_rsa_public_key_size_bytes(sf_crypto_key_size_t key_size, uint32_t *key_size_bytes );
static ssp_err_t sf_crypto_cipher_get_rsa_pvt_key_size_bytes(sf_crypto_key_type_t key_type,
                                                            sf_crypto_key_size_t key_size,
                                                            uint32_t *key_size_bytes);
static ssp_err_t sf_crypto_cipher_get_rsa_block_size_bytes(sf_crypto_key_size_t key_size, uint32_t *block_size_bytes);
static void sf_crypto_cipher_rsa_context_buffer_pointer_data_zeroise(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);
static ssp_err_t sf_crypto_cipher_rsa_encrypt_update(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                     sf_crypto_data_handle_t const * const p_data_in,
                                                     sf_crypto_data_handle_t * const p_data_out);
static ssp_err_t sf_crypto_cipher_rsa_init_validate(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                    sf_crypto_cipher_op_mode_t cipher_operation_mode,
                                                    sf_crypto_key_t const * const p_key,
                                                    sf_crypto_cipher_rsa_init_params_t * p_rsa_params);
static ssp_err_t sf_crypto_cipher_rsa_decrypt_update(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                    sf_crypto_data_handle_t const * const p_data_in,
                                                    sf_crypto_data_handle_t * const p_data_out);
static ssp_err_t sf_crypto_cipher_rsa_pkcs_1_5_encode_block(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);
static ssp_err_t sf_crypto_cipher_rsa_hal_encrypt_decrypt(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                          uint32_t *p_key,
                                                          uint32_t *p_domain,
                                                          uint32_t num_words,
                                                          uint32_t *p_input,
                                                          uint32_t *p_output);

static ssp_err_t sf_crypto_cipher_rsa_encrypt_final(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                    sf_crypto_data_handle_t const * const p_data_in,
                                                    sf_crypto_data_handle_t * const p_data_out);


static ssp_err_t sf_crypto_cipher_rsa_decrypt_final(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                    sf_crypto_data_handle_t const * const p_data_in,
                                                    sf_crypto_data_handle_t * const p_data_out);
static ssp_err_t sf_crypto_cipher_rsa_pkcs_1_5_decode_block(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);

static ssp_err_t sf_crypto_cipher_rsa_private_free_context_buffer_key(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);
static ssp_err_t sf_crypto_cipher_rsa_private_free_context_buffer(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);
static ssp_err_t sf_crypto_cipher_rsa_hal_open(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                        sf_crypto_cipher_cfg_t const * const p_cfg);

static ssp_err_t sf_crypto_cipher_rsa_hal_close(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);

static ssp_err_t sf_crypto_cipher_instance_rsa_memory_allocate(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);

static ssp_err_t sf_crypto_cipher_instance_rsa_memory_deallocate(sf_crypto_cipher_instance_ctrl_t * const p_ctrl);
static ssp_err_t sf_crypto_cipher_rsa_validate_encrypt_final_out_params(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                        sf_crypto_data_handle_t * const p_data_out);
static ssp_err_t sf_crypto_cipher_rsa_helper_pkcs_1_5_encode_block(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                   sf_crypto_data_handle_t const * const p_data_in);
static ssp_err_t sf_crypto_cipher_rsa_final_input_buffer_check(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                               sf_crypto_data_handle_t const * const p_data_in);
static ssp_err_t sf_crypto_cipher_rsa_init_validate_padding_scheme(sf_crypto_cipher_padding_scheme_t padding_scheme);


/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

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



/*******************************************************************************************************************//**
 * Private Functions.
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @brief   Routine to calculate and return the Encryption block size based on the key size.
 *
 * @param[in]   key_size_bits   key size in bits.
  **********************************************************************************************************************/
__STATIC_INLINE uint32_t SF_CRYPTO_CIPHER_RSA_EB_SIZE_BYTES(uint32_t key_size_bits)
{
    return (RSA_MODULUS_SIZE_BYTES(key_size_bits));
}

/*******************************************************************************************************************//**
 * @brief   Routine to calculate and return the Encryption block size based on the key size.
 *
 * @param[in]   key_size_bits   key size in bits.
  **********************************************************************************************************************/
__STATIC_INLINE uint32_t SF_CRYPTO_CIPHER_RSA_PKCS_1_5_EB_DATA_SIZE_BYTES(uint32_t key_size_bits)
{
    return ((SF_CRYPTO_CIPHER_RSA_EB_SIZE_BYTES(key_size_bits)) - (SF_CRYPTO_PKCS_1_5_EB_OVERHEAD));
}


/*******************************************************************************************************************//**
 * @brief          Subroutine to open a Crypto RSA HAL module. This function is called by SF_CRYPTO_CIPHER_Open().
 *
 * @param[in,out]  p_ctrl                   Pointer to a Crypto Key Framework module control block
 * @param[in,out]  p_cfg                    Pointer to a Crypto Key Framework module configuration structure
 * @retval         SSP_SUCCESS              RSA HAL module is successfully opened.
 * @retval         SSP_ERR_OUT_OF_MEMORY    Failed to allocate memory to store RSA HAL module control block.
 * @retval         SSP_ERR_INTERNAL         RTOS service returned a unexpected error.
 * @return                                  See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_hal_open (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                sf_crypto_cipher_cfg_t const * const p_cfg)

{
    SSP_PARAMETER_NOT_USED(p_cfg);

    ssp_err_t   err = SSP_SUCCESS;
    uint32_t    err_crypto;
    sf_crypto_instance_ctrl_t * p_fwk_common_ctrl;
    rsa_cfg_t   hal_cfg = {0};

    /** Get a Crypto common control block and the interface. */
    p_fwk_common_ctrl = p_ctrl->p_lower_lvl_fwk_common_ctrl;

    /** Allocate memory for a Crypto HAL control block from the byte pool. */
    err = sf_crypto_memory_allocate (&p_fwk_common_ctrl->byte_pool,
                                     &p_ctrl->p_hal_ctrl,
                                     sizeof(rsa_ctrl_t),
                                     p_fwk_common_ctrl->wait_option);

    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /* Get a RSA interface instance. */
     sf_crypto_cipher_rsa_interface_get(p_ctrl);

     if (NULL == p_ctrl->p_hal_api)
     {
         sf_crypto_memory_release(p_ctrl->p_hal_ctrl, sizeof(rsa_ctrl_t));
         return SSP_ERR_UNSUPPORTED;
     }

     /** Set Crypto HAL API instance with the control common hardware api. */
     hal_cfg.p_crypto_api = ((crypto_instance_t *)p_fwk_common_ctrl->p_lower_lvl_crypto)->p_api;

     /** Open the Crypto HAL RSA module. */
     err_crypto = ((rsa_api_t *)p_ctrl->p_hal_api)->open ((rsa_ctrl_t *)(p_ctrl->p_hal_ctrl), &hal_cfg);
     if ((uint32_t)SSP_SUCCESS != err_crypto)
     {
        return (ssp_err_t)err_crypto;
     }

      return err;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to close a Crypto RSA HAL module. This function is called by SF_CRYPTO_CIPHER_Close().
 *
 * @param[in]  p_ctrl                   Pointer to a Crypto Key Framework module control block
 * @retval     SSP_SUCCESS              RSA HAL module is successfully closed.
 * @return                              See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_hal_close (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    void * p_tmp = p_ctrl->p_hal_ctrl;

    /** Close the Crypto HAL driver. */
    ssp_err_t   err = (ssp_err_t) ((rsa_api_t *)p_ctrl->p_hal_api)->close((rsa_ctrl_t *)p_ctrl->p_hal_ctrl);
    if ((uint32_t) SSP_SUCCESS == err)
    {
        /** Release the control block memory back to byte pool */
        err = sf_crypto_memory_release(p_tmp, sizeof(rsa_ctrl_t));
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to get a RSA HAL API instance. This function is called by sf_crypto_cipher_open_rsa().
 *
 * @param[in,out]  p_ctrl        Pointer to a Key framework control block, whose p_hal_api filled with HAL RSA
 *                               interface. This indicates NULL, for not supported MCUs
 **********************************************************************************************************************/
void sf_crypto_cipher_rsa_interface_get (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    sf_crypto_instance_ctrl_t * p_fwk_common_ctrl;
    crypto_instance_t         * p_crypto_hal;
    crypto_interface_get_param_t param;

    /** Get a Crypto common control block and the HAL instance. */
    p_fwk_common_ctrl = (sf_crypto_instance_ctrl_t *)(p_ctrl->p_lower_lvl_fwk_common_ctrl);
    p_crypto_hal = (crypto_instance_t *)(p_fwk_common_ctrl->p_lower_lvl_crypto);


    /** Check the RSA key type. */
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

    /** Check the RSA key type. */
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

    /** Get the HAL API instance for a selected algorithm type. */
    param.algorithm_type = CRYPTO_ALGORITHM_TYPE_RSA;
    p_crypto_hal->p_api->interfaceGet(&param, &p_ctrl->p_hal_api);

}

/*******************************************************************************************************************//**
 * @brief   Subroutine to free memory allocated for the key in the context buffer for RSA cipher operation.
 *
 * @param[in,out]   p_ctrl                  Pointer to a Crypto Key Framework module control block.
 * @retval          SSP_SUCCESS             Memory allocation was successful.
 * @retval          SSP_ERR_TIMEOUT         Was unable to allocate the memory within the specified time to wait.
 * @retval          SSP_ERR_OUT_OF_MEMORY   Requested size is zero or larger than the pool.
 * @retval          SSP_ERR_INTERNAL        RTOS service returned a unexpected error. *
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_private_free_context_buffer_key (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_rsa_context_t * p_context = (sf_crypto_cipher_rsa_context_t *)p_ctrl->p_cipher_context_buffer;
    if (p_context == NULL)
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }
    /** Release the memory allocated for the RSA Context Buffer */
    err = sf_crypto_memory_release (p_context->cipher_key,
                                    p_context->cipher_key_buffer_size);

    return err;
} /* End of function sf_crypto_cipher_rsa_memory_free_context_buffer */

/*******************************************************************************************************************//**
 * @brief   Subroutine to free memory allocated for the context buffer for RSA cipher operation.
 *
 * @param[in,out]   p_ctrl                  Pointer to a Crypto Key Framework module control block.
 * @retval          SSP_SUCCESS             Memory allocation was successful.
 * @retval          SSP_ERR_TIMEOUT         Was unable to allocate the memory within the specified time to wait.
 * @retval          SSP_ERR_OUT_OF_MEMORY   Requested size is zero or larger than the pool.
 * @retval          SSP_ERR_INTERNAL        RTOS service returned a unexpected error. *
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_private_free_context_buffer (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;

    /** Release the memory allocated for the RSA Context Buffer */
    err = sf_crypto_memory_release (p_ctrl->p_cipher_context_buffer,
                                    sizeof (sf_crypto_cipher_rsa_context_t));
    return err;
} /* End of function sf_crypto_cipher_rsa_memory_free_context_buffer */

/*******************************************************************************************************************//**
 * @brief   Subroutine to allocate memory required for the context buffer for RSA cipher operation.
 *
 * @param[in,out]   p_ctrl                  Pointer to a Crypto Key Framework module control block.
 * @retval          SSP_SUCCESS             Memory allocation was successful.
 * @retval          SSP_ERR_TIMEOUT         Was unable to allocate the memory within the specified time to wait.
 * @retval          SSP_ERR_OUT_OF_MEMORY   Requested size is zero or larger than the pool.
 * @retval          SSP_ERR_INTERNAL        RTOS service returned a unexpected error.
 * @retval          SSP_ERR_INTERNAL        An internal error occurred during processing.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_instance_rsa_memory_allocate (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err= SSP_SUCCESS;
    sf_crypto_instance_ctrl_t * p_ctrl_common = p_ctrl->p_lower_lvl_fwk_common_ctrl;
    sf_crypto_cipher_rsa_context_t * p_context = NULL;

    /** Allocate memory for the RSA context buffer. Extra 3 bytes are allocated to get the buffer WORD aligned*/
    err = sf_crypto_memory_allocate (&p_ctrl_common->byte_pool,
                                     (void **)&p_ctrl->p_cipher_context_buffer,
                                     sizeof (sf_crypto_cipher_rsa_context_t),
                                     p_ctrl_common->wait_option);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    p_context = (sf_crypto_cipher_rsa_context_t *)p_ctrl->p_cipher_context_buffer;

    /** Zeroise the context buffer and set all pointers to NULL */
    sf_crypto_cipher_rsa_zeroise_context_buffer(p_ctrl);

    err = sf_crypto_cipher_get_rsa_pvt_key_size_bytes(p_ctrl->key_type,
                                                      p_ctrl->key_size,
                                                      &(p_context->cipher_key_buffer_size));
    if (SSP_SUCCESS != err)
    {
       return err;
    }

    err = sf_crypto_cipher_get_rsa_block_size_bytes(p_ctrl->key_size, &(p_context->partial_block_size));
    if (SSP_SUCCESS != err)
    {
       return SSP_ERR_INTERNAL;
    }

    /** Allocate memory for key in the context buffer Extra 3bytes are allocated to get the buffer WORD aligned*/
    err = sf_crypto_memory_allocate (&p_ctrl_common->byte_pool,
                                     (void **)&p_context->cipher_key,
                                     p_context->cipher_key_buffer_size,
                                     p_ctrl_common->wait_option);

    if (SSP_SUCCESS != err)
    {
        /* Not checking error here as we are already sending a mem alloc error. */
        sf_crypto_cipher_rsa_private_free_context_buffer(p_ctrl);
        return err;
 
    }
    /** Allocate memory for partial block. Extra 3bytes are allocated to get the buffer WORD aligned. */
    err = sf_crypto_memory_allocate (&p_ctrl_common->byte_pool,
                                     (void **)&p_context->partial_block,
                                      p_context->partial_block_size,
                                      p_ctrl_common->wait_option);
    if (SSP_SUCCESS != err)
    {
        /** Not checking error here as we are already sending a mem alloc error.
         * Free the memory for the key and then the context buffer itself.
         */
        sf_crypto_cipher_rsa_private_free_context_buffer_key(p_ctrl);
        sf_crypto_cipher_rsa_private_free_context_buffer(p_ctrl);
        return err;
    }
    return (err);
}

/*******************************************************************************************************************//**
 * @brief   Subroutine to check if key type is RSA.
 *
 * @param[in]       key_type    Key type enum.
 * @retval          true        Key type is RSA plain text (standard or CRT)/ wrapped.
 * @retval          false       Key type is not RSA.
 **********************************************************************************************************************/
bool sf_crypto_cipher_is_key_type_rsa (sf_crypto_key_type_t key_type)
{
    bool return_value = false;

    /** Check the key type is valid for RSA operations. */
    if ((key_type == SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT) ||
        (key_type == SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT) ||
        (key_type == SF_CRYPTO_KEY_TYPE_RSA_WRAPPED))
     {
        return_value = true;
     }
    return return_value;
}

/*******************************************************************************************************************//**
 * @brief   Routine to handle the cipherInit RSA operation.
 *
 * @param[in,out]   p_ctrl                      Pointer to the cipher framework module control block used
 *                                              in the open() call.
 * @param[in]       cipher_operation_mode       The cipher operation mode - encrypt / decrypt.
 * @param[in]       p_key                       Pointer to the key to be used for the cipher operations.
 * @param[in]       p_algorithm_specific_params Pointer to the algorithm specific parameters.
 *
 * @retval          SSP_SUCCESS                 Cipher operation was updated successfully.
 * @retval          SSP_ERR_INVALID_SIZE        The out buffer is inadequate to hold the output data.
 * @return                                      See @ref Common_Error_Codes or HAL driver for other possible
 *                                              return codes or causes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_cipher_rsa_init (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                     sf_crypto_cipher_op_mode_t cipher_operation_mode,
                                     sf_crypto_key_t const * const p_key,
                                     sf_crypto_cipher_algorithm_init_params_t * p_algorithm_specific_params)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_rsa_context_t *p_context = (sf_crypto_cipher_rsa_context_t *)p_ctrl->p_cipher_context_buffer;
    sf_crypto_cipher_rsa_init_params_t * p_rsa_params =
             (sf_crypto_cipher_rsa_init_params_t *)p_algorithm_specific_params;

    err = sf_crypto_cipher_rsa_init_validate (p_ctrl,
                                              cipher_operation_mode,
                                              p_key,
                                              p_rsa_params);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** All buffers within the context buffer are allocated at Open. Now just zeroise their contents. */
     sf_crypto_cipher_rsa_context_buffer_pointer_data_zeroise(p_ctrl);

    /** Copy data into the control block / context buffer. */
    p_context->operation_mode = cipher_operation_mode;

    memcpy(p_context->cipher_key,
           p_key->p_data,
           p_key->data_length);

    p_context->cipher_key_length = p_key->data_length;
    p_context->padding_scheme = p_rsa_params->padding_scheme;

    return err;
}


/*******************************************************************************************************************//**
 * @brief   Subroutine to update the cipher RSA operation.
 * This algorithm is only suitable for messages of limited length. The total number of input bytes processed during
 * encryption may not be more than k-11, where k is the RSA key's modulus size in bytes.
 *   The encryption block(EB) during encryption with a Public key is built as follows:
 *  EB = 00 || 02 || PS || 00 || M
 *      :: M (input bytes) is the plaintext message
 *      :: PS is an octet string of length k-3-||M|| of pseudo random nonzero octets.
 *         The length of PS must be at least 8 octets.
 *      :: k is the RSA modulus size.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 * @param[in]       p_data_in               Pointer to the input data structure - has the pointer to
 *                                          input data and the data length
 * @param[in, out]  p_data_out              Pointer to the output data structure - has the pointer to
 *                                          output data and the buffer length on input.
 *                                          If data is filled the length is updated on output.
 *
 * @retval          SSP_SUCCESS             Cipher operation was updated successfully.
 * @retval          SSP_ERR_UNSUPPORTED     Unknown cipher operation mode was passed in.
 * @retval          SSP_ERR_INVALID_SIZE    The out buffer is inadequate to hold the output data.
 * @return                                  See @ref Common_Error_Codes or HAL driver for other possible return
 *                                          codes or causes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_cipher_rsa_update (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                       sf_crypto_data_handle_t const * const p_data_in,
                                       sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_rsa_context_t * p_context = (sf_crypto_cipher_rsa_context_t *)p_ctrl->p_cipher_context_buffer;

    /** input parameters validation - done at the entry nothing to do here
     * no output on update - so no checks for output buffer / length
     * collect input up to the block size for encryption when no padding is selected.
     * collect data up to block size -11 for encryption when padding is selected
     * collect input up to the block size for decryption
     * if it exceeds the above return error
     */

    if (SF_CRYPTO_CIPHER_OP_MODE_ENCRYPT == p_context->operation_mode)
    {
        err = sf_crypto_cipher_rsa_encrypt_update(p_ctrl, p_data_in, p_data_out);
    }
    else
    {
        err = sf_crypto_cipher_rsa_decrypt_update(p_ctrl, p_data_in, p_data_out);
    }
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Copy the data into the partial block */
    memcpy (p_context->partial_block + p_context->partial_bytes_buffered, p_data_in->p_data, p_data_in->data_length);
    p_context->partial_bytes_buffered += p_data_in->data_length;

    return err;
}

/*******************************************************************************************************************//**
 * @brief   Helper routine to encode the PKCS#1 v1.5 encoding
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 * @param[in, out]  p_data_in               Pointer to the input data structure - has the pointer to
 *                                          input data and the data length
 *
 * @retval          SSP_SUCCESS                 Cipher operation was updated successfully.
 * @retval          SSP_ERR_MESSAGE_TOO_LONG    The input data / message is too long for the given operation.
 * @retval          SSP_ERR_INTERNAL            An internal error occurred during processing.
 * @return                                      See @ref Common_Error_Codes or HAL driver for other possible return
 *                                              codes or causes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_helper_pkcs_1_5_encode_block (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                    sf_crypto_data_handle_t const * const p_data_in)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t key_size_bits = 0U;
    sf_crypto_cipher_rsa_context_t * p_context = (sf_crypto_cipher_rsa_context_t *)p_ctrl->p_cipher_context_buffer;

    err = sf_crypto_cipher_get_rsa_modulus_size_bits(p_ctrl->key_size, &key_size_bits);
    if (SSP_SUCCESS != err)
    {
        return SSP_ERR_INTERNAL;
    }

    if ((p_data_in) &&
        ((p_context->partial_bytes_buffered + p_data_in->data_length) >
        ((uint32_t)SF_CRYPTO_CIPHER_RSA_PKCS_1_5_EB_DATA_SIZE_BYTES(key_size_bits))))
    {
        err = SSP_ERR_MESSAGE_TOO_LONG;
    }
    else
    {
        if ((p_data_in) && (p_data_in->data_length))
        {
            /** Copy any input data into the partial block. */
            memcpy (p_context->partial_block + p_context->partial_bytes_buffered,
                    p_data_in->p_data,
                    p_data_in->data_length);
            p_context->partial_bytes_buffered += p_data_in->data_length;
        }
       err = sf_crypto_cipher_rsa_pkcs_1_5_encode_block(p_ctrl);
    }
    return err;
}


/*******************************************************************************************************************//**
 * @brief   Subroutine to finalize the cipher RSA encrypt operation.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 * @param[in, out]  p_data_in               Pointer to the input data structure - has the pointer to
 *                                          input data and the data length
 * @param[in, out]  p_data_out              Pointer to the output data structure - has the pointer to
 *                                          output data and the data length.

 * @retval          SSP_SUCCESS                 Cipher operation was updated successfully.
 * @retval          SSP_ERR_MESSAGE_TOO_LONG    The input data / message is too long for the given operation.
 * @retval          SSP_ERR_INVALID_SIZE        The out buffer is inadequate to hold the output data.
 * @retval          SSP_ERR_UNSUPPORTED         Unknown padding scheme supplied.
 * @retval          SSP_ERR_INTERNAL            An internal error occurred during processing.
 * @return                                      See @ref Common_Error_Codes or HAL driver for other possible return
 *                                              codes or causes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_encrypt_final (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                     sf_crypto_data_handle_t const * const p_data_in,
                                                     sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_rsa_context_t * p_context = (sf_crypto_cipher_rsa_context_t *)p_ctrl->p_cipher_context_buffer;
    uint32_t key_size_bits = 0U;


    err = sf_crypto_cipher_rsa_validate_encrypt_final_out_params(p_ctrl,
                                                                 p_data_out);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    err = sf_crypto_cipher_get_rsa_modulus_size_bits(p_ctrl->key_size, &key_size_bits);
    if (SSP_SUCCESS != err)
    {
        return SSP_ERR_INTERNAL;
    }

    switch(p_context->padding_scheme)
    {
        case SF_CRYPTO_CIPHER_PADDING_SCHEME_PKCS1_1_5:
            err = sf_crypto_cipher_rsa_helper_pkcs_1_5_encode_block(p_ctrl,
                                                                    p_data_in);

        break;
        case SF_CRYPTO_CIPHER_PADDING_SCHEME_NO_PADDING:
            if ((uint32_t)(SF_CRYPTO_CIPHER_RSA_EB_SIZE_BYTES(key_size_bits)) !=
                (p_context->partial_bytes_buffered + p_data_in->data_length))
            {
                err = SSP_ERR_INVALID_SIZE;
            }
            if ((uint32_t)(SF_CRYPTO_CIPHER_RSA_EB_SIZE_BYTES(key_size_bits)) == (p_context->partial_bytes_buffered + p_data_in->data_length))
            {
                memcpy (p_context->partial_block + p_context->partial_bytes_buffered, p_data_in->p_data, p_data_in->data_length);
                p_context->partial_bytes_buffered += p_data_in->data_length;
            }


        break;
        default:
            err = SSP_ERR_UNSUPPORTED;
        break;
    }
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** At this point we have a block which has to be encrypted by the HAL API*/
    err = sf_crypto_cipher_rsa_hal_encrypt_decrypt(p_ctrl,
                                                   (uint32_t *)p_context->cipher_key,
                                                   NULL,
                                                   (uint32_t)p_context->partial_bytes_buffered /
                                                   SF_CRYPTO_CIPHER_BYTES_PER_WORD,
                                                   (uint32_t *)p_context->partial_block,
                                                   (uint32_t *)p_data_out->p_data);
    if (SSP_SUCCESS == err)
    {
        p_data_out->data_length = p_context->partial_bytes_buffered;
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief   Subroutine to call the HAL RSA encrypt/decrypt API.
 *
 * @param[in, out]  p_ctrl      Pointer to the cipher framework module control block used
 *                              in the open() call.
 * @param[in]       p_key       Pointer to the ket used for the cipher operation.
 * @param[in]       p_domain    Pointer to the domain parameters for the cipher operation - NULL for RSA.
 * @param[in]       num_words   Number of words supplied as the input data.
 * @param[in]       p_input     Pointer to the input data for the cipher operation.
 * @param[in]       p_output    Pointer to the output data for the cipher operation.
 *
 * @retval          SSP_SUCCESS             Cipher operation was updated successfully.
 * @return                                  See @ref Common_Error_Codes or HAL driver for other possible return codes
 *                                          or causes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_hal_encrypt_decrypt (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                           uint32_t *p_key,
                                                           uint32_t *p_domain,
                                                           uint32_t num_words,
                                                           uint32_t *p_input,
                                                           uint32_t *p_output)
{
    rsa_ctrl_t * p_hal_rsa_ctrl = (rsa_ctrl_t *)(p_ctrl->p_hal_ctrl);
    uint32_t hal_err = (uint32_t)SSP_SUCCESS;

    sf_crypto_cipher_rsa_context_t * p_context = (sf_crypto_cipher_rsa_context_t *)p_ctrl->p_cipher_context_buffer;

    if (SF_CRYPTO_CIPHER_OP_MODE_ENCRYPT == (sf_crypto_cipher_op_mode_t)p_context->operation_mode)
    {
        hal_err = ((rsa_api_t *)p_ctrl->p_hal_api)->encrypt((rsa_ctrl_t *)p_hal_rsa_ctrl,
                                                            p_key,
                                                            p_domain,
                                                            num_words,
                                                            p_input,
                                                            p_output);
    }
    else
    {
        if (SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT == p_ctrl->key_type)
        {
            hal_err = ((rsa_api_t *)p_ctrl->p_hal_api)->decryptCrt((rsa_ctrl_t *)(p_ctrl->p_hal_ctrl),
                                                                   p_key,
                                                                   p_domain,
                                                                   num_words,
                                                                   p_input,
                                                                   p_output);
        }
        else
        {
        hal_err = ((rsa_api_t *)p_ctrl->p_hal_api)->decrypt((rsa_ctrl_t *)(p_ctrl->p_hal_ctrl),
                                                            p_key,
                                                            p_domain,
                                                            num_words,
                                                            p_input,
                                                            p_output);
        }

    }

    return (ssp_err_t)hal_err;

}


/*******************************************************************************************************************//**
 * @brief   Subroutine to format the Encryption Block as per the PKCS#1 v1.5 for RSA encrypt operation.
 * The partial data block in the context buffer will be formatted in place to avoid allocating extra memory from
 * the byte pool or creating a block on the stack.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *
 * @retval          SSP_SUCCESS             Cipher operation was updated successfully.
 * @retval          SSP_ERR_INVALID_SIZE    The out buffer is inadequate to hold the output data.
 * @retval          SSP_ERR_INTERNAL        Internal processing error occurred.
 * @return                                  See @ref Common_Error_Codes or HAL driver for other possible return codes
 *                                          or causes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_pkcs_1_5_encode_block (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_rsa_context_t * p_context = (sf_crypto_cipher_rsa_context_t *)p_ctrl->p_cipher_context_buffer;
    uint32_t block_size_bytes = 0U;
    uint8_t random_buffer[SF_CRYPTO_PKCS_1_5_EB_PS_MIN_LENGTH] ={0};
    sf_crypto_data_handle_t random_numbers;
    uint8_t * padding_start = (p_context->partial_block) + (uint32_t)SF_CRYPTO_PKCS_1_5_EB_START_BYTE_LENGTH + (uint32_t)SF_CRYPTO_PKCS_1_5_EB_BT_BYTE_LENGTH;

    /** message_end points to the last byte of the message to be encoded.  */
    uint8_t * message_end = (((p_context->partial_block) + (p_context->partial_bytes_buffered)) - (1U));
    uint8_t * buffer_end;

    random_numbers.p_data = random_buffer;
    random_numbers.data_length = sizeof random_buffer;

    /** Get the encryption block size. */
     err = sf_crypto_cipher_get_rsa_block_size_bytes(p_ctrl->key_size, &block_size_bytes);
     if (SSP_SUCCESS != err)
     {
         return SSP_ERR_INTERNAL;
     }

    /** buffer_end points to the last byte of the EB to be encoded.  */
    buffer_end = (((p_context->partial_block) + block_size_bytes) - (1U));

    /** move the data to the end of the buffer to format the Encryption Block EB in place in the context buffer
     * EB = 00 || BT || PS || 00 || D
     */
    for (uint32_t i = 0U; i < (uint32_t)(p_context->partial_bytes_buffered); i++)
    {
        (*(buffer_end - i)) =  (*(message_end - i));
    }

    /* This sets the separator byte to 0.  */
    memset (p_context->partial_block, 0U, (block_size_bytes - p_context->partial_bytes_buffered));

    *(p_context->partial_block + (uint8_t)SF_CRYPTO_PKCS_1_5_EB_START_BYTE_LENGTH) = (uint8_t)SF_CRYPTO_PKCS_1_5_BT_02;

    /** Now get random numbers to fill the Padding String PS
     * Also ensure that the random numbers do not contain any zeroes.
     */
    uint32_t random_bytes_needed = ((block_size_bytes - p_context->partial_bytes_buffered) -
           ((uint8_t)SF_CRYPTO_PKCS_1_5_EB_OVERHEAD - (uint8_t)SF_CRYPTO_PKCS_1_5_EB_PS_MIN_LENGTH));

    while (random_bytes_needed)
    {
        err = p_ctrl->p_sf_crypto_trng_api->randomNumberGenerate(p_ctrl->p_lower_lvl_sf_crypto_trng_ctrl, &random_numbers);
        if (SSP_SUCCESS != err)
        {
            return err;
        }

        for (uint8_t i = 0U; i < (uint8_t)SF_CRYPTO_PKCS_1_5_EB_PS_MIN_LENGTH; i++)
        {
            if (((uint8_t)(random_numbers.p_data[i]) != 0U) && random_bytes_needed)
            {
                *padding_start = random_numbers.p_data[i];
                padding_start++;
                random_bytes_needed--;
            }
        }
        memset (random_buffer, 0U, sizeof random_buffer);
    }

    p_context->partial_bytes_buffered = block_size_bytes;

    return err;
}

/*******************************************************************************************************************//**
 * @brief   Subroutine to finalize the cipher RSA  decrypt operation.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *                                          in the open() call.
 * @param[in, out]  p_data_in               Pointer to the input data structure - has the pointer to
 *                                          input data and the data length
 * @param[in, out]  p_data_out              Pointer to the output data structure - has the pointer to
 *                                          output data and the data length.

 * @retval          SSP_SUCCESS             Cipher operation was updated successfully.
 * @retval          SSP_ERR_INVALID_SIZE    The out buffer is inadequate to hold the output data.
 * @retval          SSP_ERR_INTERNAL        An internal error occurred during processing.
 * @return                                  See @ref Common_Error_Codes or HAL driver for other possible return codes
 *                                          or causes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_decrypt_final(sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                    sf_crypto_data_handle_t const * const p_data_in,
                                                    sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_rsa_context_t * p_context = (sf_crypto_cipher_rsa_context_t *)p_ctrl->p_cipher_context_buffer;
    sf_crypto_key_size_t local_key_size = p_ctrl->key_size;
    uint32_t key_size_bits = 0U;

    err = sf_crypto_cipher_get_rsa_modulus_size_bits(local_key_size, &key_size_bits);
    if (SSP_SUCCESS != err)
    {
        return SSP_ERR_INTERNAL;
    }

    if ((p_context->partial_bytes_buffered + p_data_in->data_length) !=
         ((uint32_t)SF_CRYPTO_CIPHER_RSA_EB_SIZE_BYTES(key_size_bits)))
    {
        return SSP_ERR_RSA_DECRYPTION_ERROR;
    }
    else
    {
        /** Copy any input data into the partial block. */
        memcpy (p_context->partial_block + p_context->partial_bytes_buffered,
                 p_data_in->p_data,
                 p_data_in->data_length);
        p_context->partial_bytes_buffered += p_data_in->data_length;
    }

    /** At this point we have a block which has to be decrypted by the HAL API*/
    err = sf_crypto_cipher_rsa_hal_encrypt_decrypt(p_ctrl,
                                                   (uint32_t *)p_context->cipher_key,
                                                   NULL,
                                                   (uint32_t)p_context->partial_bytes_buffered /
                                                   SF_CRYPTO_CIPHER_BYTES_PER_WORD,
                                                   (uint32_t *)p_context->partial_block,
                                                   (uint32_t *)p_context->partial_block);
    if (SSP_SUCCESS != err)
    {
        return SSP_ERR_RSA_DECRYPTION_ERROR;
    }



    switch(p_context->padding_scheme)
    {
        case SF_CRYPTO_CIPHER_PADDING_SCHEME_PKCS1_1_5:
            err = sf_crypto_cipher_rsa_pkcs_1_5_decode_block(p_ctrl);
        break;
        case SF_CRYPTO_CIPHER_PADDING_SCHEME_NO_PADDING:
            err = SSP_SUCCESS;
        break;
        default:
            err = SSP_ERR_UNSUPPORTED;
        break;
    }
    if (SSP_SUCCESS == err)
    {
        /** Before copying data into the output buffer ensure that the output buffer is of sufficient size.*/
        if ( p_data_out->data_length < p_context->partial_bytes_buffered)
        {
            return SSP_ERR_INVALID_SIZE;
        }
        memcpy (p_data_out->p_data,
                p_context->partial_block,
                p_context->partial_bytes_buffered);
        p_data_out->data_length = p_context->partial_bytes_buffered;
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief   Subroutine to decode the output of RSA decrypt operation of PKCS 1.5 encoded block.
 *
 * @param[in, out]  p_ctrl                  Pointer to the cipher framework module control block used
 *
 * @retval          SSP_SUCCESS                     Cipher operation was updated successfully.
 * @retval          SSP_ERR_RSA_DECRYPTION_ERROR    Error in decrypting data.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_pkcs_1_5_decode_block (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_rsa_context_t * p_context = (sf_crypto_cipher_rsa_context_t *)p_ctrl->p_cipher_context_buffer;
    uint8_t * encoded_message = p_context->partial_block;
    uint32_t number_random_bytes = 0;

    /** Check if the Encoded Message format is correct */
    if (((uint8_t)SF_CRYPTO_PKCS_1_5_EB_START_BYTE != *(encoded_message)) ||
        ((uint8_t)SF_CRYPTO_PKCS_1_5_BT_02 != *(encoded_message + SF_CRYPTO_PKCS_1_5_EB_START_BYTE_LENGTH)))
    {
        return SSP_ERR_RSA_DECRYPTION_ERROR;
    }

    encoded_message += (SF_CRYPTO_PKCS_1_5_EB_START_BYTE_LENGTH  + SF_CRYPTO_PKCS_1_5_EB_BT_BYTE_LENGTH);

    /** check if the data separator is at least 8 octets away or the Padding string is at least eight octets long. */
    while ((*(encoded_message) != (uint8_t)SF_CRYPTO_PKCS_1_5_EB_DATA_SEPARATOR) &&
            (encoded_message < (p_context->partial_block + p_context->partial_bytes_buffered)))
    {
        number_random_bytes++;
        encoded_message++;
    }
    if (number_random_bytes < SF_CRYPTO_PKCS_1_5_EB_PS_MIN_LENGTH)
    {
        return SSP_ERR_RSA_DECRYPTION_ERROR;
    }

    if (*encoded_message == SF_CRYPTO_PKCS_1_5_EB_DATA_SEPARATOR)
    {
        encoded_message++;
    }
    if (encoded_message < (p_context->partial_block + p_context->partial_bytes_buffered))
    {
        memcpy (p_context->partial_block,
                encoded_message,
                ((uint32_t)(p_context->partial_block + p_context->partial_bytes_buffered) - (uint32_t)encoded_message));
    }
    p_context->partial_bytes_buffered = (uint32_t)((p_context->partial_block + p_context->partial_bytes_buffered) -
                                          (uint32_t)encoded_message);
    return err;

}

/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto  Cipher Framework to validate the output data bufferfor final RSA encryption 
 
 * @retval          SSP_SUCCESS             Validation operation was successful.
 * @retval          SSP_ERR_INVALID_SIZE    The out buffer is inadequate to hold the output data.
 * @retval          SSP_ERR_INTERNAL        An internal error occurred during processing.
 * **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_validate_encrypt_final_out_params (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                         sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t block_size_bytes = 0U;

    /** Get the encryption block size. */
     err = sf_crypto_cipher_get_rsa_block_size_bytes (p_ctrl->key_size, &block_size_bytes);
     if (SSP_SUCCESS != err)
     {
         return SSP_ERR_INTERNAL;
     }
     /** If the output data buffer length is insufficient to hold the encryption data return error.*/
     if (p_data_out->data_length < block_size_bytes)
     {
         return SSP_ERR_INVALID_SIZE;
     }
     return err;
}

/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto  Cipher Framework to call the appropriate routine for final encryption based on
 * the algorithm for the cipherFinal operation.
 * This function is called by SF_CRYPTO_CIPHER_CipherFinal().
 *
 * @param[in,out]   p_ctrl          Control block for this instance of the Cipher framework.
 * @param[in]       p_data_in   Pointer to an input data buffer and the input data length.
 *
 * @retval SSP_SUCCESS              The cipher update operation was successful.
 * @retval SSP_ERR_INVALID_ARGUMENT One or more input parameters are invalid.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_final_input_buffer_check (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                                sf_crypto_data_handle_t const * const p_data_in)
{
    ssp_err_t err = SSP_SUCCESS;

    /** If input data is NULL and no data has been buffered through an update operation, return error
     *  This applies to both encryption and decryption operations.
     *  Hence check at the top before branching.
     */
    if ((SF_CRYPTO_CIPHER_STATE_INITIALIZED == p_ctrl->status) &&
        ((NULL == p_data_in) ||
         (NULL == p_data_in->p_data) ||
         (0U == p_data_in->data_length)))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    /** Check if input data length is non zero but buffer is NULL */
    if ((p_data_in) &&
        (p_data_in->data_length > 0U) &&
        (NULL == p_data_in->p_data))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }
    return err;
} /* End of function sf_crypto_cipher_rsa_final_input_buffer_check */

/*******************************************************************************************************************//**
 * @brief   Routine to process the final encryption / decryption operation for RSA.
 * This function is called by SF_CRYPTO_CIPHER_CipherFinal().
 *
 * @retval SSP_SUCCESS          The cipher update operation was successful.
 * @retval SSP_ERR_UNSUPPORTED  The module does not support the algorithm bbased on the key type specified by the user.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_cipher_rsa_final (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                      sf_crypto_data_handle_t const * const p_data_in,
                                      sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_rsa_context_t * p_context = (sf_crypto_cipher_rsa_context_t *)p_ctrl->p_cipher_context_buffer;

#if SF_CRYPTO_CIPHER_CFG_PARAM_CHECKING_ENABLE
    /** Data will always be output from RSA cipherFinal (for both encrypt and decrypt) operation */
    SSP_ASSERT(p_data_out->p_data);
#endif
    
    err = sf_crypto_cipher_rsa_final_input_buffer_check(p_ctrl, p_data_in);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    switch (p_context->operation_mode)
    {
        case SF_CRYPTO_CIPHER_OP_MODE_ENCRYPT:
            err = sf_crypto_cipher_rsa_encrypt_final(p_ctrl, p_data_in, p_data_out);
        break;
        case SF_CRYPTO_CIPHER_OP_MODE_DECRYPT:
            err = sf_crypto_cipher_rsa_decrypt_final(p_ctrl, p_data_in, p_data_out);
        break;
        default:
            err = SSP_ERR_UNSUPPORTED;
        break;
    }
    /** Clean up the data in the context buffer before exiting. */
    sf_crypto_cipher_rsa_context_buffer_pointer_data_zeroise(p_ctrl);
    return err;
}

/*******************************************************************************************************************//**
 * A subroutine to zeroise the memory pointed to by the fields within the context buffer.
 * NOTE: the buffer sizes pointed to by the original allocation should NOT be cleared.
 * That size will be used when freeing up the memory.
 *
 * @param[in,out]   p_ctrl      Pointer to the Cipher framework module control block.
 **********************************************************************************************************************/
static void sf_crypto_cipher_rsa_context_buffer_pointer_data_zeroise (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    sf_crypto_cipher_rsa_context_t * p_context = (sf_crypto_cipher_rsa_context_t *)p_ctrl->p_cipher_context_buffer;

    if (p_context->cipher_key)
    {
        memset(p_context->cipher_key, 0, p_context->cipher_key_buffer_size);
    }
    if(p_context->partial_block)
    {
        memset(p_context->partial_block, 0, p_context->partial_block_size);
    }
    p_context->cipher_key_length = 0U;
    p_context->partial_bytes_buffered = 0U;

    return;
}

/*******************************************************************************************************************//**
 * A subroutine to release the memory allocated for context buffer.
 * All the data pointed to by the context buffer is first zeroised and freed.
 * Next all elements of the context buffer are zeroised and pointers are set to NULL.
 *
 * @param[in,out]   p_ctrl      The control block for the Cipher module.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_instance_rsa_memory_deallocate(sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;
    sf_crypto_cipher_rsa_context_t * p_context = (sf_crypto_cipher_rsa_context_t *)p_ctrl->p_cipher_context_buffer;

    /** First zeroise the data pointed to from the context buffer
     * and then release the memory resources used for this instance
     */
    sf_crypto_cipher_rsa_context_buffer_pointer_data_zeroise(p_ctrl);

    /** Release the memory allocated for Partial Block */
    sf_crypto_memory_release(p_context->partial_block,
                                   p_context->partial_block_size);

    /** Release the memory allocated for Key */
    sf_crypto_cipher_rsa_private_free_context_buffer_key(p_ctrl);

    /** Zeroise the context buffer and set all pointers to NULL */
    sf_crypto_cipher_rsa_zeroise_context_buffer(p_ctrl);

    /** Release the memory allocated for the context buffer */
    err = sf_crypto_cipher_rsa_private_free_context_buffer(p_ctrl);

    return err;
}

/*******************************************************************************************************************//**
 * A subroutine to init / reset the RSA context buffer.
 * All elements are zeroised and pointers are set to NULL.
 *
 * @param[in,out]   p_ctrl      Pointer to the Cipher control block structure passed in at open().
 **********************************************************************************************************************/
static void sf_crypto_cipher_rsa_zeroise_context_buffer (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    sf_crypto_cipher_rsa_context_t *p_context = (sf_crypto_cipher_rsa_context_t *)p_ctrl->p_cipher_context_buffer;
    memset (p_ctrl->p_cipher_context_buffer, 0, sizeof(sf_crypto_cipher_rsa_context_t));

    p_context->cipher_key = NULL;
    p_context->partial_block = NULL;

} /* End of function sf_crypto_cipher_rsa_zeroise_context_buffer */

/*******************************************************************************************************************//**
 * A subroutine to calculate the RSA key size in bytes.
 *
 * @param[in]       key_type        The enum for key type.
 * @param[in]       key_size        The enum for key size.
 * @param[in, out]  key_size_bytes  The size of the private key in bytes.
 * @retval          SSP_SUCCESS     Key size in bytes returned successfully.
 * @retval          SSP_ERR_INTERNAL    Internal error occurred during processing.
 * @retval          SSP_ERR_INVALID_ARGUMENT    One of the input arguments is invalid.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_get_rsa_pvt_key_size_bytes (sf_crypto_key_type_t key_type,
                                                              sf_crypto_key_size_t key_size,
                                                              uint32_t *key_size_bytes)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t rsa_size_bits = 0U;
    *key_size_bytes = 0U;

    err = sf_crypto_cipher_get_rsa_modulus_size_bits(key_size, &rsa_size_bits);
    if (SSP_SUCCESS != err)
    {
        return SSP_ERR_INTERNAL;
    }

    switch (key_type)
    {
        case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
            *key_size_bytes = RSA_PLAIN_TEXT_PRIVATE_KEY_SIZE_BYTES(rsa_size_bits);
        break;

        case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
            *key_size_bytes = RSA_PLAIN_TEXT_CRT_KEY_SIZE_BYTES(rsa_size_bits);
        break;

        case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
            *key_size_bytes = RSA_WRAPPPED_PRIVATE_KEY_SIZE_BYTES(rsa_size_bits);
        break;
        default:
            err = SSP_ERR_INVALID_ARGUMENT;
        break;
    }
    return err;
}

/*******************************************************************************************************************//**
 * A subroutine to calculate the RSA Public key size in bytes.
 *
 * @param[in]       key_size            enum for key size.
 * @param[in, out]  key_size_bytes      key size in bytes filled on return.
 *
 * @retval          SSP_SUCCESS         Public key is computed and returned successfully.
 * @retval          SSP_ERR_INTERNAL    An error occurred during processing.
 *
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_get_rsa_public_key_size_bytes(sf_crypto_key_size_t key_size,
                                                                uint32_t *key_size_bytes)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t rsa_size_bits = 0U;

    *key_size_bytes = 0U;

    err = sf_crypto_cipher_get_rsa_modulus_size_bits(key_size, &rsa_size_bits);
    if (SSP_SUCCESS != err)
    {
        return SSP_ERR_INTERNAL;
    }

    *key_size_bytes = RSA_PLAIN_TEXT_PUBLIC_KEY_SIZE_BYTES(rsa_size_bits);

    return err;
}

/*******************************************************************************************************************//**
 * A subroutine to calculate the RSA modulus size in bits.
 *
 * @param[in]       key_size        The key size of enum type sf_crypto_key_size_t
 * @param[in, out]  key_size_bits   The RSA Modulus size in bits for the given RSA Key size.
 *
 * @retval  SSP_SUCCESS             The modulus size is returned successfully.
 * @retval SSP_ERR_INVALID_ARGUMENT The key_size input param is invalid.
 *
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_get_rsa_modulus_size_bits(sf_crypto_key_size_t key_size, uint32_t *key_size_bits)
{
    ssp_err_t err = SSP_ERR_INVALID_ARGUMENT;
    *key_size_bits = 0U;

    switch(key_size)
    {
        case SF_CRYPTO_KEY_SIZE_RSA_1024:
            *key_size_bits = (uint32_t)SF_CRYPTO_CIPHER_RSA_1024_MODULUS_BITS;
            err = SSP_SUCCESS;
        break;
        case SF_CRYPTO_KEY_SIZE_RSA_2048:
            *key_size_bits = (uint32_t)SF_CRYPTO_CIPHER_RSA_2048_MODULUS_BITS;
            err = SSP_SUCCESS;
        break;

        default:
            /* Do nothing. SSP_ERR_INVALID_ARGUMENT is returned automatically. */
         break;
    }
    return err;
}

/*******************************************************************************************************************//**
 * A subroutine to calculate the RSA encryption block size / modulus size in bytes.
 * The block size is same as the size of the RSA modulus.
 *
 * @param[in]   key_size            The key size of enum type sf_crypto_key_size_t
 * @param[out]  block_size_bytes    The RSA Block size in bytes for the given RSA Key size.
 *
 * @retval SSP_SUCCESS              The block size is returned successfully.
 * @retval SSP_ERR_INVALID_ARGUMENT The key_size input param is invalid.

 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_get_rsa_block_size_bytes (sf_crypto_key_size_t key_size, uint32_t *block_size_bytes)
{
    ssp_err_t err = SSP_ERR_INVALID_ARGUMENT;
    *block_size_bytes = 0U;

    switch (key_size)
    {
        case SF_CRYPTO_KEY_SIZE_RSA_1024:
            *block_size_bytes = (uint32_t)RSA_MODULUS_SIZE_BYTES(SF_CRYPTO_CIPHER_RSA_1024_MODULUS_BITS);
            err = SSP_SUCCESS;
        break;

        case SF_CRYPTO_KEY_SIZE_RSA_2048:
            *block_size_bytes = (uint32_t)RSA_MODULUS_SIZE_BYTES(SF_CRYPTO_CIPHER_RSA_2048_MODULUS_BITS);
            err = SSP_SUCCESS;
        break;

        default:
            /* Do nothing. SSP_ERR_INVALID_ARGUMENT is returned automatically. */
        break;
    }

    return err;
}

/*******************************************************************************************************************//**
 * A subroutine to get the RSA modulus and block size in bytes.
 *
 * @param[in] p_ctrl        Pointer to the control block for the Cipher module.
 * @param[in] key_size_bits Pointer to the key size in number of bits.
 * @param[in, out]   block_size_bytes   The RSA Block size in bytes for the given RSA Key size in bits.
 * @retval SSP_SUCCESS              The modulus size is returned successfully.
 * @retval SSP_ERR_INVALID_ARGUMENT The key_size input param is invalid.
 * @retval SSP_ERR_INTERNAL         Internal processing error.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_get_modulus_block_sizes (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                               uint32_t *key_size_bits,
                                                               uint32_t *block_size_bytes)
{
    ssp_err_t err = SSP_SUCCESS;
    *key_size_bits = 0U;
    *block_size_bytes = 0U;

    err = sf_crypto_cipher_get_rsa_modulus_size_bits(p_ctrl->key_size, key_size_bits);
    if(SSP_SUCCESS != err)
    {
        return SSP_ERR_INTERNAL;
    }
    err = sf_crypto_cipher_get_rsa_block_size_bytes(p_ctrl->key_size, block_size_bytes);
    if (SSP_SUCCESS != err)
    {
        return SSP_ERR_INTERNAL;
    }

    return err;

}
/*******************************************************************************************************************//**
 * A routine to handle the cipherUpdate - encrypt operation for RSA.
 *
 * Input parameters validation - done at the entry nothing to do here
 * No output on update - so no checks for output buffer / length
 * Collect input up to the block size for encryption when no padding is selected.
 * Collect data up to block size -11 for encryption when padding is selected
 * if it exceeds the above return error
 *
 * @param[in,out]   p_ctrl          The control block for the Cipher module.
 * @param[in]       p_data_in       The pointer to the input data (structure with pointer to data and  data length.)
 * @param[in]       p_data_out       The pointer to the input data (structure with pointer to data and  data length.)
 *
 * @retval          SSP_ERR_INTERNAL    Internal error occurred during processing.
 * @retval          SSP_SUCCESS         Update operation was completed successfully.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_encrypt_update (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                      sf_crypto_data_handle_t const * const p_data_in,
                                                      sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t block_size_bytes = 0U;
    uint32_t key_size_bits = 0U;
    sf_crypto_cipher_rsa_context_t * p_context = (sf_crypto_cipher_rsa_context_t *)p_ctrl->p_cipher_context_buffer;

    err = sf_crypto_cipher_rsa_get_modulus_block_sizes(p_ctrl,
                                                       &key_size_bits,
                                                       &block_size_bytes);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    if (SF_CRYPTO_CIPHER_OP_MODE_ENCRYPT == p_context->operation_mode)
    {
         switch (p_context->padding_scheme)
          {
              case SF_CRYPTO_CIPHER_PADDING_SCHEME_NO_PADDING:
                  if ((p_data_in->data_length + p_context->partial_bytes_buffered) > block_size_bytes)
                  {
                      err = SSP_ERR_MESSAGE_TOO_LONG;
                  }
              break;
              case SF_CRYPTO_CIPHER_PADDING_SCHEME_PKCS1_1_5:
                  if ((p_data_in->data_length + p_context->partial_bytes_buffered) >
                      (uint32_t)SF_CRYPTO_CIPHER_RSA_PKCS_1_5_EB_DATA_SIZE_BYTES(key_size_bits))
                  {
                      err = SSP_ERR_MESSAGE_TOO_LONG;
                  }

              break;
              default:
                  err = SSP_ERR_UNSUPPORTED;
              break;
          }
    }
    p_data_out->data_length = 0U;    /** since no data is output, set output data length to 0. */

    return err;
}/* End of function sf_crypto_cipher_rsa_encrypt_update */

/*******************************************************************************************************************//**
 * A routine to handle the cipherUpdate - decrypt operation for RSA.
 *
 * Input parameters validation - done at the entry nothing to do here
 * No output on update - so no checks for output buffer / length
 * collect input up to the block size for decryption.
 * if it exceeds the above return error
 *
 * @param[in,out]   p_ctrl          The control block for the Cipher module.
 * @param[in]       p_data_in       The pointer to the input data (structure with pointer to data and  data length).
 * @param[in]       p_data_out       The pointer to the input data (structure with pointer to data and  data length).
 * @retval          SSP_SUCCESS     The validation of the init parameters for the RSA cipher operation was successful.
 * @retval          SSP_ERR_MESSAGE_TOO_LONG    The input data / message is too long for the given operation..
 * @retval          SSP_ERR_INVALID_CALL    This call is invalid and no operation can occur.
 * @retval          SSP_ERR_INTERNAL An internal processing error occurred.
  **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_decrypt_update (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                      sf_crypto_data_handle_t const * const p_data_in,
                                                      sf_crypto_data_handle_t * const p_data_out)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t block_size_bytes = 0U;
    sf_crypto_cipher_rsa_context_t * p_context = (sf_crypto_cipher_rsa_context_t *)p_ctrl->p_cipher_context_buffer;

    err = sf_crypto_cipher_get_rsa_block_size_bytes(p_ctrl->key_size, &block_size_bytes);
    if (SSP_SUCCESS != err)
    {
        return SSP_ERR_INTERNAL;
    }

    if ((p_data_in->data_length > 0U) &&
        ((p_context->partial_bytes_buffered + p_data_in->data_length) > block_size_bytes))
    {
        /* Block is already full and cannot take any more input data. Has to be finalized. */
        err = SSP_ERR_MESSAGE_TOO_LONG;
    }
    p_data_out->data_length = 0U;    // since no data is output, set output data length to 0.

    return err;
} /* End of function sf_crypto_cipher_rsa_decrypt_update */

/*******************************************************************************************************************//**
 * A subroutine to validate the parameters for the RSA padding scheme.
 *
 * @param[in]       padding_scheme            Padding mode to be used for Cipher operation
 *                                            for encrypting/ decrypting input data.
 *
 * @retval          SSP_SUCCESS     The validation of the init parameters for the RSA cipher operation was successful.
 * @retval          SSP_ERR_INVALID_ARGUMENT    At least one of the input parameters are invalid.
 * **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_init_validate_padding_scheme (sf_crypto_cipher_padding_scheme_t padding_scheme)
{
    ssp_err_t err = SSP_SUCCESS;
    if ((padding_scheme != SF_CRYPTO_CIPHER_PADDING_SCHEME_NO_PADDING) &&
            (padding_scheme != SF_CRYPTO_CIPHER_PADDING_SCHEME_PKCS1_1_5))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }
    return err;
} /* End of function sf_crypto_cipher_rsa_init_validate_padding_scheme*/

/*******************************************************************************************************************//**
 * A subroutine to validate the parameters for the RSA cipherInit operation.
 *
 * @param[in,out]   p_ctrl                  The control block for the Cipher module.
 * @param[in]       cipher_operation_mode   The operation mode of enum type sf_crypto_cipher_op_mode_t.
 * @param[in]       p_key                   The key for the subsequent cipher operation.
 * @param[in]       p_rsa_params            The parameters specific to the RSA algorithm used for the Cipher operation.
 *
 * @retval          SSP_SUCCESS     The validation of the init parameters for the RSA cipher operation was successful.
 * @retval          SSP_ERR_INVALID_ARGUMENT    At least one of the input parameters are invalid.
 * **********************************************************************************************************************/
static ssp_err_t sf_crypto_cipher_rsa_init_validate (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                     sf_crypto_cipher_op_mode_t cipher_operation_mode,
                                                     sf_crypto_key_t const * const p_key,
                                                     sf_crypto_cipher_rsa_init_params_t * p_rsa_params)
{
    ssp_err_t err = SSP_SUCCESS;
    uint32_t key_size_bytes = 0U;

    /** Validate padding scheme*/
    err = sf_crypto_cipher_rsa_init_validate_padding_scheme(p_rsa_params->padding_scheme);
    if (SSP_SUCCESS != err)
    {
         return SSP_ERR_INVALID_ARGUMENT;
    }

    if (SF_CRYPTO_CIPHER_OP_MODE_DECRYPT == cipher_operation_mode)
    {
        /** Decryption is only supported with the private key. */
         err = sf_crypto_cipher_get_rsa_pvt_key_size_bytes(p_ctrl->key_type,
                                                           p_ctrl->key_size,
                                                           &key_size_bytes);

    }
    else
    {
        /** Encryption is only supported with the public key. */
        err = sf_crypto_cipher_get_rsa_public_key_size_bytes(p_ctrl->key_size, &key_size_bytes);
    }
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Validate key length and buffer */
    if ((p_key->data_length != key_size_bytes) ||
            (NULL == p_key->p_data))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    return err;
}

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
ssp_err_t sf_crypto_cipher_initialize_rsa_instance(sf_crypto_cipher_instance_ctrl_t * p_ctrl,
                                                   sf_crypto_cipher_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;
    err = sf_crypto_cipher_instance_rsa_memory_allocate(p_ctrl);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /* If successful, continue to open sf_crypto_trng as RSA needs TRNG */
    err = sf_crypto_cipher_open_sf_trng_instance(p_ctrl, p_cfg);
    if (SSP_SUCCESS != err)
    {
      sf_crypto_cipher_instance_rsa_memory_deallocate(p_ctrl);
      return err;
    }

    err = sf_crypto_cipher_rsa_hal_open(p_ctrl, p_cfg);
    if (SSP_SUCCESS != err)
    {
        sf_crypto_cipher_close_sf_trng_instance(p_ctrl);
        sf_crypto_cipher_instance_rsa_memory_deallocate(p_ctrl);
    }

    return err;

}

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
ssp_err_t sf_crypto_cipher_deinitialize_rsa_instance(sf_crypto_cipher_instance_ctrl_t * p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;

    /** First close the Framework TRNG instance. */
    err = sf_crypto_cipher_close_sf_trng_instance(p_ctrl);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /** Then close the HAL RSA instance. */
    err = sf_crypto_cipher_rsa_hal_close(p_ctrl);
    if (SSP_SUCCESS == err)
    {
        err = sf_crypto_cipher_instance_rsa_memory_deallocate(p_ctrl);
    }

    return err;
}

/*******************************************************************************************************************//**
 * @} (end defgroup SF_CRYPTO_CIPHER)
 **********************************************************************************************************************/
