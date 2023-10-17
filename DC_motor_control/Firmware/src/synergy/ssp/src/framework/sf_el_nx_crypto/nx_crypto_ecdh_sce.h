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
/**   Elliptic-curve Diffie-Hellman (ECDH)                                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nx_crypto_ecdh_sce.h                                PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    NetX Crypto ECDH module.                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  07-15-2018     Timothy Stapko           Initial Version 5.11 SP1      */
/*                                                                        */
/**************************************************************************/

#ifndef NX_CRYPTO_ECDH_SCE_H
#define NX_CRYPTO_ECDH_SCE_H

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

#include "nx_crypto_ec_sce.h"
#include "r_rsa_api.h"
#include "common_data.h"
#include "r_ecc_api.h"


#ifndef NX_CRYPTO_ECDH_SCE_MAX_KEY_SIZE
#define NX_CRYPTO_ECDH_SCE_MAX_KEY_SIZE     (32)
#endif /* NX_CRYPTO_ECDH_SCE_MAX_KEY_SIZE */



typedef struct NX_CRYPTO_ECDH_SCE_STRUCT
{
    const NX_CRYPTO_EC_SCE *nx_crypto_ecdh_curve;

    ULONG            nx_crypto_ecdh_private_key[NX_CRYPTO_ECDH_SCE_MAX_KEY_SIZE >> 2];
    ULONG            nx_crypto_ecdh_public_key[NX_CRYPTO_ECDH_SCE_MAX_KEY_SIZE >> 1];
    ULONG            nx_crypto_ecdh_shared_key[NX_CRYPTO_ECDH_SCE_MAX_KEY_SIZE >> 1];
    UINT             nx_crypto_ecdh_key_size;
} NX_CRYPTO_ECDH_SCE;


UINT _nx_crypto_ecdh_key_pair_import_sce(NX_CRYPTO_ECDH_SCE  *ecdh_ptr,
                                         UCHAR  *local_private_key_ptr,
                                         ULONG   local_private_key_len,
                                         UCHAR  *local_public_key_ptr,
                                         ULONG   local_public_key_len);
UINT _nx_crypto_ecdh_private_key_export_sce(NX_CRYPTO_ECDH_SCE  *ecdh_ptr,
                                            UCHAR  *local_private_key_ptr,
                                            ULONG   local_private_key_len,
                                            ULONG  *actual_local_private_key_len);
UINT _nx_crypto_ecdh_setup_sce(NX_CRYPTO_ECDH_SCE *ecdh_ptr,
                               UCHAR *local_public_key_ptr,
                               ULONG local_public_key_len,
                               ULONG *actual_local_public_key_len);

UINT _nx_crypto_ecdh_compute_secret_sce(NX_CRYPTO_ECDH_SCE *ecdh_ptr,
                                        UCHAR *share_secret_key_ptr,
                                        ULONG share_secret_key_len,
                                        ULONG *actual_share_secret_key_len,
                                        UCHAR *remote_public_key,
                                        ULONG remote_public_key_len);

UINT  _nx_crypto_method_ecdh_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
                                           VOID *handle, /* Crypto handler */
                                           struct NX_CRYPTO_METHOD_STRUCT* method,
                                           UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                           UCHAR* input, ULONG input_length_in_byte,
                                           UCHAR* iv_ptr,
                                           UCHAR* output, ULONG output_length_in_byte,
                                           VOID *crypto_metadata, ULONG crypto_metadata_size,
                                           VOID *packet_ptr,
                                           VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status));


#ifdef __cplusplus
}
#endif

#endif /* NX_CRYPTO_ECDH_SCE_H */
