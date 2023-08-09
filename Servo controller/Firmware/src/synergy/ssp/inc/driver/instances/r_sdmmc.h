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
 * File Name    : r_sdmmc.h
 * Description  : SDMMC driver header file.
 **********************************************************************************************************************/

#ifndef R_SDMMC_H
#define R_SDMMC_H

/*******************************************************************************************************************//**
 * @file r_sdmmc.h
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"
#include "r_sdmmc_cfg.h"
#include "r_sdmmc_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/*******************************************************************************************************************//**
 * @ingroup HAL_Library
 * @defgroup SDMMC SDMMC
 * @brief Driver for the SD/MMC Host Interface (SDHI).
 *
 * SD/MMC driver to access SD, eMMC, and SDIO devices.
 * @{
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define SDMMC_CODE_VERSION_MAJOR (2U)
#define SDMMC_CODE_VERSION_MINOR (0U)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
/** Card detection configuration options. */
/* Card detection using DAT3 is not supported. */
typedef enum e_sdmmc_card_detect
{
    SDMMC_CARD_DETECT_NONE,   ///< Card detection unused.
    SDMMC_CARD_DETECT_CD,     ///< Card detection using the SDnCD pin
} sdmmc_card_detect_t;

/** Write protection configuration option */
typedef enum e_sdmmc_write_protect
{
    SDMMC_WRITE_PROTECT_NONE,    ///< Write protection unused.
    SDMMC_WRITE_PROTECT_WP,      ///< write protection using SDnWP pin
} sdmmc_write_protect_t;

/* Private enumeration used for transfers. */
typedef enum e_sdmmc_transfer_dir
{
    SDMMC_TRANSFER_DIR_NONE,
    SDMMC_TRANSFER_DIR_READ,
    SDMMC_TRANSFER_DIR_WRITE
} sdmmc_transfer_dir_t;

/* Private structure used in control block. */
typedef union
{
    uint32_t  word;

    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    struct s_sdhi_event_type
    {
        uint32_t  response_end       : 1;          //< response end detected
        uint32_t  reserved_1         : 1;
        uint32_t  access_end         : 1;          //< access end detected
        uint32_t  card_removed       : 1;          //< card removal detected by cd pin
        uint32_t  card_inserted      : 1;          //< card insertion detected by cd pin
        uint32_t  reserved_5         : 3;
        uint32_t  card_dat3_removed  : 1;          //< card removal detected by dat3 pin
        uint32_t  card_dat3_inserted : 1;          //< card inserion detected by dat3 pin
        uint32_t  reservered_10      : 6;
        uint32_t  cmd_err            : 1;          //< command error
        uint32_t  crc_err            : 1;          //< crc error
        uint32_t  end_err            : 1;          //< end bit error
        uint32_t  dto                : 1;          //< data timeout
        uint32_t  ilw                : 1;          //< illegal write address
        uint32_t  ilr                : 1;          //< illegal read address
        uint32_t  rspt               : 1;          //< response timeout
        uint32_t  event_error        : 1;          //< all error flags combined
        uint32_t  bre                : 1;          //< buffer read enable
        uint32_t  bwe                : 1;          //< buffer write enable
        uint32_t  reserved_26        : 5;
        uint32_t  ila_err            : 1;          //< illegal access
    }  bit;
} sdhi_event_t;

/* Private structure used in control block. */
typedef union
{
    uint32_t  word;

    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    struct s_sdhi_sdio_event_type
    {
        uint32_t  ioirq       : 1;                 //< sdio interrupt status
        uint32_t  event_error : 1;
        uint32_t  reserved_2  : 12;
        uint32_t  expub52     : 1;                 //< EXPUB52 status
        uint32_t  exwt        : 1;                 //< sdio exwt status
        uint32_t  reserved_16 : 16;
    }  bit;
} sdhi_sdio_event_t;

/* Unused typedef included for compatibility with previous releases. */
typedef struct s_sdhi_int_status_type
{
    uint32_t  info2_mask;
} sdhi_int_status_t;

/* Unused typedef included for compatibility with previous releases. */
typedef enum e_sdmmc_io_command
{
    SDMMC_IO_COMMAND_DIRECT_IO = 52,
    SDMMC_IO_COMMAND_DIRECT_IO_EXT = 53,
} sdmmc_io_command_t;

/* Unused typedef included for compatibility with previous releases. */
typedef struct s_sdmmc_io_mode
{
    sdmmc_io_command_t         command;
    sdmmc_io_transfer_mode_t   transfer_mode;
    sdmmc_io_address_mode_t    address_mode;
    sdmmc_io_write_mode_t      write_mode;
} sdmmc_io_mode_t;

/** SDMMC instance control block.  This is private to the SSP and should not be used or modified by the application. */
typedef struct st_sdmmc_instance_ctrl
{
    uint32_t                    open;                       ///< used to determine if channel is open
    sdmmc_hw_t                  hw;                         ///< Temporary storage for channel, media type and bus width
    transfer_instance_t const * p_lower_lvl_transfer;       ///< Transfer instance used to transfer data with DMA or DTC
    sdmmc_info_t                status;                     ///< To load various status information
    bool                        transfer_in_progress;       ///< Flag to detect transfer status
    bool                        write_protect;              ///< write protect status
    void                     (* p_callback)(sdmmc_callback_args_t * p_args); ///< User callback pointer
    void const                * p_context;                  ///< Placeholder for user data
    R_SDHI0_Type              * p_reg;                      ///< Base register information
    volatile sdhi_event_t       sdhi_event;                 ///< Update event status
    IRQn_Type                   transfer_irq;               ///< Scan end IRQ number
    sdmmc_transfer_dir_t        transfer_dir;               ///< Info on read or write operation in progress
    uint8_t *                   p_transfer_data;            ///< Temporary storage for transfer data
    uint32_t                    transfer_blocks_total;      ///< Total transfer block count
    uint32_t                    transfer_block_current;     ///< Transfer current block
    uint32_t                    transfer_block_size;        ///< Transfer block size
    uint32_t                    aligned_buff[SDMMC_MAX_BLOCK_SIZE / sizeof(uint32_t)];///< Aligned buffer
} sdmmc_instance_ctrl_t;

/** Extended SDMMC configuration, to be pointed to p_extend. */
typedef struct st_sdmmc_extended_cfg
{
    /** Block size in bytes.  Block size must be 512 bytes for SD cards and eMMC devices.  Block size can be 1-512
     * bytes for SDIO. */
    uint32_t              block_size;

    /** Whether or not card detection is used. */
    sdmmc_card_detect_t   card_detect;

    /** Select whether or not to use the write protect pin. Select Not Used if the MCU or device does not have a write protect pin. */
    sdmmc_write_protect_t write_protect;
} sdmmc_extended_cfg_t;

/**********************************************************************************************************************
 * Exported global variables
 **********************************************************************************************************************/
/** @cond INC_HEADER_DEFS_SEC */
/** Filled in Interface API structure for this Instance. */
extern const sdmmc_api_t g_sdmmc_on_sdmmc;
/** @endcond */

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // R_SDMMC_H

/*******************************************************************************************************************//**
 * @} (end addtogroup SDMMC)
 **********************************************************************************************************************/
