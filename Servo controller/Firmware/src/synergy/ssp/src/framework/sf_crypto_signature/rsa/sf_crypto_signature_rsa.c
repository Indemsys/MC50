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
 * File Name    : sf_crypto_signature_rsa.c
 * Description  : Crypto Signature Framework RSA specific implementation.
***********************************************************************************************************************/

/***********************************************************************************************************************
 * @file
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto_api.h"
#include "sf_crypto.h"
#include "sf_crypto_signature_api.h"
#include "sf_crypto_signature.h"
#include "sf_crypto_signature_private_rsa.h"
#include "../sf_crypto_signature_common.h"
#include "r_crypto_api.h"
#include "r_rsa_api.h"
#include "sf_crypto_hash.h"
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define BITS_IN_BYTE                        (8U)
#define SHA_1_DER_ENC_PREFEX_LENGTH         (15U)
#define SHA_224_DER_ENC_PREFEX_LENGTH       (19U)
#define SHA_256_DER_ENC_PREFEX_LENGTH       (19U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_interface_get_rsa(sf_crypto_signature_instance_ctrl_t * const p_ctrl);
static
ssp_err_t
sf_crypto_signature_update_operation_rsa(sf_crypto_signature_instance_ctrl_t * const p_ctrl,
                                         sf_crypto_data_handle_t const * const p_update_data);
static
ssp_err_t
sf_crypto_signature_validate_plain_text_private_key_size_rsa(sf_crypto_signature_instance_ctrl_t * const p_ctrl,
                                                             uint32_t key_length);
static
ssp_err_t
sf_crypto_signature_validate_plain_text_crt_private_key_size_rsa(sf_crypto_signature_instance_ctrl_t * const p_ctrl,
                                                                 uint32_t key_length);
static
ssp_err_t
sf_crypto_signature_validate_wrapped_private_key_size_rsa(sf_crypto_signature_instance_ctrl_t * const p_ctrl,
                                                          uint32_t key_length);
static
ssp_err_t
sf_crypto_signature_validate_private_key_size_rsa(sf_crypto_signature_instance_ctrl_t * const p_ctrl,
                                                  uint32_t key_length);
static
ssp_err_t
sf_crypto_signature_validate_public_key_size_rsa(sf_crypto_signature_instance_ctrl_t * const p_ctrl,
                                                 uint32_t key_length);
static
ssp_err_t
sf_crypto_signature_validate_key_size_rsa(sf_crypto_signature_instance_ctrl_t * const p_ctrl,
                                          sf_crypto_signature_mode_t  operation_mode,
                                          uint32_t key_length);
static
uint32_t
sf_crypto_signature_get_block_size_rsa(sf_crypto_signature_instance_ctrl_t * const p_ctrl);
static
uint32_t
sf_crypto_signature_get_private_key_size(sf_crypto_signature_instance_ctrl_t * const p_ctrl);
static
ssp_err_t
sf_crypto_signature_data_formatting_der_encoding(sf_crypto_signature_instance_ctrl_t * const p_ctrl);
static
ssp_err_t
sf_crypto_signature_der_enc_data_formatting_to_pkcs1_v_1_5(sf_crypto_signature_instance_ctrl_t * const p_ctrl);
static
ssp_err_t
sf_crypto_signature_allocate_hal_memory_and_get_interface(sf_crypto_signature_instance_ctrl_t * const p_ctrl,
                                                          sf_crypto_signature_cfg_t const * const p_cfg);
static
ssp_err_t
sf_crypto_signature_open_and_init_hash(sf_crypto_signature_instance_ctrl_t * const p_ctrl);
static
ssp_err_t
sf_crypto_signature_final_and_close_hash (sf_crypto_signature_instance_ctrl_t * const p_ctrl);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
static const uint8_t sha1_der_enc_prefex[SHA_1_DER_ENC_PREFEX_LENGTH] =
    {
     0x30U, 0x21U, 0x30U, 0x09U, 0x06U,
     0x05U, 0x2bU, 0x0eU, 0x03U, 0x02U,
     0x1aU, 0x05U, 0x00U, 0x04U, 0x14U
    };
static const uint8_t sha224_der_enc_prefex[SHA_224_DER_ENC_PREFEX_LENGTH] =
    {
     0x30U, 0x2dU, 0x30U, 0x0dU, 0x06U,
     0x09U, 0x60U, 0x86U, 0x48U, 0x01U,
     0x65U, 0x03U, 0x04U, 0x02U, 0x04U,
     0x05U, 0x00U, 0x04U, 0x1CU
    };
static const uint8_t sha256_der_enc_prefex[SHA_256_DER_ENC_PREFEX_LENGTH] =
    {
     0x30U, 0x31U, 0x30U, 0x0dU, 0x06U,
     0x09U, 0x60U, 0x86U, 0x48U, 0x01U,
     0x65U, 0x03U, 0x04U, 0x02U, 0x01U,
     0x05U, 0x00U, 0x04U, 0x20U
    };
/***********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup SF_Library
 * @addtogroup SF_CRYPTO_SIGNATURE
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
Functions
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to check configuration params - Key size.
 * This function is called by sf_crypto_signature_validate_config().
 *
 * @param[in]      p_cfg            Pointer to sf_crypto_signature_cfg_t configuration structure.
 *
 * @retval SSP_SUCCESS              Valid RSA Key size.
 * @retval SSP_ERR_INVALID_ARGUMENT Invalid RSA Key size.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_signature_key_size_config_rsa (sf_crypto_signature_cfg_t const * const  p_cfg)
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
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to open the RSA HAL driver.
 * This function is called by sf_crypto_signature_hal_open().
 *
 * @param[in,out]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]      p_cfg            Pointer to sf_crypto_signature_cfg_t configuration structure.
 *
 * @retval SSP_SUCCESS              The module was successfully opened.
 * @retval SSP_ERR_INTERNAL         Critical internal error.
 * @return                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_signature_open_rsa (sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                        sf_crypto_signature_cfg_t const * const p_cfg)
{
    ssp_err_t                         err                 = SSP_SUCCESS;
    sf_crypto_instance_ctrl_t         * p_ctrl_common;
    rsa_cfg_t                         hal_cfg;
    uint32_t                          block_size          = 0U;
    uint32_t                          private_key_size    = 0U;

    /* Get a Crypto common control block and the interface. */
    p_ctrl->p_lower_lvl_common_ctrl = (sf_crypto_instance_ctrl_t *)p_cfg->p_lower_lvl_crypto_common->p_ctrl;
    p_ctrl_common = p_ctrl->p_lower_lvl_common_ctrl;

    err = sf_crypto_signature_allocate_hal_memory_and_get_interface(p_ctrl, p_cfg);
    if (SSP_SUCCESS != err)
    {
        return (err);
    }

    /* Calculate size of signature block based on algorithm */
    block_size = sf_crypto_signature_get_block_size_rsa(p_ctrl);
    if (0U == block_size)
    {
        memset(p_ctrl->p_hal_ctrl, 0U, (uint32_t)sizeof(rsa_ctrl_t));
        sf_crypto_memory_release(p_ctrl->p_hal_ctrl, (uint32_t)sizeof(rsa_ctrl_t));
        return SSP_ERR_INTERNAL;
    }
    /* Allocate 1 block memory for message buffer. */
    err = sf_crypto_memory_allocate  (&p_ctrl_common->byte_pool,
                                     (void **)(&p_ctrl->operation_context.buffer.p_data),
                                     block_size,
                                     p_ctrl_common->wait_option);
    if ( (err != SSP_SUCCESS) || (NULL == (void *)p_ctrl->operation_context.buffer.p_data))
    {
        /* Release memory allocated for HAL control block as Open has failed. */
        memset(p_ctrl->p_hal_ctrl, 0U, (uint32_t)sizeof(rsa_ctrl_t));
        sf_crypto_memory_release(p_ctrl->p_hal_ctrl, (uint32_t)sizeof(rsa_ctrl_t));
        return err;
    }
    memset(p_ctrl->operation_context.buffer.p_data, 0U, (size_t)block_size);

    /* Calculate size of private key */
    private_key_size = sf_crypto_signature_get_private_key_size(p_ctrl);
    if (0U == private_key_size)
    {
        memset(p_ctrl->operation_context.buffer.p_data, 0U , block_size);
        /*LDRA_NOANALYSIS Below rule not working */
        /*LDRA_INSPECTED 458 S This is not an implicit conversion. */
        sf_crypto_memory_release((void *)p_ctrl->operation_context.buffer.p_data, block_size);
        memset(p_ctrl->p_hal_ctrl, 0U, (uint32_t)sizeof(rsa_ctrl_t));
        /* Release memory allocated for HAL control block as Open has failed. */
        /*LDRA_INSPECTED 458 S This is not an implicit conversion. */
        sf_crypto_memory_release(p_ctrl->p_hal_ctrl, (uint32_t)sizeof(rsa_ctrl_t));
        /*LDRA_ANALYSIS */
        return SSP_ERR_INTERNAL;
    }
    /* Allocate public_key_size for key buffer. */
    err = sf_crypto_memory_allocate (&p_ctrl_common->byte_pool,
                                    (void **)(&p_ctrl->operation_context.p_key_data),
                                    private_key_size,
                                    p_ctrl_common->wait_option);
    if ( (err != SSP_SUCCESS) || (NULL == (void *)p_ctrl->operation_context.p_key_data))
    {
       memset(p_ctrl->operation_context.buffer.p_data, 0U , block_size);
       /*LDRA_NOANALYSIS Below rule not working */
       /*LDRA_INSPECTED 458 S This is not an implicit conversion. */
       sf_crypto_memory_release((void *)p_ctrl->operation_context.buffer.p_data, block_size);
       /* Release memory allocated for HAL control block as Open has failed. */
       memset(p_ctrl->p_hal_ctrl, 0U, (uint32_t)sizeof(rsa_ctrl_t));
       /*LDRA_INSPECTED 458 S This is not an implicit conversion. */
       sf_crypto_memory_release(p_ctrl->p_hal_ctrl, (uint32_t)sizeof(rsa_ctrl_t));
       /*LDRA_ANALYSIS */
       return err;
    }
    memset(p_ctrl->operation_context.p_key_data, 0U, (size_t)block_size);

    /* Allocate memory block for algorithm specific parameter structure */
    err = sf_crypto_memory_allocate (&p_ctrl_common->byte_pool,
                                    (void **)(&p_ctrl->operation_context.p_aglorithm_specific_params),
                                    (uint32_t)sizeof(sf_crypto_signature_rsa_specific_params_t),
                                    p_ctrl_common->wait_option);
    if ( (err != SSP_SUCCESS) || (NULL == (void *)p_ctrl->operation_context.p_aglorithm_specific_params))
    {
       memset(p_ctrl->operation_context.p_key_data, 0U , private_key_size);
       /*LDRA_NOANALYSIS Below rule not working */
       /*LDRA_INSPECTED 458 S This is not an implicit conversion. */
       sf_crypto_memory_release((void *)p_ctrl->operation_context.p_key_data, private_key_size);
       memset(p_ctrl->operation_context.buffer.p_data, 0U , block_size);
       /*LDRA_INSPECTED 458 S This is not an implicit conversion. */
       sf_crypto_memory_release((void *)p_ctrl->operation_context.buffer.p_data, block_size);
       /* Release memory allocated for HAL control block as Open has failed. */
       memset(p_ctrl->p_hal_ctrl, 0U, (uint32_t)sizeof(rsa_ctrl_t));
       sf_crypto_memory_release(p_ctrl->p_hal_ctrl, (uint32_t)sizeof(rsa_ctrl_t));
       /*LDRA_ANALYSIS */
       return err;
    }
    memset(p_ctrl->operation_context.p_aglorithm_specific_params, 0U, sizeof(sf_crypto_signature_rsa_specific_params_t));

    /* Set Crypto HAL API instance with the control common hardware api. */
    hal_cfg.p_crypto_api = ((crypto_instance_t *)p_ctrl_common->p_lower_lvl_crypto)->p_api;

    err  = (ssp_err_t)((rsa_api_t *)p_ctrl->p_hal_api)->open(p_ctrl->p_hal_ctrl, &hal_cfg);

    return err;
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to close the RSA HAL driver.
 * This function is called by sf_crypto_signature_hal_close().
 *
 * @param[in,out]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 *
 * @retval SSP_SUCCESS              The module was successfully closed.
 * @retval SSP_ERR_INTERNAL         Critical internal error.
 * @return                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_signature_close_rsa (sf_crypto_signature_instance_ctrl_t * p_ctrl)
{
    ssp_err_t    err                 = SSP_SUCCESS;
    void         * p_tmp             = p_ctrl->p_hal_ctrl;
    uint32_t     block_size          = 0U;
    uint32_t     private_key_size    = 0U;
    sf_crypto_hash_instance_ctrl_t *p_ctrl_hash =
                                   (sf_crypto_hash_instance_ctrl_t *)p_ctrl->p_lower_lvl_sf_crypto_hash->p_ctrl;
    /* Close Hash module */
    err = p_ctrl->p_lower_lvl_sf_crypto_hash->p_api->close(p_ctrl_hash);
    if ((SSP_ERR_NOT_OPEN != err) && (SSP_SUCCESS != err))
    {
        return err;
    }
    /* Release memory allocated for holding algorithm specific params. */
    memset(p_ctrl->operation_context.p_aglorithm_specific_params, 0U, sizeof(sf_crypto_signature_rsa_specific_params_t));
    err = sf_crypto_memory_release((void *)(p_ctrl->operation_context.p_aglorithm_specific_params),
                                   (uint32_t)sizeof(sf_crypto_signature_rsa_specific_params_t));
    if ( err != SSP_SUCCESS)
    {
        return err;
    }
    /* Calculate private key size. */
    private_key_size = sf_crypto_signature_get_private_key_size(p_ctrl);
    if(0U == private_key_size)
    {
        return SSP_ERR_INTERNAL;
    }
    memset(p_ctrl->operation_context.p_key_data, 0U, private_key_size);
    err = sf_crypto_memory_release((void *)(p_ctrl->operation_context.p_key_data),
                                   private_key_size);
    if ( err != SSP_SUCCESS)
    {
        return err;
    }

    /* Calculate size of signature block based on algorithm */
    block_size = sf_crypto_signature_get_block_size_rsa(p_ctrl);
    if(0U == block_size)
    {
        return SSP_ERR_INTERNAL;
    }
    memset(p_ctrl->operation_context.buffer.p_data, 0U , block_size);
    /* Release context buffer block memory */
    /*LDRA_NOANALYSIS Below rule not working */
    /*LDRA_INSPECTED 458 S This is not an implicit conversion. */
    err = sf_crypto_memory_release((void *)p_ctrl->operation_context.buffer.p_data, block_size);
    /*LDRA_ANALYSIS */
    if ( err != SSP_SUCCESS)
    {
        return err;
    }
    err  = (ssp_err_t)((rsa_api_t *)p_ctrl->p_hal_api)->close(p_ctrl->p_hal_ctrl);
    if (err == SSP_SUCCESS)
    {
        memset(p_ctrl->p_hal_ctrl, 0U, (uint32_t)sizeof(rsa_ctrl_t));
        /* Release the control block memory back to byte pool */
        err = sf_crypto_memory_release(p_tmp, (uint32_t)sizeof(rsa_ctrl_t));
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to initialize the Signature framework module context.
 * This function is called by sf_crypto_signature_hal_context_init().
 *
 * @param[in,out]  p_ctrl                       Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]      operation_mode               Perform Sign Or Verify operation.
 * @param[in]      p_algorithm_specific_params  Algorithm specific params.
 * @param[in]      p_key                        Private key if sign operation is to be performed.
 *                                              Public key if verify operation is to be performed.
 *
 * @retval SSP_SUCCESS                              The module context was successfully initialized.
 * @retval SSP_ERR_INTERNAL                         Critical internal error.
 * @retval SSP_ERR_INVALID_CALL                     Invalid call to this API.
 * @retval SSP_ERR_UNSUPPORTED                      Invalid Hash module request.
 * @retval SSP_ERR_CRYPTO_INVALID_OPERATION_MODE    Invalid operation mode requested.
 * @return                                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_signature_context_init_rsa (sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                                sf_crypto_signature_mode_t operation_mode,
                                                sf_crypto_signature_algorithm_init_params_t * const p_algorithm_specific_params,
                                                sf_crypto_key_t const * const p_key)
{
    ssp_err_t    err         = SSP_SUCCESS;
    uint32_t     block_size  = 0U;
    sf_crypto_signature_rsa_specific_params_t * rsa_specific_params =
            (sf_crypto_signature_rsa_specific_params_t *)p_ctrl->operation_context.p_aglorithm_specific_params;
    /* Verify State Transition. This check needs to be done at this level as until now we
     * do not know the operation mode.*/
    if (SF_CRYPTO_SIGNATURE_MODE_SIGN == operation_mode)
    {
        err = sf_crypto_signature_validate_sign_operation_state_transition(p_ctrl,
                                                                       SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_INITIALIZED);
    }
    else
    {
        err = sf_crypto_signature_validate_verify_operation_state_transition(p_ctrl,
                                                                       SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_INITIALIZED);
    }

    if (SSP_SUCCESS != err)
    {
        return (err);
    }

    /* Validate key length */
    err = sf_crypto_signature_validate_key_size_rsa(p_ctrl, operation_mode, p_key->data_length);
    if (SSP_SUCCESS != err)
    {
        return (err);
    }

    /* Set up context for sign/verify operation */
    p_ctrl->operation_context.operation_mode = operation_mode;
    memcpy(rsa_specific_params, p_algorithm_specific_params, sizeof(sf_crypto_signature_rsa_specific_params_t));
    p_ctrl->operation_context.key_data_length = p_key->data_length;
    memset(&p_ctrl->operation_context.p_key_data[0], 0U, p_ctrl->operation_context.key_data_length);
    memcpy(&p_ctrl->operation_context.p_key_data[0], p_key->p_data, p_ctrl->operation_context.key_data_length);

    /* Calculate size of signature block based on algorithm */
    block_size = sf_crypto_signature_get_block_size_rsa(p_ctrl);

    if(0U == block_size)
    {
        return SSP_ERR_INTERNAL;
    }
    else
    {
        memset(p_ctrl->operation_context.buffer.p_data, 0U, (size_t)block_size);
        p_ctrl->operation_context.buffer.data_length = 0U;
    }

    /* Open Hash module only when needed. */
    if (SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_NONE != rsa_specific_params->message_format)
    {
        err = sf_crypto_signature_open_and_init_hash(p_ctrl);
    }
    /* Set Operation mode */
    if (SSP_SUCCESS == err)
    {
        if (SF_CRYPTO_SIGNATURE_MODE_SIGN == p_ctrl->operation_context.operation_mode)
        {
            p_ctrl->operation_state = SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_INITIALIZED;
        }
        else
        {
            p_ctrl->operation_state = SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_INITIALIZED;
        }
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to perform sign update operation.
 * This function is called by sf_crypto_signature_hal_sign_update().
 *
 * @param[in,out]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]      p_message        Pointer to data handle containing update data and its length.
 *
 * @retval SSP_SUCCESS                              Sign update was performed successfully.
 * @retval SSP_ERR_ASSERTION                        Critical internal error.
 * @retval SSP_ERR_UNSUPPORTED                      Invalid Hash module request.
 * @retval SSP_ERR_CRYPTO_BUF_OVERFLOW              Update data exceeded the block size.
 * @retval SSP_ERR_CRYPTO_INVALID_OPERATION_MODE    Invalid operation mode requested.
 * @return                                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_signature_sign_update_rsa (sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                               sf_crypto_data_handle_t const * const p_message)
{
    ssp_err_t                               err             = SSP_SUCCESS;
    if (SF_CRYPTO_SIGNATURE_MODE_SIGN != p_ctrl->operation_context.operation_mode)
    {
        return (SSP_ERR_CRYPTO_INVALID_OPERATION_MODE);
    }
    /* Call helper function to perform update operation. */
    err = sf_crypto_signature_update_operation_rsa(p_ctrl, p_message);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to perform verify update operation.
 * This function is called by sf_crypto_signature_hal_verify_update().
 *
 * @param[in,out]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]      p_message        Pointer to data handle containing message in appropriate format and its length.
 *
 * @retval SSP_SUCCESS                              Verify update operation was performed successfully.
 * @retval SSP_ERR_ASSERTION                        Critical internal error.
 * @retval SSP_ERR_UNSUPPORTED                      Invalid Hash module request.
 * @retval SSP_ERR_CRYPTO_BUF_OVERFLOW              Update data exceeded the block size.
 * @retval SSP_ERR_CRYPTO_INVALID_OPERATION_MODE    Invalid operation mode requested.
 * @return                                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_signature_verify_update_rsa (sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                                 sf_crypto_data_handle_t const * const p_message)
{
    ssp_err_t    err = SSP_SUCCESS;

    if (SF_CRYPTO_SIGNATURE_MODE_VERIFY != p_ctrl->operation_context.operation_mode)
    {
        return (SSP_ERR_CRYPTO_INVALID_OPERATION_MODE);
    }
    /* Call helper function to perform update operation. */
    err = sf_crypto_signature_update_operation_rsa(p_ctrl, p_message);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to perform sign final operation.
 * This function is called by sf_crypto_signature_hal_sign_final().
 *
 * @param[in,out]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]      p_message        Pointer to data handle containing last block of data and its length.
 *                                  If there is no more data to be passed this param can be set to NULL.
 * @param[in,out]  p_dest           Pointer to data handle containing pointer to a buffer for storing signature.
 *                                  The data_length of this handle must be populated with the buffer length.
 *                                  Upon successful return this data_length will be updated with the number
 *                                  of bytes written to this buffer.
 *
 * @retval SSP_SUCCESS                              Sign Final operation was performed successfully.
 * @retval SSP_ERR_INTERNAL                         Critical internal error.
 * @retval SSP_ERR_CRYPTO_INVALID_SIZE              Not enough space to store signature.
 * @retval SSP_ERR_UNSUPPORTED                      Invalid Hash module request.
 * @retval SSP_ERR_CRYPTO_BUF_OVERFLOW              Update data exceeded the block size.
 * @return                                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_signature_sign_final_rsa (sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                              sf_crypto_data_handle_t const * const p_message,
                                              sf_crypto_data_handle_t * const p_dest)
{
    ssp_err_t    err         = SSP_SUCCESS;
    uint32_t     block_size  = 0U;
    sf_crypto_signature_rsa_specific_params_t * rsa_specific_params =
            (sf_crypto_signature_rsa_specific_params_t *)p_ctrl->operation_context.p_aglorithm_specific_params;
    block_size = sf_crypto_signature_get_block_size_rsa(p_ctrl);
    if (block_size > p_dest->data_length)
    {
        return(SSP_ERR_CRYPTO_INVALID_SIZE);
    }
    /* Check for last block of message. */
    if (NULL != p_message)
    {
        err = sf_crypto_signature_update_operation_rsa(p_ctrl, p_message);
        if (SSP_SUCCESS != err)
        {
            return err;
        }

    }
    if (SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_NONE != rsa_specific_params->message_format)
    {
        err = sf_crypto_signature_final_and_close_hash(p_ctrl);
        if (SSP_SUCCESS != err)
        {
            return (err);
        }

        /* Format Data */

        /* Generate DER Encoding */
        /* At this point the buffer contains H, where H is the Hash */
        err = sf_crypto_signature_data_formatting_der_encoding(p_ctrl);
        if (SSP_SUCCESS != err)
        {
            return (err);
        }
        /* At this point the buffer looks like 0x00 ... ||T, where T = DER_HASH_PREFEX||H */
        err = sf_crypto_signature_der_enc_data_formatting_to_pkcs1_v_1_5(p_ctrl);
        if (SSP_SUCCESS != err)
        {
            return (err);
        }
    }
    /* For SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_NONE directly call the HAL API */
    if (SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT != p_ctrl->key_type)
    {
        err  = (ssp_err_t)((rsa_api_t *)p_ctrl->p_hal_api)->sign(p_ctrl->p_hal_ctrl,
                                                                 (uint32_t*)&p_ctrl->operation_context.p_key_data[0],
                                                                 NULL,
                                                                 (p_ctrl->operation_context.buffer.data_length / 4U),
                                                                 (uint32_t *)p_ctrl->operation_context.buffer.p_data,
                                                                 (uint32_t *)p_dest->p_data);
    }
    else
    {
        err  = (ssp_err_t)((rsa_api_t *)p_ctrl->p_hal_api)->signCrt(p_ctrl->p_hal_ctrl,
                                                                    (uint32_t*)&p_ctrl->operation_context.p_key_data[0],
                                                                    NULL,
                                                                    (p_ctrl->operation_context.buffer.data_length / 4U),
                                                                    (uint32_t *)p_ctrl->operation_context.buffer.p_data,
                                                                    (uint32_t *)p_dest->p_data);
    }
    /* Update data length with number of bytes written to user buffer */
    if (SSP_SUCCESS == err)
    {
        p_dest->data_length = block_size;
    }
    return err;
}

/*******************************************************************************************************************//**
 * @brief   Function for Crypto Signature Framework to perform verify final operation.
 * This function is called by sf_crypto_signature_hal_verify_final().
 *
 * @param[in,out]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]      p_signature      Pointer to data handle containing signature and its length.
 * @param[in]      p_message        Pointer to data handle containing message in appropriate format and its length.
 *
 * @retval SSP_SUCCESS                              Verify Final operation was performed successfully.
 * @retval SSP_ERR_INTERNAL                         Critical internal error.
 * @retval SSP_ERR_CRYPTO_INVALID_SIZE              Invalid signature length.
 * @retval SSP_ERR_UNSUPPORTED                      Invalid Hash module request.
 * @retval SSP_ERR_CRYPTO_BUF_OVERFLOW              Update data exceeded the block size.
 * @return                                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_signature_verify_final_rsa (sf_crypto_signature_instance_ctrl_t * const  p_ctrl,
                                                sf_crypto_data_handle_t const * const p_signature,
                                                sf_crypto_data_handle_t const * const p_message)
{
    ssp_err_t    err             = SSP_SUCCESS;
    uint32_t     block_size      = 0U;
    sf_crypto_signature_rsa_specific_params_t * rsa_specific_params =
            (sf_crypto_signature_rsa_specific_params_t *)p_ctrl->operation_context.p_aglorithm_specific_params;
    block_size = sf_crypto_signature_get_block_size_rsa(p_ctrl);
    if (block_size != p_signature->data_length)
    {
        return(SSP_ERR_CRYPTO_INVALID_SIZE);
    }
    /* Check for last block of message. */
    if (NULL != p_message)
    {
        err = sf_crypto_signature_update_operation_rsa(p_ctrl, p_message);
        if (SSP_SUCCESS != err)
        {
            return err;
        }
    }
    if (SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_NONE != rsa_specific_params->message_format)
    {
        err = sf_crypto_signature_final_and_close_hash(p_ctrl);
        if (SSP_SUCCESS != err)
        {
            return (err);
        }

        /* Format Data */

        /* Generate DER Encoding */
        /* At this point the buffer contains H, where H is the Hash */
        err = sf_crypto_signature_data_formatting_der_encoding(p_ctrl);
        if (SSP_SUCCESS != err)
        {
            return (err);
        }
        /* At this point the buffer looks like 0x00 ... ||T, where T = DER_HASH_PREFEX||H */
        err = sf_crypto_signature_der_enc_data_formatting_to_pkcs1_v_1_5(p_ctrl);
        if (SSP_SUCCESS != err)
        {
            return (err);
        }
    }
    /* For SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_NONE directly call the HAL API */
    err  = (ssp_err_t)((rsa_api_t *)p_ctrl->p_hal_api)->verify(p_ctrl->p_hal_ctrl,
                                                               (uint32_t*)&p_ctrl->operation_context.p_key_data[0],
                                                               NULL,
                                                               (p_ctrl->operation_context.buffer.data_length / 4U),
                                                               (uint32_t *)p_signature->p_data,
                                                               (uint32_t *)p_ctrl->operation_context.buffer.p_data);
    if ( SSP_SUCCESS != err)
    {
        return err;
    }
    return err;
}

/*******************************************************************************************************************//**
 * @} (end defgroup SF_CRYPTO_SIGNATURE)
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief   Function to get RSA interface.
 * This function is called by sf_crypto_signature_open_rsa().
 *
 * @param[in,out]  p_ctrl                 Pointer to Crypto Signature Framework instance control block structure.
 *
 * @retval SSP_SUCCESS                    RSA HAL API interface was returned successfully.
 * @return                                See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_signature_interface_get_rsa (sf_crypto_signature_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t                       err               = SSP_SUCCESS;
    crypto_instance_t             * p_crypto;
    sf_crypto_instance_ctrl_t     * p_ctrl_common;
    crypto_interface_get_param_t    rsa_interface_info;
    /* Get RSA HAL API interface instance structure */
    rsa_interface_info.algorithm_type = CRYPTO_ALGORITHM_TYPE_RSA;
    /* Check the RSA key type. */
    switch(p_ctrl->key_type)
    {
        case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
        {
            rsa_interface_info.key_type = CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT;
        break;
        }
        case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
        {
            rsa_interface_info.key_type = CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT;
        break;
        }
        case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
        {
            rsa_interface_info.key_type = CRYPTO_KEY_TYPE_RSA_WRAPPED;
        break;
        }
        default:
        {
            rsa_interface_info.key_type = CRYPTO_KEY_TYPE_MAX;
        break;
        }
    }
    /* Check the RSA key type. */
    switch(p_ctrl->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_RSA_1024:
        {
            rsa_interface_info.key_size = CRYPTO_KEY_SIZE_RSA_1024;
        break;
        }
        case SF_CRYPTO_KEY_SIZE_RSA_2048:
        {
            rsa_interface_info.key_size = CRYPTO_KEY_SIZE_RSA_2048;
        break;
        }
        default:
        {
            rsa_interface_info.key_size = CRYPTO_KEY_SIZE_MAX;
        break;
        }
    }
    p_ctrl_common = p_ctrl->p_lower_lvl_common_ctrl;
    p_crypto = (crypto_instance_t *)p_ctrl_common->p_lower_lvl_crypto;
    err = (ssp_err_t)(p_crypto->p_api)->interfaceGet(&rsa_interface_info, &p_ctrl->p_hal_api);
    return (err);
}

