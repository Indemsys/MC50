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
/**     Transport Layer Security (TLS)                                    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_secure_tls.h"
#include "nx_crypto_aes_sce.h"
#include "nx_crypto_sha1_sce.h"
#include "nx_crypto_hmac_sha1_sce.h"
#include "nx_crypto_sha2_sce.h"
#include "nx_crypto_hmac_sha2_sce.h"
#include "nx_crypto_md5_sce.h"
#include "nx_crypto_hmac_md5_sce.h"
#include "nx_crypto_phash_sce.h"
#include "nx_crypto_tls_prf_1_sce.h"
#include "nx_crypto_tls_prf_sha256_sce.h"
#include "nx_crypto_rsa_sce.h"
#include "nx_crypto_ec_sce.h"
#include "nx_crypto_ecdh_sce.h"
#include "nx_crypto_ecdsa_sce.h"
#include "nx_crypto_sha5.h"
#include "nx_crypto_hkdf.h"
#include "nx_crypto_hmac.h"

/* TLS1.3 source code in Azure stack uses the software crypto methods directly by
 * the implementation, whereas, SSP uses the Synergy HW and associated crypto methods.
 * To retain the compatibility below definition is made. */
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
#define crypto_method_ecdhe_sce     crypto_method_ecdhe
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_crypto_synergys7_ciphersuites                    PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*     This table of function pointers provides a mapping from TLS        */
/*     ciphersuites to the necessary cryptographic methods for a given    */
/*     platform. It can be used as a model to develop a hardware-specific */
/*     cryptography table for TLS.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*  07-15-2018     Timothy Stapko           Modified Comment(s), added    */
/*                                            ciphersuites for ECDSA,     */
/*                                            ECDHE, and AES-GCM mode,    */
/*                                            resulting in version 5.11SP1*/
/*                                                                        */
/**************************************************************************/

/* Define cryptographic methods for use with TLS. */


/* Following declarations for Crypto methods are taken from software crypto block,
 * this can be removed when software crpto block is supported */

/* Declare the MD5 hash method */
NX_CRYPTO_METHOD crypto_method_md5_sce =
{
    NX_CRYPTO_HASH_MD5,                       /* MD5 algorithm                         */
    0,                                        /* Key size in bits                      */
    0,                                        /* IV size in bits, not used             */
    NX_CRYPTO_HMAC_MD5_ICV_FULL_LEN_IN_BITS,  /* Transmitted ICV size in bits          */
    SF_EL_NX_CRYPTO_MD5_BLOCK_SIZE_BYTES,     /* Block size in bytes                   */
    sizeof(NX_MD5_SCE),                       /* Metadata size in bytes                */
    NX_NULL,                                  /* Initialization routine, not used      */
    NX_NULL,                                  /* Cleanup routine, not used             */
    _nx_crypto_method_md5_sce_operation       /* MD5 operation                         */
};

/* Declare the SHA384 hash method */
NX_CRYPTO_METHOD crypto_method_sha384 =
{
    NX_CRYPTO_HASH_SHA384,                         /* SHA384 algorithm                      */
    0,                                             /* Key size in bits                      */
    0,                                             /* IV size in bits, not used             */
    NX_CRYPTO_SHA384_ICV_LEN_IN_BITS,              /* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES,          /* Block size in bytes                   */
    sizeof(NX_CRYPTO_SHA512),                      /* Metadata size in bytes                */
    _nx_crypto_method_sha512_init,                 /* SHA384 initialization routine         */
    _nx_crypto_method_sha512_cleanup,              /* SHA384 cleanup routine                */
    _nx_crypto_method_sha512_operation             /* SHA384 operation                      */
};

