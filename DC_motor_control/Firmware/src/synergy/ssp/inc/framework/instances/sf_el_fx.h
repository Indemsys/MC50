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
* File Name    : sf_el_fx.h
* Description  : SF_EL_FX
***********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @ingroup SF_Library
 * @defgroup SF_EL_FX FX_IO Framework
 * @brief FileX adaptation layer for block media device drivers.
 *
 * SF_EL_FX FileX I/O is a single entry function which adapts FileX to Renesas Synergy block media device drivers.
 *
 * @section SF_EL_FX_API_SUMMARY Summary
 * SF_EL_FX Has no API file.
 *
 * @{
***********************************************************************************************************************/

#ifndef SF_EL_FX_API_H
#define SF_EL_FX_API_H

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
/* Include driver API and ThreadX API */
#include "bsp_api.h"
#include "fx_api.h"
#include "sf_block_media_api.h"

/* Common macro for SSP header files. There is also a corresponding SSP_FOOTER macro at the end of this file. */
SSP_HEADER

/**********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/
/** Version of the API defined in this file */
#define SF_EL_FX_API_VERSION_MAJOR   (2)
#define SF_EL_FX_API_VERSION_MINOR   (0)

/** Version of code that implements the API defined in this file */
#define SF_EL_FX_CODE_VERSION_MAJOR  (2U)
#define SF_EL_FX_CODE_VERSION_MINOR  (0U)

/**********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/

/** Block Media Control Block Type */

/** Media partition table update status */
typedef enum e_sf_el_fx_media_partition_table_update_status
{
    SF_EL_FX_PARTITION_TABLE_UPDATE_DISABLE = 0U, ///< Partition table update is disabled
    SF_EL_FX_PARTITION_TABLE_UPDATE_ENABLE  = 1U  ///< Partition table update is enabled
}sf_el_fx_media_partition_table_update_status_t;

/** Media partition exist status */
typedef enum e_sf_el_fx_media_partition_exist_status
{
    SF_EL_FX_NO_PARTITIONS       = 0U,  ///< No partition in memory
    SF_EL_FX_MULTIPLE_PARTITIONS = 1U   ///< Multiple partitions in memory
}sf_el_fx_media_partition_exist_status_t;

/** Media partition types */
typedef enum e_sf_el_fx_media_partition_type
{
    SF_EL_FX_PARTITION_TYPE_UNKNOWN                       = 0x00U,  ///< Partition type unknown
    SF_EL_FX_PARTITION_TYPE_FAT_16_MEMORY_LESS_THAN_32MB  = 0x04U,  ///< Partition type fat16 with size less than 32 MB
    SF_EL_FX_PARTITION_TYPE_EXTENDED                      = 0x05U,  ///< Partition type extended
    SF_EL_FX_PARTITION_TYPE_FAT_16_MEMORY_MORE_THAN_32MB  = 0x06U,  ///< Partition type fat16 with size more than 32 MB
    SF_EL_FX_PARTITION_TYPE_EXFAT                         = 0x07U,  ///< Partition type exFAT
    SF_EL_FX_PARTITION_TYPE_FAT_32                        = 0x0BU,  ///< Partition type fat32
	SF_EL_FX_PARTITION_TYPE_EXTENDED_INT13                = 0x0FU   ///< Partition type extended with interrupt 13
}sf_el_fx_media_partition_type_t;

/** Media initialization status */
typedef enum e_sf_el_fx_media_init_status
{
    SF_EL_FX_SYS_UNINIT       = 0U,  ///< System not initialized
    SF_EL_FX_SYS_INIT_PARTIAL = 1U,  ///< System partial initialized
    SF_EL_FX_SYS_INIT_FULL    = 2U   ///< System full initialized
}sf_el_fx_media_init_status_t;

/** Media MBR/EBR table status */
typedef enum e_sf_el_fx_media_mbr_ebr_status
{
    SF_EL_FX_DONOT_EXIST_OR_INVALID = 0x00U, ///< MBR/EBR table do not exist or is invalid
    SF_EL_FX_EXIST_AND_VALID        = 0x01U  ///< MBT/EBR table is valid
}sf_el_fx_media_mbr_ebr_status_t;

/** Media partition global open status */
typedef enum e_sf_el_fx_media_partition_global_open_status
{
    SF_EL_FX_PARTITION_GLOBAL_CLOSE = 0U, ///< All partitions are closed
    SF_EL_FX_PARTITION_GLOBAL_OPEN  = 1U  ///< At least one partition is open
}sf_el_fx_media_partition_global_open_status_t;

/** Media partition open status */
typedef enum e_sf_el_fx_media_partition_open_status
{
    SF_EL_FX_PARTITION_CLOSE = 0U,  ///< Partition is close
    SF_EL_FX_PARTITION_OPEN  = 1U   ///< Partition is open
}sf_el_fx_media_partition_open_status_t;

/** Media partition format status */
typedef enum e_sf_el_fx_media_partition_format_status
{
    SF_EL_FX_PARTITION_UNFORMATED          = 0U,  ///< Partition is not formated
    SF_EL_FX_PARTITION_PRE_RESET_FORMATED  = 1U,  ///< Partition exist before reset of the system
    SF_EL_FX_PARTITION_POST_RESET_FORMATED = 2U   ///< Partition created after system reset
}sf_el_fx_media_partition_format_status_t;

/** Media partition EBR buffer update */
typedef enum e_sf_el_fx_media_partition_ebr_buff_update
{
    SF_EL_FX_EBR_BUFF_UPDATE_PRESENT = 0U,  ///< Update present EBR table
    SF_EL_FX_EBR_BUFF_UPDATE_NEXT    = 1U,  ///< Update next EBR table
}sf_el_fx_media_partition_ebr_buff_update_t;

typedef void sf_el_fx_ctrl_t;

/** Individual partition offset, size, open and format status data */
typedef struct st_sf_el_fx_media_partition_info
{
    uint32_t                                 offset;         ///< Partition offset
    uint32_t                                 size;           ///< Partition size
    sf_el_fx_media_partition_open_status_t   open_status;    ///< Partition open status
    sf_el_fx_media_partition_format_status_t format_status;  ///< Partition format status
}sf_el_fx_media_partition_data_info_t;

/** MBR status and buffer data */
typedef struct st_sf_el_fx_media_mbr_info
{
    sf_el_fx_media_mbr_ebr_status_t status;     ///< MBR table status
    uint8_t                         buff[512];  ///< MBR buffer
}sf_el_fx_media_mbr_info_t;

/** EBR status, buffer and base address data */
typedef struct st_sf_el_fx_media_ebr_info
{
    sf_el_fx_media_mbr_ebr_status_t status;     ///< EBR table status
    uint32_t                        base_addr;  ///< EBR table base address
    uint8_t                         buff[512];  ///< EBR buffer
}sf_el_fx_media_ebr_info_t;

/** MBR and EBR table information */
typedef struct st_sf_el_fx_media_boot_record_table_info
{
    sf_el_fx_media_mbr_info_t mbr;  ///< MBR information
    sf_el_fx_media_ebr_info_t ebr;  ///< EBR information
}sf_el_fx_media_boot_record_table_info_t;

/** Global open and counter status of the partition */
typedef struct st_sf_el_fx_media_global_open_info
{
    sf_el_fx_media_partition_global_open_status_t status;   ///< Global partition open/close status
    uint32_t                                      counter;  ///< Partition open counter
}sf_el_fx_media_global_open_info_t;

/** Multiple partition status with total and actual count of partitions */
typedef struct st_sf_el_fx_media_partition_data_info
{
    sf_el_fx_media_partition_exist_status_t   multiple_partitions_status;  ///< Single or multiple partition status in the memory
    sf_el_fx_media_partition_data_info_t    * p_data;                      ///< Pointer to partition data
    uint32_t                                  total_count;                 ///< Total number of partitions
    uint32_t                                  actual_count;                ///< Actual number of partitions created
}sf_el_fx_media_partition_info_t;

/** Available media information */
typedef struct st_sf_el_fx_media_info
{
    sf_el_fx_media_init_status_t            init_status;           ///< Media init status
    sf_el_fx_media_global_open_info_t       global_open;           ///< Global open status and counter
    sf_el_fx_media_boot_record_table_info_t boot_record_table;     ///< Boot table
    sf_el_fx_media_partition_info_t         partition;             ///< Partition details
    uint32_t                                memory_total_sectors;  ///< Total sectors of the memory
    uint32_t                                memory_free_sectors;   ///< Memory sectors available
}sf_el_fx_media_info_t;

/** Callback function parameter data */
typedef struct st_sf_el_fx_callback_args
{
    uint32_t   * p_hidden_sector;   ///< Partition base address
    void const * p_context;         ///< Placeholder for user data
} sf_el_fx_callback_args_t;

/** SF_EL_FX configuration block. */
typedef struct st_sf_el_fx_config
{

    sf_el_fx_media_partition_data_info_t  * p_partition_data;
    sf_block_media_instance_t             * p_lower_lvl_block_media;                       ///< Lower level block media pointer
    void const                            * p_context;                                     ///< Pointer to user-provided context
    void                                  * p_extend;                                      ///< Any configuration data needed by the hardware
    uint32_t                                total_partitions;                              ///< Total partition as per the user configuration input
    void                                 (* p_callback)(sf_el_fx_callback_args_t *p_args); ///< User callback to specify partitions offset when partitions exist and board got reset
} sf_el_fx_config_t;

/**  SF_EL_FX instance control block. */
typedef struct st_sf_el_fx_instance_ctrl
{
    uint32_t                     open;                                           ///< Flag to determine if the device is open
    sf_el_fx_media_info_t        media_info;                                     ///< Available media and partition details
    sf_block_media_instance_t  * p_lower_lvl_block_media;                        ///< Lower level block media pointer
    void const                 * p_context;                                      ///< Pointer to user-provided context
    void                      (* p_callback)(sf_el_fx_callback_args_t * p_args); ///< User callback to specify partitions offset when partitions exist and board got reset
} sf_el_fx_instance_ctrl_t;

/** This structure encompasses everything that is needed to use an instance of this interface. */
typedef struct st_sf_el_fx_instance
{
    sf_el_fx_ctrl_t          * p_ctrl;    ///< Pointer to the control structure for this instance
    sf_el_fx_config_t  const * p_config;  ///< Pointer to the configuration structure for this instance
} sf_el_fx_t;

/**********************************************************************************************************************
 * Public Functions
 **********************************************************************************************************************/

void SF_EL_FX_BlockDriver (FX_MEDIA * media_ptr);

/* Common macro for SSP header files. There is also a corresponding SSP_HEADER macro at the top of this file. */
SSP_FOOTER

#endif //SF_EL_FX_API_H

/*******************************************************************************************************************//**
 * @} (end defgroup SF_EL_FX)
***********************************************************************************************************************/