/*******************************************************************************************************************//**
 * @brief   Common function to perform update operation.
 * This function is called by sf_crypto_signature_sign_update_rsa and sf_crypto_signature_verify_update_rsa.
 *
 * @param[in,out]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]      p_update_data    Pointer to data handle of update data.
 * @retval SSP_SUCCESS                              Update (for sign / verify) operation was successful.
 * @retval SSP_ERR_UNSUPPORTED                      Invalid Operation mode.
 * @retval SSP_ERR_CRYPTO_BUF_OVERFLOW              Update data exceeded the block size.
 * @return                                          See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_update_operation_rsa (sf_crypto_signature_instance_ctrl_t * const p_ctrl,
                                          sf_crypto_data_handle_t const * const p_update_data)
{
    ssp_err_t iret = SSP_SUCCESS;
    uint32_t curr_cursor = 0U;
    uint32_t block_size = 0U;
    sf_crypto_signature_context_t * p_operation_context = (sf_crypto_signature_context_t *)&p_ctrl->operation_context;
    sf_crypto_signature_rsa_specific_params_t * rsa_specific_params =
            (sf_crypto_signature_rsa_specific_params_t *)p_operation_context->p_aglorithm_specific_params;
    /* Place cursor to the next available position */
    block_size = sf_crypto_signature_get_block_size_rsa(p_ctrl);
    curr_cursor = p_operation_context->buffer.data_length;
    switch(rsa_specific_params->message_format)
    {
        case SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_NONE:
        {
            /* Check if there is enough space in buffer. */
            if ((block_size - curr_cursor) >= p_update_data->data_length)
            {
                /* Copy data */
                memcpy(p_operation_context->buffer.p_data + curr_cursor, p_update_data->p_data, p_update_data->data_length);
                p_operation_context->buffer.data_length += p_update_data->data_length;
            }
            else
            {
                iret = SSP_ERR_CRYPTO_BUF_OVERFLOW;
            }
        break;
        }
        /* Fall through here is intentional */
        case SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_RSA_SHA1_PKCS1_1_5:
        case SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_RSA_SHA224_PKCS1_1_5:
        case SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_RSA_SHA256_PKCS1_1_5:
        {
            /* Compute Hash of input message */
            p_ctrl->p_lower_lvl_sf_crypto_hash->p_api->hashUpdate(p_ctrl->p_lower_lvl_sf_crypto_hash->p_ctrl,
                                                                  p_update_data);
            /* No need to update data length here. This hash framework module will take care of this. */
        break;
        }
        default:
        {
            iret = SSP_ERR_UNSUPPORTED;
        break;
        }
    }
    return (iret);
}