/* Declare the SHA512 hash method */
NX_CRYPTO_METHOD crypto_method_sha512 =
{
    NX_CRYPTO_HASH_SHA512,                         /* SHA512 algorithm                      */
    0,                                             /* Key size in bits                      */
    0,                                             /* IV size in bits, not used             */
    NX_CRYPTO_SHA512_ICV_LEN_IN_BITS,              /* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES,          /* Block size in bytes                   */
    sizeof(NX_CRYPTO_SHA512),                      /* Metadata size in bytes                */
    _nx_crypto_method_sha512_init,                 /* SHA512 initialization routine         */
    _nx_crypto_method_sha512_cleanup,              /* SHA512 cleanup routine                */
    _nx_crypto_method_sha512_operation             /* SHA512 operation                      */
};

/* Declare the NULL encrypt */
NX_CRYPTO_METHOD crypto_method_null_sce =
{
    NX_CRYPTO_ENCRYPTION_NULL,                /* Name of the crypto algorithm          */
    0,                                        /* Key size in bits, not used            */
    0,                                        /* IV size in bits, not used             */
    0,                                        /* ICV size in bits, not used            */
    4,                                        /* Block size in bytes                   */
    0,                                        /* Metadata size in bytes                */
    NX_NULL,                                  /* Initialization routine, not used      */
    NX_NULL,                                  /* Cleanup routine, not used             */
    NX_NULL                                   /* NULL operation                        */
};

/* Declare the HMAC MD5 authentication method */
NX_CRYPTO_METHOD crypto_method_hmac_md5_sce =
{
    NX_CRYPTO_AUTHENTICATION_HMAC_MD5_128,    /* MD5 algorithm                         */
    NX_CRYPTO_HMAC_MD5_KEY_LEN_IN_BITS,       /* Key size in bits                      */
    0,                                        /* IV size in bits, not used             */
    NX_CRYPTO_HMAC_MD5_ICV_FULL_LEN_IN_BITS,  /* Transmitted ICV size in bits          */
    SF_EL_NX_CRYPTO_MD5_BLOCK_SIZE_BYTES,     /* Block size in bytes, not used         */
    sizeof(NX_MD5_HMAC_SCE),                  /* Metadata size in bytes                */
    NX_NULL,                                  /* Initialization routine, not used      */
    NX_NULL,                                  /* Cleanup routine, not used             */
    _nx_crypto_method_hmac_md5_sce_operation  /* HMAC MD5 operation                    */
};


/* Declare the AES-CBC 128 encrytion method. */
NX_CRYPTO_METHOD crypto_method_aes_cbc_128_sce =
{
    NX_CRYPTO_ENCRYPTION_AES_CBC,                /* AES crypto algorithm                  */
    NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,           /* Key size in bits                      */
    NX_CRYPTO_AES_IV_LEN_IN_BITS,                /* IV size in bits                       */
    0,                                           /* ICV size in bits, not used.           */
    (NX_CRYPTO_AES_BLOCK_SIZE_IN_BITS >> 3),     /* Block size in bytes.                  */
    sizeof(NX_CRYPTO_AES_SCE),                   /* Metadata size in bytes                */
    _nx_crypto_method_aes_sce_init,              /* AES-CBC initialization routine.       */
    NX_NULL,                                     /* AES-CBC cleanup routine, not used.    */
    _nx_crypto_method_aes_sce_operation          /* AES-CBC operation                     */
};

