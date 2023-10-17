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
 * File Name    : sf_el_ux_comms.c
 * Description  : USBX CDC ACM device implementation of communications API.
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "sf_el_ux_comms_v2.h"

#include "sf_el_ux_comms_private_api.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#ifndef SF_EL_UX_COMMS_ERROR_RETURN
/*LDRA_INSPECTED 77 S This macro does not work when surrounded by parentheses. */
#define SF_EL_UX_COMMS_ERROR_RETURN(a, err) SSP_ERROR_RETURN((a), (err), &g_module_name[0], &module_version)
#endif

#define SF_EL_UX_COMMS_MAX_PACKET_SIZE     (64U)

/** "UXCM" in ASCII, used to identify USBX comms handle */
#define OPEN (0x5558434DU)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void sf_el_ux_comms_read_leftover(sf_el_ux_comms_instance_ctrl_t * p_ctrl, uint8_t * const p_dest, uint32_t * p_bytes_remaining);
static ssp_err_t  sf_el_ux_comms_parameter_check(sf_el_ux_comms_instance_ctrl_t * p_ctrl, uint8_t const * const p_dest_or_src);
#if SF_EL_UX_COMMS_CFG_OPEN_CLOSE_ENABLE
extern void sf_el_ux_comms_device_open(void);
extern UINT sf_el_ux_comms_device_close(void);
#endif

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/
#if defined(__GNUC__)
/* This structure is affected by warnings from a GCC compiler bug.  This pragma suppresses the warnings in this
 * structure only.*/
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
/** Module version data structure */
static const ssp_version_t module_version =
{
    .api_version_minor  = SF_COMMS_API_VERSION_MINOR,
    .api_version_major  = SF_COMMS_API_VERSION_MAJOR,
    .code_version_major = SF_EL_UX_COMMS_CODE_VERSION_MAJOR,
    .code_version_minor = SF_EL_UX_COMMS_CODE_VERSION_MINOR,
};
#if defined(__GNUC__)
/* Restore warning settings for 'missing-field-initializers' to as specified on command line. */
/*LDRA_INSPECTED 69 S */
#pragma GCC diagnostic pop
#endif

/** Name of module used by error logger macro */
#if BSP_CFG_ERROR_LOG != 0
static const char g_module_name[] = "sf_el_ux_comms";
#endif

/** API mapping for communications interface */
/*LDRA_INSPECTED 27 D This structure must be accessible in user code. It cannot be static. */
const sf_comms_api_t  g_sf_el_ux_comms_on_sf_comms =
{
    .open       = SF_EL_UX_COMMS_Open,
    .close      = SF_EL_UX_COMMS_Close,
    .write      = SF_EL_UX_COMMS_Write,
    .read       = SF_EL_UX_COMMS_Read,
    .lock       = SF_EL_UX_COMMS_Lock,
    .unlock     = SF_EL_UX_COMMS_Unlock,
    .versionGet = SF_EL_UX_COMMS_VersionGet
};

/*******************************************************************************************************************//**
 * @addtogroup SF_EL_UX_COMMS_V2
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief Initializes a USB channel for CDC ACM mode.
 *
 * @param[in]  p_api_ctrl          Pointer to control structure block.
 * @param[in]  p_cfg               Pointer to configuration structure block. This parameter is not used in the framework
 *                                 hence the NULL parameter check not implemented.
 * @retval     SSP_SUCCESS         Channel opened successfully.
 * @retval     SSP_ERR_ASSERTION   p_api_ctrl pointer parameter to control block is NULL.
 * @retval     SSP_ERR_TIMEOUT     Semaphore not available in timeout.
 * @retval     SSP_ERR_INTERNAL    Transmit/Receive mutex or Semaphore creation fails.
 * @retval     SSP_ERR_IN_USE      Channel/peripheral is running/busy.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t SF_EL_UX_COMMS_Open (sf_comms_ctrl_t * const p_api_ctrl, sf_comms_cfg_t const * const p_cfg)
{
    SSP_PARAMETER_NOT_USED (p_cfg);
    UINT error;
    sf_el_ux_comms_instance_ctrl_t * p_ctrl = (sf_el_ux_comms_instance_ctrl_t *) p_api_ctrl;

#if SF_EL_UX_COMMS_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
#endif

    /* Reconfiguring not permitted at this time. */
    SF_EL_UX_COMMS_ERROR_RETURN(OPEN != p_ctrl->open, SSP_ERR_IN_USE);

