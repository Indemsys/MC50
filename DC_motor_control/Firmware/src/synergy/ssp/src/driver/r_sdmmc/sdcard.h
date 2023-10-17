/***********************************************************************************************************************
 * Copyright [2015-2021] Renesas Electronics Corporation and/or its licensors. All Rights Reserved.
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
 * File Name    : sdcard.h
 * Description  : SD Card information header file.
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @file sdcard.h
 **********************************************************************************************************************/
#ifndef SDCARD_H
#define SDCARD_H

/**********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/
#include "bsp_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define SDMMC_CSD_VERSION_1_0                  (0U)
#define SDMMC_CSD_VERSION_2_0                  (1U)    /* value of 1 is CSD 2.0 according to SD spec. */

#define SDMMC_SD_VERSION_1_01                  (0U)    /* SD version 1.0 and 1.01 */
#define SDMMC_SD_VERSION_1_10                  (1U)    /* SD version 1.10  */
#define SDMMC_SD_VERSION_2_00                  (2U)    /* SD version 2.00 or version 3.0x, check SD_SPEC3  */
#define SDMMC_SD_VERSION_3_0X                  (3U)    /* SD version derived from 2.00 and SD_SPEC3 */

#define SDMMC_EMMC_VERSION_4_1                  (1U)    /* value of 1 is CSD 1.1, eMMC 4.1 according to eMMC spec. */
#define SDMMC_EMMC_VERSION_4_2                  (2U)    /* value of 2 is CSD 1.2, eMMC 4.2 according to eMMC spec. */
#define SDMMC_EMMC_VERSION_4_3                  (3U)    /* value of 3 is CSD 1.3, eMMC 4.3 according to eMMC spec. */
#define SDMMC_EMMC_VERSION_OBSOLETE             (4U)    /* value of 4 is CSD 1.4, Obsolete acording to eMMC spec. */
#define SDMMC_EMMC_VERSION_4_4_1                (5U)    /* value of 5 is CSD 1.3, eMMC 4.41 according to eMMC spec. */
#define SDMMC_EMMC_VERSION_4_5                  (6U)    /* value of 6 is CSD 1.3, eMMC 4.5, 4.51 according to eMMC spec. */
#define SDMMC_EMMC_VERSION_5_0                  (7U)    /* value of 7 is CSD 1.3, eMMC 5.0, 5.1 according to eMMC spec. */

#define SDMMC_CSD_VERSION_EXT                  (3U)

#define SDMMC_CSD_VERSION_2_0_MULT             (0x80000U)

