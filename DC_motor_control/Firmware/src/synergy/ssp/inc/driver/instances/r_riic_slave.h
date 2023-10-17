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
* File Name    : r_riic_slave.h
* Description  : I2C Bus (RIIC Slave) Module instance header file.
***********************************************************************************************************************/


/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup RIIC_SLAVE IIC Slave
 * @brief Driver for the I2C Bus Slave Interface (IIC Slave).
 *
 * This module supports the Renesas Inter-Integrated Circuit (IIC) peripheral. It implements the following interfaces:
 *   - @ref I2C_API r_i2c_api.h
 * @{
***********************************************************************************************************************/

#ifndef R_RIIC_SLAVE_H
#define R_RIIC_SLAVE_H

#include "bsp_api.h"
#include "r_riic_slave_cfg.h"
#include "r_i2c_api.h"

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
#define RIIC_SLAVE_CODE_VERSION_MAJOR   (2U)
#define RIIC_SLAVE_CODE_VERSION_MINOR   (0U)

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/
/** I2C control structure. DO NOT INITIALIZE. */
typedef struct st_riic_slave_instance_ctrl
{
    i2c_cfg_t   info;           ///< Information describing I2C device
    uint32_t    open;           ///< Flag to determine if the device is open
    void      * p_reg;          ///< Base register for this channel

    IRQn_Type   rxi_irq;        ///< Receive IRQ number
    IRQn_Type   txi_irq;        ///< Transmit IRQ number
    IRQn_Type   eri_irq;        ///< Error IRQ number
    IRQn_Type   tei_irq;        ///< Transmit end IRQ number

    /* Current transfer information. */
    uint8_t     *   p_buff;         /**< Holds the data associated with the transfer */
    uint32_t        total;          /**< Holds the total number of data bytes to transfer */
    uint32_t        remain;         /**< Tracks the remaining data bytes to transfer */
    uint32_t        loaded;         /**< Tracks the number of data bytes written to the register */
    uint32_t        transaction_count;   /**< Tracks the actual number of transactions */

    volatile bool   notify_request; /**< Track whether the master request is notified to the application */
    volatile bool   read;           /**< Holds the direction of the data byte transfer */
    volatile bool   err;            /**< Tracks whether or not an error occurred during processing */
    volatile bool   slave_busy;     /**< Tracks if the slave is busy performing a transaction  */
    volatile bool   do_dummy_read; /** << Tracks whether a dummy read is issued on the first RX */
    volatile bool   start_interrupt_enabled; /**<< Tracks whether the start interrupt is enabled */
    volatile bool   restarted;      /** << Tracks whether previous transaction restarted */
} riic_slave_instance_ctrl_t;

/**********************************************************************************************************************
Exported global variables
***********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern i2c_api_slave_t const g_i2c_slave_on_riic;
/** @endcond */

#endif // R_RIIC_SLAVE_H

/*******************************************************************************************************************//**
 * @} (end defgroup RIIC_SLAVE IIC Slave)
***********************************************************************************************************************/
