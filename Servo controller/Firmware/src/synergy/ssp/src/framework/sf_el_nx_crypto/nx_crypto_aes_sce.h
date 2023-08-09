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
/** Huge Number Support                                                   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_aes_sce.h                                 PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Express Logic, Inc.                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto AES module.                                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  07-15-2018     Timothy Stapko           Modified comment(s),          */
/*                                            supported GCM mode,         */
/*                                            resulting in version 5.11SP1*/
/*                                                                        */
/**************************************************************************/

#ifndef NX_CRYPTO_AES_SCE_H
#define NX_CRYPTO_AES_SCE_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "tx_api.h"
#include "nx_port.h"
#include "nx_crypto.h"

#include "r_aes_api.h"
#include "common_data.h"
extern const aes_instance_t g_sce_aes_128_cbc;
extern const aes_instance_t g_sce_aes_256_cbc;
extern const aes_instance_t g_sce_aes_128_gcm;
extern const aes_instance_t g_sce_aes_192_gcm;
extern const aes_instance_t g_sce_aes_256_gcm;
  
  
/* API return values.  */


  /* AES expects key sizes in the number of 32-bit words each key takes. */
#define NX_CRYPTO_AES_KEY_SIZE_128_BITS (4)
#define NX_CRYPTO_AES_KEY_SIZE_192_BITS (6)
#define NX_CRYPTO_AES_KEY_SIZE_256_BITS (8)

#define NX_CRYPTO_AES_256_KEY_LEN_IN_BITS (256)
#define NX_CRYPTO_AES_192_KEY_LEN_IN_BITS (192)
#define NX_CRYPTO_AES_128_KEY_LEN_IN_BITS (128)

#define NX_CRYPTO_AES_MAX_KEY_SIZE (NX_CRYPTO_AES_KEY_SIZE_256_BITS)    /* Maximum key size in bytes. */
  
#define NX_CRYPTO_AES_BLOCK_SIZE (16U) /* The AES block size for all NetX Crypto operations, in bytes. */
#define NX_CRYPTO_AES_BLOCK_SIZE_WORDS (4U) /* The AES block size for all NetX Crypto operations, in WORDS (32-bits). */
#define NX_CRYPTO_AES_BLOCK_SIZE_IN_BITS (128)
#define NX_CRYPTO_AES_IV_LEN_IN_BITS (128)
#define NX_CRYPTO_AES_IV_LEN_IN_BYTES (16U)
#define NX_CRYPTO_AES_IV_LEN_IN_WORDS (4U)
#define NX_CRYPTO_AES_GCM_TAG_LEN_IN_BYTES (16U)
#define NX_CRYPTO_AES_GCM_TAG_LEN_IN_WORDS (4U)

#define NX_CRYPTO_AES_INPUT_BUFFER_SIZE NX_CRYPTO_AES_OUTPUT_BUFFER_SIZE /* Intermediate stage AES data buffer size. */

/* Disable the alignment for AES-GCM. By default, the alignment is enabled for IAR compiler only. */
/*
#define NX_CRYPTO_AES_DISABLE_ALIGNMENT
*/

typedef struct NX_CRYPTO_AES_SCE_struct
{
    /* Number of *words* in the cipher key - can be 4 (128 bits), 6 (192 bits), or 8 (256 bits). */
    UINT nx_crypto_aes_key_size;

    /* The key schedule is as large as the key size (max = 256 bits) times the number of rounds for
       that key size (14 for 256-bit keys) plus 1. */
    UCHAR nx_crypto_aes_key_schedule[NX_CRYPTO_AES_MAX_KEY_SIZE * 15 * 4];

    /* Pointer of additional data. */
    UCHAR *nx_crypto_aes_additional_data;

    /* Length of additional data. */
    UINT nx_crypto_aes_additional_data_len;

    /* iv data buffer. */
    UCHAR nx_crypto_aes_iv_data[NX_CRYPTO_AES_IV_LEN_IN_BYTES];

#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
    /* Define output buffer when input and output overlapped. */
    UCHAR nx_crypto_aes_output_buffer[NX_CRYPTO_AES_OUTPUT_BUFFER_SIZE];

#if defined(__IAR_SYSTEMS_ICC__) && !defined(NX_CRYPTO_AES_DISABLE_ALIGNMENT)
    UCHAR nx_crypto_aes_input_buffer[NX_CRYPTO_AES_INPUT_BUFFER_SIZE];
#endif
#endif
} NX_CRYPTO_AES_SCE;


UINT nx_aes_cbc_decrypt_sce(NX_CRYPTO_AES_SCE *aes_ptr, ULONG length, UCHAR *input, UCHAR *output);
UINT nx_aes_cbc_encrypt_sce(NX_CRYPTO_AES_SCE *aes_ptr, ULONG length, UCHAR *input, UCHAR *output);
UINT nx_aes_key_set_sce(NX_CRYPTO_AES_SCE *aes_ptr, UINT operation_type, UCHAR *key, UCHAR key_size);

UINT _nx_crypto_aes_gcm_encrypt_sce(UINT op,
                                    aes_instance_t *crypto_ptr,
                                    NX_CRYPTO_AES_SCE *ctx,
                                    UCHAR *input,
                                    ULONG input_length_in_byte,
                                    UCHAR *output,
                                    UCHAR *tag);
UINT _nx_crypto_aes_gcm_encrypt_sce_be( UINT op,
                                        aes_instance_t *crypto_ptr,
                                        NX_CRYPTO_AES_SCE *ctx,
                                        UCHAR *input,
                                        ULONG input_length_in_byte,
                                        UCHAR *output,
                                        UCHAR *tag);
UINT _nx_crypto_aes_gcm_decrypt_sce(UINT op,
                                    aes_instance_t *crypto_ptr,
                                    NX_CRYPTO_AES_SCE *ctx,
                                    UCHAR *input,
                                    ULONG input_length_in_byte,
                                    UCHAR *output,
                                    UCHAR *tag);
UINT _nx_crypto_aes_gcm_decrypt_sce_be( UINT op,
                                        aes_instance_t *crypto_ptr,
                                        NX_CRYPTO_AES_SCE *ctx,
                                        UCHAR *input,
                                        ULONG input_length_in_byte,
                                        UCHAR *output,
                                        UCHAR *tag);
UINT _nx_crypto_method_aes_gcm_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
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
UINT _nx_crypto_method_aes_cbc_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
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
UINT  _nx_crypto_method_aes_sce_init(struct NX_CRYPTO_METHOD_STRUCT* method,
                                     UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                     VOID **handle,
                                     VOID *crypto_metadata,
                                     ULONG crypto_metadata_size);

UINT  _nx_crypto_method_aes_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
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

#ifdef __cplusplus
}
#endif


#endif /* NX_CRYPTO_AES_SCE_H_ */
