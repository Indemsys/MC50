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

/**********************************************************************************************************************
 * File Name    : sf_crypto_hash.c
 * Description  : SSP Crypto HASH Framework Module
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * @file
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_crypto_hash.h"
#include "sf_crypto_private_api.h"
#include "sf_crypto_hash_private_api.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
/** HASH terminator bit pattern */
#define SF_CRYPTO_HASH_TERMINATOR                       (0x80U)

/** Size of HASH terminator */
#define SF_CRYPTO_HASH_TERMINATOR_SIZE                  (1U)

/** Message bit-length in bytes */
#define SF_CRYPTO_HASH_BIT_LENGTH_IN_BYTES              (8U)

/** Message Block size for each HASH algorithm in bytes */
#define SF_CRYPTO_HASH_BLOCK_SIZE_MD5_SHA1_SHA224_SHA256    (64U)

/** Number of bytes given as extra bytes for memory boundary alignment when allocating memory in the byte pool. */
#define SF_CRYPTO_HASH_ALIGNMENT_ADJUSTMENT_BYTES       (3U)

/** A macro to get the memory address aligned on WORD memory boundary. */
#define SF_CRYPTO_HASH_WORD_ALIGNMENT_MASK              (0x3U)

/* A macro for byte swap function */
#define MD5_DIGEST_SIZE_IN_WORDS                         (4U)

#define MD5_INITIAL_VALUE_LENGTH_IN_BYTES                 (16U)
#define SHA1_INITIAL_VALUE_LENGTH_IN_BYTES                (20U)
#define SHA256_INITIAL_VALUE_LENGTH_IN_BYTES              (32U)
#define SHA224_INITIAL_VALUE_LENGTH_IN_BYTES              (32U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
#if SF_CRYPTO_HASH_CFG_PARAM_CHECKING_ENABLE
static ssp_err_t sf_crypto_hash_open_param_check (sf_crypto_hash_instance_ctrl_t * const p_ctrl,
                                                 sf_crypto_hash_cfg_t const * const p_cfg);

static ssp_err_t sf_crypto_hash_message_digest_final_param_check (sf_crypto_hash_instance_ctrl_t * const p_ctrl,
                                                 sf_crypto_data_handle_t * const p_msg_digest, uint32_t * p_size);
#endif
static bool      sf_crypto_hash_module_open_status_check (sf_crypto_hash_state_t status);

static ssp_err_t sf_crypto_hash_instance_initialize(sf_crypto_hash_instance_ctrl_t * p_ctrl, uint32_t digest_size);

static ssp_err_t sf_crypto_hash_instance_deinitialize(sf_crypto_hash_instance_ctrl_t * p_ctrl, uint32_t digest_size);

static ssp_err_t sf_crypto_hash_message_digest_update_status_check (sf_crypto_hash_state_t status);

static ssp_err_t sf_crypto_hash_message_digest_final_status_check (sf_crypto_hash_state_t status);

static void * sf_crypto_hash_interface_get (sf_crypto_hash_instance_ctrl_t const * const p_ctrl);

static uint32_t  sf_crypto_hash_digest_size_get (sf_crypto_hash_type_t hash_type, bool final);

static uint32_t  sf_crypto_hash_message_block_size_get (sf_crypto_hash_type_t hash_type);

static ssp_err_t sf_crypto_hash_memory_allocate(sf_crypto_hash_instance_ctrl_t * const p_ctrl, uint32_t digest_size);

static void      sf_crypto_hash_message_digest_initialize (sf_crypto_hash_type_t hash_type,
                                                           sf_crypto_hash_context_t * p_hash_context);

static ssp_err_t sf_crypto_hash_data_blocks (sf_crypto_hash_instance_ctrl_t * p_ctrl,
                                             sf_crypto_data_handle_t const * const p_data_in);

static ssp_err_t sf_crypto_hash_get_message_digest (sf_crypto_hash_instance_ctrl_t * p_ctrl,
                                                    sf_crypto_data_handle_t * const p_msg_digest, uint32_t * p_size);


/*******************************************************************************************************************//**
 * @brief      This function takes the input address, if it is not aligned on the WORD (4 byte) boundary,
 * aligns it  and returns the address.
 * @param[in]  address - aligned or unaligned
 * @retval     address - aligned on the WORD (4 byte) boundary
 **********************************************************************************************************************/
__STATIC_INLINE uint32_t SF_CRYPTO_HASH_WordAlign(uint32_t address)
{
    return (((address) + (SF_CRYPTO_HASH_WORD_ALIGNMENT_MASK)) & ~(SF_CRYPTO_HASH_WORD_ALIGNMENT_MASK));
}

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
/** HASH initial value for md5 */
static const uint8_t initial_hash_value_md5[MD5_INITIAL_VALUE_LENGTH_IN_BYTES]    =
                                                     {0x67u, 0x45u, 0x23u, 0x01u, 0xefu, 0xcdu, 0xabu, 0x89u,
                                                      0x98u, 0xbau, 0xdcu, 0xfeu, 0x10u, 0x32u, 0x54u, 0x76u};

/** HASH initial value for SHA-1 */
static const uint8_t initial_hash_value_sha1[SHA1_INITIAL_VALUE_LENGTH_IN_BYTES]   =
                                                     {0x67u, 0x45u, 0x23u, 0x01u, 0xefu, 0xcdu, 0xabu, 0x89u,
                                                      0x98u, 0xbau, 0xdcu, 0xfeu, 0x10u, 0x32u, 0x54u, 0x76u,
                                                      0xc3u, 0xd2u, 0xe1u, 0xf0u};

/** HASH initial value for SHA-256 */
static const uint8_t initial_hash_value_sha256[SHA256_INITIAL_VALUE_LENGTH_IN_BYTES] =
                                                     {0x6au, 0x09u, 0xe6u, 0x67u, 0xbbu, 0x67u, 0xaeu, 0x85u,
                                                      0x3cu, 0x6eu, 0xf3u, 0x72u, 0xa5u, 0x4fu, 0xf5u, 0x3au,
                                                      0x51u, 0x0eu, 0x52u, 0x7fu, 0x9bu, 0x05u, 0x68u, 0x8cu,
                                                      0x1fu, 0x83u, 0xd9u, 0xabu, 0x5bu, 0xe0u, 0xcdu, 0x19u};

/** HASH initial value for SHA-224 */
static const uint8_t initial_hash_value_sha224[SHA256_INITIAL_VALUE_LENGTH_IN_BYTES] =
                                                     {0xc1u, 0x05u, 0x9eu, 0xd8u, 0x36u, 0x7cu, 0xd5u, 0x07u,
                                                      0x30u, 0x70u, 0xddu, 0x17u, 0xf7u, 0x0eu, 0x59u, 0x39u,
                                                      0xffu, 0xc0u, 0x0bu, 0x31u, 0x68u, 0x58u, 0x15u, 0x11u,
                                                      0x64u, 0xf9u, 0x8fu, 0xa7u, 0xbeu, 0xfau, 0x4fu, 0xa4u};