#define SDMMC_CMD_GO_IDLE_STATE             (0U)
#define SDMMC_CMD_SEND_OP_COND              (1U)
#define SDMMC_CMD_ALL_SEND_CID              (2U)
#define SDMMC_CMD_SEND_RELATIVE_ADDR        (3U)
#define SDMMC_CMD_SET_DSR                   (4U)
#define SDMMC_CMD_SDIO                      (5U)
#define SDMMC_CMD_SET_BUS_WIDTH             (6U)
#define SDMMC_CMD_SWITCH                    (6U)
#define SDMMC_CMD_SEL_DES_CARD              (7U)
#define SDMMC_CMD_IF_COND                   (8U)
#define SDMMC_CMD_SEND_CSD                  (9U)
#define SDMMC_CMD_SEND_CID                  (10U)
#define SDMMC_CMD_READ_DAT_UNTIL_STOP       (11U)
#define SDMMC_CMD_STOP_TRANSMISSION         (12U)
#define SDMMC_CMD_SEND_STATUS               (13U)
#define SDMMC_CMD_GO_INACTIVE_STATE         (15U)
#define SDMMC_CMD_SET_BLOCKLEN              (16U)
#define SDMMC_CMD_READ_SINGLE_BLOCK         (17U)
#define SDMMC_CMD_READ_MULTIPLE_BLOCK       (18U)
#define SDMMC_CMD_WRITE_DAT_UNTIL_STOP      (20U)
#define SDMMC_CMD_SET_BLOCK_COUNT           (23U)
#define SDMMC_CMD_WRITE_SINGLE_BLOCK        (24U)
#define SDMMC_CMD_WRITE_MULTIPLE_BLOCK      (25U)
#define SDMMC_CMD_PROGRAM_CID               (26U)
#define SDMMC_CMD_PROGRAM_CSD               (27U)
#define SDMMC_CMD_SET_WRITE_PROT            (28U)
#define SDMMC_CMD_CLR_WRITE_PROT            (29U)
#define SDMMC_CMD_SEND_WRITE_PROT           (30U)
#define SDMMC_CMD_ERASE_WR_BLK_START        (32U)
#define SDMMC_CMD_ERASE_WR_BLK_END          (33U)
#define SDMMC_CMD_UNTAG_SECTOR              (34U)
#define SDMMC_CMD_TAG_ERASE_GROUP_START     (35U)
#define SDMMC_CMD_TAG_ERASE_GROUP_END       (36U)
#define SDMMC_CMD_UNTAG_ERASE_GROUP         (37U)
#define SDMMC_CMD_ERASE                     (38U)
#define SDMMC_CMD_FAST_IO                   (39U)
#define SDMMC_CMD_GO_IRQ_STATE              (40U)
#define SDMMC_CMD_SD_SEND_OP_COND           (41U)
#define SDMMC_CMD_LOCK_UNLOCK               (42U)
#define SDMMC_CMD_IO_RW_DIRECT              (52U)
#define SDMMC_CMD_IO_READ_EXT_SINGLE_BLOCK  (0x1c35U)
#define SDMMC_CMD_IO_READ_EXT_MULTI_BLOCK   (0x7c35U)
#define SDMMC_CMD_IO_EXT_MULTI_BLOCK        (0x6000U)
#define SDMMC_CMD_IO_WRITE_EXT_SINGLE_BLOCK (0x0c35U)
#define SDMMC_CMD_IO_WRITE_EXT_MULTI_BLOCK  (0x6c35U)

#define SDMMC_CMD_APP_CMD                   (55U)
#define SDMMC_CMD_GEN_CMD                   (56U)
#define SDMMC_CMD_READ_OCR                  (58U)
#define SDMMC_CMD_CRC_ON_OFF                (59U)
#define SDMMC_CMD_C_ACMD                    (1U << 6)   /* APP Command */

#define SDMMC_SD_CARD_SPEED_25MZ               (0x32U)
#define SDMMC_SD_CARD_SPEED_50MZ               (0x5AU)
#define SDMMC_SD_CARD_SPEED_100MZ              (0x0BU)
#define SDMMC_SD_CARD_SPEED_200MZ              (0x2BU)

#define SDMMC_IF_COND_VOLTAGE            (1U)
#define SDMMC_IF_COND_CHECK_PATTERN      (0xAAU)

#define SDMMC_OCR_CAPACITY_SC                   (0U << 30)
#define SDMMC_OCR_CAPACITY_HC                   (1U << 30)

#define SDMMC_OCR_VDD_2_7_V                     (1U << 15)
#define SDMMC_OCR_VDD_2_8_V                     (1U << 16)
#define SDMMC_OCR_VDD_2_9_V                     (1U << 17)
#define SDMMC_OCR_VDD_3_0_V                     (1U << 18)
#define SDMMC_OCR_VDD_3_1_V                     (1U << 19)
#define SDMMC_OCR_VDD_3_2_V                     (1U << 20)
#define SDMMC_OCR_VDD_3_3_V                     (1U << 21)
#define SDMMC_OCR_VDD_3_4_V                     (1U << 22)
#define SDMMC_OCR_VDD_3_5_V                     (1U << 23)
#define SDMMC_OCR_VDD_SUPPORTED                                                        \
    (SDMMC_OCR_VDD_2_7_V | SDMMC_OCR_VDD_2_8_V | SDMMC_OCR_VDD_2_9_V | SDMMC_OCR_VDD_3_0_V | SDMMC_OCR_VDD_3_1_V | \
     SDMMC_OCR_VDD_3_2_V | SDMMC_OCR_VDD_3_3_V | SDMMC_OCR_VDD_3_4_V  | SDMMC_OCR_VDD_3_5_V)

