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
 * File Name    : sf_crypto_private.c
 * Description  : Crypto Framework Private Module, contains definitions for all private functions required for Crypto
 *                procedures
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

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @addtogroup SF_CRYPTO
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @brief      Subroutine to allocate memory in byte pool.
 * This function would be called by crypto framework modules.
 *
 * @param[in]  p_byte_pool              Pointer to a Byte memory pool.
 * @param[out] pp_memory                Pointer to the pointer to a memory block being used by crypto framework modules.
 * @param[in]  memory_size              Size of the memory block being allocated.
 * @param[in]  wait_option              Wait option for RTOS byte memory poll service.
 * @retval     SSP_SUCCESS              RSA HAL module is successfully allocated.
 * @retval     SSP_ERR_TIMEOUT          Was unable to allocate the memory within the specified time to wait.
 * @retval     SSP_ERR_OUT_OF_MEMORY    Requested size is zero or larger than the pool.
 * @retval     SSP_ERR_INTERNAL         RTOS service returned a unexpected error.
 * @note       This is an internal function for SF_CRYPTO_XXX which does not perform any parameter checks. It is
 *             caller's responsibility to pass valid parameters.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_memory_allocate ( TX_BYTE_POOL * p_byte_pool, void ** pp_memory, uint32_t memory_size,
                                                                                                uint32_t wait_option)
{
    ssp_err_t   err = SSP_SUCCESS;
    uint32_t    tx_err;

    /* Allocate memory in byte pool. */
    tx_err = tx_byte_allocate(p_byte_pool, pp_memory, memory_size, wait_option);
    if ((uint32_t)TX_NO_MEMORY == tx_err)
    {
        err = SSP_ERR_TIMEOUT;
    }
    else if ((uint32_t)TX_SIZE_ERROR == tx_err)
    {
        err = SSP_ERR_OUT_OF_MEMORY;
    }
    else if ((uint32_t)TX_SUCCESS != tx_err)
    {
        err = SSP_ERR_INTERNAL;
    }
    else
    {
        /* Clear the memory block which got allocated from byte pool. */
        memset (*pp_memory, 0, memory_size);
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to release the allocated memory back to byte pool.
 * This function would be called by crypto framework modules.
 *
 * @param[in]  p_memory                 Pointer to a memory block allocated in the byte pool.
 * @param[in]  memory_size              Size of the allocated memory block.
 * @retval     SSP_SUCCESS              Memory block was successfully released.
 * @retval     SSP_ERR_INTERNAL         RTOS service returned a unexpected error.
 * @note       This function is not a user API but an internal function for SF_CRYPTO_XXX which does not perform any
 *             parameter checks. It is caller's responsibility to pass valid parameters.
 **********************************************************************************************************************/
ssp_err_t sf_crypto_memory_release (void * p_memory, uint32_t memory_size)
{
    ssp_err_t   err = SSP_SUCCESS;
    uint32_t    tx_err;

    /* Erasing the memory before deallocation */
    memset (p_memory, 0, memory_size);

    /* Delete the allocated memory block in the byte pool. */
    tx_err = tx_byte_release(p_memory);
    if ((uint32_t)TX_SUCCESS != tx_err)
    {
        err = SSP_ERR_INTERNAL;
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to increment the counter which counts the number of SF_CRYPTO_XXX modules opened.
 *
 * @param[in,out]   p_api_ctrl      Pointer to a crypto framework control block
 * @note       This function is not a user API but an internal function for SF_CRYPTO_XXX. Caller is responsible to
 *             call this function to let SF_CRYPTO be aware of the SF_CRYPTO_XXX was opened. Valid parameter must be
 *             passed by caller to this function since it is not checked by this function.
 **********************************************************************************************************************/
void sf_crypto_open_counter_increment (sf_crypto_ctrl_t * const p_api_ctrl)
{
    sf_crypto_instance_ctrl_t * p_ctrl = (sf_crypto_instance_ctrl_t *) p_api_ctrl;

    /* Enter a critical section before checking the module status. */
    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    p_ctrl->open_counter++;

    SSP_CRITICAL_SECTION_EXIT;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to decrement the counter which counts the number of SF_CRYPTO_XXX modules opened.
 * If the number has already been zero, the function does nothing.
 *
 * @param[in,out]   p_api_ctrl      Pointer to a crypto framework control block
 * @note       This function is not a user API but an internal function for SF_CRYPTO_XXX. SF_CRYPTO_XXX is responsible
 *             to call this function to let SF_CRYPTO be aware of the SF_CRYPTO_XXX was opened. Valid parameter must be
 *             passed by caller to this function since it is not checked by this function.
 **********************************************************************************************************************/
void sf_crypto_open_counter_decrement (sf_crypto_ctrl_t * const p_api_ctrl)
{
    sf_crypto_instance_ctrl_t * p_ctrl = (sf_crypto_instance_ctrl_t *) p_api_ctrl;

    /* Enter a critical section before checking the module status. */
    SSP_CRITICAL_SECTION_DEFINE;
    SSP_CRITICAL_SECTION_ENTER;

    if (0U < p_ctrl->open_counter)
    {
        p_ctrl->open_counter--;
    }

    SSP_CRITICAL_SECTION_EXIT;
}

/*******************************************************************************************************************//**
 * @} (end defgroup SF_CRYPTO)
 **********************************************************************************************************************/

