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
/**   HMAC SHA1 Digest Algorithm (SHA1)                                   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


#include "nx_crypto_sha2_sce.h"
#include "nx_crypto_hmac_sha2_sce.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_crypto_hmac_sha256_sce                          PORTABLE C      */
/*                                                           5.11         */
/*                                                                        */ 
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calculate the HMAC SHA256.                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    input_ptr                             input byte stream             */
/*    input_length                          input byte stream length      */
/*    key_ptr                               key stream                    */
/*    key_length                            key stream length             */
/*    digest_ptr                            generated SHA256 digest       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_crypto_sha256_sce_initialize      Initialize the SHA2 context   */
/*    _nx_crypto_sha256_sce_update          Update the digest with padding*/
/*                                            and length of digest        */ 
/*    _nx_crypto_sha256_sce_digest_calculate                              */
/*                                          Calculate the SHA2 digest     */ 
/*                                                                        */ 
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_hmac_sha256_sce_operation                         */
/*                                          Handle SHA2 operation         */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */ 
/**************************************************************************/
UINT _nx_crypto_hmac_sha256_sce(UCHAR *input_ptr, UINT input_length,
                                UCHAR *key_ptr, UINT key_length,
                                UCHAR *digest_ptr)
{
NX_SHA256_SCE   context;
UCHAR       k_ipad[64];
UCHAR       k_opad[64];
UCHAR       temp_key[32];
int         i;
        
    /* If key is longer than 64 bytes, reset it to key=SHA(key). */
    if (key_length > 64) 
    {
        _nx_crypto_sha256_sce_initialize(&context);

        _nx_crypto_sha256_sce_update(&context, key_ptr, key_length);

        _nx_crypto_sha256_sce_digest_calculate(&context, temp_key);

        key_ptr = temp_key;

        key_length = 32;

    }


    /* The HMAC_SHA256 transform looks like:
     
       SHA256(K XOR opad, SHA256(K XOR ipad, text))

       where K is an n byte key,
       ipad is the byte 0x36 repeated 64 times,
       opad is the byte 0x5c repeated 64 times,
       and text is the data being protected.      */

    memset(k_ipad, 0, sizeof(k_ipad));

    memset(k_opad, 0, sizeof(k_opad));

    memcpy(k_ipad, key_ptr, key_length);

    memcpy(k_opad, key_ptr, key_length);


    /* XOR key with ipad and opad values. */
    for (i = 0; i < 64; i++) 
    {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5c;
    }
        
    /* Perform inner SHA256. */
 
    _nx_crypto_sha256_sce_initialize(&context);
          
    _nx_crypto_sha256_sce_update(&context, k_ipad, 64);
    _nx_crypto_sha256_sce_update(&context, input_ptr, input_length);

    _nx_crypto_sha256_sce_digest_calculate(&context, digest_ptr);

    /* Perform inner SHA256. */

    _nx_crypto_sha256_sce_initialize(&context);

    _nx_crypto_sha256_sce_update(&context, k_opad, 64);
                                          
    _nx_crypto_sha256_sce_update(&context, digest_ptr, 32);
     
    _nx_crypto_sha256_sce_digest_calculate(&context, digest_ptr);

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
     
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hmac_sha256_sce_initialize               PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs SHA256 HMAC initialization.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               pointer to HMAC metadata      */
/*    key_ptr                               key stream                    */
/*    key_length                            key stream length             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_sha256_sce_initialize      Initialize the SHA2 context   */
/*    _nx_crypto_sha256_sce_update          Update the digest with padding*/
/*                                            and length of digest        */ 
/*    _nx_crypto_sha256_sce_digest_calculate                              */
/*                                          Calculate the SHA2 digest     */ 
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_hmac_sha256_sce_operation                         */
/*                                          Handle SHA2 operation         */ 
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
UINT _nx_crypto_hmac_sha256_sce_initialize(NX_SHA256_HMAC_SCE *context, UCHAR *key_ptr, UINT key_length)
{
UCHAR       temp_key[32];
int         i;

    /* If key is longer than 64 bytes, reset it to key=SHA256(key). */
    if (key_length > 64)
    {

        _nx_crypto_sha256_sce_initialize(&context->nx_sha256_hmac_context);

        _nx_crypto_sha256_sce_update(&context->nx_sha256_hmac_context, key_ptr, key_length);

        _nx_crypto_sha256_sce_digest_calculate(&context->nx_sha256_hmac_context, temp_key);

        key_ptr = temp_key;

        key_length = 32;

    }

    _nx_crypto_sha256_sce_initialize(&context->nx_sha256_hmac_context);

    /* The HMAC_SHA256 transform looks like:

       SHA256(K XOR opad, SHA256(K XOR ipad, text))

       where K is an n byte key,
       ipad is the byte 0x36 repeated 64 times,
       opad is the byte 0x5c repeated 64 times,
       and text is the data being protected.      */

    memset(context->nx_sha256_hmac_k_ipad, 0, sizeof(context->nx_sha256_hmac_k_ipad));

    memset(context->nx_sha256_hmac_k_opad, 0, sizeof(context->nx_sha256_hmac_k_opad));

    memcpy(context->nx_sha256_hmac_k_ipad, key_ptr, key_length);

    memcpy(context->nx_sha256_hmac_k_opad, key_ptr, key_length);


    /* XOR key with ipad and opad values. */
    for (i = 0; i < 64; i++)
    {
        context->nx_sha256_hmac_k_ipad[i] ^= 0x36;
        context->nx_sha256_hmac_k_opad[i] ^= 0x5c;
    }

    /* Kick off the inner hash with our padded key. */
    _nx_crypto_sha256_sce_update(&context->nx_sha256_hmac_context, context->nx_sha256_hmac_k_ipad, 64);
    
    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hmac_sha256_sce_update                   PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs SHA256 HMAC update.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               pointer to HMAC metadata      */
/*    key_ptr                               key stream                    */
/*    key_length                            key stream length             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_sha256_sce_update          Update the digest with padding*/
/*                                            and length of digest        */ 
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_hmac_sha256_sce_operation                         */
/*                                          Handle SHA2 operation         */ 
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
UINT _nx_crypto_hmac_sha256_sce_update(NX_SHA256_HMAC_SCE *context, UCHAR *input_ptr, UINT input_length)
{
    /* Update inner SHA256. */
    _nx_crypto_sha256_sce_update(&context->nx_sha256_hmac_context, input_ptr, input_length);

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_hmac_sha256_sce_digest_calculate         PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs SHA256 HMAC digest calculation.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               pointer to HMAC metadata      */
/*    digest_ptr                            generated crypto digest       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_sha256_sce_initialize      Initialize the SHA2 context   */
/*    _nx_crypto_sha256_sce_update          Update the digest with padding*/
/*                                            and length of digest        */ 
/*    _nx_crypto_sha256_sce_digest_calculate                              */
/*                                          Calculate the SHA2 digest     */ 
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_hmac_sha256_sce_operation                         */
/*                                          Handle SHA2 operation         */ 
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
UINT _nx_crypto_hmac_sha256_sce_digest_calculate(NX_SHA256_HMAC_SCE *context, UCHAR *digest_ptr)
{
    /* Perform outer SHA256. */

    _nx_crypto_sha256_sce_digest_calculate(&context->nx_sha256_hmac_context, digest_ptr);

    _nx_crypto_sha256_sce_initialize(&context->nx_sha256_hmac_context);

    _nx_crypto_sha256_sce_update(&context->nx_sha256_hmac_context, context->nx_sha256_hmac_k_opad, 64);

    _nx_crypto_sha256_sce_update(&context->nx_sha256_hmac_context, digest_ptr, 32);

    _nx_crypto_sha256_sce_digest_calculate(&context->nx_sha256_hmac_context, digest_ptr);

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);

}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_hmac_sha256_sce_operation         PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles HMAC SHA256 Authentication operation.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    Operation Type                */
/*                                          Encrypt, Decrypt, Authenticate*/
/*    handler                               Pointer to crypto context     */
/*    key                                   Pointer to key                */
/*    key_size_in_bits                      Length of key size in bits    */
/*    input                                 Input Stream                  */
/*    input_length_in_byte                  Input Stream Length           */
/*    iv_ptr                                Initialized Vector            */
/*    output                                Output Stream                 */
/*    output_length_in_byte                 Output Stream Length          */
/*    packet_ptr                            Pointer to packet             */
/*    nx_crypto_hw_process_callback         Callback function pointer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_sha256_sce_initialize      Initialize the SHA2 context   */
/*    _nx_crypto_sha256_sce_update          Update the digest with padding*/
/*                                            and length of digest        */ 
/*    _nx_crypto_sha256_sce_digest_calculate                              */
/*                                          Calculate the SHA2 digest     */ 
/*    _nx_crypto_hmac_sha256_sce                                          */
/*                                          Calculate the SHA2 digest     */ 
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
UINT _nx_crypto_method_hmac_sha256_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
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
NX_SHA256_HMAC_SCE *ctx;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    ctx = (NX_SHA256_HMAC_SCE*)crypto_metadata;

    switch(op)
    {
    case NX_CRYPTO_HASH_INITIALIZE:
        _nx_crypto_hmac_sha256_sce_initialize(ctx, key, key_size_in_bits >> 3);
        break;

    case NX_CRYPTO_HASH_UPDATE:
        _nx_crypto_hmac_sha256_sce_update(ctx, input, input_length_in_byte);
        break;

    case NX_CRYPTO_HASH_CALCULATE:
        _nx_crypto_hmac_sha256_sce_digest_calculate(ctx, output);
        break;

    default:
        _nx_crypto_hmac_sha256_sce(input, input_length_in_byte, key, (key_size_in_bits >> 3), output);
        break;
    }

    return NX_CRYPTO_SUCCESS;

}
