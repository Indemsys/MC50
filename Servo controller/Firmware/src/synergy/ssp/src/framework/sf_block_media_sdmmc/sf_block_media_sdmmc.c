/***********************************************************************************************************************
 * Copyright [2015-2023] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : sf_block_media_sdmmc.c
 * Description  : Block Media driver interface SDMMC driver for SDHI/SDMMC device.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @file sf_block_media_sdmmc.c
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

#include <string.h>
#include "sf_block_media_sdmmc.h"
#include "sf_block_media_sdmmc_cfg.h"
#include "sf_block_media_sdmmc_private_api.h"
#include "r_sdmmc.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define SF_BLOCK_MEDIA_SDMMC_TRANSFER_TIMEOUT (10000)

/** Macro for error logger. */
#ifndef SF_BLOCK_MEDIA_SDMMC_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define SF_BLOCK_MEDIA_SDMMC_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &g_block_media_sdmmc_version)
#endif

/** "BMSO" in ASCII, used to identify block media SDMMC handle*/
#define SF_BLOCK_MEDIA_SDMMC_OPEN (0x424D534FU)
/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Exported global variables (to be accessed by other files)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void sf_block_media_sdmmc_callback (sdmmc_callback_args_t * pcb_arg);

/***********************************************************************************************************************
 * Global Variables
 **********************************************************************************************************************/

/** Block Media SDMMC function pointers   */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const sf_block_media_api_t g_sf_block_media_on_sdmmc =
{
    .open       = SF_Block_Media_SDMMC_Open,
    .read       = SF_Block_Media_SDMMC_Read,
    .write      = SF_Block_Media_SDMMC_Write,
    .ioctl      = SF_Block_Media_SDMMC_Control,
    .close      = SF_Block_Media_SDMMC_Close,
    .versionGet = SF_Block_Media_SDMMC_VersionGet
};

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "sf_block_media_sdmmc";
#endif

#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug. This pragma suppresses the warnings in this
 * structure only.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Version data structure used by error logger macro. */
