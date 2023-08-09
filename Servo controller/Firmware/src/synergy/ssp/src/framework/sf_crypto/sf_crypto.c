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
 * File Name    : sf_crypto.c
 * Description  : Crypto Framework Common Module
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @file
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto.h"
#include "sf_crypto_private_api.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
#if SF_CRYPTO_CFG_PARAM_CHECKING_ENABLE
static ssp_err_t sf_crypto_open_param_check (sf_crypto_instance_ctrl_t const * const p_api_ctrl,
                                             sf_crypto_cfg_t const * const p_cfg);
#endif

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
static const ssp_version_t sf_crypto_version =
{
    .api_version_major  = SF_CRYPTO_API_VERSION_MAJOR,
    .api_version_minor  = SF_CRYPTO_API_VERSION_MINOR,
    .code_version_major = SF_CRYPTO_CODE_VERSION_MAJOR,
    .code_version_minor = SF_CRYPTO_CODE_VERSION_MINOR
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
const sf_crypto_api_t         g_sf_crypto_api =
{
    .open       = SF_CRYPTO_Open,
    .lock       = SF_CRYPTO_Lock,
    .unlock     = SF_CRYPTO_Unlock,
    .close      = SF_CRYPTO_Close,
    .statusGet  = SF_CRYPTO_StatusGet,
    .versionGet = SF_CRYPTO_VersionGet
};
/*LDRA_ANALYSIS */

/*******************************************************************************************************************//**
 * @addtogroup SF_CRYPTO
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
Functions
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Framework Common Open operation.
 *
 * @param[in,out]   p_api_ctrl      Pointer to a Crypto framework control block
 * @param[in]       p_cfg           Pointer to a Crypto framework configuration structure
 *
 * @retval SSP_SUCCESS              Crypto framework was successfully opened.
 * @retval SSP_ERR_ASSERTION        NULL pointer is passed.
 * @retval SSP_ERR_INTERNAL         RTOS service returned a unexpected error.
 * @retval SSP_ERR_CRYPTO_HAL_ERROR Crypto HAL driver retuned an error.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_Open (sf_crypto_ctrl_t * const p_api_ctrl, sf_crypto_cfg_t const * const p_cfg)
{
    ssp_err_t   err = SSP_SUCCESS;
    uint32_t    err_crypto;
    uint32_t    tx_err;
    sf_crypto_instance_ctrl_t * p_ctrl = (sf_crypto_instance_ctrl_t *) p_api_ctrl;
    crypto_instance_t         * p_lower_lvl_crypto;

#if SF_CRYPTO_CFG_PARAM_CHECKING_ENABLE
    /** Verify if parameters are valid. */
    err = sf_crypto_open_param_check(p_ctrl, p_cfg);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
#endif

    /** Check if the module has been opened. If opened, return error. */
    if (SF_CRYPTO_OPENED == p_ctrl->status)
    {
        return SSP_ERR_ALREADY_OPEN;
    }

    /* Create a mutex for the module internal use.
     * This mutex synchronizes threads which are accessing the hardware.
     * This ensures only one tread is accessing the hardware at any given time. */
    tx_err = tx_mutex_create(&p_ctrl->mutex, "sf_crypto_mtx", TX_INHERIT);
    if ((uint32_t)TX_SUCCESS != tx_err)
    {
        return SSP_ERR_INTERNAL;
    }

    /* Create a semaphore for internal event synchronization. */
    tx_err = tx_semaphore_create(&p_ctrl->semaphore, "sf_crypto_sema", 0UL);
    if ((uint32_t)TX_SUCCESS != tx_err)
    {
        tx_mutex_delete(&p_ctrl->mutex);
        return SSP_ERR_INTERNAL;
    }

    /* Create a byte pool for allocating memory at run-time. */
    tx_err = tx_byte_pool_create(&p_ctrl->byte_pool, "sf_crypto_pool",
                                     p_cfg->p_memory_pool, p_cfg->memory_pool_size);
    if ((uint32_t)TX_SUCCESS != tx_err)
    {
        tx_mutex_delete(&p_ctrl->mutex);
        tx_semaphore_delete(&p_ctrl->semaphore);
        return SSP_ERR_INTERNAL;
    }

    /* Save a crypto engine driver instance in the control block. */
    p_lower_lvl_crypto = (crypto_instance_t *)p_cfg->p_lower_lvl_crypto;
    p_ctrl->p_lower_lvl_crypto = p_lower_lvl_crypto;

    /** Open a lower level crypto driver and ensure the engine is initialized here or already
     * in other SCE supported stack. */
    err_crypto = p_lower_lvl_crypto->p_api->open (p_lower_lvl_crypto->p_ctrl, p_lower_lvl_crypto->p_cfg);
    if (((uint32_t)SSP_SUCCESS != err_crypto) && ((uint32_t)SSP_ERR_CRYPTO_SCE_ALREADY_OPEN != err_crypto))
    {
        tx_mutex_delete(&p_ctrl->mutex);
        tx_semaphore_delete(&p_ctrl->semaphore);
        tx_byte_pool_delete(&p_ctrl->byte_pool);

        err = SSP_ERR_CRYPTO_HAL_ERROR;
    }
    else
    {
        /* Save the module configuration parameters in the control block. */
        p_ctrl->wait_option  = p_cfg->wait_option;
        p_ctrl->close_option = p_cfg->close_option;
        p_ctrl->open_counter = 0U;
        p_ctrl->p_callback   = NULL;    /* This is currently reserved for future use. Just set NULL to it here. */
        p_ctrl->p_context    = NULL;    /* This is currently reserved for future use. Just set NULL to it here. */

        /* Mark the module status as Opened. */
        p_ctrl->status = SF_CRYPTO_OPENED;
    }

    return (err);

} /* End of function SF_CRYPTO_Open */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Framework Common Close operation.
 *
 * @param[in,out]   p_api_ctrl      Pointer to a Crypto framework control block
 *
 * @retval SSP_SUCCESS              Module was successfully closed.
 * @retval SSP_ERR_NOT_OPEN         Module has not opened.
 * @retval SSP_ERR_ASSERTION        NULL pointer is passed as an input parameter.
 * @retval SSP_ERR_INTERNAL         RTOS service returned a unexpected error.
 * @retval SSP_ERR_CRYPTO_HAL_ERROR Unable to successfully close the Crypto Common HAL module.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_Close (sf_crypto_ctrl_t * const p_api_ctrl)
{
    uint32_t    tx_err;
    uint32_t    err_crypto;
    sf_crypto_instance_ctrl_t * p_ctrl = (sf_crypto_instance_ctrl_t *) p_api_ctrl;
    crypto_instance_t         * p_lower_lvl_crypto;

#if SF_CRYPTO_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid. */
    SSP_ASSERT(p_ctrl);
#endif

    /** Check if the module has been opened. If not, return error. */
    if (SF_CRYPTO_OPENED != p_ctrl->status)
    {
        return SSP_ERR_NOT_OPEN;
    }
    else if ((SF_CRYPTO_CLOSE_OPTION_DEFAULT == p_ctrl->close_option) && (0U < p_ctrl->open_counter))
    {
        return SSP_ERR_IN_USE;
    }

    /* To make sure no any SF_CRYPTO_XXX module is opening the module at this point. Thread preemption. Check if
     * the mutex can be acquired here. If not acquired, it is not safe to proceed the closure.
     */
    tx_err = tx_mutex_get(&p_ctrl->mutex, TX_NO_WAIT);
    if (TX_SUCCESS != tx_err)
    {
        return SSP_ERR_IN_USE;
    }
    else
    {
        p_lower_lvl_crypto = p_ctrl->p_lower_lvl_crypto;

       /** Close a lower level crypto driver. */
       err_crypto = p_lower_lvl_crypto->p_api->close (p_lower_lvl_crypto->p_ctrl);
       if ((uint32_t)SSP_SUCCESS != err_crypto)
       {
           tx_mutex_put(&p_ctrl->mutex);
           return SSP_ERR_CRYPTO_HAL_ERROR;
       }

       /* Delete the semaphore. */
       tx_semaphore_delete(&p_ctrl->semaphore);

       /* Delete the byte pool. */
       tx_byte_pool_delete(&p_ctrl->byte_pool);

       /* Delete the mutex. */
       tx_mutex_delete(&p_ctrl->mutex);

       /* Clean up the module configuration parameters in the control block. */
       p_ctrl->wait_option = 0U;
       p_ctrl->p_callback  = NULL;
       p_ctrl->p_context   = NULL;
       p_ctrl->p_lower_lvl_crypto = NULL;

       /* Mark the module status as Closed. */
       p_ctrl->status = SF_CRYPTO_CLOSED;
    }

    return SSP_SUCCESS;
} /* End of function SF_CRYPTO_Close */

