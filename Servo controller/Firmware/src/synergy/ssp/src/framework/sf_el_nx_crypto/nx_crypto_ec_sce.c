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
/**   Elliptic Curve                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_ec_sce.h"

/* secp192r1 domain. */
static const UCHAR _nx_crypto_ec_secp192r1_sce_domain[] =
{
     /* a = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFE FFFFFFFF FFFFFFFC */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC,

    /* b = 64210519 E59C80E7 0FA7E9AB 72243049 FEB8DEEC C146B9B1 */
    0x64, 0x21, 0x05, 0x19, 0xE5, 0x9C, 0x80, 0xE7,
    0x0F, 0xA7, 0xE9, 0xAB, 0x72, 0x24, 0x30, 0x49,
    0xFE, 0xB8, 0xDE, 0xEC, 0xC1, 0x46, 0xB9, 0xB1,

    /* p = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFE FFFFFFFF FFFFFFFF */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* n = FFFFFFFF FFFFFFFF FFFFFFFF 99DEF836 146BC9B1 B4D22831 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x99, 0xDE, 0xF8, 0x36,
    0x14, 0x6B, 0xC9, 0xB1, 0xB4, 0xD2, 0x28, 0x31
};
static const UCHAR _nx_crypto_ec_secp192r1_sce_domain_be[] =
{
     /* a = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFE FFFFFFFF FFFFFFFC */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,

    /* b = 64210519 E59C80E7 0FA7E9AB 72243049 FEB8DEEC C146B9B1 */
    0x19, 0x05, 0x21, 0x64, 0xE7, 0x80, 0x9C, 0xE5,
    0xAB, 0xE9, 0xA7, 0x0F, 0x49, 0x30, 0x24, 0x72,
    0xEC, 0xDE, 0xB8, 0xFE, 0xB1, 0xB9, 0x46, 0xC1,

    /* p = FFFFFFFF FFFFFFFF FFFFFFFF FFFFFFFE FFFFFFFF FFFFFFFF */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* n = FFFFFFFF FFFFFFFF FFFFFFFF 99DEF836 146BC9B1 B4D22831 */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x36, 0xF8, 0xDE, 0x99,
    0xB1, 0xC9, 0x6B, 0x14, 0x31, 0x28, 0xD2, 0xB4
};
static const r_crypto_data_handle_t _nx_crypto_ec_secp192r1_sce_domain_with_order_handle =
{
    (uint32_t *)_nx_crypto_ec_secp192r1_sce_domain,
    sizeof(_nx_crypto_ec_secp192r1_sce_domain) >> 2
};
const r_crypto_data_handle_t _nx_crypto_ec_secp192r1_sce_domain_with_order_handle_be =
{
    (uint32_t *)_nx_crypto_ec_secp192r1_sce_domain_be,
    sizeof(_nx_crypto_ec_secp192r1_sce_domain_be) >> 2
};
static const r_crypto_data_handle_t _nx_crypto_ec_secp192r1_sce_domain_without_order_handle =
{
    (uint32_t *)_nx_crypto_ec_secp192r1_sce_domain,
    (sizeof(_nx_crypto_ec_secp192r1_sce_domain) >> 4) * 3
};
const r_crypto_data_handle_t _nx_crypto_ec_secp192r1_sce_domain_without_order_handle_be =
{
    (uint32_t *)_nx_crypto_ec_secp192r1_sce_domain_be,
    (sizeof(_nx_crypto_ec_secp192r1_sce_domain_be) >> 4) * 3
};
/* secp192r1 g. */
static const UCHAR _nx_crypto_ec_secp192r1_sce_g[] =
{
    /* G.x = 188DA80E B03090F6 7CBF20EB 43A18800 F4FF0AFD 82FF1012 */
    0x18, 0x8D, 0xA8, 0x0E, 0xB0, 0x30, 0x90, 0xF6,
    0x7C, 0xBF, 0x20, 0xEB, 0x43, 0xA1, 0x88, 0x00,
    0xF4, 0xFF, 0x0A, 0xFD, 0x82, 0xFF, 0x10, 0x12,

    /* G.y = 07192B95 FFC8DA78 631011ED 6B24CDD5 73F977A1 1E794811*/
    0x07, 0x19, 0x2B, 0x95, 0xFF, 0xC8, 0xDA, 0x78,
    0x63, 0x10, 0x11, 0xED, 0x6B, 0x24, 0xCD, 0xD5,
    0x73, 0xF9, 0x77, 0xA1, 0x1E, 0x79, 0x48, 0x11
};
static const UCHAR _nx_crypto_ec_secp192r1_sce_g_be[] =
{
    /* G.x = 188DA80E B03090F6 7CBF20EB 43A18800 F4FF0AFD 82FF1012 */
    0x0E, 0xA8, 0x8D, 0x18, 0xF6, 0x90, 0x30, 0xB0,
    0xEB, 0x20, 0xBF, 0x7C, 0x00, 0x88, 0xA1, 0x43,
    0xFD, 0x0A, 0xFF, 0xF4, 0x12, 0x10, 0xFF, 0x82,

    /* G.y = 07192B95 FFC8DA78 631011ED 6B24CDD5 73F977A1 1E794811*/
    0x95, 0x2B, 0x19, 0x07, 0x78, 0xDA, 0xC8, 0xFF,
    0xED, 0x11, 0x10, 0x63, 0xD5, 0xCD, 0x24, 0x6B,
    0xA1, 0x77, 0xF9, 0x73, 0x11, 0x48, 0x79, 0x1E
};
static const r_crypto_data_handle_t _nx_crypto_ec_secp192r1_sce_g_handle =
{
    (uint32_t *)_nx_crypto_ec_secp192r1_sce_g,
    sizeof(_nx_crypto_ec_secp192r1_sce_g) >> 2
};
static const r_crypto_data_handle_t _nx_crypto_ec_secp192r1_sce_g_handle_be =
{
    (uint32_t *)_nx_crypto_ec_secp192r1_sce_g_be,
    sizeof(_nx_crypto_ec_secp192r1_sce_g_be) >> 2
};
const NX_CRYPTO_EC_SCE _nx_crypto_ec_secp192r1_sce =
{
    &_nx_crypto_ec_secp192r1_sce_domain_with_order_handle,
    &_nx_crypto_ec_secp192r1_sce_domain_without_order_handle,
    &_nx_crypto_ec_secp192r1_sce_g_handle,
    NX_CRYPTO_EC_192_PRIVATE_KEY_SIZE_BYTES,
    &g_sce_ecc192,
    &g_sce_ecc_ctrl192
};
const NX_CRYPTO_EC_SCE _nx_crypto_ec_secp192r1_sce_be =
{
    &_nx_crypto_ec_secp192r1_sce_domain_with_order_handle_be,
    &_nx_crypto_ec_secp192r1_sce_domain_without_order_handle_be,
    &_nx_crypto_ec_secp192r1_sce_g_handle_be,
    NX_CRYPTO_EC_192_PRIVATE_KEY_SIZE_BYTES,
    &g_sce_ecc192,
    &g_sce_ecc_ctrl192
};

