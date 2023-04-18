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
/**   Elliptic Curve Digital Signature Algorithm (ECDSA)                  */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_ecdsa_sce.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecdsa_sign_sce                           PORTABLE C      */
/*                                                          5.11 SP1      */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function computes a signature of the hash data using the       */
/*    private key.                                                        */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecdsa_ptr                             ECDSA context                 */
/*    hash                                  Hash data to be signed        */
/*    hash_length                           Length of hash data           */
/*    private_key                           Pointer to EC private key     */
/*    signature                             Pointer to signature output   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_ec_key_pair_generation_extra                             */
/*                                          Generate EC Key Pair          */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ecdsa_sign_sce(NX_CRYPTO_ECDSA_SCE *ecdsa_ptr, UCHAR *hash, UINT hash_length,
                                              UCHAR *private_key, UINT private_key_length,
                                              UCHAR *signature, ULONG signature_length,
                                              ULONG *actual_signature_length)
{
UINT                  i;
UINT                  curve_size;
UINT                  r_size;
UINT                  s_size;
UCHAR                 pad_zero_r;
UCHAR                 pad_zero_s;
UINT                  sequence_size;
UCHAR                *signature_r;
UCHAR                *signature_s;
const NX_CRYPTO_EC_SCE *ecc_ptr = ecdsa_ptr -> nx_crypto_ecdsa_curve;
const ecc_instance_t *instance_ptr = ecc_ptr -> nx_crypto_ecc_instance;
const ecc_ctrl_t *control_ptr = ecc_ptr -> nx_crypto_ecc_control;
r_crypto_data_handle_t privkey, sig_r, sig_s, digest;

    /* Signature format follows ASN1 DER encoding as per RFC 4492, section 5.8:
     * Size: 1   | 1 or 2 | 1   |   1   | 0 or 1 | N |  1  |  1   | 0 or 1 | M
     * Data: SEQ |  Size  | INT |  Size | 0x00   | r | INT | Size | 0x00   | s  */

    curve_size = ecc_ptr -> nx_crypto_ecc_key_size;

    /* Check the signature_length for worst case. */
    if (signature_length < ((curve_size << 1) + 9))
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    if (private_key_length > curve_size)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }


    /* Setup crypto data. */
    privkey.p_data = (uint32_t *)private_key;
    privkey.data_length = private_key_length >> 2;

    /* Truncate the hash data to the size of group order. */
    if (hash_length > curve_size)
    {
        hash_length = curve_size;
    }

    if (hash_length == curve_size)
    {
        digest.p_data = (uint32_t *)hash;
    }
    else
    {
        NX_CRYPTO_MEMSET(ecdsa_ptr -> nx_crypto_ecdsa_hash, 0, curve_size - hash_length);
        NX_CRYPTO_MEMCPY(&((UCHAR *)ecdsa_ptr -> nx_crypto_ecdsa_hash)[curve_size - hash_length], hash, hash_length);
        digest.p_data = (uint32_t *)ecdsa_ptr -> nx_crypto_ecdsa_hash;
    }
    digest.data_length = curve_size >> 2;


    /* Output r and s as two INTEGER in ASN.1 encoding */
    signature_r = signature + 3;
    sig_r.p_data = (uint32_t *)signature_r;
    sig_r.data_length = curve_size >> 2;
    signature_s = signature + (curve_size + 6);
    sig_s.p_data = (uint32_t *)signature_s;
    sig_s.data_length = curve_size >> 2;

    if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
    {
        if (SSP_SUCCESS != instance_ptr -> p_api -> sign((ecc_ctrl_t *)control_ptr,
                                                         (r_crypto_data_handle_t *)ecc_ptr -> nx_crypto_ecc_domain_with_order,
                                                         (r_crypto_data_handle_t *)ecc_ptr -> nx_crypto_ecc_g,
                                                         &privkey,
                                                         &digest,
                                                         &sig_r,
                                                         &sig_s))
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }
    }
    else
    {
        ssp_err_t err = SSP_SUCCESS;

        /*
         *  Convert the private key and digest into big endian format for HAL API
         */
        for (i = 0; i < privkey.data_length; i++)
        {
            privkey.p_data[i] = __REV(privkey.p_data[i]);
        }
        for (i = 0; i < digest.data_length; i++)
        {
            digest.p_data[i] = __REV(digest.p_data[i]);
        }

        err = instance_ptr -> p_api -> sign((ecc_ctrl_t *)control_ptr,
                                                         (r_crypto_data_handle_t *)ecc_ptr -> nx_crypto_ecc_domain_with_order,
                                                         (r_crypto_data_handle_t *)ecc_ptr -> nx_crypto_ecc_g,
                                                         &privkey,
                                                         &digest,
                                                         &sig_r,
                                                         &sig_s);
        /*
         *  Convert the private key and digest back into little endian format for the upper layers
         */
        for (i = 0; i < privkey.data_length; i++)
        {
            privkey.p_data[i] = __REV(privkey.p_data[i]);
        }
        for (i = 0; i < digest.data_length; i++)
        {
            digest.p_data[i] = __REV(digest.p_data[i]);
        }

        if (SSP_SUCCESS != err)
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }

        /*
         *  Sign API is successful.
         *  Convert the signature components into little endian format for the upper layers
         */

        for (i = 0; i < sig_r.data_length; i++)
        {
            sig_r.p_data[i] = __REV(sig_r.p_data[i]);
        }
        for (i = 0; i < sig_s.data_length; i++)
        {
            sig_s.p_data[i] = __REV(sig_s.p_data[i]);
        }
    }

    r_size = sig_r.data_length << 2;
    s_size = sig_s.data_length << 2;

    /* Trim prefix zeros. */
    for (i = 0; i < r_size; i++)
    {
        if (signature_r[i])
        {

            /* Loop until none zero byte. */
            break;
        }
    }
    signature_r += i;
    r_size -= i;

    /* The most significant bit must be zero to indicate positive integer. */
    /* Pad zero at the front if necessary. */
    pad_zero_r = (signature_r[0] & 0x80) ? 1 : 0;

    for (i = 0; i < s_size; i++)
    {
        if (signature_s[i])
        {

            /* Loop until none zero byte. */
            break;
        }
    }
    signature_s += i;
    s_size -= i;

    /* The most significant bit must be zero to indicate positive integer. */
    /* Pad zero at the front if necessary. */
    pad_zero_s = (signature_s[0] & 0x80) ? 1 : 0;

    /* Size of sequence. */
    sequence_size = r_size + pad_zero_r + s_size + pad_zero_s + 4;

    signature[0] = 0x30;    /* SEQUENCE */
    if (sequence_size < 0x80)
    {
        signature[1] = (UCHAR)sequence_size;
        signature += 2;
        *actual_signature_length = sequence_size + 2;
    }
    else
    {
        signature[1] = 0x81;
        signature[2] = (UCHAR)sequence_size;
        signature += 3;
        *actual_signature_length = sequence_size + 3;
    }

    /* Setup r. */
    NX_CRYPTO_MEMMOVE(&signature[2 + pad_zero_r], signature_r, r_size);
    signature[0] = 0x02;    /* Integer */
    signature[1] = (UCHAR)(r_size + pad_zero_r);
    if (pad_zero_r)
    {
        signature[2] = 0;
    }
    signature += (2u + pad_zero_r + r_size);

    /* Setup s. */
    NX_CRYPTO_MEMMOVE(&signature[2 + pad_zero_s], signature_s, s_size);
    signature[0] = 0x02;    /* Integer */
    signature[1] = (UCHAR)(s_size + pad_zero_s);
    if (pad_zero_s)
    {
        signature[2] = 0;
    }

    return NX_CRYPTO_SUCCESS;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecdsa_verify_sce                         PORTABLE C      */