/* Declare the AES-CBC 256 encryption method */
NX_CRYPTO_METHOD crypto_method_aes_cbc_256_sce =
{
    NX_CRYPTO_ENCRYPTION_AES_CBC,                /* AES crypto algorithm                  */
    NX_CRYPTO_AES_256_KEY_LEN_IN_BITS,           /* Key size in bits                      */
    NX_CRYPTO_AES_IV_LEN_IN_BITS,                /* IV size in bits                       */
    0,                                           /* ICV size in bits, not used.           */
    (NX_CRYPTO_AES_BLOCK_SIZE_IN_BITS >> 3),     /* Block size in bytes.                  */
    sizeof(NX_CRYPTO_AES_SCE),                   /* Metadata size in bytes                */
    _nx_crypto_method_aes_sce_init,              /* AES-CBC initialization routine.       */
    NX_NULL,                                     /* AES-CBC cleanup routine, not used.    */
    _nx_crypto_method_aes_sce_operation          /* AES-CBC operation                     */
};
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
/* Declare the AES-GCM encrytion method. */
NX_CRYPTO_METHOD crypto_method_aes_128_gcm_16_sce =
{
    NX_CRYPTO_ENCRYPTION_AES_GCM_16,             /* AES crypto algorithm                   */
    NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,           /* Key size in bits                       */
    32,                                          /* IV size in bits                        */
    128,                                         /* ICV size in bits                       */
    (NX_CRYPTO_AES_BLOCK_SIZE_IN_BITS >> 3),     /* Block size in bytes.                   */
    sizeof(NX_CRYPTO_AES_SCE),                   /* Metadata size in bytes                 */
    _nx_crypto_method_aes_sce_init,              /* AES-GCM initialization routine.        */
    NX_NULL,                                     /* AES-GCM cleanup routine.               */
    _nx_crypto_method_aes_gcm_sce_operation,     /* AES-GCM operation                      */
};
#endif

/* Declare the AES-GCM encrytion method. */
NX_CRYPTO_METHOD crypto_method_aes_192_gcm_16_sce =
{
    NX_CRYPTO_ENCRYPTION_AES_GCM_16,             /* AES crypto algorithm                   */
    NX_CRYPTO_AES_192_KEY_LEN_IN_BITS,           /* Key size in bits                       */
    32,                                          /* IV size in bits                        */
    128,                                         /* ICV size in bits                       */
    (NX_CRYPTO_AES_BLOCK_SIZE_IN_BITS >> 3),     /* Block size in bytes.                   */
    sizeof(NX_CRYPTO_AES_SCE),                   /* Metadata size in bytes                 */
    _nx_crypto_method_aes_sce_init,              /* AES-GCM initialization routine.        */
    NX_NULL,                                     /* AES-GCM cleanup routine.               */
    _nx_crypto_method_aes_gcm_sce_operation,     /* AES-GCM operation                      */
};

/* Declare the AES-GCM encrytion method. */
NX_CRYPTO_METHOD crypto_method_aes_256_gcm_16_sce =
{
    NX_CRYPTO_ENCRYPTION_AES_GCM_16,             /* AES crypto algorithm                   */
    NX_CRYPTO_AES_256_KEY_LEN_IN_BITS,           /* Key size in bits                       */
    32,                                          /* IV size in bits                        */
    128,                                         /* ICV size in bits                       */
    (NX_CRYPTO_AES_BLOCK_SIZE_IN_BITS >> 3),     /* Block size in bytes.                   */
    sizeof(NX_CRYPTO_AES_SCE),                   /* Metadata size in bytes                 */
    _nx_crypto_method_aes_sce_init,              /* AES-GCM initialization routine.        */
    NX_NULL,                                     /* AES-GCM cleanup routine.               */
    _nx_crypto_method_aes_gcm_sce_operation,     /* AES-GCM operation                      */
};
#endif
/* Declare the HMAC SHA1 authentication method */
NX_CRYPTO_METHOD crypto_method_hmac_sha1_sce =
{
    NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_160,            /* HMAC SHA1 algorithm                   */
    0,                                                 /* Key size in bits                      */
    0,                                                 /* IV size in bits, not used             */
    NX_CRYPTO_HMAC_SHA1_ICV_FULL_LEN_IN_BITS,          /* Transmitted ICV size in bits          */
    64,                                                /* Block size in bytes, not used         */
    sizeof(NX_SHA1_HMAC_SCE),                          /* Metadata size in bytes                */
    NX_NULL,                                           /* Initialization routine, not used      */
    NX_NULL,                                           /* Cleanup routine, not used             */
    _nx_crypto_method_hmac_sha1_sce_operation          /* HMAC SHA1 operation                   */
};

