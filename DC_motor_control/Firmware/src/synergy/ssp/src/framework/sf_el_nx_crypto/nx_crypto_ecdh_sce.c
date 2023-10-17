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
/**   Elliptic-curve Diffie-Hellman (ECDH)                                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_ecdh_sce.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecdh_key_pair_import_sce                 PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up a Elliptic-curve Diffie-Hellman context by    */
/*    importing a local key pair.                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecdh_ptr                              ECDH context                  */
/*    local_private_key_ptr                 Pointer to local private key  */
/*    local_private_key_len                 Local private key length      */
/*    local_public_key_ptr                  Pointer to local public key   */
/*    local_public_key_len                  Remote public key length      */
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
UINT _nx_crypto_ecdh_key_pair_import_sce(NX_CRYPTO_ECDH_SCE  *ecdh_ptr,
                                         UCHAR  *local_private_key_ptr,
                                         ULONG   local_private_key_len,
                                         UCHAR  *local_public_key_ptr,
                                         ULONG   local_public_key_len)
{
UINT public_key_len;
const NX_CRYPTO_EC_SCE *ecc_ptr = ecdh_ptr -> nx_crypto_ecdh_curve;

    NX_CRYPTO_PARAMETER_NOT_USED(local_public_key_ptr);

    if (local_private_key_len > sizeof(ecdh_ptr -> nx_crypto_ecdh_private_key))
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    public_key_len = 1 + (ecc_ptr -> nx_crypto_ecc_key_size << 1);
    if (local_public_key_len > public_key_len)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Clear the private key buffer. */
    NX_CRYPTO_MEMSET(ecdh_ptr -> nx_crypto_ecdh_private_key, 0,
                     sizeof(ecdh_ptr -> nx_crypto_ecdh_private_key));

    /* Assign the desired key size based on the chosen elliptic curve. */
    ecdh_ptr -> nx_crypto_ecdh_key_size = ecc_ptr -> nx_crypto_ecc_key_size;

    /* Copy the shared key. */
    NX_CRYPTO_MEMCPY(ecdh_ptr -> nx_crypto_ecdh_private_key, local_private_key_ptr,
                     local_private_key_len);

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecdh_private_key_export_sce              PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function exports the local private key in Elliptic-curve       */
/*    Diffie-Hellman context.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecdh_ptr                              ECDH context                  */
/*    local_private_key_ptr                 Pointer to local private key  */
/*    local_private_key_len                 Local private key length      */
/*    actual_local_private_key_len          Pointer to private key length */
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
UINT _nx_crypto_ecdh_private_key_export_sce(NX_CRYPTO_ECDH_SCE  *ecdh_ptr,
                                            UCHAR  *local_private_key_ptr,
                                            ULONG   local_private_key_len,
                                            ULONG  *actual_local_private_key_len)
{

    /* Make sure the key size was assigned before we do anything else. Generally, this means
       _nx_crypto_ecdh_setup was not called to set up the NX_CRYPTO_ECDH structure prior to this call.  */
    if (ecdh_ptr -> nx_crypto_ecdh_key_size == 0)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Check to make sure the buffer is large enough to hold the private key. */
    if (local_private_key_len < ecdh_ptr -> nx_crypto_ecdh_key_size)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Copy the private key into the return buffer. */
    NX_CRYPTO_MEMCPY(local_private_key_ptr, ecdh_ptr -> nx_crypto_ecdh_private_key,
                     ecdh_ptr -> nx_crypto_ecdh_key_size);

    *actual_local_private_key_len = ecdh_ptr -> nx_crypto_ecdh_key_size;

    return(NX_CRYPTO_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecdh_setup_sce                           PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up a Elliptic-curve Diffie-Hellman context by    */
/*    generating a local key pair.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecdh_ptr                              ECDH context                  */
/*    share_secret_key_ptr                  Shared secret buffer pointer  */
/*    share_secret_key_len_ptr              Length of shared secret       */
/*    local_public_key_ptr                  Pointer to local public key   */
/*    local_public_key_len                  Remote public key length      */
/*    scratch_buf_ptr                       Pointer to scratch buffer,    */
/*                                            which cannot be smaller     */
/*                                            than 6 times of the key     */
/*                                            size (in bytes). This       */
/*                                            scratch buffer can be       */
/*                                            reused after this function  */
/*                                            returns.                    */
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
UINT _nx_crypto_ecdh_setup_sce(NX_CRYPTO_ECDH_SCE *ecdh_ptr,
                               UCHAR *local_public_key_ptr,
                               ULONG local_public_key_len,
                               ULONG *actual_local_public_key_len)
{
UINT public_key_len;

const NX_CRYPTO_EC_SCE *ecc_ptr = ecdh_ptr -> nx_crypto_ecdh_curve;
const ecc_instance_t *instance_ptr = ecc_ptr -> nx_crypto_ecc_instance;
const ecc_ctrl_t *control_ptr = ecc_ptr -> nx_crypto_ecc_control;
r_crypto_data_handle_t private_key, public_key;

    /* Check buffer size. */
    public_key_len = 1 + (ecc_ptr -> nx_crypto_ecc_key_size << 1);
    if (local_public_key_len < public_key_len)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Clear the private key buffer. */
    NX_CRYPTO_MEMSET(ecdh_ptr -> nx_crypto_ecdh_private_key, 0,
                     sizeof(ecdh_ptr -> nx_crypto_ecdh_private_key));

    /* Setup crypto data. */
    private_key.p_data = (uint32_t *)ecdh_ptr -> nx_crypto_ecdh_private_key;
    private_key.data_length = sizeof(ecdh_ptr -> nx_crypto_ecdh_private_key) >> 2;
    public_key.p_data = (uint32_t *)ecdh_ptr -> nx_crypto_ecdh_public_key;
    public_key.data_length = sizeof(ecdh_ptr -> nx_crypto_ecdh_public_key);

    /* Generate key pair. */
    if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
    {
        if (instance_ptr -> p_api -> keyCreate((ecc_ctrl_t *)control_ptr,
                                               (r_crypto_data_handle_t *)ecc_ptr -> nx_crypto_ecc_domain_with_order,
                                               (r_crypto_data_handle_t *)ecc_ptr -> nx_crypto_ecc_g,
                                               &private_key,
                                               &public_key))
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }

    }
    else
    {
        uint32_t i = 0;

        if (instance_ptr -> p_api -> keyCreate((ecc_ctrl_t *)control_ptr,
                                                 (r_crypto_data_handle_t *)ecc_ptr -> nx_crypto_ecc_domain_with_order,
                                                 (r_crypto_data_handle_t *)ecc_ptr -> nx_crypto_ecc_g,
                                                 &private_key,
                                                 &public_key))
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL);
        }
        /*
         *  Store the keys in little endian format for going up through the stack
         */
        for (i = 0; i < private_key.data_length; i++)
        {
            private_key.p_data[i] = __REV(private_key.p_data[i]);
        }
        for (i = 0; i < public_key.data_length; i++)
        {
            public_key.p_data[i] = __REV(public_key.p_data[i]);
        }

    }

    /* Setup the actual key size. */
    ecdh_ptr -> nx_crypto_ecdh_key_size = ecc_ptr -> nx_crypto_ecc_key_size;

    /* Copy the public key. */
    local_public_key_ptr[0] = 0x04;
    NX_CRYPTO_MEMCPY(&local_public_key_ptr[1], ecdh_ptr -> nx_crypto_ecdh_public_key, public_key_len - 1);
    *actual_local_public_key_len = public_key_len;
    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_ecdh_compute_secret_sce                  PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function computes the Elliptic-curve Diffie-Hellman shared     */
/*    secret using an existing Elliptic-curve Diffie-Hellman context      */
/*    and a public key received from a remote entity.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ecdh_ptr                              ECDH context                  */
/*    share_secret_key_ptr                  Shared secret buffer pointer  */
/*    share_secret_key_len_ptr              Length of shared secret       */
/*    remote_public_key                     Pointer to remote public key  */
/*    remote_public_key_len                 Remote public key length      */
/*    scratch_buf_ptr                       Pointer to scratch buffer,    */
/*                                            which cannot be smaller     */
/*                                            than 8 times of the key     */
/*                                            size (in bytes). This       */
/*                                            scratch buffer can be       */
/*                                            reused after this function  */
/*                                            returns.                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_crypto_huge_number_extract        Extract huge number           */
/*    _nx_crypto_huge_number_setup          Setup huge number             */
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
UINT _nx_crypto_ecdh_compute_secret_sce(NX_CRYPTO_ECDH_SCE *ecdh_ptr,
                                        UCHAR *share_secret_key_ptr,
                                        ULONG share_secret_key_len,
                                        ULONG *actual_share_secret_key_len,
                                        UCHAR *remote_public_key,
                                        ULONG remote_public_key_len)
{
const NX_CRYPTO_EC_SCE *ecc_ptr = ecdh_ptr -> nx_crypto_ecdh_curve;
const ecc_instance_t *instance_ptr = ecc_ptr -> nx_crypto_ecc_instance;
const ecc_ctrl_t *control_ptr = ecc_ptr -> nx_crypto_ecc_control;
r_crypto_data_handle_t private_key, public_key, shared_secret;


    /* Make sure the remote public key is small enough to fit into the huge number buffer. */
    if (remote_public_key_len > 1 + (ecdh_ptr -> nx_crypto_ecdh_key_size << 1))
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Check to make sure the buffer is large enough to hold the shared secret key. */
    if (share_secret_key_len < ecdh_ptr -> nx_crypto_ecdh_key_size)
    {
        return(NX_CRYPTO_SIZE_ERROR);
    }

    /* Setup crypto data. */
    NX_CRYPTO_MEMCPY(ecdh_ptr -> nx_crypto_ecdh_public_key, &remote_public_key[1], remote_public_key_len - 1);
    private_key.p_data = (uint32_t *)ecdh_ptr -> nx_crypto_ecdh_private_key;
    private_key.data_length = ecdh_ptr -> nx_crypto_ecdh_key_size >> 2;
    public_key.p_data = (uint32_t *)ecdh_ptr -> nx_crypto_ecdh_public_key;
    public_key.data_length = (remote_public_key_len - 1) >> 2;
    shared_secret.p_data = (uint32_t *)ecdh_ptr -> nx_crypto_ecdh_shared_key;
    shared_secret.data_length = share_secret_key_len >> 2;

    if (CRYPTO_WORD_ENDIAN_LITTLE == p_lower_lvl_hw_crypto->p_cfg->endian_flag)
    {
        if (instance_ptr -> p_api -> scalarMultiplication((ecc_ctrl_t *)control_ptr,
                                                          (r_crypto_data_handle_t *)ecc_ptr -> nx_crypto_ecc_domain_without_order,
                                                          &private_key,
                                                          &public_key,
                                                          &shared_secret))
        {
            return(NX_CRYPTO_NOT_SUCCESSFUL );
        }
    }
    else
    {
        ssp_err_t err = SSP_SUCCESS;
        uint32_t i = 0;

        /*
         *  Convert the keys into big endian format for HAL API
         */
        for (i = 0; i < private_key.data_length; i++)
        {
            private_key.p_data[i] = __REV(private_key.p_data[i]);
        }
        for (i = 0; i < public_key.data_length; i++)
        {
            public_key.p_data[i] = __REV(public_key.p_data[i]);
        }

        err = instance_ptr -> p_api -> scalarMultiplication((ecc_ctrl_t *)control_ptr,
                                                            (r_crypto_data_handle_t *)ecc_ptr -> nx_crypto_ecc_domain_without_order,
                                                            &private_key,
                                                            &public_key,
                                                            &shared_secret);
        /*
         *  Store the keys back in little endian format for going up through the stack
         */
        for (i = 0; i < private_key.data_length; i++)
        {
            private_key.p_data[i] = __REV(private_key.p_data[i]);
        }
        for (i = 0; i < public_key.data_length; i++)
        {
            public_key.p_data[i] = __REV(public_key.p_data[i]);
        }

        if (SSP_SUCCESS != err)

        {
            return(NX_CRYPTO_NOT_SUCCESSFUL );
        }
        /*
         *  Scalar multiplication API is successful.
         *  Store the shared_secret back in little endian format for going up through the stack
         */
        for (i = 0; i < shared_secret.data_length; i++)
        {
            shared_secret.p_data[i] = __REV(shared_secret.p_data[i]);
        }
    }

    NX_CRYPTO_MEMCPY(share_secret_key_ptr, ecdh_ptr -> nx_crypto_ecdh_shared_key,
                     ecdh_ptr -> nx_crypto_ecdh_key_size);
    *actual_share_secret_key_len = ecdh_ptr -> nx_crypto_ecdh_key_size;

    return(NX_CRYPTO_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_method_ecdh_sce_operation                PORTABLE C      */
/*                                                           5.11 SP1     */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the ECDH operation function for crypto method.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    op                                    Operation                     */
/*    handle                                Handle to method              */
/*    method                                Pointer to ECDH crypto method */
/*    key                                   Exponent of ECDH operation    */
/*    key_size_in_bits                      Size of exponent in bits      */
/*    input                                 Input stream                  */
/*    input_length_in_byte                  Length of input in byte       */
/*    iv_ptr                                Initial Vector (not used)     */
/*    output                                Output stream                 */
/*    output_length_in_byte                 Length of output in byte      */
/*    crypto_metadata                       Pointer to ECDH context       */
/*    crypto_metadata_size                  Size of ECDH context          */
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
/*  07-15-2018     Timothy Stapko           Initial Version 5.11 SP1      */
/*                                                                        */
/**************************************************************************/
UINT  _nx_crypto_method_ecdh_sce_operation(UINT op, /* Encrypt, Decrypt, Authenticate */
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
NX_CRYPTO_ECDH_SCE *ecdh;
UINT                status = NX_CRYPTO_SUCCESS;
NX_CRYPTO_EXTENDED_OUTPUT
                   *extended_output;

NX_CRYPTO_PARAMETER_NOT_USED(method);
NX_CRYPTO_PARAMETER_NOT_USED(handle);
NX_CRYPTO_PARAMETER_NOT_USED(key);
NX_CRYPTO_PARAMETER_NOT_USED(key_size_in_bits);
NX_CRYPTO_PARAMETER_NOT_USED(iv_ptr);
NX_CRYPTO_PARAMETER_NOT_USED(output_length_in_byte);
NX_CRYPTO_PARAMETER_NOT_USED(crypto_metadata_size);
NX_CRYPTO_PARAMETER_NOT_USED(packet_ptr);
NX_CRYPTO_PARAMETER_NOT_USED(nx_crypto_hw_process_callback);

    ecdh = (NX_CRYPTO_ECDH_SCE *)crypto_metadata;

    if (op == NX_CRYPTO_EC_CURVE_SET)
    {

        /* Set curve to be used in the ECDH. */
        status = ((NX_CRYPTO_METHOD *)input) -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_GET,
        		                                                    NX_CRYPTO_NULL,
                                                                    (NX_CRYPTO_METHOD *)input,
																	NX_CRYPTO_NULL, 0,
																	NX_CRYPTO_NULL, 0,
																	NX_CRYPTO_NULL,
                                                                    (UCHAR *)&ecdh -> nx_crypto_ecdh_curve,
                                                                    sizeof(NX_CRYPTO_EC_SCE *),
																	NX_CRYPTO_NULL, 0,
																	NX_CRYPTO_NULL, NX_CRYPTO_NULL);

    }
    else if (op == NX_CRYPTO_DH_SETUP)
    {

        /* Setup local key pair. */
        extended_output = (NX_CRYPTO_EXTENDED_OUTPUT *)output;
        status = _nx_crypto_ecdh_setup_sce(ecdh,
                                           extended_output -> nx_crypto_extended_output_data,
                                           extended_output -> nx_crypto_extended_output_length_in_byte,
                                           &extended_output -> nx_crypto_extended_output_actual_size);
    }
    else if (op == NX_CRYPTO_DH_KEY_PAIR_IMPORT)
    {

        /* Import local key pair. */
        status = _nx_crypto_ecdh_key_pair_import_sce(ecdh,
                                                     key, (key_size_in_bits >> 3),
                                                     input, input_length_in_byte);
    }
    else if (op == NX_CRYPTO_DH_PRIVATE_KEY_EXPORT)
    {

        /* Export local private key. */
        extended_output = (NX_CRYPTO_EXTENDED_OUTPUT *)output;
        status = _nx_crypto_ecdh_private_key_export_sce(ecdh,
                                                        extended_output -> nx_crypto_extended_output_data,
                                                        extended_output -> nx_crypto_extended_output_length_in_byte,
                                                        &extended_output -> nx_crypto_extended_output_actual_size);
    }
    else if (op == NX_CRYPTO_DH_CALCULATE)
    {

        /* Compute shared secret. */
        extended_output = (NX_CRYPTO_EXTENDED_OUTPUT *)output;
        status = _nx_crypto_ecdh_compute_secret_sce(ecdh,
                                                    extended_output -> nx_crypto_extended_output_data,
                                                    extended_output -> nx_crypto_extended_output_length_in_byte,
                                                    &extended_output -> nx_crypto_extended_output_actual_size,
                                                    input,
                                                    input_length_in_byte);
    }
    else
    {
        status = NX_CRYPTO_INVALID_ALGORITHM;
    }

    return(status);
}