/*******************************************************************************************************************//**
 * @brief   Function to validate RSA plain-text private key length.
 *
 * @param[in]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]  key_length       Length of input key.
 *
 * @retval SSP_ERR_CRYPTO_INVALID_SIZE        Invalid Key Length.
 * @retval SSP_SUCCESS                        Valid Key Length.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_validate_plain_text_private_key_size_rsa (sf_crypto_signature_instance_ctrl_t * const p_ctrl,
                                                              uint32_t key_length)
{
    ssp_err_t error = SSP_ERR_CRYPTO_INVALID_SIZE;
    switch(p_ctrl->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_RSA_1024:
        {
            error = (RSA_PLAIN_TEXT_1024_PRIVATE_KEY_SIZE_BYTES == key_length)
                    ? SSP_SUCCESS : SSP_ERR_CRYPTO_INVALID_SIZE;
        break;
        }
        case SF_CRYPTO_KEY_SIZE_RSA_2048:
        {
            error = (RSA_PLAIN_TEXT_2048_PRIVATE_KEY_SIZE_BYTES == key_length)
                    ? SSP_SUCCESS : SSP_ERR_CRYPTO_INVALID_SIZE;
        break;
        }
        default:
        {
            /* Key Length is a validated parameter before entering this API.
             * Reaching here means some severe error has occurred. */
        break;
        }
    }
    return (error);
}