#if SF_EL_UX_COMMS_CFG_OPEN_CLOSE_ENABLE
    /** Initialize the USB controller in device mode and configure in CDC-ACM class */
    sf_el_ux_comms_device_open();
#endif

    if(NULL == p_ctrl->p_cdc)
    {
        /** Create semaphore if the USBX CDC instance is not ready. */
        error = tx_semaphore_create(&p_ctrl->semaphore, (CHAR *) "UX_COMMS_SEMAPHORE", 0);

        /**If the semaphore creation fails, return error. */
        SF_EL_UX_COMMS_ERROR_RETURN(TX_SUCCESS == error, SSP_ERR_INTERNAL);

        /** Suspend here until a USBX CDC instance is created by the USBX CDC for this module. */
        error = tx_semaphore_get(&p_ctrl->semaphore, SF_EL_UX_COMMS_CFG_BUFFER_TIMEOUT_COUNT);
        if(TX_SUCCESS != error)
        {
            return SSP_ERR_TIMEOUT;
        }
    }

    /** Create the mutex for protecting the access to control structure for transmit and related hardware*/
    error = tx_mutex_create(&p_ctrl->mutex[0], (CHAR *) "SF_EL_UX_COMMS TX", TX_INHERIT);
    if(TX_SUCCESS == error)
    {
        /** Create the mutex for protecting the access to control structure for receive and related hardware*/
        error = tx_mutex_create(&p_ctrl->mutex[1], (CHAR *) "SF_EL_UX_COMMS RX", TX_INHERIT);

        /* If the Receive mutex fails, delete Transmit mutex. */
        if(TX_SUCCESS != error)
        {
            /* The return codes are not checked because delete cannot fail
               when called with successfully created RTOS objects.*/
            tx_mutex_delete(&p_ctrl->mutex[0]);
        }
    }

    SF_EL_UX_COMMS_ERROR_RETURN(TX_SUCCESS == error, SSP_ERR_INTERNAL);

    /** Mark control block open. */
    p_ctrl->open = OPEN;

    return SSP_SUCCESS;

}  /* End of function SF_EL_UX_COMMS_Open() */

/*******************************************************************************************************************//**
 * @brief   Releases all the ThreadX Resources.
 *
 * @retval  SSP_SUCCESS              Channel successfully closed
 * @retval  SSP_ERR_ASSERTION        Pointer to control block is NULL
 * @retval  SSP_ERR_NOT_OPEN         Module is not opened.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t SF_EL_UX_COMMS_Close (sf_comms_ctrl_t * const p_api_ctrl)
{
    sf_el_ux_comms_instance_ctrl_t * p_ctrl = (sf_el_ux_comms_instance_ctrl_t *) p_api_ctrl;

#if SF_EL_UX_COMMS_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
#endif
    /** Check if module has been opened. */
    SF_EL_UX_COMMS_ERROR_RETURN((OPEN == p_ctrl->open), SSP_ERR_NOT_OPEN);

#if SF_EL_UX_COMMS_CFG_OPEN_CLOSE_ENABLE
    UINT error = SSP_SUCCESS;
    /** Un-initialize the USB controller */
    error = sf_el_ux_comms_device_close();
    /** Return an error, if USB Un-initialize is failed */
    if(SSP_SUCCESS != error)
    {
        return SSP_ERR_ASSERTION;
    }
