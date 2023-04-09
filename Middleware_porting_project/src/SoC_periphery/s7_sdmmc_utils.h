#ifndef MC50_SDMMC_UTILS_H
  #define MC50_SDMMC_UTILS_H


  #define    SD_SET_PASSWORD   1
  #define    SD_UNLOCK         2
  #define    SD_CLEAR_PASSWORD 3


typedef union
{
    __packed struct
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
    }
    T_csd_v1;

    __packed struct
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
      uint32_t reserved_47        : 1;              ///<   [47:47]
      uint32_t c_size             : 22;             ///<   [69:48] device size
      uint32_t reserved_70        : 6;              ///<   [75:70]
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
    }
    T_csd_v2;
}
T_csd;



uint32_t   SD_card_open(void);
uint32_t   SD_card_close(void);
uint32_t   SD_get_password(char **buf, uint8_t *len);
uint8_t    SD_fill_lock_struct_with_password(T_Lock_Card_Data_Structure *p_lcds);
uint32_t   SD_card_pre_init(void);
uint32_t   SD_get_card_locked_satus(uint8_t *p_lock_stat);
uint32_t   SD_password_operations(uint8_t operation, char *pass_buff, uint8_t pass_len);



#endif



