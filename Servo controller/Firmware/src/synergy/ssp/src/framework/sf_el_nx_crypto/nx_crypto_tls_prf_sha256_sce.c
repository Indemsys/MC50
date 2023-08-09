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
#define NX_SECURE_SOURCE_CODE 

#include "nx_crypto_tls_prf_sha256_sce.h"
#include "nx_crypto_phash_sce.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_tls_prf_sha256_sce                       PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function implements the TLS Pseudo-Random Function (PRF) that  */
/*    TLS uses in key generation and hashing. This version is for TLSv1.2 */
/*    and uses SHA-256. The PRF generates an arbitrary amount of pseudo-  */
/*    random data given a secret key, a label (used as part of the seed)  */
/*    and a random seed value.                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    prf                                   PRF metadata                  */
/*    secret                                Key used to generate data     */
/*    secret_len                            Length of secret key          */
/*    label                                 Label uses as part of seed    */
/*    label_len                             Length of label               */
/*    seed                                  Random seed value             */
/*    seed_len                              Length of random seed         */
/*    output                                Output buffer                 */
/*    desired_length                        Desired number of bytes       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_phash_sha_256_sce          Perform the TLS PHASH         */
/*                                            using SHA-256               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_prf_sha_256_sce_operation                         */
/*                                          Handle PRF operation          */
/*                                            using SHA-256               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
UINT _nx_crypto_tls_prf_sha256_sce(NX_SECURE_TLS_PRF_SHA_256_SCE *prf, UCHAR *secret, UINT secret_len, UCHAR *label, UINT label_len, UCHAR *seed, UINT seed_len, UCHAR *output, UINT desired_length)
{
    /* From RFC 5246 (TLS 1.2):

        TLS's PRF is created by applying P_hash to the secret as:

            PRF(secret, label, seed) = P_<hash>(secret, label + seed)

        The label is an ASCII string.  It should be included in the exact
        form it is given without a length byte or trailing null character.
        For example, the label "slithy toves" would be processed by hashing
        the following bytes:

            73 6C 69 74 68 79 20 74 6F 76 65 73

    */

UINT seed_label_len;


    /* Concatenate label and seed. */
    memcpy(prf->nx_secure_tls_prf_seed_label_buf, label, label_len);
    memcpy(&prf->nx_secure_tls_prf_seed_label_buf[label_len], seed, seed_len);
    seed_label_len = label_len + seed_len;
    
    /* Now run PHASH on the secret, using SHA-256-HMAC. */
    _nx_crypto_phash_sha_256_sce(&prf->nx_secure_tls_phash_sha_256, secret, secret_len, prf->nx_secure_tls_prf_seed_label_buf, seed_label_len, output, desired_length);
  
    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_prf_sha_256_sce_init              PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the PRF crypto module with SHA-1.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    method                                Crypto Method Object          */
/*    key                                   Key                           */
/*    key_size_in_bits                      Size of the key, in bits      */
/*    handle                                Handle, specified by user     */
/*    crypto_metadata                       Metadata area                 */
/*    crypto_metadata_size                  Size of the metadata area     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
UINT  _nx_crypto_method_prf_sha_256_sce_init(struct NX_CRYPTO_METHOD_STRUCT* method,
                                 UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                 VOID **handle,
                                 VOID *crypto_metadata,
                                 ULONG crypto_metadata_size)
{
NX_SECURE_TLS_PRF_SHA_256_SCE* prf;

    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);

    /* Get our control block. */
    prf = (NX_SECURE_TLS_PRF_SHA_256_SCE*)crypto_metadata;

    /* Set the secret using the key value. */
    prf->nx_secure_tls_prf_secret = key;
    prf->nx_secure_tls_prf_secret_length = key_size_in_bits;


    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_prf_sha_256_sce_operation         PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the PRF SHA-1 algorithm.                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    PRF operation                 */
/*    handle                                Crypto handle                 */
/*    method                                Cryption Method Object        */
/*    key                                   Encryption Key                */
/*    key_size_in_bits                      Key size in bits              */
/*    input                                 Input data                    */
/*    input_length_in_byte                  Input data size               */
/*    iv_ptr                                Initial vector                */
/*    output                                Output buffer                 */
/*    output_length_in_byte                 Output buffer size            */
/*    crypto_metadata                       Metadata area                 */
/*    crypto_metadata_size                  Metadata area size            */
/*    packet_ptr                            Pointer to packet             */
/*    nx_crypto_hw_process_callback         Callback function pointer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_tls_prf_sha256_sce         Implement the TLS PRF         */
/*                                            using SHA-256               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
UINT  _nx_crypto_method_prf_sha_256_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
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
                                        VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status))
{
UINT status;
NX_SECURE_TLS_PRF_SHA_256_SCE* prf;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    /* Get our control block. */
    prf = (NX_SECURE_TLS_PRF_SHA_256_SCE*)crypto_metadata;

    /* This must be a PRF operation. */
    if(op != NX_CRYPTO_PRF)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    /* PRF secret - set by init method
       PRF label - key
       PRF seed - input*/
    status = _nx_crypto_tls_prf_sha256_sce(prf, prf->nx_secure_tls_prf_secret, prf->nx_secure_tls_prf_secret_length,
                                       key, key_size_in_bits, input, input_length_in_byte, output, output_length_in_byte);

    return status;

}