/*******************************************************************************************************************//**
 * @brief   Function to validate RSA plain-text crt private key length.
 *
 * @param[in]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]  key_length       Length of input key.
 *
 * @retval SSP_ERR_CRYPTO_INVALID_SIZE        Invalid Key Length.
 * @retval SSP_SUCCESS                        Valid Key Length.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_validate_plain_text_crt_private_key_size_rsa (sf_crypto_signature_instance_ctrl_t * const p_ctrl,
                                                                  uint32_t key_length)
{
    ssp_err_t error = SSP_ERR_CRYPTO_INVALID_SIZE;
    switch(p_ctrl->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_RSA_1024:
        {
            error = (RSA_PLAIN_TEXT_1024_CRT_PRIVATE_KEY_SIZE_BYTES == key_length)
                    ? SSP_SUCCESS : SSP_ERR_CRYPTO_INVALID_SIZE;
        break;
        }
        case SF_CRYPTO_KEY_SIZE_RSA_2048:
        {
            error = (RSA_PLAIN_TEXT_2048_CRT_PRIVATE_KEY_SIZE_BYTES == key_length)
                    ? SSP_SUCCESS : SSP_ERR_CRYPTO_INVALID_SIZE;
        break;
        }
        default:
        {
            /* Key Length is a validated parameter before entering this API.
             * Reaching here means some severe error has occurred. */
        break;
        }
    }
    return (error);
}