/* CMD52 Arguments */
#define SDMMC_SDIO_CMD52_READ                    (0U)
#define SDMMC_SDIO_CMD52_WRITE                   (1U)
#define SDMMC_SDIO_CMD52_NO_READ_AFTER_WRITE     (0U)
#define SDMMC_SDIO_CMD52_READ_AFTER_WRITE        (1U)

/* SWITCH command argument's bit postion */
#define SDMMC_SWITCH_ACCESS_SHIFT             (24U)
#define SDMMC_SWITCH_INDEX_SHIFT              (16U)
#define SDMMC_SWITCH_VALUE_SHIFT              (8U)

#define SDMMC_EMMC_EXT_CSD_SIZE                        (512U)

/* Offsets */
#define SDMMC_EMMC_EXT_CSD_BUS_WIDTH_OFFSET    (183U)
#define SDMMC_EMMC_EXT_CSD_HS_TIMING_OFFSET    (185U)
#define SDMMC_EMMC_EXT_CSD_REVISION_OFFSET     (192U)
#define SDMMC_EMMC_EXT_CSD_DEVICE_TYPE_OFFSET  (196U)
#define SDMMC_EMMC_EXT_CSD_SEC_COUNT_OFFSET    (212U)

/* Commands */
#define SDMMC_EMMC_SWITCH_ACCESS_WRITE_BYTE (3U)
#define SDMMC_EMMC_HIGH_SPEED_52_MHZ_BIT    (2U)
#define SDMMC_EMMC_HIGH_SPEED_MODE       (((SDMMC_EMMC_SWITCH_ACCESS_WRITE_BYTE << 24U) | \
                                     (SDMMC_EMMC_EXT_CSD_HS_TIMING_OFFSET << 16U)) | \
                                     (SDMMC_EMMC_HIGH_SPEED_52_MHZ_BIT << 8U))

#define SDMMC_SD_SWITCH_STATUS_SIZE                  (64U)
#define SDMMC_SD_SWITCH_HIGH_SPEED_RESPONSE          (13U)
#define SDMMC_SD_SWITCH_HIGH_SPEED_ERROR_RESPONSE    (16U)
#define SDMMC_SD_SWITCH_HIGH_SPEED_ERROR             (0x0fU)
#define SDMMC_SD_SWITCH_HIGH_SPEED_OK                (0x02U)

#define SDMMC_EMMC_CMD_SWITCH_WBUSY      (0x506U)                      /* eMMC CMD6 switch command "with response busy" */
#define SDMMC_EMMC_CMD_SEND_EXT_CSD      (0x1c08U)                     /* CMD 8, read data */
#define SDMMC_EMMC_DEFAULT_CLOCK_RATE    (26000000U)                   /* 26 MHz */
#define SDMMC_EMMC_HIGH_SPEED_CLOCK_RATE (52000000U)                   /* 52 MHz */
#define SDMMC_SD_HIGH_SPEED_MODE_SWITCH  (0x80FFFFF1U)                 /* set SD high speed */
#define SDMMC_SD_HIGH_SPEED_MODE_CHECK   (0x00FFFFF1U)                 /* check SD high speed function*/
#define SDMMC_SD_DEFAULT_CLOCK_RATE      (25000000U)                   /* 25 MHz */
#define SDMMC_SD_HIGH_SPEED_CLOCK_RATE   (50000000U)                   /* 50 MHz */
#define SDMMC_SDIO_HIGH_SPEED_CLOCK_RATE (50000000U)                   /* 50 MHz */