/* Declare the HMAC SHA256 authentication method */
NX_CRYPTO_METHOD crypto_method_hmac_sha256_sce =
{
    NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_256,       /* HMAC SHA256 algorithm                 */
    0,                                            /* Key size in bits                      */
    0,                                            /* IV size in bits, not used             */
    NX_CRYPTO_HMAC_SHA256_ICV_FULL_LEN_IN_BITS,   /* Transmitted ICV size in bits          */
    64,                                           /* Block size in bytes, not used         */
    sizeof(NX_SHA256_HMAC_SCE),                   /* Metadata size in bytes                */
    NX_NULL,                                      /* Initialization routine, not used      */
    NX_NULL,                                      /* Cleanup routine, not used             */
    _nx_crypto_method_hmac_sha256_sce_operation   /* HMAC SHA256 operation                 */
};


/* Declare the RSA public cipher method. */
NX_CRYPTO_METHOD crypto_method_rsa_sce =
{
    NX_CRYPTO_KEY_EXCHANGE_RSA,               /* RSA crypto algorithm                   */
    0,                                            /* Key size in bits                      */
    0,                                            /* IV size in bits                       */
    0,                                            /* ICV size in bits, not used.           */
    0,                                            /* Block size in bytes.                  */
#if (NX_CRYPTO_MAX_RSA_MODULUS_SIZE == RSA_KEY_SIZE_4096_BITS)
    sizeof(NX_CRYPTO_RSA),                        /* Metadata size in bytes                */
#else
    sizeof(NX_CRYPTO_RSA_SCE),                    /* Metadata size in bytes                */
#endif
    _nx_crypto_method_rsa_sce_init,               /* RSA initialization routine.           */
    NX_NULL,                                      /* RSA cleanup routine, not used.        */
    _nx_crypto_method_rsa_sce_operation           /* RSA operation                         */
};

/* Declare a placeholder for PSK authentication. */
NX_CRYPTO_METHOD crypto_method_auth_psk_sce =
{
    NX_CRYPTO_KEY_EXCHANGE_PSK,               /* PSK placeholder                        */
    0,                                        /* Key size in bits                       */
    0,                                        /* IV size in bits                        */
    0,                                        /* ICV size in bits, not used.            */
    0,                                        /* Block size in bytes.                   */
    0,                                        /* Metadata size in bytes                 */
    NX_NULL,                                  /* Initialization routine.                */
    NX_NULL,                                  /* Cleanup routine, not used.             */
    NX_NULL                                   /* Operation                              */
};

/* Declare the SHA1 hash method */
NX_CRYPTO_METHOD crypto_method_sha1_sce =
{
    NX_CRYPTO_HASH_SHA1,                           /* SHA1 algorithm                        */
    0,                                             /* Key size in bits                      */
    0,                                             /* IV size in bits, not used             */
    160,                                           /* Transmitted ICV size in bits          */
    64,                                            /* Block size in bytes, not used         */
    sizeof(NX_SHA1_SCE),                           /* Metadata size in bytes                */
    NX_NULL,                                       /* Initialization routine, not used      */
    NX_NULL,                                       /* Cleanup routine, not used             */
    _nx_crypto_method_sha1_sce_operation           /* SHA1 operation                        */
};

/* Declare the SHA256 hash method */
NX_CRYPTO_METHOD crypto_method_sha256_sce =
{
    NX_CRYPTO_HASH_SHA256,                         /* SHA256 algorithm                      */
    0,                                             /* Key size in bits                      */
    0,                                             /* IV size in bits, not used             */
    NX_CRYPTO_SHA256_ICV_LEN_IN_BITS,              /* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA2_BLOCK_SIZE_IN_BYTES,            /* Block size in bytes, not used         */
    sizeof(NX_SHA256_SCE),                         /* Metadata size in bytes                */
    _nx_crypto_method_sha256_sce_init,                 /* Initialization routine, not used      */
    _nx_crypto_method_sha256_sce_cleanup,              /* Cleanup routine, not used             */
    _nx_crypto_method_sha256_sce_operation         /* SHA256 operation                      */
};