/* secp256r1 domain. */
static const UCHAR _nx_crypto_ec_secp256r1_sce_domain[] =
{
    /* a = FFFFFFFF 00000001 00000000 00000000 00000000 FFFFFFFF FFFFFFFF FFFFFFFC */
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC,

    /* b = 5AC635D8 AA3A93E7 B3EBBD55 769886BC 651D06B0 CC53B0F6 3BCE3C3E 27D2604B */
    0x5A, 0xC6, 0x35, 0xD8, 0xAA, 0x3A, 0x93, 0xE7,
    0xB3, 0xEB, 0xBD, 0x55, 0x76, 0x98, 0x86, 0xBC,
    0x65, 0x1D, 0x06, 0xB0, 0xCC, 0x53, 0xB0, 0xF6,
    0x3B, 0xCE, 0x3C, 0x3E, 0x27, 0xD2, 0x60, 0x4B,

    /* p = FFFFFFFF 00000001 00000000 00000000 00000000 FFFFFFFF FFFFFFFF FFFFFFFF */
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* n = FFFFFFFF 00000000 FFFFFFFF FFFFFFFF BCE6FAAD A7179E84 F3B9CAC2 FC632551 */
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xBC, 0xE6, 0xFA, 0xAD, 0xA7, 0x17, 0x9E, 0x84,
    0xF3, 0xB9, 0xCA, 0xC2, 0xFC, 0x63, 0x25, 0x51
};