#endif

    /** Delete transmit mutex. */
    /* The return codes are not checked because delete cannot fail
       when called with successfully created RTOS objects.*/
    tx_mutex_delete(&p_ctrl->mutex[0]);

    /** Delete receive mutex. */
    /* The return codes are not checked because delete cannot fail
       when called with successfully created RTOS objects.*/
    tx_mutex_delete(&p_ctrl->mutex[1]);

    /** Deletes a semaphore for USBX CDC instance */
    /* The return codes are not checked because delete cannot fail
       when called with successfully created RTOS objects.*/
    tx_semaphore_delete(&p_ctrl->semaphore);
    p_ctrl->open = 0U ;

    return SSP_SUCCESS;
}   /* End of function SF_EL_UX_COMMS_Close() */

/*******************************************************************************************************************//**
 * @brief Read data from the USBX CDC-ACM driver
 *
 * @retval  SSP_SUCCESS                  Data reception ends successfully.
 * @retval  SSP_ERR_INTERNAL             An error has occurred if usb read operation fails or buffer overflow occurred.
 * @retval  SSP_ERR_TIMEOUT              Receive mutex get timed out
 *
 * @note This API is reentrant.
 **********************************************************************************************************************/
ssp_err_t SF_EL_UX_COMMS_Read (sf_comms_ctrl_t * const p_api_ctrl, uint8_t * const p_dest, uint32_t const bytes, UINT const timeout)
{
    uint32_t bytes_remaining = bytes;
    UINT tx_err;
    sf_el_ux_comms_instance_ctrl_t * p_ctrl = (sf_el_ux_comms_instance_ctrl_t *) p_api_ctrl;

    /* Do the error code parameter check for module to be opened or not and USB to be plugged in or not.  */
    ssp_err_t err = sf_el_ux_comms_parameter_check(p_ctrl, p_dest);
    SF_EL_UX_COMMS_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Get mutex. */
    tx_err = tx_mutex_get(&p_ctrl->mutex[1], timeout);
    SF_EL_UX_COMMS_ERROR_RETURN(TX_SUCCESS == tx_err, SSP_ERR_TIMEOUT);

    /** Set timeout value in the transfer request. */
    ux_device_class_cdc_acm_ioctl(p_ctrl->p_cdc, UX_SLAVE_CLASS_CDC_ACM_IOCTL_SET_READ_TIMEOUT, (ULONG*)timeout);

    /** If there is data leftover from the last packet, use it. */
    sf_el_ux_comms_read_leftover(p_ctrl, p_dest, &bytes_remaining);

    UINT status;
    while (bytes_remaining > 0U)
    {
        /** Read from the CDC class.  */
        uint32_t actual_length = 0U;
        uint32_t size = SF_EL_UX_COMMS_CFG_BUFFER_MAX_LENGTH;
        if (0U == (bytes % SF_EL_UX_COMMS_MAX_PACKET_SIZE))
        {
            /* A USB CDC transfer is complete when all requested bytes or a packet size less than wMaxPacketSize is
             * received. If the buffer size is a multiple of wMaxPacketSize, the exact requested size must be
             * specified.  Otherwise, the maximum buffer length will be requested to ensure overflow data is
             * not lost. */
            size = bytes;
        }
        size = SF_EL_UX_COMMS_CFG_BUFFER_MAX_LENGTH;
        status = _ux_device_class_cdc_acm_read(p_ctrl->p_cdc, (uint8_t *) &p_ctrl->rx_memory[0],
                 size, (ULONG *) &actual_length);

        /** Release mutex in case of buffer overflow or read error .*/
        if((0U == actual_length) || ((UINT) UX_SUCCESS != status))
        {
            /* The return code is not checked here because tx_mutex_put cannot fail when called with a mutex owned by the
             * current thread.  The mutex is owned by the current thread because this call follows a successful call to
             * tx_mutex_get. */
            tx_mutex_put(&p_ctrl->mutex[1]);
            if((UINT) TX_NO_INSTANCE == status)
            {
                return SSP_ERR_TIMEOUT;
            }

            SSP_ERROR_LOG(SSP_ERR_INTERNAL, &g_module_name[0], &module_version);
            return SSP_ERR_INTERNAL;
        }
        uint32_t transfer_length = actual_length;
        if (actual_length > bytes_remaining)
        {
            transfer_length = bytes_remaining;
            p_ctrl->index = transfer_length;
            p_ctrl->leftover_length = actual_length - bytes_remaining;
        }
        _ux_utility_memory_copy(p_dest + (bytes - bytes_remaining), &p_ctrl->rx_memory, transfer_length);
        bytes_remaining -= transfer_length;
    }

    /** Release mutex for Read API. */
    /* The return code is not checked here because tx_mutex_put cannot fail when called with a mutex owned by the
     * current thread.  The mutex is owned by the current thread because this call follows a successful call to
     * tx_mutex_get. */
    tx_err = tx_mutex_put(&p_ctrl->mutex[1]);

    return SSP_SUCCESS;
}  /* End of function SF_EL_UX_COMMS_Read() */

