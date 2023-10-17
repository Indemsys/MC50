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
 * File Name    : sf_crypto.h
 * Description  : Interface definition for SSP Crypto Framework Common Module instance.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup SF_Library
 * @defgroup SF_CRYPTO SSP Crypto Common Framework
 * @brief RTOS-integrated Crypto Common Framework Module.
 *
 * @{
 **********************************************************************************************************************/

#ifndef SF_CRYPTO_H
#define SF_CRYPTO_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto_api.h"
#include "sf_crypto_cfg.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** The API version of SSP Crypto Framework Common Module */
#define SF_CRYPTO_CODE_VERSION_MAJOR       (2U)
#define SF_CRYPTO_CODE_VERSION_MINOR       (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** SSP Crypto Framework Common Module instance control block */
typedef struct st_sf_crypto_instance_ctrl
{
    sf_crypto_state_t    status;                ///< Module status
    TX_MUTEX        mutex;                      ///< Mutex used in the Crypto Framework
    TX_SEMAPHORE    semaphore;                  ///< Semaphore used in the Crypto Framework (Reserve)
    TX_BYTE_POOL    byte_pool;                  ///< Byte pool used in the Crypto Framework
    uint32_t        wait_option;                ///< Wait time option used for RTOS service calls
    uint32_t        open_counter;               ///< Counter to keep the number of SF_CRYPTO_XXX opened
    void          * p_lower_lvl_crypto;         ///< Pointer to a low-level Crypto engine HAL driver instance
    void         (* p_callback)(sf_crypto_callback_args_t * p_args); ///< Pointer to callback function
    void          * p_context;                  ///< Pointer to a context
    sf_crypto_close_option_t    close_option;   ///< Close option
} sf_crypto_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const sf_crypto_api_t    g_sf_crypto_api;
/** @endcond */

/** @} (end defgroup SF_CRYPTO) */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_CRYPTO_H */
