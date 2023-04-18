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


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Crypto Component                                                 */
/**                                                                       */
/**   Elliptic Curve Digital Signature Algorithm (ECDSA)                  */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_crypto_ecdsa_sce.h                                PORTABLE C     */
/*                                                          5.11 SP1      */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX ECDSA module.                                                  */
/*                                                                        */
/*    It is assumed that nx_api.h and nx_port.h have already been         */
/*    included.                                                           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  07-15-2018     Timothy Stapko           Initial Version 5.11 SP1      */
/*                                                                        */
/**************************************************************************/

#ifndef  NX_CRYPTO_ECDSA_SCE_H
#define  NX_CRYPTO_ECDSA_SCE_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_crypto.h"
#include "nx_crypto_ec_sce.h"


#ifndef NX_CRYPTO_ECDSA_SCE_MAX_KEY_SIZE
#define NX_CRYPTO_ECDSA_SCE_MAX_KEY_SIZE     (32)
#endif /* NX_CRYPTO_ECDSA_SCE_MAX_KEY_SIZE */


/* ECDSA signature structure. */
typedef struct NX_CRYPTO_ECDSA_SCE
{
    NX_CRYPTO_EC_SCE *nx_crypto_ecdsa_curve;
    ULONG             nx_crypto_ecdsa_hash[NX_CRYPTO_ECDSA_SCE_MAX_KEY_SIZE >> 2];
    ULONG             nx_crypto_ecdsa_signature_r[NX_CRYPTO_ECDSA_SCE_MAX_KEY_SIZE >> 2];
    ULONG             nx_crypto_ecdsa_signature_s[NX_CRYPTO_ECDSA_SCE_MAX_KEY_SIZE >> 2];
} NX_CRYPTO_ECDSA_SCE;

/* Define the function prototypes for ECDSA.  */

UINT _nx_crypto_ecdsa_sign_sce(NX_CRYPTO_ECDSA_SCE *ecdsa_ptr,
                               UCHAR *hash,
                               UINT hash_length,
                               UCHAR *private_key,
                               UINT private_key_length,
                               UCHAR *signature,
                               ULONG signature_length,
                               ULONG *actual_signature_length);

UINT _nx_crypto_ecdsa_verify_sce(NX_CRYPTO_ECDSA_SCE *ecdsa_ptr,
                                 UCHAR *hash,
                                 UINT hash_length,
                                 UCHAR *public_key,
                                 UINT public_key_length,
                                 UCHAR *signature,
                                 UINT signature_length);

UINT _nx_crypto_method_ecdsa_sce_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                      UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                      VOID  **handle,
                                      VOID  *crypto_metadata,
                                      ULONG crypto_metadata_size);

UINT _nx_crypto_method_ecdsa_sce_cleanup(VOID *crypto_metadata);

UINT _nx_crypto_method_ecdsa_sce_operation(UINT op,
                                           VOID *handle,
                                           struct NX_CRYPTO_METHOD_STRUCT *method,
                                           UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                           UCHAR *input, ULONG input_length_in_byte,
                                           UCHAR *iv_ptr,
                                           UCHAR *output, ULONG output_length_in_byte,
                                           VOID *crypto_metadata, ULONG crypto_metadata_size,
                                           VOID *packet_ptr,
                                           VOID (*nx_crypto_hw_process_callback)(VOID *, UINT));

#ifdef __cplusplus
}
#endif

#endif /* NX_CRYPTO_ECDSA_SCE_H */