/*******************************************************************************************************************//**
 * @brief   Function to validate RSA wrapped private key length.
 *
 * @param[in]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]  key_length       Length of input key.
 *
 * @retval SSP_ERR_CRYPTO_INVALID_SIZE        Invalid Key Length.
 * @retval SSP_SUCCESS                        Valid Key Length.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_validate_wrapped_private_key_size_rsa (sf_crypto_signature_instance_ctrl_t * const p_ctrl,
                                                           uint32_t key_length)
{
    ssp_err_t error = SSP_ERR_CRYPTO_INVALID_SIZE;
    switch(p_ctrl->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_RSA_1024:
        {
            error = (RSA_PLAIN_TEXT_1024_WRAPPED_PRIVATE_KEY_SIZE_BYTES == key_length)
                    ? SSP_SUCCESS : SSP_ERR_CRYPTO_INVALID_SIZE;
        break;
        }
        case SF_CRYPTO_KEY_SIZE_RSA_2048:
        {
            error = (RSA_PLAIN_TEXT_2048_WRAPPED_PRIVATE_KEY_SIZE_BYTES == key_length)
                    ? SSP_SUCCESS : SSP_ERR_CRYPTO_INVALID_SIZE;
        break;
        }
        default:
        {
            /* Key Length is a validated parameter before entering this API.
             * Reaching here means some severe error has occurred. */
        break;
        }
    }
    return (error);
}