/*                                                          5.11 SP1      */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function verifies the signature of the hash data using the     */
/*    public key.                                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecdsa_ptr                             ECDSA context                 */
/*    hash                                  Hash data to be verified      */
/*    hash_length                           Length of hash data           */
/*    public_key                            Pointer to EC public key      */
/*    signature                             Signature to be verified      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_setup          Generate private key          */
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
NX_CRYPTO_KEEP UINT _nx_crypto_ecdsa_verify_sce(NX_CRYPTO_ECDSA_SCE *ecdsa_ptr, UCHAR *hash, UINT hash_length,
                                                UCHAR *public_key, UINT public_key_length,
                                                UCHAR *signature, UINT signature_length)
{
UINT                  i;
UINT                  curve_size;
UINT                  rs_size;
const NX_CRYPTO_EC_SCE *ecc_ptr = ecdsa_ptr -> nx_crypto_ecdsa_curve;
const ecc_instance_t *instance_ptr = ecc_ptr -> nx_crypto_ecc_instance;
const ecc_ctrl_t *control_ptr = ecc_ptr -> nx_crypto_ecc_control;
r_crypto_data_handle_t pubkey, sig_r, sig_s, digest;


    /* Signature format follows ASN1 DER encoding as per RFC 4492, section 5.8:
     * Size: 1   | 1 or 2 | 1   |   1   | 0 or 1 | N |  1  |  1   | 0 or 1 | M
     * Data: SEQ |  Size  | INT |  Size | 0x00   | r | INT | Size | 0x00   | s  */

    /* FIXME: Add public key validity check. */

    curve_size = ecc_ptr -> nx_crypto_ecc_key_size;

    if (public_key_length != 1 + (curve_size << 1))
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Buffer should contain the signature data sequence. */
    if (signature[0] != 0x30)
    {
        return(NX_CRYPTO_AUTHENTICATION_FAILED);
    }

    /* Check the size in SEQUENCE.  */
    if (signature[1] & 0x80)
    {
        if (signature_length < (signature[2] + 3u))
        {
            return(NX_CRYPTO_SIZE_ERROR);
        }
        signature_length = signature[2];
        signature += 3;
    }
    else 
    {
        if (signature_length < (signature[1] + 2u))
        {
            return(NX_CRYPTO_SIZE_ERROR);
        }
        signature_length = signature[1];
        signature += 2;
    }


    /* Setup crypto data. */
    pubkey.p_data = (uint32_t *)(public_key + 1);
    pubkey.data_length = curve_size >> 1;

    if (signature_length < (signature[1] + 2u))
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Read r value from input signature. */
    rs_size = signature[1];
    if (rs_size >= curve_size)
    {
        sig_r.p_data = (uint32_t *)&signature[2 + rs_size - curve_size];
    }
    else
    {
        NX_CRYPTO_MEMSET(ecdsa_ptr -> nx_crypto_ecdsa_signature_r, 0, curve_size - rs_size);
        NX_CRYPTO_MEMCPY(&((UCHAR *)ecdsa_ptr -> nx_crypto_ecdsa_signature_r)[curve_size - rs_size], &signature[2], rs_size);
        sig_r.p_data = (uint32_t *)ecdsa_ptr -> nx_crypto_ecdsa_signature_r;
    }
    sig_r.data_length = curve_size >> 2;
    
    signature_length -= (signature[1] + 2u);
    signature += signature[1] + 2;

    if (signature_length < (signature[1] + 2u))
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Read s value from input signature. */
    rs_size = signature[1];
    if (rs_size >= curve_size)
    {
        sig_s.p_data = (uint32_t *)&signature[2 + rs_size - curve_size];
    }
    else
    {
        NX_CRYPTO_MEMSET(ecdsa_ptr -> nx_crypto_ecdsa_signature_s, 0, curve_size - rs_size);
        NX_CRYPTO_MEMCPY(&((UCHAR *)ecdsa_ptr -> nx_crypto_ecdsa_signature_s)[curve_size - rs_size], &signature[2], rs_size);
        sig_s.p_data = (uint32_t *)ecdsa_ptr -> nx_crypto_ecdsa_signature_s;
    }
    sig_s.data_length = curve_size >> 2;

    /* Truncate the hash data to the size of group order. */
    if (hash_length > curve_size)
    {
        hash_length = curve_size;
    }

    if (hash_length == curve_size)
    {
        digest.p_data = (uint32_t *)hash;
    }
    else
    {
        NX_CRYPTO_MEMSET(ecdsa_ptr -> nx_crypto_ecdsa_hash, 0, curve_size - hash_length);
        NX_CRYPTO_MEMCPY(&((UCHAR *)ecdsa_ptr -> nx_crypto_ecdsa_hash)[curve_size - hash_length], hash, hash_length);
        digest.p_data = (uint32_t *)ecdsa_ptr -> nx_crypto_ecdsa_hash;
    }
    digest.data_length = curve_size >> 2;

    if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
    {
        if (SSP_SUCCESS != instance_ptr -> p_api -> verify((ecc_ctrl_t *)control_ptr,
                                                           (r_crypto_data_handle_t *)ecc_ptr -> nx_crypto_ecc_domain_with_order,
                                                           (r_crypto_data_handle_t *)ecc_ptr -> nx_crypto_ecc_g,
                                                           &pubkey,
                                                           &digest,
                                                           &sig_r,
                                                           &sig_s))
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }
    }
    else
    {
        ssp_err_t err = SSP_SUCCESS;
        /*
         *  Convert the public key, digest and signature into big endian format for HAL API
         */
        for (i = 0; i < pubkey.data_length; i++)
        {
            pubkey.p_data[i] = __REV(pubkey.p_data[i]);
        }
        for (i = 0; i < digest.data_length; i++)
        {
            digest.p_data[i] = __REV(digest.p_data[i]);
        }
        for (i = 0; i < sig_r.data_length; i++)
        {
            sig_r.p_data[i] = __REV(sig_r.p_data[i]);
        }
        for (i = 0; i < sig_s.data_length; i++)
        {
            sig_s.p_data[i] = __REV(sig_s.p_data[i]);
        }

        err = instance_ptr -> p_api -> verify((ecc_ctrl_t *)control_ptr,
                                                           (r_crypto_data_handle_t *)ecc_ptr -> nx_crypto_ecc_domain_with_order,
                                                           (r_crypto_data_handle_t *)ecc_ptr -> nx_crypto_ecc_g,
                                                           &pubkey,
                                                           &digest,
                                                           &sig_r,
                                                           &sig_s);
        /*
         *  Convert the public key, digest and signature into little endian format for the upper layers
         */
        for (i = 0; i < pubkey.data_length; i++)
        {
            pubkey.p_data[i] = __REV(pubkey.p_data[i]);
        }
        for (i = 0; i < digest.data_length; i++)
        {
            digest.p_data[i] = __REV(digest.p_data[i]);
        }
        for (i = 0; i < sig_r.data_length; i++)
        {
            sig_r.p_data[i] = __REV(sig_r.p_data[i]);
        }
        for (i = 0; i < sig_s.data_length; i++)
        {
            sig_s.p_data[i] = __REV(sig_s.p_data[i]);
        }

        if (SSP_SUCCESS != err)
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }
    }

    return NX_CRYPTO_SUCCESS;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ecdsa_sce_init                    PORTABLE C      */
