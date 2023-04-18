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
/** AES Encryption                                                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/



/* Include necessary system files.  */

#include "nx_crypto_sce.h"
#include "nx_crypto_aes_sce.h"



/**************************************************************************/
/* Utility routines                                                       */
/**************************************************************************/
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
static uint32_t byte_swap32(uint32_t a);
static void private_copy_aes_gcm_block_bytes_to_word_be (uint32_t *p_dest, uint32_t *p_source, uint32_t num_bytes);
static void private_copy_aes_gcm_be_word_block_to_le_bytes (uint8_t *p_dest, uint32_t *p_source, uint32_t num_bytes);

/**
 * @brief Routine to swap bytes in a WORD.
 * ( Built in routine was getting optimized not giving the desired results)
 * @param[in]   a   - WORD to be swapped
 * @retval  bytes of the WORD are swapped. big endian to little and vice versa.
 */

static uint32_t byte_swap32(uint32_t a)
{
return ((((a)>>24)&0xff)|(((a)<<8)&0xff0000)|(((a)>>8)&0xff00)|(((a)<<24)&0xff000000));
}

/**
 * @brief Private helper function to copy bytes to a word array of AES Block size.
 * Since the number of bytes may not be a multiple of 4 / word size, it is manipulated here.
 *
 * @param[in,out] p_dest - pointer to uint32_t array of 4 WORDS / AES block size.
 * @param[in] p_source   - pointer to uint32_t array
 * @param[in] num_bytes  - number of bytes to be copied which is less than or equal to the AES block size.
 *
 * @note - If num_bytes is not the expected range, the routine does not do anything.
 * This is a private routine for a specific purpose so should be used cautiously.
 */
static void private_copy_aes_gcm_block_bytes_to_word_be (uint32_t *p_dest, uint32_t *p_source, uint32_t num_bytes)
{
     uint8_t * ptr;

    if ((0U == num_bytes ) || (((uint32_t)NX_CRYPTO_AES_BLOCK_SIZE) < num_bytes))
    {
        return;
    }

    memcpy(p_dest, p_source, NX_CRYPTO_AES_BLOCK_SIZE);

    // zeroise the buffer past the data.
     ptr = (uint8_t *)&p_dest[0];
     memset ((ptr+num_bytes), 0U, ((uint32_t)NX_CRYPTO_AES_BLOCK_SIZE - num_bytes));

     for (uint8_t i = 0U; i < (uint8_t)NX_CRYPTO_AES_BLOCK_SIZE_WORDS; i++)
     {
        p_dest[i] = byte_swap32(p_dest[i]);
     }
}

/**
 * @brief Private helper function to copy bytes from word array of AES Block size to a byte array.
 * Since the number of bytes may not be a multiple of 4 / word size, it is manipulated here.
 *
 * @param[in, out] p_dest  - pointer to uint8_t array
 * @param[in] p_source     - pointer to uint32_t array of 4 WORDS / AES block size.
 * @param[in] num_bytes    - number of bytes to be copied which is less than or equal to the AES block size.
 *
 * @note - If num_bytes is not the expected range, the routine does not do anything.
 * This is a private routine for a specific purpose so should be used cautiously.
 */