#define SDMMC_EMMC_ERASE_ARGUMENT_TRIM   (1U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/
typedef enum e_sdmmc_priv_sd_state
{
    SDMMC_PRIV_SD_STATE_IDLE,
    SDMMC_PRIV_SD_STATE_READY,
    SDMMC_PRIV_SD_STATE_IDENT,
    SDMMC_PRIV_SD_STATE_STBY,
    SDMMC_PRIV_SD_STATE_TRANSFER,
    SDMMC_PRIV_SD_STATE_DATA,
    SDMMC_PRIV_SD_STATE_RCV,
    SDMMC_PRIV_SD_STATE_PROG,
    SDMMC_PRIV_SD_STATE_DIS,
    SDMMC_PRIV_SD_STATE_RSVD9,
    SDMMC_PRIV_SD_STATE_RSVD10,
    SDMMC_PRIV_SD_STATE_RSVD11,
    SDMMC_PRIV_SD_STATE_RSVD12,
    SDMMC_PRIV_SD_STATE_RSVD13,
    SDMMC_PRIV_SD_STATE_RSVD14,
    SDMMC_PRIV_SD_STATE_RSVD15
} sdmmc_priv_sd_state_t;

typedef union u_sdmmc_priv_card_status
{
    uint32_t  status;

    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    struct
    {
        uint16_t byte_0;

        uint16_t byte_1;
    } word;

    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    struct
    {
        uint32_t   reserved_0         : 3;

        uint32_t   ake_seq_error      : 1;
        uint32_t   reserved_4         : 1;
        uint32_t   app_cmd            : 1;
        uint32_t   reserved_6         : 2;
        uint32_t   ready_for_data     : 1;
        sdmmc_priv_sd_state_t current_state      : 4;
        uint32_t   erase_reset        : 1;
        uint32_t   card_ecc_disable   : 1;
        uint32_t   wp_erase_skip      : 1;
        uint32_t   csd_overwrite      : 1;
        uint32_t   reserved_17        : 2;
        uint32_t   error              : 1;
        uint32_t   cc_error           : 1;
        uint32_t   card_ecc_failed    : 1;
        uint32_t   illegal_command    : 1;
        uint32_t   com_crc_error      : 1;
        uint32_t   lock_unlock_failed : 1;
        uint32_t   card_is_locked     : 1;
        uint32_t   wp_violation       : 1;
        uint32_t   erase_param        : 1;
        uint32_t   erase_seq_error    : 1;
        uint32_t   block_len_error    : 1;
        uint32_t   address_error      : 1;
        uint32_t   out_of_range       : 1;
    } r1;

    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    struct
    {
        uint32_t reserved_0           : 7;

        uint32_t reserved_lvr         : 1;
        uint32_t reserved_8           : 7;
        uint32_t v_27_28              : 1;
        uint32_t v_28_29              : 1;
        uint32_t v_29_30              : 1;
        uint32_t v_30_31              : 1;
        uint32_t v_31_32              : 1;
        uint32_t v_32_33              : 1;
        uint32_t v_33_34              : 1;
        uint32_t v_34_35              : 1;
        uint32_t v_35_36              : 1;
        uint32_t s18A                 : 1;
        uint32_t reserved_25          : 5;
        uint32_t card_capacity_status : 1;
        uint32_t power_up_status      : 1;
    } r3;

    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    struct
    {
        uint32_t ocr          : 24;

        uint32_t reserved_24  : 3;
        uint32_t memory       : 1;
        uint32_t io_functions : 3;
        uint32_t ready        : 1;
    } r4;

    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    struct
    {
        uint32_t read_write_data  : 8;

        uint32_t out_of_range     : 1;
        uint32_t invalid_function : 1;
        uint32_t rfu              : 1;
        uint32_t error            : 1;
        uint32_t current_state    : 2;
        uint32_t illegal_command  : 1;
        uint32_t crc_error        : 1;
        uint32_t reserved_16      : 16;
    } r5;

    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    struct
    {
        uint32_t   reserved_0      : 3;

        uint32_t   ake_seq_error   : 1;
        uint32_t   reserved_4      : 1;
        uint32_t   app_cmd         : 1;
        uint32_t   reserved_6      : 2;
        uint32_t   ready_for_data  : 1;
        sdmmc_priv_sd_state_t current_state   : 4;
        uint32_t   error           : 1;
        uint32_t   illegal_command : 1;
        uint32_t   com_crc_error   : 1;
        uint32_t   rca             : 16;                   // Relative Card Address
    } r6;

    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    struct
    {
        uint32_t check_pattern    : 8;

        uint32_t voltage_accepted : 4;
        uint32_t reserved_11      : 20;
    } r7;
} sdmmc_priv_card_status_t;

