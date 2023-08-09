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
/**     SHA-256 Digest Algorithm (SHA2)                                   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_sha2_sce.h"





/* Define the padding array.  This is used to pad the message such that its length is
   64 bits shy of being a multiple of 512 bits long.  */
static UCHAR   _nx_crypto_sha256_padding[64] = {0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_sha256_sce_initialize                    PORTABLE C      */ 
/*                                                           5.11         */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the SHA256 context. It must be called     */
/*    prior to creating a SHA256 digest.                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA256 context pointer        */
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
/*    _nx_crypto_method_sha256_sce_operation                              */
/*                                          Handle SHA2 operation         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
UINT  _nx_crypto_sha256_sce_initialize(NX_SHA256_SCE *context)
{
    /* Determine if the context is non-null.  */
    if (context == NX_CRYPTO_NULL)
        return(NX_CRYPTO_PTR_ERROR);

    /* First, clear the bit count for this context.  */
    context -> nx_sha256_bit_count[0] =  0;                   /* Clear the lower 32-bits of the count.*/
    context -> nx_sha256_bit_count[1] =  0;                   /* Clear the upper 32-bits of the count.*/

    /* Initialize SHA-256 state. */
    if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
    {
        context -> nx_sha256_states[0] = 0x67e6096a; /* A H0 */
        context -> nx_sha256_states[1] = 0x85ae67bb; /* B H1 */
        context -> nx_sha256_states[2] = 0x72f36e3c; /* C H2 */
        context -> nx_sha256_states[3] = 0x3af54fa5; /* D H3 */
        context -> nx_sha256_states[4] = 0x7f520e51; /* E H4 */
        context -> nx_sha256_states[5] = 0x8c68059b; /* F H5 */
        context -> nx_sha256_states[6] = 0xabd9831f; /* G H6 */
        context -> nx_sha256_states[7] = 0x19cde05b; /* H H7 */
    }
    else
    {
        /* Initialize SHA-256 state. */
         context -> nx_sha256_states[0] = 0x6a09e667; /* A H0 */
         context -> nx_sha256_states[1] = 0xbb67ae85; /* B H1 */
         context -> nx_sha256_states[2] = 0x3c6ef372; /* C H2 */
         context -> nx_sha256_states[3] = 0xa54ff53a; /* D H3 */
         context -> nx_sha256_states[4] = 0x510e527f; /* E H4 */
         context -> nx_sha256_states[5] = 0x9b05688c; /* F H5 */
         context -> nx_sha256_states[6] = 0x1f83d9ab; /* G H6 */
         context -> nx_sha256_states[7] = 0x5be0cd19; /* H H7 */
    }

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_sha256_sce_update                        PORTABLE C      */
/*                                                           5.11         */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function updates the SHA256 digest with new input from the     */
/*    caller.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA256 context pointer        */
/*    input_ptr                             Pointer to input data         */
/*    input_length                          Number of bytes in input      */
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
/*    _nx_crypto_method_sha256_sce_operation                              */
/*                                          Handle SHA1 operation         */
/*    _nx_crypto_sha256_digest_calculate    Calculate the SHA2 digest     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
UINT _nx_crypto_sha256_sce_update(NX_SHA256_SCE *context, UCHAR *input_ptr, UINT input_length)
{
ULONG   current_bytes;
ULONG   needed_fill_bytes;

    /* Determine if the context is non-null.  */
    if (context == NX_CRYPTO_NULL)
        return(NX_CRYPTO_PTR_ERROR);

    /* Determine if there is a length.  */
    if (input_length == 0)
        return(NX_CRYPTO_SUCCESS);

    /* Calculate the current byte count mod 64. Note the reason for the
       shift by 3 is to account for the 8 bits per byte.  */
    current_bytes =  (context -> nx_sha256_bit_count[0] >> 3) & 0x3F;

    /* Calculate the current number of bytes needed to be filled.  */
    needed_fill_bytes =  64 - current_bytes;

    /* Update the total bit count based on the input length.  */
    context -> nx_sha256_bit_count[0] += (input_length << 3);

    /* Determine if there is roll-over of the bit count into the MSW.  */
    if (context -> nx_sha256_bit_count[0] < (input_length << 3))
    {

        /* Yes, increment the MSW of the bit count.  */
        context -> nx_sha256_bit_count[1]++;
    }

    /* Update upper total bit count word.  */
    context -> nx_sha256_bit_count[1] +=  (input_length >> 29);

    /* Check for a partial buffer that needs to be transformed.  */
    if ((current_bytes) && (input_length >= needed_fill_bytes))
    {
        /* Yes, we can complete the buffer and transform it.  */

        /* Copy the appropriate portion of the input buffer into the internal
           buffer of the context.  */
        memcpy((void *) &(context -> nx_sha256_buffer[current_bytes]), (void *) input_ptr, needed_fill_bytes);

        /* Process the 64-byte (512 bit) buffer.  */
        if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
        {
            g_sce_hash_sha2.p_api->hashUpdate(g_sce_hash_sha2.p_ctrl, (uint32_t*)context -> nx_sha256_buffer, 16, (uint32_t *)context -> nx_sha256_states);
        }
        else
        {
            _nx_crypto_sha256_sce_process_buffer(context, context -> nx_sha256_buffer);
        }

        /* Adjust the pointers and length accordingly.  */
        input_length =  input_length - needed_fill_bytes;
        input_ptr =     input_ptr + needed_fill_bytes;

        /* Clear the remaining bits, since the buffer was processed.  */
        current_bytes =  0;
    }

    /* Process any and all whole blocks of input.  */
    while (input_length >= 64)
    {

        /* Process this 64-byte (512 bit) buffer.  */

        if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
        {
            g_sce_hash_sha2.p_api->hashUpdate(g_sce_hash_sha2.p_ctrl, (uint32_t*)input_ptr, 16, (uint32_t *)context -> nx_sha256_states);
        }
        else
        {
            _nx_crypto_sha256_sce_process_buffer(context, input_ptr);
        }

        /* Adjust the pointers and length accordingly.  */
        input_length =  input_length - 64;
        input_ptr =     input_ptr + 64;
    }

    /* Determine if there is anything left.  */
    if (input_length)
    {
        /* Save the remaining bytes in the internal buffer after any remaining bytes
           so that it is processed later.  */
        memcpy((void *) &(context -> nx_sha256_buffer[current_bytes]), (void *) input_ptr, input_length);
    }

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_sha256_sce_digest_calculate              PORTABLE C      */
/*                                                           5.11         */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates the final SHA256 digest. It is called      */
/*    when there is no more input for the digest and returns the 32-byte  */
/*    (256-bit) SHA256 digest to the caller.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA256 context pointer        */
/*    digest                                Pointer to return buffer      */
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
/*    _nx_crypto_method_sha256_sce_operation                              */
/*                                          Handle SHA1 operation         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
UINT _nx_crypto_sha256_sce_digest_calculate(NX_SHA256_SCE *context, UCHAR digest[32])
{
UCHAR   bit_count_string[8];
ULONG   current_byte_count;
ULONG   padding_bytes;


    /* Move the lower portion of the bit count into the array.  */
    bit_count_string[0] =  (UCHAR) (context -> nx_sha256_bit_count[1] >> 24);
    bit_count_string[1] =  (UCHAR) (context -> nx_sha256_bit_count[1] >> 16);
    bit_count_string[2] =  (UCHAR) (context -> nx_sha256_bit_count[1] >> 8);
    bit_count_string[3] =  (UCHAR) (context -> nx_sha256_bit_count[1]);
    bit_count_string[4] =  (UCHAR) (context -> nx_sha256_bit_count[0] >> 24);
    bit_count_string[5] =  (UCHAR) (context -> nx_sha256_bit_count[0] >> 16);
    bit_count_string[6] =  (UCHAR) (context -> nx_sha256_bit_count[0] >> 8);
    bit_count_string[7] =  (UCHAR) (context -> nx_sha256_bit_count[0]);

    /* Calculate the current byte count.  */
    current_byte_count =  (context -> nx_sha256_bit_count[0] >> 3) & 0x3F;

    /* Calculate the padding bytes needed.  */
    padding_bytes =  (current_byte_count < 56) ? (56 - current_byte_count) : (120 - current_byte_count);

    /* Add any padding required.  */
    _nx_crypto_sha256_sce_update(context, _nx_crypto_sha256_padding, padding_bytes);

    /* Add the in the length.  */
    _nx_crypto_sha256_sce_update(context, bit_count_string, 8);

    /* Now store the digest in the caller specified destination.  */
    if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
    {
        memcpy(digest, context -> nx_sha256_states, 32);
    }
    else
    {
        digest[ 0] =  (UCHAR) (context -> nx_sha256_states[0] >> 24);
        digest[ 1] =  (UCHAR) (context -> nx_sha256_states[0] >> 16);
        digest[ 2] =  (UCHAR) (context -> nx_sha256_states[0] >> 8);
        digest[ 3] =  (UCHAR) (context -> nx_sha256_states[0]);
        digest[ 4] =  (UCHAR) (context -> nx_sha256_states[1] >> 24);
        digest[ 5] =  (UCHAR) (context -> nx_sha256_states[1] >> 16);
        digest[ 6] =  (UCHAR) (context -> nx_sha256_states[1] >> 8);
        digest[ 7] =  (UCHAR) (context -> nx_sha256_states[1]);
        digest[ 8] =  (UCHAR) (context -> nx_sha256_states[2] >> 24);
        digest[ 9] =  (UCHAR) (context -> nx_sha256_states[2] >> 16);
        digest[10] =  (UCHAR) (context -> nx_sha256_states[2] >> 8);
        digest[11] =  (UCHAR) (context -> nx_sha256_states[2]);
        digest[12] =  (UCHAR) (context -> nx_sha256_states[3] >> 24);
        digest[13] =  (UCHAR) (context -> nx_sha256_states[3] >> 16);
        digest[14] =  (UCHAR) (context -> nx_sha256_states[3] >> 8);
        digest[15] =  (UCHAR) (context -> nx_sha256_states[3]);
        digest[16] =  (UCHAR) (context -> nx_sha256_states[4] >> 24);
        digest[17] =  (UCHAR) (context -> nx_sha256_states[4] >> 16);
        digest[18] =  (UCHAR) (context -> nx_sha256_states[4] >> 8);
        digest[19] =  (UCHAR) (context -> nx_sha256_states[4]);
        digest[20] =  (UCHAR) (context -> nx_sha256_states[5] >> 24);
        digest[21] =  (UCHAR) (context -> nx_sha256_states[5] >> 16);
        digest[22] =  (UCHAR) (context -> nx_sha256_states[5] >> 8);
        digest[23] =  (UCHAR) (context -> nx_sha256_states[5]);
        digest[24] =  (UCHAR) (context -> nx_sha256_states[6] >> 24);
        digest[25] =  (UCHAR) (context -> nx_sha256_states[6] >> 16);
        digest[26] =  (UCHAR) (context -> nx_sha256_states[6] >> 8);
        digest[27] =  (UCHAR) (context -> nx_sha256_states[6]);
        digest[28] =  (UCHAR) (context -> nx_sha256_states[7] >> 24);
        digest[29] =  (UCHAR) (context -> nx_sha256_states[7] >> 16);
        digest[30] =  (UCHAR) (context -> nx_sha256_states[7] >> 8);
        digest[31] =  (UCHAR) (context -> nx_sha256_states[7]);
    }

    /* Return successful completion.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_sha256_sce_operation              PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the SHA256 algorithm.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    AES operation                 */
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
/*    _nx_crypto_sha256_sce_initialize      Initialize the SHA2 consent   */
/*    _nx_crypto_sha256_sce_update          Update the digest with padding*/
/*                                            and length of digest        */
/*    _nx_crypto_sha256_sce_digest_calculate                              */
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
UINT _nx_crypto_method_sha256_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
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
NX_SHA256_SCE ctx;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    switch(op)
    {
    case NX_CRYPTO_HASH_INITIALIZE:
        _nx_crypto_sha256_sce_initialize((NX_SHA256_SCE*)crypto_metadata);
        break;

    case NX_CRYPTO_HASH_UPDATE:
        _nx_crypto_sha256_sce_update((NX_SHA256_SCE*)crypto_metadata, input, input_length_in_byte);
        break;

    case NX_CRYPTO_HASH_CALCULATE:
        _nx_crypto_sha256_sce_digest_calculate((NX_SHA256_SCE*)crypto_metadata, output);
        break;

    default:
        _nx_crypto_sha256_sce_initialize(&ctx);
        _nx_crypto_sha256_sce_update(&ctx, input, input_length_in_byte);
        _nx_crypto_sha256_sce_digest_calculate(&ctx, output);
        break;
    }

    return NX_CRYPTO_SUCCESS;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_sha256_sce_process_buffer                PORTABLE C      */
/*                                                           5.10 SP1     */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function invokes Synergy SSP crypto library for SHA256         */
/*    process on 64-byte (512-bit) blocks of data.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA256 context pointer        */
/*    buffer                                Pointer to 64-byte buffer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_sha256_update                                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-01-2017     Timothy Stapko           Initial Version 5.10 SP1      */
/*                                                                        */
/**************************************************************************/
VOID  _nx_crypto_sha256_sce_process_buffer(NX_SHA256_SCE *context, UCHAR buffer[64])
{
uint32_t u32buf[16];
uint32_t *u32input;
UINT i;


    /* Endianness convert */
    u32input = (uint32_t *)buffer;
    for(i = 0; i < 16; i++)
    {
        u32buf[i] = __REV(u32input[i]);
    }


    /* Update hash */
    g_sce_hash_sha2.p_api->hashUpdate(g_sce_hash_sha2.p_ctrl, u32buf, 16, (uint32_t *)context -> nx_sha256_states);

}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_sha256_init                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the common crypto method init callback for         */
/*    Microsoft supported SHA256 cryptographic algorithm.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    method                                Pointer to crypto method      */
/*    key                                   Pointer to key                */
/*    key_size_in_bits                      Length of key size in bits    */
/*    handler                               Returned crypto handler       */
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
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_crypto_method_sha256_sce_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
                                                   UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                   VOID  **handle,
                                                   VOID  *crypto_metadata,
                                                   ULONG crypto_metadata_size)
{

    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(handle);

    NX_CRYPTO_STATE_CHECK

    if ((method == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((((ULONG)crypto_metadata) & 0x3) != 0)
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_SHA256_SCE))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_sha256_cleanup                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function cleans up the crypto metadata.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_metadata                       Crypto metadata               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NX_CRYPTO_MEMSET                      Set the memory                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT  _nx_crypto_method_sha256_sce_cleanup(VOID *crypto_metadata)
{

    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clean up the crypto metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_SHA256_SCE));
#else
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
#endif/* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}
