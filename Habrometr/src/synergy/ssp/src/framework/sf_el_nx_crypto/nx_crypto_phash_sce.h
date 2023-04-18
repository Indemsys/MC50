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
/**    Transport Layer Security (TLS)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#ifndef SRC_NX_SECURE_PHASH_SCE_H_
#define SRC_NX_SECURE_PHASH_SCE_H_


#include "nx_crypto_hmac_sha2_sce.h"

/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_secure_tls_phash_sce.h                           PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the TLS P-HASH function described in RFCs 2246,   */
/*    4346, and 5246. It is used in the TLS PRF function as a wrapper to  */
/*    various hash routines to generate arbitrary-length data.            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/


typedef struct NX_SECURE_TLS_PHASH_SCE_STRUCT {
    UCHAR nx_secure_phash_temp_a[110];    /* This needs to be large enough to hold the seed + the label -
                                              "master secret" + 64 bytes random = 77 bytes, plus
                                               the hash size (32 bytes for SHA-256). */

    UCHAR hash_metadata_area[sizeof(NX_SHA256_HMAC_SCE) * 2];
} NX_SECURE_TLS_PHASH_SCE;

extern NX_CRYPTO_METHOD crypto_method_hmac_md5_sce;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha1_sce;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha256_sce;

#define _nx_crypto_phash_md5(phash, secret, s_len, seed, seed_len, output, desired_length) \
        _nx_crypto_phash_sce(phash, secret, s_len, seed, seed_len, output, desired_length, &crypto_method_hmac_md5_sce)

#define _nx_crypto_phash_sha_1_sce(phash, secret, s_len, seed, seed_len, output, desired_length) \
        _nx_crypto_phash_sce(phash, secret, s_len, seed, seed_len, output, desired_length, &crypto_method_hmac_sha1_sce)

#define _nx_crypto_phash_sha_256_sce(phash, secret, s_len, seed, seed_len, output, desired_length) \
        _nx_crypto_phash_sce(phash, secret, s_len, seed, seed_len, output, desired_length, &crypto_method_hmac_sha256_sce)



UINT _nx_crypto_phash_sce(NX_SECURE_TLS_PHASH_SCE *phash, UCHAR *secret, UINT secret_len, UCHAR *seed, UINT seed_len,
                            UCHAR *output, UINT desired_length, NX_CRYPTO_METHOD *hash_method);


#endif
