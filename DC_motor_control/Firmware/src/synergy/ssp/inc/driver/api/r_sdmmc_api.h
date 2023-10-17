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
 * File Name    : r_sdmmc_api.h
 * Description  : SD/MMC driver API header file.
 **********************************************************************************************************************/

#ifndef DRV_SDMMC_API_H
#define DRV_SDMMC_API_H

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
/* Register definitions, common services and error codes. */
#include "bsp_api.h"
#include "r_transfer_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/*******************************************************************************************************************//**
 * @ingroup Interface_Library
 * @defgroup SDMMC_API SD/MMC Interface
 * @brief Interface for accessing SD, eMMC, and SDIO devices.
 *
 * @section SDMMC_API_SUMMARY Summary
 * The r_sdmmc interface provides standard SD and eMMC media functionality. A complete file system can be
 * implemented with FileX, sf_el_fx, sf_block_media_sdmmc and r_sdmmc modules.  This driver also supports SDIO.
 * The SD/MMC interface is implemented by:
 * - @ref SDMMC
 *
 * Related SSP architecture topics:
 * - @ref ssp-interfaces
 * - @ref ssp-predefined-layers
 * - @ref using-ssp-modules
 *
 * SD/MMC description: @ref ModuleSDMMC
 *
 * @{
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define SDMMC_API_VERSION_MAJOR (2U)
#define SDMMC_API_VERSION_MINOR (0U)

#define SDMMC_MAX_BLOCK_SIZE   (512U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** SD/MMC status */
typedef enum e_sdmmc_ready_status
{
    SDMMC_STATUS_CARD_NOT_READY = 0x00,     ///< SD card or eMMC device has not been initialized.
    SDMMC_STATUS_CARD_READY,                ///< SD card or eMMC device has been initialized and is ready to access.
} sdmmc_ready_status_t;

/** SD/MMC media uses SD protocol or MMC protocol. */
typedef enum e_sdmmc_card_type
{
    SDMMC_CARD_TYPE_MMC,                    ///< The media is an eMMC device.
    SDMMC_CARD_TYPE_SD                      ///< The media is an SD card.
} sdmmc_card_type_t;

/** SD/MMC media is embedded or it can be inserted and removed.  */
typedef enum e_media_type
{
    SDMMC_MEDIA_TYPE_EMBEDDED,              ///< The media is an embedded card, or eMMC device.
    SDMMC_MEDIA_TYPE_CARD                   ///< The media is an pluggable card.
} sdmmc_media_type_t;

/** SD/MMC data bus is 1, 4 or 8 bits wide. */
typedef enum e_sdmmc_bus_width
{
    SDMMC_BUS_WIDTH_1_BIT  = 1,             ///< Data bus is 1 bit wide.
    SDMMC_BUS_WIDTH_4_BITS = 4,             ///< Data bus is 4 bits wide.
    SDMMC_BUS_WIDTH_8_BITS = 8,             ///< Data bus is 8 bits wide.
} sdmmc_bus_width_t;

/** Channel, media type, bus width defined by the hardware. */
typedef struct s_sdmmc_hw
{
    uint8_t             channel;                ///< Channel of SD/MMC host interface.
    sdmmc_media_type_t  media_type;             ///< Embedded or pluggable card.
    sdmmc_bus_width_t   bus_width;              ///< Device bus width is 1, 4 or 8 bits wide.
} sdmmc_hw_t;

/** Status and other information obtained from the media device. */
typedef struct s_sdmmc_info
{
    sdmmc_card_type_t  card_type;               ///< SD or eMMC

    /** False if card was removed (only applies if MCU supports card detection and SDnCD pin is connected).
     *
     *  True otherwise.
     *
     *  If ready is false, the driver must be closed, then reopened with a card inserted.
     */
    bool               ready;
    bool               hc;                      ///< true = Card is High Capacity card
    bool               sdio;                    ///< true = SDIO present
    bool               write_protected;         ///< true = Card is write protected
    bool               transfer_in_progress;    ///< true = Card is busy
    uint8_t            csd_version;             ///< CSD version
    uint8_t            device_type;             ///< Speed and data rate (eMMC)
    sdmmc_bus_width_t  bus_width;               ///< Current media bus width
    uint8_t            hs_timing;               ///< High Speed status
    uint32_t           sdhi_rca;                ///< Relative Card Address
    uint32_t           max_clock_rate;          ///< Maximum clock rate for media card
    uint32_t           clock_rate;              ///< Current clock rate
    uint32_t           sector_size;             ///< Sector size
    uint32_t           sector_count;            ///< Sector count
    uint32_t           erase_sector_count;      ///< Minimum erasable unit (in 512 byte sectors)
} sdmmc_info_t;

/** SDIO transfer mode, configurable in SDIO read/write extended commands. */
typedef enum e_sdmmc_io_transfer_mode
{
    SDMMC_IO_MODE_TRANSFER_BYTE = 0,            ///< SDIO byte transfer mode
    SDMMC_IO_MODE_TRANSFER_BLOCK                ///< SDIO block transfer mode
} sdmmc_io_transfer_mode_t;

/** SDIO address mode, configurable in SDIO read/write extended commands. */
typedef enum e_sdmmc_io_address_mode
{
    SDMMC_IO_ADDRESS_MODE_FIXED = 0,            ///< Write all data to the same address
    SDMMC_IO_ADDRESS_MODE_INCREMENT             ///< Increment destination address after each write
} sdmmc_io_address_mode_t;

/** Controls the RAW (read after write) flag of CMD52. Used to read back the status after writing a control register. */
typedef enum e_sdmmc_io_write_mode
{
    SDMMC_IO_WRITE_MODE_NO_READ = 0,            ///< Write only (do not read back)
    SDMMC_IO_WRITE_READ_AFTER_WRITE             ///< Read back the register after write
} sdmmc_io_write_mode_t;

/** Events that can trigger a callback function */
typedef enum e_sdmmc_event
{
    SDMMC_EVENT_CARD_REMOVED      = 0x01,       ///< Card removed event.
    SDMMC_EVENT_CARD_INSERTED     = 0x02,       ///< Card inserted event.
    SDMMC_EVENT_ACCESS            = 0x04,       ///< Access event.
    SDMMC_EVENT_SDIO              = 0x08,       ///< IO event.
    SDMMC_EVENT_TRANSFER_COMPLETE = 0x10,       ///< Read or write complete.
    SDMMC_EVENT_TRANSFER_ERROR    = 0x20,       ///< Read or write failed.
    SDMMC_EVENT_NONE              = 0x00,       ///< No event.
} sdmmc_event_t;

/** Callback function parameter data */
typedef struct st_sdmmc_callback_args
{
    sdmmc_event_t  event;               ///< The event can be used to identify what caused the callback.
    void const   * p_context;           ///< Placeholder for user data.
} sdmmc_callback_args_t;

/** SD/MMC Configuration */
typedef struct st_sdmmc_cfg
{
    /* SD/MMC generic configuration */
    sdmmc_hw_t                   hw;                           ///<  Channel, media type, bus width defined by the hardware.
    transfer_instance_t  const * p_lower_lvl_transfer;         ///< Transfer instance used to move data with DMA or DTC

    /* Configuration for SD/MMC Event processing */
    void (* p_callback)(sdmmc_callback_args_t * p_args);  ///<  Pointer to callback function
    void const * p_context;                               ///<  User defined context passed into callback function

    /* Pointer to SD/MMC peripheral specific configuration */
    void const * p_extend;                                ///<  SD/MMC hardware dependent configuration
    uint8_t      access_ipl;                              ///< Access interrupt priority
    uint8_t      sdio_ipl;                                ///< SDIO interrupt priority
    uint8_t      card_ipl;                                ///< Card interrupt priority
    uint8_t      dma_req_ipl;                             ///< DMA request interrupt priority
} sdmmc_cfg_t;

/** SD/MMC control block.  Allocate an instance specific control block to pass into the SD/MMC API calls.
 * @par Implemented as
 * - sdmmc_instance_ctrl_t
 */
typedef void sdmmc_ctrl_t;

/** SD/MMC functions implemented at the HAL layer API. */
typedef struct st_sdmmc_api
{
    /** Open an SD/MMC device.  If the device is a card, the card must be plugged in prior to calling this API.
     * This API blocks until the device initialization procedure is complete.
     *
     * @par Implemented as
     * R_SDMMC_Open()
     *
     * @param[in]     p_ctrl    Pointer to SD/MMC instance control block.
     * @param[in]     p_cfg     Pointer to SD/MMC instance configuration structure.
     */
    ssp_err_t (* open)(sdmmc_ctrl_t      * const p_ctrl,
                       sdmmc_cfg_t const * const p_cfg);

    /** Close open SD/MMC device.
     *
     * @par Implemented as
     * R_SDMMC_Close()
     *
     * @param[in]     p_ctrl    Pointer to an open SD/MMC instance control block.
     */
    ssp_err_t (* close)(sdmmc_ctrl_t * const p_ctrl);

    /** Read data from an SD/MMC channel.
     * This API is not supported for SDIO devices.
     *
     * @par Implemented as
     * R_SDMMC_Read()
     *
     * @param[in]     p_ctrl          Pointer to an open SD/MMC instance control block.
     * @param[out]    p_dest          Pointer to data buffer to read data to.
     * @param[in]     start_sector    First sector address to read.
     * @param[in]     sector_count    Number of sectors to read.  All sectors must be in the range of
     *                                sdmmc_info_t::sector_count.
     */
    ssp_err_t (* read)(sdmmc_ctrl_t * const p_ctrl,
                       uint8_t      * const p_dest,
                       uint32_t       const start_sector,
                       uint32_t       const sector_count);

    /** Write data to SD/MMC channel.
     * This API is not supported for SDIO devices.
     *
     * @par Implemented as
     * R_SDMMC_Write()
     *
     * @param[in]     p_ctrl          Pointer to an open SD/MMC instance control block.
     * @param[in]     p_source        Pointer to data buffer to write data from.
     * @param[in]     start_sector    First sector address to write to.
     * @param[in]     sector_count    Number of sectors to write.  All sectors must be in the range of
     *                                sdmmc_info_t::sector_count.
     */
    ssp_err_t (* write)(sdmmc_ctrl_t  * const p_ctrl,
                        uint8_t const * const p_source,
                        uint32_t        const start_sector,
                        uint32_t        const sector_count);

    /** The Control function sends control commands to and receives info from the SD/MMC port.
     *
     * @par Implemented as
     * R_SDMMC_Control()
     *
     * @param[in]     p_ctrl    Pointer to an open SD/MMC instance control block.
     * @param[in]     command   Command to execute. The list of supported commands is below.
     * @param[in,out] p_data    Pointer to data in or out.  For each command, this data should be cast as follows:
     *                            * SSP_COMMAND_GET_SECTOR_COUNT     : [out] (uint32_t *) p_data
     *                            * SSP_COMMAND_GET_SECTOR_SIZE      : [out] (uint32_t *) p_data
     *                            * SSP_COMMAND_GET_WRITE_PROTECTED  : [out] (bool *) p_data
     *                            * SSP_COMMAND_SET_BLOCK_SIZE       : [in]  (uint32_t *) p_data
     */
    ssp_err_t (* control)(sdmmc_ctrl_t * const p_ctrl,
                         ssp_command_t   const command,
                         void          *       p_data);

    /** Read one byte of I/O data from an SDIO device.
     * This API is not supported for SD or eMMC memory devices.
     *
     * @par Implemented as
     * R_SDMMC_ReadIo()
     *
     * @param[in]     p_ctrl    Pointer to an open SD/MMC instance control block.
     * @param[out]    p_data    Pointer to location to store data byte.
     * @param[in]     function  SDIO Function Number.
     * @param[in]     address   SDIO register address.
     */
    ssp_err_t (* readIo)(sdmmc_ctrl_t * const p_ctrl,
                         uint8_t      * const p_data,
                         uint32_t       const function,
                         uint32_t       const address);

    /** Write one byte of I/O data to an SDIO device.
     * This API is not supported for SD or eMMC memory devices.
     *
     * @par Implemented as
     * R_SDMMC_WriteIo()
     *
     * @param[in]     p_ctrl            Pointer to an open SD/MMC instance control block.
     * @param[in,out] p_data            Pointer to data byte to write.  Read data is also provided here if
     *                                  read_after_write is true.
     * @param[in]     function          SDIO Function Number.
     * @param[in]     address           SDIO register address.
     * @param[in]     read_after_write  Whether or not to read back the same register after writing
     */
    ssp_err_t (* writeIo)(sdmmc_ctrl_t        * const p_ctrl,
                          uint8_t             * const p_data,
                          uint32_t              const function,
                          uint32_t              const address,
                          sdmmc_io_write_mode_t const read_after_write);

    /** Read multiple bytes or blocks of I/O data from an SDIO device.
     * This API is not supported for SD or eMMC memory devices.
     *
     * @par Implemented as
     * R_SDMMC_ReadIoExt()
     *
     * @param[in]     p_ctrl          Pointer to an open SD/MMC instance control block.
     * @param[out]    p_dest          Pointer to data buffer to read data to.
     * @param[in]     function        SDIO Function Number.
     * @param[in]     address         SDIO register address.
     * @param[in]     count           Number of bytes or blocks to read, maximum 512 bytes or 511 blocks.
     * @param[in]     transfer_mode   Byte or block mode
     * @param[in]     address_mode    Fixed or incrementing address mode
     */
    ssp_err_t (* readIoExt)(sdmmc_ctrl_t     * const p_ctrl,
                            uint8_t          * const p_dest,
                            uint32_t           const function,
                            uint32_t           const address,
                            uint32_t         * const count,
                            sdmmc_io_transfer_mode_t transfer_mode,
                            sdmmc_io_address_mode_t  address_mode);

    /** Write multiple bytes or blocks of I/O data to an SDIO device.
     * This API is not supported for SD or eMMC memory devices.
     *
     * @par Implemented as
     * R_SDMMC_WriteIoExt()
     *
     * @param[in]     p_ctrl          Pointer to an open SD/MMC instance control block.
     * @param[in]     p_source        Pointer to data buffer to write data from.
     * @param[in]     function_number SDIO Function Number.
     * @param[in]     address         SDIO register address.
     * @param[in]     count           Number of bytes or blocks to write, maximum 512 bytes or 511 blocks.
     * @param[in]     transfer_mode   Byte or block mode
     * @param[in]     address_mode    Fixed or incrementing address mode
     */
    ssp_err_t (* writeIoExt)(sdmmc_ctrl_t     * const p_ctrl,
                             uint8_t    const * const p_source,
                             uint32_t           const function,
                             uint32_t           const address,
                             uint32_t           const count,
                             sdmmc_io_transfer_mode_t transfer_mode,
                             sdmmc_io_address_mode_t  address_mode);

    /** Enables SDIO interrupt for SD/MMC instance.
     * This API is not supported for SD or eMMC memory devices.
     *
     * @par Implemented as
     * R_SDMMC_IoIntEnable
     *
     * @param[in]     p_ctrl    Pointer to an open SD/MMC instance control block.
     * @param[in]     enable    Interrupt enable = true, interrupt disable = false.
     */
    ssp_err_t (* IoIntEnable)(sdmmc_ctrl_t * const p_ctrl,
                              bool                 enable);

    /** Returns the version of the SD/MMC driver.
     *
     * @par Implemented as
     * R_SDMMC_VersionGet()
     *
     * @param[out]  p_version       Pointer to return version information to.
     */
    ssp_err_t (* versionGet)(ssp_version_t * const p_version);

    /** Get SD/MMC device info.
     *
     * @par Implemented as
     * R_SDMMC_InfoGet()
     *
     * @param[in]     p_ctrl    Pointer to an open SD/MMC instance control block.
     * @param[out]    p_info    Pointer to return device information to.
     */
    ssp_err_t (* infoGet)(sdmmc_ctrl_t * const p_ctrl,
                          sdmmc_info_t * const p_info);

    /** Erase SD/MMC sectors. The sector size for erase is fixed at 512 bytes.
     * This API is not supported for SDIO devices.
     *
     * @par Implemented as
     * R_SDMMC_Erase
     *
     * @param[in]     p_ctrl        Pointer to an open SD/MMC instance control block.
     * @param[in]     start_sector  First sector to erase. Must be a multiple of sdmmc_info_t::erase_sector_count.
     * @param[in]     sector_count  Number of sectors to erase. Must be a multiple of sdmmc_info_t::erase_sector_count.
     *                              All sectors must be in the range of sdmmc_info_t::sector_count.
     */
    ssp_err_t (* erase)(sdmmc_ctrl_t * const p_ctrl,
                        uint32_t       const start_sector,
                        uint32_t       const sector_count);
} sdmmc_api_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_sdmmc_instance
{
    sdmmc_ctrl_t      * p_ctrl;    ///< Pointer to the control structure for this instance
    sdmmc_cfg_t const * p_cfg;     ///< Pointer to the configuration structure for this instance
    sdmmc_api_t const * p_api;     ///< Pointer to the API structure for this instance
} sdmmc_instance_t;

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif /* DRV_SDMMC_API_H */

/*******************************************************************************************************************//**
 * @} (end addtogroup SDMMC_API)
 **********************************************************************************************************************/