/** SDMMC card specific data */
typedef struct st_sdmmc_priv_csd_reg
{
    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    union
    {
        uint8_t  array[16];

        /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
#if defined(__ICCARM__)
        __packed struct
#elif defined(  __GNUC__)
        /*LDRA_INSPECTED 293 S  All structures in this union must be packed since some bitfields span more than one 32-bit word. */
        struct __attribute__((__packed__))
#endif
        {
            volatile uint32_t sdrsp10;
            volatile uint32_t sdrsp32;
            volatile uint32_t sdrsp54;
            volatile uint32_t sdrsp76;
        } reg;

        /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
#if defined(__ICCARM__)
        __packed struct
#elif defined(  __GNUC__)
        /*LDRA_INSPECTED 293 S  All structures in this union must be packed since some bitfields span more than one 32-bit word. */
        struct __attribute__((__packed__))
#endif
        {
            uint32_t reserved_8         : 2;              ///<   [9:8]
            uint32_t file_format        : 2;              ///<   [11:10] file format
            uint32_t tmp_write_protect  : 1;              ///<   [12]    temporary write protection
            uint32_t perm_write_protect : 1;              ///<   [13]    permanent write protection
            uint32_t copy               : 1;              ///<   [14]    copy flag
            uint32_t file_format_grp    : 1;              ///<   [15]    file format group
            uint32_t reserved_20        : 5;              ///<   [16-20]
            uint32_t write_bl_partial   : 1;              ///<   [21]    partial blocks for write allowed
            uint32_t write_bl_len       : 4;              ///<   [25:22] max. write data block length
            uint32_t r2w_factor         : 3;              ///<   [28:26] write speed factor
            uint32_t reserved_29        : 2;              ///<   [30:29]
            uint32_t wp_grp_enable      : 1;              ///<   [31]    write protect group enable
            uint32_t wp_grp_size        : 7;              ///<   [38:32] write protect group size
            uint32_t sector_size        : 7;              ///<   [45:39] erase sector size
            uint32_t erase_blk_en       : 1;              ///<   [46]    erase single block enable
            uint32_t c_size_mult        : 3;              ///<   [49:47] device size multiplier*/
            uint32_t vdd_w_curr_max     : 3;              ///<   [52:50] max. write current for vdd max
            uint32_t vdd_w_curr_min     : 3;              ///<   [55:53] max. write current for vdd min
            uint32_t vdd_r_curr_max     : 3;              ///<   [58:56] max. read current for vdd max
            uint32_t vdd_r_curr_min     : 3;              ///<   [61:59] max. read current for vdd min
            uint32_t c_size             : 12;             ///<   [73:62] device size
            uint32_t reserved_74        : 2;              ///<   [75:74]
            uint32_t dsr_imp            : 1;              ///<   [76]    dsr implemented
            uint32_t read_blk_misalign  : 1;              ///<   [77]    read block misalignment
            uint32_t write_blk_misalign : 1;              ///<   [78]    write block misalignment
            uint32_t read_bl_partial    : 1;              ///<   [79]    partial blocks for read allowed
            uint32_t read_bl_len        : 4;              ///<   [83:80] max read data block length
            uint32_t ccc                : 12;             ///<   [95:84] card command classes
            uint32_t tran_speed         : 8;              ///<   [103:96] max. data transfer rate
            uint32_t nsac               : 8;              ///<   [111:104] data read access-time-2 clk cycles
            uint32_t taac               : 8;              ///<   [119:112] data read access-time-1
            uint32_t reserved_120       : 6;              ///<   [125:120]
            uint32_t csd_structure      : 2;              ///<   [127:126] csd structure
            uint32_t reserved_128       : 8;              ///<   [135:128]
        } csd_v1_b;

        /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
#if defined(__ICCARM__)
        __packed struct
#elif defined(  __GNUC__)
        /*LDRA_INSPECTED 293 S  All structures in this union must be packed since some bitfields span more than one 32-bit word. */
        struct __attribute__((__packed__))
#endif
        {
            uint32_t reserved_8         : 2;             ///<   [9:8]
            uint32_t file_format        : 2;             ///<   [11:10] file format
            uint32_t tmp_write_protect  : 1;             ///<   [12]    temporary write protection
            uint32_t perm_write_protect : 1;             ///<   [13]    permanent write protection
            uint32_t copy               : 1;             ///<   [14]    copy flag
            uint32_t file_format_grp    : 1;             ///<   [15]    file format group
            uint32_t reserved_20        : 5;             ///<   [20:16]
            uint32_t write_bl_partial   : 1;             ///<   [21]    partial blocks for write allowed
            uint32_t write_bl_len       : 4;             ///<   [25:22] max. write data block length
            uint32_t r2w_factor         : 3;             ///<   [28:26] write speed factor
            uint32_t reserved_29        : 2;             ///<   [30:29]
            uint32_t wp_grp_enable      : 1;             ///<   [31]    write protect group enable
            uint32_t wp_grp_size        : 7;             ///<   [38:32] write protect group size
            uint32_t sector_size        : 7;             ///<   [45:39] erase sector size
            uint32_t erase_blk_en       : 1;             ///<   [46]    erase single block enable
            uint32_t reserved_47        : 1;             ///<   [47]
            uint32_t c_size             : 22;            ///<   [69:48] device size
            uint32_t reserved_70        : 6;             ///<   [75:70]
            uint32_t dsr_imp            : 1;             ///<   [76]    dsr implemented
            uint32_t read_blk_misalign  : 1;             ///<   [77]    read block misalignment
            uint32_t write_blk_misalign : 1;             ///<   [78]    write block misalignment
            uint32_t read_bl_partial    : 1;             ///<   [79]    partial blocks for read allowed
            uint32_t read_bl_len        : 4;             ///<   [83:80] max read data block length
            uint32_t ccc                : 12;            ///<   [95:84] card command classes
            uint32_t tran_speed         : 8;             ///<   [103:96] max. data transfer rate
            uint32_t nsac               : 8;             ///<   [111:104] data read access-time-2 clk cycles
            uint32_t taac               : 8;             ///<   [119:112] data read access-time-1
            uint32_t reserved_120       : 6;             ///<   [125:120]
            uint32_t csd_structure      : 2;             ///<   [127:126] csd structure
            uint32_t reserved_128       : 8;             ///<   [135:128]
        } csd_v2_b;

        /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
#if defined(__ICCARM__)
        __packed struct
#elif defined(  __GNUC__)
        /*LDRA_INSPECTED 293 S  All structures in this union must be packed since some bitfields span more than one 32-bit word. */
        struct __attribute__((__packed__))
#endif
        {
            uint32_t reserved_0       : 1;

            uint32_t crc              : 7;
            uint32_t mfg_date         : 12;
            uint32_t reserved_23      : 4;
            uint32_t serial_number    : 32;
            uint32_t product_revision : 8;
            uint64_t product_name     : 40;
            uint32_t oem_id           : 16;
            uint32_t mfg_id           : 8;
            uint32_t reserved_128     : 8;
        } cid;
    };
} sdmmc_priv_csd_reg_t;




