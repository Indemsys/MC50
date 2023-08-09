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
 * File Name    : sf_block_media_api.h
 * Description  : Block Media Driver Shared Interface definition
 **********************************************************************************************************************/

#ifndef SF_BLOCK_MEDIA_API_H
#define SF_BLOCK_MEDIA_API_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
/* Includes board and MCU related header files. */
#include "bsp_api.h"
/* Includes driver interface. */
#include "tx_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/*******************************************************************************************************************//**
 * @ingroup SF_Interface_Library
 * @defgroup SF_BLOCK_MEDIA_API Block Media Framework Interface
 * @brief RTOS-integrated File system Interface to access Synergy block media devices.
 *
 * The interface provides an
 * adaption layer from the  FileX I/O to block media devices.
 * @section BLOCK_MEDIA_API_SUMMARY Summary
 *
 * Related SSP architecture topics:
 *  - @ref ssp-interfaces
 *  - @ref ssp-predefined-layers
 *  - @ref using-ssp-modules
 *
 * See also FileX Interface description: @ref FrameworkFileXPortBlockMedia
  * @{
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define BLOCK_MEDIA_API_VERSION_MAJOR (2U)
#define BLOCK_MEDIA_API_VERSION_MINOR (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** Interface Configuration */
typedef struct st_sf_block_media_cfg
{
    uint32_t    block_size;                     ///< Block size in bytes.
    void const * p_extend;                      ///< Instance dependent configuration.
} sf_block_media_cfg_t;

/** Block media framework control block.  Allocate an instance specific control block to pass into the
 * block media framework API calls.
 * @par Implemented as
 * - sf_block_media_sdmmc_instance_ctrl_t
 * - sf_block_media_ram_instance_ctrl_t
 * - sf_block_media_qspi_instance_ctrl_t
 * - sf_block_media_levelx_nor_instance_ctrl_t
 */
typedef void sf_block_media_ctrl_t;

/** Shared Interface definition for Block Media */
typedef struct st_sf_block_media_api
{
    /** Open a device channel for read/write and control.
     * @par Implemented as
     * - SF_Block_Media_SDMMC_Open()
     * - SF_BLOCK_MEDIA_RAM_Open()
     * - SF_BLOCK_MEDIA_QSPI_Open()
     * - SF_BLOCK_MEDIA_LX_NOR_Open()
     *
     * @param[in]     p_cfg     Pointer to the media configuration structure for a channel.
     */
    ssp_err_t (* open)(sf_block_media_ctrl_t *         p_ctrl, sf_block_media_cfg_t const * const p_cfg);

    /** Read data from a media channel.
     * @par Implemented as
     * - SF_Block_Media_SDMMC_Read()
     * - SF_BLOCK_MEDIA_RAM_Read()
     * - SF_BLOCK_MEDIA_QSPI_Read()
     * - SF_BLOCK_MEDIA_LX_NOR_Read()
     *
     * @param[in]   p_cfg           Pointer to the media configuration structure for a channel.
     * @param[in]   p_dest          Destination address to read data out.
     * @param[in]   start_sector    Beginning sector address to read.
     * @param[in]   sector_count    Number of sectors to read.
     */
    ssp_err_t (* read)(sf_block_media_ctrl_t *          p_ctrl,
                       uint8_t * const                  p_dest,
                       uint32_t const                   start_sector,
                       uint32_t const                   sector_count);

    /** Write data to a media channel.
     * @par Implemented as
     * - SF_Block_Media_SDMMC_Write()
     * - SF_BLOCK_MEDIA_RAM_Write()
     * - SF_BLOCK_MEDIA_QSPI_Write()
     * - SF_BLOCK_MEDIA_LX_NOR_Write()
     *
     * @param[in]   p_cfg           Pointer to the media configuration structure for a channel.
     * @param[in]   p_src           Source address of data for writing.
     * @param[in]   start_sector    Beginning sector address to write to.
     * @param[in]   sector_count    Number of sectors to write.
     */
    ssp_err_t (* write)(sf_block_media_ctrl_t *          p_ctrl,
                        uint8_t const * const            p_src,
                        uint32_t const                   start_sector,
                        uint32_t const                   sector_count);

    /** Send control commands to and receives the status from the media port.
     * @par Implemented as
     * - SF_Block_Media_SDMMC_Control()
     * - SF_BLOCK_MEDIA_RAM_Control()
     * - SF_BLOCK_MEDIA_QSPI_Control()
     * - SF_BLOCK_MEDIA_LX_NOR_Control()
     *
     * @param[in]       p_cfg       Pointer to the media configuration structure for a channel.
     * @param[in]       command     Command to execute.
     * @param[in,out]   p_data      Void pointer to data in or out.
     */
    ssp_err_t (* ioctl)(sf_block_media_ctrl_t *         p_ctrl,
                        ssp_command_t const             command,
                        void                          * p_data);

    /** Close the open media channel.
     * @par Implemented as
     * - SF_Block_Media_SDMMC_Close()
     * - SF_BLOCK_MEDIA_RAM_Close()
     * - SF_BLOCK_MEDIA_QSPI_Close()
     * - SF_BLOCK_MEDIA_LX_NOR_Close()
     *
     * @param[in] p_cfg    Pointer to the media configuration structure for a channel.
     */
    ssp_err_t (* close)(sf_block_media_ctrl_t *          p_ctrl);

    /** Return the version of the driver.
     * @par Implemented as
     * - SF_Block_Media_SDMMC_VersionGet()
     * - SF_BLOCK_MEDIA_RAM_VersionGet()
     * - SF_BLOCK_MEDIA_QSPI_VersionGet()
     * - SF_BLOCK_MEDIA_LX_NOR_VersionGet()
     *
     * @param[in]   p_cfg       Pointer to the media configuration structure for a channel.
     * @param[out]  p_version   Memory address to return version information to.
     */
    ssp_err_t (* versionGet)(ssp_version_t * const p_version);
} sf_block_media_api_t;

/** Interface Instance  */
typedef struct st_sf_block_media_instance
{
    sf_block_media_ctrl_t       * p_ctrl;         ///< Block media pointer to device driver control structure
    sf_block_media_cfg_t const  * p_cfg;          ///< Block media pointer to device driver configuration structure
    sf_block_media_api_t const  * p_api;          ///< Block media pointer to device driver api structure

} sf_block_media_instance_t;

/**********************************************************************************************************************
 * Public Functions
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @} (end defgroup SF_BLOCK_MEDIA_API)
 **********************************************************************************************************************/

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* SF_BLOCK_MEDIA_API_H */