/*******************************************************************************************************************//**
 * @brief Write data to the USBX CDC-ACM framework.
 *
 * @retval  SSP_SUCCESS                  Data transmission finished successfully.
 * @retval  SSP_ERR_INTERNAL             An error has occurred, when usb write operation fails.
 * @retval  SSP_ERR_TIMEOUT              Transmit mutex get timed out or when DTR and RTS state setting gets timed out.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t SF_EL_UX_COMMS_Write (sf_comms_ctrl_t * const p_api_ctrl, uint8_t const * const p_src, uint32_t const bytes, UINT const timeout)
{
    UINT tx_err;
    sf_el_ux_comms_instance_ctrl_t * p_ctrl = (sf_el_ux_comms_instance_ctrl_t *) p_api_ctrl;

    /* Do the error code parameter check for module is opened or not and USB to be plugged in or not.  */
    ssp_err_t err = sf_el_ux_comms_parameter_check(p_ctrl, p_src);
    SF_EL_UX_COMMS_ERROR_RETURN(SSP_SUCCESS == err, err);

    /** Wait for DTR and RTS state to set. */
    if (((UCHAR)UX_FALSE == p_ctrl->p_cdc->ux_slave_class_cdc_acm_data_dtr_state) && ((UCHAR)UX_FALSE == p_ctrl->p_cdc->ux_slave_class_cdc_acm_data_rts_state))
    {
    	SSP_ERROR_LOG(SSP_ERR_TIMEOUT, &g_module_name[0], &module_version);
    	return SSP_ERR_TIMEOUT;
    }

    /** Get Transmit mutex. */
    tx_err = tx_mutex_get(&p_ctrl->mutex[0], timeout);
    SF_EL_UX_COMMS_ERROR_RETURN((UINT)TX_SUCCESS == tx_err, SSP_ERR_TIMEOUT);

    /** Set timeout value in the transfer request. */
    ux_device_class_cdc_acm_ioctl(p_ctrl->p_cdc, UX_SLAVE_CLASS_CDC_ACM_IOCTL_SET_WRITE_TIMEOUT, (ULONG *)timeout);

    /* Send requested number of bytes.  */
    uint32_t actual_length = 0;
    UINT status = _ux_device_class_cdc_acm_write(p_ctrl->p_cdc, (uint8_t *) p_src, bytes, (ULONG *) &actual_length);

    /** Release mutex in case of error condition and log the error .*/
    if((UINT) UX_SUCCESS == status)
    {
        /* Send 0 byte packet. Forced ZLP.  */
        status =  _ux_device_class_cdc_acm_write(p_ctrl->p_cdc, (uint8_t *) p_src, 0, (ULONG *) &actual_length);
    }

    /** Release Transmit mutex for Write API. */
    /* The return code is not checked here because tx_mutex_put cannot fail when called with a mutex owned by the
     * current thread.  The mutex is owned by the current thread because this call follows a successful call to
     * tx_mutex_get. */
    tx_err = tx_mutex_put(&p_ctrl->mutex[0]);
    if((UINT) TX_NO_INSTANCE == status)
    {
        return SSP_ERR_TIMEOUT;
    }

    SF_EL_UX_COMMS_ERROR_RETURN(((UINT)UX_SUCCESS == status), SSP_ERR_INTERNAL);


    return SSP_SUCCESS;
}  /* End of function SF_EL_UX_COMMS_Write() */