/*******************************************************************************************************************//**
 * @brief   Locks the module. This API is utilized for locking shared resources.
 *
 * @param[in,out]       p_api_ctrl      Pointer to a Crypto framework control block
 * @retval SSP_SUCCESS                  Module resources are successfully locked.
 * @retval SSP_ERR_TIMEOUT              Unable to get ownership of the mutex within the specified time.
 * @retval SSP_ERR_INTERNAL             Thread suspension was aborted. Critical error.
 * @retval SSP_ERR_ASSERTION            NULL pointer is passed.as an input parameter.
 * @retval SSP_ERR_NOT_OPEN             The module is not yet opened.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_Lock (sf_crypto_ctrl_t * const p_api_ctrl)
{
    uint32_t    tx_err;
    ssp_err_t   error;
    sf_crypto_instance_ctrl_t * p_ctrl = (sf_crypto_instance_ctrl_t *) p_api_ctrl;

#if SF_CRYPTO_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid. */
    SSP_ASSERT(p_ctrl);
#endif

    /** Check if the module has been opened. If not, return error. */
    if (SF_CRYPTO_OPENED != p_ctrl->status)
    {
        return SSP_ERR_NOT_OPEN;
    }

    /** Acquire the mutex. */
    tx_err = tx_mutex_get(&p_ctrl->mutex, p_ctrl->wait_option);
    switch (tx_err)
    {
    case (uint32_t)TX_SUCCESS:
        error = SSP_SUCCESS;
        break;

    case (uint32_t)TX_NOT_AVAILABLE:
        error = SSP_ERR_TIMEOUT;
        break;

    default:
        error = SSP_ERR_INTERNAL;
        break;
    }

    return error;

} /* End of function SF_CRYPTO_Lock */