/* Declare the TLSv1.0/1.1 PRF hash method */
NX_CRYPTO_METHOD crypto_method_tls_prf_1_sce =
{
    NX_CRYPTO_PRF_HMAC_SHA1,                       /* TLS PRF algorithm                     */
    0,                                             /* Key size in bits, not used            */
    0,                                             /* IV size in bits, not used             */
    0,                                             /* Transmitted ICV size in bits, not used*/
    0,                                             /* Block size in bytes, not used         */
    sizeof(NX_SECURE_TLS_PRF_1_SCE),               /* Metadata size in bytes                */
    _nx_crypto_method_prf_1_sce_init,              /* Initialization routine to set secret  */
    NX_NULL,                                       /* Cleanup routine, not used             */
    _nx_crypto_method_prf_1_sce_operation          /* TLS PRF operation                     */
};

/* Declare the TLSv1.2 default PRF hash method */
NX_CRYPTO_METHOD crypto_method_tls_prf_sha_256_sce =
{
    NX_CRYPTO_PRF_HMAC_SHA2_256,                   /* TLS PRF algorithm                     */
    0,                                             /* Key size in bits, not used            */
    0,                                             /* IV size in bits, not used             */
    0,                                             /* Transmitted ICV size in bits, not used*/
    0,                                             /* Block size in bytes, not used         */
    sizeof(NX_SECURE_TLS_PRF_SHA_256_SCE),         /* Metadata size in bytes                */
    _nx_crypto_method_prf_sha_256_sce_init,        /* Initialization routine to set secret  */
    NX_NULL,                                       /* Cleanup routine, not used             */
    _nx_crypto_method_prf_sha_256_sce_operation    /* TLS PRF operation                     */
};

/* Declare the ECDHE crypto method */
NX_CRYPTO_METHOD crypto_method_ecdhe_sce =
{
    NX_CRYPTO_KEY_EXCHANGE_ECDHE,                /* ECDHE crypto algorithm                 */
    0,                                           /* Key size in bits                       */
    0,                                           /* IV size in bits                        */
    0,                                           /* ICV size in bits, not used             */
    0,                                           /* Block size in bytes                    */
    sizeof(NX_CRYPTO_ECDH_SCE),                  /* Metadata size in bytes                 */
    NX_NULL,                                     /* Initialization routine, not used       */
    NX_NULL,                                     /* Cleanup routine, not used              */
    _nx_crypto_method_ecdh_sce_operation,        /* ECDH operation                         */
};

/* Declare the ECDH crypto method */
NX_CRYPTO_METHOD crypto_method_ecdh_sce =
{
    NX_CRYPTO_KEY_EXCHANGE_ECDH,                 /* ECDH crypto algorithm                  */
    0,                                           /* Key size in bits                       */
    0,                                           /* IV size in bits                        */
    0,                                           /* ICV size in bits, not used             */
    0,                                           /* Block size in bytes                    */
    sizeof(NX_CRYPTO_ECDH_SCE),                  /* Metadata size in bytes                 */
    NX_NULL,                                     /* Initialization routine, not used       */
    NX_NULL,                                     /* Cleanup routine, not used              */
    _nx_crypto_method_ecdh_sce_operation,        /* ECDH operation                         */
};

/* Declare the ECDSA crypto method */
NX_CRYPTO_METHOD crypto_method_ecdsa_sce =
{
    NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA,           /* ECDSA crypto algorithm                 */
    0,                                           /* Key size in bits                       */
    0,                                           /* IV size in bits                        */
    0,                                           /* ICV size in bits, not used             */
    0,                                           /* Block size in bytes                    */
    sizeof(NX_CRYPTO_ECDSA_SCE),                 /* Metadata size in bytes                 */
    _nx_crypto_method_ecdsa_sce_init,            /* ECDSA initialization routine           */
    _nx_crypto_method_ecdsa_sce_cleanup,         /* ECDSA cleanup routine                  */
    _nx_crypto_method_ecdsa_sce_operation,       /* ECDSA operation                        */
};

