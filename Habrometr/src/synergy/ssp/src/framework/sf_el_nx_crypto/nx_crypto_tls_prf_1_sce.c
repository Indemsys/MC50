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


#include "nx_crypto_tls_prf_1_sce.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_tls_prf_1_sce                            PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function implements the TLS Pseudo-Random Function (PRF) that  */
/*    TLS uses in key generation and hashing. This version is for TLSv1.1 */
/*    and uses SHA-1 in combination with MD5. The PRF generates an        */
/*    arbitrary amount of pseudo-random data given a secret key, a label  */
/*    (used as part of the seed) and a random seed value.                 */
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
/*    _nx_crypto_phash_md5                  Perform the TLS PHASH         */
/*                                            using MD5                   */
/*    _nx_crypto_phash_sha_1                Perform the TLS PHASH         */
/*                                            using SHA-1                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_prf_1_sce_operation Handle PRF operation          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
UINT _nx_crypto_tls_prf_1_sce(NX_SECURE_TLS_PRF_1_SCE *prf, UCHAR *secret, UINT secret_len, UCHAR *label, UINT label_len, UCHAR *seed, UINT seed_len, UCHAR *output, UINT desired_length)
{
  /* From the RFC (TLS 1.1):

   TLS's PRF is created by splitting the secret into two halves and
   using one half to generate data with P_MD5 and the other half to
   generate data with P_SHA-1, then exclusive-ORing the outputs of these
   two expansion functions together.

   S1 and S2 are the two halves of the secret, and each is the same
   length.  S1 is taken from the first half of the secret, S2 from the
   second half.  Their length is created by rounding up the length of
   the overall secret, divided by two; thus, if the original secret is
   an odd number of bytes long, the last byte of S1 will be the same as
   the first byte of S2.
  
  
       L_S = length in bytes of secret;
       L_S1 = L_S2 = ceil(L_S / 2);
  
   The secret is partitioned into two halves (with the possibility of
   one shared byte) as described above, S1 taking the first L_S1 bytes,
   and S2 the last L_S2 bytes.

   The PRF is then defined as the result of mixing the two pseudorandom
   streams by exclusive-ORing them together.
  
       PRF(secret, label, seed) = P_MD5(S1, label + seed) XOR
                                  P_SHA-1(S2, label + seed);
  
   The label is an ASCII string.  It should be included in the exact
   form it is given without a length byte or trailing null character.
   For example, the label "slithy toves" would be processed by hashing
   the following bytes:

       73 6C 69 74 68 79 20 74 6F 76 65 73

   Note that because MD5 produces 16-byte outputs and SHA-1 produces
   20-byte outputs, the boundaries of their internal iterations will not
   be aligned.  Generating an 80-byte output will require that P_MD5
   iterate through A(5), while P_SHA-1 will only iterate through A(4).
  
  
  */

UINT L_S1, L_S2;
UCHAR *secret_half_point;
UINT seed_label_len;
UINT i;

    /* Calculate secret half-length. */
    L_S1 = L_S2 = (secret_len + 1) / 2;
    
    /* Find the offset of the half-way point. If the size of the secret is not even, overlap
       the last byte of the first half and first byte of the second half. */
    if((secret_len % 2) == 0)
    {
        /* Evenly divisible, offset is just after the half-length. */
        secret_half_point = &secret[L_S1];
    }
    else
    {
        /* Length is odd - overlap a byte. */
        secret_half_point = &secret[L_S1 - 1];
    }
    
    /* Concatenate label and seed. */
    memcpy(prf->nx_secure_tls_prf_seed_label_buf, label, label_len);
    memcpy(&prf->nx_secure_tls_prf_seed_label_buf[label_len], seed, seed_len);
    seed_label_len = label_len + seed_len;
    
    /* Now run PHASH on each half, first PHASH_MD5. */
    _nx_crypto_phash_md5(&prf->nx_secure_tls_phash_md5, secret, L_S1, prf->nx_secure_tls_prf_seed_label_buf, seed_label_len, prf->nx_secure_tls_prf_md5_buf, desired_length);

    /* ... and now PHASH_SHA1. */
    _nx_crypto_phash_sha_1_sce(&prf->nx_secure_tls_phash_sha_1, secret_half_point, L_S2, prf->nx_secure_tls_prf_seed_label_buf, seed_label_len, prf->nx_secure_tls_prf_sha1_buf, desired_length);
    
    /* Finally, XOR the results and place in the output buffer. */
    for(i = 0; i < desired_length; ++i)
    {
        output[i] = prf->nx_secure_tls_prf_md5_buf[i] ^ prf->nx_secure_tls_prf_sha1_buf[i];
    }
  
    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_prf_1_sce_init                    PORTABLE C      */
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
UINT  _nx_crypto_method_prf_1_sce_init(struct NX_CRYPTO_METHOD_STRUCT* method,
                                       UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                       VOID **handle,
                                       VOID *crypto_metadata,
                                       ULONG crypto_metadata_size)
{
NX_SECURE_TLS_PRF_1_SCE* prf;

    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);

    /* Get our control block. */
    prf = (NX_SECURE_TLS_PRF_1_SCE*)crypto_metadata;

    /* Set the secret using the key value. */
    prf->nx_secure_tls_prf_secret = key;
    prf->nx_secure_tls_prf_secret_length = (USHORT)(key_size_in_bits << 3);

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_prf_1_sce_operation               PORTABLE C      */
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
/*    _nx_crypto_tls_prf_1_sce              Implement the TLS PRF         */
/*                                            using SHA-1                 */
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
UINT  _nx_crypto_method_prf_1_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
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
NX_SECURE_TLS_PRF_1_SCE* prf;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    /* Get our control block. */
    prf = (NX_SECURE_TLS_PRF_1_SCE*)crypto_metadata;

    /* This must be a PRF operation. */
    if(op != NX_CRYPTO_PRF)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    /* PRF secret - set by init method
       PRF label - key
       PRF seed - input*/
    status = _nx_crypto_tls_prf_1_sce(prf, prf->nx_secure_tls_prf_secret, prf->nx_secure_tls_prf_secret_length >> 3,
                                        key, key_size_in_bits, input, input_length_in_byte, output, output_length_in_byte);

    return status;

}