static void private_copy_aes_gcm_be_word_block_to_le_bytes (uint8_t *p_dest, uint32_t *p_source, uint32_t num_bytes)
{
     uint8_t * ptr;
     uint32_t tmp2[NX_CRYPTO_AES_BLOCK_SIZE_WORDS] = {0};

    if ((0U == num_bytes ) || (((uint32_t)NX_CRYPTO_AES_BLOCK_SIZE) < num_bytes))
    {
        return;
    }

    memcpy(tmp2, p_source, NX_CRYPTO_AES_BLOCK_SIZE);


     for (uint8_t i = 0U; i < (uint8_t)NX_CRYPTO_AES_BLOCK_SIZE_WORDS; i++)
     {
         tmp2[i] = byte_swap32(tmp2[i]);
     }

     // zeroise the buffer past the data.
      ptr = (uint8_t *)&tmp2[0];
      memset ((ptr+num_bytes), 0U, ((uint32_t)NX_CRYPTO_AES_BLOCK_SIZE - num_bytes));

     memcpy (p_dest, ptr, num_bytes);
}
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_aes_key_set_sce                                  PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function configures key for AES encryption and decryption.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    aes_ptr                               Pointer to AES control block  */
/*    operation_type                        Operation type                */
/*    key                                   Pointer to key string         */
/*    key_size                              Size of the key, valid values */
/*                                            are: 4 and 8                */
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
/*    _nx_crypto_method_aes_sce_init        Initialize crypto method      */
/*    _nx_crypto_method_aes_sce_operation   Handle AES operation          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
UINT nx_aes_key_set_sce(NX_CRYPTO_AES_SCE *aes_ptr, UINT operation_type, UCHAR *key, UCHAR key_size)
{
    NX_CRYPTO_PARAMETER_NOT_USED(operation_type);
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
    if(key_size != 4 && key_size != 6 && key_size != 8)
    {
        return(NX_CRYPTO_AES_UNSUPPORTED_KEY_SIZE);
    }
#else
    if(key_size != 4 && key_size != 8)
    {
        return(NX_CRYPTO_AES_UNSUPPORTED_KEY_SIZE);
    }
#endif

    /* Set the AES key size (should be in 32-bit *words*). */
    aes_ptr->nx_crypto_aes_key_size = key_size;

    if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
    {
            memcpy(aes_ptr->nx_crypto_aes_key_schedule, key, (size_t)key_size << 2);
    }
    else
    {
        uint32_t *u32_key;
        uint32_t *rev_key;
        uint8_t i;

        /* Little Endian to Big Endian conversion */
        u32_key = (uint32_t *)key;
        rev_key = (uint32_t *)(aes_ptr->nx_crypto_aes_key_schedule);
        for (i = 0; i < key_size; i++)
        {
            rev_key[i] = __REV(u32_key[i]);
        }
    }

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_aes_cbc_encrypt_sce                              PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs AES encryption for CBC mode.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    aes_ptr                               Pointer to AES control block  */
/*    length                                The length of output buffer   */
/*    input                                 Pointer to an input message   */
/*    output                                Pointer to an output buffer   */
/*                                            for storing the encrypted   */
/*                                            message                     */
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
/*    _nx_crypto_method_aes_sce_operation   Handle AES operation          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
UINT nx_aes_cbc_encrypt_sce(NX_CRYPTO_AES_SCE *aes_ptr, ULONG length, UCHAR *input, UCHAR *output)
{
uint32_t* tmp;
UINT i;

    if (CRYPTO_WORD_ENDIAN_BIG == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
    {
        /* Input data - Little Endian to Big Endian conversion */
        tmp = (uint32_t *)input;
        for(i = 0; i < length / NX_CRYPTO_SCE_BYTES_PER_WORD; i++)
        {
            tmp[i] = __REV(tmp[i]);
        }
    }

    if (aes_ptr->nx_crypto_aes_key_size == NX_CRYPTO_AES_KEY_SIZE_128_BITS)
    {
        g_sce_aes_128_cbc.p_api->encrypt(g_sce_aes_128_cbc.p_ctrl,
                                         (uint32_t *)(aes_ptr->nx_crypto_aes_key_schedule),
                                         (uint32_t *)(aes_ptr->nx_crypto_aes_iv_data), length / NX_CRYPTO_SCE_BYTES_PER_WORD,
                                         (uint32_t *)input, (uint32_t *)output);
    }
    else if (aes_ptr->nx_crypto_aes_key_size == NX_CRYPTO_AES_KEY_SIZE_256_BITS)
    {
        g_sce_aes_256_cbc.p_api->encrypt(g_sce_aes_256_cbc.p_ctrl,
                                         (uint32_t *)(aes_ptr->nx_crypto_aes_key_schedule),
                                         (uint32_t *)(aes_ptr->nx_crypto_aes_iv_data), length / NX_CRYPTO_SCE_BYTES_PER_WORD,
                                         (uint32_t *)input, (uint32_t *)output);
    }
    else
    {
        return (NX_CRYPTO_AES_UNSUPPORTED_KEY_SIZE);
    }


    if (CRYPTO_WORD_ENDIAN_BIG == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
    {
        /* Output data - Big Endian to Little Endian conversion */
        tmp = (uint32_t *)output;
        for(i = 0; i < length / NX_CRYPTO_SCE_BYTES_PER_WORD; i++)
        {
            tmp[i] = __REV(tmp[i]);
        }
    }

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_aes_cbc_decrypt_sce                              PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs AES decryption for CBC mode.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    aes_ptr                               Pointer to AES control block  */
/*    length                                The length of output buffer   */
/*    input                                 Pointer to an input message   */
/*    output                                Pointer to an output buffer   */
/*                                            for storing the encrypted   */
/*                                            message                     */
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
/*    _nx_crypto_method_aes_sce_operation   Handle AES operation          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  12-15-2017     Timothy Stapko           Initial Version 5.11          */
/*                                                                        */
/**************************************************************************/
UINT nx_aes_cbc_decrypt_sce(NX_CRYPTO_AES_SCE *aes_ptr, ULONG length, UCHAR *input, UCHAR *output)
{
uint32_t* tmp;
UINT i;

    if (CRYPTO_WORD_ENDIAN_BIG == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
    {
        /* Input data - Little Endian to Big Endian conversion */
        tmp = (uint32_t *)input;
        for(i = 0; i < length / NX_CRYPTO_SCE_BYTES_PER_WORD; i++)
        {
            tmp[i] = __REV(tmp[i]);
        }
    }

    if (aes_ptr->nx_crypto_aes_key_size == NX_CRYPTO_AES_KEY_SIZE_128_BITS)
    {
         g_sce_aes_128_cbc.p_api->decrypt(g_sce_aes_128_cbc.p_ctrl,
                                          (uint32_t *)(aes_ptr->nx_crypto_aes_key_schedule),
                                          (uint32_t *)(aes_ptr->nx_crypto_aes_iv_data), length / NX_CRYPTO_SCE_BYTES_PER_WORD,
                                          (uint32_t *)input, (uint32_t *)output);
    }
    else if (aes_ptr->nx_crypto_aes_key_size == NX_CRYPTO_AES_KEY_SIZE_256_BITS)
    {
         g_sce_aes_256_cbc.p_api->decrypt(g_sce_aes_256_cbc.p_ctrl,
                                          (uint32_t *)(aes_ptr->nx_crypto_aes_key_schedule),
                                          (uint32_t *)(aes_ptr->nx_crypto_aes_iv_data), length / NX_CRYPTO_SCE_BYTES_PER_WORD,
                                          (uint32_t *)input, (uint32_t *)output);
    }
    else
    {
        return (NX_CRYPTO_AES_UNSUPPORTED_KEY_SIZE);
    }


    if (CRYPTO_WORD_ENDIAN_BIG == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
    {
        /* Output data - Big Endian to Little Endian conversion */
        tmp = (uint32_t *)output;
        for(i = 0; i < length / NX_CRYPTO_SCE_BYTES_PER_WORD; i++)
        {
            tmp[i] = __REV(tmp[i]);
        }
    }
    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_aes_gcm_encrypt_sce                      PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs AES encryption for GCM mode.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    AES operation                 */
/*    crypto_ptr                            Pointer to AES Instance       */
/*    ctx                                   Pointer to AES control block  */
/*    input                                 Pointer to an input message   */
/*    input_length_in_byte                  The length of input buffer    */
/*    output                                Pointer to an output buffer   */
/*                                            for storing the encrypted   */
/*                                            message                     */
/*    tag                                   Tag buffer for output         */
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
/*    _nx_crypto_method_aes_gcm_sce_operation                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  07-15-2018     Timothy Stapko           Initial Version 5.11 SP1      */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
UINT _nx_crypto_aes_gcm_encrypt_sce(UINT op,
                                    aes_instance_t *crypto_ptr,
                                    NX_CRYPTO_AES_SCE *ctx,
                                    UCHAR *input,
                                    ULONG input_length_in_byte,
                                    UCHAR *output,
                                    UCHAR *tag)
{
UINT status;

    if((NX_CRYPTO_ENCRYPT_INITIALIZE == op) || (NX_CRYPTO_ENCRYPT == op))
    {
        /* Set additional authentication data. */
        status = crypto_ptr -> p_api -> zeroPaddingEncrypt(crypto_ptr -> p_ctrl,
                                                           (uint32_t *)(ctx -> nx_crypto_aes_key_schedule),
                                                           (uint32_t *)(ctx -> nx_crypto_aes_iv_data),
                                                           ctx -> nx_crypto_aes_additional_data_len,
                                                           (uint32_t *)ctx -> nx_crypto_aes_additional_data,
                                                           NULL);
        if (status)
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }
    }


    if((NX_CRYPTO_ENCRYPT_UPDATE == op) || (NX_CRYPTO_ENCRYPT == op))
    {
        /* Encrypt data. */
        status = crypto_ptr -> p_api -> zeroPaddingEncrypt(crypto_ptr -> p_ctrl,
                                                           (uint32_t *)(ctx -> nx_crypto_aes_key_schedule),
                                                           (uint32_t *)(ctx -> nx_crypto_aes_iv_data),
                                                           input_length_in_byte,
                                                           (uint32_t *)input,
                                                           (uint32_t *)output);
        if (status)
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }
    }

    if((NX_CRYPTO_ENCRYPT_CALCULATE == op) || (NX_CRYPTO_ENCRYPT == op))
    {
        /* Get TAG. */
        status = crypto_ptr -> p_api-> zeroPaddingEncrypt(crypto_ptr -> p_ctrl,
                                                          (uint32_t *)(ctx -> nx_crypto_aes_key_schedule),
                                                          (uint32_t *)(ctx -> nx_crypto_aes_iv_data),
                                                          0,
                                                          NULL,
                                                          (uint32_t *)tag);
        if (status)
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }
    }
    return(NX_CRYPTO_SUCCESS);
}
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_aes_gcm_encrypt_sce_be                      PORTABLE C   */
/*                                                           5.11 SP1     */
/*                                                                        */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs AES encryption for GCM mode.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    AES operation                 */
/*    crypto_ptr                            Pointer to AES Instance       */
/*    ctx                                   Pointer to AES control block  */
/*    input                                 Pointer to an input message   */
/*    input_length_in_byte                  The length of input buffer    */
/*    output                                Pointer to an output buffer   */
/*                                            for storing the decrypted   */
/*                                            message                     */
/*    tag                                   Tag buffer for verification   */
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
/*    _nx_crypto_method_aes_gcm_sce_operation                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*                                                                        */
/**************************************************************************/

#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
UINT _nx_crypto_aes_gcm_encrypt_sce_be( UINT op,
                                        aes_instance_t *crypto_ptr,
                                        NX_CRYPTO_AES_SCE *ctx,
                                        UCHAR *input,
                                        ULONG input_length_in_byte,
                                        UCHAR *output,
                                        UCHAR *tag)
{
UINT status;
/*
 * For Big endian support
 */
uint32_t i = 0;
uint32_t num_blocks = 0;
uint32_t remainder_len = 0;
uint32_t tmp_length = 0;
uint32_t *tmp;
uint32_t tmp_aes_array[NX_CRYPTO_AES_BLOCK_SIZE_WORDS] = {0};
uint32_t tmp_aes_result_array[NX_CRYPTO_AES_BLOCK_SIZE_WORDS] = {0};

    /* Set additional authentication data. */
    if((NX_CRYPTO_ENCRYPT_INITIALIZE == op) || (NX_CRYPTO_ENCRYPT == op))
    {
        /* AAD data - Little Endian to Big Endian conversion */
        num_blocks = ctx -> nx_crypto_aes_additional_data_len / NX_CRYPTO_AES_BLOCK_SIZE;
        remainder_len =  ctx -> nx_crypto_aes_additional_data_len - (num_blocks * NX_CRYPTO_AES_BLOCK_SIZE);
        tmp_length = ctx -> nx_crypto_aes_additional_data_len - remainder_len;
        tmp = (uint32_t *)ctx -> nx_crypto_aes_additional_data;

        if (num_blocks)
        {
            for(i = 0; i < tmp_length / NX_CRYPTO_SCE_BYTES_PER_WORD; i++)
            {
                tmp[i] = __REV(tmp[i]);
            }

            status = crypto_ptr -> p_api -> zeroPaddingEncrypt(crypto_ptr -> p_ctrl,
                                                               (uint32_t *)(ctx -> nx_crypto_aes_key_schedule),
                                                               (uint32_t *)(ctx->nx_crypto_aes_iv_data),
                                                               tmp_length,
                                                               (uint32_t *)ctx -> nx_crypto_aes_additional_data,
                                                               NULL);

            /*
             * Revert the AAD back to little endian format
             */
            tmp = (uint32_t *)ctx -> nx_crypto_aes_additional_data;
            for(i = 0; i < tmp_length / NX_CRYPTO_SCE_BYTES_PER_WORD; i++)
            {
                tmp[i] = __REV(tmp[i]);
            }


            if (status)
            {
                return(NX_CRYPTO_NOT_SUCCESSFUL);
            }
        }

        /* Set additional authentication data - if there are remaining bytes. */
        if (0 != remainder_len)
        {
            private_copy_aes_gcm_block_bytes_to_word_be (tmp_aes_array,
                                                         (uint32_t *)(tmp + (tmp_length/NX_CRYPTO_SCE_BYTES_PER_WORD)),
                                                         remainder_len);

            status = crypto_ptr -> p_api -> zeroPaddingEncrypt(crypto_ptr -> p_ctrl,
                                                               (uint32_t *)(ctx -> nx_crypto_aes_key_schedule),
                                                               (uint32_t *)(ctx->nx_crypto_aes_iv_data),
                                                               remainder_len,
                                                               (uint32_t *)tmp_aes_array,
                                                               NULL);
            if (status)
            {
                return(NX_CRYPTO_NOT_SUCCESSFUL);
            }
        }
    }


    /*
     * Encrypt data.
     */

    if((NX_CRYPTO_ENCRYPT_UPDATE == op) || (NX_CRYPTO_ENCRYPT == op))
    {
        /* input data - Little Endian to Big Endian conversion */
        num_blocks = input_length_in_byte / NX_CRYPTO_AES_BLOCK_SIZE;
        remainder_len =  input_length_in_byte - (num_blocks * NX_CRYPTO_AES_BLOCK_SIZE);
        tmp_length = input_length_in_byte - remainder_len;
        tmp = (uint32_t *)input;

        if (num_blocks)
        {
             for(i = 0; i < tmp_length / NX_CRYPTO_SCE_BYTES_PER_WORD; i++)
             {
                 tmp[i] = __REV(tmp[i]);
             }

            status = crypto_ptr -> p_api -> zeroPaddingEncrypt(crypto_ptr -> p_ctrl,
                                                               (uint32_t *)(ctx -> nx_crypto_aes_key_schedule),
                                                               (uint32_t *)(ctx->nx_crypto_aes_iv_data),
                                                               tmp_length,
                                                               (uint32_t *)input,
                                                               (uint32_t *)output);
            if (status)
            {
                return(NX_CRYPTO_NOT_SUCCESSFUL);
            }
        }
        if (0 != remainder_len)
        {
            private_copy_aes_gcm_block_bytes_to_word_be (tmp_aes_array, (uint32_t *)(tmp + tmp_length / NX_CRYPTO_SCE_BYTES_PER_WORD) , remainder_len);

            status = crypto_ptr -> p_api -> zeroPaddingEncrypt(crypto_ptr -> p_ctrl,
                                                               (uint32_t *)(ctx -> nx_crypto_aes_key_schedule),
                                                               (uint32_t *)(ctx->nx_crypto_aes_iv_data),
                                                               remainder_len,
                                                               (uint32_t *)tmp_aes_array,
                                                               (uint32_t *)tmp_aes_result_array);
            if (status)
            {
                return(NX_CRYPTO_NOT_SUCCESSFUL);
            }

        }

        /* Output data - Big Endian to Little Endian conversion */
         tmp = (uint32_t *)output;
         for(i = 0; i < input_length_in_byte / NX_CRYPTO_SCE_BYTES_PER_WORD; i++)
         {
             tmp[i] = __REV(tmp[i]);
         }
         private_copy_aes_gcm_be_word_block_to_le_bytes ((output + tmp_length),  tmp_aes_result_array, remainder_len );
    }


    /* Get TAG. */
    if((NX_CRYPTO_ENCRYPT_CALCULATE == op) || (NX_CRYPTO_ENCRYPT == op))
    {
        status = crypto_ptr -> p_api-> zeroPaddingEncrypt(crypto_ptr -> p_ctrl,
                                                          (uint32_t *)(ctx -> nx_crypto_aes_key_schedule),
                                                          (uint32_t *)(ctx->nx_crypto_aes_iv_data),
                                                          0,
                                                          NULL,
                                                          (uint32_t *)tag);
        if (status)
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }

        /* Output tag - Big Endian to Little Endian conversion */
        tmp = (uint32_t *)tag;
        for(i = 0; i < NX_CRYPTO_AES_GCM_TAG_LEN_IN_BYTES / NX_CRYPTO_SCE_BYTES_PER_WORD; i++)
        {
            tmp[i] = __REV(tmp[i]);
        }
    }

    return(NX_CRYPTO_SUCCESS);
}
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_aes_gcm_decrypt_sce                      PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs AES decryption for GCM mode.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    AES operation                 */
/*    crypto_ptr                            Pointer to AES Instance       */
/*    ctx                                   Pointer to AES control block  */
/*    input                                 Pointer to an input message   */
/*    input_length_in_byte                  The length of input buffer    */
/*    output                                Pointer to an output buffer   */
/*                                            for storing the decrypted   */
/*                                            message                     */
/*    tag                                   Tag buffer for verification   */
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
/*    _nx_crypto_method_aes_gcm_sce_operation                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  07-15-2018     Timothy Stapko           Initial Version 5.11 SP1      */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
UINT _nx_crypto_aes_gcm_decrypt_sce(UINT op,
                                    aes_instance_t *crypto_ptr,
                                    NX_CRYPTO_AES_SCE *ctx,
                                    UCHAR *input,
                                    ULONG input_length_in_byte,
                                    UCHAR *output,
                                    UCHAR *tag)
{
UINT status;

    if((NX_CRYPTO_DECRYPT_INITIALIZE == op) || (NX_CRYPTO_DECRYPT == op))
    {
        /* Set additional authentication data. */
        status = crypto_ptr -> p_api -> zeroPaddingDecrypt(crypto_ptr -> p_ctrl,
                                                           (uint32_t *)(ctx -> nx_crypto_aes_key_schedule),
                                                           (uint32_t *)(ctx -> nx_crypto_aes_iv_data),
                                                           ctx -> nx_crypto_aes_additional_data_len,
                                                           (uint32_t *)ctx -> nx_crypto_aes_additional_data,
                                                           NULL);

        if (status)
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }
    }

    if((NX_CRYPTO_DECRYPT_UPDATE == op) || (NX_CRYPTO_DECRYPT == op))
    {
        /* Decrypt data. */
        status = crypto_ptr -> p_api -> zeroPaddingDecrypt(crypto_ptr -> p_ctrl,
                                                           (uint32_t *)(ctx -> nx_crypto_aes_key_schedule),
                                                           (uint32_t *)(ctx -> nx_crypto_aes_iv_data),
                                                           input_length_in_byte,
                                                           (uint32_t *)input,
                                                           (uint32_t *)output);
        if (status)
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }
    }

    if((NX_CRYPTO_DECRYPT_CALCULATE == op) || (NX_CRYPTO_DECRYPT == op))
    {
        /* Set TAG. */
        status = crypto_ptr -> p_api -> setGcmTag(crypto_ptr -> p_ctrl, 4,
                                                  (uint32_t *)tag);
        if (status)
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }

        /* Verify additional authentication data. */
        status = crypto_ptr -> p_api -> zeroPaddingDecrypt(crypto_ptr -> p_ctrl,
                                                           (uint32_t *)(ctx -> nx_crypto_aes_key_schedule),
                                                           (uint32_t *)(ctx -> nx_crypto_aes_iv_data),
                                                           0,
                                                           NULL,
                                                           NULL);

        if (status)
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }
    }

    return(NX_CRYPTO_SUCCESS);
}
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_aes_gcm_decrypt_sce_be                      PORTABLE C   */
/*                                                           5.11 SP1     */
/*                                                                        */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs AES decryption for GCM mode.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    AES operation                 */
/*    crypto_ptr                            Pointer to AES Instance       */
/*    ctx                                   Pointer to AES control block  */
/*    input                                 Pointer to an input message   */
/*    input_length_in_byte                  The length of input buffer    */
/*    output                                Pointer to an output buffer   */
/*                                            for storing the decrypted   */
/*                                            message                     */
/*    tag                                   Tag buffer for verification   */
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
/*    _nx_crypto_method_aes_gcm_sce_operation                             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
UINT _nx_crypto_aes_gcm_decrypt_sce_be( UINT op,
                                        aes_instance_t *crypto_ptr,
                                        NX_CRYPTO_AES_SCE *ctx,
                                        UCHAR *input,
                                        ULONG input_length_in_byte,
                                        UCHAR *output,
                                        UCHAR *tag)
{
UINT status;

/*
 * For Big endian support
 */