/*                                                          5.11 SP1      */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the common crypto method init callback for         */
/*    Express Logic supported ECDSA cryptograhic algorithm.               */
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
/*  07-15-2018     Timothy Stapko           Initial Version 5.11 SP1      */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT  _nx_crypto_method_ecdsa_sce_init(struct  NX_CRYPTO_METHOD_STRUCT *method,
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

    if(crypto_metadata_size < sizeof(NX_CRYPTO_ECDSA_SCE))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ecdsa_sc_cleanupe                 PORTABLE C      */
/*                                                          5.11 SP1      */
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
/*  07-15-2018     Timothy Stapko           Initial Version 5.11 SP1      */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT  _nx_crypto_method_ecdsa_sce_cleanup(VOID *crypto_metadata)
{

    NX_CRYPTO_STATE_CHECK

#ifdef NX_SECURE_KEY_CLEAR
    if (!crypto_metadata)
        return (NX_CRYPTO_SUCCESS);

    /* Clean up the crypto metadata.  */
    NX_CRYPTO_MEMSET(crypto_metadata, 0, sizeof(NX_CRYPTO_ECDSA_SCE));
#else
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
#endif/* NX_SECURE_KEY_CLEAR  */

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ecdsa_sce_operation               PORTABLE C      */
/*                                                          5.11 SP1      */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs an ECDSA operation.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    ECDSA operation               */
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
/*    _nx_crypto_ecdsa_sign_sce             Sign using ECDSA              */
/*    _nx_crypto_ecdsa_verify_sce           Verify ECDSA signature        */
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
NX_CRYPTO_KEEP UINT _nx_crypto_method_ecdsa_sce_operation(UINT op,
                                                          VOID *handle,
                                                          struct NX_CRYPTO_METHOD_STRUCT *method,
                                                          UCHAR *key, NX_CRYPTO_KEY_SIZE key_size_in_bits,
                                                          UCHAR *input, ULONG input_length_in_byte,
                                                          UCHAR *iv_ptr,
                                                          UCHAR *output, ULONG output_length_in_byte,
                                                          VOID *crypto_metadata, ULONG crypto_metadata_size,
                                                          VOID *packet_ptr,
                                                          VOID (*nx_crypto_hw_process_callback)(VOID *, UINT))
{
NX_CRYPTO_ECDSA_SCE *ecdsa;
UINT                 status = NX_CRYPTO_SUCCESS;
NX_CRYPTO_EXTENDED_OUTPUT
                    *extended_output;

    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    NX_CRYPTO_STATE_CHECK

    /* Verify the metadata addrsss is 4-byte aligned. */
    if((method == NX_CRYPTO_NULL) || (crypto_metadata == NX_CRYPTO_NULL) || ((((ULONG)crypto_metadata) & 0x3) != 0))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    if(crypto_metadata_size < sizeof(NX_CRYPTO_ECDSA_SCE))
    {
        return(NX_CRYPTO_PTR_ERROR);
    }

    ecdsa = (NX_CRYPTO_ECDSA_SCE *)crypto_metadata;

    if (op == NX_CRYPTO_EC_CURVE_SET)
    {
        /* Set curve to be used in the ECDSA. */
        status = ((NX_CRYPTO_METHOD *)input) -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_GET,
                                                                    NX_CRYPTO_NULL,
                                                                    (NX_CRYPTO_METHOD *)input,
                                                                    NX_CRYPTO_NULL, 0,
                                                                    NX_CRYPTO_NULL, 0,
                                                                    NX_CRYPTO_NULL,
                                                                    (UCHAR *)&ecdsa -> nx_crypto_ecdsa_curve,
                                                                    sizeof(NX_CRYPTO_EC_SCE *),
                                                                    NX_CRYPTO_NULL, 0,
                                                                    NX_CRYPTO_NULL, NX_CRYPTO_NULL);
    }
    else if (op == NX_CRYPTO_AUTHENTICATE)
    {
        if (key == NX_CRYPTO_NULL)
        {
            return(NX_CRYPTO_PTR_ERROR);
        }

        extended_output = (NX_CRYPTO_EXTENDED_OUTPUT *)output;

        status = _nx_crypto_ecdsa_sign_sce(ecdsa,
                                           input,
                                           input_length_in_byte,
                                           key,
                                           key_size_in_bits >> 3,
                                           extended_output -> nx_crypto_extended_output_data,
                                           extended_output -> nx_crypto_extended_output_length_in_byte,
                                           &extended_output -> nx_crypto_extended_output_actual_size);
    }
    else if (op == NX_CRYPTO_VERIFY)
    {
        if (key == NX_CRYPTO_NULL)
        {
            return(NX_CRYPTO_PTR_ERROR);
        }

        status = _nx_crypto_ecdsa_verify_sce(ecdsa,
                                             input,
                                             input_length_in_byte,
                                             key,
                                             key_size_in_bits >> 3,
                                             output, output_length_in_byte);
    }
    else
    {
        status = NX_CRYPTO_NOT_SUCCESSFUL;
    }

    return(status);
}