/*******************************************************************************************************************//**
 * @brief   Function to validate RSA private key length based on key type.
 *
 * @param[in]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]  key_length       Length of input key.
 *
 * @retval SSP_ERR_CRYPTO_INVALID_SIZE        Invalid Key Length.
 * @retval SSP_SUCCESS                        Valid Key Length.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_validate_private_key_size_rsa (sf_crypto_signature_instance_ctrl_t * const p_ctrl,
                                                   uint32_t key_length)
{
    ssp_err_t error = SSP_ERR_CRYPTO_INVALID_SIZE;
    switch(p_ctrl->key_type)
    {
        case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
        {
            error = sf_crypto_signature_validate_plain_text_private_key_size_rsa(p_ctrl, key_length);
        break;
        }
        case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
        {
            error = sf_crypto_signature_validate_plain_text_crt_private_key_size_rsa(p_ctrl, key_length);
        break;
        }
        case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
        {
            error = sf_crypto_signature_validate_wrapped_private_key_size_rsa(p_ctrl, key_length);
        break;
        }
        default:
        {
            /* Key Type is a validated parameter before entering this API.
             * Reaching here means some severe error has occurred. */
        break;
        }
    }
    return (error);
}

/*******************************************************************************************************************//**
 * @brief   Function to validate RSA public key length based on key type.
 *
 * @param[in]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]  key_length       Length of input key.
 *
 * @retval SSP_ERR_CRYPTO_INVALID_SIZE        Invalid Key Length.
 * @retval SSP_SUCCESS                        Valid Key Length.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_validate_public_key_size_rsa (sf_crypto_signature_instance_ctrl_t * const p_ctrl,
                                                  uint32_t key_length)
{
    ssp_err_t error = SSP_ERR_CRYPTO_INVALID_SIZE;

    switch(p_ctrl->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_RSA_1024:
        {
            error = (RSA_PLAIN_TEXT_1024_PUBLIC_KEY_SIZE_BYTES == key_length)
                     ? SSP_SUCCESS : SSP_ERR_CRYPTO_INVALID_SIZE;
        break;
        }
        case SF_CRYPTO_KEY_SIZE_RSA_2048:
        {
            error = (RSA_PLAIN_TEXT_2048_PUBLIC_KEY_SIZE_BYTES == key_length)
                     ? SSP_SUCCESS : SSP_ERR_CRYPTO_INVALID_SIZE;
        break;
        }
        default:
        {
            /* Key Length is a validated parameter before entering this API.
             * Reaching here means some severe error has occurred. */
        break;
        }
    }

    return (error);
}

/*******************************************************************************************************************//**
 * @brief   Function to validate RSA key length based on key type and key size.
 *
 * @param[in]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]  operation_mode   Perform Sign Or Verify operation.
 * @param[in]  key_length       Length of input key.
 *
 * @retval SSP_ERR_CRYPTO_INVALID_SIZE        Invalid Key Length.
 * @retval SSP_SUCCESS                        Valid Key Length.
 * @retval SSP_ERR_CRYPTO_INVALID_OPERATION_MODE    Invalid operation mode requested.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_validate_key_size_rsa (sf_crypto_signature_instance_ctrl_t * const p_ctrl,
                                           sf_crypto_signature_mode_t  operation_mode,
                                           uint32_t key_length)
{
    ssp_err_t err = SSP_ERR_CRYPTO_INVALID_SIZE;
    switch(operation_mode)
    {
        case SF_CRYPTO_SIGNATURE_MODE_SIGN:
        {
            err = sf_crypto_signature_validate_private_key_size_rsa(p_ctrl, key_length);
        break;
        }
        case SF_CRYPTO_SIGNATURE_MODE_VERIFY:
        {
            err = sf_crypto_signature_validate_public_key_size_rsa(p_ctrl, key_length);
        break;
        }
        default:
        {
            return (SSP_ERR_CRYPTO_INVALID_OPERATION_MODE);
        break;
        }
    }
    return (err);
}

/*******************************************************************************************************************//**
 * @brief   Function to get block size based on the RSA key length.
 *
 * @param[in]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 *
 * @retval block_size               Size of block in bytes.
 **********************************************************************************************************************/
