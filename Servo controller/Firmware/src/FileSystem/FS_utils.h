#ifndef FILE_UTILS_H
  #define FILE_UTILS_H

int32_t    Read_line_from_file(FX_FILE  *fp, char *buf, uint32_t buf_len);
uint32_t   Scanf_from_file(FX_FILE  *fp, int32_t *scan_res, char *tmp_buf, uint32_t tmp_buf_sz,  const char  *fmt_ptr, ...);
uint32_t   Recreate_file_for_write(FX_FILE  *f, CHAR *filename);

ssp_err_t  Get_SD_card_info(sf_block_media_instance_t *media, uint32_t *sector_size, uint32_t *sector_count);
ssp_err_t  FS_format(void);
uint32_t   Delete_all_files_in_current_dir(void);
uint32_t   Check_file_extension(char *file_name, const char * const *ext_list);
void       Get_file_extension(char *file_name, char *ext, uint32_t ext_buf_sz);

uint64_t   Get_media_total_sectors(void);
uint32_t   Get_media_bytes_per_sector(void);
uint64_t   Get_media_total_size(void);
uint64_t   Get_media_available_size(void);

#endif



