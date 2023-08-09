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

#include "nx_crypto_sha1_sce.h"





/* Define the padding array.  This is used to pad the message such that its length is 
   64 bits shy of being a multiple of 512 bits long.  */

static UCHAR   _nx_crypto_sha1_padding[64] = {0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_crypto_sha1_sce_initialize                      PORTABLE C      */ 
/*                                                           5.11         */
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function initializes the SHA1 context. It must be called prior */ 
/*    to creating the SHA1 digest.                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    context                               SHA1 context pointer          */ 
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
/*    _nx_crypto_method_sha1_sce_operation  Handle SHA1 operation         */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */ 
/**************************************************************************/ 
UINT  _nx_crypto_sha1_sce_initialize(NX_SHA1_SCE *context)
{

    /* Determine if the context is non-null.  */
    if (context == NX_CRYPTO_NULL)
        return(NX_CRYPTO_PTR_ERROR);

    /* First, clear the bit count for this context.  */
    context -> nx_sha1_bit_count[0] =  0;                   /* Clear the lower 32-bits of the count.*/
    context -> nx_sha1_bit_count[1] =  0;                   /* Clear the upper 32-bits of the count.*/ 

    /* Finally, setup the context states.  */
    if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
    {
        context -> nx_sha1_states[0] =  0x01234567UL;           /* Setup state A.                       */
        context -> nx_sha1_states[1] =  0x89ABCDEFUL;           /* Setup state B.                       */
        context -> nx_sha1_states[2] =  0xFEDCBA98UL;           /* Setup state C.                       */
        context -> nx_sha1_states[3] =  0x76543210UL;           /* Setup state D.                       */
        context -> nx_sha1_states[4] =  0xF0E1D2C3UL;           /* Setup state E.                       */
    }
    else
    {
        context -> nx_sha1_states[0] =  0x67452301UL;           /* Setup state A.                       */
        context -> nx_sha1_states[1] =  0xEFCDAB89UL;           /* Setup state B.                       */
        context -> nx_sha1_states[2] =  0x98BADCFEUL;           /* Setup state C.                       */
        context -> nx_sha1_states[3] =  0x10325476UL;           /* Setup state D.                       */
        context -> nx_sha1_states[4] =  0xC3D2E1F0UL;           /* Setup state E.                       */
    }

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_crypto_sha1_sce_update                          PORTABLE C      */ 
/*                                                           5.11         */
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function updates the digest calculation with new input from    */ 
/*    the caller.                                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    context                               SHA1 context pointer          */ 
/*    input_ptr                             Pointer to byte(s) of input   */ 
/*    input_length                          Length of bytes of input      */ 
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
/*    _nx_crypto_method_sha1_sce_operation  Handle SHA1 operation         */
/*    _nx_crypto_sha1_sce_digest_calculate  Calculate the SHA1 digest     */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */ 
/**************************************************************************/ 
UINT  _nx_crypto_sha1_sce_update(NX_SHA1_SCE *context, UCHAR *input_ptr, UINT input_length)
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
    current_bytes =  (context -> nx_sha1_bit_count[0] >> 3) & 0x3F;

    /* Calculate the current number of bytes needed to be filled.  */
    needed_fill_bytes =  64 - current_bytes;

    /* Update the total bit count based on the input length.  */
    context -> nx_sha1_bit_count[0] += (input_length << 3);
    
    /* Determine if there is roll-over of the bit count into the MSW.  */
    if (context -> nx_sha1_bit_count[0] < (input_length << 3))
    {

        /* Yes, increment the MSW of the bit count.  */
        context -> nx_sha1_bit_count[1]++;
    }

    /* Update upper total bit count word.  */
    context -> nx_sha1_bit_count[1] +=  (input_length >> 29);

    /* Check for a partial buffer that needs to be transformed.  */
    if ((current_bytes) && (input_length >= needed_fill_bytes))
    {

        /* Yes, we can complete the buffer and transform it.  */

        /* Copy the appropriate portion of the input buffer into the internal 
           buffer of the context.  */
        memcpy((void *) &(context -> nx_sha1_buffer[current_bytes]), (void *) input_ptr, needed_fill_bytes);

        /* Process the 64-byte (512 bit) buffer.  */
        if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
        {
            g_sce_hash_sha1.p_api->hashUpdate(g_sce_hash_sha1.p_ctrl, (uint32_t *)context -> nx_sha1_buffer, 16, (uint32_t *)context -> nx_sha1_states);
        }
        else
        {
            _nx_crypto_sha1_sce_process_buffer(context, context -> nx_sha1_buffer);
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
            g_sce_hash_sha1.p_api->hashUpdate(g_sce_hash_sha1.p_ctrl, (uint32_t *)input_ptr, 16, (uint32_t *)context -> nx_sha1_states);
        }
        else
        {
            _nx_crypto_sha1_sce_process_buffer(context, input_ptr);
        }

        /* Adjust the pointers and length accordingly.  */
        input_length =  input_length - 64;
        input_ptr =     input_ptr + 64;
    }

    /* Determine if there is anything left.  */
    if (input_length)
    {

        /* Save the remaining bytes in the internal buffer after any remaining bytes
           that it is processed later.  */
        memcpy((void *) &(context -> nx_sha1_buffer[current_bytes]), (void *) input_ptr, input_length);
    }

    /* Return success.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_crypto_sha1_sce_digest_calculate                PORTABLE C      */ 
/*                                                           5.11         */
/*  AUTHOR                                                                */ 
/*                                                                        */ 
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */ 
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function finishes calculation of the SHA1 digest. It is called */ 
/*    where there is no further input needed for the digest. The resulting*/ 
/*    20-byte (160-bit) SHA1 digest is returned to the caller.            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    context                               SHA1 context pointer          */ 
/*    digest                                Pointer to return digest in   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_crypto_sha1_sce_update            Update the digest with padding*/ 
/*                                            and length of digest        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_crypto_method_sha1_sce_operation  Handle SHA1 operation         */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */ 
/**************************************************************************/ 
UINT  _nx_crypto_sha1_sce_digest_calculate(NX_SHA1_SCE *context, UCHAR digest[20])
{

UCHAR   bit_count_string[8];
ULONG   current_byte_count;
ULONG   padding_bytes;


    /* Move the lower portion of the bit count into the array.  */
    bit_count_string[0] =  (UCHAR) (context -> nx_sha1_bit_count[1] >> 24);
    bit_count_string[1] =  (UCHAR) (context -> nx_sha1_bit_count[1] >> 16);
    bit_count_string[2] =  (UCHAR) (context -> nx_sha1_bit_count[1] >> 8);
    bit_count_string[3] =  (UCHAR) (context -> nx_sha1_bit_count[1]);
    bit_count_string[4] =  (UCHAR) (context -> nx_sha1_bit_count[0] >> 24);
    bit_count_string[5] =  (UCHAR) (context -> nx_sha1_bit_count[0] >> 16);
    bit_count_string[6] =  (UCHAR) (context -> nx_sha1_bit_count[0] >> 8);
    bit_count_string[7] =  (UCHAR) (context -> nx_sha1_bit_count[0]);

    /* Calculate the current byte count.  */
    current_byte_count =  (context -> nx_sha1_bit_count[0] >> 3) & 0x3F;

    /* Calculate the padding bytes needed.  */
    padding_bytes =  (current_byte_count < 56) ? (56 - current_byte_count) : (120 - current_byte_count);

    /* Add any padding required.  */
    _nx_crypto_sha1_sce_update(context, _nx_crypto_sha1_padding, padding_bytes);

    /* Add the in the length.  */
    _nx_crypto_sha1_sce_update(context, bit_count_string, 8);

    /* Now store the digest in the caller specified destination.  */
    if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
    {
        memcpy(digest, context -> nx_sha1_states, 20);
    }
    else
    {
        digest[ 0] =  (UCHAR) (context -> nx_sha1_states[0] >> 24);
        digest[ 1] =  (UCHAR) (context -> nx_sha1_states[0] >> 16);
        digest[ 2] =  (UCHAR) (context -> nx_sha1_states[0] >> 8);
        digest[ 3] =  (UCHAR) (context -> nx_sha1_states[0]);
        digest[ 4] =  (UCHAR) (context -> nx_sha1_states[1] >> 24);
        digest[ 5] =  (UCHAR) (context -> nx_sha1_states[1] >> 16);
        digest[ 6] =  (UCHAR) (context -> nx_sha1_states[1] >> 8);
        digest[ 7] =  (UCHAR) (context -> nx_sha1_states[1]);
        digest[ 8] =  (UCHAR) (context -> nx_sha1_states[2] >> 24);
        digest[ 9] =  (UCHAR) (context -> nx_sha1_states[2] >> 16);
        digest[10] =  (UCHAR) (context -> nx_sha1_states[2] >> 8);
        digest[11] =  (UCHAR) (context -> nx_sha1_states[2]);
        digest[12] =  (UCHAR) (context -> nx_sha1_states[3] >> 24);
        digest[13] =  (UCHAR) (context -> nx_sha1_states[3] >> 16);
        digest[14] =  (UCHAR) (context -> nx_sha1_states[3] >> 8);
        digest[15] =  (UCHAR) (context -> nx_sha1_states[3]);
        digest[16] =  (UCHAR) (context -> nx_sha1_states[4] >> 24);
        digest[17] =  (UCHAR) (context -> nx_sha1_states[4] >> 16);
        digest[18] =  (UCHAR) (context -> nx_sha1_states[4] >> 8);
        digest[19] =  (UCHAR) (context -> nx_sha1_states[4]);
    }

    /* Return successful completion.  */
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_sha1_sce_operation                PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the SHA1 algorithm.                                                 */
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
/*    _nx_crypto_sha1_sce_initialize        Initialize the SHA1 context   */
/*    _nx_crypto_sha1_sce_update            Update the digest with padding*/
/*                                            and length of digest        */
/*    _nx_crypto_sha1_sce_digest_calculate  Calculate the SHA1 digest     */
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
UINT  _nx_crypto_method_sha1_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
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
NX_SHA1_SCE ctx;

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
        _nx_crypto_sha1_sce_initialize((NX_SHA1_SCE*)crypto_metadata);
        break;

    case NX_CRYPTO_HASH_UPDATE:
        _nx_crypto_sha1_sce_update((NX_SHA1_SCE*)crypto_metadata, input, input_length_in_byte);
        break;

    case NX_CRYPTO_HASH_CALCULATE:
        _nx_crypto_sha1_sce_digest_calculate((NX_SHA1_SCE*)crypto_metadata, output);
        break;

    default:
        _nx_crypto_sha1_sce_initialize(&ctx);
        _nx_crypto_sha1_sce_update(&ctx, input, input_length_in_byte);
        _nx_crypto_sha1_sce_digest_calculate(&ctx, output);
        break;
    }

    return NX_CRYPTO_SUCCESS;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_sha1_sce_process_buffer                                  */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts the endianness of the data in the buffer     */
/*    and calls the SCE hash update                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               SHA1 context pointer          */
/*    buffer                                64 byte size buffer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                None                          */
/*                                                                        */
/*  CALLS                                                                 */
/*      None                                                              */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*      _nx_crypto_sha1_sce_update                                        */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*                                                                        */
/**************************************************************************/

VOID  _nx_crypto_sha1_sce_process_buffer(NX_SHA1_SCE *context, UCHAR buffer[64])
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
    g_sce_hash_sha1.p_api->hashUpdate(g_sce_hash_sha1.p_ctrl, u32buf, 16, (uint32_t *)context -> nx_sha1_states);

}
