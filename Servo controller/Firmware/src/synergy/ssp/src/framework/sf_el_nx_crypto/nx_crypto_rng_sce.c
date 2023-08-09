/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

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

/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** NetX Component                                                        */
/**                                                                       */
/**   RNG                                                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "r_trng_api.h"

extern const trng_instance_t g_sce_trng;
int _nx_crypto_rng_sce(void);
int rand(void);
void srand(unsigned int seed);

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_crypto_rng_sce                                   PORTABLE C     */
/*                                                           5.11 SP1     */
/*                                                                        */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function generates a random number.                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    random number                                                       */
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */
/*  07-15-2018     Timothy Stapko           Initial Version 5.11SP1       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-07-2019     Renesas                  Updated to check for error    */
/*                                                                        */
/*                                                                        */ 
/**************************************************************************/
int _nx_crypto_rng_sce(void)
{
    uint32_t rnd;
    uint32_t hal_err;

    /* Generate a 32-bit long random number.  */
    hal_err = g_sce_trng.p_api -> read(g_sce_trng.p_ctrl, &rnd, 1);
    if (SSP_SUCCESS != hal_err)
    {
        return 0;
    }

    /* Return the generated random number.  */
    return((int)rnd);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    rand                                                                */
/*                                                                        */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Renesas Electronics Corporation                                     */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function overrides the C library function to use the HW TRNG.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    random number                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-07-2019         Renesas              Added override to the         */
/*                                          rand() function.              */
/*                                                                        */
/*                                                                        */
/**************************************************************************/

int rand(void)
{
    return (_nx_crypto_rng_sce());
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_rng_sce                                   PORTABLE C     */
/*                                                           5.11 SP1     */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function overrides the C lib function srand() called from      */
/*    the BSP code to avoid a multiple re-definition error when           */
/* overriding rand()                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  06-04-2019     Renesas                  Updated to check for error    */
/*                                                                        */
/*                                                                        */
/**************************************************************************/
void srand(unsigned int seed)
{
    SSP_PARAMETER_NOT_USED(seed);
}
