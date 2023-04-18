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


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Crypto Component                                                 */
/**                                                                       */
/**     Renesas Synergy Crypto engine                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "r_aes_api.h"
#include "r_hash_api.h"
#include "r_rsa_api.h"
#include "r_trng_api.h"
#include "r_ecc_api.h"
#include "nx_crypto_sce.h"

ssp_err_t sce_initialize(void);

aes_ctrl_t g_sce_aes_128_cbc_ctrl;
aes_cfg_t g_sce_aes_128_cbc_cfg =
{ .p_crypto_api = &g_sce_crypto_api };
const aes_instance_t g_sce_aes_128_cbc =
{ .p_ctrl = &g_sce_aes_128_cbc_ctrl, .p_cfg = &g_sce_aes_128_cbc_cfg, .p_api = &g_aes128cbc_on_sce };


aes_ctrl_t g_sce_aes_256_cbc_ctrl;
aes_cfg_t g_sce_aes_256_cbc_cfg =
{ .p_crypto_api = &g_sce_crypto_api };
const aes_instance_t g_sce_aes_256_cbc =
{ .p_ctrl = &g_sce_aes_256_cbc_ctrl, .p_cfg = &g_sce_aes_256_cbc_cfg, .p_api = &g_aes256cbc_on_sce };

aes_ctrl_t g_sce_aes_128_gcm_ctrl;
aes_cfg_t g_sce_aes_128_gcm_cfg =
{ .p_crypto_api = &g_sce_crypto_api };
const aes_instance_t g_sce_aes_128_gcm =
{ .p_ctrl = &g_sce_aes_128_gcm_ctrl, .p_cfg = &g_sce_aes_128_gcm_cfg, .p_api = &g_aes128gcm_on_sce };

aes_ctrl_t g_sce_aes_192_gcm_ctrl;
aes_cfg_t g_sce_aes_192_gcm_cfg =
{ .p_crypto_api = &g_sce_crypto_api };
const aes_instance_t g_sce_aes_192_gcm =
{ .p_ctrl = &g_sce_aes_192_gcm_ctrl, .p_cfg = &g_sce_aes_192_gcm_cfg, .p_api = &g_aes192gcm_on_sce };

aes_ctrl_t g_sce_aes_256_gcm_ctrl;
aes_cfg_t g_sce_aes_256_gcm_cfg =
{ .p_crypto_api = &g_sce_crypto_api };
const aes_instance_t g_sce_aes_256_gcm =
{ .p_ctrl = &g_sce_aes_256_gcm_ctrl, .p_cfg = &g_sce_aes_256_gcm_cfg, .p_api = &g_aes256gcm_on_sce };

/** Synergy Crypto Engine MD5 driver instance which is used by nx_crypto_md5_sce to calculate MD5 hash */
hash_ctrl_t g_sce_hash_md5_ctrl;
hash_cfg_t g_sce_hash_md5_cfg;
hash_cfg_t g_sce_hash_md5_cfg =
{
    .p_crypto_api = &g_sce_crypto_api
};
const hash_instance_t g_sce_hash_md5 =
{
    .p_ctrl = &g_sce_hash_md5_ctrl ,
    .p_cfg = &g_sce_hash_md5_cfg ,
    .p_api = &g_md5_hash_on_sce
};


hash_ctrl_t g_sce_hash_sha1_ctrl;
hash_cfg_t g_sce_hash_sha1_cfg =
{
    .p_crypto_api = &g_sce_crypto_api
};
const hash_instance_t g_sce_hash_sha1 =
{
    .p_ctrl = &g_sce_hash_sha1_ctrl ,
    .p_cfg = &g_sce_hash_sha1_cfg ,
    .p_api = &g_sha1_hash_on_sce
};


hash_ctrl_t g_sce_hash_sha2_ctrl;
hash_cfg_t g_sce_hash_sha2_cfg =
{
    .p_crypto_api = &g_sce_crypto_api
};
const hash_instance_t g_sce_hash_sha2 =
{
    .p_ctrl = &g_sce_hash_sha2_ctrl ,
    .p_cfg = &g_sce_hash_sha2_cfg ,
    .p_api = &g_sha256_hash_on_sce
};


rsa_ctrl_t g_sce_rsa_1024_ctrl;
rsa_cfg_t g_sce_rsa_1024_cfg =
{
    .p_crypto_api = &g_sce_crypto_api
};
const rsa_instance_t g_sce_rsa_1024 =
{
    .p_ctrl = &g_sce_rsa_1024_ctrl ,
    .p_cfg = &g_sce_rsa_1024_cfg ,
    .p_api = &g_rsa1024_on_sce
};


rsa_ctrl_t g_sce_rsa_2048_ctrl;
rsa_cfg_t g_sce_rsa_2048_cfg =
{
    .p_crypto_api = &g_sce_crypto_api
};
const rsa_instance_t g_sce_rsa_2048 =
{
    .p_ctrl = &g_sce_rsa_2048_ctrl ,
    .p_cfg = &g_sce_rsa_2048_cfg ,
    .p_api = &g_rsa2048_on_sce
};

trng_ctrl_t g_sce_trng_ctrl;
trng_cfg_t g_sce_trng_cfg =
{ .p_crypto_api = &g_sce_crypto_api, .nattempts = 2 };
const trng_instance_t g_sce_trng =
{ .p_ctrl = &g_sce_trng_ctrl, .p_cfg = &g_sce_trng_cfg, .p_api = &g_trng_on_sce };

