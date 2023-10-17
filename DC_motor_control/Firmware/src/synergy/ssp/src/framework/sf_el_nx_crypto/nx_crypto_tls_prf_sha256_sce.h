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


#include "nx_secure_tls.h"
#include "nx_crypto_phash_sce.h"


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_crypto_tls_prf_sha256_sce.h                      PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the TLS Pseudo-Random Function (PRF) as described */
/*    in RFC 5246. This PRF is used for default key generation in TLS     */
/*    version 1.2. Ciphersuites may choose their own PRF in TLS version   */
/*    1.2 as well.                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/


typedef struct NX_SECURE_TLS_PRF_SHA_256_SCE_STRUCT {
    /* Buffer to hold PRF seed and label for concatenation. */
    UCHAR nx_secure_tls_prf_seed_label_buf[100];

    /* Pointer to secret data and its length. */
    UCHAR *nx_secure_tls_prf_secret;
    NX_CRYPTO_KEY_SIZE nx_secure_tls_prf_secret_length;

    /* PHASH metadata. */
    NX_SECURE_TLS_PHASH_SCE nx_secure_tls_phash_sha_256;

} NX_SECURE_TLS_PRF_SHA_256_SCE;


UINT _nx_crypto_tls_prf_sha256_sce(NX_SECURE_TLS_PRF_SHA_256_SCE *prf, UCHAR *secret, UINT secret_len, UCHAR *label, UINT label_len,
                                   UCHAR *seed, UINT seed_len, UCHAR *output, UINT desired_length);

UINT  _nx_crypto_method_prf_sha_256_sce_init(struct NX_CRYPTO_METHOD_STRUCT* method,
                                             UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                             VOID **handle,
                                             VOID *crypto_metadata,
                                             ULONG crypto_metadata_size);

UINT  _nx_crypto_method_prf_sha_256_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
                                                  VOID *handle, /* Crypto handler */
                                                  struct NX_CRYPTO_METHOD_STRUCT* method,
                                                  UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                  UCHAR* input, ULONG input_length_in_byte,
                                                  UCHAR* iv_ptr,
                                                  UCHAR* output, ULONG output_length_in_byte,
                                                  VOID *crypto_metadata, ULONG crypto_metadata_size,
                                                  VOID *packet_ptr,
                                                  VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status));

