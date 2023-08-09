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

/*************************************************************************//*******************************************
 * File Name    : sf_spi.c
 * Description  : Source code for SPI framework API functions.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "tx_api.h"
#include "sf_spi_api.h"
#include "sf_spi.h"
#include "sf_spi_private_api.h"

/*******************************************************************************************************************//**
 * @addtogroup SF_SPI
 * @{
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#ifndef SF_SPI_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define SF_SPI_ERROR_RETURN(expression, error) \
                            SSP_ERROR_RETURN((expression), (error), &g_module_name[0], &g_sf_spi_version)
#endif

/* Internal ThreadX mutex ID, used to know if a mutex is created. */
#define TX_MUTEX_ID                             ((ULONG) 0x4D555445)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
/* Callback functions for AMS framework */
static void sf_spi_callback (spi_callback_args_t * parg);

static ssp_err_t sf_spi_common_start (sf_spi_instance_ctrl_t * const p_ctrl);

static ssp_err_t sf_spi_common_wait (sf_spi_instance_ctrl_t * const p_ctrl, uint32_t const timeout);

static void sf_spi_common_finish (sf_spi_instance_ctrl_t * const p_ctrl);

static ssp_err_t sf_spi_bus_device_check (sf_spi_instance_ctrl_t * const p_ctrl);

static void sf_spi_chipselect_assert (ioport_port_pin_t chip_select, ioport_level_t active_level);

static void sf_spi_chipselect_deassert (ioport_port_pin_t chip_select, ioport_level_t active_level);

#if SF_SPI_CFG_PARAM_CHECKING_ENABLE
static bool sf_spi_check_lower_lvl_driver_parameters (sf_spi_cfg_t const * const p_cfg);
static bool sf_spi_check_common_parameters (sf_spi_ctrl_t * const p_ctrl,
                                            uint32_t const length);
#endif

static ssp_err_t sf_spi_reconfigure_device (sf_spi_instance_ctrl_t * const p_ctrl);

static ssp_err_t sf_spi_lower_lvl_driver_open (sf_spi_instance_ctrl_t * const p_ctrl, sf_spi_cfg_t const * const p_cfg,
                                               spi_cfg_t * tmp_cfg);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