ecc_ctrl_t g_sce_ecc_ctrl192;
ecc_ctrl_t g_sce_ecc_ctrl256;
ecc_cfg_t g_sce_ecc_cfg =
{
    .p_crypto_api = &g_sce_crypto_api
};
const ecc_instance_t g_sce_ecc192 =
{
    .p_ctrl = &g_sce_ecc_ctrl192 ,
    .p_cfg = &g_sce_ecc_cfg ,
    .p_api = &g_ecc192_on_sce
};
const ecc_instance_t g_sce_ecc256 =
{
    .p_ctrl = &g_sce_ecc_ctrl256 ,
    .p_cfg = &g_sce_ecc_cfg ,
    .p_api = &g_ecc256_on_sce
};


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    sce_initialize                                      PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes crypto driver.                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    ssp_err_t                             Error code returned.          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  07-15-2018     Timothy Stapko           Modified comment(s),          */
/*                                            supported random number     */
/*                                            generation, supported ECC,  */
/*                                            resulting in version 5.11SP1*/
/*                                                                        */
/**************************************************************************/
ssp_err_t sce_initialize(void)
{
    ssp_err_t err = SSP_SUCCESS;

    /* Open the secure crypto engine driver and ensure initialization is done here or already in other
     * SCE supported stack. */
    err = (ssp_err_t)(p_lower_lvl_hw_crypto->p_api->open(p_lower_lvl_hw_crypto->p_ctrl, p_lower_lvl_hw_crypto->p_cfg));
    if ((SSP_SUCCESS != err) && (SSP_ERR_CRYPTO_SCE_ALREADY_OPEN != err))
    {
        return err;
    }

    /* Open the AES driver */
    err = (ssp_err_t)(g_sce_aes_128_cbc.p_api->open(g_sce_aes_128_cbc.p_ctrl, g_sce_aes_128_cbc.p_cfg));
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    err = (ssp_err_t)(g_sce_aes_256_cbc.p_api->open(g_sce_aes_256_cbc.p_ctrl, g_sce_aes_256_cbc.p_cfg));
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /* Open the AES 128 GCM driver and close it just to ensure its availability. 
        It will be opened and closed for every encrypt and decrypt operation.*/
    err = (ssp_err_t)(g_sce_aes_128_gcm.p_api->open(g_sce_aes_128_gcm.p_ctrl, g_sce_aes_128_gcm.p_cfg));
    if (SSP_SUCCESS != err)
    {
        return err;
    }

     err = (ssp_err_t)(g_sce_aes_128_gcm.p_api->close(g_sce_aes_128_gcm.p_ctrl));
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /* Open the AES 192 GCM driver and close it just to ensure its availability. 
        It will be opened and closed for every encrypt and decrypt operation.*/
    err = (ssp_err_t)(g_sce_aes_192_gcm.p_api->open(g_sce_aes_192_gcm.p_ctrl, g_sce_aes_192_gcm.p_cfg));
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    
    err = (ssp_err_t)(g_sce_aes_192_gcm.p_api->close(g_sce_aes_192_gcm.p_ctrl));
    if (SSP_SUCCESS != err)
    {
        return err;
    }    
    
    /* Open the AES 256 GCM driver and close it just to ensure its availability. 
        It will be opened and closed for every encrypt and decrypt operation.*/
    err = (ssp_err_t)(g_sce_aes_256_gcm.p_api->open(g_sce_aes_256_gcm.p_ctrl, g_sce_aes_256_gcm.p_cfg));
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    
    err = (ssp_err_t)(g_sce_aes_256_gcm.p_api->close(g_sce_aes_256_gcm.p_ctrl));
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /* Open the RSA driver */
    err = (ssp_err_t)(g_sce_rsa_1024.p_api->open(g_sce_rsa_1024.p_ctrl, g_sce_rsa_1024.p_cfg));
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    err = (ssp_err_t)(g_sce_rsa_2048.p_api->open(g_sce_rsa_2048.p_ctrl, g_sce_rsa_2048.p_cfg));
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /* Open the HASH driver */
    err = (ssp_err_t)(g_sce_hash_md5.p_api->open(g_sce_hash_md5.p_ctrl, g_sce_hash_md5.p_cfg));
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    err = (ssp_err_t)(g_sce_hash_sha1.p_api->open(g_sce_hash_sha1.p_ctrl, g_sce_hash_sha1.p_cfg));
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    err = (ssp_err_t)(g_sce_hash_sha2.p_api->open(g_sce_hash_sha2.p_ctrl, g_sce_hash_sha2.p_cfg));
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    /* Open the TRNG driver */
    err = (ssp_err_t)(g_sce_trng.p_api->open(g_sce_trng.p_ctrl, g_sce_trng.p_cfg));
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    
    /* Open the ECC driver */
    err = g_sce_ecc192.p_api->open(g_sce_ecc192.p_ctrl, g_sce_ecc192.p_cfg);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
    
        
    err = g_sce_ecc256.p_api->open(g_sce_ecc256.p_ctrl, g_sce_ecc256.p_cfg);
    /* SSP_SUCCESS will be returned if successful else the error code will be returned. */
    return err;
}