static const UCHAR _nx_crypto_ec_secp256r1_sce_domain_be[] =
{
    /* a = FFFFFFFF 00000001 00000000 00000000 00000000 FFFFFFFF FFFFFFFF FFFFFFFC */
    0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xFF, 0xFF, 0xFF,

    /* b = 5AC635D8 AA3A93E7 B3EBBD55 769886BC 651D06B0 CC53B0F6 3BCE3C3E 27D2604B */
    0xD8, 0x35, 0xC6, 0x5A, 0xE7, 0x93, 0x3A, 0xAA,
    0x55, 0xBD, 0xEB, 0xB3, 0xBC, 0x86, 0x98, 0x76,
    0xB0, 0x06, 0x1D, 0x65, 0xF6, 0xB0, 0x53, 0xCC,
    0x3E, 0x3C, 0xCE, 0x3B, 0x4B, 0x60, 0xD2, 0x27,

    /* p = FFFFFFFF 00000001 00000000 00000000 00000000 FFFFFFFF FFFFFFFF FFFFFFFF */
    0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

    /* n = FFFFFFFF 00000000 FFFFFFFF FFFFFFFF BCE6FAAD A7179E84 F3B9CAC2 FC632551 */
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xAD, 0xFA, 0xE6, 0xBC, 0x84, 0x9E, 0x17, 0xA7,
    0xC2, 0xCA, 0xB9, 0xF3, 0x51, 0x25, 0x63, 0xFC
};
static const r_crypto_data_handle_t _nx_crypto_ec_secp256r1_sce_domain_with_order_handle =
{
    (uint32_t *)_nx_crypto_ec_secp256r1_sce_domain,
    sizeof(_nx_crypto_ec_secp256r1_sce_domain) >> 2
};
const r_crypto_data_handle_t _nx_crypto_ec_secp256r1_sce_domain_with_order_handle_be =
{
    (uint32_t *)_nx_crypto_ec_secp256r1_sce_domain_be,
    sizeof(_nx_crypto_ec_secp256r1_sce_domain_be) >> 2
};
static const r_crypto_data_handle_t _nx_crypto_ec_secp256r1_sce_domain_without_order_handle =
{
    (uint32_t *)_nx_crypto_ec_secp256r1_sce_domain,
    (sizeof(_nx_crypto_ec_secp256r1_sce_domain) >> 4) * 3
};

const r_crypto_data_handle_t _nx_crypto_ec_secp256r1_sce_domain_without_order_handle_be =
{
    (uint32_t *)_nx_crypto_ec_secp256r1_sce_domain_be,
    (sizeof(_nx_crypto_ec_secp256r1_sce_domain_be) >> 4) * 3
};
/* secp256r1 g. */
static const UCHAR _nx_crypto_ec_secp256r1_sce_g[] =
{
    /* G.x = 6B17D1F2 E12C4247 F8BCE6E5 63A440F2 77037D81 2DEB33A0 F4A13945 D898C296 */
    0x6B, 0x17, 0xD1, 0xF2, 0xE1, 0x2C, 0x42, 0x47,
    0xF8, 0xBC, 0xE6, 0xE5, 0x63, 0xA4, 0x40, 0xF2,
    0x77, 0x03, 0x7D, 0x81, 0x2D, 0xEB, 0x33, 0xA0,
    0xF4, 0xA1, 0x39, 0x45, 0xD8, 0x98, 0xC2, 0x96,

    /* G.y = 4FE342E2 FE1A7F9B 8EE7EB4A 7C0F9E16 2BCE3357 6B315ECE CBB64068 37BF51F5 */
    0x4F, 0xE3, 0x42, 0xE2, 0xFE, 0x1A, 0x7F, 0x9B,
    0x8E, 0xE7, 0xEB, 0x4A, 0x7C, 0x0F, 0x9E, 0x16,
    0x2B, 0xCE, 0x33, 0x57, 0x6B, 0x31, 0x5E, 0xCE,
    0xCB, 0xB6, 0x40, 0x68, 0x37, 0xBF, 0x51, 0xF5
};
static const UCHAR _nx_crypto_ec_secp256r1_sce_g_be[] =
{
    /* G.x = 6B17D1F2 E12C4247 F8BCE6E5 63A440F2 77037D81 2DEB33A0 F4A13945 D898C296 */
    0xF2, 0xD1, 0x17, 0x6B, 0x47, 0x42, 0x2C, 0xE1,
    0xE5, 0xE6, 0xBC, 0xF8, 0xF2, 0x40, 0xA4, 0x63,
    0x81, 0x7D, 0x03, 0x77, 0xA0, 0x33, 0xEB, 0x2D,
    0x45, 0x39, 0xA1, 0xF4, 0x96, 0xC2, 0x98, 0xD8,

    /* G.y = 4FE342E2 FE1A7F9B 8EE7EB4A 7C0F9E16 2BCE3357 6B315ECE CBB64068 37BF51F5 */
    0xE2, 0x42, 0xE3, 0x4F, 0x9B, 0x7F, 0x1A, 0xFE,
    0x4A, 0xEB, 0xE7, 0x8E, 0x16, 0x9E, 0x0F, 0x7C,
    0x57, 0x33, 0xCE, 0x2B, 0xCE, 0x5E, 0x31, 0x6B,
    0x68, 0x40, 0xB6, 0xCB, 0xF5, 0x51, 0xBF, 0x37
};
static const r_crypto_data_handle_t _nx_crypto_ec_secp256r1_sce_g_handle =
{
    (uint32_t *)_nx_crypto_ec_secp256r1_sce_g,
    sizeof(_nx_crypto_ec_secp256r1_sce_g) >> 2
};

