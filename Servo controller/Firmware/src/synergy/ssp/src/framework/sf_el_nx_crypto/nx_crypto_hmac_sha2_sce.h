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
/** NetX Component                                                        */
/**                                                                       */
/**   HMAC SHA256 Digest Algorithm (SHA256)                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  COMPONENT DEFINITION                                   RELEASE        */ 
/*                                                                        */ 
/*    nx_crypto_hmac_sha2_sce.h                           PORTABLE C      */ 
/*                                                           5.11         */
/*                                                                        */
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the NetX HMAC SHA256 algorithm, derived from      */
/*    RFC2202. From a user-specified number of input bytes and key, this  */ 
/*    produces a 32-byte (256-bit) digest or sometimes called a hash      */
/*    value. The resulting digest is returned in a 32-byte array supplied */
/*    by the caller.                                                      */ 
/*                                                                        */ 
/*    It is assumed that nx_api.h and nx_port.h have already been         */ 
/*    included.                                                           */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */ 
/**************************************************************************/ 

#ifndef  NX_HMAC_SHA2_SCE_H
#define  NX_HMAC_SHA2_SCE_H

#include "nx_crypto.h"
#include "nx_crypto_sha2_sce.h"

#define NX_CRYPTO_HMAC_SHA256_KEY_LEN_IN_BITS         256
#define NX_CRYPTO_HMAC_SHA256_ICV_FULL_LEN_IN_BITS    256

typedef struct NX_SHA256_HMAC_SCE_struct
{
    NX_SHA256_SCE   nx_sha256_hmac_context;
    UCHAR           nx_sha256_hmac_k_ipad[64];
    UCHAR           nx_sha256_hmac_k_opad[64];
} NX_SHA256_HMAC_SCE;

/* Define the function prototypes for HMAC SHA256.  */
UINT _nx_crypto_hmac_sha256_sce(UCHAR *input_ptr, UINT input_length,
                            UCHAR *key_ptr, UINT key_length,
                            UCHAR *digest_ptr);

UINT _nx_crypto_hmac_sha256_sce_initialize(NX_SHA256_HMAC_SCE *context, UCHAR *key_ptr, UINT key_length);
UINT _nx_crypto_hmac_sha256_sce_update(NX_SHA256_HMAC_SCE *context, UCHAR *input_ptr, UINT input_length);
UINT _nx_crypto_hmac_sha256_sce_digest_calculate(NX_SHA256_HMAC_SCE *context, UCHAR *digest_ptr);

UINT  _nx_crypto_method_hmac_sha256_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
                                                     VOID *handle, /* Crypto handler */
                                                     struct NX_CRYPTO_METHOD_STRUCT* method,
                                                     UCHAR *key,
                                                     NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                     UCHAR* input,
                                                     ULONG input_length_in_byte,
                                                     UCHAR* iv_ptr,
                                                     UCHAR* output,
                                                     ULONG output_length_in_byte,
                                                     VOID *crypto_metadata,
                                                     ULONG crypto_metadata_size,
                                                     VOID *packet_ptr,
                                                     VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status));

#endif