#if defined(__GNUC__)
/* This structure is affected by warnings from the GCC compiler bug gcc.gnu.org/bugzilla/show_bug.cgi?id=60784
 * This pragma suppresses the warnings in this structure only, and will be removed when the SSP compiler is updated to
 * v5.3.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

/** Version data structure used by error logger macro. */
static const ssp_version_t g_sf_spi_version =
{
    .api_version_minor  = SF_SPI_API_VERSION_MINOR,
    .api_version_major  = SF_SPI_API_VERSION_MAJOR,
    .code_version_minor = SF_SPI_CODE_VERSION_MINOR,
    .code_version_major = SF_SPI_CODE_VERSION_MAJOR,
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

#if BSP_CFG_ERROR_LOG != 0
static const char          g_module_name[] = "sf_spi";
#endif

/***********************************************************************************************************************
 * Implementation of Interface
 **********************************************************************************************************************/
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const sf_spi_api_t g_sf_spi_on_sf_spi =
{
    .open      = SF_SPI_Open,
    .read      = SF_SPI_Read,
    .write     = SF_SPI_Write,
    .writeRead = SF_SPI_WriteRead,
    .close     = SF_SPI_Close,
    .lock      = SF_SPI_Lock,
    .lockWait  = SF_SPI_LockWait,
    .unlock    = SF_SPI_Unlock,
    .version   = SF_SPI_VersionGet
};

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/******************************************************************************************************************//**
 * @brief  Initialize a SPI bus and open low level SPI driver. 
 * @retval SSP_SUCCESS                  SPI channel is successfully opened.
 * @retval SSP_ERR_ASSERTION            One of the following parameters is NULL:
 *                                      p_api_ctrl, p_cfg,
 *                                      Pointer to Open, Close, Read, Write, or Writeread API interfaces,
 *                                      p_cfg->p_bus or p_cfg->p_lower_lvl_cfg.
 * @retval SSP_ERR_INTERNAL             Internal error occurred.
 * @retval SSP_ERR_ALREADY_OPEN         Same SPI framework device is already open.
 * @return See @ref Common_Error_Codes and lower level driver function for other possible return codes.
 *         This driver function is
 *         * spi_api_t::open
 * @note This function is reentrant for any channel.
 * @note Control block must be cleared by caller before calling this function.
 **********************************************************************************************************************/
ssp_err_t SF_SPI_Open (sf_spi_ctrl_t * const p_api_ctrl, sf_spi_cfg_t const * const p_cfg)
{
    sf_spi_instance_ctrl_t * p_ctrl = (sf_spi_instance_ctrl_t *) p_api_ctrl;

#if SF_SPI_CFG_PARAM_CHECKING_ENABLE
    /* Perform parameter checking */
    SSP_ASSERT(NULL != p_cfg);
    SSP_ASSERT(NULL != p_ctrl);
    /* Perform Lower level driver parameter checking */
    bool error_flag = true;
    error_flag = sf_spi_check_lower_lvl_driver_parameters(p_cfg);
    SF_SPI_ERROR_RETURN((true == error_flag), SSP_ERR_ASSERTION);

#endif /* if SF_SPI_CFG_PARAM_CHECKING_ENABLE */

    /** Check whether device is already opened or not. */
    SF_SPI_ERROR_RETURN((SF_SPI_DEV_STATE_OPENED != p_ctrl->dev_state), SSP_ERR_ALREADY_OPEN);

    ssp_err_t err = SSP_SUCCESS;
    spi_cfg_t tmp_cfg = {0};
    UINT status = TX_MUTEX_ERROR;

    p_ctrl->p_bus                    = p_cfg->p_bus;                           /** Copy bus to control */
    p_ctrl->chip_select              = p_cfg->chip_select;                     /** Copy chip_select to control */
    p_ctrl->chip_select_level_active = p_cfg->chip_select_level_active;        /** Copy chip_select level to control */
    p_ctrl->locked                   = false;                                  /** Initialize bus lock to false*/

    memcpy(&tmp_cfg, p_cfg->p_lower_lvl_cfg, sizeof(spi_cfg_t));

    /** Set framework level callback function. */
    tmp_cfg.p_callback = sf_spi_callback;

    /** Save context for use in ISRs. */
    tmp_cfg.p_context = (void *) p_ctrl;

    /** Use bus channel in device open. */
    tmp_cfg.channel = p_cfg->p_bus->channel;

    while (TX_SUCCESS != status)
    {
        /** Enter a critical section before checking the device count mutex status. */
        SSP_CRITICAL_SECTION_DEFINE;
        SSP_CRITICAL_SECTION_ENTER;

        /** Check if device count mutex is already created. If not then create the mutex. */
        if (TX_MUTEX_ID != p_ctrl->p_bus->device_count_mutex.tx_mutex_id)
        {
            /** Create device_count_mutex. This is used to protect shared variable device_count
             * in bus control structure. */
            status = tx_mutex_create(&(p_ctrl->p_bus->device_count_mutex), (CHAR *)"SF_SPI device count mutex",
                                     TX_INHERIT);

            if (TX_SUCCESS != status)
            {
                /** If mutex create fails, return error. */
                SSP_ERROR_LOG(SSP_ERR_INTERNAL, &g_module_name[0], &g_sf_spi_version);
                SSP_CRITICAL_SECTION_EXIT;
                return SSP_ERR_INTERNAL;
            }
        }
        /** Exit critical section */
        SSP_CRITICAL_SECTION_EXIT;

        /** Acquire the device count mutex before accessing the shared resource.
         *  Try again if the mutex was deleted in close. */
        status = tx_mutex_get(&(p_ctrl->p_bus->device_count_mutex), TX_WAIT_FOREVER);
    }

    /* Open only for the first device on the bus.*/
    err = sf_spi_lower_lvl_driver_open(p_ctrl, p_cfg, &tmp_cfg);
    SF_SPI_ERROR_RETURN((SSP_SUCCESS == err), err);

    /** Increment device count.*/
    p_ctrl->p_bus->device_count++;

    /* Release the device count mutex and return code is not checked here because tx_mutex_put cannot fail
     * when called with a mutex owned by the current thread.  The mutex is owned by the current thread because
     * this call follows a successful call to tx_mutex_get. */
    tx_mutex_put(&(p_ctrl->p_bus->device_count_mutex));

    /** Save device configuration for reconfiguration.*/
    memcpy(&p_ctrl->lower_lvl_cfg, &tmp_cfg, sizeof(tmp_cfg));

    /** Set device state as Opened. */
    p_ctrl->dev_state = SF_SPI_DEV_STATE_OPENED;

    /** Initialize chip select. */
    sf_spi_chipselect_deassert(p_ctrl->chip_select, p_ctrl->chip_select_level_active);

    return err;
} /* End of function SF_SPI_Open() */

/******************************************************************************************************************//**
 * @brief  Starts the transfer process and receives data from SPI device.
 * @retval SSP_SUCCESS                  Data read completed successfully.
 * @retval SSP_ERR_ASSERTION            One of the following parameters is NULL:
 *                                      p_api_ctrl, p_dest, length.
 * @retval SSP_ERR_NOT_OPEN             Device not opened.
 * @return See @ref Common_Error_Codes and lower level driver function for other possible return codes.
 *         This driver function is:
 *         * spi_api_t::read
 **********************************************************************************************************************/
ssp_err_t SF_SPI_Read (sf_spi_ctrl_t   * const p_api_ctrl,
                       void           * const  p_dest,
                       uint32_t const          length,
                       spi_bit_width_t const   bit_width,
                       uint32_t const          timeout)

{
    sf_spi_instance_ctrl_t * p_ctrl = (sf_spi_instance_ctrl_t *) p_api_ctrl;

#if SF_SPI_CFG_PARAM_CHECKING_ENABLE
    /* Perform parameter checking */
    SSP_ASSERT(NULL != p_dest);
    /* Check the data p_ctrl, length and timeout */
    bool error_flag = true;
    error_flag = sf_spi_check_common_parameters(p_ctrl, length);
    SF_SPI_ERROR_RETURN((true == error_flag), SSP_ERR_ASSERTION);

#endif /* if SF_SPI_CFG_PARAM_CHECKING_ENABLE */

    ssp_err_t err = SSP_SUCCESS;
    UINT status = TX_SUCCESS;

    /** Check whether device is open. */
    SF_SPI_ERROR_RETURN((SF_SPI_DEV_STATE_OPENED == p_ctrl->dev_state), SSP_ERR_NOT_OPEN);

    /** Get mutex for this bus. */
    status = tx_mutex_get(p_ctrl->p_bus->p_lock_mutex, timeout);
    SF_SPI_ERROR_RETURN((TX_SUCCESS == status), SSP_ERR_INTERNAL);

    /** Start transfer process - check lock, check reconfiguration, check bus compatibility, enable chip select. */
    err = sf_spi_common_start(p_ctrl);
    if (SSP_SUCCESS != err)
    {
        /** Release the mutex */
        /* The return code is not checked here because tx_mutex_put cannot fail when called with a
         * mutex owned by the current thread.  The mutex is owned by the current thread because this
         * call follows a successful call to tx_mutex_get. */
        tx_mutex_put(p_ctrl->p_bus->p_lock_mutex);
        SSP_ERROR_LOG(err, &g_module_name[0], &g_sf_spi_version);
        return err;
    }

    /** Perform read. */
    err = p_ctrl->p_bus->p_lower_lvl_api->read(p_ctrl->p_lower_lvl_ctrl, p_dest, length, bit_width);
    if (SSP_SUCCESS == err)
    {
        /* Wait for callback to set event flag. */
        err = sf_spi_common_wait(p_ctrl, timeout);
    }
    if(SSP_SUCCESS != err)
    {
        SSP_ERROR_LOG(err, &g_module_name[0], &g_sf_spi_version);
    }

    /** Finish transfer. */
    sf_spi_common_finish(p_ctrl);

    /* Release mutex. The return code is not checked here because mutex put cannot fail when called with a mutex
     * owned by the current thread. The mutex is owned by the current thread because this call follows a successful
     * call to mutex get from common start function. */
    tx_mutex_put(p_ctrl->p_bus->p_lock_mutex);

    return err;
} /* End of function SF_SPI_Read() */

/******************************************************************************************************************//**
 * @brief  Starts the transfer process and writes data to SPI device.
 * @retval SSP_SUCCESS                  Data write completed successfully.
 * @retval SSP_ERR_ASSERTION            One of the following parameters may be NULL:
 *                                      p_api_ctrl, p_src, length.
 * @retval SSP_ERR_NOT_OPEN             Device not opened.
 * @return See @ref Common_Error_Codes and lower level driver function for other possible return codes.
 *         This driver function is:
 *         * spi_api_t::write
 **********************************************************************************************************************/
ssp_err_t SF_SPI_Write (sf_spi_ctrl_t     * const p_api_ctrl,
                        void           * const    p_src,
                        uint32_t const            length,
                        spi_bit_width_t const     bit_width,
                        uint32_t const            timeout)
{
    sf_spi_instance_ctrl_t * p_ctrl = (sf_spi_instance_ctrl_t *) p_api_ctrl;

#if SF_SPI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_src);
    /* Check the data p_ctrl, length and timeout */
    bool error_flag = true;
    error_flag = sf_spi_check_common_parameters(p_ctrl, length);
    SF_SPI_ERROR_RETURN((true == error_flag), SSP_ERR_ASSERTION);

#endif /* if SF_SPI_CFG_PARAM_CHECKING_ENABLE */

    ssp_err_t err = SSP_SUCCESS;
    UINT status = TX_SUCCESS;

    /** Check whether device is open. */
    SF_SPI_ERROR_RETURN((SF_SPI_DEV_STATE_OPENED == p_ctrl->dev_state), SSP_ERR_NOT_OPEN);

    /** Get mutex for this bus. */
    status = tx_mutex_get(p_ctrl->p_bus->p_lock_mutex, timeout);
    SF_SPI_ERROR_RETURN((TX_SUCCESS == status), SSP_ERR_INTERNAL);

    /** Start transfer process - check lock, check reconfiguration, check bus compatibility, enable chip select. */
    err = sf_spi_common_start(p_ctrl);
    if (SSP_SUCCESS != err)
    {
        /** Release the mutex */
        /* The return code is not checked here because tx_mutex_put cannot fail when called with a
         * mutex owned by the current thread.  The mutex is owned by the current thread because this
         * call follows a successful call to tx_mutex_get. */
        tx_mutex_put(p_ctrl->p_bus->p_lock_mutex);
        SSP_ERROR_LOG(err, &g_module_name[0], &g_sf_spi_version);
        return err;
    }

    /** Perform write. */
    err = p_ctrl->p_bus->p_lower_lvl_api->write(p_ctrl->p_lower_lvl_ctrl, p_src, length, bit_width);
    if (SSP_SUCCESS == err)
    {
        /* Wait for callback to set event flag. */
        err = sf_spi_common_wait(p_ctrl, timeout);
    }
    if(SSP_SUCCESS != err)
    {
        SSP_ERROR_LOG(err, &g_module_name[0], &g_sf_spi_version);
    }

    /** Finish transfer. */
    sf_spi_common_finish(p_ctrl);

    /* Release mutex. The return code is not checked here because mutex put cannot fail when called with a mutex
     * owned by the current thread. The mutex is owned by the current thread because this call follows a successful
     * call to mutex get from common start function. */
    tx_mutex_put(p_ctrl->p_bus->p_lock_mutex);

    return err;
} /* End of function SF_SPI_Write() */

/******************************************************************************************************************//**
 * @brief  Simultaneously transmit data to SPI device while receiving data from SPI device(full duplex).
 * @retval SSP_SUCCESS                  Data write completed successfully.
 * @retval SSP_ERR_ASSERTION            One of the following parameters may be NULL:
 *                                      p_api_ctrl, p_src, p_dest, length.
 * @retval SSP_ERR_NOT_OPEN             Device not opened.
 * @return See @ref Common_Error_Codes and lower level driver function for other possible return codes.
 *         This driver function is:
 *         * spi_api_t::writeRead
 *********************************************************************************************************************/
ssp_err_t SF_SPI_WriteRead (sf_spi_ctrl_t  * const  p_api_ctrl,
                            void            * const p_src,
                            void            * const p_dest,
                            uint32_t const          length,
                            spi_bit_width_t const   bit_width,
                            uint32_t const          timeout)

{
    sf_spi_instance_ctrl_t * p_ctrl = (sf_spi_instance_ctrl_t *) p_api_ctrl;

#if SF_SPI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_src);
    SSP_ASSERT(NULL != p_dest);
    /* Check the data p_ctrl, length and timeout */
    bool error_flag = true;
    error_flag = sf_spi_check_common_parameters(p_ctrl, length);
    SF_SPI_ERROR_RETURN((true == error_flag), SSP_ERR_ASSERTION);

#endif /* if SF_SPI_CFG_PARAM_CHECKING_ENABLE */

    ssp_err_t err = SSP_SUCCESS;
    UINT status = TX_SUCCESS;

    /** Check whether device is open. */
    SF_SPI_ERROR_RETURN((SF_SPI_DEV_STATE_OPENED == p_ctrl->dev_state), SSP_ERR_NOT_OPEN);

    /** Get mutex for this bus. */
    status = tx_mutex_get(p_ctrl->p_bus->p_lock_mutex, timeout);
    SF_SPI_ERROR_RETURN((TX_SUCCESS == status), SSP_ERR_INTERNAL);

    /** Start transfer process - check lock, check reconfiguration, check bus compatibility, enable chip select. */
    err = sf_spi_common_start(p_ctrl);
    if (SSP_SUCCESS != err)
    {
        /** Release the mutex */
        /* The return code is not checked here because tx_mutex_put cannot fail when called with a
         * mutex owned by the current thread.  The mutex is owned by the current thread because this
         * call follows a successful call to tx_mutex_get. */
        tx_mutex_put(p_ctrl->p_bus->p_lock_mutex);
        SSP_ERROR_LOG(err, &g_module_name[0], &g_sf_spi_version);
        return err;
    }

    /** Perform write read. */
    err = p_ctrl->p_bus->p_lower_lvl_api->writeRead(p_ctrl->p_lower_lvl_ctrl, p_src, p_dest, length, bit_width);
    if (SSP_SUCCESS == err)
    {
        /* Wait for callback to set event flag. */
        err = sf_spi_common_wait(p_ctrl, timeout);
    }
    if(SSP_SUCCESS != err)
    {
        SSP_ERROR_LOG(err, &g_module_name[0], &g_sf_spi_version);
    }

    /** Finish transfer. */
    sf_spi_common_finish(p_ctrl);

    /* Release mutex. The return code is not checked here because mutex put cannot fail when called with a mutex
     * owned by the current thread. The mutex is owned by the current thread because this call follows a successful
     * call to mutex get from common start function. */
    tx_mutex_put(p_ctrl->p_bus->p_lock_mutex);

    return err;
} /* End of function SF_SPI_WriteRead() */

/******************************************************************************************************************//**
  * @brief  Disable the SPI device designated by the control handle and close the RTOS services used by the bus
            if no devices are connected to the bus.
 * @retval SSP_SUCCESS                  SPI channel is successfully closed.
 * @retval SSP_ERR_ASSERTION            p_api_ctrl is NULL.
 * @retval SSP_ERR_NOT_OPEN             Device not opened.
 * @return See @ref Common_Error_Codes and lower level driver function for other possible return codes.
 *         This driver function is:
 *         * spi_api_t::close
 * @note This function is reentrant for any device.
 **********************************************************************************************************************/
ssp_err_t SF_SPI_Close (sf_spi_ctrl_t * const p_api_ctrl)
{
    sf_spi_instance_ctrl_t * p_ctrl = (sf_spi_instance_ctrl_t *) p_api_ctrl;

#if SF_SPI_CFG_PARAM_CHECKING_ENABLE
    /* Perform parameter checking. */
    SSP_ASSERT(NULL != p_ctrl);
#endif /* if SF_SPI_CFG_PARAM_CHECKING_ENABLE */

    ssp_err_t err = SSP_SUCCESS;

    /** Check whether device is open. */
    SF_SPI_ERROR_RETURN((SF_SPI_DEV_STATE_OPENED == p_ctrl->dev_state), SSP_ERR_NOT_OPEN);

    /** Acquire the device count mutex before accessing the shared resource in close. */
    /* The return codes are not checked here because Close API should not return an error except for
     * parameter checking */
    tx_mutex_get(&(p_ctrl->p_bus->device_count_mutex), TX_WAIT_FOREVER);

    /** Check the count of opened devices on the bus. If there are no devices opened or all other devices on the bus
     *  are closed then close the low level SPI driver and release the RTOS services used by the bus. */
    if (1U == p_ctrl->p_bus->device_count)
    {
        /** Get the low level control in use.*/
        sf_spi_instance_ctrl_t * p_temp;
        p_temp = (sf_spi_instance_ctrl_t *) (*p_ctrl->p_bus->pp_curr_ctrl);

        /** Close low level driver.*/
        p_ctrl->p_bus->p_lower_lvl_api->close(p_temp->p_lower_lvl_ctrl);

        /** Delete RTOS services used by the bus.*/
        /* The return codes are not checked here because mutex and event_flag delete call cannot fail when called
         * with created RTOS objects. This object was successfully created in open function. */
        tx_mutex_delete(p_ctrl->p_bus->p_lock_mutex);
        tx_event_flags_delete(p_ctrl->p_bus->p_sync_eventflag);

        /** Decrement device count.*/
        p_ctrl->p_bus->device_count--;

        /** Delete the device count mutex */
        /* The return codes are not checked here because mutex delete call cannot fail when called
         * with created RTOS objects. This object was successfully created in open function. */
        tx_mutex_delete(&(p_ctrl->p_bus->device_count_mutex));
    }
    else
    {
        /** Decrement device count.*/
        p_ctrl->p_bus->device_count--;

        /** Release the device count mutex */
        /* The return code is not checked here because mutex put cannot fail when called with a mutex owned by the
         * current thread. The mutex is owned by the current thread because this call follows a successful call to
         * mutex get */
        tx_mutex_put(&(p_ctrl->p_bus->device_count_mutex));
    }

    /** Set device to closed state*/
    p_ctrl->dev_state = SF_SPI_DEV_STATE_CLOSED;

    return err;
}  /* End of function SF_SPI_Close() */

/******************************************************************************************************************//**
 * @brief  Lock the bus for a device.
 * @retval SSP_SUCCESS                  SPI bus is successfully locked.
 * @retval SSP_ERR_ASSERTION            p_api_ctrl is NULL.
 * @retval SSP_ERR_NOT_OPEN             Device not opened.
 * @retval SSP_ERR_IN_USE               In-use error.
 * @note This function is reentrant for any device.
 **********************************************************************************************************************/
ssp_err_t SF_SPI_Lock (sf_spi_ctrl_t * const p_api_ctrl)
{
    sf_spi_instance_ctrl_t * p_ctrl = (sf_spi_instance_ctrl_t *) p_api_ctrl;

#if SF_SPI_CFG_PARAM_CHECKING_ENABLE
    /* Perform parameter checking. */
    SSP_ASSERT(NULL != p_ctrl);
#endif /* if SF_SPI_CFG_PARAM_CHECKING_ENABLE */

    ssp_err_t err = SSP_SUCCESS;
    UINT  status  = TX_SUCCESS;

    /** Check whether device is open. */
    SF_SPI_ERROR_RETURN((SF_SPI_DEV_STATE_OPENED == p_ctrl->dev_state), SSP_ERR_NOT_OPEN);

    /** Get the mutex for this device. */
    status = tx_mutex_get(p_ctrl->p_bus->p_lock_mutex, TX_NO_WAIT);
    SF_SPI_ERROR_RETURN(TX_SUCCESS == status, SSP_ERR_IN_USE);

    /** Start transfer process - check lock, check reconfiguration, check bus compatibility, enable chip select. */
    err = sf_spi_common_start(p_ctrl);
    if (SSP_SUCCESS != err)
    {
        /** Release the mutex */
        /* The return code is not checked here because tx_mutex_put cannot fail when called with a
         * mutex owned by the current thread.  The mutex is owned by the current thread because this
         * call follows a successful call to tx_mutex_get. */
        tx_mutex_put(p_ctrl->p_bus->p_lock_mutex);
        SSP_ERROR_LOG(err, &g_module_name[0], &g_sf_spi_version);
        return err;
    }

    /** Set lock flag. */
    p_ctrl->locked = true;

    return err;
}  /* End of function SF_SPI_Lock() */

/******************************************************************************************************************//**
 * @brief  Unlock the bus for a particular device and make the bus usable for other devices.
 * @retval SSP_SUCCESS                  SPI bus is successfully unlocked.
 * @retval SSP_ERR_ASSERTION            p_api_ctrl is NULL.
 * @retval SSP_ERR_NOT_OPEN             Device not opened.
 * @retval SSP_ERR_IN_USE               In-use error.
 * @note This function is reentrant for any device.
 **********************************************************************************************************************/
ssp_err_t SF_SPI_Unlock (sf_spi_ctrl_t * const p_api_ctrl)
{
    sf_spi_instance_ctrl_t * p_ctrl = (sf_spi_instance_ctrl_t *) p_api_ctrl;

#if SF_SPI_CFG_PARAM_CHECKING_ENABLE
    /* Perform parameter checking. */
    SSP_ASSERT(NULL != p_ctrl);
#endif /* if SF_SPI_CFG_PARAM_CHECKING_ENABLE */

    UINT  status  = TX_SUCCESS;

    /** Check whether device is open. */
    SF_SPI_ERROR_RETURN((SF_SPI_DEV_STATE_OPENED == p_ctrl->dev_state), SSP_ERR_NOT_OPEN);

    /** Acquire the mutex. */
    status = tx_mutex_get(p_ctrl->p_bus->p_lock_mutex, TX_NO_WAIT);
    SF_SPI_ERROR_RETURN(TX_SUCCESS == status, SSP_ERR_IN_USE);

    /* The lock flag should not be cleared and chip select should not be deasserted until
     * unlock() API is called equal number of times the Lock() was called. */
    if(2UL == (p_ctrl->p_bus->p_lock_mutex->tx_mutex_ownership_count))
    {
        /** Clear lock flag. */
        p_ctrl->locked = false;

        /** Disable slave. */
        sf_spi_chipselect_deassert(p_ctrl->chip_select, p_ctrl->chip_select_level_active);
    }

    /* The return code is not checked here because tx_mutex_put cannot fail when called with a mutex owned by
     * the current thread. The mutex is owned by the current thread because this call follows a successful
     * call to tx_mutex_get. */
    tx_mutex_put(p_ctrl->p_bus->p_lock_mutex);

    /** Release the mutex so that others can use the bus. */
    /* The call to tx_mutex_put() will not fail if bus is already locked. If tx_mutex_put() fails
     * it means bus is not locked hence no need to reset the lock flag in case of failure */
    status = tx_mutex_put(p_ctrl->p_bus->p_lock_mutex);
    SF_SPI_ERROR_RETURN(TX_SUCCESS == status, SSP_ERR_IN_USE);

    return SSP_SUCCESS;
}  /* End of function SF_SPI_Unlock() */

/******************************************************************************************************************//**
 * @brief  Get the version information of the framework.
 * @retval SSP_ERR_ASSERTION            p_version is NULL.
 * @retval SSP_SUCCESS                  Successful return.
 **********************************************************************************************************************/
ssp_err_t SF_SPI_VersionGet (ssp_version_t   * const p_version)
{
    /** Checks error. Further parameter checking can be done at the driver layer. */
#if SF_SPI_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(p_version != NULL);
#endif /* if SF_SPI_CFG_PARAM_CHECKING_ENABLE */

    p_version->version_id = g_sf_spi_version.version_id;

    return SSP_SUCCESS;
}  /* End of function SF_SPI_VersionGet() */

/******************************************************************************************************************//**
 * @brief  Lock the SPI bus resource. Once bus is locked by a device it can not be used by other devices.
 *
 * @retval SSP_SUCCESS          SPI channel is successfully locked within the specified timeout.
 * @retval SSP_ERR_ASSERTION    Pointer to SPI control block is NULL.
 * @retval SSP_ERR_NOT_OPEN     Device not opened.
 * @retval SSP_ERR_TIMEOUT      Mutex not available in timeout.
 * @note   This function is reentrant for any device.
 **********************************************************************************************************************/
ssp_err_t SF_SPI_LockWait (sf_spi_ctrl_t * const p_api_ctrl, uint32_t const timeout)
{
    sf_spi_instance_ctrl_t * p_ctrl = (sf_spi_instance_ctrl_t *) p_api_ctrl;

#if SF_SPI_CFG_PARAM_CHECKING_ENABLE
    /* Perform parameter checking. */
    SSP_ASSERT(p_ctrl);
#endif /* if SF_SPI_CFG_PARAM_CHECKING_ENABLE */

    ssp_err_t err = SSP_SUCCESS;
    UINT status = TX_SUCCESS;

    /** Check whether device is open. */
    SF_SPI_ERROR_RETURN((SF_SPI_DEV_STATE_OPENED == p_ctrl->dev_state), SSP_ERR_NOT_OPEN);

    /** Get the mutex for this device. */
    status = tx_mutex_get(p_ctrl->p_bus->p_lock_mutex, timeout);
    SF_SPI_ERROR_RETURN(TX_SUCCESS == status, SSP_ERR_TIMEOUT);

    if (*p_ctrl->p_bus->pp_curr_ctrl != (sf_spi_instance_ctrl_t *) p_ctrl)
    {
        err = sf_spi_bus_device_check(p_ctrl);
        if (SSP_SUCCESS != err)
        {
            /* The return code is not checked here because mutex put cannot fail when called with a mutex owned by the
             * current thread. The mutex is owned by the current thread because this call follows a successful call to
             * mutex get */
            tx_mutex_put(p_ctrl->p_bus->p_lock_mutex);
            SSP_ERROR_LOG(err, &g_module_name[0], &g_sf_spi_version);
            return err;
        }
        err = sf_spi_reconfigure_device(p_ctrl);
        if (SSP_SUCCESS != err)
        {
            /* The return code is not checked here because mutex put cannot fail when called with a mutex owned by the
             * current thread. The mutex is owned by the current thread because this call follows a successful call to
             * mutex get */
            tx_mutex_put(p_ctrl->p_bus->p_lock_mutex);
            SSP_ERROR_LOG(err, &g_module_name[0], &g_sf_spi_version);
            return err;
        }
    }

    /** Set lock flag. */
    p_ctrl->locked = true;

    /** Enable slave. */
    sf_spi_chipselect_assert(p_ctrl->chip_select, p_ctrl->chip_select_level_active);

    return SSP_SUCCESS;
}  /* End of function SF_SPI_LockWait() */

/*******************************************************************************************************************//**
 * @} (end addtogroup SF_SPI)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/******************************************************************************************************************//**
 * @brief  Common SPI transfer start function. Used in all framework read write calls.
 *         This function checks whether there is any need for reconfiguration and if yes then it checks whether device
 *         is supported by the bus. If bus supports the device, reconfigures the bus and then
 *         enables the slave.
 * @param[in] p_ctrl                    Control handle for SPI framework driver context for a device
 * @retval SSP_SUCCESS                  SPI channel is successfully closed
 * @return See @ref Common_Error_Codes and lower level driver function for other possible return codes.
 *         These driver functions are:
 *         * spi_api_t::close
 *         * spi_api_t::open
 * @note This function is reentrant for any device.
 **********************************************************************************************************************/
static ssp_err_t sf_spi_common_start (sf_spi_instance_ctrl_t * const p_ctrl)
{
    bool reconfigure = false;
    ssp_err_t err = SSP_SUCCESS;

    /* Clear event flag to get new events */
    /* The return code is not checked here because flag set cannot fail when
     * called with created RTOS objects. These objects were successfully created in this function.*/
    tx_event_flags_set(p_ctrl->p_bus->p_sync_eventflag, TX_CLEAR_ID, TX_AND);

    if (false == p_ctrl->locked)
    {
        /** See if SPI bus needs to be reconfigured. */
        if (*p_ctrl->p_bus->pp_curr_ctrl != (sf_spi_instance_ctrl_t *) p_ctrl)
        {
            reconfigure = true;

            /** Different device is using bus than last time. Check that bus supports device. */
            err = sf_spi_bus_device_check(p_ctrl);
        }

        if (SSP_SUCCESS == err)
        {
            if (true == reconfigure)
            {
                /** Need to reconfigure. */
                err = sf_spi_reconfigure_device(p_ctrl);
                SF_SPI_ERROR_RETURN((SSP_SUCCESS == err), err);
            }

            /** Enable slave. */
            sf_spi_chipselect_assert(p_ctrl->chip_select, p_ctrl->chip_select_level_active);
        }
    }

    return err;
}

/******************************************************************************************************************//**
 * @brief  Common SPI wait. Waits for an operation to finish.
 * @param[in] p_ctrl                    Control handle for SPI framework driver context for a device
 * @param[in] timeout                   ThreadX timeout.
 * @retval SSP_SUCCESS                  SPI channel is successfully closed.
 * @retval SSP_ERR_INTERNAL             Internal error occurs.
 * @retval SSP_ERR_TRANSFER_ABORTED     The data transfer was aborted.
 * @retval SSP_ERR_UNDERFLOW            Read underflow occurs.
 * @retval SSP_ERR_MODE_FAULT           Mode fault error occurs.
 * @retval SSP_ERR_READ_OVERFLOW        Read overflow occurs.
 * @retval SSP_ERR_PARITY               Parity error occurs.
 * @retval SSP_ERR_OVERRUN              Overrun error occurs.
 * @retval SSP_ERR_TIMEOUT              'SPI_EVENT_TRANSFER_COMPLETE' timeout.
 * @note This function is reentrant for any device.
 **********************************************************************************************************************/
static ssp_err_t sf_spi_common_wait (sf_spi_instance_ctrl_t * const p_ctrl,
                                     uint32_t const        timeout)
{
    ssp_err_t err = SSP_SUCCESS;
    UINT status = TX_SUCCESS;
    sf_spi_instance_ctrl_t * ptemp;
    ULONG actual_flags = 0U;
    UINT event = ( (UINT) (1U << SPI_EVENT_TRANSFER_COMPLETE) |
                 (UINT) (1U << SPI_EVENT_TRANSFER_ABORTED) |
                 (UINT) (1U << SPI_EVENT_ERR_MODE_FAULT) |
                 (UINT) (1U << SPI_EVENT_ERR_READ_OVERFLOW) |
                 (UINT) (1U << SPI_EVENT_ERR_PARITY) |
                 (UINT) (1U << SPI_EVENT_ERR_OVERRUN) |
                 (UINT) (1U << SPI_EVENT_ERR_FRAMING) |
                 (UINT) (1U << SPI_EVENT_ERR_MODE_UNDERRUN));

    /* Wait for operation to finish. */
    status = tx_event_flags_get(p_ctrl->p_bus->p_sync_eventflag, (event), TX_OR_CLEAR,
                                &actual_flags, timeout);
    if (TX_SUCCESS != status)
    {
        if (TX_NO_EVENTS == status)
        {
            ptemp = (sf_spi_instance_ctrl_t *) (*p_ctrl->p_bus->pp_curr_ctrl); /** Get the current device.*/

            /** Close the device currently using the bus. */
            p_ctrl->p_bus->p_lower_lvl_api->close(ptemp->p_lower_lvl_ctrl);

            /** Reassign bus for  device to use. */
            err = p_ctrl->p_bus->p_lower_lvl_api->open(p_ctrl->p_lower_lvl_ctrl, &p_ctrl->lower_lvl_cfg);
            SF_SPI_ERROR_RETURN(SSP_SUCCESS == err, err);

            err = SSP_ERR_TIMEOUT;
        }
        else
        {
            err = SSP_ERR_INTERNAL;
        }
        return err;
    }

    if (0UL == ( (ULONG) (1U << SPI_EVENT_TRANSFER_COMPLETE) & actual_flags))
    {
        if ( (ULONG) (1U << SPI_EVENT_ERR_MODE_UNDERRUN) & actual_flags)
        {
            err = SSP_ERR_UNDERFLOW;
        }
        else if ( (ULONG) (1U << SPI_EVENT_ERR_MODE_FAULT) & actual_flags)
        {
            err = SSP_ERR_MODE_FAULT;
        }
        else if ( (ULONG) (1U << SPI_EVENT_ERR_READ_OVERFLOW) & actual_flags)
        {
            err = SSP_ERR_READ_OVERFLOW;
        }
        else if ( (ULONG) (1U << SPI_EVENT_ERR_PARITY) & actual_flags)
        {
            err = SSP_ERR_PARITY;
        }
        else if ( (ULONG) (1U << SPI_EVENT_ERR_OVERRUN) & actual_flags)
        {
            err = SSP_ERR_OVERRUN;
        }
        else
        {
            err = SSP_ERR_TRANSFER_ABORTED;
        }
    }

    return err;
}

/******************************************************************************************************************//**
 * @brief  Common SPI finish. Deassert chip select.
 * @param[in]  p_ctrl                   Control handle for SPI framework driver context for a device
 * @note This function is reentrant for any device.
 **********************************************************************************************************************/
static void sf_spi_common_finish (sf_spi_instance_ctrl_t * const p_ctrl)
{
    if (false == p_ctrl->locked)
    {
        /* Disable slave. */
        sf_spi_chipselect_deassert(p_ctrl->chip_select, p_ctrl->chip_select_level_active);
    }
}

/******************************************************************************************************************//**
 * @brief  SPI device check, checks device compatibility on the bus.
 * @param[in]  p_ctrl                   Control handle for SPI framework driver context for a device
 * @retval SSP_SUCCESS                  Selected device supported by the SPI bus
 * @retval SSP_ERR_UNSUPPORTED          Selected device not supported by the SPI bus
 * @note This function is reentrant for any device.
 **********************************************************************************************************************/
static ssp_err_t sf_spi_bus_device_check (sf_spi_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;

    /* Check to make sure the bus supports this device. */
    if (p_ctrl->p_bus->freq_hz_min > p_ctrl->lower_lvl_cfg.bitrate)
    {
        /* Baud rate not in range. */
        err = SSP_ERR_UNSUPPORTED;
    }

    return err;
}

/******************************************************************************************************************//**
 * @brief  SPI SSP framework level callback
 * @param[in]  pcb_arg                  Pointer to callback parameters
 **********************************************************************************************************************/
static void sf_spi_callback (spi_callback_args_t * pcb_arg)
{
    sf_spi_instance_ctrl_t * volatile p_ctrl  = (sf_spi_instance_ctrl_t *) pcb_arg->p_context;
    TX_EVENT_FLAGS_GROUP * volatile pevtflg = p_ctrl->p_bus->p_sync_eventflag;

    /** Event occurs wake up the suspended thread. */
    pevtflg = p_ctrl->p_bus->p_sync_eventflag;   /** Points location for event flag for reception */

    /** Set flag to trigger waiting thread. */
    /* The return code is not checked here because flag set cannot fail when
     * called with created RTOS objects. These objects were successfully created in this function.*/
    tx_event_flags_set((TX_EVENT_FLAGS_GROUP *)pevtflg, (ULONG) (1U << (pcb_arg->event)), TX_OR);
}

/******************************************************************************************************************//**
 * @brief  SPI SSP framework level chip select utility function
 * @param[in]  chip_select              Chip select pin
 * @param[in]  active_level             Active high or active low
 **********************************************************************************************************************/
static void sf_spi_chipselect_assert(ioport_port_pin_t chip_select, ioport_level_t active_level)
{
    ((active_level) == IOPORT_LEVEL_LOW ? g_ioport_on_ioport.pinWrite((chip_select), IOPORT_LEVEL_LOW)
     : g_ioport_on_ioport.pinWrite(chip_select, IOPORT_LEVEL_HIGH));
}

/******************************************************************************************************************//**
 * @brief  SPI SSP framework level chip select utility function
 * @param[in]  chip_select              Chip select pin
 * @param[in]  active_level             Active high or active low
 **********************************************************************************************************************/
static void sf_spi_chipselect_deassert(ioport_port_pin_t chip_select, ioport_level_t active_level)
{
    ((active_level) == IOPORT_LEVEL_LOW ? g_ioport_on_ioport.pinWrite((chip_select), IOPORT_LEVEL_HIGH)
     : g_ioport_on_ioport.pinWrite(chip_select, IOPORT_LEVEL_LOW));
}

#if SF_SPI_CFG_PARAM_CHECKING_ENABLE
/******************************************************************************************************************//**
 * @brief  Checks whether lower level SPI module and bus are defined
 * @param[in]  p_cfg                    Pointer to SPI framework Configuration Structure
 * @retval true                         Lower level SPI driver, and SPI bus are configured
 * @retval false                        Lower level SPI driver, and SPI bus are not configured
 **********************************************************************************************************************/
static bool sf_spi_check_lower_lvl_driver_parameters(sf_spi_cfg_t const * const p_cfg)
{
    bool ret = true;

    /* Perform parameter checking. */
    if (((((NULL == p_cfg->p_bus->p_lower_lvl_api->open)
            || (NULL == p_cfg->p_bus->p_lower_lvl_api->close))
            || (NULL == p_cfg->p_bus->p_lower_lvl_api->read)))
            || (NULL == p_cfg->p_bus->p_lower_lvl_api->write)
            || (NULL == p_cfg->p_bus->p_lower_lvl_api->writeRead)
            || (NULL == p_cfg->p_bus)
            || (NULL == p_cfg->p_lower_lvl_cfg))
    {
        ret = false;
    }

    return ret;
}

/******************************************************************************************************************//**
 * @brief  Checks if SPI framework control block address, length and timeout are NULL.

 * @param[in]  p_ctrl                   Pointer to SPI framework control block
 * @param[in]  length                   Number of bytes of data to be transferred
 * @retval true                         SPI framework control block address, length are not NULL.
 * @retval false                        SPI framework control block address, length are NULL.
 **********************************************************************************************************************/
static bool sf_spi_check_common_parameters(sf_spi_ctrl_t * const p_ctrl, uint32_t const length)
{
    bool ret = true;

    if ((NULL == p_ctrl) || (0 == length))
    {
        ret = false;
    }

    return ret;
}
#endif

/******************************************************************************************************************//**
 * @brief  Assign a new device address to current device.
 * @param[in]  p_ctrl                   Control handle for SPI framework context for a device
 * @retval SSP_SUCCESS                  New device address assigned to current device
 * @return See @ref Common_Error_Codes and lower level drivers for other possible return codes.
 *         These driver functions are:
 *         * spi_api_t::close
 *         * spi_api_t::open
 **********************************************************************************************************************/
static ssp_err_t sf_spi_reconfigure_device(sf_spi_instance_ctrl_t * const p_ctrl)
{
    ssp_err_t err = SSP_SUCCESS;

    sf_spi_instance_ctrl_t * ptemp;
    ptemp = (sf_spi_instance_ctrl_t *) (*p_ctrl->p_bus->pp_curr_ctrl); /** Get the current device.*/

    /** Close the device currently using the bus. */
    err = p_ctrl->p_bus->p_lower_lvl_api->close(ptemp->p_lower_lvl_ctrl);
    SF_SPI_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Assign bus for the new device to use. */
    err = p_ctrl->p_bus->p_lower_lvl_api->open(p_ctrl->p_lower_lvl_ctrl, &p_ctrl->lower_lvl_cfg);
    SF_SPI_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Assign this device to current. */
    *p_ctrl->p_bus->pp_curr_ctrl = (sf_spi_instance_ctrl_t *) p_ctrl;

    return err;
}

/******************************************************************************************************************//**
 * @brief  Opens lower level driver on the bus and creates RTOS resources.
 * @param[in,out]  p_ctrl                   Control handle for SPI framework context for a device.
 * @param[in]      p_cfg                    Pointer to SPI framework Configuration Structure.
 * @param[in]      tmp_cfg                  Low level configuration used to configure the low level driver.
 * @retval SSP_SUCCESS                      New device address assigned to current device
 * @retval SSP_ERR_INTERNAL                 Internal error occurred.
 * @return See @ref Common_Error_Codes and lower level drivers for other possible return codes.
 *         These driver functions are:
 *         * spi_api_t::open
 **********************************************************************************************************************/
static ssp_err_t sf_spi_lower_lvl_driver_open (sf_spi_instance_ctrl_t * const p_ctrl, sf_spi_cfg_t const * const p_cfg,
                                               spi_cfg_t * tmp_cfg)
{
    ssp_err_t err = SSP_SUCCESS;
    /* Open only for the first device on the bus.
     * If the device requires a bus reconfiguration then that will happen when a later read/write occurs. */
    if (0U == p_ctrl->p_bus->device_count)
    {
        UINT status = TX_SUCCESS;

        /** Create mutex to protect bus transfers. */
        status = tx_mutex_create(p_ctrl->p_bus->p_lock_mutex, (CHAR *) (p_ctrl->p_bus->p_bus_name), TX_INHERIT);
        if (TX_SUCCESS != status)
        {
            /* Return codes are not checked in error condition processing because these objects were created
             * during this function. */
            tx_mutex_delete(&(p_ctrl->p_bus->device_count_mutex));
            SSP_ERROR_LOG(SSP_ERR_INTERNAL, &g_module_name[0], &g_sf_spi_version);
            return SSP_ERR_INTERNAL;
        }

        /** Create Event flag for this bus. */
        status = tx_event_flags_create(p_ctrl->p_bus->p_sync_eventflag, (CHAR *) (p_ctrl->p_bus->p_bus_name));
        if (TX_SUCCESS != status)
        {
            /* Return codes are not checked in error condition processing because these objects were created
             * during this function. */
            tx_mutex_delete(p_ctrl->p_bus->p_lock_mutex);
            tx_mutex_delete(&(p_ctrl->p_bus->device_count_mutex));
            SSP_ERROR_LOG(SSP_ERR_INTERNAL, &g_module_name[0], &g_sf_spi_version);
            return SSP_ERR_INTERNAL;
        }

        /** Open the low level SPI module. */
        err = p_cfg->p_bus->p_lower_lvl_api->open(p_ctrl->p_lower_lvl_ctrl, tmp_cfg);
        if (SSP_SUCCESS != err)
        {
            /* Return codes are not checked in error condition processing because these objects were created
             * during this function. */
            tx_mutex_delete(p_ctrl->p_bus->p_lock_mutex);
            tx_event_flags_delete(p_ctrl->p_bus->p_sync_eventflag);
            tx_mutex_delete(&(p_ctrl->p_bus->device_count_mutex));
            SSP_ERROR_LOG(err, &g_module_name[0], &g_sf_spi_version);
            return err;
        }

        /** Assign last used device ctrl on this bus. */
        *p_ctrl->p_bus->pp_curr_ctrl = (sf_spi_instance_ctrl_t *) p_ctrl;

    }
    return err;
}

/* End of file */
