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
 * Copyright [2018-2021] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
/**   MD5 Digest Algorithm (MD5)                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_sce.h"
#include "nx_crypto_md5_sce.h"


/* Define the padding array.  This is used to pad the message such that its length is
   64 bits shy of being a multiple of 512 bits long.  */

static UCHAR _nx_crypto_md5_padding[64] =
{ 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_md5_sce_initialize                       PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the MD5 context. It must be called prior  */
/*    to creating the MD5 digest.                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               MD5 context pointer           */
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
/*    _nx_crypto_method_md5_operation       Handle MD5 operation          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
UINT _nx_crypto_md5_sce_initialize(NX_MD5_SCE *context)
{

    /* Determine if the context is non-null.  */
    if (context == NX_CRYPTO_NULL)
    {
        return (NX_CRYPTO_PTR_ERROR);
    }

    /* First, clear the bit count for this context.  */
    context->nx_md5_bit_count[0] = 0; /* Clear the lower 32-bits of the count */
    context->nx_md5_bit_count[1] = 0; /* Clear the upper 32-bits of the count */

    /* Finally, setup the context states.  */
    context->nx_md5_states[0] = 0x67452301UL; /* Setup state A */
    context->nx_md5_states[1] = 0xEFCDAB89UL; /* Setup state B */
    context->nx_md5_states[2] = 0x98BADCFEUL; /* Setup state C */
    context->nx_md5_states[3] = 0x10325476UL; /* Setup state D */

    /* Return success.  */
    return (NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_md5_sce_update                           PORTABLE C      */
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
/*    context                               MD5 context pointer           */
/*    input_ptr                             Pointer to byte(s) of input   */
/*    input_length                          Length of bytes of input      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_md5_process_buffer         Process complete buffer,      */
/*                                            which is 64-bytes in size   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_md5_digest_calculate       Perform calculation of the    */
/*                                            MD5 digest                  */
/*    _nx_crypto_method_md5_operation       Handle MD5 operation          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
UINT _nx_crypto_md5_sce_update(NX_MD5_SCE *context, UCHAR *input_ptr, UINT input_length)
{

    ULONG current_bytes;
    ULONG needed_fill_bytes;

    /* Determine if the context is non-null.  */
    if (context == NX_CRYPTO_NULL)
    {
        return (NX_CRYPTO_PTR_ERROR);
    }

    /* Determine if there is a length.  */
    if (input_length == 0)
    {
        return (NX_CRYPTO_SUCCESS);
    }

    /* Calculate the current byte count mod 64. Note the reason for the
     shift by 3 is to account for the 8 bits per byte.  */
    current_bytes = (context->nx_md5_bit_count[0] >> 3) & 0x3F;

    /* Calculate the current number of bytes needed to be filled.  */
    needed_fill_bytes = SF_EL_NX_CRYPTO_MD5_BLOCK_SIZE_BYTES - current_bytes;

    /* Update the total bit count based on the input length.  */
    context->nx_md5_bit_count[0] += (input_length << 3);

    /* Determine if there is roll-over of the bit count into the MSW.  */
    if (context->nx_md5_bit_count[0] < (input_length << 3))
    {
        /* Yes, increment the MSW of the bit count.  */
        context->nx_md5_bit_count[1]++;
    }

    /* Update upper total bit count word.  */
    context->nx_md5_bit_count[1] += (input_length >> 29);

    /* Check for a partial buffer that needs to be transformed.  */
    if ((current_bytes) && (input_length >= needed_fill_bytes))
    {
        /* Yes, we can complete the buffer and transform it.  */

        /* Copy the appropriate portion of the input buffer into the internal
         buffer of the context.  */
        memcpy ((void *) &(context->nx_md5_buffer[current_bytes]), (void *) input_ptr, needed_fill_bytes);

        /* Process the 64-byte (512 bit) buffer.  */
        if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
        {
            _nx_crypto_md5_sce_process_buffer_for_little_endian (context, context->nx_md5_buffer);
         }
         else
         {
             _nx_crypto_md5_sce_process_buffer_for_big_endian (context, context -> nx_md5_buffer);
         }

        /* Adjust the pointers and length accordingly.  */
        input_length = input_length - needed_fill_bytes;
        input_ptr = input_ptr + needed_fill_bytes;

        /* Clear the remaining bits, since the buffer was processed.  */
        current_bytes = 0;
    }

    /* Process any and all whole blocks of input.  */
    while (input_length >= SF_EL_NX_CRYPTO_MD5_BLOCK_SIZE_BYTES)
    {

        /* Process this 64-byte (512 bit) buffer.  */
        if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
        {
            _nx_crypto_md5_sce_process_buffer_for_little_endian (context, input_ptr);
        }
        else
        {
            _nx_crypto_md5_sce_process_buffer_for_big_endian (context, input_ptr);
        }

        /* Adjust the pointers and length accordingly.  */
        input_length = input_length - SF_EL_NX_CRYPTO_MD5_BLOCK_SIZE_BYTES;
        input_ptr = input_ptr + SF_EL_NX_CRYPTO_MD5_BLOCK_SIZE_BYTES;
    }

    /* Determine if there is anything left.  */
    if (input_length)
    {

        /* Save the remaining bytes in the internal buffer after any remaining bytes
         that it is processed later.  */
        memcpy ((void *) &(context->nx_md5_buffer[current_bytes]), (void *) input_ptr, input_length);
    }

    /* Return success.  */
    return (NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_md5_sce_digest_calculate                 PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finishes calculation of the MD5 digest. It is called  */
/*    where there is no further input needed for the digest. The resulting*/
/*    16-byte (128-bit) MD5 digest is returned to the caller.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               MD5 context pointer           */
/*    digest                                16-byte (128-bit) digest      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_md5_update                 Update the digest with padding*/
/*                                            and length of digest        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_md5_operation       Handle MD5 operation          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
UINT _nx_crypto_md5_sce_digest_calculate(NX_MD5_SCE *context, UCHAR digest[16])
{

    UCHAR bit_count_string[8];
    ULONG current_byte_count;
    ULONG padding_bytes;

    /* Move the lower portion of the bit count into the array.  */
    bit_count_string[0] = (UCHAR) context->nx_md5_bit_count[0];
    bit_count_string[1] = (UCHAR) (context->nx_md5_bit_count[0] >> 8);
    bit_count_string[2] = (UCHAR) (context->nx_md5_bit_count[0] >> 16);
    bit_count_string[3] = (UCHAR) (context->nx_md5_bit_count[0] >> 24);
    bit_count_string[4] = (UCHAR) context->nx_md5_bit_count[1];
    bit_count_string[5] = (UCHAR) (context->nx_md5_bit_count[1] >> 8);
    bit_count_string[6] = (UCHAR) (context->nx_md5_bit_count[1] >> 16);
    bit_count_string[7] = (UCHAR) (context->nx_md5_bit_count[1] >> 24);

    /* Calculate the current byte count mod 64. Note the reason for the
     shift by 3 is to account for the 8 bits per byte.  */
    current_byte_count = (context->nx_md5_bit_count[0] >> 3) & 0x3F;

    /* Calculate the padding bytes needed.  */
    padding_bytes = (current_byte_count < 56) ? (56 - current_byte_count) : (120 - current_byte_count);

    /* Add any padding required.  */
    _nx_crypto_md5_sce_update (context, _nx_crypto_md5_padding, padding_bytes);

    /* Add the in the length.  */
    _nx_crypto_md5_sce_update (context, bit_count_string, 8);

    /* No need to swap when in big endian mode because the result is already in little endian. */
    memcpy (digest, context->nx_md5_states, SF_EL_NX_CRYPTO_MD5_DIGEST_SIZE_BYTES);


    /* Return successful completion.  */
    return (NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_md5_sce_process_buffer_for_little_endian PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function actually uses the MD5 algorithm to process a 64-byte  */
/*    (512 bit) buffer.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               MD5 context pointer           */
/*    buffer                                64-byte buffer                */
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
/*    _nx_crypto_md5_update                 Update the digest with padding*/
/*                                            and length of digest        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
VOID _nx_crypto_md5_sce_process_buffer_for_little_endian(NX_MD5_SCE *context, UCHAR buffer[64])
{
    uint32_t i = 0;

    /** As endian_flag under SCE configuration properties is set to CRYPTO_WORD_ENDIAN_LITTLE,
     *  nx_md5_states values need to be byte swapped */
    for (i = 0; i < 4; i++)
    {
        context->nx_md5_states[i] = __REV (context->nx_md5_states[i]);
    }

    /** Calculate the hash using MD5 Crypto engine driver */
    g_sce_hash_md5.p_api->hashUpdate (g_sce_hash_md5.p_ctrl, (uint32_t *) buffer, SF_EL_NX_CRYPTO_MD5_DIGEST_SIZE_BYTES,
                                      (uint32_t *) context->nx_md5_states);
    /** Byte swap and store the calculated MD5 hash */
    for (i = 0; i < 4; i++)
    {
        context->nx_md5_states[i] = __REV (context->nx_md5_states[i]);
    }
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_md5_sce_operation                PORTABLE C       */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the MD5 algorithm.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    MD5 operation                 */
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
/*    _nx_crypto_md5_initialize             Initialize the MD5 context    */
/*    _nx_crypto_md5_update                 Update the digest with padding*/
/*                                            and length of digest        */
/*    _nx_crypto_md5_digest_calculate       Perform calculation of the    */
/*                                            MD5 digest                  */
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
UINT _nx_crypto_method_md5_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
                                         VOID *handle, /* Crypto handler */
                                         struct NX_CRYPTO_METHOD_STRUCT *method,
                                         UCHAR *key,
                                         NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                         UCHAR *input,
                                         ULONG input_length_in_byte,
                                         UCHAR *iv_ptr,
                                         UCHAR *output,
                                         ULONG output_length_in_byte,
                                         VOID *crypto_metadata,
                                         ULONG crypto_metadata_size,
                                         VOID *packet_ptr,
                                         VOID (*nx_crypto_hw_process_callback)(VOID *packet_ptr, UINT status))
{
    NX_MD5_SCE ctx;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    switch (op)
    {
        case NX_CRYPTO_HASH_INITIALIZE:
            _nx_crypto_md5_sce_initialize ((NX_MD5_SCE *) crypto_metadata);
        break;

        case NX_CRYPTO_HASH_UPDATE:
            _nx_crypto_md5_sce_update ((NX_MD5_SCE *) crypto_metadata, input, input_length_in_byte);
        break;

        case NX_CRYPTO_HASH_CALCULATE:
            _nx_crypto_md5_sce_digest_calculate ((NX_MD5_SCE *) crypto_metadata, output);
        break;

        default:
            _nx_crypto_md5_sce_initialize (&ctx);
            _nx_crypto_md5_sce_update (&ctx, input, input_length_in_byte);
            _nx_crypto_md5_sce_digest_calculate (&ctx, output);
        break;
    }

    return NX_CRYPTO_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_md5_sce_process_buffer_for_big_endian    PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function swaps byte order required for big endian mode   in    */
/*    (512 bit) buffer.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    context                               MD5 context pointer           */
/*    buffer                                64-byte buffer                */
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
/*    _nx_crypto_md5_update                 Update the digest with padding*/
/*                                            and length of digest        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/**************************************************************************/
VOID  _nx_crypto_md5_sce_process_buffer_for_big_endian(NX_MD5_SCE *context, UCHAR buffer[64])
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

    /** Calculate the hash using MD5 Crypto engine driver */
    g_sce_hash_md5.p_api->hashUpdate (g_sce_hash_md5.p_ctrl,
                                      u32buf,
                                      SF_EL_NX_CRYPTO_MD5_DIGEST_SIZE_BYTES,
                                      (uint32_t *) context->nx_md5_states);
}
