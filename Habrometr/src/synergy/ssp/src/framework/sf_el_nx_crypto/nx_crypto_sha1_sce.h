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
/**   SHA1 Digest Algorithm (SHA1)                                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  COMPONENT DEFINITION                                   RELEASE        */ 
/*                                                                        */ 
/*    nx_crypto_sha1_sce.h                                PORTABLE C      */ 
/*                                                           5.11         */
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the symbols for Synergy SSP SCE driver.           */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */ 
/**************************************************************************/ 

#ifndef  NX_CRYPTO_SHA1_SCE_H
#define  NX_CRYPTO_SHA1_SCE_H

#include "nx_crypto.h"
#include "r_hash_api.h"
#include "common_data.h"
extern const hash_instance_t g_sce_hash_sha1;

/* Define the SHA1 context structure.  */

typedef struct NX_SHA1_SCE_STRUCT
{

    UINT        nx_sha1_states[5];                      /* Contains each state (A,B,C,D).   */
    ULONG       nx_sha1_bit_count[2];                   /* Contains the 64-bit total bit    */ 
                                                        /*   count, where index 0 holds the */ 
                                                        /*   least significant bit count and*/ 
                                                        /*   index 1 contains the most      */ 
                                                        /*   significant portion of the bit */ 
                                                        /*   count.                         */ 
    UCHAR       nx_sha1_buffer[64];                     /* Working buffer for SHA1 algorithm*/
                                                        /*   where partial buffers are      */ 
                                                        /*   accumulated until a full block */ 
                                                        /*   can be processed.              */ 
} NX_SHA1_SCE;


/* Define the function prototypes for SHA1.  */

UINT        _nx_crypto_sha1_sce_initialize(NX_SHA1_SCE *context);
UINT        _nx_crypto_sha1_sce_update(NX_SHA1_SCE *context, UCHAR *input_ptr, UINT input_length);
UINT        _nx_crypto_sha1_sce_digest_calculate(NX_SHA1_SCE *context, UCHAR digest[20]);
VOID        _nx_crypto_sha1_sce_process_buffer(NX_SHA1_SCE *context, UCHAR buffer[64]);

UINT  _nx_crypto_method_sha1_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
                                           VOID *handle, /* Crypto handler */
                                           struct NX_CRYPTO_METHOD_STRUCT* method,
                                           UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                           UCHAR* input, ULONG input_length_in_byte,
                                           UCHAR* iv_ptr,
                                           UCHAR* output, ULONG output_length_in_byte,
                                           VOID *crypto_metadata, ULONG crypto_metadata_size,
                                           VOID *packet_ptr,
                                           VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status));
#endif