uint32_t i = 0;
uint32_t num_blocks = 0;
uint32_t remainder_len = 0;
uint32_t tmp_length = 0;
uint32_t *tmp;
uint32_t tmp_aes_array[NX_CRYPTO_AES_BLOCK_SIZE_WORDS] = {0};
uint32_t tmp_aes_result_array[NX_CRYPTO_AES_BLOCK_SIZE_WORDS] = {0};

    /* Set additional authentication data. */
    if((NX_CRYPTO_DECRYPT_INITIALIZE == op) || (NX_CRYPTO_DECRYPT == op))
    {
        /* AAD data - Little Endian to Big Endian conversion */
        num_blocks = ctx -> nx_crypto_aes_additional_data_len / NX_CRYPTO_AES_BLOCK_SIZE;
        remainder_len =  ctx -> nx_crypto_aes_additional_data_len - (num_blocks * NX_CRYPTO_AES_BLOCK_SIZE);
        tmp_length = ctx -> nx_crypto_aes_additional_data_len - remainder_len;
        tmp = (uint32_t *)ctx -> nx_crypto_aes_additional_data;

        if (num_blocks)
        {
            for(i = 0; i < tmp_length / NX_CRYPTO_SCE_BYTES_PER_WORD; i++)
            {
                tmp[i] = __REV(tmp[i]);
            }

            status = crypto_ptr -> p_api -> zeroPaddingDecrypt(crypto_ptr -> p_ctrl,
                                                               (uint32_t *)(ctx -> nx_crypto_aes_key_schedule),
                                                               (uint32_t *)(ctx->nx_crypto_aes_iv_data),
                                                               tmp_length,
                                                               (uint32_t *)ctx -> nx_crypto_aes_additional_data,
                                                               NULL);
            /*
             * Revert the AAD back to little endian format
             */
            tmp = (uint32_t *)ctx -> nx_crypto_aes_additional_data;
            for(i = 0; i < tmp_length / NX_CRYPTO_SCE_BYTES_PER_WORD; i++)
            {
                tmp[i] = __REV(tmp[i]);
            }


            if (status)
            {
                return(NX_CRYPTO_NOT_SUCCESSFUL);
            }
        }

        /* Set additional authentication data - if there are remaining bytes. */
        if (0 != remainder_len)
        {
            private_copy_aes_gcm_block_bytes_to_word_be (tmp_aes_array,
                                                         (uint32_t *)(tmp + (tmp_length/NX_CRYPTO_SCE_BYTES_PER_WORD)),
                                                         remainder_len);

            status = crypto_ptr -> p_api -> zeroPaddingDecrypt(crypto_ptr -> p_ctrl,
                                                               (uint32_t *)(ctx -> nx_crypto_aes_key_schedule),
                                                               (uint32_t *)(ctx->nx_crypto_aes_iv_data),
                                                               remainder_len,
                                                               (uint32_t *)tmp_aes_array,
                                                               NULL);
            if (status)
            {
                return(NX_CRYPTO_NOT_SUCCESSFUL);
            }
        }
    }

    /* Decrypt data. */
    if((NX_CRYPTO_DECRYPT_UPDATE == op) || (NX_CRYPTO_DECRYPT == op))
    {
        /* input data - Little Endian to Big Endian conversion */
         num_blocks = input_length_in_byte / NX_CRYPTO_AES_BLOCK_SIZE;
         remainder_len =  input_length_in_byte - (num_blocks * NX_CRYPTO_AES_BLOCK_SIZE);
         tmp_length = input_length_in_byte - remainder_len;
         tmp = (uint32_t *)input;

         if (num_blocks)
         {
             for(i = 0; i < tmp_length / NX_CRYPTO_SCE_BYTES_PER_WORD; i++)
             {
                 tmp[i] = __REV(tmp[i]);
             }

            status = crypto_ptr -> p_api -> zeroPaddingDecrypt(crypto_ptr -> p_ctrl,
                                                               (uint32_t *)(ctx -> nx_crypto_aes_key_schedule),
                                                               (uint32_t *)(ctx->nx_crypto_aes_iv_data),
                                                               tmp_length,
                                                               (uint32_t *)input,
                                                               (uint32_t *)output);
            if (status)
            {
                return(NX_CRYPTO_NOT_SUCCESSFUL);
            }
         }

        if (0 != remainder_len)
        {
            private_copy_aes_gcm_block_bytes_to_word_be (tmp_aes_array,
                                                         (uint32_t *)(tmp + (tmp_length / NX_CRYPTO_SCE_BYTES_PER_WORD)),
                                                         remainder_len);

            status = crypto_ptr -> p_api -> zeroPaddingDecrypt(crypto_ptr -> p_ctrl,
                                                               (uint32_t *)(ctx -> nx_crypto_aes_key_schedule),
                                                               (uint32_t *)(ctx->nx_crypto_aes_iv_data),
                                                               remainder_len,
                                                               (uint32_t *)tmp_aes_array,
                                                               (uint32_t *)tmp_aes_result_array);
            if (status)
            {
                return(NX_CRYPTO_NOT_SUCCESSFUL);
            }
        }

        /* Output data - Big Endian to Little Endian conversion */
        if (num_blocks)
        {
             tmp = (uint32_t *)output;
             for(i = 0; i < input_length_in_byte / NX_CRYPTO_SCE_BYTES_PER_WORD; i++)
             {
                 tmp[i] = __REV(tmp[i]);
             }
        }
        private_copy_aes_gcm_be_word_block_to_le_bytes ((output + tmp_length), tmp_aes_result_array, remainder_len);
    }

    if((NX_CRYPTO_DECRYPT_CALCULATE == op) || (NX_CRYPTO_DECRYPT == op))
    {
        /* Set TAG. */
        memcpy (tmp_aes_array, (uint32_t *)tag, NX_CRYPTO_AES_GCM_TAG_LEN_IN_BYTES );

        /*
         * Convert to big endian format for HAL API.
         */
        for (i = 0; i < NX_CRYPTO_AES_GCM_TAG_LEN_IN_WORDS; i++)
        {
            tmp_aes_array[i] = __REV(tmp_aes_array[i]);
        }

        status = crypto_ptr ->p_api->setGcmTag(crypto_ptr -> p_ctrl, 4,  (uint32_t *)tmp_aes_array);
        if (status)
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }

        /* Verify additional authentication data. */
        status = crypto_ptr -> p_api -> zeroPaddingDecrypt(crypto_ptr -> p_ctrl,
                                                           (uint32_t *)(ctx -> nx_crypto_aes_key_schedule),
                                                           (uint32_t *)(ctx->nx_crypto_aes_iv_data),
                                                           0,
                                                           NULL,
                                                           NULL);

        if (status)
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }

    }

    return(NX_CRYPTO_SUCCESS);
}
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_aes_gcm_sce_operation             PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the AES GCM algorithm. The supported tag size is 16 bytes only.     */
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
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_aes_gcm_decrypt_sce        Decrypt data for AES GCM mode */
/*    _nx_crypto_aes_gcm_encrypt_sce        Encrypt data for AES GCM mode */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  07-15-2018     Timothy Stapko           Initial Version 5.11 SP1      */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
UINT _nx_crypto_method_aes_gcm_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
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
UINT i;
NX_CRYPTO_AES_SCE *ctx;
aes_instance_t *crypto_ptr;
UCHAR adjusted_tag[16];
UCHAR *adjusted_input;
UCHAR *adjusted_output;
ULONG actual_data_length;
uint32_t *iv_ptr_big_endian;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    if(key != NX_CRYPTO_NULL)
    {
        /* We only support 128-bit or 256-bit key size for the time-being. */
        if((key_size_in_bits != 128) && (key_size_in_bits != 192) && (key_size_in_bits != 256))
        {
            return(NX_CRYPTO_AES_UNSUPPORTED_KEY_SIZE);
        }
    }

    ctx = (NX_CRYPTO_AES_SCE*)crypto_metadata;

    if (method -> nx_crypto_algorithm != NX_CRYPTO_ENCRYPTION_AES_GCM_16)
    {
        return(NX_CRYPTO_INVALID_ALGORITHM);
    }

    if (method -> nx_crypto_ICV_size_in_bits != 128)
    {
        /* Only support 128 bits ICV length. */
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if (ctx -> nx_crypto_aes_key_size == 4)
    {
        crypto_ptr = (aes_instance_t *)&g_sce_aes_128_gcm;
    }
    else if (ctx -> nx_crypto_aes_key_size == 6)
    {
        crypto_ptr = (aes_instance_t *)&g_sce_aes_192_gcm;
    }
    else
    {
        crypto_ptr = (aes_instance_t *)&g_sce_aes_256_gcm;
    }

#if defined(__IAR_SYSTEMS_ICC__) && !defined(NX_CRYPTO_AES_DISABLE_ALIGNMENT)
    if ((UINT)input & 3)
    {

        /* Input is not 4 bytes aligned. */
        if (input_length_in_byte > NX_CRYPTO_AES_INPUT_BUFFER_SIZE)
        {

            /* Internal buffer is too small. */
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }

        adjusted_input = ctx -> nx_crypto_aes_input_buffer;
        NX_CRYPTO_MEMCPY(adjusted_input, input, input_length_in_byte);
    }
    else
#endif
    {
        adjusted_input = input;
    }

#if defined(__IAR_SYSTEMS_ICC__) && !defined(NX_CRYPTO_AES_DISABLE_ALIGNMENT)
    if ((UINT)output & 3)
    {

        /* Input is not 4 bytes aligned. */
        adjusted_output = ctx -> nx_crypto_aes_output_buffer;
    }
    else
#endif
    {
        adjusted_output = output;
    }

    if ((adjusted_input == input) && (adjusted_output == output))
    {

        /* Overlap check. */
        if (((LONG)(input - output) < (LONG)output_length_in_byte) &&
            ((LONG)(output - input) < (LONG)input_length_in_byte))
        {

            /* Input and Output buffer overlapped. */
            adjusted_output = ctx -> nx_crypto_aes_output_buffer;
        }
    }

    if((NX_CRYPTO_SET_ADDITIONAL_DATA == op) ||
       (NX_CRYPTO_ENCRYPT_INITIALIZE == op ) ||
       (NX_CRYPTO_DECRYPT_INITIALIZE == op))
    {
        /* Update the crypto meta data with the AAD and AAD data length. */
        ctx -> nx_crypto_aes_additional_data = adjusted_input;
        ctx -> nx_crypto_aes_additional_data_len = input_length_in_byte;
    }

    /* Data Initialization for each crypto operation. */
    switch(op)
    {
        case NX_CRYPTO_SET_ADDITIONAL_DATA:
        {
            return(NX_CRYPTO_SUCCESS);
        } break;
        case NX_CRYPTO_ENCRYPT_INITIALIZE:
        case NX_CRYPTO_DECRYPT_INITIALIZE:
        case NX_CRYPTO_ENCRYPT:
        case NX_CRYPTO_DECRYPT:
        {
            /* Check for a valid iv pointer during initialization. */
            if (NX_CRYPTO_NULL == iv_ptr)
            {
                return(NX_CRYPTO_PTR_ERROR);
            }

            /* Open the crypto HW module here. */
            crypto_ptr -> p_api -> open(crypto_ptr -> p_ctrl, crypto_ptr -> p_cfg);

            if (iv_ptr[0] != 12)
            {
                /* Only support 96 bits IV length. */
                return(NX_CRYPTO_NOT_SUCCESSFUL);
            }

            /* Store the iv data in the crypto metadata for further encryption process. */
            NX_CRYPTO_MEMCPY(ctx->nx_crypto_aes_iv_data, iv_ptr + 1, 12);
            ctx -> nx_crypto_aes_iv_data[12] = 0;
            ctx -> nx_crypto_aes_iv_data[13] = 0;
            ctx -> nx_crypto_aes_iv_data[14] = 0;
            ctx -> nx_crypto_aes_iv_data[15] = 1;

            /* Little Endian to Big Endian conversion of IV */
            if (CRYPTO_WORD_ENDIAN_BIG == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
            {
                iv_ptr_big_endian = (uint32_t*)(ctx ->nx_crypto_aes_iv_data);

                for(i = 0; i < NX_CRYPTO_AES_IV_LEN_IN_WORDS; i++)
                {
                    iv_ptr_big_endian[i] = __REV(iv_ptr_big_endian[i]);
                }
            }

        } break;
        case NX_CRYPTO_DECRYPT_CALCULATE:
        {
            /* Copy the tag value from input for verification. */
            NX_CRYPTO_MEMCPY(adjusted_tag, input, NX_CRYPTO_AES_GCM_TAG_LEN_IN_BYTES);
        } break;
        default:
        {
            /*Do Nothing. */
        } break;
    }

    /* Extract the length of the data to be encrypted/decrypted from total input packet length provided. */
    if(NX_CRYPTO_DECRYPT == op)
    {
        actual_data_length = input_length_in_byte - NX_CRYPTO_AES_GCM_TAG_LEN_IN_BYTES;
        /* Extract the tag from the input data buffer. */
        NX_CRYPTO_MEMCPY(adjusted_tag, input + actual_data_length, NX_CRYPTO_AES_GCM_TAG_LEN_IN_BYTES);
    }
    else
    {
        actual_data_length = input_length_in_byte;
    }


    if ((actual_data_length > NX_CRYPTO_AES_OUTPUT_BUFFER_SIZE) &&
        (adjusted_output != output))
    {
        /* Internal buffer is too small. */
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    /* Check for the requested operation and trigger the HW crypto routines. */
    if((NX_CRYPTO_ENCRYPT_INITIALIZE == op) ||
       (NX_CRYPTO_ENCRYPT_UPDATE == op) ||
       (NX_CRYPTO_ENCRYPT_CALCULATE == op) ||
       (NX_CRYPTO_ENCRYPT == op))
    {
        if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
        {
            status = _nx_crypto_aes_gcm_encrypt_sce(op,
                                                    crypto_ptr,
                                                    ctx,
                                                    adjusted_input,
                                                    actual_data_length,
                                                    adjusted_output,
                                                    adjusted_tag);
        }
        else
        {
            status = _nx_crypto_aes_gcm_encrypt_sce_be(op,
                                                       crypto_ptr,
                                                       ctx,
                                                       adjusted_input,
                                                       actual_data_length,
                                                       adjusted_output,
                                                       adjusted_tag);
        }

    }
    else if((NX_CRYPTO_DECRYPT_INITIALIZE == op) ||
            (NX_CRYPTO_DECRYPT_UPDATE == op) ||
            (NX_CRYPTO_DECRYPT_CALCULATE == op) ||
            (NX_CRYPTO_DECRYPT == op))
    {
        if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
        {
            status = _nx_crypto_aes_gcm_decrypt_sce(op,
                                                    crypto_ptr,
                                                    ctx,
                                                    adjusted_input,
                                                    actual_data_length,
                                                    adjusted_output,
                                                    adjusted_tag);
        }
        else
        {
            status = _nx_crypto_aes_gcm_decrypt_sce_be(op,
                                                       crypto_ptr,
                                                       ctx,
                                                       adjusted_input,
                                                       actual_data_length,
                                                       adjusted_output,
                                                       adjusted_tag);
        }
    }
    else
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    /* Update the encrypted/decrypted data in the output buffer in specific format, considering the
     *  operation requested. */
    if(NX_CRYPTO_SUCCESS == status)
    {

        if((NX_CRYPTO_ENCRYPT_UPDATE == op) || (NX_CRYPTO_ENCRYPT == op) ||
           (NX_CRYPTO_DECRYPT_UPDATE == op) || (NX_CRYPTO_DECRYPT == op))
        {
            if(output != adjusted_output)
            {
                /* Update the output data buffer with the encrypted/decrypted data. */
                NX_CRYPTO_MEMCPY(output, adjusted_output, actual_data_length);
            }

            if(NX_CRYPTO_ENCRYPT == op)
            {
                /* In case of encryption, append the tag at the end of the output buffer.  */
                NX_CRYPTO_MEMCPY(output + actual_data_length, adjusted_tag, NX_CRYPTO_AES_GCM_TAG_LEN_IN_BYTES);
            }
        }
        else if(NX_CRYPTO_ENCRYPT_CALCULATE == op)
        {
            /* Update the output buffer with the tag. */
            NX_CRYPTO_MEMCPY(output, adjusted_tag, NX_CRYPTO_AES_GCM_TAG_LEN_IN_BYTES);
        }
        else
        {
            /* Do Nothing. */
        }

        /* Close the HW crypto module here, only after a complete encrypt/decrypt process
         *  or after tag calculation/verification. */
        if((NX_CRYPTO_DECRYPT_CALCULATE == op) || (NX_CRYPTO_DECRYPT == op) ||
           (NX_CRYPTO_ENCRYPT_CALCULATE == op) || (NX_CRYPTO_ENCRYPT == op))
        {
            crypto_ptr -> p_api -> close(crypto_ptr -> p_ctrl);
        }
    }

    return(status);
}
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_aes_cbc_sce_operation             PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the AES CBC algorithm.                                        .     */
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
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*     nx_aes_key_set_sce                   Sets the Encryption Key       */
/*    nx_aes_cbc_decrypt_sce            Decrypt data for AES CBC mode     */
/*    nx_aes_cbc_encrypt_sce            Encrypt data for AES CBC mode     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_crypto_method_aes_sce_operation                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  07-15-2018     Timothy Stapko           Initial Version 5.11 SP1      */
/*                                                                        */
/**************************************************************************/
UINT _nx_crypto_method_aes_cbc_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
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
NX_CRYPTO_AES_SCE *ctx;
uint32_t *iv_ptr_big_endian;
UINT i;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    if(key != NX_NULL)
    {
        /* We only support 128-bit or 256-bit key size for the time-being. */
        if((key_size_in_bits != 128) && (key_size_in_bits != 192) && (key_size_in_bits != 256))
            return(NX_CRYPTO_AES_UNSUPPORTED_KEY_SIZE);
    }

    ctx = (NX_CRYPTO_AES_SCE*)crypto_metadata;

    /* Update the iv data to crypto metadata area only as a part of initialization or a complete
     * encryption/decryption process. */
    if((NX_CRYPTO_ENCRYPT_INITIALIZE == op) || (NX_CRYPTO_ENCRYPT == op) ||
       (NX_CRYPTO_DECRYPT_INITIALIZE == op) || (NX_CRYPTO_DECRYPT == op))
    {
        if(NX_CRYPTO_NULL == iv_ptr)
        {
            return(NX_CRYPTO_PTR_ERROR);
        }

        NX_CRYPTO_MEMCPY(ctx ->nx_crypto_aes_iv_data, iv_ptr, NX_CRYPTO_AES_IV_LEN_IN_BYTES);

        /* Little Endian to Big Endian conversion of IV */
        if (CRYPTO_WORD_ENDIAN_BIG == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
        {
            iv_ptr_big_endian = (uint32_t*)(ctx ->nx_crypto_aes_iv_data);

            for(i = 0; i < NX_CRYPTO_AES_IV_LEN_IN_WORDS; i++)
            {
                iv_ptr_big_endian[i] = __REV(iv_ptr_big_endian[i]);
            }
        }

        /* Encryption/Decryption initialization process ends here, and the function returns. */
        if((NX_CRYPTO_ENCRYPT != op) && (NX_CRYPTO_DECRYPT != op))
        {
            return(NX_CRYPTO_SUCCESS);
        }
    }

    switch(op)
    {
        case NX_CRYPTO_ENCRYPT_UPDATE:
            /* fall through */
        case NX_CRYPTO_ENCRYPT:
        {
            return(nx_aes_cbc_encrypt_sce(ctx, input_length_in_byte, input, output));
        } break;

        case NX_CRYPTO_DECRYPT_UPDATE:
            /* fall through */
        case NX_CRYPTO_DECRYPT:
        {
            return(nx_aes_cbc_decrypt_sce(ctx, input_length_in_byte, input, output));
        } break;
        case NX_CRYPTO_ENCRYPT_CALCULATE:
        case NX_CRYPTO_DECRYPT_CALCULATE:
        {
            /*Do Nothing.*/
            return(NX_CRYPTO_SUCCESS);
        } break;

        default:
        {
            return (NX_CRYPTO_NOT_SUCCESSFUL);
        } break;
    }

    //return(NX_CRYPTO_SUCCESS);
}
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_aes_sce_operation                 PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts and decrypts a message using                 */
/*    the AES algorithm.                                                  */
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
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_aes_key_set_sce                    Set AES key                   */
/*    nx_aes_cbc_decrypt_sce                Decrypt data for AES CBC mode */
/*    nx_aes_cbc_encrypt_sce                Encrypt data for AES CBC mode */
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
/*                                            supported GCM mode,         */
/*                                            resulting in version 5.11SP1*/
/*                                                                        */
/**************************************************************************/
UINT  _nx_crypto_method_aes_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
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

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    /* Check if the algorithm is cbc. */
    if (method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_CBC)
    {
        return(_nx_crypto_method_aes_cbc_sce_operation(op,
                                                       handle,
                                                       method,
                                                       key,
                                                       key_size_in_bits,
                                                       input,
                                                       input_length_in_byte,
                                                       iv_ptr,
                                                       output,
                                                       output_length_in_byte,
                                                       crypto_metadata,
                                                       crypto_metadata_size,
                                                       packet_ptr,
                                                       nx_crypto_hw_process_callback));
    }

#ifdef    NX_SECURE_ENABLE_AEAD_CIPHER
    else if ((method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_GCM_8) ||
             (method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_GCM_12) ||
             (method -> nx_crypto_algorithm == NX_CRYPTO_ENCRYPTION_AES_GCM_16))
    {
        return(_nx_crypto_method_aes_gcm_sce_operation(op,
                                                       handle,
                                                       method,
                                                       key,
                                                       key_size_in_bits,
                                                       input,
                                                       input_length_in_byte,
                                                       iv_ptr,
                                                       output,
                                                       output_length_in_byte,
                                                       crypto_metadata,
                                                       crypto_metadata_size,
                                                       packet_ptr,
                                                       nx_crypto_hw_process_callback));
    }
#endif
    else
    {
        return(NX_CRYPTO_INVALID_ALGORITHM);
    }


    //return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_aes_sce_init                      PORTABLE C      */
/*                                                           5.11         */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the AES crypto module.                    */
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
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_aes_key_set_sce                    Set AES key                   */
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
UINT  _nx_crypto_method_aes_sce_init(struct NX_CRYPTO_METHOD_STRUCT* method,
                                     UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                     VOID **handle,
                                     VOID *crypto_metadata,
                                     ULONG crypto_metadata_size)
{
NX_CRYPTO_AES_SCE *ctx;


    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);

    if(key != NX_CRYPTO_NULL)
    {
        ctx = (NX_CRYPTO_AES_SCE*)crypto_metadata;
        /* We only support 128-bit or 256-bit key size for the time-being. */
        if((key_size_in_bits != 128) && (key_size_in_bits != 192) && (key_size_in_bits != 256))
            return(NX_CRYPTO_AES_UNSUPPORTED_KEY_SIZE);

        nx_aes_key_set_sce(ctx, NX_CRYPTO_DECRYPT, key, (UCHAR)(key_size_in_bits >> 5));
    }

    return(NX_CRYPTO_SUCCESS);
}
