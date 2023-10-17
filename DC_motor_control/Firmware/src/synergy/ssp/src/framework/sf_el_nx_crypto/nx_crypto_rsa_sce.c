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
/**     RSA public-key encryption algorithm                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_sce.h"
#include "nx_crypto_rsa_sce.h"




/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_rsa_sce_init                      PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the modulus for RSA context.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    method                                Pointer to RSA crypto method  */
/*    key                                   Pointer to modulus            */
/*    key_size_in_bits                      Length of modulus in bits     */
/*    handle                                Handle of method              */
/*    crypto_metadata                       Pointer to RSA context        */
/*    crypto_metadata_size                  Size of RSA context           */
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
/*  07-15-2018     Timothy Stapko           Modified comment(s),          */
/*                                            supported modulus with      */
/*                                            leading zero, resulting in  */
/*                                            version 5.11SP1             */
/*                                                                        */
/**************************************************************************/
UINT  _nx_crypto_method_rsa_sce_init(struct NX_CRYPTO_METHOD_STRUCT* method,
                                        UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                        VOID **handle,
                                        VOID *crypto_metadata,
                                        ULONG crypto_metadata_size)
{
NX_CRYPTO_RSA_SCE *ctx;

    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
	
#if (NX_CRYPTO_MAX_RSA_MODULUS_SIZE == RSA_KEY_SIZE_4096_BITS)

    /*
     * Handle RSA encryption/decryption needing 4096 bit key in software only. For the rest ,
     * continue to use the HW accelerator to do encryption/decryption
     */
    if(key_size_in_bits == RSA_KEY_SIZE_4096_BITS)
    {
        return _nx_crypto_method_rsa_init(method, key, key_size_in_bits, handle, crypto_metadata, crypto_metadata_size);
    }
#endif

    ctx = (NX_CRYPTO_RSA_SCE*)crypto_metadata;

    ctx -> nx_crypto_rsa_modulus = key;
    ctx -> nx_crypto_rsa_modulus_length = key_size_in_bits >> 3;

    if ((ctx -> nx_crypto_rsa_modulus_length == 129 ||
        ctx -> nx_crypto_rsa_modulus_length == 257) && *key == 0)
    {
        /* Skip leading zero. */
        ctx -> nx_crypto_rsa_modulus_length--;
        ctx -> nx_crypto_rsa_modulus++;
    }
    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_rsa_sce_operation                 PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the RSA operation function for crypto method.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    Operation                     */
/*    handle                                Handle to method              */
/*    method                                Pointer to RSA crypto method  */
/*    key                                   Exponent of RSA operation     */
/*    key_size_in_bits                      Size of exponent in bits      */
/*    input                                 Input stream                  */
/*    input_length_in_byte                  Length of input in byte       */
/*    iv_ptr                                Initial Vector (not used)     */
/*    output                                Output stream                 */
/*    output_length_in_byte                 Length of output in byte      */
/*    crypto_metadata                       Pointer to RSA context        */
/*    crypto_metadata_size                  Size of RSA context           */
/*    packet_ptr                            Pointer to packet (not used)  */
/*    nx_crypto_hw_process_callback         Pointer to callback function  */
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
UINT  _nx_crypto_method_rsa_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
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
NX_CRYPTO_RSA_SCE *ctx;
UCHAR *scratch;
UCHAR *exponent;
UINT exponent_length;
UINT i;
uint32_t* u32scratch;

    NX_CRYPTO_PARAMETER_NOT_USED(op);
    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    ctx = (NX_CRYPTO_RSA_SCE*)crypto_metadata;
    exponent_length = key_size_in_bits >> 3;
	
#if (NX_CRYPTO_MAX_RSA_MODULUS_SIZE == RSA_KEY_SIZE_4096_BITS)
    /*
     * Handle RSA encryption/decryption needing 4096 bit key in software only. For the rest,
     * continue to use the HW accelerator to do encryption/decryption
     */
    if(((exponent_length == NX_RSA_EXP_LENGTH_3_BYTE || exponent_length == NX_RSA_EXP_LENGTH_4_BYTE ||  exponent_length == NX_RSA_EXP_LENGTH_512_BYTE) && (ctx->nx_crypto_rsa_modulus_length == NX_RSA_MOD_LENGTH_512_BYTE && input_length_in_byte == NX_RSA_MOD_LENGTH_512_BYTE)))
    {
        return _nx_crypto_method_rsa_operation(op, handle, method, key, key_size_in_bits, input, input_length_in_byte, iv_ptr, output, output_length_in_byte, crypto_metadata, crypto_metadata_size, packet_ptr, nx_crypto_hw_process_callback);
    }
#endif

    exponent = key;
    scratch = (UCHAR*)ctx -> nx_crypto_rsa_scratch_buffer;

    /* Hardware RSA driver does not need to set prime number P and Q. So simply return. */
    if((op == NX_CRYPTO_SET_PRIME_P) || (op == NX_CRYPTO_SET_PRIME_Q))
        return(NX_CRYPTO_SUCCESS);


    if((exponent_length == NX_RSA_EXP_LENGTH_256_BYTE || exponent_length == NX_RSA_EXP_LENGTH_257_BYTE) && ctx->nx_crypto_rsa_modulus_length == NX_RSA_MOD_LENGTH_256_BYTE && input_length_in_byte == NX_RSA_MOD_LENGTH_256_BYTE)
    {
        if(exponent_length == NX_RSA_EXP_LENGTH_257_BYTE)
        {
            /* Skip leading zero. */
            exponent = exponent + 1;
        }
        memcpy(scratch, exponent, NX_RSA_EXP_LENGTH_256_BYTE);
        memcpy(scratch+NX_RSA_MOD_LENGTH_256_BYTE, ctx -> nx_crypto_rsa_modulus, NX_RSA_MOD_LENGTH_256_BYTE);

        if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
        {
            g_sce_rsa_2048.p_api->decrypt(g_sce_rsa_2048.p_ctrl, (uint32_t*)scratch, NULL, input_length_in_byte / NX_CRYPTO_SCE_BYTES_PER_WORD, (uint32_t*)input, (uint32_t*)output);
        }
        else
        {
            /*  Copy input to scratch buffer, of modulus length - because that is the expected block size*/
            memcpy((scratch + NX_RSA_EXP_LENGTH_256_BYTE + NX_RSA_MOD_LENGTH_256_BYTE), input, NX_RSA_MOD_LENGTH_256_BYTE);

            /* Reverse the endianness for the exponent, modulus and the input data */
            u32scratch = (uint32_t*)scratch;
            for(i = 0; i < ((NX_RSA_EXP_LENGTH_256_BYTE + NX_RSA_MOD_LENGTH_256_BYTE + NX_RSA_MOD_LENGTH_256_BYTE) / NX_CRYPTO_SCE_BYTES_PER_WORD); ++i)
            {
                u32scratch[i] = __REV(u32scratch[i]);
            }
            g_sce_rsa_2048.p_api->decrypt(g_sce_rsa_2048.p_ctrl,
                                          (uint32_t*)scratch,
                                          NULL,
                                          input_length_in_byte / NX_CRYPTO_SCE_BYTES_PER_WORD,
                                          (uint32_t*)(scratch + NX_RSA_EXP_LENGTH_256_BYTE + NX_RSA_MOD_LENGTH_256_BYTE),
                                          (uint32_t*)output);

            /* Reverse the endianness of the output data */
            u32scratch = (uint32_t*)output;
            for(i = 0; i < (NX_RSA_MOD_LENGTH_256_BYTE / NX_CRYPTO_SCE_BYTES_PER_WORD); ++i)
            {
                u32scratch[i] = __REV(u32scratch[i]);
            }
        }
    }
    else if((exponent_length == NX_RSA_EXP_LENGTH_128_BYTE || exponent_length == NX_RSA_EXP_LENGTH_129_BYTE) && ctx->nx_crypto_rsa_modulus_length == NX_RSA_MOD_LENGTH_128_BYTE && input_length_in_byte == NX_RSA_MOD_LENGTH_128_BYTE)
    {
        if(exponent_length == NX_RSA_EXP_LENGTH_129_BYTE)
        {
            /* Skip leading zero. */
            exponent = exponent + 1;
        }
        memcpy(scratch, exponent, NX_RSA_EXP_LENGTH_128_BYTE);
        memcpy(scratch+NX_RSA_MOD_LENGTH_128_BYTE, ctx -> nx_crypto_rsa_modulus, NX_RSA_MOD_LENGTH_128_BYTE);

        if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
        {
            g_sce_rsa_1024.p_api->decrypt(g_sce_rsa_1024.p_ctrl, (uint32_t*)scratch, NULL, input_length_in_byte / NX_CRYPTO_SCE_BYTES_PER_WORD, (uint32_t*)input, (uint32_t*)output);
        }
        else
        {
            /*  Copy input to scratch buffer, of modulus length - because that is the expected block size*/
            memcpy((scratch + NX_RSA_EXP_LENGTH_128_BYTE + NX_RSA_MOD_LENGTH_128_BYTE), input, NX_RSA_MOD_LENGTH_128_BYTE);

            /* Reverse the endianness for the exponent, modulus and the input data */
            u32scratch = (uint32_t*)scratch;
            for(i = 0; i < ((NX_RSA_EXP_LENGTH_128_BYTE + NX_RSA_MOD_LENGTH_128_BYTE + NX_RSA_MOD_LENGTH_128_BYTE) / NX_CRYPTO_SCE_BYTES_PER_WORD); ++i)
            {
                u32scratch[i] = __REV(u32scratch[i]);
            }
            g_sce_rsa_1024.p_api->decrypt(g_sce_rsa_1024.p_ctrl,
                                          (uint32_t*)scratch,
                                          NULL,
                                          input_length_in_byte / NX_CRYPTO_SCE_BYTES_PER_WORD,
                                          (uint32_t*)(scratch + NX_RSA_EXP_LENGTH_128_BYTE + NX_RSA_MOD_LENGTH_128_BYTE),
                                          (uint32_t*)output);

            /* Reverse the endianness of the output data */
            u32scratch = (uint32_t*)output;
            for(i = 0; i < (NX_RSA_MOD_LENGTH_128_BYTE / NX_CRYPTO_SCE_BYTES_PER_WORD); ++i)
            {
                u32scratch[i] = __REV(u32scratch[i]);
            }
        }
    }
    else if((exponent_length == NX_RSA_EXP_LENGTH_3_BYTE || exponent_length == NX_RSA_EXP_LENGTH_4_BYTE) && ctx->nx_crypto_rsa_modulus_length == NX_RSA_MOD_LENGTH_256_BYTE && input_length_in_byte == NX_RSA_MOD_LENGTH_256_BYTE)
    {
        if(exponent_length == NX_RSA_EXP_LENGTH_3_BYTE)
        {
            scratch[0] = 0;
            scratch[1] = key[0];
            scratch[2] = key[1];
            scratch[3] = key[2];
        }
        else if(exponent_length == NX_RSA_EXP_LENGTH_4_BYTE)
        {
            memcpy(scratch, exponent, NX_RSA_EXP_LENGTH_4_BYTE);
        }
        memcpy(scratch+4, ctx -> nx_crypto_rsa_modulus, NX_RSA_MOD_LENGTH_256_BYTE);

        if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
        {
            g_sce_rsa_2048.p_api->encrypt(g_sce_rsa_2048.p_ctrl, (uint32_t*)scratch, NULL, input_length_in_byte / NX_CRYPTO_SCE_BYTES_PER_WORD, (uint32_t*)input, (uint32_t*)output);
        }
        else
        {
            /*  Copy input to scratch buffer, of modulus length - because that is the expected block size*/
            memcpy((scratch + NX_RSA_EXP_LENGTH_256_BYTE + NX_RSA_MOD_LENGTH_256_BYTE), input, NX_RSA_MOD_LENGTH_256_BYTE);

            /* Reverse the endianness for the exponent, modulus and the input data */
            u32scratch = (uint32_t*)scratch;
            for(i = 0; i < ((NX_RSA_EXP_LENGTH_256_BYTE + NX_RSA_MOD_LENGTH_256_BYTE + NX_RSA_MOD_LENGTH_256_BYTE) / NX_CRYPTO_SCE_BYTES_PER_WORD); ++i)
            {
               u32scratch[i] = __REV(u32scratch[i]);
            }
            g_sce_rsa_2048.p_api->encrypt(g_sce_rsa_2048.p_ctrl,
                                          (uint32_t*)scratch,
                                          NULL,
                                          input_length_in_byte / NX_CRYPTO_SCE_BYTES_PER_WORD,
                                          (uint32_t*)(scratch + NX_RSA_EXP_LENGTH_256_BYTE + NX_RSA_MOD_LENGTH_256_BYTE),
                                          (uint32_t*)output);

            /* Reverse the endianness of the output data */
            u32scratch = (uint32_t*)output;
            for(i = 0; i < (NX_RSA_MOD_LENGTH_256_BYTE / NX_CRYPTO_SCE_BYTES_PER_WORD); ++i)
            {
                u32scratch[i] = __REV(u32scratch[i]);
            }
        }
    }
    else if((exponent_length == NX_RSA_EXP_LENGTH_3_BYTE || exponent_length == NX_RSA_EXP_LENGTH_4_BYTE) && ctx->nx_crypto_rsa_modulus_length == NX_RSA_MOD_LENGTH_128_BYTE && input_length_in_byte == NX_RSA_MOD_LENGTH_128_BYTE)
    {
        if(exponent_length == NX_RSA_EXP_LENGTH_3_BYTE)
        {
            scratch[0] = 0;
            scratch[1] = key[0];
            scratch[2] = key[1];
            scratch[3] = key[2];
        }
        else if(exponent_length == NX_RSA_EXP_LENGTH_4_BYTE)
        {
            memcpy(scratch, exponent, NX_RSA_EXP_LENGTH_4_BYTE);
        }
        memcpy(scratch+4, ctx -> nx_crypto_rsa_modulus, NX_RSA_MOD_LENGTH_128_BYTE);

        if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
        {
            g_sce_rsa_1024.p_api->encrypt(g_sce_rsa_1024.p_ctrl, (uint32_t*)scratch, NULL, input_length_in_byte / NX_CRYPTO_SCE_BYTES_PER_WORD, (uint32_t*)input, (uint32_t*)output);
        }
        else
        {
            /*  Copy input to scratch buffer, of modulus length - because that is the expected block size*/
            memcpy((scratch + NX_RSA_EXP_LENGTH_128_BYTE + NX_RSA_MOD_LENGTH_128_BYTE), input, NX_RSA_MOD_LENGTH_128_BYTE);

            /* Reverse the endianness for the exponent, modulus and the input data */
            u32scratch = (uint32_t*)scratch;
            for(i = 0; i < ((NX_RSA_EXP_LENGTH_128_BYTE + NX_RSA_MOD_LENGTH_128_BYTE + NX_RSA_MOD_LENGTH_128_BYTE) / NX_CRYPTO_SCE_BYTES_PER_WORD); ++i)
            {
                u32scratch[i] = __REV(u32scratch[i]);
            }
            g_sce_rsa_1024.p_api->encrypt(g_sce_rsa_1024.p_ctrl,
                                          (uint32_t*)scratch,
                                          NULL,
                                          input_length_in_byte / NX_CRYPTO_SCE_BYTES_PER_WORD,
                                          (uint32_t*)(scratch + NX_RSA_EXP_LENGTH_128_BYTE + NX_RSA_MOD_LENGTH_128_BYTE),
                                          (uint32_t*)output);

            /* Reverse the endianness of the output data */
            u32scratch = (uint32_t*)output;
            for(i = 0; i < (NX_RSA_MOD_LENGTH_128_BYTE / NX_CRYPTO_SCE_BYTES_PER_WORD); ++i)
            {
                u32scratch[i] = __REV(u32scratch[i]);
            }
        }
    }
    else if((exponent_length == NX_RSA_EXP_LENGTH_1_BYTE || exponent_length == NX_RSA_EXP_LENGTH_2_BYTE) && ctx->nx_crypto_rsa_modulus_length == NX_RSA_MOD_LENGTH_256_BYTE && input_length_in_byte == NX_RSA_MOD_LENGTH_256_BYTE)
    {
        if(exponent_length == NX_RSA_EXP_LENGTH_1_BYTE)
        {
            scratch[0] = 0;
            scratch[1] = 0;
            scratch[2] = 0;
            scratch[3] = key[0];
        }
        else if(exponent_length == NX_RSA_EXP_LENGTH_2_BYTE)
        {
            scratch[0] = 0;
            scratch[1] = 0;
            scratch[2] = key[0];
            scratch[3] = key[1];
        }

        memcpy(scratch+4, ctx -> nx_crypto_rsa_modulus, NX_RSA_MOD_LENGTH_256_BYTE);

        if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
        {
            g_sce_rsa_2048.p_api->encrypt(g_sce_rsa_2048.p_ctrl, (uint32_t*)scratch, NULL, input_length_in_byte / NX_CRYPTO_SCE_BYTES_PER_WORD, (uint32_t*)input, (uint32_t*)output);
        }
        else
        {
            /*  Copy input to scratch buffer, of modulus length - because that is the expected block size*/
            memcpy((scratch + NX_RSA_EXP_LENGTH_256_BYTE + NX_RSA_MOD_LENGTH_256_BYTE), input, NX_RSA_MOD_LENGTH_256_BYTE);

            /* Reverse the endianness for the exponent, modulus and the input data */
            u32scratch = (uint32_t*)scratch;
            for(i = 0; i < ((NX_RSA_EXP_LENGTH_256_BYTE + NX_RSA_MOD_LENGTH_256_BYTE + NX_RSA_MOD_LENGTH_256_BYTE) / NX_CRYPTO_SCE_BYTES_PER_WORD); ++i)
            {
                u32scratch[i] = __REV(u32scratch[i]);
            }
            g_sce_rsa_2048.p_api->decrypt(g_sce_rsa_2048.p_ctrl,
                                          (uint32_t*)scratch,
                                          NULL,
                                          input_length_in_byte / NX_CRYPTO_SCE_BYTES_PER_WORD,
                                          (uint32_t*)(scratch + NX_RSA_EXP_LENGTH_256_BYTE + NX_RSA_MOD_LENGTH_256_BYTE),
                                          (uint32_t*)output);
        }
    }
    else if((exponent_length == NX_RSA_EXP_LENGTH_1_BYTE || exponent_length == NX_RSA_EXP_LENGTH_2_BYTE) && ctx->nx_crypto_rsa_modulus_length == NX_RSA_MOD_LENGTH_128_BYTE && input_length_in_byte == NX_RSA_MOD_LENGTH_128_BYTE)
    {
        if(exponent_length == NX_RSA_EXP_LENGTH_1_BYTE)
        {
           scratch[0] = 0;
           scratch[1] = 0;
           scratch[2] = 0;
           scratch[3] = key[0];
        }
        else if(exponent_length == NX_RSA_EXP_LENGTH_2_BYTE)
        {
            scratch[0] = 0;
            scratch[1] = 0;
            scratch[2] = key[0];
            scratch[3] = key[1];
        }

        memcpy(scratch+4, ctx -> nx_crypto_rsa_modulus, NX_RSA_MOD_LENGTH_128_BYTE);

        if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
        {
            g_sce_rsa_1024.p_api->encrypt(g_sce_rsa_1024.p_ctrl, (uint32_t*)scratch, NULL, input_length_in_byte / NX_CRYPTO_SCE_BYTES_PER_WORD, (uint32_t*)input, (uint32_t*)output);
        }
        else
        {
            /*  Copy input to scratch buffer, of modulus length - because that is the expected block size*/
            memcpy((scratch + NX_RSA_EXP_LENGTH_128_BYTE + NX_RSA_MOD_LENGTH_128_BYTE), input, NX_RSA_MOD_LENGTH_128_BYTE);

            /* Reverse the endianness for the exponent, modulus and the input data */
            u32scratch = (uint32_t*)scratch;
            for(i = 0; i < ((NX_RSA_EXP_LENGTH_128_BYTE + NX_RSA_MOD_LENGTH_128_BYTE + NX_RSA_MOD_LENGTH_128_BYTE) / NX_CRYPTO_SCE_BYTES_PER_WORD); ++i)
            {
                u32scratch[i] = __REV(u32scratch[i]);
            }
            g_sce_rsa_1024.p_api->encrypt(g_sce_rsa_1024.p_ctrl,
                                          (uint32_t*)scratch,
                                          NULL,
                                          input_length_in_byte / NX_CRYPTO_SCE_BYTES_PER_WORD,
                                          (uint32_t*)(scratch + NX_RSA_EXP_LENGTH_128_BYTE + NX_RSA_MOD_LENGTH_128_BYTE),
                                          (uint32_t*)output);

            /* Reverse the endianness of the output data */
            u32scratch = (uint32_t*)output;
            for(i = 0; i < (NX_RSA_MOD_LENGTH_128_BYTE / NX_CRYPTO_SCE_BYTES_PER_WORD); ++i)
            {
                u32scratch[i] = __REV(u32scratch[i]);
            }
        }
    }
    else
    {

        /* FIXME: return unsupported operation */
        return(1);


    }
    return(NX_CRYPTO_SUCCESS);
}