#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug. This pragma suppresses the warnings in this
 * structure only.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/* Version data structure */
static const ssp_version_t sf_crypto_hash_version =
{
    .api_version_major  = SF_CRYPTO_HASH_API_VERSION_MAJOR,
    .api_version_minor  = SF_CRYPTO_HASH_API_VERSION_MINOR,
    .code_version_major = SF_CRYPTO_HASH_CODE_VERSION_MAJOR,
    .code_version_minor = SF_CRYPTO_HASH_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/***********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/*LDRA_NOANALYSIS LDRA_INSPECTED below not working. */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const sf_crypto_hash_api_t         g_sf_crypto_hash_api =
{
    .open                = SF_CRYPTO_HASH_Open,
    .close               = SF_CRYPTO_HASH_Close,
    .hashInit            = SF_CRYPTO_HASH_MessageDigestInit,
    .hashUpdate          = SF_CRYPTO_HASH_MessageDigestUpdate,
    .hashFinal           = SF_CRYPTO_HASH_MessageDigestFinal,
    .versionGet          = SF_CRYPTO_HASH_VersionGet
};

/*LDRA_ANALYSIS */

/*******************************************************************************************************************//**
 * @ingroup SF_Library
 * @addtogroup SF_CRYPTO_HASH
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
Functions
***********************************************************************************************************************/
/*******************************************************************************************************************//**
 * @brief   SSP Crypto HASH Framework Open operation.
 *
 * @retval SSP_SUCCESS          The module was successfully opened.
 * @retval SSP_ERR_ASSERTION    NULL is passed through an argument.
 * @retval SSP_ERR_CRYPTO_COMMON_NOT_OPENED     Crypto Framework Common Module has yet been opened.
 * @retval SSP_ERR_ALREADY_OPEN The module has been already opened.
 * @retval SSP_ERR_UNSUPPORTED  HASH algorithms are not supported for the MCU part.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_HASH_Open (sf_crypto_hash_ctrl_t * const p_api_ctrl, sf_crypto_hash_cfg_t const * const p_cfg)
{
    ssp_err_t                         err = SSP_SUCCESS;
    sf_crypto_hash_instance_ctrl_t  * p_ctrl = (sf_crypto_hash_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t       * p_ctrl_common;
    sf_crypto_api_t                 * p_api_common;
    uint32_t                          digest_size = 0;
    sf_crypto_state_t                 common_module_status = SF_CRYPTO_CLOSED;

#if SF_CRYPTO_HASH_CFG_PARAM_CHECKING_ENABLE
    /** Verify if parameters are valid. */
    err = sf_crypto_hash_open_param_check(p_ctrl, p_cfg);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
#endif

    /** Get a Crypto Framework common instance, the control block and interface. */
    p_ctrl->p_lower_lvl_crypto_common = p_cfg->p_lower_lvl_crypto_common;
    p_ctrl_common = p_ctrl->p_lower_lvl_crypto_common->p_ctrl;
    p_api_common  = (sf_crypto_api_t *)p_ctrl->p_lower_lvl_crypto_common->p_api;

    /** Get a lower-level HASH instance. */
    p_ctrl->p_lower_lvl_instance = p_cfg->p_lower_lvl_instance;


    /** Check if the Crypto Framework common instance has been opened. If not yet opened, return an error. */
    err = p_api_common->statusGet(p_ctrl_common, &common_module_status);
    if (SSP_SUCCESS != err)
    {
        return err;
    }

    if (SF_CRYPTO_OPENED != common_module_status)
    {
        err = SSP_ERR_CRYPTO_COMMON_NOT_OPENED;
    }

    if (SSP_SUCCESS == err)
    {
        /** Check if the Crypto HASH Framework module has been opened. */
        if (false == sf_crypto_hash_module_open_status_check(p_ctrl->status))
        {
             /* Set a HASH type and initial HASH value. */
             p_ctrl->hash_type                           = p_cfg->hash_type;
             p_ctrl->hash_context.message_bytes          = 0U;
             p_ctrl->hash_context.message_bytes_buffered = 0U;

             /* Get the size of message digest. */
             digest_size = sf_crypto_hash_digest_size_get(p_cfg->hash_type, false);
        }
        else
        {
            err = SSP_ERR_ALREADY_OPEN;
        }
    }

    if (SSP_SUCCESS == err)
    {
        /* Note: sf_crypto.lock() is not called because the HAL driver does not access HW during open.
         * The HAL interfaceGet API is also called which does not access HW either.
         * In the future, if HW will be accessed through the HAL driver; lock should be acquired.
         */

        /** Create memory resources used by this module and open a HASH HAL module. */
        err = sf_crypto_hash_instance_initialize (p_ctrl, digest_size);
        if (SSP_SUCCESS == err)
        {
            /** Increment the open counter. */
            sf_crypto_open_counter_increment (p_ctrl_common);

            /** We have successfully processed the open procedures.
             * Mark the module status as 'Opened'.
             */
            p_ctrl->status = SF_CRYPTO_HASH_OPENED;
        }
    }

    return err;
} /* End of function SF_CRYPTO_HASH_Open */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto HASH Framework Close operation.
 *
 * @retval SSP_SUCCESS          The module was successfully closed.
 * @retval SSP_ERR_ASSERTION    NULL is passed through the argument.
 * @retval SSP_ERR_NOT_OPEN     The module has yet been opened. Call Open API first.
 * @retval SSP_ERR_UNSUPPORTED  HASH algorithms are not supported for the MCU part.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_HASH_Close (sf_crypto_hash_ctrl_t * const p_api_ctrl)
{
    ssp_err_t                         err = SSP_ERR_NOT_OPEN;
    sf_crypto_hash_instance_ctrl_t  * p_ctrl = (sf_crypto_hash_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t       * p_ctrl_common;
    uint32_t                          digest_size = 0;

#if SF_CRYPTO_HASH_CFG_PARAM_CHECKING_ENABLE
    /** Verify parameters are valid. */
    SSP_ASSERT(p_ctrl);
#endif

    /** Get a Crypto Framework common control block and the interface. */
    p_ctrl_common = p_ctrl->p_lower_lvl_crypto_common->p_ctrl;

    /** Check if the Crypto HASH Framework has been opened. */
    if (true == sf_crypto_hash_module_open_status_check(p_ctrl->status))
    {
        /* Note: sf_crypto.lock() is not called because the HAL driver does not access HW during close.
         * In the future, if HW will be accessed through the HAL driver; lock should be acquired.
         */

        /** Delete memory resources used by this module and close the HASH HAL module. */
        err = sf_crypto_hash_instance_deinitialize(p_ctrl, digest_size);
        if (SSP_SUCCESS == err)
        {
            /** Mark the module status as 'Closed'. */
            p_ctrl->status = SF_CRYPTO_HASH_CLOSED;

            /** Decrement the open counter to enable users to close SF_CRYPTO module. */
            sf_crypto_open_counter_decrement (p_ctrl_common);
        }
    }
    else
    {
    }

    return err;
} /* End of function SF_CRYPTO_HASH_Close */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto HASH Framework - Generates the initial message digest in an internal context buffer. Can be
 * called once messageDigestFinal() is called to initialize a new digest operation. Unless a different HASH type is
 * used, users do not need to close the module for new digest operation. This is a blocking call.
 *
 * @retval SSP_SUCCESS          The module updated a message Digest successfully.
 * @retval SSP_ERR_ASSERTION    NULL is passed through an argument.
 * @retval SSP_ERR_NOT_OPEN     The module has yet been opened. Call Open API first.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 * @note When this function is called if SF_CRYPTO_HASH_DIGEST_INITIALIZED or SF_CRYPTO_HASH_DIGEST_UPDATED, the
 *  message digest will be initialized.
 **********************************************************************************************************************/
ssp_err_t   SF_CRYPTO_HASH_MessageDigestInit (sf_crypto_hash_ctrl_t * const p_api_ctrl)
{
    ssp_err_t   err = SSP_ERR_NOT_OPEN;
    sf_crypto_hash_instance_ctrl_t * p_ctrl = (sf_crypto_hash_instance_ctrl_t *) p_api_ctrl;

#if SF_CRYPTO_HASH_CFG_PARAM_CHECKING_ENABLE
    /** Verify if parameters are valid. */
    SSP_ASSERT(p_ctrl);
#endif

    /** Check if the Crypto HASH Framework module has been opened. If not yet opened, return an error. */
    if (true == sf_crypto_hash_module_open_status_check(p_ctrl->status))
    {
        /* Initialize the hash context. */
        p_ctrl->hash_context.message_bytes           = 0U;
        p_ctrl->hash_context.message_bytes_buffered  = 0U;

        /* Note: sf_crypto.lock() is not called because the HAL driver does not access HW during init.
         * In the future, if HW will be accessed through the HAL driver; lock should be acquired.
         */

        /** Create initial message digest code in the module context. */
        sf_crypto_hash_message_digest_initialize (p_ctrl->hash_type, &p_ctrl->hash_context);

        /** Mark the module status as 'Digest Initialized'. */
        p_ctrl->status = SF_CRYPTO_HASH_DIGEST_INITIALIZED;

        err = SSP_SUCCESS;
    }

    return (err);
} /* End of function SF_CRYPTO_HASH_MessageDigestInit */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto HASH Framework - Hashes input data and saves it in an internal context buffer. Can be called
 * multiple times for additional blocks of data. This is a blocking call
 *
 * @retval SSP_SUCCESS          The module updated a message Digest successfully.
 * @retval SSP_ERR_ASSERTION    NULL is passed through an argument.
 * @retval SSP_ERR_NOT_OPEN     The module has yet been opened. Call Open API first.
 * @retval SSP_ERR_UNSUPPORTED  HASH algorithms are not supported for the MCU part.
 * @retval SSP_ERR_INVALID_CALL Function call was made if the module state had not yet transitioned to
 *                              SF_CRYPTO_HASH_DIGEST_INITIALIZED.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 **********************************************************************************************************************/
ssp_err_t   SF_CRYPTO_HASH_MessageDigestUpdate (sf_crypto_hash_ctrl_t * const p_api_ctrl,
                                                sf_crypto_data_handle_t const * const p_data_in)
{
    ssp_err_t   err = SSP_SUCCESS;
    sf_crypto_hash_instance_ctrl_t * p_ctrl = (sf_crypto_hash_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t      * p_ctrl_common;
    sf_crypto_api_t                * p_api_common;

#if SF_CRYPTO_HASH_CFG_PARAM_CHECKING_ENABLE
    /** Verify if parameters are valid. */
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_data_in);
#endif

    /** Get a Crypto Framework common control block and the interface. */
    p_ctrl_common = p_ctrl->p_lower_lvl_crypto_common->p_ctrl;
    p_api_common  = (sf_crypto_api_t *)p_ctrl->p_lower_lvl_crypto_common->p_api;

    /** Check if the Crypto Framework has been in 'Digest Initialized' status. If not, return an error. */
    err = sf_crypto_hash_message_digest_update_status_check(p_ctrl->status);
    if (SSP_SUCCESS == err)
    {
        /* Lock the module to access to Crypto HAL module. */
        err = p_api_common->lock (p_ctrl_common);
    }

    if (SSP_SUCCESS == err)
    {
        /** Update the message digest. */
        err = sf_crypto_hash_data_blocks (p_ctrl, p_data_in);

        if (SSP_SUCCESS == err)
        {
            /** Mark the module status as 'Digest Updated'. */
            p_ctrl->status = SF_CRYPTO_HASH_DIGEST_UPDATED;
        }

        /** Unlock the module. */
        ssp_err_t err_unlock = p_api_common->unlock (p_ctrl_common);
        if (SSP_SUCCESS == err)
        {
            err = err_unlock;
        }
    }

    return (err);
} /* End of function SF_CRYPTO_HASH_MessageDigestUpdate */

/*******************************************************************************************************************//**
 * @brief   SSP Crypto HASH Framework - Hashes the last block of data and returns the message digest in the output
 * buffer. Once this function is called, no additional function calls can be made but open function call can be made to
 * initialize a new digest operation. The Output buffer will contain the message digest on success and the buffer length
 * will be updated to reflect the size of the digest. On error, only the length is set to 0.This is a blocking call.
 *
 * @retval SSP_SUCCESS          The module updated a message Digest successfully.
 * @retval SSP_ERR_ASSERTION    NULL is passed through an argument.
 * @retval SSP_ERR_INVALID_SIZE The memory size to store a message digest is not sufficient for the digest type.
 * @retval SSP_ERR_NOT_OPEN     The module has yet been opened. Call Open API first.
 * @retval SSP_ERR_UNSUPPORTED  HASH algorithms are not supported for the MCU part.
 * @retval SSP_ERR_INVALID_CALL Function call was made if the module state had not yet transitioned to
 *                              SF_CRYPTO_HASH_DIGEST_UPDATED.
 * @return                      See @ref Common_Error_Codes for other possible return codes.
 * @note   p_msg_digest->p_data must be WORD aligned. The memory allocation to store a message digest is user's
 *         responsibility.
 **********************************************************************************************************************/
ssp_err_t   SF_CRYPTO_HASH_MessageDigestFinal (sf_crypto_hash_ctrl_t * const p_api_ctrl,
                                               sf_crypto_data_handle_t * const p_msg_digest, uint32_t * p_size)
{
    ssp_err_t   err = SSP_SUCCESS;
    sf_crypto_hash_instance_ctrl_t  * p_ctrl = (sf_crypto_hash_instance_ctrl_t *) p_api_ctrl;
    sf_crypto_instance_ctrl_t       * p_ctrl_common;
    sf_crypto_api_t                 * p_api_common;

#if SF_CRYPTO_HASH_CFG_PARAM_CHECKING_ENABLE
    /** Verify if parameters are valid */
    err = sf_crypto_hash_message_digest_final_param_check(p_ctrl, p_msg_digest, p_size);
    if (SSP_SUCCESS != err)
    {
        return err;
    }
#endif

    /** Get a Crypto Framework common control block and the interface. */
    p_ctrl_common = p_ctrl->p_lower_lvl_crypto_common->p_ctrl;
    p_api_common  = (sf_crypto_api_t *)p_ctrl->p_lower_lvl_crypto_common->p_api;

    /** Check if the Crypto Framework has been in 'Digest Updated' status. If not, return an error. */
    err = sf_crypto_hash_message_digest_final_status_check(p_ctrl->status);
    if (SSP_SUCCESS == err)
    {
        /* Lock the module to access to Crypto HAL module. */
        err = p_api_common->lock (p_ctrl_common);
    }

    if (SSP_SUCCESS == err)
    {
        /** Update the message digest. */
        err = sf_crypto_hash_get_message_digest (p_ctrl, p_msg_digest, p_size);
        if (SSP_SUCCESS == err)
        {
            /** We are all set now. A message digest is returned. Mark the module status as 'Opened'. */
            p_ctrl->status = SF_CRYPTO_HASH_OPENED;
        }

        /** Unlock the module. */
        ssp_err_t err_unlock = p_api_common->unlock (p_ctrl_common);

        if (SSP_SUCCESS == err)
        {
            err = err_unlock;
        }
    }

    return (err);
} /* End of function SF_CRYPTO_HASH_MessageDigestFinal */

/*******************************************************************************************************************//**
 * @brief      Gets driver version based on compile time macros.
 *
 * @retval     SSP_SUCCESS          Successful close.
 * @retval     SSP_ERR_ASSERTION    The parameter p_version is NULL.
 **********************************************************************************************************************/
ssp_err_t SF_CRYPTO_HASH_VersionGet (ssp_version_t * const p_version)
{
#if SF_CRYPTO_HASH_CFG_PARAM_CHECKING_ENABLE
    /* Verify parameters are valid. */
    SSP_ASSERT(p_version);
#endif

    p_version->version_id = sf_crypto_hash_version.version_id;

    return SSP_SUCCESS;
} /* End of function SF_CRYPTO_HASH_VersionGet */

/*******************************************************************************************************************//**
 * @} (end defgroup SF_CRYPTO_HASH)
 **********************************************************************************************************************/

/***********************************************************************************************************************
Private Functions
***********************************************************************************************************************/
#if SF_CRYPTO_HASH_CFG_PARAM_CHECKING_ENABLE
/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto HASH Framework to check open API parameters.
 * This function is called by SF_CRYPTO_HASH_Open().
 *
 * @param[in]   p_ctrl          Pointer to a HASH framework control block.
 * @param[in]   p_cfg           Pointer to a HASH framework configuration structure.
 * @retval SSP_SUCCESS          Parameters are all valid.
 * @retval SSP_ERR_ASSERTION    NULL is passed through an argument.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_hash_open_param_check (sf_crypto_hash_instance_ctrl_t * const p_ctrl,
                                                  sf_crypto_hash_cfg_t const * const p_cfg)
{
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_cfg);
    SSP_ASSERT(p_cfg->p_lower_lvl_crypto_common);

    if(  (SF_CRYPTO_HASH_ALGORITHM_MD5    != p_cfg->hash_type)
       &&(SF_CRYPTO_HASH_ALGORITHM_SHA1   != p_cfg->hash_type)
       &&(SF_CRYPTO_HASH_ALGORITHM_SHA256 != p_cfg->hash_type)
       &&(SF_CRYPTO_HASH_ALGORITHM_SHA224 != p_cfg->hash_type))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto HASH Framework to check messageDigestFinal API parameters.
 * This function is called by SF_CRYPTO_HASH_MessageDigestFinal().
 *
 * @param[in]   p_ctrl          Pointer to a HASH framework control block.
 * @param[in]   p_msg_digest    Pointer to an output data buffer and the buffer size.
 * @param[in]   p_size          Pointer to the 32-bit memory space to store the size of message digest.
 * @retval SSP_SUCCESS          Parameters are all valid.
 * @retval SSP_ERR_ASSERTION    NULL is passed through an argument.
 * @retval SSP_ERR_INVALID_SIZE The memory size to store a message digest is not sufficient for the digest type.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_hash_message_digest_final_param_check (sf_crypto_hash_instance_ctrl_t * const p_ctrl,
                                                    sf_crypto_data_handle_t * const p_msg_digest, uint32_t * p_size)
{
    SSP_ASSERT(p_ctrl);
    SSP_ASSERT(p_msg_digest);
    SSP_ASSERT(p_size);
    SSP_ASSERT(p_msg_digest->p_data);
    SSP_ASSERT(0U == ((uint32_t)p_msg_digest->p_data % sizeof(uint32_t)));

    if (p_msg_digest->data_length < sf_crypto_hash_digest_size_get (p_ctrl->hash_type, true))
    {
        return SSP_ERR_INVALID_SIZE;
    }

    return SSP_SUCCESS;
}
#endif


/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto HASH Framework to initialize software and hardware instances.
 * This function is called by SF_CRYPTO_HASH_Open().
 *
 * @param[in, out]  p_ctrl          Pointer to a HASH framework control block.
 * @param[in]       digest_size     Digest size.
 * @retval SSP_SUCCESS              Software and hardware instances were successfully initialized.
 * @retval SSP_ERR_UNSUPPORTED      HASH algorithms are not supported for the MCU part.
 * @note This function does not evaluate the argument. It is the caller's responsibility.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_hash_instance_initialize(sf_crypto_hash_instance_ctrl_t  * p_ctrl, uint32_t digest_size)
{
    ssp_err_t           err = SSP_SUCCESS;
    uint32_t            err_crypto;
    hash_api_t        * p_hash_api;

    /* Allocate memory for a hash context. */
    err = sf_crypto_hash_memory_allocate(p_ctrl, digest_size);
    if (SSP_SUCCESS == err)
    {
        /* Get low-level HASH HAL module API instance. */
        p_hash_api = sf_crypto_hash_interface_get(p_ctrl);
        if (NULL != p_hash_api)
        {
            /* Open low-level HASH HAL module. */
            err_crypto = p_hash_api->open (p_ctrl->p_lower_lvl_instance->p_ctrl, p_ctrl->p_lower_lvl_instance->p_cfg);
            if ((uint32_t)SSP_SUCCESS != err_crypto)
            {
                err = (ssp_err_t)err_crypto;
            }
        }
        else
        {
            /* An obtained API instance was not valid. */
            err = SSP_ERR_UNSUPPORTED;
        }

        if (SSP_SUCCESS != err)
        {
            /* Failed to open HAL module. Release all the memory allocated for a hash context. */
            sf_crypto_memory_release (p_ctrl->hash_context.p_message_digest_org, digest_size + SF_CRYPTO_HASH_ALIGNMENT_ADJUSTMENT_BYTES);
            uint32_t block_size = sf_crypto_hash_message_block_size_get(p_ctrl->hash_type);
            sf_crypto_memory_release (p_ctrl->hash_context.p_message_buffer_org, block_size + SF_CRYPTO_HASH_ALIGNMENT_ADJUSTMENT_BYTES);
        }
    }

    return err;
}

/*******************************************************************************************************************//**
* @brief   Sub-routine for Crypto HASH Framework to de-initialize software and hardware instances.
* This function is called by SF_CRYPTO_HASH_Close().
*
* @param[in, out]  p_ctrl          Pointer to a HASH framework control block.
* @param[in]       digest_size     Digest size.
* @retval SSP_SUCCESS              Software and hardware instances were successfully de-initialized.
* @retval SSP_ERR_UNSUPPORTED      HASH algorithms are not supported for the MCU part.
* @note This function does not evaluate the argument. It is the caller's responsibility.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_hash_instance_deinitialize(sf_crypto_hash_instance_ctrl_t * p_ctrl, uint32_t digest_size)
{
    ssp_err_t           err = SSP_SUCCESS;
    uint32_t            err_crypto;
    hash_api_t        * p_hash_api;
    uint32_t            block_size  = 0;

    /* Get low-level HASH HAL module API instance. */
    p_hash_api = sf_crypto_hash_interface_get(p_ctrl);
    if (NULL != p_hash_api)
    {
        /* Close Crypto HAL module. */
        err_crypto = p_hash_api->close (p_ctrl->p_lower_lvl_instance->p_ctrl);
        if ((uint32_t)SSP_SUCCESS != err_crypto)
        {
            err = (ssp_err_t)err_crypto;
        }
        else
        {
            /* Get the size of message digest. */
            digest_size = sf_crypto_hash_digest_size_get(p_ctrl->hash_type, false);
            block_size  = sf_crypto_hash_message_block_size_get(p_ctrl->hash_type);

            /* Zeroise and release memory. */
            err = sf_crypto_memory_release (p_ctrl->hash_context.p_message_digest_org,
                                            digest_size + SF_CRYPTO_HASH_ALIGNMENT_ADJUSTMENT_BYTES);
            if ((uint32_t)SSP_SUCCESS == err)
            {
                err = sf_crypto_memory_release (p_ctrl->hash_context.p_message_buffer_org,
                                                 block_size + SF_CRYPTO_HASH_ALIGNMENT_ADJUSTMENT_BYTES);
            }
        }
    }
    else
    {
        /* An obtained API instance was not valid. */
        err = SSP_ERR_UNSUPPORTED;
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto HASH Framework to get size of message digest.
 * This function is called by SF_CRYPTO_HASH_Open(). SF_CRYPTO_HASH_Close(), SF_CRYPTO_HASH_MessageDigestFinal(),
 * sf_crypto_hash_message_digest_initialize() and sf_crypto_hash_get_message_digest().
 *
 * @param[in]   hash_type            HASH type. SHA1, SHA224, SHA256 and MD5 are supported.
 * @param[in]   final                Set True to get the final message digest size for specified HASH type. Set False
 *                                   to get intermediate message digest size. The digest size would be different for
 *                                   SHA224.
 * @retval      size                 Size of message digest in bytes.
 **********************************************************************************************************************/
static uint32_t sf_crypto_hash_digest_size_get (sf_crypto_hash_type_t hash_type, bool final)
{
    uint32_t size;

    switch (hash_type)
    {
    case SF_CRYPTO_HASH_ALGORITHM_SHA224:
        if (true == final)
        {
            size = SF_CRYPTO_HASH_MESSAGE_DIGEST_SIZE_SHA224;
        }
        else
        {
            /* SHA-224 needs 8 words for an intermediate message digest. */
            size = SF_CRYPTO_HASH_MESSAGE_DIGEST_SIZE_SHA256;
        }
        break;

    case SF_CRYPTO_HASH_ALGORITHM_SHA256:
        size = SF_CRYPTO_HASH_MESSAGE_DIGEST_SIZE_SHA256;
        break;

    case SF_CRYPTO_HASH_ALGORITHM_MD5:
        size = SF_CRYPTO_HASH_MESSAGE_DIGEST_SIZE_MD5;
        break;

    default: /* SF_CRYPTO_HASH_ALGORITHM_SHA1 */
        size = SF_CRYPTO_HASH_MESSAGE_DIGEST_SIZE_SHA1;
        break;
    }

    return size;
}

/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto HASH Framework to get size of message block.
 * This function is called by SF_CRYPTO_HASH_MessageDigestUpdate().
 *
 * @param[in]   hash_type            HASH type. SHA1, SHA224, SHA256 and MD5 are supported.
 * @retval      size                 Size of message block in bytes.
 **********************************************************************************************************************/
static uint32_t sf_crypto_hash_message_block_size_get (sf_crypto_hash_type_t hash_type)
{
    uint32_t size;

    switch (hash_type)
    {
    case SF_CRYPTO_HASH_ALGORITHM_MD5:
    case SF_CRYPTO_HASH_ALGORITHM_SHA1:
    case SF_CRYPTO_HASH_ALGORITHM_SHA224:
    case SF_CRYPTO_HASH_ALGORITHM_SHA256:
    default:
        size = (uint32_t)SF_CRYPTO_HASH_BLOCK_SIZE_MD5_SHA1_SHA224_SHA256;
        break;
    }

    return size;
}

/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto HASH Framework to allocate required memory in the byte memory pool.
 * This function is called by SF_CRYPTO_HASH_Open(). Allocates additional 3 bytes when allocating memory space from the
 * byte pool memory since the start address might not be aligned to word memory boundary.
 * @param[in, out]   p_ctrl          Pointer to a HASH framework control block.
 * @param[in]        digest_size     Digest size.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_hash_memory_allocate(sf_crypto_hash_instance_ctrl_t * const p_ctrl, uint32_t digest_size)
{
    ssp_err_t                   err = SSP_SUCCESS;
    sf_crypto_instance_ctrl_t * p_ctrl_common;

    /* Get a Crypto common control block and the interface. */
    p_ctrl_common = p_ctrl->p_lower_lvl_crypto_common->p_ctrl;

    /* Allocate memory for message digest. Extra 3bytes are allocated to get the buffer WORD aligned. */
    err = sf_crypto_memory_allocate (&p_ctrl_common->byte_pool,
                                     (void **)&p_ctrl->hash_context.p_message_digest_org,
                                     (digest_size + SF_CRYPTO_HASH_ALIGNMENT_ADJUSTMENT_BYTES),
                                     p_ctrl_common->wait_option);
    if ((uint32_t)SSP_SUCCESS == err)
    {
        /* WORD align the  p_message_digest allocated above. */

        p_ctrl->hash_context.p_message_digest = (uint8_t *)(SF_CRYPTO_HASH_WordAlign((uint32_t)(p_ctrl->hash_context.p_message_digest_org)));

        uint32_t block_size = sf_crypto_hash_message_block_size_get(p_ctrl->hash_type);

        /* Allocate memory for message buffer. Extra 3bytes are allocated to get the buffer WORD aligned. */
        err = sf_crypto_memory_allocate (&p_ctrl_common->byte_pool,
                                         (void **)&p_ctrl->hash_context.p_message_buffer_org,
                                         (block_size + SF_CRYPTO_HASH_ALIGNMENT_ADJUSTMENT_BYTES),
                                         p_ctrl_common->wait_option);
        if ((uint32_t)SSP_SUCCESS != err)
        {
            /* Release memory and unlock the module if HAL module returned an error. */
            sf_crypto_memory_release (p_ctrl->hash_context.p_message_digest_org, digest_size + SF_CRYPTO_HASH_ALIGNMENT_ADJUSTMENT_BYTES);
        }else
        {
            /* WORD align the  p_message_buffer allocated above. */
            p_ctrl->hash_context.p_message_buffer = (uint8_t *)(SF_CRYPTO_HASH_WordAlign((uint32_t)(p_ctrl->hash_context.p_message_buffer_org)));
        }
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief   Sub-routine for Crypto HASH Framework to set initial message digest.
 * This function is called by SF_CRYPTO_HASH_Open().
 *
 * @param[in]   hash_type            HASH type. SHA1, SHA224, SHA256 and MD5 are supported.
 * @param[out]  p_hash_context       Pointer to a HASH context data.
 **********************************************************************************************************************/
static void sf_crypto_hash_message_digest_initialize (sf_crypto_hash_type_t hash_type,
                                                      sf_crypto_hash_context_t * p_hash_context)
{
    uint8_t * p_message_digest;
    uint8_t * p_initial_hash_value;

    p_message_digest = p_hash_context->p_message_digest;

    switch (hash_type)
    {
    case SF_CRYPTO_HASH_ALGORITHM_SHA256:
        p_initial_hash_value = (uint8_t *)initial_hash_value_sha256;
        break;

    case SF_CRYPTO_HASH_ALGORITHM_SHA224:
        p_initial_hash_value = (uint8_t *)initial_hash_value_sha224;
        break;

    case SF_CRYPTO_HASH_ALGORITHM_MD5:
        p_initial_hash_value = (uint8_t *)initial_hash_value_md5;
        break;

    case SF_CRYPTO_HASH_ALGORITHM_SHA1:
        p_initial_hash_value = (uint8_t *)initial_hash_value_sha1;
        break;

    default: /* SF_CRYPTO_HASH_ALGORITHM_SHA1 */
        p_initial_hash_value = (uint8_t *)initial_hash_value_sha1;
        break;
    }

    /* The copy source and the destination memory regions are not overlapped but LDRA reports they are overlapped.
     * It is clear that there is no overlap so suppress the rule 480 S for following code. */
    /*LDRA_INSPECTED 480 S */

    memcpy(p_message_digest, p_initial_hash_value, sf_crypto_hash_digest_size_get(hash_type, false));

    p_hash_context->message_bytes = 0U;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to get a HASH HAL API instance. This function is called by
 * sf_crypto_hash_instance_initialize() and sf_crypto_hash_instance_deinitialize(), sf_crypto_hash_data_blocks() and
 * sf_crypto_hash_get_message_digest().
 *
 * @param[in]  p_ctrl           Pointer to a HASH framework control block.
 * @retval     non-NULL         Address to a HASH API instance.
 * @retval     NULL             No any HASH API instance available.
 **********************************************************************************************************************/
static void * sf_crypto_hash_interface_get (sf_crypto_hash_instance_ctrl_t const * const p_ctrl)
{
    sf_crypto_instance_ctrl_t * p_lower_level_crypto_ctrl;
    crypto_instance_t         * p_crypto;
    crypto_interface_get_param_t param;
    hash_api_t                * p_hash_api = NULL;

    /* Get a Crypto common control block and the HAL instance. */
    p_lower_level_crypto_ctrl = (sf_crypto_instance_ctrl_t *)(p_ctrl->p_lower_lvl_crypto_common->p_ctrl);
    p_crypto = (crypto_instance_t *)(p_lower_level_crypto_ctrl->p_lower_lvl_crypto);

    /* Check the HASH algorithm type specified and set the parameter for HAL instanceGet API. */
    switch (p_ctrl->hash_type)
    {
    case SF_CRYPTO_HASH_ALGORITHM_SHA256:
        param.hash_type = CRYPTO_TYPE_HASH_256;
        break;

    case SF_CRYPTO_HASH_ALGORITHM_SHA224:
        param.hash_type = CRYPTO_TYPE_HASH_224;
        break;

    case SF_CRYPTO_HASH_ALGORITHM_MD5:
        param.hash_type = CRYPTO_TYPE_HASH_MD5;
        break;

    default: /* SF_CRYPTO_HASH_ALGORITHM_SHA1 */
        param.hash_type = CRYPTO_TYPE_HASH_1;
        break;
    }

    /* Get the HAL API instance for a selected algorithm type. */
    param.algorithm_type = CRYPTO_ALGORITHM_TYPE_HASH;
    p_crypto->p_api->interfaceGet(&param, &p_hash_api);

    return p_hash_api;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to check if the module has been opened.
 *
 * @param[in]       status          Current module status.
 * @retval          Bool            True if module is opened. False is module is closed.
 * @note This function does not evaluate the argument. It is the caller's responsibility.
 **********************************************************************************************************************/
static bool sf_crypto_hash_module_open_status_check (sf_crypto_hash_state_t status)
{
    bool opened = false;

    if (  (SF_CRYPTO_HASH_OPENED             == status)
        ||(SF_CRYPTO_HASH_DIGEST_INITIALIZED == status)
        ||(SF_CRYPTO_HASH_DIGEST_UPDATED     == status))
    {
        opened = true;
    }

    return opened;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to check if the module has been opened and the message digest has been initialized.
 *
 * @param[in]       status                  Current module status.
 * @retval          SSP_SUCCESS             Module has been opened and the message digest has been ever updated.
 * @retval          SSP_ERR_NOT_OPEN        Module is not opened.
 * @retval          SSP_ERR_INVALID_CALL    Message digest has not yet updated.
 * @note This function does not evaluate the argument. It is the caller's responsibility.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_hash_message_digest_update_status_check (sf_crypto_hash_state_t status)
{
    ssp_err_t err = SSP_SUCCESS;

    if (false == sf_crypto_hash_module_open_status_check(status))
    {
        err = SSP_ERR_NOT_OPEN;
    }
    else if (SF_CRYPTO_HASH_OPENED == status)
    {
        err = SSP_ERR_INVALID_CALL;
    }
    else
    {
        /* Module status is SF_CRYPTO_HASH_DIGEST_INITIALIZED or SF_CRYPTO_HASH_DIGEST_UPDATED */
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to check if the module has been opened and the message digest has been ever updated.
 *
 * @param[in]       status                  Current module status.
 * @retval          SSP_SUCCESS             Module has been opened and the message digest has been ever updated.
 * @retval          SSP_ERR_NOT_OPEN        Module is not opened.
 * @retval          SSP_ERR_INVALID_CALL    Message digest has not yet updated.
 * @note This function does not evaluate the argument. It is the caller's responsibility.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_hash_message_digest_final_status_check (sf_crypto_hash_state_t status)
{
    ssp_err_t err = SSP_SUCCESS;

    if (false == sf_crypto_hash_module_open_status_check(status))
    {
        err = SSP_ERR_NOT_OPEN;
    }

    if (SF_CRYPTO_HASH_DIGEST_UPDATED != status)
    {
        err = SSP_ERR_INVALID_CALL;
    }
    else
    {
        /* Module status is in SF_CRYPTO_HASH_DIGEST_UPDATED. */
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to hash data blocks and update the message digest.
 *
 * @param[in, out]  p_ctrl          Pointer to a HASH framework control block.
 * @param[in]       p_data_in       Input data.
 * @retval SSP_SUCCESS              Module has been opened and the message digest has been ever updated.
 * @retval SSP_ERR_UNSUPPORTED      HASH algorithms are not supported for the MCU part.
 * @return                          See @ref Common_Error_Codes for other possible return codes.
 * @note This function does not evaluate the argument. It is the caller's responsibility.
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_hash_data_blocks (sf_crypto_hash_instance_ctrl_t * p_ctrl,
                                             sf_crypto_data_handle_t const * const p_data_in)
{
    ssp_err_t       err = SSP_SUCCESS;
    uint32_t        err_crypto;
    uint32_t        number_of_blocks;
    uint32_t        block_size;
    uint32_t        total_data_length;
    uint32_t        remaining_data_length;
    uint32_t        copy_len;
    uint8_t       * p_data;
    uint8_t       * p_message_buffer;
    hash_api_t    * p_hash_api;

    /* Get data. */
    p_data = p_data_in->p_data;

    /* Update the number of processed data bytes. */
    p_ctrl->hash_context.message_bytes = p_ctrl->hash_context.message_bytes + (uint64_t)p_data_in->data_length;

    /* Get length of data including the bytes already buffered in the hash context in the module control block. */
    total_data_length = p_data_in->data_length + p_ctrl->hash_context.message_bytes_buffered;

    /* Get the HASH block size. */
    block_size = sf_crypto_hash_message_block_size_get(p_ctrl->hash_type);

    /* Calculate the total number of blocks to hash. */
    number_of_blocks = total_data_length / block_size;

    /* Calculate the data length to copy to the message buffer in the hash context. */
    copy_len = block_size - p_ctrl->hash_context.message_bytes_buffered;
    if (p_data_in->data_length < copy_len)
    {
        copy_len = p_data_in->data_length;
    }

     p_message_buffer = p_ctrl->hash_context.p_message_buffer;

    /* Copy data to the message buffer. */
    /* The copy source memory (user buffer) and the destination memory(hash context) are not overlapped but LDRA
     * reports they are overlapped. It is clear that there is no overlap so suppress the rule 480 S for following
     * code. */
    /*LDRA_INSPECTED 480 S */
    memcpy((p_message_buffer + p_ctrl->hash_context.message_bytes_buffered), p_data, copy_len);

    /* Update pointer to user data. */
    p_data = p_data + copy_len;

    /* Calculate the length of remaining data. */
    remaining_data_length = p_data_in->data_length - copy_len;

    /* Count up the number of data bytes saved in the message buffer. */
    p_ctrl->hash_context.message_bytes_buffered = p_ctrl->hash_context.message_bytes_buffered + copy_len;

    /* Get low-level HASH HAL module API instance. */
    p_hash_api = sf_crypto_hash_interface_get(p_ctrl);
    if (NULL != p_hash_api)
    {
        if (number_of_blocks > 0U)
        {
            for (uint32_t i = 0U; i < number_of_blocks; i++)
            {
                /* Call the HASH HAL module to update message digest. Note that HAL module needs Number of words
                 * but not bytes */
                err_crypto = p_hash_api->hashUpdate (p_ctrl->p_lower_lvl_instance->p_ctrl,
                                                     (uint32_t *)p_message_buffer,
                                                     (block_size / sizeof(uint32_t)),
                                                     (uint32_t *)p_ctrl->hash_context.p_message_digest);
                if ((uint32_t)SSP_SUCCESS != err_crypto)
                {
                    /* Error happened. Quit HASH update operation and return an error. */
                    err = (ssp_err_t)err_crypto;
                    break;
                }

                if (remaining_data_length >= block_size)
                {
                    /* Copy data to the message buffer. */
                    /* The copy source memory (user buffer) and the destination memory(hash context) are not overlapped but LDRA
                     * reports they are overlapped. It is clear that there is no overlap so suppress the rule 480 S for following
                     * code. */
                    /*LDRA_INSPECTED 480 S */
                    memcpy(p_message_buffer, p_data, block_size);

                    /* Get next data and rest of data length. */
                    p_data = p_data + block_size;

                    /* Calculate the length of remaining data. */
                    remaining_data_length = remaining_data_length - block_size;
                }
            }

            if (SSP_SUCCESS == err)
            {
                /* Copy remaining data to save in the hash context for the next message digest update. */
                /* The copy source memory (user buffer) and the destination memory(hash context) are not overlapped but
                 * LDRA reports they are overlapped. It is clear that there is no overlap so suppress the rule 480 S for
                 * following code. */
                /*LDRA_INSPECTED 480 S */
                memcpy(p_message_buffer, p_data, remaining_data_length);
                p_ctrl->hash_context.message_bytes_buffered = remaining_data_length;
            }
        }
    }
    else
    {
        /* An obtained API instance was not valid. */
        err = SSP_ERR_UNSUPPORTED;
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief      Subroutine to hash final data block(s) and get the message digest.
 *
 * @param[in, out]  p_ctrl          Pointer to a HASH framework control block.
 * @param[in ]      p_msg_digest    Pointer to an output data buffer and the buffer size. Message digest will be
 *                                  generated in the buffer.
 * @param[in]       p_size          Size of buffer.

 *
 * @retval SSP_SUCCESS              The module updated a message Digest successfully.
 * @retval SSP_ERR_UNSUPPORTED      HASH algorithms are not supported for the MCU part.
 * @return                          See @ref Common_Error_Codes for other possible return codes.
 *
 * @note Special Case for MD5 where the length needs to be byte SWAPPED.
 *
 **********************************************************************************************************************/
static ssp_err_t sf_crypto_hash_get_message_digest (sf_crypto_hash_instance_ctrl_t * p_ctrl,
                                                    sf_crypto_data_handle_t * const p_msg_digest, uint32_t * p_size)
{
    ssp_err_t       err = SSP_SUCCESS;
    uint32_t        err_crypto;
    uint32_t        block_size;
    uint16_t        pad_len = 0;
    uint8_t       * p_message_buffer;
    uint8_t       * p_message_digest;
    uint32_t      * p_message_digest32;
    hash_api_t    * p_hash_api;

    /* Get the HASH block length. */
    block_size = sf_crypto_hash_message_block_size_get(p_ctrl->hash_type);

    /* Align the message buffer and message digest on WORD memory boundary. */
    p_message_buffer = p_ctrl->hash_context.p_message_buffer;
    p_message_digest = p_ctrl->hash_context.p_message_digest;

    /* Add HASH terminator code. */
    *(uint8_t *)(p_message_buffer + p_ctrl->hash_context.message_bytes_buffered)
                                                                    = SF_CRYPTO_HASH_TERMINATOR;

    /* Calculate number of bytes to pad the buffer with zero. */
    pad_len = (uint16_t)(block_size - (p_ctrl->hash_context.message_bytes_buffered
                                       + (uint32_t)SF_CRYPTO_HASH_TERMINATOR_SIZE));

    /* Pad rest of the buffer with zero. */
    memset((p_ctrl->hash_context.p_message_buffer
            + (p_ctrl->hash_context.message_bytes_buffered + SF_CRYPTO_HASH_TERMINATOR_SIZE)), 0x0U, pad_len);

    /* Get low-level HASH HAL module API instance. */
    p_hash_api = sf_crypto_hash_interface_get(p_ctrl);
    if (NULL != p_hash_api)
    {
        if (SF_CRYPTO_HASH_BIT_LENGTH_IN_BYTES > pad_len)
        {
            /* There is no enough space to place the bit-length field. We need to hash current block and
             * another hashing as the final procedure will follow next. Call the HASH HAL module to update
             * message digest. Note that HAL module needs Number of words but not bytes. */

            err_crypto = p_hash_api->hashUpdate (p_ctrl->p_lower_lvl_instance->p_ctrl,
                                                 (uint32_t *)p_message_buffer,
                                                 (block_size / 4U),
                                                 (uint32_t *)p_message_digest);
            if ((uint32_t)SSP_SUCCESS == err_crypto)
            {
                /* Pad the buffer with zero again to prepare the last block to hash. */
                memset(p_message_buffer, 0U, block_size);
            }
            else
            {
                /* Failed in HASH HAL driver. Save the error code. */
                err = (ssp_err_t)err_crypto;
            }
        }

        if (SSP_SUCCESS == err)
        {
            /* Place 64-bit bit-length field at the last 64-bit region in the last block. */
            /* Place 64-bit bit-length field at the last 64-bit region in the last block. */
           uint8_t * p_bit_length =  (uint8_t *)((uint32_t)p_message_buffer
                                              + (block_size - SF_CRYPTO_HASH_BIT_LENGTH_IN_BYTES));
            if (p_ctrl->hash_type != SF_CRYPTO_HASH_ALGORITHM_MD5)
            {
                uint32_t  bit_length_32bit = (uint32_t)((p_ctrl->hash_context.message_bytes << 3) >> 32);
				p_bit_length[0] = (uint8_t)(bit_length_32bit >> 24);
				p_bit_length[1] = (uint8_t)(bit_length_32bit >> 16);
				p_bit_length[2] = (uint8_t)(bit_length_32bit >> 8);
				p_bit_length[3] = (uint8_t)(bit_length_32bit >> 0);

				bit_length_32bit = (uint32_t)(p_ctrl->hash_context.message_bytes << 3);
				p_bit_length[4] = (uint8_t)(bit_length_32bit >> 24);
				p_bit_length[5] = (uint8_t)(bit_length_32bit >> 16);
				p_bit_length[6] = (uint8_t)(bit_length_32bit >> 8);
				p_bit_length[7] = (uint8_t)(bit_length_32bit >> 0);
            }
            else
            /*  Special Case for MD5 byte swapping of the data length (7->0, 6->1, 5->2, 4->3). */
            {
            	uint32_t  bit_length_32bit = (uint32_t)((p_ctrl->hash_context.message_bytes << 3) >> 32);
                p_bit_length[7] = (uint8_t)(bit_length_32bit >> 24);
                p_bit_length[6] = (uint8_t)(bit_length_32bit >> 16);
                p_bit_length[5] = (uint8_t)(bit_length_32bit >> 8);
                p_bit_length[4] = (uint8_t)(bit_length_32bit >> 0);

                bit_length_32bit = (uint32_t)(p_ctrl->hash_context.message_bytes << 3);
                p_bit_length[3] = (uint8_t)(bit_length_32bit >> 24);
                p_bit_length[2] = (uint8_t)(bit_length_32bit >> 16);
                p_bit_length[1] = (uint8_t)(bit_length_32bit >> 8);
                p_bit_length[0] = (uint8_t)(bit_length_32bit >> 0);
            }



            /* Call the HASH HAL module to hash the last block and output a generated message digest. */
            err_crypto = p_hash_api->hashUpdate (p_ctrl->p_lower_lvl_instance->p_ctrl,
                                                 (uint32_t *)p_message_buffer,
                                                 (block_size / 4U),
                                                 (uint32_t *)p_message_digest);
            if ((uint32_t)SSP_SUCCESS == err_crypto)
            {

                /* Successfully a message digest is generated. Return the message digest length. */
                *p_size = sf_crypto_hash_digest_size_get(p_ctrl->hash_type, true);

                /** Special Case for MD5 - swap endian required */
                if (p_ctrl->hash_type == SF_CRYPTO_HASH_ALGORITHM_MD5){

                    p_message_digest32 = (uint32_t *)p_message_digest;

                	/** Swap required for Message Digest for MD5 */
                    for (uint8_t i = 0U; i < (uint8_t)MD5_DIGEST_SIZE_IN_WORDS; i++)
                    {
                       	p_message_digest32[i] = (uint32_t)__REV(p_message_digest32[i]);
                    }
                }

                /* Copy generated message digest to user memory space. */
                memcpy(p_msg_digest->p_data, p_message_digest, p_msg_digest->data_length);
            }
            else
            {
                /* Failed in the HASH HAL module. Return zero as the message digest length. */
                *p_size = 0U;

                /* Failed in HASH HAL driver. Save the error code. */
                err = (ssp_err_t)err_crypto;
            }
        }
    }
    else
    {
        /* An obtained API instance was not valid. */
        err = SSP_ERR_UNSUPPORTED;
    }

    return err;
}
