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
/*    nx_crypto_rsa_sce.h                                 PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto RSA module.                                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/

#ifndef NX_CRYPTO_RSA_SCE_H
#define NX_CRYPTO_RSA_SCE_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Include the ThreadX and port-specific data type file.  */

#include "tx_api.h"
#include "nx_crypto.h"
    /*#include "nx_port.h"*/

#include "r_rsa_api.h"
#include "common_data.h"
#include "nx_crypto_rsa.h"

  
/* API return values.  */


extern const rsa_instance_t g_sce_rsa_2048;
extern const rsa_instance_t g_sce_rsa_1024;

/* RSA Key exponent length in BYTE.*/

typedef enum NX_RSA_EXP_LENGTH_ENUM
{
    NX_RSA_EXP_LENGTH_1_BYTE = 1,               /*1*/
    NX_RSA_EXP_LENGTH_2_BYTE,                   /*2*/
    NX_RSA_EXP_LENGTH_3_BYTE,                   /*3*/
    NX_RSA_EXP_LENGTH_4_BYTE,                   /*4*/
    NX_RSA_EXP_LENGTH_128_BYTE = 128,           /*128*/
    NX_RSA_EXP_LENGTH_129_BYTE,                 /*129*/
    NX_RSA_EXP_LENGTH_256_BYTE = 256,           /*256*/
    NX_RSA_EXP_LENGTH_257_BYTE,                 /*257*/
    NX_RSA_EXP_LENGTH_512_BYTE = 512            /*512*/
} NX_RSA_EXP_LENGTH;

/* RSA Key Modulus length in BYTE.*/
#define NX_RSA_MOD_LENGTH_128_BYTE  128     /*128*/
#define NX_RSA_MOD_LENGTH_256_BYTE  256     /*256*/
#define NX_RSA_MOD_LENGTH_512_BYTE  512     /*512*/


#define RSA_KEY_SIZE_4096_BITS   4096

typedef struct NX_CRYPTO_RSA_SCE_struct
{
    /* Pointer to the rsa modulus. */
    UCHAR *nx_crypto_rsa_modulus;

    /* RSA modulus length in bytes */
    UINT nx_crypto_rsa_modulus_length;

    /* Scratch buffer for RSA calculations. */
    USHORT nx_crypto_rsa_scratch_buffer[256];


} NX_CRYPTO_RSA_SCE;






UINT  _nx_crypto_method_rsa_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
                                          VOID *handle, /* Crypto handler */
                                          struct NX_CRYPTO_METHOD_STRUCT* method,
                                          UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                          UCHAR* input, ULONG input_length_in_byte,
                                          UCHAR* iv_ptr,
                                          UCHAR* output, ULONG output_length_in_byte,
                                          VOID *crypto_metadata, ULONG crypto_metadata_size,
                                          VOID *packet_ptr,
                                          VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status));

UINT  _nx_crypto_method_rsa_sce_init(struct NX_CRYPTO_METHOD_STRUCT* method,
                                     UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                     VOID **handle,
                                     VOID *crypto_metadata, ULONG crypto_metadata_size);

#ifdef __cplusplus
}
#endif

#endif /* NX_CRYPTO_RSA_SCE_H_ */
