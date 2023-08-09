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
 * File Name    : sf_crypto_cipher_private.c
 * Description  : Crypto Framework Private Module, contains definitions for all private functions required for Crypto
 *                procedures
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @file
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto_cipher.h"
#include "sf_crypto_cipher_private_api.h"
#include "../sf_crypto/sf_crypto_private_api.h"

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
 * @addtogroup SF_CRYPTO_CIPHER
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief   Opens the lower level framework TRNG instance:
 *
 * @param   [in,out]   p_ctrl       Pointer to Crypto Cipher Framework instance control block structure.
 * @param   [in]       p_cfg        Pointer to the configuration structure for Cipher module .
 *
 * @retval  SSP_SUCCESS             The sf_crypto_trng instance was opened successfully.
 * @return                          See @ref Common_Error_Codes or HAL driver for other possible
 *                                  return codes or causes. This function calls:
 *                                  sf_crypto_trng_api->open
**********************************************************************************************************************/
ssp_err_t sf_crypto_cipher_open_sf_trng_instance (sf_crypto_cipher_instance_ctrl_t * const p_ctrl,
                                                 sf_crypto_cipher_cfg_t const * const p_cfg)
{
    ssp_err_t err = SSP_SUCCESS;

     err = p_ctrl->p_sf_crypto_trng_api->open(p_cfg->p_lower_lvl_crypto_trng->p_ctrl,
                                              p_cfg->p_lower_lvl_crypto_trng->p_cfg);
     if (SSP_ERR_ALREADY_OPEN == err)
     {
         err = SSP_SUCCESS;
     }
     return err;
} /* End of function sf_crypto_cipher_open_sf_trng_instance */

/*******************************************************************************************************************//**
 * @brief   Closes the lower level framework TRNG instance:
 *
 * @param   [in,out]   p_ctrl       Pointer to Crypto Cipher Framework instance control block structure.
  *
 * @retval  SSP_SUCCESS             The sf_crypto_trng instance was opened successfully.
 * @return                          See @ref Common_Error_Codes or HAL driver for other possible
 *                                  return codes or causes. This function calls:
 *                                  sf_crypto_trng_api->open
**********************************************************************************************************************/

ssp_err_t sf_crypto_cipher_close_sf_trng_instance (sf_crypto_cipher_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;

     err = p_ctrl->p_sf_crypto_trng_api->close(p_ctrl->p_lower_lvl_sf_crypto_trng_ctrl);

     return err;
} /* End of function sf_crypto_cipher_close_sf_trng_instance */

/*******************************************************************************************************************//**
 * @} (end defgroup SF_CRYPTO_CIPHER)
 **********************************************************************************************************************/