/* Declare a placeholder for EC SECP192R1. */
NX_CRYPTO_METHOD crypto_method_ec_secp192_sce =
{
    NX_CRYPTO_EC_SECP192R1,                      /* EC placeholder                         */
    192,                                         /* Key size in bits                       */
    0,                                           /* IV size in bits                        */
    0,                                           /* ICV size in bits, not used.            */
    0,                                           /* Block size in bytes.                   */
    0,                                           /* Metadata size in bytes                 */
    NX_NULL,                                     /* Initialization routine.                */
    NX_NULL,                                     /* Cleanup routine, not used.             */
    _nx_crypto_method_ec_secp192r1_sce_operation,/* Operation                              */
};

/* Declare a placeholder for EC SECP256R1. */
NX_CRYPTO_METHOD crypto_method_ec_secp256_sce =
{
    NX_CRYPTO_EC_SECP256R1,                      /* EC placeholder                         */
    256,                                         /* Key size in bits                       */
    0,                                           /* IV size in bits                        */
    0,                                           /* ICV size in bits, not used.            */
    0,                                           /* Block size in bytes.                   */
    0,                                           /* Metadata size in bytes                 */
    NX_NULL,                                     /* Initialization routine.                */
    NX_NULL,                                     /* Cleanup routine, not used.             */
    _nx_crypto_method_ec_secp256r1_sce_operation,/* Operation                              */
};

/* Define generic HMAC cryptographic routine. */
NX_CRYPTO_METHOD crypto_method_hmac =
{
    NX_CRYPTO_HASH_HMAC,                            /* HMAC algorithm                        */
    0,                                              /* Key size in bits, not used            */
    0,                                              /* IV size in bits, not used             */
    0,                                              /* Transmitted ICV size in bits, not used*/
    0,                                              /* Block size in bytes, not used         */
    sizeof(NX_CRYPTO_HMAC),                         /* Metadata size in bytes                */
    _nx_crypto_method_hmac_init,                    /* HKDF initialization routine           */
    _nx_crypto_method_hmac_cleanup,                 /* HKDF cleanup routine                  */
    _nx_crypto_method_hmac_operation                /* HKDF operation                        */
};

/* Define generic HMAC-based Key Derivation Function method. */
NX_CRYPTO_METHOD crypto_method_hkdf =
{
    NX_CRYPTO_HKDF_METHOD,                          /* HKDF algorithm                        */
    0,                                              /* Key size in bits, not used            */
    0,                                              /* IV size in bits, not used             */
    0,                                              /* Transmitted ICV size in bits, not used*/
    0,                                              /* Block size in bytes, not used         */
    sizeof(NX_CRYPTO_HKDF) + sizeof(NX_CRYPTO_HMAC),/* Metadata size in bytes                */
    _nx_crypto_method_hkdf_init,                    /* HKDF initialization routine           */
    _nx_crypto_method_hkdf_cleanup,                 /* HKDF cleanup routine                  */
    _nx_crypto_method_hkdf_operation                /* HKDF operation                        */
};

