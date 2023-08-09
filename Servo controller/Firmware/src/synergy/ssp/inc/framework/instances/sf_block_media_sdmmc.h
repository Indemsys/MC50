/***********************************************************************************************************************
 * Copyright [2015-2023] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
* File Name    : sf_block_media_sdmmc.h
* Description  : Block Media for SDMMC header file.
***********************************************************************************************************************/


#ifndef R_BM_SDMMC_H
#define R_BM_SDMMC_H

/*******************************************************************************************************************//**
 * @file sf_block_media_sdmmc.h
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "bsp_api.h"
#include "sf_block_media_api.h"
#include "r_sdmmc_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/*******************************************************************************************************************//**
 * @ingroup SF_Library
 * @defgroup SF_BLOCK_MEDIA_SDMMC BLOCK_MEDIA_SDMMC
 * @brief RTOS-integrated Block Media framework for SDMMC driver.
 *
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define BLOCK_MEDIA_SDMMC_CODE_VERSION_MAJOR   (2U)
#define BLOCK_MEDIA_SDMMC_CODE_VERSION_MINOR   (0U)


/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

typedef struct st_block_media_on_sdmmc_cfg
{
    sdmmc_instance_t     const  * const  p_lower_lvl_sdmmc;  ///< Pointer to SDMMC instance structure
}   sf_block_media_on_sdmmc_cfg_t;

/** SDMMC block media instance control block. */
typedef struct st_sf_block_media_sdmmc_instance_ctrl
{
    uint32_t                             block_size;         ///< Block size in bytes.
    sdmmc_instance_t                   * p_lower_lvl_sdmmc;  ///< Pointer to SDMMC instance structure
    TX_EVENT_FLAGS_GROUP                 eventflag;          ///< Pointer to the event flag object for SDMMC data transfer
    uint32_t                             open;               ///< Used to determine if framework is initialized.
} sf_block_media_sdmmc_instance_ctrl_t;

/**********************************************************************************************************************
Exported global variables
***********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
extern const sf_block_media_api_t g_sf_block_media_on_sdmmc;
/** @endcond */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // R_BM_SDMMC_H

/*******************************************************************************************************************//**
 * @} (end defgroup SF_BLOCK_MEDIA_SDMMC)
***********************************************************************************************************************/