static const ssp_version_t g_block_media_sdmmc_version =
{
    .api_version_minor  = BLOCK_MEDIA_API_VERSION_MINOR,
    .api_version_major  = BLOCK_MEDIA_API_VERSION_MAJOR,
    .code_version_major = BLOCK_MEDIA_SDMMC_CODE_VERSION_MAJOR,
    .code_version_minor = BLOCK_MEDIA_SDMMC_CODE_VERSION_MINOR
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/*******************************************************************************************************************//**
 * @addtogroup SF_BLOCK_MEDIA_SDMMC
 *
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief  Open device for read/write and control.
 *
 * Open an SD or MMC device port for read/write and control. This function initializes
 * the SDMMC driver and hardware the first time it is called out of reset.
 *
 * @retval SSP_SUCCESS              Port is available and is now open for read, write, and control access.
 * @retval SSP_ERR_ASSERTION        p_ctrl, p_cfg, p_block_media_cfg or p_sdmmc is NULL.
 * @retval SSP_ERR_INTERNAL         OS service call fails.
 * @retval SSP_ERR_IN_USE           The channel specified has already been opened. No configurations were changed. Call
 *                                  the associated Close function or use associated Control commands to reconfigure the
 *                                  channel.
 * @return                          See @ref Common_Error_Codes or lower level drivers for other possible return codes.
 *                                  This function calls:
 *                                  * sdmmc_api_t::open
 * @note This function is reentrant for different channels. It is not reentrant for the same channel.
 *
 **********************************************************************************************************************/
ssp_err_t SF_Block_Media_SDMMC_Open (sf_block_media_ctrl_t * const p_api_ctrl, sf_block_media_cfg_t const * const p_cfg)
{
    sf_block_media_sdmmc_instance_ctrl_t * p_ctrl = (sf_block_media_sdmmc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t                             ret_val;
    sf_block_media_on_sdmmc_cfg_t const * p_block_media_cfg;
    sdmmc_instance_t                    * p_sdmmc;
    sdmmc_cfg_t                           tmp_lower_lvl_cfg;
    UINT                                  tx_err = TX_SUCCESS;

    p_block_media_cfg = p_cfg->p_extend;
    p_sdmmc           = (sdmmc_instance_t *) p_block_media_cfg->p_lower_lvl_sdmmc;

#if (BLOCK_MEDIA_SDMMC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_block_media_cfg);
    SSP_ASSERT(NULL != p_sdmmc);
#endif

    SF_BLOCK_MEDIA_SDMMC_ERROR_RETURN(SF_BLOCK_MEDIA_SDMMC_OPEN != p_ctrl->open, SSP_ERR_IN_USE);

    /* Temporary copy config parameters during open processing */
    memcpy(&tmp_lower_lvl_cfg, p_sdmmc->p_cfg, sizeof(sdmmc_cfg_t));
    tmp_lower_lvl_cfg.p_callback = sf_block_media_sdmmc_callback;               /* set SSP framework level callback
                                                                                 *function */

    /* Save context for use in ISRs */
    tmp_lower_lvl_cfg.p_context = (void *) p_ctrl;

    ret_val                     = p_sdmmc->p_api->open(p_sdmmc->p_ctrl, &tmp_lower_lvl_cfg);
    if (ret_val == SSP_SUCCESS)
    {
        p_ctrl->p_lower_lvl_sdmmc = p_sdmmc;

        /**  Create SDMMC event flag and put it into context */
        tx_err = tx_event_flags_create(&p_ctrl->eventflag, (CHAR *) "SDMMC Transfer");
        if (tx_err == TX_SUCCESS)
        {
            /**  Mark the stream as open by initializing "BMSO" in its ASCII equivalent. */
            p_ctrl->open = SF_BLOCK_MEDIA_SDMMC_OPEN;
        }
        else
        {
            /** Cleanup before logging the error */
            /* The return codes are not checked because valid parameters are passed here and the Close API does not
             * return an error except for parameter checking */
            p_sdmmc->p_api->close(p_sdmmc->p_ctrl);
            SSP_ERROR_LOG(SSP_ERR_INTERNAL, &g_module_name[0], &g_block_media_sdmmc_version);
            return SSP_ERR_INTERNAL;
        }
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief  Read data from SD/MMC.
 *
 * Read data from an SD or MMC device port.
 *
 * @retval  SSP_SUCCESS                  Data read successfully.
 * @retval  SSP_ERR_ASSERTION            p_ctrl, p_sdmmc or p_dest is NULL.
 * @retval  SSP_ERR_NOT_OPEN             The channel is not opened.
 * @retval  SSP_ERR_INTERNAL             OS service call fails.
 * @retval  SSP_ERR_READ_FAILED          Data read failed.
 * @return                               See @ref Common_Error_Codes or lower level drivers for other possible return codes.
 *                                       This function calls:
 *                                       * sdmmc_api_t::read
 * @note This function is reentrant for different channels. It is not reentrant for the same channel.
 *
 **********************************************************************************************************************/

ssp_err_t SF_Block_Media_SDMMC_Read (sf_block_media_ctrl_t * const p_api_ctrl,
                                     uint8_t * const               p_dest,
                                     uint32_t  const               start_block,
                                     uint32_t  const               block_count)
{
    sf_block_media_sdmmc_instance_ctrl_t * p_ctrl = (sf_block_media_sdmmc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t                              ret_val;
    sdmmc_instance_t                     * p_sdmmc;
    ULONG                                  actual_flags = 0U;
    UINT                                   status = TX_SUCCESS;

    p_sdmmc       = (sdmmc_instance_t *) p_ctrl->p_lower_lvl_sdmmc;

#if (BLOCK_MEDIA_SDMMC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_sdmmc);
    SSP_ASSERT(NULL != p_dest);
#endif

    SF_BLOCK_MEDIA_SDMMC_ERROR_RETURN(SF_BLOCK_MEDIA_SDMMC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    ret_val = p_sdmmc->p_api->read(p_sdmmc->p_ctrl, p_dest, start_block, block_count);

    if (ret_val != SSP_SUCCESS)
    {
        return ret_val;
    }
    else
    {
        /** Wait until read operation is completed. Event is signaled in event flag object. */
        status = tx_event_flags_get(&p_ctrl->eventflag, SDMMC_EVENT_TRANSFER_COMPLETE, TX_OR_CLEAR,
                                    &actual_flags, SF_BLOCK_MEDIA_SDMMC_TRANSFER_TIMEOUT);

        if (TX_SUCCESS != status)
        {
            ret_val = SSP_ERR_INTERNAL;
        }
        else if (SDMMC_EVENT_TRANSFER_COMPLETE == actual_flags)
        {
            ret_val = SSP_SUCCESS;
        }
        else
        {
                ret_val = SSP_ERR_READ_FAILED;
        }
    }
        return ret_val;
}

/*******************************************************************************************************************//**
 * @brief  Write data to SDMMC channel.
 *
 * @retval  SSP_SUCCESS                  Card write finished successfully.
 * @retval  SSP_ERR_ASSERTION            p_ctrl, p_sdmmc or p_src is NULL.
 * @retval  SSP_ERR_NOT_OPEN             The channel is not opened.
 * @retval  SSP_ERR_INTERNAL             OS service call fails.
 * @retval  SSP_ERR_WRITE_FAILED         Data write failed.
 * @return                               See @ref Common_Error_Codes or lower level drivers for other possible return codes.
 *                                       This function calls:
 *                                       * sdmmc_api_t::write
 * @note This function is reentrant for different channels.
 *
 **********************************************************************************************************************/

ssp_err_t SF_Block_Media_SDMMC_Write (sf_block_media_ctrl_t * const p_api_ctrl,
                                      uint8_t  const * const        p_src,
                                      uint32_t const                start_block,
                                      uint32_t const                block_count)
{
    sf_block_media_sdmmc_instance_ctrl_t * p_ctrl = (sf_block_media_sdmmc_instance_ctrl_t *) p_api_ctrl;

    ssp_err_t                              ret_val;
    sdmmc_instance_t                     * p_sdmmc;
    ULONG                                  actual_flags = 0U;
    UINT                                   status = TX_SUCCESS;

    p_sdmmc       = (sdmmc_instance_t *) p_ctrl->p_lower_lvl_sdmmc;

#if (BLOCK_MEDIA_SDMMC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_sdmmc);
    SSP_ASSERT(NULL != p_src);
#endif

    SF_BLOCK_MEDIA_SDMMC_ERROR_RETURN(SF_BLOCK_MEDIA_SDMMC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    ret_val = p_sdmmc->p_api->write(p_sdmmc->p_ctrl,  p_src, start_block, block_count);
    if (ret_val != SSP_SUCCESS)
    {
        return ret_val;
    }
    else
    {
        /** Wait until write operation is completed. Event is signaled in event flag object. */
        status = tx_event_flags_get(&p_ctrl->eventflag, SDMMC_EVENT_TRANSFER_COMPLETE, TX_OR_CLEAR,
                                    &actual_flags, SF_BLOCK_MEDIA_SDMMC_TRANSFER_TIMEOUT);

        if (TX_SUCCESS != status)
        {
            ret_val = SSP_ERR_INTERNAL;
        }
        else if (SDMMC_EVENT_TRANSFER_COMPLETE == actual_flags)
        {
            ret_val = SSP_SUCCESS;
        }
        else
        {
                ret_val = SSP_ERR_WRITE_FAILED;
        }
    }
        return ret_val;

}

/*******************************************************************************************************************//**
 * @brief  Send control commands to and receive status of SD/MMC port.
 *
 * Send control commands to the SD/MMC port and receive the status of the SD/MMC port.
 *
 * @retval SSP_SUCCESS              Command executed successfully.
 * @retval SSP_ERR_ASSERTION        p_ctrl or p_sdmmc or p_data is Null.
 * @retval SSP_ERR_NOT_OPEN         The channel is not opened.
 * @return                          See @ref Common_Error_Codes or lower level drivers for other possible return codes.
 *                                  This function calls:
 *                                  * sdmmc_api_t::control
 * @note This function is reentrant for different channels. It is not reentrant for the same channel.
 *
 **********************************************************************************************************************/
ssp_err_t SF_Block_Media_SDMMC_Control (sf_block_media_ctrl_t * const p_api_ctrl,
                                        ssp_command_t const           command,
                                        void                        * p_data)
{
    sf_block_media_sdmmc_instance_ctrl_t * p_ctrl = (sf_block_media_sdmmc_instance_ctrl_t *) p_api_ctrl;

    sdmmc_instance_t                     * p_sdmmc;
    ssp_err_t                              ret_val;

    p_sdmmc       = (sdmmc_instance_t *) p_ctrl->p_lower_lvl_sdmmc;

#if (BLOCK_MEDIA_SDMMC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_sdmmc);
    SSP_ASSERT(NULL != p_data);
#endif

    SF_BLOCK_MEDIA_SDMMC_ERROR_RETURN(SF_BLOCK_MEDIA_SDMMC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    ret_val = p_sdmmc->p_api->control(p_sdmmc->p_ctrl, command, p_data);

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief      Close open device port.
 *
 * Close an open SD/MMC device port.
 *
 * @retval     SSP_SUCCESS              Successful close.
 * @retval     SSP_ERR_ASSERTION        p_ctrl or p_sdmmc is NULL.
 * @retval     SSP_ERR_NOT_OPEN         The channel is not opened.
 * @note This function is reentrant for different channels. It is not reentrant for the same channel.
 *
 **********************************************************************************************************************/

ssp_err_t SF_Block_Media_SDMMC_Close (sf_block_media_ctrl_t * const p_api_ctrl)
{
    sf_block_media_sdmmc_instance_ctrl_t * p_ctrl = (sf_block_media_sdmmc_instance_ctrl_t *) p_api_ctrl;

    sdmmc_instance_t               * p_sdmmc;

    p_sdmmc            = (sdmmc_instance_t *) p_ctrl->p_lower_lvl_sdmmc;

#if    (BLOCK_MEDIA_SDMMC_CFG_PARAM_CHECKING_ENABLE)
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_sdmmc);
#endif

    SF_BLOCK_MEDIA_SDMMC_ERROR_RETURN(SF_BLOCK_MEDIA_SDMMC_OPEN == p_ctrl->open, SSP_ERR_NOT_OPEN);

    /* The return codes are not checked here because Close API should not return an error except for
     * parameter checking */
    p_sdmmc->p_api->close(p_sdmmc->p_ctrl);

    /* The return codes are not checked here because event flag delete call cannot fail when called
     * with created RTOS objects. This object was successfully created in open function. */
    tx_event_flags_delete(&p_ctrl->eventflag);

    /** Mark control block as unused so it can be reconfigured. */
    p_ctrl->open = 0U;

    return SSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * @brief  Get version of Block Media SD/MMC driver.
 *
 * Return the version of the firmware and API.
 *
 * @retval SSP_ERR_ASSERTION        p_version is Pointer.
 * @retval SSP_SUCCESS              version read successfully.
 * @note This function is reentrant.
 *
 **********************************************************************************************************************/

ssp_err_t  SF_Block_Media_SDMMC_VersionGet (ssp_version_t * const p_version)
{
#if    (BLOCK_MEDIA_SDMMC_CFG_PARAM_CHECKING_ENABLE)
    /* Check pointer for NULL values */
    SSP_ASSERT(NULL != p_version);
#endif

    p_version->version_id = g_block_media_sdmmc_version.version_id;
    return SSP_SUCCESS;
}

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/******************************************************************************************************************//**
 * SDMMC SSP framework level callback
 * @param[in]  pcb_arg Pointer to callback parameters
 * @retval void
 **********************************************************************************************************************/
static void sf_block_media_sdmmc_callback (sdmmc_callback_args_t * pcb_arg)
{
    sf_block_media_sdmmc_instance_ctrl_t * p_ctrl       = (sf_block_media_sdmmc_instance_ctrl_t *) pcb_arg->p_context;
    TX_EVENT_FLAGS_GROUP           * p_flag             = &p_ctrl->eventflag;

    if ((SDMMC_EVENT_TRANSFER_COMPLETE == pcb_arg->event) || (SDMMC_EVENT_TRANSFER_ERROR == pcb_arg->event))
    {
        /** Transfer complete event occurs wake up the suspended thread.  */
        p_flag = &p_ctrl->eventflag;   /* Points location for event flag for transfer. */

        /* The return code is not checked here because event flag set is called with valid input arguments */
        tx_event_flags_set(p_flag, (ULONG)pcb_arg->event, TX_OR);
    }

}  /* End of function sf_block_media_sdmmc_callback() */

/*******************************************************************************************************************//**
 * @} (end addtogroup BLOCK_MEDIA_SDMMC)
 **********************************************************************************************************************/
