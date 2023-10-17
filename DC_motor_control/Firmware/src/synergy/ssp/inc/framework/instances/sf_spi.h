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
 * File Name    : sf_spi.h
 * Description  : SPI framework driver header file
 **********************************************************************************************************************/

#ifndef SF_SPI_H
#define SF_SPI_H

/*******************************************************************************************************************//**
 * @ingroup SF_Library
 * @defgroup SF_SPI SPI Framework
 * @brief RTOS-integrated SPI Framework.
 *
 * @{
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
/* Includes driver interface. */
#include "r_spi_api.h"
#include "sf_spi_api.h"
#include "sf_spi_cfg.h"
#include "tx_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define SF_SPI_CODE_VERSION_MAJOR (2U)
#define SF_SPI_CODE_VERSION_MINOR (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** SPI device context. DO NOT INITIALIZE.  Initialization occurs when sf_spi_api_t::open is called. */
typedef struct st_sf_spi_instance_ctrl
{
    sf_spi_bus_t      * p_bus;                        ///< Bus using this device (copy from cfg)
    ioport_port_pin_t   chip_select;                  ///< Chip select for this device (copy from cfg)
    ioport_level_t      chip_select_level_active;     ///< Polarity of CS, active High or Low (copy from cfg)
    spi_cfg_t           lower_lvl_cfg;                ///< SPI peripheral configuration, use for bus reconfiguration
    spi_ctrl_t        * p_lower_lvl_ctrl;             ///< SPI peripheral control block
    sf_spi_dev_state_t  dev_state;                    ///< Device status
    bool                locked;                       ///< Lock and unlock bus for a device
} sf_spi_instance_ctrl_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const sf_spi_api_t g_sf_spi_on_sf_spi;
/** @endcond */

/*******************************************************************************************************************//**
 * @} (end defgroup SF_SPI)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_SPI_H */
