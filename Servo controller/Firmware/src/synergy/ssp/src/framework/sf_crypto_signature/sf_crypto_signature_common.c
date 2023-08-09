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
 * File Name    : sf_crypto_signature_common.c
 * Description  : Crypto Signature Framework Module common functions.
***********************************************************************************************************************/

/***********************************************************************************************************************
 * @file
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto_signature_api.h"
#include "sf_crypto_signature.h"
#include "sf_crypto_signature_private.h"
#include "sf_crypto_signature_common.h"
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static
ssp_err_t
sf_crypto_signature_check_next_state(sf_crypto_signature_operation_state_t next_state,
                                     sf_crypto_signature_operation_state_t check_state);
/***********************************************************************************************************************
 * Private global variables
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
 * @brief   SSP Crypto Signature Framework State transition validation for sign operation.
 *
 * @param[in]   p_ctrl                         Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]   next_state                     Requested next state.
 *
 * @retval      SSP_SUCCESS                    Valid call to the calling API.
 * @retval      SSP_ERR_INVALID_CALL           Invalid call to the calling API.
 **********************************************************************************************************************/
ssp_err_t
sf_crypto_signature_validate_sign_operation_state_transition (sf_crypto_signature_instance_ctrl_t * p_ctrl,
                                                              sf_crypto_signature_operation_state_t next_state)
{
    ssp_err_t iret = SSP_ERR_INVALID_CALL;
    switch(p_ctrl->operation_state)
    {
        case SF_CRYPTO_SIGNATURE_OPERATION_STATE_OPEN:
        {
            switch (next_state)
            {
                case SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_INITIALIZED:
                {
                break;
                }
                default:
                {
                    return iret;
                break;
                }
            }
        break;
        }
        case SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_INITIALIZED:
        {
            switch (next_state)
            {
                case SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_UPDATED:
                case SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_FINALIZED:
                {
                break;
                }
                default:
                {
                    return iret;
                break;
                }
            }
        break;
        }
        case SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_UPDATED:
        {
            switch (next_state)
            {
                case SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_UPDATED:
                case SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_FINALIZED:
                {
                break;
                }
                default:
                {
                    return iret;
                break;
                }
            }
        break;
        }
        case SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_FINALIZED:
        {
            switch (next_state)
            {
                case SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_INITIALIZED:
                {
                break;
                }
                default:
                {
                    return iret;
                break;
                }
            }
        break;
        }
        case SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_FINALIZED:
        {
            iret = sf_crypto_signature_check_next_state(next_state,
                                                        SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_INITIALIZED);
            return iret;
        break;
        }
        default:
        {
            return SSP_ERR_INVALID_CALL;
        break;
        }
    }
    return (SSP_SUCCESS);
}

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Signature Framework State transition validation for verify operation.
 *
 * @param[in]   p_ctrl                         Pointer to Crypto Signature Framework instance control block structure.
 * @param[in]   next_state                     Requested next state.
 *
 * @retval SSP_SUCCESS                         Valid call to the calling API.
 * @retval SSP_ERR_INVALID_CALL                Invalid call to the calling API.
 **********************************************************************************************************************/
ssp_err_t
sf_crypto_signature_validate_verify_operation_state_transition (sf_crypto_signature_instance_ctrl_t * p_ctrl,
                                                                sf_crypto_signature_operation_state_t next_state)
{
    ssp_err_t iret = SSP_ERR_INVALID_CALL;
    switch(p_ctrl->operation_state)
    {
        case SF_CRYPTO_SIGNATURE_OPERATION_STATE_OPEN:
        {
            switch (next_state)
            {
                case SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_INITIALIZED:
                {
                break;
                }
                default:
                {
                    return iret;
                break;
                }
            }
        break;
        }
        case SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_INITIALIZED:
        {
            switch (next_state)
            {
                case SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_UPDATED:
                case SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_FINALIZED:
                {
                break;
                }
                default:
                {
                    return iret;
                break;
                }
            }
        break;
        }
        case SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_UPDATED:
        {
            switch (next_state)
            {
                case SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_UPDATED:
                case SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_FINALIZED:
                {
                break;
                }
                default:
                {
                    return iret;
                break;
                }
            }
        break;
        }
        case SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_FINALIZED:
        {
            switch (next_state)
            {
                case SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_INITIALIZED:
                {
                break;
                }
                default:
                {
                    return iret;
                break;
                }
            }
        break;
        }
        case SF_CRYPTO_SIGNATURE_OPERATION_STATE_SIGN_FINALIZED:
        {
            iret = sf_crypto_signature_check_next_state(next_state,
                                                        SF_CRYPTO_SIGNATURE_OPERATION_STATE_VERIFY_INITIALIZED);
            return iret;
        break;
        }
        default:
        {
            return SSP_ERR_INVALID_CALL;
        break;
        }
    }
    return (SSP_SUCCESS);
}
/*******************************************************************************************************************//**
 * @} (end defgroup SF_CRYPTO_SIGNATURE)
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief   SSP Crypto Signature Framework check current and next state
 *
 * @param[in]   next_state                Next state
 * @param[in]   check_state               State to check against
 *
 * @retval SSP_SUCCESS                            Next state and check state are the same.
 * @retval SSP_ERR_INVALID_CALL                   Next state and Check state are not the same.
 **********************************************************************************************************************/

static
ssp_err_t
sf_crypto_signature_check_next_state (sf_crypto_signature_operation_state_t next_state,
                                      sf_crypto_signature_operation_state_t check_state)
{

    if(check_state == next_state)
    {
        return SSP_SUCCESS;
    }
    return SSP_ERR_INVALID_CALL;
}