const r_crypto_data_handle_t _nx_crypto_ec_secp256r1_sce_g_handle_be =
{
    (uint32_t *)_nx_crypto_ec_secp256r1_sce_g_be,
    sizeof(_nx_crypto_ec_secp256r1_sce_g_be) >> 2
};

const NX_CRYPTO_EC_SCE _nx_crypto_ec_secp256r1_sce =
{
    &_nx_crypto_ec_secp256r1_sce_domain_with_order_handle,
    &_nx_crypto_ec_secp256r1_sce_domain_without_order_handle,
    &_nx_crypto_ec_secp256r1_sce_g_handle,
    NX_CRYPTO_EC_256_PRIVATE_KEY_SIZE_BYTES,
    &g_sce_ecc256,
    &g_sce_ecc_ctrl256
};

const NX_CRYPTO_EC_SCE _nx_crypto_ec_secp256r1_sce_be =
{
    &_nx_crypto_ec_secp256r1_sce_domain_with_order_handle_be,
    &_nx_crypto_ec_secp256r1_sce_domain_without_order_handle_be,
    &_nx_crypto_ec_secp256r1_sce_g_handle_be,
    NX_CRYPTO_EC_256_PRIVATE_KEY_SIZE_BYTES,
    &g_sce_ecc256,
    &g_sce_ecc_ctrl256
};

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ec_secp192r1_sce_operation        PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the secp192r1 curve.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    Operation                     */
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
UINT _nx_crypto_method_ec_secp192r1_sce_operation(UINT op,
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
    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(input);
    NX_CRYPTO_PARAMETER_NOT_USED(input_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(output);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    if (op != NX_CRYPTO_EC_CURVE_GET)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
    {

        *((NX_CRYPTO_EC_SCE **)output) = (NX_CRYPTO_EC_SCE *)&_nx_crypto_ec_secp192r1_sce;
    }
    else
    {
        *((NX_CRYPTO_EC_SCE **)output) = (NX_CRYPTO_EC_SCE *)&_nx_crypto_ec_secp192r1_sce_be;
    }

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ec_secp256r1_sce_operation        PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the secp256r1 curve.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    Operation                     */
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
UINT _nx_crypto_method_ec_secp256r1_sce_operation(UINT op,
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
    NX_CRYPTO_PARAMETER_NOT_USED(handle);
    NX_CRYPTO_PARAMETER_NOT_USED(method);
    NX_CRYPTO_PARAMETER_NOT_USED(key);
    NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
    NX_CRYPTO_PARAMETER_NOT_USED(input);
    NX_CRYPTO_PARAMETER_NOT_USED(input_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(output);
    NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata);
    NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
    NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
    NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    if (op != NX_CRYPTO_EC_CURVE_GET)
    {
        return(NX_CRYPTO_NOT_SUCCESSFUL);
    }

    if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
    {
        *((NX_CRYPTO_EC_SCE **)output) = (NX_CRYPTO_EC_SCE *)&_nx_crypto_ec_secp256r1_sce;
    }
    else
    {
        *((NX_CRYPTO_EC_SCE **)output) = (NX_CRYPTO_EC_SCE *)&_nx_crypto_ec_secp256r1_sce_be;
    }

    return(NX_CRYPTO_SUCCESS);
}