typedef struct st_sdmmc_priv_cid_reg
{
    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    union
    {
        uint8_t  array[16];

        /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
#if defined(__ICCARM__)
        __packed struct
#elif defined(  __GNUC__)
        /*LDRA_INSPECTED 293 S  All structures in this union must be packed since some bitfields span more than one 32-bit word. */
        struct __attribute__((__packed__))
#endif
        {
            volatile uint32_t sdrsp10;
            volatile uint32_t sdrsp32;
            volatile uint32_t sdrsp54;
            volatile uint32_t sdrsp76;
        } reg;

        /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
#if defined(__ICCARM__)
        __packed struct
#elif defined(  __GNUC__)
        /*LDRA_INSPECTED 293 S  All structures in this union must be packed since some bitfields span more than one 32-bit word. */
        struct __attribute__((__packed__))
#endif
        {
            uint32_t reserved_0       : 1;
            uint32_t crc              : 7;
            uint32_t mfg_date         : 12;
            uint8_t  reserved_23      : 4;
            uint32_t serial_number    : 32;
            uint8_t  product_revision : 8;
            uint8_t  product_name5    : 8;
            uint8_t  product_name4    : 8;
            uint8_t  product_name3    : 8;
            uint8_t  product_name2    : 8;
            uint8_t  product_name1    : 8;
            uint32_t oem_id           : 16;
            uint32_t mfg_id           : 8;
            uint32_t reserved_128     : 8;
        } cid;
    };

} sdmmc_priv_cid_reg_t;

