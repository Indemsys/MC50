#ifndef FS_INIT_H
  #define FS_INIT_H


#define FS_MEMORY_SZ               (1024*10)


typedef struct
{
    uint32_t fx_media_open_result;
    uint32_t fx_corr_buff_alloc_res;
    uint32_t fs_detected_errors;
    uint32_t fs_check_error_code;
    uint32_t creation_misc_dir_res;
    uint32_t creation_records_dir_res;
    uint32_t creation_log_dir_res;

} T_file_system_init_results;


extern FX_MEDIA                    fat_fs_media;
extern sf_block_media_instance_t   sd_card_media_cbl;
extern sdmmc_instance_ctrl_t       sd_card_ctrl;
extern sf_el_fx_t                  fat_fs_media_cfg;
extern uint8_t                     fs_memory[];

uint32_t   Get_fs_memory_size(void);
uint32_t   Init_SD_card_file_system(void);
char      *Get_Flash_password(void);

T_file_system_init_results* Get_FS_init_res(void);

#endif // FS_INIT_H



