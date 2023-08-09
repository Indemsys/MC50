/***********************************************************************************************************************
 * Copyright [2017-2023] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
* File Name    : r_sce_hash.h
* Description  : SCE_HASH
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @file
***********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @ingroup SCE
 * @addtogroup SCE_HASH
 * @{
***********************************************************************************************************************/

#ifndef R_SCE_HASH_H
#define R_SCE_HASH_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/

#include "r_crypto_api.h"
#include "r_hash_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define SCE_HASH_CODE_VERSION_MAJOR   (01)
#define SCE_HASH_CODE_VERSION_MINOR   (00)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/**********************************************************************************************************************
Exported global variables
***********************************************************************************************************************/

/**********************************************************************************************************************
Function Prototypes
***********************************************************************************************************************/



uint32_t R_SCE_HASH_VersionGet(ssp_version_t * const p_version);



/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // R_SCE_HASH_H

/*******************************************************************************************************************//**
 * @} (end addtogroup SCE_HASH)
***********************************************************************************************************************/