/******************************************************************************************************************//**
 * @brief Lock the USB COM resource.
 *
 * @retval SSP_SUCCESS          Locking a USB COM resource successful.
 * @retval SSP_ERR_ASSERTION    Pointer to control block is NULL.
 * @retval SSP_ERR_NOT_OPEN     Module is not opened.
 * @retval SSP_ERR_TIMEOUT      Mutex not available in timeout.
**********************************************************************************************************************/
ssp_err_t SF_EL_UX_COMMS_Lock (sf_comms_ctrl_t * const p_api_ctrl, sf_comms_lock_t lock_type, UINT timeout)
{
    sf_el_ux_comms_instance_ctrl_t * p_ctrl = (sf_el_ux_comms_instance_ctrl_t *) p_api_ctrl;

#if SF_EL_UX_COMMS_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
#endif

    SF_EL_UX_COMMS_ERROR_RETURN((OPEN == p_ctrl->open), SSP_ERR_NOT_OPEN);

    UINT tx_err;

    /** If lock type is ALL, both TX and RX gets locked else either TX or RX is locked. */
    if (SF_COMMS_LOCK_ALL == lock_type)
    {
        tx_err = tx_mutex_get(&p_ctrl->mutex[0], timeout);
        if(TX_SUCCESS == tx_err)
        {
            tx_err = tx_mutex_get(&p_ctrl->mutex[1], timeout);
            if(TX_SUCCESS != tx_err)
            {
                /* The return code is not checked here because tx_mutex_put cannot fail when called with a mutex owned by the
                 * current thread.  The mutex is owned by the current thread because this call follows a successful call to
                 * tx_mutex_get. */
                tx_mutex_put(&p_ctrl->mutex[0]);
            }
        }
    }
    else
    {
        tx_err = tx_mutex_get(&p_ctrl->mutex[lock_type], timeout);
    }

    /** If TX or RX fails to acquire mutex, return error. */
    SF_EL_UX_COMMS_ERROR_RETURN(TX_SUCCESS == tx_err, SSP_ERR_TIMEOUT);

    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief Unlock the USB COM resource.
 *
 * @retval SSP_SUCCESS          Unlocking a USB COM resource successful.
 * @retval SSP_ERR_ASSERTION    Pointer to control block is NULL.
 * @retval SSP_ERR_NOT_OPEN     Module is not opened.
 * @retval SSP_ERR_INTERNAL     Failed to release the mutex.
**********************************************************************************************************************/
ssp_err_t SF_EL_UX_COMMS_Unlock (sf_comms_ctrl_t * const p_api_ctrl, sf_comms_lock_t lock_type)
{
    sf_el_ux_comms_instance_ctrl_t * p_ctrl = (sf_el_ux_comms_instance_ctrl_t *) p_api_ctrl;

#if SF_EL_UX_COMMS_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
#endif

    SF_EL_UX_COMMS_ERROR_RETURN((OPEN == p_ctrl->open), SSP_ERR_NOT_OPEN);

    UINT tx_err_tx = TX_SUCCESS;
    UINT tx_err_rx = TX_SUCCESS;

    /** Unlock the USB CDC COM resource as per the user request. */
    if ((SF_COMMS_LOCK_TX == lock_type) || (SF_COMMS_LOCK_ALL == lock_type))
    {
        tx_err_tx = tx_mutex_put(&p_ctrl->mutex[0]);
    }

    if ((SF_COMMS_LOCK_RX == lock_type) || (SF_COMMS_LOCK_ALL == lock_type))
    {
        tx_err_rx = tx_mutex_put(&p_ctrl->mutex[1]);
    }

    /** If USB CDC COM resource fails to unlock,returns error. */
    SF_EL_UX_COMMS_ERROR_RETURN((TX_SUCCESS == tx_err_tx) && (TX_SUCCESS == tx_err_rx), SSP_ERR_INTERNAL);
    return SSP_SUCCESS;
}  /* End of function SF_EL_UX_COMMS_Unlock() */

/*******************************************************************************************************************//**
 * @brief Get driver version
 *
 * @param[out] p_version   Version will be stored here.
 *
 * @note This function is reentrant.
 **********************************************************************************************************************/
ssp_err_t SF_EL_UX_COMMS_VersionGet (ssp_version_t * const p_version)
{
#if SF_EL_UX_COMMS_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_version);
#endif

    *p_version = module_version;
    return SSP_SUCCESS;
}  /* End of function SF_EL_UX_COMMS_VersionGet() */

/*******************************************************************************************************************//**
 * @} (end addtogroup SF_EL_UX_COMMS)
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Private Functions
 **********************************************************************************************************************/

/******************************************************************************************************************//**
 * @brief   Subroutine for SF_EL_UX_COMMS_Read API which will be called to read the leftover data from last packet.
 *
 * @param[in]      p_ctrl             Pointer to the sf_el_ux_comms control block.
 * @param[out]     p_dest             Pointer to the destination buffer.
 * @param[in,out]  p_bytes_remaining  Pointer to remaining bytes.
**********************************************************************************************************************/
static void sf_el_ux_comms_read_leftover(sf_el_ux_comms_instance_ctrl_t * p_ctrl, uint8_t * const p_dest, uint32_t * p_bytes_remaining)
{
    if (p_ctrl->leftover_length > 0U)
    {
        uint32_t transfer_length = p_ctrl->leftover_length;
        if (p_ctrl->leftover_length > *p_bytes_remaining)
        {
            transfer_length = *p_bytes_remaining;
        }
        _ux_utility_memory_copy(p_dest, &p_ctrl->rx_memory[p_ctrl->index], transfer_length);
        p_ctrl->leftover_length -= transfer_length;
        p_ctrl->index += transfer_length;
        *p_bytes_remaining -= transfer_length;
    }
}
/******************************************************************************************************************//**
 * @brief   Subroutine for SF_EL_UX_COMMS Read/Write API parameter check.
 *
 * @param[in]      p_ctrl             Pointer to the sf_el_ux_comms control block.
 * @param[in,out]  p_dest_or_src      Pointer to the destination or source buffer.
 *
 * @retval  SSP_ERR_ASSERTION         Pointer to control block is NULL
 * @retval  SSP_ERR_NOT_OPEN          Module is not opened.
 * @retval  SSP_ERR_INTERNAL          USB device not enumerated or USBX write call returned an error.
 * @retval  SSP_SUCCESS               Parameter check successful.
**********************************************************************************************************************/
static ssp_err_t  sf_el_ux_comms_parameter_check(sf_el_ux_comms_instance_ctrl_t * p_ctrl, uint8_t const * const p_dest_or_src)
{
#if SF_EL_UX_COMMS_CFG_PARAM_CHECKING_ENABLE
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_dest_or_src);
#else
    SSP_PARAMETER_NOT_USED(p_dest_or_src);
#endif

    /** Check if module has been opened. */
    SF_EL_UX_COMMS_ERROR_RETURN((OPEN == p_ctrl->open), SSP_ERR_NOT_OPEN);

    /** Check for the USB to be plugged in. */
    SF_EL_UX_COMMS_ERROR_RETURN((NULL != p_ctrl->p_cdc), SSP_ERR_INTERNAL);

    return SSP_SUCCESS;

}
/* End of file */