static
uint32_t
sf_crypto_signature_get_block_size_rsa (sf_crypto_signature_instance_ctrl_t * const p_ctrl)
{
    uint32_t block_size = 0;
    switch(p_ctrl->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_RSA_1024:
        {
            block_size = RSA_KEY_BITS_1024 / BITS_IN_BYTE;
        break;
        }
        case SF_CRYPTO_KEY_SIZE_RSA_2048:
        {
            block_size = RSA_KEY_BITS_2048 / BITS_IN_BYTE;
        break;
        }
        default:
            /* Key Length is a validated parameter before entering this API.
             * Reaching here means some severe error has occurred.
             * Caller must validate a non-zero block size after calling this helper function */
        break;
    }
    return block_size;
}

/*******************************************************************************************************************//**
 * @brief   Function to RSA private key length based on key size and key type.
 *
 * @param[in]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 *
 * @retval private_key_length   Private Key Length.
 **********************************************************************************************************************/
static
uint32_t
sf_crypto_signature_get_private_key_size(sf_crypto_signature_instance_ctrl_t * const p_ctrl)
{
    uint32_t private_key_size = 0U;
    switch(p_ctrl->key_size)
    {
        case SF_CRYPTO_KEY_SIZE_RSA_1024:
        {
            switch(p_ctrl->key_type)
            {
                case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
                {
                    private_key_size = RSA_PLAIN_TEXT_1024_PRIVATE_KEY_SIZE_BYTES;
                break;
                }
                case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
                {
                    private_key_size = RSA_PLAIN_TEXT_1024_CRT_PRIVATE_KEY_SIZE_BYTES;
                break;
                }
                case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
                {
                    private_key_size = RSA_PLAIN_TEXT_1024_WRAPPED_PRIVATE_KEY_SIZE_BYTES;
                break;
                }
                default:
                {
                    /* Key Type is a validated parameter before entering this API.
                     * Reaching here means some severe error has occurred.
                     * Caller must validate a non-zero block size after calling this helper function */
                break;
                }
            }
        break;
        }
        case SF_CRYPTO_KEY_SIZE_RSA_2048:
        {
            switch(p_ctrl->key_type)
            {
                case SF_CRYPTO_KEY_TYPE_RSA_PLAIN_TEXT:
                {
                    private_key_size = RSA_PLAIN_TEXT_2048_PRIVATE_KEY_SIZE_BYTES;
                break;
                }
                case SF_CRYPTO_KEY_TYPE_RSA_CRT_PLAIN_TEXT:
                {
                    private_key_size = RSA_PLAIN_TEXT_2048_CRT_PRIVATE_KEY_SIZE_BYTES;
                break;
                }
                case SF_CRYPTO_KEY_TYPE_RSA_WRAPPED:
                {
                    private_key_size = RSA_PLAIN_TEXT_2048_WRAPPED_PRIVATE_KEY_SIZE_BYTES;
                break;
                }
                default:
                {
                    /* Key Type is a validated parameter before entering this API.
                     * Reaching here means some severe error has occurred.
                     * Caller must validate a non-zero block size after calling this helper function */
                break;
                }
            }
        break;
        }
        default:
        {
            /* Key Length is a validated parameter before entering this API.
             * Reaching here means some severe error has occurred.
             * Caller must validate a non-zero block size after calling this helper function */
        break;
        }
    }
    return private_key_size;
}

/*******************************************************************************************************************//**
 * @brief   Function prepend DER prefix to the Hash present in the context buffer.
 *
 * @param[in, out]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 *
 * @retval SSP_SUCCESS              Function returned successfully.
 * @retval SSP_ERR_UNSUPPORTED      Invalid Operation Mode.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_data_formatting_der_encoding (sf_crypto_signature_instance_ctrl_t * const p_ctrl)
{
    uint32_t     curr_cursor     = 0U;
    sf_crypto_signature_context_t * operation_context = (sf_crypto_signature_context_t *)&p_ctrl->operation_context;
    sf_crypto_signature_rsa_specific_params_t * rsa_specific_params =
            (sf_crypto_signature_rsa_specific_params_t *)operation_context->p_aglorithm_specific_params;
    /* Append appropriate der prefix */
    switch (rsa_specific_params->message_format)
    {
        case SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_RSA_SHA1_PKCS1_1_5:
        {
            curr_cursor = SHA_1_DER_ENC_PREFEX_LENGTH;
            /* Push hash data to make space for DER prefix */
            memmove(operation_context->buffer.p_data + curr_cursor,
                    operation_context->buffer.p_data,
                    operation_context->buffer.data_length);
            memcpy(operation_context->buffer.p_data, sha1_der_enc_prefex, SHA_1_DER_ENC_PREFEX_LENGTH);
            operation_context->buffer.data_length += SHA_1_DER_ENC_PREFEX_LENGTH;
        break;
        }
        case SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_RSA_SHA224_PKCS1_1_5:
        {
            curr_cursor = SHA_224_DER_ENC_PREFEX_LENGTH;
            /* Push hash data to make space for DER prefix */
            memmove(operation_context->buffer.p_data + curr_cursor,
                    operation_context->buffer.p_data,
                    operation_context->buffer.data_length);
            memcpy(operation_context->buffer.p_data, sha224_der_enc_prefex, SHA_224_DER_ENC_PREFEX_LENGTH);
            operation_context->buffer.data_length += SHA_224_DER_ENC_PREFEX_LENGTH;
        break;
        }
        case SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_RSA_SHA256_PKCS1_1_5:
        {
        	curr_cursor = SHA_256_DER_ENC_PREFEX_LENGTH;
            /* Push hash data to make space for DER prefix */
            memmove(operation_context->buffer.p_data + curr_cursor,
                    operation_context->buffer.p_data,
                    operation_context->buffer.data_length);
            memcpy(operation_context->buffer.p_data, sha256_der_enc_prefex, SHA_256_DER_ENC_PREFEX_LENGTH);
            operation_context->buffer.data_length += SHA_256_DER_ENC_PREFEX_LENGTH;
        break;
        }
        default:
        {
            /* Must not land here ... */
            return SSP_ERR_UNSUPPORTED;
        break;
        }
    }
    return (SSP_SUCCESS);
}

