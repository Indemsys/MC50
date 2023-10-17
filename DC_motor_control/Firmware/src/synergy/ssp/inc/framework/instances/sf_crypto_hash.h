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
 * File Name    : sf_crypto_hash.h
 * Description  : Interface definition for SSP Crypto HASH Framework module instance.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup SF_Library
 * @defgroup SF_CRYPTO_HASH SSP Crypto Hash Framework
 * @brief RTOS-integrated Crypto HASH Framework Module.
 *
 * @{
 **********************************************************************************************************************/

#ifndef SF_CRYPTO_HASH_H
#define SF_CRYPTO_HASH_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto_hash_api.h"
#include "sf_crypto_hash_cfg.h"
#include "sf_crypto.h"
#include "r_crypto_api.h"

/* Includes Cryptography algorithms. */
#include "r_hash_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** The API version of SSP Crypto Framework */
#define SF_CRYPTO_HASH_CODE_VERSION_MAJOR       (2U)
#define SF_CRYPTO_HASH_CODE_VERSION_MINOR       (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** SSP Crypto HASH Framework instance control block */
typedef struct st_sf_crypto_hash_instance_ctrl
{
    sf_crypto_hash_state_t      status;             ///< Module status
    sf_crypto_hash_type_t       hash_type;          ///< HASH algorithm type
    sf_crypto_hash_context_t    hash_context;       ///< Context for calculating message digest
    sf_crypto_instance_t      * p_lower_lvl_crypto_common;  ///< Pointer to a Crypto Framework common instance
    hash_instance_t           * p_lower_lvl_instance;       ///< pointer to lower-level crypto module control structure
} sf_crypto_hash_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const sf_crypto_hash_api_t    g_sf_crypto_hash_api;
/** @endcond */

/** @} (end defgroup SF_CRYPTO_HASH) */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_CRYPTO_HASH_H */
