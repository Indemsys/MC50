/***********************************************************************************************************************
 * Copyright [2017-2023] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
 *
 * The contents of this file (the "contents") are proprietary and confidential to Renesas Electronics Corporation
 * and/or its licensors ("Renesas") and subject to statutory and contractual protections.
 *
 * Unless otherwise expressly agreed in writing between Renesas and you: 1) you may not use, copy, modify, distribute,
 * display, or perform the contents; 2) you may not use any name or mark of Renesas for advertising or publicity
 * purposes or in connection with your use of the contents; 3) RENESAS MAKES NO WARRANTY OR REPRESENTATIONS ABOUT THE
 * SUITABILITY OF THE CONTENTS FOR ANY PURPOSE; THE CONTENTS ARE PROVIDED "AS IS" WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY, INCLUDING THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 * NON-INFRINGEMENT; AND 4) RENESAS SHALL NOT BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, OR CONSEQUENTIAL DAMAGES,
 * INCLUDING DAMAGES RESULTING FROM LOSS OF USE, DATA, OR PROJECTS, WHETHER IN AN ACTION OF CONTRACT OR TORT, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE CONTENTS. Third-party contents included in this file may
 * be subject to different terms.
 **********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : r_sce_key_installation.h
* Description  : Functions for configuring and using the Key Installation
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @file
***********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @ingroup SCE
 * @addtogroup SCE_KEY_INSTALLATION
 * @{
***********************************************************************************************************************/

#ifndef R_SCE_KEY_INSTALLATION_H_
#define R_SCE_KEY_INSTALLATION_H_

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/

#include "r_crypto_api.h"
#include "r_key_installation_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define SCE_KEY_INSTALLATION_CODE_VERSION_MAJOR   (02U)
#define SCE_KEY_INSTALLATION_CODE_VERSION_MINOR   (00U)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/** Key Installation Interface control structure */
typedef struct
{
    crypto_ctrl_t       * p_crypto_ctrl;  ///< pointer to crypto engine control structure
    crypto_api_t const  * p_crypto_api;   ///< pointer to crypto engine API
} key_installation_instance_ctrl_t;

/**********************************************************************************************************************
Exported global variables
***********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* R_SCE_KEY_INSTALLATION_H_ */

/*******************************************************************************************************************//**
 * @} (end addtogroup SCE_KEY_INSTALLATION)
***********************************************************************************************************************/