/* Lookup table used to map ciphersuites to cryptographic routines. */
NX_SECURE_TLS_CIPHERSUITE_INFO _nx_crypto_ciphersuite_lookup_table_synergys7[] =
{
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    /* Ciphersuite,                           public cipher,            public_auth,              session cipher & cipher mode,      iv size, key size,  hash method,                    hash size, TLS PRF */
    {TLS_AES_128_GCM_SHA256,                  &crypto_method_ecdhe_sce, &crypto_method_ecdsa_sce, &crypto_method_aes_128_gcm_16_sce, 96,      16,        &crypto_method_sha256_sce,      32,        &crypto_method_hkdf},
#endif

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
    {TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256, &crypto_method_ecdhe_sce, &crypto_method_ecdsa_sce, &crypto_method_aes_128_gcm_16_sce, 16,      16,        &crypto_method_null_sce,        0,         &crypto_method_tls_prf_sha_256_sce},
    {TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,   &crypto_method_ecdhe_sce, &crypto_method_rsa_sce,   &crypto_method_aes_128_gcm_16_sce, 16,      16,        &crypto_method_null_sce,        0,         &crypto_method_tls_prf_sha_256_sce},
#endif
    {TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256, &crypto_method_ecdhe_sce, &crypto_method_ecdsa_sce, &crypto_method_aes_cbc_128_sce,    16,      16,        &crypto_method_hmac_sha256_sce, 32,        &crypto_method_tls_prf_sha_256_sce},
    {TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256,   &crypto_method_ecdhe_sce, &crypto_method_rsa_sce,   &crypto_method_aes_cbc_128_sce,    16,      16,        &crypto_method_hmac_sha256_sce, 32,        &crypto_method_tls_prf_sha_256_sce},
#endif

    {TLS_RSA_WITH_AES_256_CBC_SHA,            &crypto_method_rsa_sce,   &crypto_method_rsa_sce,   &crypto_method_aes_cbc_256_sce,    16,      32,        &crypto_method_hmac_sha1_sce,   20,        &crypto_method_tls_prf_sha_256_sce},
    {TLS_RSA_WITH_AES_128_CBC_SHA,            &crypto_method_rsa_sce,   &crypto_method_rsa_sce,   &crypto_method_aes_cbc_128_sce,    16,      16,        &crypto_method_hmac_sha1_sce,   20,        &crypto_method_tls_prf_sha_256_sce},

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
    {TLS_PSK_WITH_AES_128_CBC_SHA,            &crypto_method_null_sce,  &crypto_method_auth_psk_sce,  &crypto_method_aes_cbc_128_sce, 16,      16,       &crypto_method_hmac_sha1_sce,   20,        &crypto_method_tls_prf_sha_256_sce},
    {TLS_PSK_WITH_AES_256_CBC_SHA,            &crypto_method_null_sce,  &crypto_method_auth_psk_sce,  &crypto_method_aes_cbc_256_sce, 16,      32,       &crypto_method_hmac_sha1_sce,   20,        &crypto_method_tls_prf_sha_256_sce},
 // {TLS_PSK_WITH_AES_128_CBC_SHA256,         &crypto_method_null_sce,  &crypto_method_auth_psk_sce,  &crypto_method_aes_cbc_128_sce, 16,      16,       &crypto_method_hmac_sha256_sce, 32,        &crypto_method_tls_prf_sha_256_sce},
 // {TLS_PSK_WITH_AES_128_CCM_8,              &crypto_method_null_sce,  &crypto_method_auth_psk_sce,  &crypto_method_aes_ccm_8,       16,      16,       &crypto_method_null_sce,        0,         &crypto_method_tls_prf_sha_256_sce},
#endif /* NX_SECURE_ENABLE_PSK_CIPHERSUITES */

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
    {TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256,  &crypto_method_ecdh_sce,  &crypto_method_ecdsa_sce, &crypto_method_aes_128_gcm_16_sce, 16,      16,        &crypto_method_null_sce,        0,         &crypto_method_tls_prf_sha_256_sce},
    {TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256,    &crypto_method_ecdh_sce,  &crypto_method_rsa_sce,   &crypto_method_aes_128_gcm_16_sce, 16,      16,        &crypto_method_null_sce,        0,         &crypto_method_tls_prf_sha_256_sce},
#endif
    {TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256,  &crypto_method_ecdh_sce,  &crypto_method_ecdsa_sce, &crypto_method_aes_cbc_128_sce,    16,      16,        &crypto_method_hmac_sha256_sce, 32,        &crypto_method_tls_prf_sha_256_sce},
    {TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256,    &crypto_method_ecdh_sce,  &crypto_method_rsa_sce,   &crypto_method_aes_cbc_128_sce,    16,      16,        &crypto_method_hmac_sha256_sce, 32,        &crypto_method_tls_prf_sha_256_sce},
#endif
    {TLS_RSA_WITH_NULL_SHA,                   &crypto_method_rsa_sce,   &crypto_method_rsa_sce,   &crypto_method_null_sce,           0,       0,         &crypto_method_hmac_sha1_sce,   20,        &crypto_method_tls_prf_sha_256_sce},
    {TLS_RSA_WITH_NULL_MD5,                   &crypto_method_rsa_sce,   &crypto_method_rsa_sce,   &crypto_method_null_sce,           0,       0,         &crypto_method_hmac_md5_sce,    16,        &crypto_method_tls_prf_sha_256_sce},

#if 0
    {TLS_NULL_WITH_NULL_NULL,                 &crypto_method_null_sce,  &crypto_method_null_sce,  &crypto_method_null_sce,           0,       0,         &crypto_method_null_sce,      0,         &crypto_method_tls_prf_sha_256_sce},
#endif
};