/*******************************************************************************************************************//**
 * @brief   Unlocks the module. This API is utilized for unlocking shared resources.
 *
 * @param[in,out]   p_api_ctrl          Pointer to a Crypto framework control block
 * @retval SSP_SUCCESS                  Module resources are successfully unlocked.
 * @retval SSP_ERR_ASSERTION            NULL pointer is passed as an input parameter.
 * @retval SSP_ERR_INTERNAL             Mutex is not owned by a caller thread.
 * @retval SSP_ERR_NOT_OPEN             The module is not yet opened.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_Unlock (sf_crypto_ctrl_t * const p_api_ctrl)
{
    ssp_err_t   error = SSP_SUCCESS;
    uint32_t    tx_err;
    sf_crypto_instance_ctrl_t * p_ctrl = (sf_crypto_instance_ctrl_t *) p_api_ctrl;

#if SF_CRYPTO_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid. */
    SSP_ASSERT(p_ctrl);
#endif

    /** Check if the module has been opened. If not, return error. */
    if (SF_CRYPTO_OPENED != p_ctrl->status)
    {
        return SSP_ERR_NOT_OPEN;
    }

    /** Return the mutex. */
    tx_err = tx_mutex_put(&p_ctrl->mutex);
    if ((uint32_t)TX_SUCCESS != tx_err)
    {
        error = SSP_ERR_INTERNAL;
    }

    return error;

} /* End of function SF_CRYPTO_Unlock */

/*******************************************************************************************************************//**
 * @brief       Gets the Crypto Common Framework module status.
 *
 * @param[in]   p_api_ctrl          Pointer to a Crypto framework control block
 * @param[out]  p_status            Memory location to store module status.
 *
 * @retval      SSP_SUCCESS         Status returned successfully.
 * @retval      SSP_ERR_ASSERTION   The parameter p_status is NULL.
 * @retval      SSP_ERR_CRYPTO_COMMON_NOT_OPENED   This common module is not yet opened.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_StatusGet (sf_crypto_ctrl_t * const p_api_ctrl, sf_crypto_state_t * p_status)
{
    sf_crypto_instance_ctrl_t * p_ctrl = (sf_crypto_instance_ctrl_t *)p_api_ctrl;
#if SF_CRYPTO_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid. */
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_status);
#endif

    /** Check if the module has a valid / known status else return error.
     */
    if ((SF_CRYPTO_OPENED != p_ctrl->status) && (SF_CRYPTO_CLOSED != p_ctrl->status))
    {
        /* Other Crypto Framework modules will propogate this error to the caller. It will be helpful to explicitly
         * indicate that the common module is not opened instead of the generic SSP_ERR_NOT_OPEN which may imply that
         * the module from which the call is made is not open. */

        return SSP_ERR_CRYPTO_COMMON_NOT_OPENED;
    }

    /** Return the module status. */
    *p_status = p_ctrl->status;

    return SSP_SUCCESS;

} /* End of function SF_CRYPTO_StatusGet */

/*******************************************************************************************************************//**
 * @brief      Gets the version of Crypto Common Framework module.
 *
 * @param[out] p_version            Pointer to the memory to store the version information.
 *
 * @retval     SSP_SUCCESS          Successful close.
 * @retval     SSP_ERR_ASSERTION    The parameter p_version is NULL.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_VersionGet (ssp_version_t * const p_version)
{
#if SF_CRYPTO_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid. */
    SSP_ASSERT(p_version);
#endif

    /** Return the module version. */
    p_version->version_id = sf_crypto_version.version_id;

    return SSP_SUCCESS;

} /* End of function SF_CRYPTO_VersionGet */

/*******************************************************************************************************************//**
 * @} (end defgroup SF_CRYPTO)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

#if SF_CRYPTO_CFG_PARAM_CHECKING_ENABLE
/*******************************************************************************************************************//**
 * @brief   Subroutine for SF_CRYPTO_Open to check parameters.
 *
 * @param[in]   p_ctrl              Pointer to a Crypto framework control block
 * @param[in]   p_cfg               Pointer to a Crypto framework configuration structure
 * @retval      SSP_SUCCESS         All the parameters are valid.
 * @retval      SSP_ERR_ASSERTION   NULL pointer is passed.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_open_param_check (sf_crypto_instance_ctrl_t const * const p_ctrl,
                                                    sf_crypto_cfg_t const * const p_cfg)
{
    /* Verify if parameters are valid. */
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_cfg);
    SSP_ASSERT(p_cfg->p_lower_lvl_crypto);
    SSP_ASSERT(p_cfg->p_memory_pool);

    return SSP_SUCCESS;

} /* End of function sf_crypto_open_param_check */
#endif
