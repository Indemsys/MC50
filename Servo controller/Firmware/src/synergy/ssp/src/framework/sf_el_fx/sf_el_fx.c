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
 * File Name    : sf_el_fx.c
 * Description  : FileX File System I/O
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @addtogroup SF_EL_FX
 * @{
 **********************************************************************************************************************/

/** SSP FileX Support. */

/***********************************************************************************************************************
 * Includes
 **********************************************************************************************************************/

/* Include necessary system files.  */

#include "fx_api.h"
#include "bsp_api.h"
#include "sf_el_fx.h"
#include "sf_el_fx_cfg.h"

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/
#define SF_EL_FX_55AA_SIGNATURE_OFFSET                              (0x1FEU)
#define SF_EL_FX_BOOT_SIGNATURE_VALUE                               (0xAA55U)
#define SF_EL_FX_JUMP_INST_OFFSET                                   (0U)
#define SF_EL_FX_JUMP_INST_SET1_BYTE0                               (0xE9U)
#define SF_EL_FX_JUMP_INST_SET2_BYTE0                               (0xEBU)
#define SF_EL_FX_JUMP_INST_SET1_BYTE1                               (0x76U)
#define SF_EL_FX_JUMP_INST_SET2_BYTE2                               (0x90U)
#define SF_EL_FX_FAT16_SECTORS_PER_FAT                              (0x16U)
#define SF_EL_FX_FAT32_SECTORS_PER_FAT                              (0x24U)
#define SF_EL_FX_FAT16_SECTOR_COUNT                                 (0x13U)
#define SF_EL_FX_FAT32_SECTOR_COUNT                                 (0x20U)
#define SF_EL_FX_PARTITION0_OFFSET                                  (0x1C6U)
#define SF_EL_FX_PARTITION0_TYPE_OFFSET                             (0x1C2U)

#ifdef FX_ENABLE_EXFAT
#define SF_EL_FX_ENABLE_EXFAT           (1U)
#else
#define SF_EL_FX_ENABLE_EXFAT           (0U)
#endif

#define SF_EL_FX_MBR_EBR_SECTOR_SIZE                                (0x200U) //512

/* Macro Definitions for MBR(Master Boot Record) Table*/
#define SF_EL_FX_MBR_TOTAL_PARTITION_ENTRTY                         (4U)
#define SF_EL_FX_MBR_SIGNATURE_OFFSET                               SF_EL_FX_55AA_SIGNATURE_OFFSET
#define SF_EL_FX_MBR_SIGNATURE_VALUE                                SF_EL_FX_BOOT_SIGNATURE_VALUE
#define SF_EL_FX_MBR_SIGNATURE_VALUE_BYTE_LSB_MASK                  (0x00FFU)
#define SF_EL_FX_MBR_SIGNATURE_VALUE_BYTE_MSB_MASK                  (0xFF00U)
#define SF_EL_FX_MBR_SECTOR_SIZE                                    SF_EL_FX_MBR_EBR_SECTOR_SIZE
#define SF_EL_FX_MBR_BOOT_STRAP_START_OFFSET                        (0U)     // 0th byte
#define SF_EL_FX_MBR_BOOT_STRAP_END_OFFSET                          (0x1BDU) // 445th byte
#define SF_EL_FX_MBR_PRIMARY_PARTITION_ENTRY_SIZE                   (0x10U)  // 16 bytes per partition entry
#define SF_EL_FX_MBR_PRIMARY_PARTITION_1_OFFSET                     (0x1BEU)
#define SF_EL_FX_MBR_PRIMARY_PARTITION_2_OFFSET                     (0x1CEU)
#define SF_EL_FX_MBR_PRIMARY_PARTITION_3_OFFSET                     (0x1DEU)
#define SF_EL_FX_MBR_PRIMARY_PARTITION_4_OFFSET                     (0x1EEU)

/* Macro Definitions for MBR Primary Partition */
#define SF_EL_FX_MBR_PRIMARY_PARTITION_BOOT_FLAG_OFFSET             (0x00U)
#define SF_EL_FX_MBR_PRIMARY_PARTITION_CHS_START_VALUE_OFFSET       (0x01U)
#define SF_EL_FX_MBR_PRIMARY_PARTITION_TYPE_OFFSET                  (0x04U)
#define SF_EL_FX_MBR_PRIMARY_PARTITION_CHS_END_VALUE_OFFSET         (0x05U)
#define SF_EL_FX_MBR_PRIMARY_PARTITION_LBA_OF_FIRST_SECTOR_OFFSET   (0x08U)
#define SF_EL_FX_MBR_PRIMARY_PARTITION_SIZE_IN_SECTOR_OFFSET        (0x0CU)

/* Macro Definitions for EBR(Extended Boot Record) Table */
#define SF_EL_FX_EBR_TOTAL_PARTITION_ENTRTY                         (2U)
#define SF_EL_FX_EBR_SIGNATURE_OFFSET                               SF_EL_FX_55AA_SIGNATURE_OFFSET
#define SF_EL_FX_EBR_SIGNATURE_VALUE                                SF_EL_FX_BOOT_SIGNATURE_VALUE
#define SF_EL_FX_EBR_SECTOR_SIZE                                    SF_EL_FX_MBR_EBR_SECTOR_SIZE
#define SF_EL_FX_EBR_BOOT_STRAP_START_OFFSET                        SF_EL_FX_MBR_BOOT_STRAP_START_OFFSET
#define SF_EL_FX_EBR_BOOT_STRAP_END_OFFSET                          SF_EL_FX_MBR_BOOT_STRAP_END_OFFSET
#define SF_EL_FX_EBR_PRIMARY_PARTITION_ENTRY_SIZE                   SF_EL_FX_MBR_PRIMARY_PARTITION_ENTRY_SIZE
#define SF_EL_FX_EBR_PRIMARY_PARTITION_1_OFFSET                     SF_EL_FX_MBR_PRIMARY_PARTITION_1_OFFSET
#define SF_EL_FX_EBR_PRIMARY_PARTITION_2_OFFSET                     SF_EL_FX_MBR_PRIMARY_PARTITION_2_OFFSET
#define SF_EL_FX_EBR_PRIMARY_PARTITION_3_OFFSET                     SF_EL_FX_MBR_PRIMARY_PARTITION_3_OFFSET
#define SF_EL_FX_EBR_PRIMARY_PARTITION_4_OFFSET                     SF_EL_FX_MBR_PRIMARY_PARTITION_4_OFFSET

/* Macro Definitions for EBR Primary Partition */
#define SF_EL_FX_EBR_PRIMARY_PARTITION_BOOT_FLAG_OFFSET             SF_EL_FX_MBR_PRIMARY_PARTITION_BOOT_FLAG_OFFSET
#define SF_EL_FX_EBR_PRIMARY_PARTITION_CHS_START_VALUE_OFFSET       SF_EL_FX_MBR_PRIMARY_PARTITION_CHS_START_VALUE_OFFSET
#define SF_EL_FX_EBR_PRIMARY_PARTITION_TYPE_OFFSET                  SF_EL_FX_MBR_PRIMARY_PARTITION_TYPE_OFFSET
#define SF_EL_FX_EBR_PRIMARY_PARTITION_CHS_END_VALUE_OFFSET         SF_EL_FX_MBR_PRIMARY_PARTITION_CHS_END_VALUE_OFFSET
#define SF_EL_FX_EBR_PRIMARY_PARTITION_LBA_OF_FIRST_SECTOR_OFFSET   SF_EL_FX_MBR_PRIMARY_PARTITION_LBA_OF_FIRST_SECTOR_OFFSET
#define SF_EL_FX_EBR_PRIMARY_PARTITION_SIZE_IN_SECTOR_OFFSET        SF_EL_FX_MBR_PRIMARY_PARTITION_SIZE_IN_SECTOR_OFFSET


#define SF_EL_FX_SECTORS_TO_BE_ADDED_AFTER_MBR_OR_EBR               (63U)

#define SF_EL_FX_PARTITION_BOOTABLE_FLAG_NON_BOOTABLE               (0x00U)
#define SF_EL_FX_PARTITION_BOOTABLE_FLAG_BOOTABLE                   (0x80U)

#define SF_EL_FX_PARTITION_SIZE_BUFFER_INDEX_FAT_16                 (0x13U)
#define SF_EL_FX_PARTITION_SIZE_BUFFER_INDEX_FAT_32                 (0x20U)
#define SF_EL_FX_PARTITION_SIZE_BUFFER_INDEX_EXFAT                  (0x48U)

/* *************************************************************
 * 1KB =                                            1024 Bytes
 * 1MB =     1024KB =     1024 * 1024    =     1,048,576 Bytes
 * 1GB =     1024MB =     1024 * 1048576 = 1,073,741,824 Bytes
 * 2GB = 2 * 1024MB = 2 * 1024 * 1048576 = 2,147,483,648 Bytes
 * ***********************************************************
 * Size of sector = 512 Bytes
 * Total sectors for 2GB memory = 2147483648/512 sectors
 *                              = 4,194,304      sectors
 * ************************************************************/
#define SF_EL_FX_FAT16_MEMORY_SIZE_MAX                              (0x400000U)

/** "FIXO" in ASCII.  Used to determine if the control block is open. */
#define SF_EL_FX_OPEN                                               (0x4649584FU)

/***********************************************************************************************************************
 * Typedef definitions
 **********************************************************************************************************************/


/***********************************************************************************************************************
 * Private function prototypes
 **********************************************************************************************************************/
static void sf_el_fx_driver_init_media_info(sf_el_fx_instance_ctrl_t * const p_ctrl);
static ssp_err_t sf_el_fx_driver_init_mbr_ebr_signature_chk(sf_block_media_instance_t * p_block_media,
                                                            uint8_t                   * p_buff,
                                                            uint32_t                    start_sector);
static ssp_err_t sf_el_fx_driver_init_validate_partition_info(uint8_t  const * const p_buff,
                                                              uint32_t const         partition_offset);
static ssp_err_t sf_el_fx_driver_init_update_partition_details(sf_el_fx_instance_ctrl_t                       * const p_ctrl,
                                                               uint8_t                                        * const p_buff,
                                                               uint32_t                                         const partition_offset,
                                                               uint32_t                                         const partition_counter,
                                                               uint32_t                                         const partition_lba,
                                                               sf_el_fx_media_partition_table_update_status_t   const partition_table_status);
static ssp_err_t  sf_el_fx_driver_init_process_ebr(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                                   sf_block_media_instance_t * const p_block_media);
static ssp_err_t sf_el_fx_driver_init_process_mbr(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                                  sf_block_media_instance_t * const p_block_media);
static void sf_el_fx_driver_init_process_mbr_ebr(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                                 sf_block_media_instance_t * const p_block_media);
static void sf_el_fx_driver_init_system_full(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                             sf_block_media_instance_t * const p_block_media);
static void sf_el_fx_driver_init_system_partial(sf_el_fx_instance_ctrl_t * const p_ctrl);
static ssp_err_t sf_el_fx_driver_partition_entry_index(sf_el_fx_instance_ctrl_t * const p_ctrl,
                                                       uint32_t                         sector_offset,
                                                       uint32_t                 * const p_partition_index);
static ssp_err_t sf_el_fx_driver_update_partition_open_status(sf_el_fx_instance_ctrl_t * const p_ctrl,
                                                              uint32_t                   const sector_offset);
static ssp_err_t sf_el_fx_driver_update_partition_close_status(sf_el_fx_instance_ctrl_t * const p_ctrl,
                                                               uint32_t                   const sector_offset);
static ssp_err_t sf_el_fx_driver_find_partition_entry(sf_el_fx_instance_ctrl_t * const p_ctrl,
                                                      uint32_t                   const sector_offset);
static ssp_err_t sf_el_fx_driver_validate_new_partition_status(sf_el_fx_instance_ctrl_t * const p_ctrl,
                                                               uint32_t                   const sector_offset,
                                                               uint32_t                   const sector_end);
static ssp_err_t sf_el_fx_driver_validate_new_partition(sf_el_fx_instance_ctrl_t       * const p_ctrl,
                                                        uint8_t                  const * const p_buff,
                                                        uint32_t                         sector_offset);
static ssp_err_t sf_el_fx_driver_new_partition_read_write_buff_comp(uint8_t  const * const p_buff_write,
                                                                    uint8_t  const * const p_buff_read,
                                                                    uint32_t         const buff_size);
static void sf_el_fx_driver_new_partition_temp_to_main_buff_cpy(uint8_t       * const p_buff_dest,
                                                                uint8_t const * const p_buff_source,
                                                                uint32_t        const buff_size);
static void sf_el_fx_driver_write_buff_sig(uint8_t * const p_buff);
static void sf_el_fx_driver_fill_partition_buff(uint8_t                         * const p_buff,
                                                uint32_t                          const partition_buff_offset,
                                                uint32_t                          const sector_offset,
                                                uint32_t                          const sector_size,
                                                sf_el_fx_media_partition_type_t   const partition_type);
static void sf_el_fx_driver_new_partition_detail_update(sf_el_fx_instance_ctrl_t * const p_ctrl,
                                                        uint32_t                   const partition_counter,
                                                        uint32_t                   const sector_offset,
                                                        uint32_t                   const sector_size);
static ssp_err_t sf_el_fx_driver_update_buff(sf_block_media_instance_t       * const p_block_media,
                                             uint8_t                         * const p_buff,
                                             uint32_t                          const buff_partition_offset,
                                             uint32_t                          const sector_offset,
                                             uint32_t                          const sector_size,
                                             uint32_t                          const mbr_ebr_rw_sector_offset,
                                             sf_el_fx_media_partition_type_t   const partition_type);
static ssp_err_t sf_el_fx_driver_update_ebr_buff(sf_el_fx_instance_ctrl_t                   * const p_ctrl,
                                                 sf_block_media_instance_t                  * const p_block_media,
                                                 uint32_t                                     const sector_offset,
                                                 uint32_t                                     const sector_size,
                                                 sf_el_fx_media_partition_type_t              const partition_type,
                                                 sf_el_fx_media_partition_ebr_buff_update_t   const buff_update_state);
static ssp_err_t sf_el_fx_driver_update_mbr_buff(sf_el_fx_instance_ctrl_t        * const p_ctrl,
                                                 sf_block_media_instance_t       * const p_block_media,
                                                 uint32_t                          const sector_offset,
                                                 uint32_t                          const sector_size,
                                                 sf_el_fx_media_partition_type_t   const partition_type);
static ssp_err_t sf_el_fx_driver_sector_size_get(sf_el_fx_instance_ctrl_t              * const p_ctrl,
                                                 uint8_t                         const * const p_buff,
                                                 uint32_t                              * const p_sector_size,
                                                 sf_el_fx_media_partition_type_t       * const p_partition_type);
static ssp_err_t sf_el_fx_driver_update_mbr_ebr(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                                sf_block_media_instance_t * const p_block_media,
                                                uint8_t             const * const p_buff,
                                                uint32_t            const         sector_offset);
static void sf_el_fx_driver_open(sf_el_fx_instance_ctrl_t * p_ctrl, sf_el_fx_config_t const * const p_config);
static ssp_err_t check_partition_offset(uint8_t * p_sector, uint32_t * p_partition_offset);
static ssp_err_t check_fat_boot_record (uint8_t * p_sector, uint32_t * p_partition_offset);
static ssp_err_t sf_el_fx_driver_release_sectors_handler(sf_block_media_instance_t * p_block_media,
                                                         uint32_t                    num_sectors,
                                                         uint32_t                    start_sector);
static ssp_err_t sf_el_fx_driver_init_handler(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                              sf_block_media_instance_t * const p_block_media,
                                              uint32_t                  * write_protect,
                                              uint32_t                  * requires_sector_release);
static ssp_err_t sf_el_fx_driver_single_partition_boot_read_handler(sf_block_media_instance_t * p_block_media,
                                                   uint8_t                   * p_buffer,
                                                   uint32_t            const   sector_offset,
                                                   uint32_t                  * driver_status);
static ssp_err_t sf_el_fx_driver_multi_partition_boot_read_handler(sf_block_media_instance_t * p_block_media,
                                                   uint8_t                   * p_buffer,
                                                   uint32_t            const   sector_offset,
                                                   uint32_t                  * driver_status);
static ssp_err_t sf_el_fx_driver_request_handler_initialization(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                                                sf_block_media_instance_t * const p_block_media,
                                                                FX_MEDIA                  *       p_fx_media );
static ssp_err_t sf_el_fx_driver_request_handler_driver_uninitialize(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                                                     sf_block_media_instance_t * const p_block_media,
                                                                     FX_MEDIA                  *       p_fx_media );
static ssp_err_t sf_el_fx_driver_request_handler_read_media_boot_sector(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                                                        sf_block_media_instance_t * const p_block_media,
                                                                        FX_MEDIA                  *       p_fx_media );
static ssp_err_t sf_el_fx_driver_request_handler_write_media_boot_sector(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                                                         sf_block_media_instance_t * const p_block_media,
                                                                         FX_MEDIA                  *       p_fx_media );
static ssp_err_t sf_el_fx_driver_request_handler(FX_MEDIA * p_fx_media);

/***********************************************************************************************************************
 * Private global variables
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * Functions
 **********************************************************************************************************************/


/** The file system relies on the media to be formatted prior to creating directories and files
 *  The sector size and sector count will change depending on the media type and size.
 *
 *  The File Allocation Table (FAT) starts after the reserved sectors in the media. The FAT area is
 *  basically an array of 12-bit, 16-bit, or 32-bit entries that determine if that cluster is
 *  allocated or part of a chain of clusters comprising a subdirectory or a file.
 *  The size of each FAT entry is determined by the number of clusters that need to be represented.
 *  If the number of clusters (derived from the total sectors divided by the sectors per cluster)
 *  is less than 4,086, 12-bit FAT entries are used. If the total number of clusters is greater
 *  than 4,086 and less than or equal to 65,525, 16-bit FAT entries are used. Otherwise, if the
 *  total number of clusters is greater than 65,525, 32-bit FAT entries are used.
 */

/*******************************************************************************************************************//**
 * @brief  Access Block Media device functions open, close, read, write and control.
 *
 * The SF_EL_FX_BlockDriver function is called from the FileX file system driver and issues requests to a Block Media
 * device through the Synergy Block Media Interface. Uses block media driver for accesses.
 * @param[in,out] p_fx_media                        FileX media control block. All information about each open media
 *                                                  device are maintained in by the FX_MEDIA data type.
 *                                                  The I/O driver communicates the success or failure of the request
 *                                                  through the fx_media_driver_status member of FX_MEDIA
 *                                                  (p_fx_media->fx_media_driver_status).
 *                                                  Possible values are documented in the FileX User Guide.
 * @retval                                          none.
 *
 * @note                                            This function returns nothing, but updates FileX media control block.This function calls
 *                                                  sf_el_fx_driver_request_handler
 **********************************************************************************************************************/
void  SF_EL_FX_BlockDriver (FX_MEDIA * p_fx_media)
{
    ssp_err_t ret_val = SSP_SUCCESS;

    /** Initialize FileX I/O status to error. It will change to FX_SUCCESS unless an operation fails. */
    p_fx_media->fx_media_driver_status = (UINT)FX_IO_ERROR;

#if  (SF_EL_FX_CFG_PARAM_CHECKING_ENABLE)

    /** Validate FX_MEDIA::fx_media_driver_info and update FX_MEDIA::fx_media_driver_status to FX_PTR_ERROR in-case of
     *    any failure. */
    if (NULL == p_fx_media->fx_media_driver_info)
    {
        p_fx_media->fx_media_driver_status = (UINT)FX_PTR_ERROR;
        return;
    }

    /** Validate sf_el_fx_config_t::p_lower_lvl_block_media and update FX_MEDIA::fx_media_driver_status to FX_PTR_ERROR
     *    in-case of any failure. */
    if ((NULL == (sf_block_media_instance_t *)
                   ((sf_el_fx_config_t *)(((sf_el_fx_t *)p_fx_media->fx_media_driver_info)->p_config))->p_lower_lvl_block_media) ||
        (NULL == (sf_el_fx_config_t *)
                   (((sf_el_fx_t *)p_fx_media->fx_media_driver_info)->p_config))                                                 ||
        (NULL == (sf_el_fx_instance_ctrl_t *)
                   (((sf_el_fx_t *)p_fx_media->fx_media_driver_info)->p_ctrl)))
    {
        p_fx_media->fx_media_driver_status = (UINT)FX_PTR_ERROR;
        return;
    }

#endif

    /** Pass control to sf_el_fx_driver_request_handler, which is responsible for setting
     *    FX_MEDIA::fx_media_driver_status in-case of any failure. */
    ret_val = sf_el_fx_driver_request_handler(p_fx_media);

    if (SSP_SUCCESS == ret_val)
    {
        /**  *
         *  Update FX_MEDIA::fx_media_driver_status to FX_SUCCESS on successful driver request. */
        p_fx_media->fx_media_driver_status = (UINT)FX_SUCCESS;
    }
}


/*******************************************************************************************************************//**
 * @} (end addtogroup SF_EL_FX)
 **********************************************************************************************************************/

/*******************************************************************************************************************//**
 * @brief   Check for partition open status
 *
 * @param[in] p_ctrl        Pointer to SF_EL_FX control block
 * @param[in] sector_offset Partition offset
 *
 * @retval  SSP_SUCCESS                 If partition is in close state.
 * @retval  SSP_ERR_ASSERTION           If partition is already in open state.
 *
 * @return                            See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                    codes. This function calls
 *                                        * sf_el_fx_driver_partition_entry_index
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_user_input_hidden_sector_status(sf_el_fx_instance_ctrl_t * const p_ctrl,
                                                          uint32_t                   const sector_offset)
{
    ssp_err_t ret_val = SSP_SUCCESS;
    uint32_t partition_index = 0xFFFFFFFF;

    /** Check if the partition details exist in the table or not */
    ret_val = sf_el_fx_driver_partition_entry_index(p_ctrl, sector_offset, &partition_index);

    /** Check if the return value is success or not */
    if (SSP_SUCCESS == ret_val)
    {
        /** Check the state of the partition open/close */
        if (SF_EL_FX_PARTITION_CLOSE != p_ctrl->media_info.partition.p_data[partition_index].open_status)
        {
            /** Return error if the parition is in open state */
            ret_val = SSP_ERR_ASSERTION;
        }
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Parameter check for control block and configuration block
 *
 * @param[in] p_ctrl          Pointer to SF_EL_FX control block
 * @param[in] p_hidden_sector Pointer to partition hidden sector
 *
 * @retval  SSP_SUCCESS                 If parameter p_ctrl, p_cfg or clock rate not NULL.
 * @retval  SSP_ERR_ASSERTION           If no partition is in closed state.
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_linear_partition_open(sf_el_fx_instance_ctrl_t * const p_ctrl,
                                                uint32_t                 * p_hidden_sector)
{
    ssp_err_t ret_val = SSP_SUCCESS;
    uint32_t i = 0;

    for (i = 0U; i < p_ctrl->media_info.partition.actual_count; i++)
    {
        if (SF_EL_FX_PARTITION_CLOSE == p_ctrl->media_info.partition.p_data[i].open_status)
        {
            *p_hidden_sector = p_ctrl->media_info.partition.p_data[i].offset;
            i = (uint32_t)0xFFFFFFFF;
            break;
        }
    }

    if ((uint32_t)0xFFFFFFFF != i)
    {
        ret_val = SSP_ERR_ASSERTION;
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Callback/Linear process to open a partition
 *
 * @param[in] p_ctrl          Pointer to SF_EL_FX control block
 * @param[in] p_hidden_sector Pointer to partition hidden sector
 *
 * @retval  SSP_SUCCESS                 If parameter p_ctrl, p_cfg or clock rate not NULL.
 * @retval  SSP_ERR_ASSERTION           If no partition is in closed state or the user input in callback is invalid.
 *
 * @return                            See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                    codes. This function calls
 *                                        * sf_el_fx_user_input_hidden_sector_status
 *                                        * sf_el_fx_linear_partition_open
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_callback(sf_el_fx_instance_ctrl_t * const p_ctrl, uint32_t * p_hidden_sector)
{
    ssp_err_t ret_val = SSP_SUCCESS;
    sf_el_fx_callback_args_t args;

    /** Check if user callback is NULL or not */
    if (NULL != p_ctrl->p_callback)
    {

        args.p_context = p_ctrl->p_context;
        args.p_hidden_sector = p_hidden_sector;

        p_ctrl->p_callback(&args);

        /** Check the partition state selected by user */
        ret_val = sf_el_fx_user_input_hidden_sector_status(p_ctrl, *p_hidden_sector);
    }
    else
    {
        /** Open the partition in linear format */
        ret_val = sf_el_fx_linear_partition_open(p_ctrl, p_hidden_sector);
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Reset variables associated with partition
 *
 * @param[in] p_ctrl          Pointer to SF_EL_FX control block
 *
 * @retval  none
 *                            This function calls
 *                            * memset
***********************************************************************************************************************/
static void sf_el_fx_driver_init_media_info(sf_el_fx_instance_ctrl_t * const p_ctrl)
{
    uint32_t i = 0;
    uint32_t partition_total_count = 0U;

    /* Initialize MBR buffer */
    memset(p_ctrl->media_info.boot_record_table.mbr.buff, 0xFF, SF_EL_FX_MBR_SECTOR_SIZE);

    /* Initialize EBR buffer */
    memset(p_ctrl->media_info.boot_record_table.ebr.buff, 0xFF, SF_EL_FX_EBR_SECTOR_SIZE);

    /* Initialize MBR status variable */
    p_ctrl->media_info.boot_record_table.mbr.status = SF_EL_FX_DONOT_EXIST_OR_INVALID;

    /* Initialize MBR status variable */
    p_ctrl->media_info.boot_record_table.ebr.status = SF_EL_FX_DONOT_EXIST_OR_INVALID;

    /* Initialize open status variable */
    p_ctrl->media_info.global_open.status  = SF_EL_FX_PARTITION_GLOBAL_CLOSE;

    /* Initialize/Reset open counter variable */
    p_ctrl->media_info.global_open.counter = 0U;

    /* Assign the total partitions entered by the user to a local variable */
    partition_total_count = p_ctrl->media_info.partition.total_count;

    /* Initialize/Reset array of structure related to partitions data */
    for (i = 0U; i < partition_total_count; i++)
    {
        p_ctrl->media_info.partition.p_data[i].format_status = SF_EL_FX_PARTITION_UNFORMATED;
        p_ctrl->media_info.partition.p_data[i].offset        = 0U;
        p_ctrl->media_info.partition.p_data[i].open_status   = SF_EL_FX_PARTITION_CLOSE;
        p_ctrl->media_info.partition.p_data[i].size          = 0U;
    }

    /* Initialize/Reset total memory sectors variable */
    p_ctrl->media_info.memory_total_sectors = 0U;

    /* Initialize/Reset memory free sectors variable */
    p_ctrl->media_info.memory_free_sectors = 0U;

    /* Initialize/Reset actual partition exist in the system */
    p_ctrl->media_info.partition.actual_count = 0U;
}

/*******************************************************************************************************************//**
 * @brief   Verify signature in MBR/EBR table
 *
 * @param[in] p_ctrl          Pointer to SF_EL_FX control block
 * @param[in] p_buff          Pointer to buffer
 * @param[in] start_sector    MBR/EBR sector
 *
 * @retval  SSP_SUCCESS                 If valid signature found.
 * @retval  SSP_ERR_ASSERTION           If no/invalid signature found.
 *
 * @return                            See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                    codes. This function calls
 *                                        * sf_block_media_api_t::read
 *                                        * _fx_utility_16_unsigned_read
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_init_mbr_ebr_signature_chk(sf_block_media_instance_t * p_block_media,
                                                            uint8_t                   * p_buff,
                                                            uint32_t                    start_sector)
{
    ssp_err_t ret_val = SSP_SUCCESS;

    uint16_t check_AA55_sector_end;
    uint16_t check_AA55_1FE;

    /** Read MBR/EBR sector as per the specified start sector data. For MBR start sector is always passed as zero */
    ret_val = p_block_media->p_api->read(p_block_media->p_ctrl, p_buff, start_sector, 1);

    /* Check if read driver is successful or not */
    if (SSP_SUCCESS == ret_val)
    {
        /* Update variable with the signature in the buffer */
        check_AA55_sector_end = (uint16_t)_fx_utility_16_unsigned_read(&p_buff[SF_EL_FX_MBR_EBR_SECTOR_SIZE - 2]);
        check_AA55_1FE = (uint16_t)_fx_utility_16_unsigned_read(&p_buff[SF_EL_FX_55AA_SIGNATURE_OFFSET]);

        /* Check if the buffer is having a valid signature or not */
        if ((SF_EL_FX_BOOT_SIGNATURE_VALUE != check_AA55_sector_end) ||
            (SF_EL_FX_BOOT_SIGNATURE_VALUE != check_AA55_1FE))
        {
            /* Update the return variable with error if invalid/no signature found */
            ret_val = SSP_ERR_ABORTED;
        }
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Partition offset and size entry in the buffer validation
 *
 * @param[in] p_buff           Pointer to buffer
 * @param[in] partition_offset Offset or start address of the partition
 *
 * @retval  SSP_SUCCESS                 If buffer has valid partition information.
 * @retval  SSP_ERR_ASSERTION           If buffer has invalid partition information.
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_init_validate_partition_info(uint8_t  const * const p_buff,
                                                              uint32_t const         partition_offset)
{
    ssp_err_t ret_val = SSP_SUCCESS;

    /* Check if MBR/EBR entry starting from location
     * SF_EL_FX_MBR_PRIMARY_PARTITION_LBA_OF_FIRST_SECTOR_OFFSET or
     * SF_EL_FX_MBR_PRIMARY_PARTITION_SIZE_IN_SECTOR_OFFSET
     * are not zero */
    if (!(p_buff[partition_offset])      &&
        (!(p_buff[partition_offset + 1]))  &&
        (!(p_buff[partition_offset + 2]))  &&
        !(p_buff[partition_offset + 3]))
    {
        return SSP_ERR_ASSERTION;
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Update table with information of partitions exist
 *
 * @param[in] p_ctrl                 Pointer to SF_EL_FX control block
 * @param[in] p_buff                 Pointer to buffer
 * @param[in] partition_offset       MBR/EBR sector
 * @param[in] partition_counter      Counter for partition number
 * @param[in] partition_table_status Variable to specify if the partition table needs to be updated or not
 *
 * @retval  SSP_SUCCESS                 If valid partition.
 * @retval  SSP_ERR_ASSERTION           If invalid partition.
 *
 * @return                            See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                    codes. This function calls
 *                                        * sf_el_fx_driver_init_validate_partition_info
 *                                        * _fx_utility_32_unsigned_read
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_init_update_partition_details(sf_el_fx_instance_ctrl_t                       * const p_ctrl,
                                                               uint8_t                                        * const p_buff,
                                                               uint32_t                                         const partition_offset_addr_in_buff,
                                                               uint32_t                                         const partition_counter,
                                                               uint32_t                                         const partition_lba,
                                                               sf_el_fx_media_partition_table_update_status_t   const partition_table_status)
{
    ssp_err_t ret_val                     = SSP_SUCCESS;
    uint32_t  offset_buff                 = 0U;
    uint32_t  partition_offset_calculated = 0U;

    /* Partition offset is the base address in MBR/EBR table for any partition
     * and SF_EL_FX_MBR_PRIMARY_PARTITION_LBA_OF_FIRST_SECTOR_OFFSET gives the location of LBA data */
    offset_buff = partition_offset_addr_in_buff + SF_EL_FX_MBR_PRIMARY_PARTITION_LBA_OF_FIRST_SECTOR_OFFSET;

    /* Check if LBA data is valid or not*/
    ret_val = sf_el_fx_driver_init_validate_partition_info(p_buff, offset_buff);
    if (SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }

    /** Update partition table with partition offset data */
    if (SF_EL_FX_PARTITION_TABLE_UPDATE_ENABLE == partition_table_status)
    {
        partition_offset_calculated = (uint32_t)_fx_utility_32_unsigned_read(&p_buff[offset_buff]);

        if (3U > partition_counter)
        {
            p_ctrl->media_info.partition.p_data[partition_counter].offset = partition_offset_calculated;
            partition_offset_calculated = 0U;
        }
        else
        {
            p_ctrl->media_info.partition.p_data[partition_counter].offset = (partition_offset_calculated + partition_lba);
        }
    }

    /* Partition offset is the base address in MBR/EBR table for any partition
     * and SF_EL_FX_MBR_PRIMARY_PARTITION_SIZE_IN_SECTOR_OFFSET gives the location of partition size data */
    offset_buff = partition_offset_addr_in_buff + SF_EL_FX_MBR_PRIMARY_PARTITION_SIZE_IN_SECTOR_OFFSET;

    /* Check if partition size data is valid or not*/
    ret_val = sf_el_fx_driver_init_validate_partition_info(p_buff, offset_buff);
    if (SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }

    /** Update partition table with partition size and format status */
    if (SF_EL_FX_PARTITION_TABLE_UPDATE_ENABLE == partition_table_status)
    {
        p_ctrl->media_info.partition.p_data[partition_counter].size = (uint32_t)_fx_utility_32_unsigned_read(&p_buff[offset_buff]) + partition_offset_calculated;

        p_ctrl->media_info.partition.p_data[partition_counter].format_status = SF_EL_FX_PARTITION_PRE_RESET_FORMATED;
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Read EBR table and update partition table
 *
 * @param[in] p_ctrl                 Pointer to SF_EL_FX control block
 * @param[in] p_block_media          Pointer to lower level block media control block
 *
 * @retval  SSP_SUCCESS                 If valid partition.
 * @retval  SSP_ERR_ASSERTION           If invalid partition.
 *
 * @return                            See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                    codes. This function calls
 *                                        * sf_el_fx_driver_init_mbr_ebr_signature_chk
 *                                        * sf_el_fx_driver_init_update_partition_details
 *                                        * _fx_utility_32_unsigned_read
***********************************************************************************************************************/
static ssp_err_t  sf_el_fx_driver_init_process_ebr(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                                   sf_block_media_instance_t * const p_block_media)
{
    ssp_err_t ret_val = SSP_SUCCESS;
    uint32_t const partition_offset[] = {SF_EL_FX_EBR_PRIMARY_PARTITION_1_OFFSET,
                                         SF_EL_FX_EBR_PRIMARY_PARTITION_2_OFFSET};
    uint32_t partition_counter = 0U;
    uint32_t ebr_lba           = 0U;
    uint32_t offset_buff       = 0U;

    /* Assign the local variable with actual partition counter */
    partition_counter = p_ctrl->media_info.partition.actual_count;

    /* Assign base address of the EBR table to a local variable */
    ebr_lba = p_ctrl->media_info.boot_record_table.ebr.base_addr;

    do
    {
        /* Reset EBR buffer before reading data from memory */
        memset(p_ctrl->media_info.boot_record_table.ebr.buff, 0x00, SF_EL_FX_EBR_SECTOR_SIZE);

        /** Check for valid EBR signature */
        ret_val = sf_el_fx_driver_init_mbr_ebr_signature_chk(p_block_media,
                                                             ((uint8_t *)p_ctrl->media_info.boot_record_table.ebr.buff),
                                                             ebr_lba);
        if (SSP_SUCCESS != ret_val)
        {
            break;
        }

        /* Update status of EBR indicating EBR exist and is valid */
        p_ctrl->media_info.boot_record_table.ebr.status = SF_EL_FX_EXIST_AND_VALID;

        if (SF_EL_FX_PARTITION_BOOTABLE_FLAG_NON_BOOTABLE == (uint32_t)p_ctrl->media_info.boot_record_table.ebr.buff[partition_offset[0]])
        {
            /** Update partition table with partition offset and size */
           ret_val = sf_el_fx_driver_init_update_partition_details(p_ctrl,
                                                                   ((uint8_t *) p_ctrl->media_info.boot_record_table.ebr.buff),
                                                                   partition_offset[0],
                                                                   partition_counter,
                                                                   ebr_lba,
                                                                   SF_EL_FX_PARTITION_TABLE_UPDATE_ENABLE);
           if (SSP_SUCCESS != ret_val)
           {
               break;
           }

           /* Subtract the sector size from the available sectors of the memory */
           p_ctrl->media_info.memory_free_sectors -= p_ctrl->media_info.partition.p_data[partition_counter].size;

           /** Increment the partition counter */
           partition_counter++;

           /** Check if another EBR table exist linked to the present EBR table */
           ret_val = sf_el_fx_driver_init_update_partition_details(p_ctrl,
                                                                   ((uint8_t *) p_ctrl->media_info.boot_record_table.ebr.buff),
                                                                   partition_offset[1],
                                                                   partition_counter,
                                                                   0U,
                                                                   SF_EL_FX_PARTITION_TABLE_UPDATE_DISABLE);
           if (SSP_SUCCESS != ret_val)
           {
               break;
           }

           /* Get the offset in EBR table after adding SF_EL_FX_EBR_PRIMARY_PARTITION_2_OFFSET and
            * SF_EL_FX_MBR_PRIMARY_PARTITION_LBA_OF_FIRST_SECTOR_OFFSET */
           offset_buff = partition_offset[1] + SF_EL_FX_MBR_PRIMARY_PARTITION_LBA_OF_FIRST_SECTOR_OFFSET;

           /** Assign next partition offset after reading from buffer location */
           ebr_lba = (uint32_t)_fx_utility_32_unsigned_read(((uint8_t *)&p_ctrl->media_info.boot_record_table.ebr.buff[offset_buff]));

           ebr_lba += p_ctrl->media_info.boot_record_table.ebr.base_addr;

        }

    } while(SSP_SUCCESS == ret_val);

    p_ctrl->media_info.partition.actual_count = partition_counter;

   return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Read MBR table and update partition table
 *
 * @param[in] p_ctrl                 Pointer to SF_EL_FX control block
 * @param[in] p_block_media          Pointer to lower level block media control block
 *
 * @retval  SSP_SUCCESS                 If valid partition.
 * @retval  SSP_ERR_ASSERTION           If invalid partition.
 *
 * @return                            See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                    codes. This function calls
 *                                        * sf_el_fx_driver_init_mbr_ebr_signature_chk
 *                                        * sf_el_fx_driver_init_update_partition_details
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_init_process_mbr(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                                  sf_block_media_instance_t * const p_block_media)
{
   ssp_err_t ret_val = SSP_SUCCESS;
   uint32_t i = 0U;
   uint32_t const partition_offset[] = {SF_EL_FX_MBR_PRIMARY_PARTITION_1_OFFSET,
                                        SF_EL_FX_MBR_PRIMARY_PARTITION_2_OFFSET,
                                        SF_EL_FX_MBR_PRIMARY_PARTITION_3_OFFSET,
                                        SF_EL_FX_MBR_PRIMARY_PARTITION_4_OFFSET};
   uint32_t partition_counter = 0U;

   /* Reset MBR buffer before reading data from memory */
   memset(p_ctrl->media_info.boot_record_table.mbr.buff, 0x00, SF_EL_FX_MBR_SECTOR_SIZE);

   /** Check for valid MBR signature */
   ret_val = sf_el_fx_driver_init_mbr_ebr_signature_chk(p_block_media,
                                                        ((uint8_t *)p_ctrl->media_info.boot_record_table.mbr.buff),
                                                        0);
   if (SSP_SUCCESS != ret_val)
   {
       return ret_val;
   }

   /* Update status of MBR indicating MBR exist and is valid */
   p_ctrl->media_info.boot_record_table.mbr.status = SF_EL_FX_EXIST_AND_VALID;

   /* Reset actual partition counter */
   p_ctrl->media_info.partition.actual_count = 0U;

   for (i = 0U; i < SF_EL_FX_MBR_TOTAL_PARTITION_ENTRTY; i++)
   {
       /** Check if partition in MBR table is bootable/non-bootable */
       if ((SF_EL_FX_PARTITION_BOOTABLE_FLAG_NON_BOOTABLE == (uint32_t)p_ctrl->media_info.boot_record_table.mbr.buff[partition_offset[i]]) ||
           (SF_EL_FX_PARTITION_BOOTABLE_FLAG_BOOTABLE     == (uint32_t)p_ctrl->media_info.boot_record_table.mbr.buff[partition_offset[i]]))
       {
           /** Update partition table with partition offset and size */
           ret_val = sf_el_fx_driver_init_update_partition_details(p_ctrl,
                                                                   ((uint8_t *) p_ctrl->media_info.boot_record_table.mbr.buff),
                                                                   partition_offset[i],
                                                                   partition_counter,
                                                                   0U,
                                                                   SF_EL_FX_PARTITION_TABLE_UPDATE_ENABLE);
           if ((SSP_SUCCESS != ret_val) || (i == (SF_EL_FX_MBR_TOTAL_PARTITION_ENTRTY - 1)))
           {
               break;
           }

           /* Update free sectors if partition counter is less than 3 */
           if ((SSP_SUCCESS == ret_val) && (partition_counter < 3U))
           {
               /* Subtract the sector size from the available sectors of the memory */
               p_ctrl->media_info.memory_free_sectors -= p_ctrl->media_info.partition.p_data[partition_counter].size;
           }

           /* Increment partition counter */
           partition_counter++;
       }
   }

   /* Assign the partition counter calculated to the global variable */
   p_ctrl->media_info.partition.actual_count = partition_counter;

   return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Read MBR/EBR table and update partition table
 *
 * @param[in] p_ctrl                 Pointer to SF_EL_FX control block
 * @param[in] p_block_media          Pointer to lower level block media control block
 *
 * @retval  none
 *
 *                                   This function calls
 *                                        * sf_el_fx_driver_init_process_mbr
 *                                        * sf_el_fx_driver_init_process_ebr
***********************************************************************************************************************/
static void sf_el_fx_driver_init_process_mbr_ebr(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                                 sf_block_media_instance_t * const p_block_media)
{
    ssp_err_t ret_val = SSP_SUCCESS;

    /** Check if total count of partition is greater than 1 */
    if (1U < p_ctrl->media_info.partition.total_count)
    {
        /** Update partition table based on MBR table */
        ret_val = sf_el_fx_driver_init_process_mbr(p_ctrl, p_block_media);

        /** If actual count is equal to 3 then read from EBR table and update the partition table */
        if ((SSP_SUCCESS == ret_val) && (3U == p_ctrl->media_info.partition.actual_count))
        {
            /* Partition offset for the 4th partition updated by reading MBR table entry will have base address of
             * EBR table entry. A sector is added in order to update the partition table with the exact offset of
             * the partition. */
            p_ctrl->media_info.boot_record_table.ebr.base_addr = p_ctrl->media_info.partition.p_data[3].offset;
            p_ctrl->media_info.partition.p_data[3].offset = 0U;
            p_ctrl->media_info.partition.p_data[3].size   = 0U;

            /** Update partition table based on EBR table */
            ret_val = sf_el_fx_driver_init_process_ebr(p_ctrl, p_block_media);
        }

        /* If total count or actual count of partition is more than 1, update the global variable */
        if ((SSP_SUCCESS == ret_val)                        ||
            (1U < p_ctrl->media_info.partition.total_count) ||
            (1U < p_ctrl->media_info.partition.actual_count))
        {
            p_ctrl->media_info.partition.multiple_partitions_status = SF_EL_FX_MULTIPLE_PARTITIONS;
        }
    }
    else
    {
        /* Update global variable with no partitions */
        p_ctrl->media_info.partition.multiple_partitions_status = SF_EL_FX_NO_PARTITIONS;
    }
}


/*******************************************************************************************************************//**
 * @brief   Initialize the system by reading MBR/EBR table entries from the memory
 *
 * @param[in] p_ctrl                 Pointer to SF_EL_FX control block
 * @param[in] p_block_media          Pointer to lower level block media control block
 *
 * @retval  none
 *
 *                                   This function calls
 *                                        * sf_el_fx_driver_init_process_mbr_ebr
***********************************************************************************************************************/
static void sf_el_fx_driver_init_system_full(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                             sf_block_media_instance_t * const p_block_media)
{
    /** After power_on/system_reset, process MBR/EBR if exist */
    sf_el_fx_driver_init_process_mbr_ebr(p_ctrl, p_block_media);

    /* Update the system status as initialize full */
    p_ctrl->media_info.init_status = SF_EL_FX_SYS_INIT_FULL;
}

/*******************************************************************************************************************//**
 * @brief   Initialize the system by assigning default values to global variables
 *
 * @param[in] p_ctrl                 Pointer to SF_EL_FX control block
 *
 * @retval  none
 *
 *                                   This function calls
 *                                        * sf_el_fx_driver_init_media_info
***********************************************************************************************************************/
static void sf_el_fx_driver_init_system_partial(sf_el_fx_instance_ctrl_t * const p_ctrl)
{
    /** Initialize the global variables associated with partition */
    sf_el_fx_driver_init_media_info(p_ctrl);

    /* Update the system status as initialize partially */
    p_ctrl->media_info.init_status = SF_EL_FX_SYS_INIT_PARTIAL;
}

/*******************************************************************************************************************//**
 * @brief   Find the index of the partition from the partition table
 *
 * @param[in]  p_ctrl            Pointer to SF_EL_FX control block
 * @param[in]  sector_offset     Partition offset
 * @param[out] p_partition_index Pointer to partition index in the table
 *
 * @retval  SSP_SUCCESS                 If partition index exist in the table.
 * @retval  SSP_ERR_ASSERTION           If partition index not found in the table.
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_partition_entry_index(sf_el_fx_instance_ctrl_t * const p_ctrl,
                                                       uint32_t                   sector_offset,
                                                       uint32_t                 * const p_partition_index)
{
    uint32_t i = 0U;

    for (i = 0U; i < p_ctrl->media_info.partition.actual_count; i++)
    {
        /** Check if partition offset matches with the entry in the table */
        if (sector_offset == p_ctrl->media_info.partition.p_data[i].offset)
        {
            /* Return the partition index value through pointer variable */
            *p_partition_index = i;

            /* Return success */
            return SSP_SUCCESS;
        }
    }

    /* Return failure */
    return SSP_ERR_ASSERTION;
}

/*******************************************************************************************************************//**
 * @brief   Make partition status as open
 *
 * @param[in]  p_ctrl            Pointer to SF_EL_FX control block
 * @param[in]  sector_offset     Partition offset
 *
 * @retval  SSP_SUCCESS                 If partition exist in the table.
 * @retval  SSP_ERR_ASSERTION           If partition non exist in the table.
 *
 * @return                            See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                    codes. This function calls
 *                                        * sf_el_fx_driver_partition_entry_index
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_update_partition_open_status(sf_el_fx_instance_ctrl_t * const p_ctrl,
                                                              uint32_t                   const sector_offset)
{
    ssp_err_t ret_val = SSP_SUCCESS;
    uint32_t partition_index = 0xFFFFFFFF;

    /** Find the index of the partition */
    ret_val = sf_el_fx_driver_partition_entry_index(p_ctrl, sector_offset, &partition_index);

    if (SSP_SUCCESS == ret_val)
    {
        /* Update the global variable partition status as open */
        p_ctrl->media_info.partition.p_data[partition_index].open_status = SF_EL_FX_PARTITION_OPEN;
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Make partition status as close
 *
 * @param[in]  p_ctrl            Pointer to SF_EL_FX control block
 * @param[in]  sector_offset     Partition offset
 *
 * @retval  SSP_SUCCESS                 If partition exist in the table.
 * @retval  SSP_ERR_ASSERTION           If partition non exist in the table.
 *
 * @return                            See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                    codes. This function calls
 *                                        * sf_el_fx_driver_partition_entry_index
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_update_partition_close_status(sf_el_fx_instance_ctrl_t * const p_ctrl,
                                                               uint32_t                   const sector_offset)
{
    ssp_err_t ret_val = SSP_SUCCESS;
    uint32_t partition_index = 0xFFFFFFFF;

    /* Find the index of the partition */
    ret_val = sf_el_fx_driver_partition_entry_index(p_ctrl, sector_offset, &partition_index);

    if (SSP_SUCCESS == ret_val)
    {
        /* Update the global variable partition status as close */
        p_ctrl->media_info.partition.p_data[partition_index].open_status = SF_EL_FX_PARTITION_CLOSE;
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Check for partition open status
 *
 * @param[in]  p_ctrl            Pointer to SF_EL_FX control block
 * @param[in]  sector_offset     Partition offset
 *
 * @retval  SSP_SUCCESS                 If partition exist and is in open state.
 * @retval  SSP_ERR_ASSERTION           If partition non exist in the table.
 *
 * @return                            See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                    codes. This function calls
 *                                        * sf_el_fx_driver_partition_entry_index
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_find_partition_entry(sf_el_fx_instance_ctrl_t * const p_ctrl,
                                                      uint32_t                   const sector_offset)
{
    ssp_err_t ret_val = SSP_SUCCESS;
    uint32_t partition_index = 0xFFFFFFFF;

    /** Find the index of the partition */
    ret_val = sf_el_fx_driver_partition_entry_index(p_ctrl, sector_offset, &partition_index);

    if (SSP_SUCCESS == ret_val)
    {
        /** Check if partition state is open or not */
       if (SF_EL_FX_PARTITION_OPEN != p_ctrl->media_info.partition.p_data[partition_index].open_status)
       {
           ret_val = SSP_ERR_ASSERTION;
       }
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Validate new partition
 *
 * @param[in]  p_ctrl            Pointer to SF_EL_FX control block
 * @param[in]  sector_offset     Partition start sector
 * @param[in]  sector_end        Partition end sector
 *
 * @retval  SSP_SUCCESS                 If partition is valid.
 * @retval  SSP_ERR_ASSERTION           If partition is invalid.
 * @retval  SSP_ERR_INVALID_ARGUMENT    An invalid argument was passed to the driver.
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_validate_new_partition_status(sf_el_fx_instance_ctrl_t * const p_ctrl,
                                                               uint32_t                   const sector_offset,
                                                               uint32_t                   const sector_end)
{
    ssp_err_t ret_val = SSP_SUCCESS;
    uint32_t i = 0U;

    if ((sector_offset >  p_ctrl->media_info.memory_total_sectors) ||
        (sector_end    >= p_ctrl->media_info.memory_total_sectors))
    {
        return SSP_ERR_INVALID_ARGUMENT;
    }

    /** Check if partition actual count is greater than zero and MBR/EBR table is valid */
    if ((0U < p_ctrl->media_info.partition.actual_count) &&
          ((SF_EL_FX_EXIST_AND_VALID == p_ctrl->media_info.boot_record_table.mbr.status) ||
           (SF_EL_FX_EXIST_AND_VALID == p_ctrl->media_info.boot_record_table.ebr.status)))
    {
        for (i = 0U; i < p_ctrl->media_info.partition.actual_count; i++)
        {
            if ((sector_offset >= p_ctrl->media_info.partition.p_data[i].offset) &&
                (sector_offset <  (p_ctrl->media_info.partition.p_data[i].offset +
                                   p_ctrl->media_info.partition.p_data[i].size)))
            {
                return SSP_ERR_INVALID_ARGUMENT;
            }

            if (sector_offset < p_ctrl->media_info.partition.p_data[i].offset)
            {
                if (0U == i)
                {
                    return SSP_ERR_INVALID_ARGUMENT;
                }
                else
                {
                    if (sector_end >= p_ctrl->media_info.partition.p_data[i].offset)
                    {
                        return SSP_ERR_INVALID_ARGUMENT;
                    }
                }
            }
        }
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Check for any previous partition exist in the memory and validate the new partition
 *
 * @param[in]  p_ctrl            Pointer to SF_EL_FX control block
 * @param[in]  sector_offset     Partition start sector
 * @param[in]  sector_end        Partition end sector
 *
 * @retval  SSP_SUCCESS                 If partition is valid.
 * @retval  SSP_ERR_ASSERTION           If partition is invalid.
 * @retval  SSP_ERR_INVALID_ARGUMENT    An invalid argument was passed to the driver.
 *
 * @return                            See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                    codes. This function calls
 *                                        * sf_el_fx_driver_sector_size_get
 *                                        * sf_el_fx_driver_partition_entry_index
 *                                        * sf_el_fx_driver_validate_new_partition_status
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_validate_new_partition(sf_el_fx_instance_ctrl_t       * const p_ctrl,
                                                        uint8_t                  const * const p_buff,
                                                        uint32_t                         sector_offset)
{
    ssp_err_t ret_val = SSP_SUCCESS;
    uint32_t sector_size = 0U;
    uint32_t sector_end = 0U;
    uint32_t partition_index = 0xFFFFFFFF;

    ret_val = sf_el_fx_driver_sector_size_get(p_ctrl, p_buff, &sector_size, NULL);
    if(SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }

    /* Find the index of the partition */
    if (SSP_SUCCESS == sf_el_fx_driver_partition_entry_index(p_ctrl, sector_offset, &partition_index))
    {
        if (sector_size == p_ctrl->media_info.partition.p_data[partition_index].size)
        {
            /* Return success */
            return SSP_SUCCESS;
        }

        /* Return error */
        return SSP_ERR_INVALID_ARGUMENT;
    }

    /* Calculate sector end after adding sector offset and its size */
    sector_end = sector_offset + (sector_size - 1U);

    /** Check if actual count of partition is greater than or equal to total count of partition or not */
    if ((p_ctrl->media_info.partition.actual_count) >= (p_ctrl->media_info.partition.total_count))
    {
        /* Return error */
        return SSP_ERR_INVALID_ARGUMENT;
    }

    /* Check if actual count of partition is greater than zero or not */
    if (0U < p_ctrl->media_info.partition.actual_count)
    {
        if (4U > p_ctrl->media_info.partition.actual_count)
        {
            if (SF_EL_FX_EXIST_AND_VALID != p_ctrl->media_info.boot_record_table.mbr.status)
            {
                /* Return error */
                return SSP_ERR_INVALID_ARGUMENT;
            }
        }
        else
        {
            if ((SF_EL_FX_EXIST_AND_VALID != p_ctrl->media_info.boot_record_table.mbr.status) ||
                (SF_EL_FX_EXIST_AND_VALID != p_ctrl->media_info.boot_record_table.ebr.status))
            {
                /* Return error */
                return SSP_ERR_INVALID_ARGUMENT;
            }

            /* Sector offset represents the LBA(starting address) of any partition. If the total number of  */
            /* partitions existing is equal or more than 4, sector before the partition offset is used for  */
            /* EBR table. Decrement 1 sector from sector_offset, will consider the EBR sector for validity. */
            sector_offset--;
        }

        /** Validate new partition based on the start and end address of the partition */
        ret_val = sf_el_fx_driver_validate_new_partition_status(p_ctrl, sector_offset, sector_end);
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Compare write and read buffers
 *
 * @param[in]  p_buff_write    Pointer to write buffer
 * @param[in]  p_buff_read     Pointer to read buffer
 * @param[in]  buff_size       size of the buffer
 *
 * @retval  SSP_SUCCESS                 If write and read buffer data are same
 * @retval  SSP_ERR_ASSERTION           If write and read buffer data are different
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_new_partition_read_write_buff_comp(uint8_t  const * const p_buff_write,
                                                                    uint8_t  const * const p_buff_read,
                                                                    uint32_t         const buff_size)
{
    ssp_err_t ret_val = SSP_SUCCESS;

    uint32_t i = 0U;

    for (i = 0U; i < buff_size; i++)
    {
        /* Check if write and read info is same or not */
        if (p_buff_write[i] != p_buff_read[i])
        {
            /* Return error */
            ret_val = SSP_ERR_ASSERTION;
            break;
        }
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Copy data from source to destination buffer
 *
 * @param[in]  p_buff_dest      Pointer to destination buffer
 * @param[in]  p_buff_source    Pointer to source buffer
 * @param[in]  buff_size        size of the buffer
 *
 * @retval  none
***********************************************************************************************************************/
static void sf_el_fx_driver_new_partition_temp_to_main_buff_cpy(uint8_t       * const p_buff_dest,
                                                                uint8_t const * const p_buff_source,
                                                                uint32_t        const buff_size)
{
    uint32_t i = 0U;

    for (i = 0U; i < buff_size; i++)
    {
        p_buff_dest[i] = p_buff_source[i];
    }
}

/*******************************************************************************************************************//**
 * @brief   Write signature data to MBR/EBR buffer
 *
 * @param[in]  p_buff            Pointer to buffer
 *
 * @retval  none
***********************************************************************************************************************/
static void sf_el_fx_driver_write_buff_sig(uint8_t * const p_buff)
{
    /* Reset MBR/EBR buffer before filling and saving to the memory */
    memset(p_buff, 0x00, SF_EL_FX_MBR_EBR_SECTOR_SIZE);

    /** Fill 510 index of MBR/EBR buffer with data 0x55 */
    p_buff[SF_EL_FX_MBR_SIGNATURE_OFFSET] = (uint8_t)(SF_EL_FX_MBR_SIGNATURE_VALUE & SF_EL_FX_MBR_SIGNATURE_VALUE_BYTE_LSB_MASK);

    /** Fill 511 index of MBR/EBR buffer with data 0xAA */
    p_buff[SF_EL_FX_MBR_SIGNATURE_OFFSET + 1U] = (uint8_t)((SF_EL_FX_MBR_SIGNATURE_VALUE & SF_EL_FX_MBR_SIGNATURE_VALUE_BYTE_MSB_MASK) >> 8U);
}

/*******************************************************************************************************************//**
 * @brief   Check for any previous partition exist in the memory and validate the new partition
 *
 * @param[in]  p_buff                   Pointer to buffer
 * @param[in]  buff_partition_offset    Buffer offset as per the partition
 * @param[in]  sector_offset            Partition start sector
 * @param[in]  sector_size              Partition size
 * @param[in]  partition_type           Type of partition
 *                                          SF_EL_FX_PARTITION_TYPE_UNKNOWN
 *                                          SF_EL_FX_PARTITION_TYPE_FAT_16_MEMORY_LESS_THAN_32MB
 *                                          SF_EL_FX_PARTITION_TYPE_EXTENDED
 *                                          SF_EL_FX_PARTITION_TYPE_FAT_16_MEMORY_MORE_THAN_32MB
 *                                          SF_EL_FX_PARTITION_TYPE_FAT_32
 *
 * @retval  none
                                         This function calls
 *                                        * _fx_utility_32_unsigned_write
***********************************************************************************************************************/
static void sf_el_fx_driver_fill_partition_buff(uint8_t                         * const p_buff,
                                                uint32_t                          const buff_partition_offset,
                                                uint32_t                          const sector_offset,
                                                uint32_t                          const sector_size,
                                                sf_el_fx_media_partition_type_t   const partition_type)
{
    /** Update buffer boot index */
    p_buff[buff_partition_offset + SF_EL_FX_MBR_PRIMARY_PARTITION_BOOT_FLAG_OFFSET] = (uint8_t)0x00;

    /** Update buffer partition type */
    p_buff[buff_partition_offset + SF_EL_FX_MBR_PRIMARY_PARTITION_TYPE_OFFSET]      = partition_type;

    /** Update buffer with partition starting sector address */
    _fx_utility_32_unsigned_write(&p_buff[buff_partition_offset + SF_EL_FX_MBR_PRIMARY_PARTITION_LBA_OF_FIRST_SECTOR_OFFSET],
                                  sector_offset);

    /** Update buffer with partition size */
    _fx_utility_32_unsigned_write(&p_buff[buff_partition_offset + SF_EL_FX_MBR_PRIMARY_PARTITION_SIZE_IN_SECTOR_OFFSET],
                                  sector_size);
}

/*******************************************************************************************************************//**
 * @brief   Update partition table with partition starting sector, size and format status
 *
 * @param[in]  p_ctrl               Pointer to SF_EL_FX control block
 * @param[in]  partition_counter    Partition table index
 * @param[in]  sector_offset        Partition start sector
 * @param[in]  sector_size          Partition size
 *
 * @retval  none
***********************************************************************************************************************/
static void sf_el_fx_driver_new_partition_detail_update(sf_el_fx_instance_ctrl_t * const p_ctrl,
                                                        uint32_t                   const partition_counter,
                                                        uint32_t                   const sector_offset,
                                                        uint32_t                   const sector_size)
{
    /** Update partition table with partition starting sector address */
    p_ctrl->media_info.partition.p_data[partition_counter].offset = sector_offset;

    /** Update partition table with the size of the partition */
    p_ctrl->media_info.partition.p_data[partition_counter].size = sector_size;

    /** Update partition table with format status of the partition */
    p_ctrl->media_info.partition.p_data[partition_counter].format_status = SF_EL_FX_PARTITION_POST_RESET_FORMATED;
}

/*******************************************************************************************************************//**
 * @brief   Update MBR/EBR buffer and write it to the memory
 *
 * @param[in]  p_block_media            Pointer to lower level block media control block
 * @param[in]  p_buff                   Pointer to the buffer
 * @param[in]  buff_partition_offset    Buffer offset as per the partition
 * @param[in]  sector_offset            Partition start sector
 * @param[in]  sector_size              Partition size
 * @param[in]  mbr_ebr_rw_sector_offset MBR/EBR sector address
 * @param[in]  partition_type           Type of partition
 *                                          SF_EL_FX_PARTITION_TYPE_UNKNOWN
 *                                          SF_EL_FX_PARTITION_TYPE_FAT_16_MEMORY_LESS_THAN_32MB
 *                                          SF_EL_FX_PARTITION_TYPE_EXTENDED
 *                                          SF_EL_FX_PARTITION_TYPE_FAT_16_MEMORY_MORE_THAN_32MB
 *                                          SF_EL_FX_PARTITION_TYPE_FAT_32
 *
 * @retval  SSP_SUCCESS                 If write to/read from memory is successful.
 * @retval  SSP_SUCCESS                 If write to and read from memory data are same.
 * @retval  SSP_ERR_ASSERTION           If write to/read from memory is unsuccessful.
 *
 * @return                            See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                    codes. This function calls
 *                                        * sf_block_media_api_t::write
 *                                        * sf_block_media_api_t::read
 *                                        * sf_el_fx_driver_fill_partition_buff
 *                                        * sf_el_fx_driver_new_partition_read_write_buff_comp
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_update_buff(sf_block_media_instance_t       * const p_block_media,
                                             uint8_t                         * const p_buff,
                                             uint32_t                          const buff_partition_offset,
                                             uint32_t                          const sector_offset,
                                             uint32_t                          const sector_size,
                                             uint32_t                          const mbr_ebr_rw_sector_offset,
                                             sf_el_fx_media_partition_type_t   const partition_type)
{
    ssp_err_t ret_val = SSP_SUCCESS;

    uint8_t temp_buff[SF_EL_FX_MBR_EBR_SECTOR_SIZE];

    /** Fill MBR/EBR buffer with partition details */
    sf_el_fx_driver_fill_partition_buff((uint8_t *)p_buff,
                                        buff_partition_offset,
                                        sector_offset,
                                        sector_size,
                                        partition_type);

    /** Write the updated MBR/EBR buffer to the memory at a specified sector */
    ret_val = p_block_media->p_api->write(p_block_media->p_ctrl,
                                          p_buff,
                                          mbr_ebr_rw_sector_offset,
                                          1);
    if(SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }

    /* Reset temp buffer before reading data from memory */
    memset(temp_buff, 0x00, SF_EL_FX_MBR_EBR_SECTOR_SIZE);

    /** Read MBR/EBR sector as per the specified start sector data. For MBR start sector is always passed as zero */
    ret_val = p_block_media->p_api->read(p_block_media->p_ctrl,
                                         temp_buff,
                                         mbr_ebr_rw_sector_offset,
                                         1);
    if(SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }

    /** Compare write and read buffer */
    ret_val = sf_el_fx_driver_new_partition_read_write_buff_comp((uint8_t *)p_buff,
                                                                 (uint8_t *)temp_buff,
                                                                 SF_EL_FX_MBR_EBR_SECTOR_SIZE);

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Update EBR buffer and write it to the memory
 *
 * @param[in]  p_ctrl                   Pointer to SF_EL_FX control block
 * @param[in]  p_block_media            Pointer to lower level block media control block
 * @param[in]  sector_offset            Partition start sector
 * @param[in]  sector_size              Partition size
 * @param[in]  partition_type           Type of partition
 *                                          SF_EL_FX_PARTITION_TYPE_UNKNOWN
 *                                          SF_EL_FX_PARTITION_TYPE_FAT_16_MEMORY_LESS_THAN_32MB
 *                                          SF_EL_FX_PARTITION_TYPE_EXTENDED
 *                                          SF_EL_FX_PARTITION_TYPE_FAT_16_MEMORY_MORE_THAN_32MB
 *                                          SF_EL_FX_PARTITION_TYPE_FAT_32
 * @param[in] buff_update_state         EBR buffer update state
 *                                          SF_EL_FX_EBR_BUFF_UPDATE_PRESENT
                                            SF_EL_FX_EBR_BUFF_UPDATE_NEXT
 *
 * @retval  SSP_SUCCESS                 If write to/read from memory is successful.
 * @retval  SSP_ERR_ASSERTION           If write to/read from memory is unsuccessful.
 *
 * @return                            See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                    codes. This function calls
 *                                        * sf_el_fx_driver_new_partition_temp_to_main_buff_cpy
 *                                        * sf_el_fx_driver_update_buff
 *                                        * sf_el_fx_driver_write_buff_sig
 *                                        * sf_el_fx_driver_new_partition_detail_update
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_update_ebr_buff(sf_el_fx_instance_ctrl_t                   * const p_ctrl,
                                                 sf_block_media_instance_t                  * const p_block_media,
                                                 uint32_t                                     const sector_offset,
                                                 uint32_t                                     const sector_size,
                                                 sf_el_fx_media_partition_type_t              const partition_type,
                                                 sf_el_fx_media_partition_ebr_buff_update_t   const buff_update_state)
{
    ssp_err_t ret_val = SSP_SUCCESS;

    uint32_t const buff_partition_offset_arr[] = {SF_EL_FX_MBR_PRIMARY_PARTITION_1_OFFSET,
                                                  SF_EL_FX_MBR_PRIMARY_PARTITION_2_OFFSET};

    uint32_t sector_offset_with_ebr_ref = 0;
    uint8_t temp_buff[SF_EL_FX_EBR_SECTOR_SIZE];

    /** Check if any partition exist  */
    if (3U == p_ctrl->media_info.partition.actual_count)
    {
        /* Update status of EBR indicating MBR exist and is valid */
        p_ctrl->media_info.boot_record_table.ebr.status = SF_EL_FX_EXIST_AND_VALID;
    }

    /* Copy EBR to temporary buffer */
    sf_el_fx_driver_new_partition_temp_to_main_buff_cpy((uint8_t *)temp_buff,
                                                        (uint8_t *)p_ctrl->media_info.boot_record_table.ebr.buff,
                                                        SF_EL_FX_EBR_SECTOR_SIZE);

    if (SF_EL_FX_EBR_BUFF_UPDATE_NEXT == buff_update_state)
    {
        sector_offset_with_ebr_ref = sector_offset - p_ctrl->media_info.boot_record_table.ebr.base_addr;

        /* Update and Write EBR buffer to memory for SF_EL_FX_MBR_PRIMARY_PARTITION_2_OFFSET */
        ret_val = sf_el_fx_driver_update_buff(p_block_media,
                                              temp_buff,
                                              buff_partition_offset_arr[1],
                                              sector_offset_with_ebr_ref - 1,
                                              sector_size,
                                              p_ctrl->media_info.partition.p_data[p_ctrl->media_info.partition.actual_count - 1].offset - 1,
                                              SF_EL_FX_PARTITION_TYPE_EXTENDED);
        if(SSP_SUCCESS != ret_val)
        {
           return ret_val;
        }

        /* Reset EBR buffer and adding signature to it */
        sf_el_fx_driver_write_buff_sig((uint8_t *)p_ctrl->media_info.boot_record_table.ebr.buff);
    }

    /* Initialize temporary buffer for first time with signature */
    sf_el_fx_driver_write_buff_sig((uint8_t *)temp_buff);

    /* Update and Write EBR buffer to memory for SF_EL_FX_MBR_PRIMARY_PARTITION_1_OFFSET */
    ret_val = sf_el_fx_driver_update_buff(p_block_media,
                                          temp_buff,
                                          buff_partition_offset_arr[0],
                                          1U,
                                          sector_size,
                                          sector_offset - 1,
                                          partition_type);
    if(SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }

    /* Copy temporary buffer to main MBR buffer */
    sf_el_fx_driver_new_partition_temp_to_main_buff_cpy((uint8_t *)p_ctrl->media_info.boot_record_table.ebr.buff,
                                                        (uint8_t *)temp_buff,
                                                        SF_EL_FX_EBR_SECTOR_SIZE);

    /** Update partition table with new partition information */
    sf_el_fx_driver_new_partition_detail_update(p_ctrl,
                                                (uint32_t)p_ctrl->media_info.partition.actual_count,
                                                sector_offset,
                                                sector_size);

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Update MBR buffer and write it to the memory
 *
 * @param[in]  p_ctrl                   Pointer to SF_EL_FX control block
 * @param[in]  p_block_media            Pointer to lower level block media control block
 * @param[in]  sector_offset            Partition start sector
 * @param[in]  sector_size              Partition size
 * @param[in]  partition_type           Type of partition
 *                                          SF_EL_FX_PARTITION_TYPE_UNKNOWN
 *                                          SF_EL_FX_PARTITION_TYPE_FAT_16_MEMORY_LESS_THAN_32MB
 *                                          SF_EL_FX_PARTITION_TYPE_EXTENDED
 *                                          SF_EL_FX_PARTITION_TYPE_FAT_16_MEMORY_MORE_THAN_32MB
 *                                          SF_EL_FX_PARTITION_TYPE_FAT_32
 *
 * @retval  SSP_SUCCESS                 If write to/read from memory is successful.
 * @retval  SSP_ERR_ASSERTION           If write to/read from memory is unsuccessful.
 *
 * @return                            See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                    codes. This function calls
 *                                        * sf_el_fx_driver_new_partition_temp_to_main_buff_cpy
 *                                        * sf_el_fx_driver_write_buff_sig
 *                                        * sf_el_fx_driver_update_buff
 *                                        * sf_el_fx_driver_new_partition_detail_update
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_update_mbr_buff(sf_el_fx_instance_ctrl_t        * const p_ctrl,
                                                 sf_block_media_instance_t       * const p_block_media,
                                                 uint32_t                          const sector_offset,
                                                 uint32_t                          const sector_size,
                                                 sf_el_fx_media_partition_type_t   const partition_type)
{
    ssp_err_t ret_val = SSP_SUCCESS;

    uint32_t const buff_partition_offset_arr[] = {SF_EL_FX_MBR_PRIMARY_PARTITION_1_OFFSET,
                                                  SF_EL_FX_MBR_PRIMARY_PARTITION_2_OFFSET,
                                                  SF_EL_FX_MBR_PRIMARY_PARTITION_3_OFFSET,
                                                  SF_EL_FX_MBR_PRIMARY_PARTITION_4_OFFSET};

    uint8_t temp_buff[SF_EL_FX_MBR_SECTOR_SIZE];

    /* Copy MBR to temporary buffer */
    sf_el_fx_driver_new_partition_temp_to_main_buff_cpy((uint8_t *)temp_buff,
                                                        (uint8_t *)p_ctrl->media_info.boot_record_table.mbr.buff,
                                                        SF_EL_FX_MBR_SECTOR_SIZE);

    /** Check if any partition exist  */
    if (!p_ctrl->media_info.partition.actual_count)
    {
        /* Initialize temporary buffer for first time with signature */
        sf_el_fx_driver_write_buff_sig((uint8_t *)temp_buff);

        /* Update status of MBR indicating MBR exist and is valid */
        p_ctrl->media_info.boot_record_table.mbr.status = SF_EL_FX_EXIST_AND_VALID;
    }

    /** Update and Write MBR buffer to memory */
    ret_val = sf_el_fx_driver_update_buff(p_block_media,
                                          temp_buff,
                                          buff_partition_offset_arr[p_ctrl->media_info.partition.actual_count],
                                          sector_offset,
                                          sector_size,
                                          0,
                                          partition_type);
    if(SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }

    /* Copy temporary buffer to main MBR buffer */
    sf_el_fx_driver_new_partition_temp_to_main_buff_cpy((uint8_t *)p_ctrl->media_info.boot_record_table.mbr.buff,
                                                        (uint8_t *)temp_buff,
                                                        SF_EL_FX_MBR_SECTOR_SIZE);

    if (3U > p_ctrl->media_info.partition.actual_count)
    {
        /** Update partition table with new partition information */
        sf_el_fx_driver_new_partition_detail_update(p_ctrl,
                                                    (uint32_t)p_ctrl->media_info.partition.actual_count,
                                                    sector_offset,
                                                    sector_size);
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Find the partition type
 *
 * @param[in]  p_ctrl                   Pointer to SF_EL_FX control block
 * @param[in]  p_buff                   Pointer to buffer
 * @param[in]  sector_size              Partition size
 * @param[in]  p_partition_type         Pointer to partition type
 *
 * @retval  SSP_SUCCESS                 If partition size is non zero.
 * @retval  SSP_ERR_ASSERTION           If partition size is zero.
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_sector_size_get(sf_el_fx_instance_ctrl_t              * const p_ctrl,
                                                 uint8_t                         const * const p_buff,
                                                 uint32_t                              * const p_sector_size,
                                                 sf_el_fx_media_partition_type_t       * const p_partition_type)
{
    ssp_err_t ret_val = SSP_SUCCESS;

    /** Read the partition size corresponding to the file system type */
    uint32_t sector_size_16 = (uint32_t)_fx_utility_16_unsigned_read((uint8_t *)&p_buff[SF_EL_FX_PARTITION_SIZE_BUFFER_INDEX_FAT_16]);
    uint32_t sector_size_32 = (uint32_t)_fx_utility_32_unsigned_read((uint8_t *)&p_buff[SF_EL_FX_PARTITION_SIZE_BUFFER_INDEX_FAT_32]);
    uint64_t sector_size_exfat = (uint64_t)_fx_utility_64_unsigned_read((uint8_t *)&p_buff[SF_EL_FX_PARTITION_SIZE_BUFFER_INDEX_EXFAT]);
    sf_el_fx_media_partition_type_t partition_type = SF_EL_FX_PARTITION_TYPE_UNKNOWN;

    /** Check if partition size is zero then return with error */
    if ((!sector_size_16) && (!sector_size_32) && (!sector_size_exfat))
    {
        return SSP_ERR_ASSERTION;
    }

    /** Update partition type as per the partition size and total size of memory available */
    if ((sector_size_16) && (!sector_size_32))
    {
        *p_sector_size  = (uint32_t)sector_size_16;
         partition_type = (sf_el_fx_media_partition_type_t)SF_EL_FX_PARTITION_TYPE_FAT_16_MEMORY_LESS_THAN_32MB;
    }
    else if ((!sector_size_16) && (sector_size_32))
    {
        *p_sector_size = (uint32_t)sector_size_32;

        /* Check if total available memory is less than 2GB or not */
        if  (SF_EL_FX_FAT16_MEMORY_SIZE_MAX >= p_ctrl->media_info.memory_total_sectors)
        {
            partition_type = (sf_el_fx_media_partition_type_t)SF_EL_FX_PARTITION_TYPE_FAT_16_MEMORY_MORE_THAN_32MB;
        }
        else
        {
            partition_type = (sf_el_fx_media_partition_type_t)SF_EL_FX_PARTITION_TYPE_FAT_32;
        }
    }
    else if ((!sector_size_16) && (!sector_size_32) && (sector_size_exfat))
    {
    	/** If its a exFAT partition, update its corresponding partition size and partition type */
        *p_sector_size  = (uint32_t)sector_size_exfat;
         partition_type = (sf_el_fx_media_partition_type_t)SF_EL_FX_PARTITION_TYPE_EXFAT;
    }

    if (NULL != p_partition_type)
    {
        *p_partition_type = (sf_el_fx_media_partition_type_t)partition_type;
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Based on partition counter update MBR/EBR buffer
 *
 * @param[in]  p_ctrl                   Pointer to SF_EL_FX control block
 * @param[in]  p_block_media            Pointer to lower level block media control block
 * @param[in]  p_buff                   Pointer to buffer
 * @param[in]  sector_size              Partition size
 *
 * @retval  SSP_SUCCESS                 If buffer update is successful.
 * @retval  SSP_ERR_ASSERTION           If buffer update is unsuccessful.
 *
 * @return                            See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                    codes. This function calls
 *                                        * sf_el_fx_driver_sector_size_get
 *                                        * sf_el_fx_driver_partition_entry_index
 *                                        * sf_el_fx_driver_update_mbr_buff
 *                                        * sf_el_fx_driver_update_ebr_buff
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_update_mbr_ebr(sf_el_fx_instance_ctrl_t        * const p_ctrl,
                                                sf_block_media_instance_t       * const p_block_media,
                                                uint8_t                   const * const p_buff,
                                                uint32_t                  const         sector_offset)
{
    ssp_err_t ret_val = SSP_SUCCESS;
    uint32_t sector_size = 0U;
    uint32_t partition_index = 0xFFFFFFFF;
    sf_el_fx_media_partition_type_t partition_type = SF_EL_FX_PARTITION_TYPE_UNKNOWN;

    ret_val = sf_el_fx_driver_sector_size_get(p_ctrl, p_buff, &sector_size, &partition_type);
    if(SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }

    /* Find the index of the partition */
    if (SSP_SUCCESS == sf_el_fx_driver_partition_entry_index(p_ctrl, sector_offset, &partition_index))
    {
        if (sector_size == p_ctrl->media_info.partition.p_data[partition_index].size)
        {
            /* Return success */
            return SSP_SUCCESS;
        }

        /* Return error */
        return SSP_ERR_INVALID_ARGUMENT;
    }

    /** Check if the existing total number of partitions is less than 4 or not */
    if (3U > p_ctrl->media_info.partition.actual_count)
    {
        ret_val = sf_el_fx_driver_update_mbr_buff(p_ctrl,
                                                  p_block_media,
                                                  sector_offset,
                                                  sector_size,
                                                  partition_type);
    }
    else if (3U == p_ctrl->media_info.partition.actual_count)
    {

        ret_val = sf_el_fx_driver_update_mbr_buff(p_ctrl,
                                                  p_block_media,
                                                  sector_offset - 1,
                                                  p_ctrl->media_info.memory_free_sectors,
                                                  SF_EL_FX_PARTITION_TYPE_EXTENDED_INT13);

        /* Check if the return value is success */
        if (SSP_SUCCESS == ret_val)
        {
            p_ctrl->media_info.boot_record_table.ebr.base_addr = sector_offset - 1;

            ret_val = sf_el_fx_driver_update_ebr_buff(p_ctrl,
                                                      p_block_media,
                                                      sector_offset,
                                                      sector_size,
                                                      partition_type,
                                                      SF_EL_FX_EBR_BUFF_UPDATE_PRESENT);
        }
    }
    else if (3U < p_ctrl->media_info.partition.actual_count)
    {
        ret_val = sf_el_fx_driver_update_ebr_buff(p_ctrl,
                                                  p_block_media,
                                                  sector_offset,
                                                  sector_size,
                                                  partition_type,
                                                  SF_EL_FX_EBR_BUFF_UPDATE_NEXT);
    }

    /* Check if the return value is success */
    if (SSP_SUCCESS == ret_val)
    {
        /** Increment the partition counter */
        p_ctrl->media_info.partition.actual_count++;

        /** Subtract the sector size from the available sectors of the memory */
        p_ctrl->media_info.memory_free_sectors -= sector_size;
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Handles the block driver initialization request
 *
 * @param[in]  p_block_media          Pointer to lower level block media control block
 * @param[out] write_protect          Pointer to write protect
 * @param[out] free_sector_update     Pointer to free sector update
 *
 * @retval  SSP_SUCCESS               If lower level block media calls are successful.
 * @retval  SSP_ERR_ASSERTION         If lower level block media calls are unsuccessful.
 *
 * @return                            See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                    codes. This function calls
 *                                        * sf_block_media_api_t::open
 *                                        * sf_block_media_api_t::ioctl
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_memory_info_get(sf_block_media_instance_t * const p_block_media,
                                                 uint32_t                  * write_protect,
                                                 uint32_t                  * free_sector_update)
{
    ssp_err_t ret_val = SSP_SUCCESS;
    uint32_t requires_sector_release = 0U;

    /** Open the block media. */
    ret_val = p_block_media->p_api->open(p_block_media->p_ctrl, p_block_media->p_cfg);
    if (SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }

    /** Get write protection status. */
    ret_val = p_block_media->p_api->ioctl(p_block_media->p_ctrl, SSP_COMMAND_GET_WRITE_PROTECTED, write_protect);
    if (SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }

    /** Get sector release require status and update FX_MEDIA::fx_media_driver_free_sector_update to FX_TRUE if sector
     *  release supported by the block media driver. */
    ret_val = p_block_media->p_api->ioctl(p_block_media->p_ctrl,
                                          SSP_COMMAND_GET_SECTOR_RELEASE,
                                          (void *) &requires_sector_release);
    if ((SSP_SUCCESS == ret_val) && (0U != requires_sector_release))
    {
        *free_sector_update = (uint32_t) FX_TRUE;
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Update SF_EL_FX control block with configuration data
 *
 * @param[in]  p_ctrl               Pointer to SF_EL_FX instance control block
 *
 * @retval  none
***********************************************************************************************************************/
static void sf_el_fx_driver_open(sf_el_fx_instance_ctrl_t * p_ctrl, sf_el_fx_config_t const * const p_config)
{
    p_ctrl->p_lower_lvl_block_media                         = p_config->p_lower_lvl_block_media;
    p_ctrl->p_context                                       = p_config->p_context;
    p_ctrl->p_callback                                      = p_config->p_callback;
    p_ctrl->media_info.partition.p_data                     = p_config->p_partition_data;
    p_ctrl->media_info.partition.total_count                = p_config->total_partitions;
    p_ctrl->open                                            = SF_EL_FX_OPEN;
    p_ctrl->media_info.boot_record_table.ebr.base_addr      = 0U;
    p_ctrl->media_info.init_status                          = SF_EL_FX_SYS_UNINIT;
    p_ctrl->media_info.partition.multiple_partitions_status = SF_EL_FX_NO_PARTITIONS;
}

/******************************************************************************************************************//**
 * @brief Checks if the sector passed is a valid boot record or master boot record. If not valid boot record
 * returns the offset to the partitions boot record.
 * @param[in]   p_sector                    Pointer to sector.
 * @param[out]  p_partition_offset          Sector number of found partition table. 0 if boot sector.
 * @retval      SSP_SUCCESS                 Found boot record or partition offset.
 * @retval      SSP_ERR_MEDIA_OPEN_FAILED   Not a valid boot record or partition table.
 *
 * @return                         See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                 codes. This function calls
 *                                        * check_fat_boot_record
 *                                        * _fx_utility_16_unsigned_read
 *                                        * _fx_utility_32_unsigned_read
 *********************************************************************************************************************/
static ssp_err_t check_partition_offset (uint8_t * p_sector, uint32_t * p_partition_offset)
{
    ssp_err_t ret;
    uint32_t offset;
    uint16_t check_AA55_1FE;
    UCHAR * p_temp = (UCHAR *) p_sector;

    /** Check for FAT boot record. */
    ret = check_fat_boot_record(p_sector, p_partition_offset);

    /** Check for master boot record. */
    if (SSP_SUCCESS != ret)
    {
        check_AA55_1FE = (uint16_t)_fx_utility_16_unsigned_read(&p_temp[SF_EL_FX_55AA_SIGNATURE_OFFSET]);


         /**  Read the offset to partition 0. */
        offset = (uint32_t)_fx_utility_32_unsigned_read(&p_temp[SF_EL_FX_PARTITION0_OFFSET]);

        /** Check for partition type is set and the offset is not zero and boot record signature for valid MBR then
          * report the partition offset. */
        if (((offset != 0U) && (p_sector[SF_EL_FX_PARTITION0_TYPE_OFFSET] != 0U)) &&
                           (SF_EL_FX_BOOT_SIGNATURE_VALUE == check_AA55_1FE))
        {
            *p_partition_offset = offset;
            return SSP_SUCCESS;
        }
    }

    return ret;
}

/******************************************************************************************************************//**
 * @brief Checks if the sector passed is a valid boot record. If not returns error.
 * @param[in]   p_sector                    Pointer to sector
 * @param[out]  p_partition_offset          Sector number of found partition table. 0 if boot sector.
 * @retval      SSP_SUCCESS                 Found boot record or partition offset
 * @retval      SSP_ERR_MEDIA_OPEN_FAILED   Not a valid boot record or partition table
 *********************************************************************************************************************/
static ssp_err_t check_fat_boot_record (uint8_t * p_sector, uint32_t * p_partition_offset)
{
    bool has_jump_instruction = false;

    uint32_t fat16_sectors_per_fat;
    uint32_t fat16_sector_count;
    uint32_t fat32_sectors_per_fat;
    uint32_t fat32_sector_count;

    uint32_t has_sectors_per_fat = 0U;
    uint32_t has_sector_count = 0U;

    uint16_t check_AA55_sector_end;
    uint16_t check_AA55_1FE;
    bool fs_is_exfat = false;
    UINT sector_size = 0U;

    UCHAR * p_temp = (UCHAR *) p_sector;

    /** Extract the number of bytes per sector for a FAT partition  */
    sector_size = _fx_utility_16_unsigned_read(&p_sector[FX_BYTES_SECTOR]);

#if (1U == SF_EL_FX_ENABLE_EXFAT)
    /** Check for a valid jump instruction for exFAT partition. */
    has_jump_instruction = (SF_EL_FX_JUMP_INST_SET2_BYTE0 == p_sector[SF_EL_FX_JUMP_INST_OFFSET]) &&
            ((SF_EL_FX_JUMP_INST_SET1_BYTE1 == p_sector[SF_EL_FX_JUMP_INST_OFFSET+1]) &&
            (SF_EL_FX_JUMP_INST_SET2_BYTE2 == p_sector[SF_EL_FX_JUMP_INST_OFFSET+2]));

    /** If byte per sector for fat is zero, and the jump instruction is valid, then it is a exFAT partition. */
    if ((true == has_jump_instruction) && (0U == sector_size))
    {
        /** Extract the byte per sector shift, and calculate byte per sector for the exFAT partition.  */
        sector_size = (uint16_t)(1U << p_sector[FX_EF_BYTE_PER_SECTOR_SHIFT]);

        fs_is_exfat = true;
    }
#endif

    /** If its a FAT partition, read its boot sector details */
    if(false == fs_is_exfat)
    {
        /** Check for a valid jump instruction. */
        has_jump_instruction = (SF_EL_FX_JUMP_INST_SET1_BYTE0 == p_sector[SF_EL_FX_JUMP_INST_OFFSET]) ||
                ((SF_EL_FX_JUMP_INST_SET2_BYTE0 == p_sector[SF_EL_FX_JUMP_INST_OFFSET]) &&
                (SF_EL_FX_JUMP_INST_SET2_BYTE2 == p_sector[SF_EL_FX_JUMP_INST_OFFSET+2]));

        /** Check for a non zero sectors per FAT. */
        fat16_sectors_per_fat = (uint32_t)_fx_utility_16_unsigned_read(&p_temp[SF_EL_FX_FAT16_SECTORS_PER_FAT]);
        fat32_sectors_per_fat = (uint32_t)_fx_utility_32_unsigned_read(&p_temp[SF_EL_FX_FAT32_SECTORS_PER_FAT]);
        has_sectors_per_fat = fat16_sectors_per_fat | fat32_sectors_per_fat;

        /** Check for a non zero sector count. */
        fat16_sector_count = (uint32_t)_fx_utility_16_unsigned_read(&p_temp[SF_EL_FX_FAT16_SECTOR_COUNT]);
        fat32_sector_count = (uint32_t)_fx_utility_32_unsigned_read(&p_temp[SF_EL_FX_FAT32_SECTOR_COUNT]);
        has_sector_count = fat16_sector_count | fat32_sector_count;
    }

    /** Check for jump instruction and sectors per fat and sector count are not 0
      * which indicate FAT Boot record or if its a valid exfat partition */
    if (fs_is_exfat || (has_jump_instruction && has_sector_count && has_sectors_per_fat))
    {
        /** Check the sector size value. */
        if (0U != (sector_size % 32U))
        {
            return SSP_ERR_MEDIA_OPEN_FAILED;
        }

        /** Check Boot record signature to make sure the buffer is valid then return
          * valid boot record. */
        check_AA55_sector_end = (uint16_t)_fx_utility_16_unsigned_read(&p_temp[sector_size-2]);
        check_AA55_1FE = (uint16_t)_fx_utility_16_unsigned_read(&p_temp[SF_EL_FX_55AA_SIGNATURE_OFFSET]);
        if ((SF_EL_FX_BOOT_SIGNATURE_VALUE == check_AA55_sector_end) ||
        		(SF_EL_FX_BOOT_SIGNATURE_VALUE == check_AA55_1FE))
        {
            * p_partition_offset = 0U;
            return SSP_SUCCESS;
        }
    }
    else
    {
        return SSP_ERR_MEDIA_OPEN_FAILED;
    }

    return SSP_SUCCESS;
}

/******************************************************************************************************************//**
 * @brief      Handles the block driver release sectors request.
 *
 * @param      p_block_media   The block media pointer
 * @param[in]  num_sectors     The number of sectors to release
 * @param[in]  start_sector    The start sector number
 *
 * @retval     SSP_SUCCESS         Driver release sectors request completed successfully
 * @return                         See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                 codes. This function calls
 *                                        * sf_block_media_api_t::ioctl
 *********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_release_sectors_handler(sf_block_media_instance_t * p_block_media,
                                                         uint32_t                    num_sectors,
                                                         uint32_t                    start_sector)
{
    ssp_err_t ret_val = SSP_SUCCESS;

    /** Release one sector at a time unless either requested number of sectors release or error detected while releasing
     * a sector. */
    while ((num_sectors) && (SSP_SUCCESS == ret_val))
    {
        ret_val = p_block_media->p_api->ioctl(p_block_media->p_ctrl,
                                              SSP_COMMAND_CTRL_SECTOR_RELEASE,
                                              (void *) (&start_sector));
        start_sector++;
        num_sectors--;
    }
    return ret_val;
}


/******************************************************************************************************************//**
 * @brief      Handles the block driver initialization request.
 *
 * @param      p_block_media       The block media pointer
 * @param      write_protect       The write protect value pointer
 * @param      free_sector_update  The free sector update value pointer
 *
 * @retval     SSP_SUCCESS         Driver initialization request completed successfully
 * @return                         See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                 codes. This function calls
 *                                        * sf_block_media_api_t::ioctl
 *                                        * sf_el_fx_driver_memory_info_get
 *********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_init_handler(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                              sf_block_media_instance_t * const p_block_media,
                                              uint32_t                  * p_write_protect,
                                              uint32_t                  * p_free_sector_update)
{
    ssp_err_t ret_val = SSP_SUCCESS;

    if (1U >= p_ctrl->media_info.partition.total_count)
    {
        ret_val = sf_el_fx_driver_memory_info_get(p_block_media, p_write_protect, p_free_sector_update);
        if (SSP_SUCCESS != ret_val)
        {
            return ret_val;
        }
    }
    else
    {
        /** Check if the driver is already in opened state */
        if (SF_EL_FX_PARTITION_GLOBAL_CLOSE == p_ctrl->media_info.global_open.status)
        {
            ret_val = sf_el_fx_driver_memory_info_get(p_block_media, p_write_protect, p_free_sector_update);
            if (SSP_SUCCESS != ret_val)
            {
                return ret_val;
            }

            /** Make driver status as opened */
            p_ctrl->media_info.global_open.status = SF_EL_FX_PARTITION_GLOBAL_OPEN;
        }

    }

    /** If the system is initialized partially, read the total sectors available in the memory */
    if (SF_EL_FX_SYS_INIT_PARTIAL == p_ctrl->media_info.init_status)
    {
        /** Get total memory sectors. */
        ret_val = p_block_media->p_api->ioctl(p_block_media->p_ctrl,
                                              SSP_COMMAND_GET_SECTOR_COUNT,
                                              &p_ctrl->media_info.memory_total_sectors);

        if (SSP_SUCCESS != ret_val)
        {
            return ret_val;
        }

        /** Initially free sectors are equal to total sectors available in the memory */
        p_ctrl->media_info.memory_free_sectors = p_ctrl->media_info.memory_total_sectors;
    }

    return ret_val;
}


/******************************************************************************************************************//**
 * @brief      Handles the block driver boot read request for multiple partitions.
 *
 * @param      p_block_media   The block media pointer
 * @param      p_buffer        The buffer pointer
 * @param      driver_status   The driver status
 * @param      sector_offset   Sector offset
 *
 * @retval     SSP_SUCCESS                 Driver boot read request completed successfully
 * @retval     SSP_ERR_MEDIA_OPEN_FAILED   Not a valid boot record or partition table
 *
 * @return                         See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                 codes. This function calls
 *                                        * sf_block_media_api_t::read
 *                                        * check_partition_offset
 *********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_multi_partition_boot_read_handler(sf_block_media_instance_t * p_block_media,
                                                   uint8_t                   * p_buffer,
                                                   uint32_t            const   sector_offset,
                                                   uint32_t                  * driver_status)
{
    ssp_err_t ret_val = SSP_SUCCESS;
    uint32_t partition_offset = 0U;

    /**  Read the boot record and return to the caller.  */
    /** Read the first non hidden sector. */
    ret_val = p_block_media->p_api->read(p_block_media->p_ctrl,
                                         p_buffer,
                                         sector_offset,
                                         1);
    if(SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }

    /** Check the partition offset to determine if the current sector is the boot record or partition table. */
    ret_val = check_partition_offset(p_buffer, &partition_offset);

    if (partition_offset > 0U)
    {
        /**   *
         *   Read the sector at the offset indicated in the partition table. */
        ret_val = p_block_media->p_api->read(p_block_media->p_ctrl,
                                             p_buffer,
                                             partition_offset,
                                             1);
        if(SSP_SUCCESS != ret_val)
        {
            return ret_val;
        }

        /**    *
         *    Check the partition offset to confirm it is the boot record. */
        ret_val = check_partition_offset(p_buffer, &partition_offset);
    }

    /** check_partition_offset failed or the current buffer is not the boot sector */
    if((ret_val != SSP_SUCCESS) || (partition_offset != 0U))
    {
        *driver_status = (uint32_t) FX_MEDIA_INVALID;
        return ret_val == SSP_SUCCESS ? SSP_ERR_MEDIA_OPEN_FAILED : ret_val;
    }
    return ret_val;
}

/******************************************************************************************************************//**
 * @brief      Handles the block driver boot read request for single partition.
 *
 * @param      p_block_media   The block media pointer
 * @param      p_buffer        The buffer pointer
 * @param      driver_status   The driver status
 * @param      sector_offset   No.of hidden sectors
 *
 * @retval     SSP_SUCCESS                 Driver boot read request completed successfully
 * @retval     SSP_ERR_MEDIA_OPEN_FAILED   Not a valid boot record or partition table
 *
 * @return                         See @ref Common_Error_Codes or lower level drivers for other possible return
 *                                 codes. This function calls
 *                                        * sf_block_media_api_t::read
 *                                        * check_fat_boot_record
 *                                        * check_partition_offset
 *********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_single_partition_boot_read_handler(sf_block_media_instance_t * p_block_media,
                                                   uint8_t                   * p_buffer,
                                                   uint32_t              const sector_offset,
                                                   uint32_t                  * driver_status)

{
    ssp_err_t ret_val =  SSP_SUCCESS;
    uint32_t partition_offset = sector_offset;

    /** Read the sector at sector offset */
    ret_val = p_block_media->p_api->read(p_block_media->p_ctrl,
                                         p_buffer,
                                         sector_offset,
                                         1);
    if(SSP_SUCCESS != ret_val)
    {
        return ret_val;
    }

    /** Check if it is a boot record and return to caller if true */
    ret_val = check_fat_boot_record(p_buffer, &partition_offset);

    if(SSP_SUCCESS != ret_val)
    {
        partition_offset = 0U;

        /** Read the first sector */
        ret_val = p_block_media->p_api->read(p_block_media->p_ctrl,
                                             p_buffer,
                                             0,
                                             1);
        if(SSP_SUCCESS != ret_val)
        {
            return ret_val;
        }

        /** Check if the first sector is boot record or MBR, if MBR, read the partition offset */
        ret_val = check_partition_offset (p_buffer, &partition_offset);

        if(partition_offset > 0U)
        {
            /** Read the sector at the partition offset */
            ret_val = p_block_media->p_api->read(p_block_media->p_ctrl,
                                                 p_buffer,
                                                 partition_offset,
                                                 1);
            if(SSP_SUCCESS != ret_val)
            {
                return ret_val;
            }

            /** Check for boot record */
            ret_val = check_fat_boot_record(p_buffer, &partition_offset);
        }

        if(SSP_SUCCESS != ret_val)
        {
            /** If unsuccessful, initialize the driver status as FX_MEDIA_INVALID and return to the caller*/
            *driver_status = (uint32_t) FX_MEDIA_INVALID;
            return ret_val;
        }
    }
    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Handles the driver initialization. After system reset reads the MBR/EBR(if exists) tables and update
 *          the data
 *
 * @param[in]  p_ctrl                   Pointer to SF_EL_FX control block
 * @param[in]  p_block_media            Pointer to lower level block media control block
 * @param[in]  p_fx_media               Pointer to FileX media
 *
 * @retval  SSP_SUCCESS                 If lower level block media calls are successful.
 * @retval  SSP_ERR_ASSERTION           If lower level block media calls are unsuccessful.
 *
 * @return                     See @ref Common_Error_Codes or lower level drivers for other possible return
 *                             codes. This function calls
 *                              * sf_el_fx_driver_open
 *                              * sf_el_fx_driver_init_system_partial
 *                              * sf_el_fx_driver_init_handler
 *                              * sf_el_fx_driver_init_system_full
 *                              * sf_el_fx_callback
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_request_handler_initialization(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                                                sf_block_media_instance_t * const p_block_media,
                                                                FX_MEDIA                  * p_fx_media )
{
    ssp_err_t ret_val = SSP_SUCCESS;
    uint32_t hidden_sector = 0U;
    sf_el_fx_config_t const * const p_config = (sf_el_fx_config_t *)(((sf_el_fx_t *)p_fx_media->fx_media_driver_info)->p_config);

#if  (SF_EL_FX_CFG_PARAM_CHECKING_ENABLE)
    /* Perform parameter checking */
    SSP_ASSERT(NULL != p_config);
    SSP_ASSERT(NULL != p_ctrl);
#endif

    if (SF_EL_FX_OPEN != p_ctrl->open)
    {
        if (SF_EL_FX_SYS_INIT_FULL != p_ctrl->media_info.init_status)
        {
            sf_el_fx_driver_open(p_ctrl, p_config);
        }
        else
        {
            p_ctrl->open = SF_EL_FX_OPEN;
        }
    }

    /** Check if the system is initialized or not */
    if (SF_EL_FX_SYS_UNINIT == p_ctrl->media_info.init_status)
    {
        /* Initialize the system partially */
        sf_el_fx_driver_init_system_partial(p_ctrl);
    }

    ret_val = sf_el_fx_driver_init_handler(p_ctrl,
                                           p_block_media,
                                           (uint32_t *) &p_fx_media->fx_media_driver_write_protect,
                                           (uint32_t *) &p_fx_media->fx_media_driver_free_sector_update);

    /* Check if init_handler is successful or not */
    if (SSP_SUCCESS == ret_val)
    {
        /* Check if the system is partially initialize */
        if (SF_EL_FX_SYS_INIT_PARTIAL == p_ctrl->media_info.init_status)
        {
            /** Initialize the system completely */
            sf_el_fx_driver_init_system_full(p_ctrl, p_block_media);
        }

        /** Check if the system is having multiple partitions */
        if (SF_EL_FX_MULTIPLE_PARTITIONS == p_ctrl->media_info.partition.multiple_partitions_status)
        {
            if (SF_EL_FX_PARTITION_PRE_RESET_FORMATED == p_ctrl->media_info.partition.p_data[0].format_status)
            {
                if (0U == p_fx_media->fx_media_hidden_sectors)
                {
                    ret_val = sf_el_fx_callback(p_ctrl, &hidden_sector);
                    if (SSP_SUCCESS == ret_val)
                    {
                        p_fx_media->fx_media_hidden_sectors = hidden_sector;
                    }
                }
            }

            if (SSP_SUCCESS == ret_val)
            {
                /** Increment the open counter */
                p_ctrl->media_info.global_open.counter++;
            }
        }
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Handles the driver close/un-initialize
 *
 * @param[in]  p_ctrl                   Pointer to SF_EL_FX control block
 * @param[in]  p_block_media            Pointer to lower level block media control block
 * @param[in]  p_fx_media               Pointer to FileX media
 *
 * @retval  SSP_SUCCESS                 If lower level block media calls are successful.
 * @retval  SSP_ERR_ASSERTION           If lower level block media calls are unsuccessful.
 *
 * @return                     See @ref Common_Error_Codes or lower level drivers for other possible return
 *                             codes. This function calls
 *                              * sf_block_media_api_t::close
 *                              * sf_el_fx_driver_update_partition_close_status
***********************************************************************************************************************/
static ssp_err_t  sf_el_fx_driver_request_handler_driver_uninitialize(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                                                      sf_block_media_instance_t * const p_block_media,
                                                                      FX_MEDIA                  *       p_fx_media )
{
    ssp_err_t  ret_val = SSP_SUCCESS;

    /** Check if the system is having multiple partitions */
    if (SF_EL_FX_MULTIPLE_PARTITIONS == p_ctrl->media_info.partition.multiple_partitions_status)
    {
        /** Check if only single partition is in opened state and status variable has value open */
        if ((1U == p_ctrl->media_info.global_open.counter) &&
                (SF_EL_FX_PARTITION_GLOBAL_OPEN == p_ctrl->media_info.global_open.status))
        {
            /**  *
             * Close/un-initialize device. */
            ret_val = p_block_media->p_api->close(p_block_media->p_ctrl);

            /* Check if the closing of the driver is successful or not */
            if (SSP_SUCCESS == ret_val)
            {
                /* Update the open status as close */
                p_ctrl->media_info.global_open.status  = SF_EL_FX_PARTITION_GLOBAL_CLOSE;

                /* Reset the partition open counter */
                p_ctrl->media_info.global_open.counter = 0U;

                /* The device is now considered closed */
                p_ctrl->open = 0U;
            }
        }
        else
        {
            /* Update the return value as success */
            ret_val = SSP_SUCCESS;

            /** Decrement the partition counter by 1 */
            p_ctrl->media_info.global_open.counter--;
        }

        /** Check if the boot read was successful or not */
        if (SSP_SUCCESS == ret_val)
        {
            /** Update the partition open status */
            ret_val = sf_el_fx_driver_update_partition_close_status(p_ctrl, (uint32_t)p_fx_media->fx_media_hidden_sectors);
        }
    }
    else
    {
        /**  *
         * Close/un-initialize device. */
        ret_val = p_block_media->p_api->close(p_block_media->p_ctrl);

        /* The device is now considered closed */
        p_ctrl->open = 0U;
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Handles the writing on boot sector of the media
 *
 * @param[in]  p_ctrl                   Pointer to SF_EL_FX control block
 * @param[in]  p_block_media            Pointer to lower level block media control block
 * @param[in]  p_fx_media               Pointer to FileX media
 *
 * @retval  SSP_SUCCESS                 If lower level block media calls are successful.
 * @retval  SSP_ERR_ASSERTION           If lower level block media calls are unsuccessful.
 *
 * @return                     See @ref Common_Error_Codes or lower level drivers for other possible return
 *                             codes. This function calls
 *                              * sf_block_media_api_t::write
 *                              * sf_el_fx_driver_find_partition_entry
 *                              * sf_el_fx_driver_validate_new_partition
 *                              * sf_el_fx_driver_update_mbr_ebr
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_request_handler_write_media_boot_sector (sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                                                          sf_block_media_instance_t * const p_block_media,
                                                                          FX_MEDIA                  *       p_fx_media )
{
    ssp_err_t ret_val = SSP_SUCCESS;

    /** Check if the system is having multiple partitions */
    if (SF_EL_FX_MULTIPLE_PARTITIONS == p_ctrl->media_info.partition.multiple_partitions_status)
    {
        /** Find partition entry */
        ret_val = sf_el_fx_driver_find_partition_entry(p_ctrl, (uint32_t)p_fx_media->fx_media_hidden_sectors);
    }

    if (SSP_SUCCESS == ret_val)
    {
        /**     *
         *     Write the boot record and return to the caller. */
        ret_val = p_block_media->p_api->write(p_block_media->p_ctrl,
                                              p_fx_media->fx_media_driver_buffer,
                                              p_fx_media->fx_media_hidden_sectors,
                                              1);
    }
    else
    {
        /** New partition validation */
        ret_val = sf_el_fx_driver_validate_new_partition(p_ctrl,
                                                         p_fx_media->fx_media_driver_buffer,
                                                         (uint32_t)p_fx_media->fx_media_hidden_sectors);

        if (SSP_SUCCESS == ret_val)
        {
            /**     *
             *     Write the boot record and return to the caller. */
            ret_val = p_block_media->p_api->write(p_block_media->p_ctrl,
                                                  p_fx_media->fx_media_driver_buffer,
                                                  p_fx_media->fx_media_hidden_sectors,
                                                  1);

            if (SSP_SUCCESS == ret_val)
            {
                /** Update MBR/EBR table */
                ret_val = sf_el_fx_driver_update_mbr_ebr(p_ctrl,
                                                         p_block_media,
                                                         p_fx_media->fx_media_driver_buffer,
                                                         (uint32_t)p_fx_media->fx_media_hidden_sectors);
            }
        }
    }

    return ret_val;
}

/*******************************************************************************************************************//**
 * @brief   Handles the read from boot sector of the media
 *
 * @param[in]  p_ctrl                   Pointer to SF_EL_FX control block
 * @param[in]  p_block_media            Pointer to lower level block media control block
 * @param[in]  p_fx_media               Pointer to FileX media
 *
 * @retval  SSP_SUCCESS                 If lower level block media calls are successful.
 * @retval  SSP_ERR_ASSERTION           If lower level block media calls are unsuccessful.
 *
 * @return                     See @ref Common_Error_Codes or lower level drivers for other possible return
 *                             codes. This function calls
 *                              * sf_el_fx_driver_single_partition_boot_read_handler
 *                              * sf_el_fx_driver_multi_partition_boot_read_handler
 *                              * sf_el_fx_driver_update_partition_open_status
***********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_request_handler_read_media_boot_sector(sf_el_fx_instance_ctrl_t  * const p_ctrl,
                                                                        sf_block_media_instance_t * const p_block_media,
                                                                        FX_MEDIA                  *       p_fx_media )
{
    ssp_err_t  ret_val = SSP_SUCCESS;

    /** Check if the system is not having partitions */
    if (SF_EL_FX_MULTIPLE_PARTITIONS != p_ctrl->media_info.partition.multiple_partitions_status)
    {
        /*Call the single partition boot read handler*/
        ret_val = sf_el_fx_driver_single_partition_boot_read_handler(p_block_media,
                                                    p_fx_media->fx_media_driver_buffer,
                                                    (uint32_t) p_fx_media->fx_media_hidden_sectors,
                                                    (uint32_t *) &p_fx_media->fx_media_driver_status);
    }
    else
    {
        /*Call the multi partition boot read handler*/
        ret_val = sf_el_fx_driver_multi_partition_boot_read_handler(p_block_media,
                                                    p_fx_media->fx_media_driver_buffer,
                                                    (uint32_t) p_fx_media->fx_media_hidden_sectors,
                                                    (uint32_t *) &p_fx_media->fx_media_driver_status);

        /** Check if the boot read was successful or not */
        if (SSP_SUCCESS == ret_val)
        {
            /* Update the partition open status */
            ret_val = sf_el_fx_driver_update_partition_open_status(p_ctrl, (uint32_t)p_fx_media->fx_media_hidden_sectors);
        }
    }

    return ret_val;
}

/******************************************************************************************************************//**
 * @brief      Handles the FileX driver request
 * @param      p_fx_media      The pointer to FileX media
 *
 * @retval     SSP_SUCCESS                 Driver request fulfilled.
 * @retval     SSP_ERR_ASSERTION           Null Pointer.
 * @retval     SSP_ERR_MEDIA_OPEN_FAILED   Not a valid boot record or partition table
 * @retval     SSP_ERR_UNSUPPORTED         Requested FileX command is not supported
 *
 * @return                     See @ref Common_Error_Codes or lower level drivers for other possible return
 *                             codes. This function calls
 *                              * sf_block_media_api_t::read
 *                              * sf_block_media_api_t::write
 *                              * sf_block_media_api_t::close
 *                              * sf_el_fx_driver_release_sectors_handler
 *                              * sf_el_fx_driver_request_handler_initialization
 *                              * sf_el_fx_driver_request_handler_driver_uninitialize
 *                              * sf_el_fx_driver_request_handler_read_media_boot_sector
 *                              * sf_el_fx_driver_request_handler_write_media_boot_sector
 *********************************************************************************************************************/
static ssp_err_t sf_el_fx_driver_request_handler(FX_MEDIA * p_fx_media)
{
    ssp_err_t                         ret_val       = SSP_SUCCESS;
    UINT                              fat_bytes_per_sector = 0;
    sf_el_fx_instance_ctrl_t  *       p_ctrl        = (sf_el_fx_instance_ctrl_t *)(((sf_el_fx_t *)
                                                          p_fx_media->fx_media_driver_info)->p_ctrl);
    sf_block_media_instance_t *       p_block_media = (sf_block_media_instance_t *)
            ((sf_el_fx_config_t *)(((sf_el_fx_t *)p_fx_media->fx_media_driver_info)->p_config))->p_lower_lvl_block_media;

#if  (SF_EL_FX_CFG_PARAM_CHECKING_ENABLE)
    /** Perform parameter checking */
    SSP_ASSERT(NULL != p_ctrl);
    SSP_ASSERT(NULL != p_block_media);
#endif

    /** Process the driver request specified in the media control block.  */
    switch (p_fx_media->fx_media_driver_request)
    {
        /** FX_DRIVER_READ \n
         *  FileX reads one or more sectors into memory by issuing a read request to the I/O driver.
         *
         */
        case (UINT)FX_DRIVER_READ:
        {
            ret_val = p_block_media->p_api->read(p_block_media->p_ctrl,
                                                 p_fx_media->fx_media_driver_buffer,
                                                 p_fx_media->fx_media_driver_logical_sector +
                                                     p_fx_media->fx_media_hidden_sectors,
                                                 p_fx_media->fx_media_driver_sectors);
            break;
        }

        /** FX_DRIVER_WRITE \n
         *  FileX writes one or more sectors to the physical media by issuing a write request to the I/O driver.
         *
         */
        case (UINT)FX_DRIVER_WRITE:
        {
            /** If the logical sector is zero, and the fat details are also zero, then it is a exfat boot
             * sector*/
            fat_bytes_per_sector = _fx_utility_16_unsigned_read(&p_fx_media->fx_media_driver_buffer[FX_BYTES_SECTOR]);
            if((0U == p_fx_media->fx_media_driver_logical_sector) && (0U == fat_bytes_per_sector))
            {
                /** Call the handler to write the exfat boot sector and its corresponding MBR and EBR
                 * sectors */
                ret_val = sf_el_fx_driver_request_handler_write_media_boot_sector(p_ctrl, p_block_media, p_fx_media);
            }
            else
            {
                ret_val = p_block_media->p_api->write(p_block_media->p_ctrl,
                                                  p_fx_media->fx_media_driver_buffer,
                                                  p_fx_media->fx_media_driver_logical_sector +
                                                      p_fx_media->fx_media_hidden_sectors,
                                                  p_fx_media->fx_media_driver_sectors);
            }
            break;
        }

        /** FX_DRIVER_FLUSH \n
         *   FileX flushes all sectors currently in the driver's sector cache to the physical media by issuing a flush
         *   request to the I/O driver. Synergy drivers do not currently cache sectors.
         *    - Command not currently supported in available Synergy modules. Return driver success.
         */
        case (UINT)FX_DRIVER_FLUSH:
        {
            break;
        }

        /** FX_DRIVER_RELEASE_SECTORS \n
         *  If previously selected by the driver during initialization, FileX informs the driver whenever one or more
         *  consecutive sectors become free.
         *
         */
        case (UINT)FX_DRIVER_RELEASE_SECTORS:
        {
            /* fx_media_driver_free_sector_update is set when FileX sends the FX_DRIVER_INIT request. FileX only
             * sends the FX_DRIVER_RELEASE_SECTORS request if fx_media_driver_free_sector_update is FX_TRUE
             */
            ret_val = sf_el_fx_driver_release_sectors_handler(p_block_media,
                                                              (uint32_t) p_fx_media->fx_media_driver_sectors,
                                                              (uint32_t) p_fx_media->fx_media_driver_logical_sector);
            break;
        }

        /** FX_DRIVER_ABORT \n
         *  FileX informs the driver to abort all further physical I/O activity with the physical media by issuing an
         *  abort request to the I/O driver. The driver should not perform any I/O again until it is re-initialized.
         */
        case (UINT)FX_DRIVER_ABORT:
        {
            /**  *
            * Close/un-initialize device. */
            ret_val = p_block_media->p_api->close(p_block_media->p_ctrl);
            break;
        }

        /** FX_DRIVER_INIT \n
         *  Although the actual driver initialization processing is application specific, it usually consists of data
         *  structure initialization and possibly some preliminary hardware initialization. This request is the first
         *  made by FileX and is done from within the fx_media_open service. If media write protection is detected,
         *  the driver fx_media_driver_write_protect member of FX_MEDIA should be set to FX_TRUE.
         */
        case (UINT)FX_DRIVER_INIT:
        {
            ret_val = sf_el_fx_driver_request_handler_initialization(p_ctrl, p_block_media, p_fx_media);
            break;
         }

        /** FX_DRIVER_UNINIT \n
         *  FileX uses the uninit command to close the media.
         */
        case (UINT)FX_DRIVER_UNINIT:
        {

            ret_val = sf_el_fx_driver_request_handler_driver_uninitialize(p_ctrl, p_block_media, p_fx_media);
            break;
        }

        /** FX_DRIVER_BOOT_READ \n
         *  Instead of using a standard read request, FileX makes a specific request to read the media's boot
         *  sector.
         */
        case (UINT)FX_DRIVER_BOOT_READ:
        {

            ret_val = sf_el_fx_driver_request_handler_read_media_boot_sector(p_ctrl, p_block_media, p_fx_media);
            break;
        }

        /** FX_DRIVER_BOOT_WRITE \n
         *  Instead of using a standard write request, FileX makes a specific request to write the media's boot
         *  sector.
         */
        case (UINT)FX_DRIVER_BOOT_WRITE:
        {

            ret_val = sf_el_fx_driver_request_handler_write_media_boot_sector(p_ctrl, p_block_media, p_fx_media);
            break;
        }

        /** default \n
         *  Invalid driver request. */
        default:
        {
            p_fx_media->fx_media_driver_status = ( UINT)FX_IO_ERROR;
            ret_val = SSP_ERR_UNSUPPORTED;
            break;
        }
    }

    return ret_val;
}