/*******************************************************************************************************************//**
 * @brief   To generate pkcs1 v1.5 using the DER encoded Hash present in the context buffer. The output data is
 *          overwritten in the same context buffer.
 *
 * @param[in,out]  p_ctrl           Pointer to Crypto Signature Framework instance control block structure.
 *
 * @retval SSP_SUCCESS              Function returned successfully.
 * @retval SSP_ERR_ASSERTION        Invalid Octet String.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_der_enc_data_formatting_to_pkcs1_v_1_5 (sf_crypto_signature_instance_ctrl_t * const p_ctrl)
{
    uint32_t    block_size      = 0U;
    uint32_t    curr_cursor     = 0U;
    uint32_t    i               = 0U;
    sf_crypto_signature_context_t * operation_context = (sf_crypto_signature_context_t *)&p_ctrl->operation_context;
    /* Calculate size of signature block based on algorithm */
    block_size = sf_crypto_signature_get_block_size_rsa(p_ctrl);
    curr_cursor = block_size - operation_context->buffer.data_length;
    /* Push der prefix and hash data -- (T) to the end of buffer */
    memmove(operation_context->buffer.p_data + curr_cursor,
            operation_context->buffer.p_data,
            operation_context->buffer.data_length);
    /* Append 0x00 0x00 */
    curr_cursor -=1U;
    operation_context->buffer.p_data[curr_cursor] = 0x00U;
    operation_context->buffer.data_length += 1U;
    /* At this point the buffer looks like 0x00 ... ||0x00||T, where T = DER_HASH_PREFEX||H */
    /* Append 0xFFs to the remaining block -2 bytes for block type information*/
    /* Validate PS Length */
    if ((curr_cursor - 2U) < 8U)
    {
        return SSP_ERR_ASSERTION;
    }
    /* Start from third position. leave first 2 bytes for appending block type */
    for(i=2U; i<curr_cursor; i++)
    {
        operation_context->buffer.p_data[i] = 0xFFU;
        operation_context->buffer.data_length += 1U;
    }
    /* At this point the buffer looks like 0x00||0x00||PS||0x00||T */
    /* Add block type in the first 2 bytes */
    operation_context->buffer.p_data[0] = 0x00U;
    operation_context->buffer.p_data[1] = 0x01U; /* Block type 1 */
    operation_context->buffer.data_length += 2U;
    /* At this point the buffer looks like 0x00||0x01||PS||0x00||T */
    return (SSP_SUCCESS);
}

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Signature Framework allocate memory for HAL and get appropriate HAL API interface.
 *
 * @param[in]   p_ctrl                            Pointer to Crypto Signature Framework instance control block
 *                                                structure.
 * @param[in]   p_cfg                             Pointer to sf_crypto_signature_cfg_t configuration structure.
 *                                                All elements of this structure must be set by user
 *
 * @retval SSP_SUCCESS                            Memory for HAL has been allocated successfully.
 * @return                                        See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_allocate_hal_memory_and_get_interface (sf_crypto_signature_instance_ctrl_t * const p_ctrl,
                                                           sf_crypto_signature_cfg_t const * const p_cfg)
{
    ssp_err_t   err = SSP_SUCCESS;
    sf_crypto_instance_ctrl_t         * p_ctrl_common;
    /* Get a Crypto common control block and the interface. */
    p_ctrl->p_lower_lvl_common_ctrl = (sf_crypto_instance_ctrl_t *)p_cfg->p_lower_lvl_crypto_common->p_ctrl;
    p_ctrl_common = p_ctrl->p_lower_lvl_common_ctrl;

    /* Allocate memory for a Crypto HAL control block in the byte pool. */
    err = sf_crypto_memory_allocate(&p_ctrl_common->byte_pool,
                                    &p_ctrl->p_hal_ctrl, sizeof(rsa_ctrl_t),
                                    p_ctrl_common->wait_option);
    if ( err == SSP_SUCCESS)
    {
        err = sf_crypto_signature_interface_get_rsa(p_ctrl);
        if ( (err != SSP_SUCCESS) || (NULL == p_ctrl->p_hal_api))
        {
            memset(p_ctrl->p_hal_ctrl, 0U, (uint32_t)sizeof(rsa_ctrl_t));
            sf_crypto_memory_release(p_ctrl->p_hal_ctrl, (uint32_t)sizeof(rsa_ctrl_t));
            return err;
        }

    }
    return err;
}
/*******************************************************************************************************************//**
 * @brief   SSP Crypto Signature Framework Open and initialize Crypto Hash Framework module.
 *
 * @param[in]   p_ctrl                            Pointer to Crypto Signature Framework instance control block
 *                                                structure.
 *
 * @retval SSP_SUCCESS                            The hash framework module was successfully opened and initialized.
 * @retval SSP_ERR_UNSUPPORTED                    Invalid message format request.
 * @retval SSP_ERR_CRYPTO_INVALID_OPERATION_MODE  Invalid operation mode requested.
 * @return                                        See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_open_and_init_hash (sf_crypto_signature_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t    err    = SSP_SUCCESS;
    sf_crypto_signature_rsa_specific_params_t * rsa_specific_params =
            (sf_crypto_signature_rsa_specific_params_t *)p_ctrl->operation_context.p_aglorithm_specific_params;

    /* Get RSA HAL API interface instance structure */

    switch(rsa_specific_params->message_format)
    {
        case SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_NONE:
        {
            /* No need to open Hash module */
        break;
        }
        case SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_RSA_SHA1_PKCS1_1_5:
        {
            p_ctrl->p_lower_lvl_sf_crypto_hash->p_cfg->hash_type = SF_CRYPTO_HASH_ALGORITHM_SHA1;
        break;
        }
        case SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_RSA_SHA224_PKCS1_1_5:
        {
            p_ctrl->p_lower_lvl_sf_crypto_hash->p_cfg->hash_type = SF_CRYPTO_HASH_ALGORITHM_SHA224;
        break;
        }
        case SF_CRYPTO_SIGNATURE_MESSAGE_OPERATION_RSA_SHA256_PKCS1_1_5:
        {
            p_ctrl->p_lower_lvl_sf_crypto_hash->p_cfg->hash_type = SF_CRYPTO_HASH_ALGORITHM_SHA256;
        break;
        }
        default:
        {
            return (SSP_ERR_UNSUPPORTED);
        break;
        }
    }

    err = p_ctrl->p_lower_lvl_sf_crypto_hash->p_api->open(p_ctrl->p_lower_lvl_sf_crypto_hash->p_ctrl,
                                                          p_ctrl->p_lower_lvl_sf_crypto_hash->p_cfg);

    if (err != SSP_SUCCESS)
    {
        return (err);
    }
    err = p_ctrl->p_lower_lvl_sf_crypto_hash->p_api->hashInit(p_ctrl->p_lower_lvl_sf_crypto_hash->p_ctrl);

    return (err);
}

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Signature Framework Finalize and Close Crypto Hash Framework module.
 *
 * @param[in]   p_ctrl                            Pointer to Crypto Signature Framework instance control block
 *                                                structure.
 *
 * @retval SSP_SUCCESS                            The hash framework module was successfully finalized and closed.
 * @return                                        See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_final_and_close_hash (sf_crypto_signature_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;
    /* Calculate size of signature block based on algorithm
     * The block size captured here is the actual block allocated in the byte pool.
     * This is much greater than the output of any HASH algorithm used (supported by this module)
     * and is hence safe to be passed while calling hashFinal API. */
    p_ctrl->operation_context.buffer.data_length = sf_crypto_signature_get_block_size_rsa(p_ctrl);

    /* Get the hashed data and write it to the buffer */
    err= p_ctrl->p_lower_lvl_sf_crypto_hash->p_api->hashFinal(p_ctrl->p_lower_lvl_sf_crypto_hash->p_ctrl,
                                                              &p_ctrl->operation_context.buffer,
                                                              (uint32_t *)&p_ctrl->operation_context.buffer.data_length);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /* Close Hash module */
    err = p_ctrl->p_lower_lvl_sf_crypto_hash->p_api->close(p_ctrl->p_lower_lvl_sf_crypto_hash->p_ctrl);

    return (err);
}