/** SDMMC card specific data extended */
typedef struct st_sdmmc_priv_csd_reg_ext
{
    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    union
    {
        uint8_t  array[512];

        /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
        struct
        {
            uint8_t reserved_0[494];

            uint8_t large_unit_size_m1;
            uint8_t context_capabilities;
            uint8_t tag_res_size;
            uint8_t tag_unit_size;
            uint8_t data_tag_support;
            uint8_t max_packed_writes;
            uint8_t max_packed_reads;
            uint8_t bkops_support;
            uint8_t hpi_features;
            uint8_t s_cmd_set;
            uint8_t ext_security_err;
            uint8_t reserved_506[6];
            uint8_t reserved_512;
        } csd_ext;
    };
} sdmmc_priv_csd_reg_ext_t;

typedef union u_sdmmc_priv_sdio_arg_t
{
    uint32_t  arg;

    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    struct
    {
        uint32_t  data             : 8;  /* read/write data */
        uint32_t  stuff_8          : 1;  /* Stuff bit */
        uint32_t  register_address : 17; /* Register Address */
        uint32_t  stuff_26         : 1;  /* Stuff bit should be set to 1 */
        uint32_t  raw              : 1;  /* 1= Read after write. */
        uint32_t  function_number  : 3;  /* function0~function7 */
        uint32_t  rw_flag          : 1;  /* 0:read, 1:write */
    } cmd_52_arg;

    /*LDRA_INSPECTED 381 S Anonymous structures and unions are allowed in SSP code. */
    struct
    {
        uint32_t  count            : 9;  /* Byte/Block Count */
        uint32_t  register_address : 17; /* Register Address */
        uint32_t  op_code          : 1;  /* 0:fixed address, 1:incrementing address */
        uint32_t  block_mode       : 1;  /* 0: Byte, 1:Block */
        uint32_t  function_number  : 3;  /* function0~function7 */
        uint32_t  rw_flag          : 1;  /* 0:read, 1:write */
    } cmd_53_arg;
} sdmmc_priv_sdio_arg_t;


sdmmc_priv_csd_reg_t *Get_csd_reg(void);
sdmmc_priv_cid_reg_t *Get_cid_reg(void);

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif // ifndef SDCARD_H
