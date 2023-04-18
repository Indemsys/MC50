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
/** NetX Secure Component                                                 */
/**                                                                       */
/**    Transport Layer Security (TLS) - PHASH function                    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


#include "nx_crypto_phash_sce.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_phash_sce                                PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs the TLS-specific PHASH function which is     */
/*    used in the TLS key-generation process. It requires a hash function */
/*    such as SHA-256 or MD5 to be passed in as a function pointer.       */
/*    PHASH generates an arbitrary number of cryptographic bytes given a  */
/*    hash secret (essentially a key) and a cryptographic seed .          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    phash                                 PHASH control block           */
/*    secret                                Hash secret (key)             */
/*    secret_len                            Length of secret in bytes     */
/*    seed                                  Cryptographic seed            */
/*    seed_len                              Length of seed in bytes       */
/*    output                                Pointer to output buffer      */
/*    desired_length                        Desired length of output data */
/*    hash_method                           Pointer to hash method        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Handle phash operation        */
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
/*                                            fixed the digest length,    */
/*                                            resulting in version 5.11SP1*/
/*                                                                        */
/**************************************************************************/
UINT _nx_crypto_phash_sce(NX_SECURE_TLS_PHASH_SCE *phash, UCHAR *secret, UINT secret_len, UCHAR *seed, UINT seed_len,
                      UCHAR *output, UINT desired_length, NX_CRYPTO_METHOD *hash_method)
{

UINT offset, a_len;
UINT digest_len;
UCHAR *temp_a;
UINT temp_a_size;

    /* From the RFC:
        P_hash(secret, seed) = HMAC_hash(secret, A(1) + seed) +
                               HMAC_hash(secret, A(2) + seed) +
                               HMAC_hash(secret, A(3) + seed) + ...

        Where + indicates concatenation.

        A() is defined as:

        A(0) = seed
        A(i) = HMAC_hash(secret, A(i-1))
    */

    /* Make sure we don't write past the end of our buffers. */
    if(seed_len > 77 || secret_len > 48 || hash_method == NX_CRYPTO_NULL)
    {
        return(NX_CRYPTO_INVALID_PARAMETER);
    }

    /* Get a pointer to our workspace. */
    temp_a = phash->nx_secure_phash_temp_a;
    temp_a_size = sizeof(phash->nx_secure_phash_temp_a);

    /* Get the length of the digest output. */
    digest_len = (hash_method -> nx_crypto_ICV_size_in_bits >> 3);

    /* Clear out our buffers and copy the seed into our A buffer to kick things off. */
    memset(temp_a, 0, temp_a_size);

    memcpy(temp_a, seed, seed_len);
    a_len = seed_len;

    for(offset = 0; offset < desired_length; offset += digest_len)
    {
        /* Calculate A[i]. */
        if(hash_method -> nx_crypto_operation != NX_CRYPTO_NULL)
        {
            hash_method -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE,
                                               NX_CRYPTO_NULL,
                                               hash_method,
                                               secret,
                                               (NX_CRYPTO_KEY_SIZE)(secret_len << 3),
                                               temp_a,
                                               a_len,
                                               NX_CRYPTO_NULL,
                                               temp_a,
                                               temp_a_size,
                                               phash->hash_metadata_area,
                                               sizeof(phash->hash_metadata_area),
                                               NX_CRYPTO_NULL,
											   NX_CRYPTO_NULL);
        }

        a_len = digest_len;

        /* Concatenate A[i] and seed to feed into digest. */
        memcpy(&temp_a[a_len], seed, seed_len);

        /* Calculate p-hash block, store in output. */
        if(hash_method -> nx_crypto_operation != NX_CRYPTO_NULL)
        {
            hash_method -> nx_crypto_operation(NX_CRYPTO_AUTHENTICATE,
                                               NX_CRYPTO_NULL,
                                               hash_method,
                                               secret,
                                               (NX_CRYPTO_KEY_SIZE)(secret_len << 3),
                                               temp_a,
                                               a_len + seed_len,
                                               NX_CRYPTO_NULL,
                                               &output[offset],
                                               temp_a_size,
                                               phash->hash_metadata_area,
                                               sizeof(phash->hash_metadata_area),
                                               NX_CRYPTO_NULL,
                                               NX_CRYPTO_NULL);
        }
    }
    return(NX_CRYPTO_SUCCESS);
}

