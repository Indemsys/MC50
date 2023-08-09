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
/***********************************************************************************************************************
 * File Name    : sf_crypto_trng.c
 * Description  : Crypto TRNG Framework API declaration.
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @file
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#include "sf_crypto_trng_cfg.h"
#include "sf_crypto_trng.h"
#include "sf_crypto_trng_private_api.h"
#include "r_trng_api.h"
#include "../sf_crypto/sf_crypto_private_api.h"
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/* Max True random number words retrievable in single HW Access.*/
#define WORDS_IN_SINGLE_HW_PROC_CALL (4)

#define BYTES_IN_WORD (4)

/* Max True random number bytes retrievable in single HW Access.*/
#define BYTES_IN_SINGLE_HW_PROC_CALL ((BYTES_IN_WORD) * (WORDS_IN_SINGLE_HW_PROC_CALL))

/* Extra call to HW to get (remaining) bytes not multiple of BYTES_IN_SINGLE_HW_PROC_CALL*/
#define EXTRA_BUFFER_SIZE_WORDS ((BYTES_IN_SINGLE_HW_PROC_CALL) / (BYTES_IN_WORD))

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
#if SF_CRYPTO_TRNG_CFG_PARAM_CHECKING_ENABLE
static ssp_err_t sf_crypto_trng_open_param_check (sf_crypto_instance_ctrl_t * const p_api_ctrl,
                                             sf_crypto_trng_cfg_t const * const p_cfg);
static ssp_err_t sf_crypto_trng_random_number_gen_param_check (sf_crypto_trng_ctrl_t * const p_api_ctrl,
                                             sf_crypto_data_handle_t *p_random_number_buff);
#endif /*SF_CRYPTO_TRNG_CFG_PARAM_CHECKING_ENABLE*/
/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug. This pragma suppresses the warnings in this
 * structure only.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

/** TRNG Version Data Structure */
static ssp_version_t sf_crypto_trng_version =
{
    .api_version_major  = SF_CRYPTO_TRNG_API_VERSION_MAJOR,
    .api_version_minor  = SF_CRYPTO_TRNG_API_VERSION_MINOR,
    .code_version_major = SF_CRYPTO_TRNG_CODE_VERSION_MAJOR,
    .code_version_minor = SF_CRYPTO_TRNG_CODE_VERSION_MINOR,
};

#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/***********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const sf_crypto_trng_api_t         g_sf_crypto_trng_api =
{
    .open                     = SF_CRYPTO_TRNG_Open,
    .close                    = SF_CRYPTO_TRNG_Close,
    .randomNumberGenerate     = SF_CRYPTO_TRNG_RandomNumberGenerate,
    .versionGet               = SF_CRYPTO_TRNG_VersionGet
};

/*******************************************************************************************************************//**
 * @ingroup SF_Library
 * @defgroup SF_CRYPTO_TRNG SSP Crypto TRNG Framework
 * @{
 **********************************************************************************************************************/


/******************************************************************************************************************//**
 * @brief    SSP Crypto TRNG Framework Open operation.
 *
 * @retval    SSP_SUCCESS                       The module was successfully opened
 * @retval    SSP_ERR_ASSERTION                 One or more input parameters are NULL.
 * @retval    SSP_ERR_CRYPTO_COMMON_NOT_OPENED  Crypto framework common module has not been opened
 *                                              before calling this API.
 * @retval    SSP_ERR_ALREADY_OPEN              The module has already been opened.
 * @return                                      See @ref Common_Error_Codes for other possible return codes.
 *********************************************************************************************************************/
