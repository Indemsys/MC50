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
/**   HMAC MD5 Digest Algorithm (MD5)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  COMPONENT DEFINITION                                   RELEASE        */ 
/*                                                                        */ 
/*    nx_crypto_hmac_md5_sce.h                           PORTABLE C       */ 
/*                                                           5.11         */
/*                                                                        */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This is the header file for MD5 operating using Synergy hardware    */ 
/*    accelerator.                                                        */ 
/*                                                                        */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */ 
/**************************************************************************/ 

#ifndef  NX_HMAC_MD5_SCE_H
#define  NX_HMAC_MD5_SCE_H

#include "nx_crypto.h"
#include "nx_crypto_md5_sce.h"

#define NX_CRYPTO_HMAC_MD5_KEY_LEN_IN_BITS      128
#define NX_CRYPTO_HMAC_MD5_ICV_FULL_LEN_IN_BITS 128

typedef struct NX_MD5_HMAC_SCE_struct
{
    NX_MD5_SCE  nx_md5_hmac_context;
    UCHAR       nx_md5_hmac_k_ipad[64];
    UCHAR       nx_md5_hmac_k_opad[64];
} NX_MD5_HMAC_SCE;



/* Define the function prototypes for HMAC MD5.  */
UINT _nx_crypto_hmac_md5_sce(NX_MD5_HMAC_SCE *context, UCHAR *input_ptr, UINT input_length, UCHAR *key_ptr,
                             UINT key_length, UCHAR *digest_ptr);

UINT _nx_crypto_hmac_md5_sce_initialize(NX_MD5_HMAC_SCE *context, UCHAR *key_ptr, UINT key_length);
UINT _nx_crypto_hmac_md5_sce_update(NX_MD5_HMAC_SCE *context, UCHAR *input_ptr, UINT input_length);
UINT _nx_crypto_hmac_md5_sce_digest_calculate(NX_MD5_HMAC_SCE *context, UCHAR *digest_ptr);

UINT _nx_crypto_method_hmac_md5_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
                                              VOID *handle, /* Crypto handler */
                                              struct NX_CRYPTO_METHOD_STRUCT *method,
                                              UCHAR *key,
                                              NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                              UCHAR *input,
                                              ULONG input_length_in_byte,
                                              UCHAR *iv_ptr,
                                              UCHAR *output,
                                              ULONG output_length_in_byte,
                                              VOID *crypto_metadata, ULONG crypto_metadata_size,
                                              VOID *packet_ptr,
                                              VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status));
#endif