const UINT _nx_crypto_ciphersuite_lookup_table_synergys7_size = sizeof(_nx_crypto_ciphersuite_lookup_table_synergys7) / sizeof(NX_SECURE_TLS_CIPHERSUITE_INFO);

/* Lookup table for X.509 digital certificates - they need a public-key algorithm and a hash routine for verification. */
NX_SECURE_X509_CRYPTO _nx_crypto_x509_cipher_lookup_table_synergys7[] =
{
    /* OID identifier,                        public cipher,            hash method */
    {NX_SECURE_TLS_X509_TYPE_RSA_MD5,        &crypto_method_rsa_sce,       &crypto_method_md5_sce},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_1,      &crypto_method_rsa_sce,       &crypto_method_sha1_sce},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_256,    &crypto_method_rsa_sce,       &crypto_method_sha256_sce},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_384,    &crypto_method_rsa_sce,       &crypto_method_sha384},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_512,    &crypto_method_rsa_sce,       &crypto_method_sha512},
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_1,    &crypto_method_ecdsa_sce,     &crypto_method_sha1_sce},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_256,  &crypto_method_ecdsa_sce,     &crypto_method_sha256_sce},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_384,  &crypto_method_ecdsa_sce,     &crypto_method_sha384},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_512,  &crypto_method_ecdsa_sce,     &crypto_method_sha512}
#endif
};

const UINT _nx_crypto_x509_cipher_lookup_table_synergys7_size = sizeof(_nx_crypto_x509_cipher_lookup_table_synergys7) / sizeof(NX_SECURE_X509_CRYPTO);

/* Define the object we can pass into TLS. */
const NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers_synergys7 =
{
    /* Ciphersuite lookup table and size. */
    _nx_crypto_ciphersuite_lookup_table_synergys7,
    sizeof(_nx_crypto_ciphersuite_lookup_table_synergys7) / sizeof(NX_SECURE_TLS_CIPHERSUITE_INFO),

#ifndef NX_SECURE_DISABLE_X509
    /* X.509 certificate cipher table and size. */
    _nx_crypto_x509_cipher_lookup_table_synergys7,
    sizeof(_nx_crypto_x509_cipher_lookup_table_synergys7) / sizeof(NX_SECURE_X509_CRYPTO),
#endif

    /* TLS version-specific methods. */
#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    &crypto_method_md5_sce,
    &crypto_method_sha1_sce,
    &crypto_method_tls_prf_1_sce,
#endif

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    &crypto_method_sha256_sce,
    &crypto_method_tls_prf_sha_256_sce,
#endif

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    &crypto_method_hkdf,
    &crypto_method_hmac,
    &crypto_method_ecdhe_sce
#endif
};

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
const USHORT nx_crypto_ecc_supported_groups_synergys7[] =
{
    (USHORT)NX_CRYPTO_EC_SECP256R1,
    (USHORT)NX_CRYPTO_EC_SECP192R1,
};

const NX_CRYPTO_METHOD *nx_crypto_ecc_curves_synergys7[] =
{
    &crypto_method_ec_secp256_sce,
    &crypto_method_ec_secp192_sce,
};

const USHORT nx_crypto_ecc_supported_groups_synergys7_size = sizeof(nx_crypto_ecc_supported_groups_synergys7) / sizeof(USHORT);
#endif