ssp_err_t SF_CRYPTO_TRNG_Open(sf_crypto_trng_ctrl_t * const p_api_ctrl, sf_crypto_trng_cfg_t const * const p_cfg)
{
    ssp_err_t                         ret_val;
    uint32_t                          ret_val_crypto;
    sf_crypto_trng_instance_ctrl_t  * p_ctrl = (sf_crypto_trng_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t       * p_ctrl_common;
    sf_crypto_api_t                 * p_api_common;
    trng_ctrl_t                     * p_ctrl_trng;
    trng_cfg_t const                * p_cfg_trng;
    trng_api_t const                * p_api_trng;
    sf_crypto_state_t                 common_module_status = SF_CRYPTO_CLOSED;

#if SF_CRYPTO_TRNG_CFG_PARAM_CHECKING_ENABLE
    /** Verify Parameters are valid */
    ret_val = sf_crypto_trng_open_param_check(p_api_ctrl, p_cfg);
    if(SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }
#endif

    ret_val = SSP_SUCCESS;
    ret_val_crypto = (uint32_t)SSP_SUCCESS;

    p_ctrl->p_lower_lvl_common_ctrl = p_cfg->p_lower_lvl_common->p_ctrl;
    p_ctrl->p_lower_lvl_common_api = (sf_crypto_api_t *)p_cfg->p_lower_lvl_common->p_api;

    p_ctrl_common = p_ctrl->p_lower_lvl_common_ctrl;
    p_api_common = p_ctrl->p_lower_lvl_common_api;

    /** Check if Crypto Common module is opened before calling this API */
    ret_val = p_api_common->statusGet(p_ctrl_common, &common_module_status);
    if (SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }

    if (SF_CRYPTO_OPENED != common_module_status)
    {
        return SSP_ERR_CRYPTO_COMMON_NOT_OPENED;
    }

    /** Check if SF Crypto TRNG is already opened */
    if (SF_CRYPTO_TRNG_OPENED == p_ctrl->status)
    {
        return SSP_ERR_ALREADY_OPEN;
    }

    /* Note: sf_crypto.lock() is not called because the HAL driver does not access HW during open.
     * In the future, if HW will be accessed through the HAL driver; lock should be acquired. */

    /** Setup HAL API */
    p_ctrl->p_lower_lvl_instance = (void *)p_cfg->p_lower_lvl_instance;

    p_ctrl_trng = ((trng_instance_t *)(p_ctrl->p_lower_lvl_instance))->p_ctrl;
    p_cfg_trng  = ((trng_instance_t *)(p_ctrl->p_lower_lvl_instance))->p_cfg;
    p_api_trng  = ((trng_instance_t *)(p_ctrl->p_lower_lvl_instance))->p_api;

    /** Call HAL API to Open TRNG HAL Driver */
    ret_val_crypto = p_api_trng->open(p_ctrl_trng, p_cfg_trng);

    if ((uint32_t)SSP_SUCCESS != ret_val_crypto)
    {
        ret_val = (ssp_err_t)ret_val_crypto;
        return ret_val;
    }

    /** Set TRNG status to Open */
    p_ctrl->status = SF_CRYPTO_TRNG_OPENED;

    /** Increment Open counter to indicate a (this) module is open */
    sf_crypto_open_counter_increment(p_ctrl_common);

    return ret_val;
}

/******************************************************************************************************************//**
 * @brief    SSP Crypto TRNG Framework Close operation.
 *
 * @retval    SSP_SUCCESS                       The module was successfully closed
 * @retval    SSP_ERR_ASSERTION                 One or more input parameters are NULL.
 * @retval    SSP_ERR_CRYPTO_COMMON_NOT_OPENED  Crypto Common module has not been opened.
 * @retval    SSP_ERR_NOT_OPEN                  The module has not been opened before calling this API.
 * @return                                      See @ref Common_Error_Codes for other possible return codes.
 *********************************************************************************************************************/
ssp_err_t SF_CRYPTO_TRNG_Close(sf_crypto_trng_ctrl_t * const p_api_ctrl)
{
    ssp_err_t                         ret_val;
    uint32_t                          ret_val_crypto;
    sf_crypto_trng_instance_ctrl_t  * p_ctrl = (sf_crypto_trng_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t       * p_ctrl_common;
    trng_ctrl_t                     * p_ctrl_trng;
    trng_api_t const                * p_api_trng;

#if SF_CRYPTO_TRNG_CFG_PARAM_CHECKING_ENABLE
    /** Verify Parameters are valid */
    SSP_ASSERT(p_api_ctrl);
#endif

    ret_val = SSP_SUCCESS;
    ret_val_crypto = (uint32_t)SSP_SUCCESS;

    p_ctrl_common = p_ctrl->p_lower_lvl_common_ctrl;

    /** Check if SF Crypto TRNG is opened before*/
    if (SF_CRYPTO_TRNG_OPENED != p_ctrl->status)
    {
        return SSP_ERR_NOT_OPEN;
    }

    p_ctrl_trng = ((trng_instance_t *)(p_ctrl->p_lower_lvl_instance))->p_ctrl;
    p_api_trng  = ((trng_instance_t *)(p_ctrl->p_lower_lvl_instance))->p_api;

    /* Note: sf_crypto.lock() is not called because the HAL driver does not access HW during close.
     * In the future, if HW will be accessed through the HAL driver; lock should be acquired. */


    /** Call HAL API to Close TRNG HAL Driver */
    ret_val_crypto = p_api_trng-> close(p_ctrl_trng);

    if ((uint32_t)SSP_SUCCESS != ret_val_crypto)
    {
        ret_val = (ssp_err_t)ret_val_crypto;
        return ret_val;
    }

    /** Set TRNG status to Closed */
    p_ctrl->status = SF_CRYPTO_TRNG_CLOSED;

    /** Decrement Open counter to indicate a (this) module is closed */
    sf_crypto_open_counter_decrement(p_ctrl_common);

    return ret_val;
}

/******************************************************************************************************************//**
 * @brief    SSP Crypto TRNG Framework True Random Generation operation.
 *
 * @retval    SSP_SUCCESS                       The module was successfully closed.
 * @retval    SSP_ERR_NOT_OPEN                  The module has not been opened before calling this API.
 * @retval    SSP_ERR_ASSERTION                 One or more input parameters are NULL.
 * @return                                      See @ref Common_Error_Codes for other possible return codes.
 *********************************************************************************************************************/
ssp_err_t SF_CRYPTO_TRNG_RandomNumberGenerate(sf_crypto_trng_ctrl_t * const p_api_ctrl,
                                              sf_crypto_data_handle_t * const p_random_number_buff)
{
    ssp_err_t                         ret_val;
    uint32_t                          ret_val_crypto;
    sf_crypto_trng_instance_ctrl_t  * p_ctrl = (sf_crypto_trng_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t       * p_ctrl_common;
    sf_crypto_api_t                 * p_api_common;
    uint32_t                          buffer_length_in_four_words;
    uint32_t                          extra_bytes;
    /*LDRA_INSPECTED 69 D EXTRA_BUFFER_SIZE_WORDS has been defined above. */
    uint32_t                          trn_buffer[EXTRA_BUFFER_SIZE_WORDS];
    trng_api_t const                * p_api_trng;
    trng_ctrl_t                     * p_ctrl_trng;


#if SF_CRYPTO_TRNG_CFG_PARAM_CHECKING_ENABLE
    /** Verify Parameters are valid */
    ret_val = sf_crypto_trng_random_number_gen_param_check(p_api_ctrl, p_random_number_buff);
    if(SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }
#endif

    ret_val = SSP_SUCCESS;
    ret_val_crypto = (uint32_t)SSP_SUCCESS;

    /* Number of 16-byte (4-word) chunks of True Random Number. */
    buffer_length_in_four_words = p_random_number_buff->data_length /(uint32_t)BYTES_IN_SINGLE_HW_PROC_CALL;

    /* Remaining number of bytes i.e. less than 16-bytes. */
    extra_bytes = (p_random_number_buff->data_length %(uint32_t)BYTES_IN_SINGLE_HW_PROC_CALL);

    p_ctrl_common = p_ctrl->p_lower_lvl_common_ctrl;
    p_api_common = p_ctrl->p_lower_lvl_common_api;

    /** Check if SF Crypto TRNG is opened before */
    if (SF_CRYPTO_TRNG_CLOSED == p_ctrl->status)
    {
        return SSP_ERR_NOT_OPEN;
    }

    /** Acquire SF Crypto Common lock before accessing HAL */
    ret_val = p_api_common->lock (p_ctrl_common);
    if (SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }

    p_ctrl_trng = ((trng_instance_t *)(p_ctrl->p_lower_lvl_instance))->p_ctrl;
    p_api_trng  = ((trng_instance_t *)(p_ctrl->p_lower_lvl_instance))->p_api;

    /** Call HAL API to generate true random number */

    /** Get 16-byte multiples of TRNs first and store/hold it in the user buffer */
    if(0 != buffer_length_in_four_words)
    {
        ret_val_crypto = p_api_trng->read(p_ctrl_trng, (uint32_t *)p_random_number_buff->p_data,
                                          buffer_length_in_four_words * (uint32_t)WORDS_IN_SINGLE_HW_PROC_CALL);
        if ((uint32_t)SSP_SUCCESS != ret_val_crypto)
        {
            /* Return the more serious error */
            ret_val = p_api_common->unlock (p_ctrl_common);
            return ((ssp_err_t)ret_val_crypto);
        }
    }

    /* In case user has requested for TRNG length not a perfect divisible of 16-bytes handle
     * the remaining bytes here. Return 16-byte True random number in a local buffer*/
    if (0 != extra_bytes)
    {
        /*LDRA_INSPECTED 69 D EXTRA_BUFFER_SIZE_WORDS has been defined above. */
        ret_val_crypto = p_api_trng->read(p_ctrl_trng, &trn_buffer[0], (uint32_t)EXTRA_BUFFER_SIZE_WORDS);
        if ((uint32_t)SSP_SUCCESS != ret_val_crypto)
        {
            /* Return the more serious error */
            ret_val = p_api_common->unlock (p_ctrl_common);
            return ((ssp_err_t)ret_val_crypto);
        }
    }

    /** Release SF Crypto Common lock */
    ret_val = p_api_common->unlock (p_ctrl_common);

    /* Since HW access is complete, do the other processing after releasing the lock. */
   if (0 != extra_bytes)
    {
        /* Copy extra/remainder bytes of True random number (a number less than 16-bytes long) into the user buffer
         * Concatenate this to the previously stored True random number(if any)*/
        memcpy((p_random_number_buff->p_data + (buffer_length_in_four_words * (uint32_t)BYTES_IN_SINGLE_HW_PROC_CALL)),
                &trn_buffer[0], extra_bytes);
    }
    return ret_val;
}

/******************************************************************************************************************//**
 * @brief    Sets TRNG Framework Code and API version based on compile time macros.
 *
 * @retval    SSP_SUCCESS        Successful close
 * @retval    SSP_ERR_ASSERTION  The parameter p_version is NULL.
 *********************************************************************************************************************/
ssp_err_t SF_CRYPTO_TRNG_VersionGet(ssp_version_t * const p_version)
{
#if SF_CRYPTO_TRNG_CFG_PARAM_CHECKING_ENABLE
    /* Verify Parameters are valid */
    SSP_ASSERT(p_version);
#endif
    p_version->version_id = sf_crypto_trng_version.version_id;
    return SSP_SUCCESS;
}

#if SF_CRYPTO_TRNG_CFG_PARAM_CHECKING_ENABLE
/*******************************************************************************************************************//**
 * @brief   SSP Crypto TRNG Framework parameter check for SF_CRYPTO_TRNG_Open API.
 *
 * @param[in]   p_api_ctrl                     Pointer to Crypto TRNG Framework instance control block structure.
 * @param[in]   p_cfg                          Pointer to Crypto TRNG Framework configuration block structure.
 *
 * @retval      SSP_SUCCESS                    Parameters are not NULL.
 * @retval      SSP_ERR_ASSERTION              One or more input parameters are NULL.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_trng_open_param_check (sf_crypto_instance_ctrl_t * const p_api_ctrl,
                                                  sf_crypto_trng_cfg_t const * const p_cfg)
{
    SSP_ASSERT(p_api_ctrl);
    SSP_ASSERT(p_cfg);
    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   SSP Crypto TRNG Framework parameter check for SF_CRYPTO_TRNG_RandomNumberGenerate API.
 *
 * @param[in]   p_api_ctrl                     Pointer to Crypto TRNG Framework instance control block structure.
 * @param[in]   p_random_number_buff           Pointer data handle which is used to return true random number.
 *
 * @retval      SSP_SUCCESS                    Parameters are not NULL.
 * @retval      SSP_ERR_ASSERTION              One or more input parameters are NULL and/or
 *                                             random number data buffer is not DWORD aligned.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_trng_random_number_gen_param_check (sf_crypto_trng_ctrl_t * const p_api_ctrl,
                                                               sf_crypto_data_handle_t *p_random_number_buff)
{
    SSP_ASSERT(p_api_ctrl);
    SSP_ASSERT(p_random_number_buff);
    SSP_ASSERT(p_random_number_buff->p_data);
    SSP_ASSERT(p_random_number_buff->data_length);
    SSP_ASSERT(0U == ((uint32_t)p_random_number_buff->p_data % sizeof(uint32_t)));
    return SSP_SUCCESS;
}
#endif /*SF_CRYPTO_TRNG_CFG_PARAM_CHECKING_ENABLE*/

/** @} (end addtogroup SF_CRYPTO_TRNG) */
