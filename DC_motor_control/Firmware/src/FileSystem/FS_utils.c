// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.08.28
// 17:13:22
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

/*-------------------------------------------------------------------------------------------------------------
  Читать строку str не длинее len из файла file оканчивающуюся на CRLF (\r\n)
  Возвращает количество символов в прочитанной строке или -1 в случае ошибки
-------------------------------------------------------------------------------------------------------------*/
int32_t Read_line_from_file(FX_FILE  *fp, char *buf, uint32_t buf_len)
{
  int32_t  indx;
  char     ch;
  ULONG    actual_size;
  uint32_t status;

  buf[0] = 0;
  indx = 0;
  do
  {
    status = fx_file_read(fp,&ch, 1,&actual_size);
    if ((actual_size > 0) && (status == TX_SUCCESS))
    {
      buf[indx++] = ch;
      buf[indx]   = 0;
      if (indx > 1)
      {
        if (strcmp(&buf[indx-2],STR_CRLF) == 0)
        {
          buf[indx-2] = 0;
          return (indx-2);
        }
      }
      if (indx >= buf_len)
      {
        return (-1);
      }
    }
    else
    {
      if (indx == 0) return -1;
      return indx;
    }
  }while (1);
}

/*-----------------------------------------------------------------------------------------------------
  Прочитать строку из файла и получить из нее заданные переменные

  \param fp
  \param fmt_ptr

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Scanf_from_file(FX_FILE  *fp, int32_t *scan_res, char *tmp_buf, uint32_t tmp_buf_sz,  const char  *fmt_ptr, ...)
{
  va_list  ap;
  va_start(ap, fmt_ptr);

  if (Read_line_from_file(fp, tmp_buf, tmp_buf_sz) < 0)
  {
    return RES_ERROR;
  }
  *scan_res = vsscanf(tmp_buf, (char *)fmt_ptr, ap);
  va_end(ap);
  return RES_OK;

}

/*-----------------------------------------------------------------------------------------------------


  \param filename

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Recreate_file_for_write(FX_FILE  *f, CHAR *filename)
{
  uint32_t res;

  res = fx_file_create(&fat_fs_media, filename);
  if (res == FX_SUCCESS)
  {
    res = fx_file_open(&fat_fs_media, f, filename,  FX_OPEN_FOR_WRITE);
  }
  else if (res == FX_ALREADY_CREATED)
  {
    res = fx_file_delete(&fat_fs_media, filename);
    fx_media_flush(&fat_fs_media);
    if (res == FX_SUCCESS)
    {
      res = fx_file_create(&fat_fs_media,filename);
      if (res == FX_SUCCESS)
      {
        res = fx_file_open(&fat_fs_media, f, filename,  FX_OPEN_FOR_WRITE);
      }
    }
  }
  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_block_media
  \param sector_size
  \param sector_count

  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t Get_SD_card_info(sf_block_media_instance_t *media, uint32_t *sector_size, uint32_t *sector_count)
{
  ssp_err_t ret_val = SSP_SUCCESS;

  ret_val = media->p_api->open(media->p_ctrl, media->p_cfg);
  if (ret_val != SSP_SUCCESS)
  {
    return ret_val;
  }

  /* Get actual sector size from media. */
  ret_val = media->p_api->ioctl(media->p_ctrl, SSP_COMMAND_GET_SECTOR_SIZE, sector_size);
  if (ret_val != SSP_SUCCESS)
  {
    return ret_val;
  }

  /* Get actual sector count from media. */
  ret_val = media->p_api->ioctl(media->p_ctrl, SSP_COMMAND_GET_SECTOR_COUNT, sector_count);
  if (ret_val != SSP_SUCCESS)
  {
    return ret_val;
  }

  /* Close driver.  */
  ret_val =  media->p_api->close(media->p_ctrl);
  return ret_val;
}

/*-----------------------------------------------------------------------------------------------------



  \return ssp_err_t
-----------------------------------------------------------------------------------------------------*/
ssp_err_t FS_format(void)
{
  uint32_t res;


  uint32_t sector_size  = 512;       // Оценочная величина
  uint32_t sector_count = 7618560;   // Оценочная величина

  // Получаем размеры вставленной SD карты
  ssp_err_t error = Get_SD_card_info(fat_fs_media_cfg.p_config->p_lower_lvl_block_media,&sector_size,&sector_count);

  if ((error != SSP_SUCCESS) || (sector_count <= 0))
  {
    return SSP_ERR_MEDIA_FORMAT_FAILED;
  }

  sector_count -= 0;


  if (Get_fs_memory_size() < 512)
  {
    return SSP_ERR_MEDIA_FORMAT_FAILED;
  }

  res = fx_media_format(&fat_fs_media, // Pointer to FileX media control block.
                        SF_EL_FX_BlockDriver,           // Driver entry
                        &fat_fs_media_cfg,              // Pointer to Block Media Driver
                        fs_memory,                      // Media buffer pointer
                        Get_fs_memory_size(),           // Media buffer size
                        (CHAR *)"Volume 1",             // Volume Name
                        1,                              // Number of FATs
                        256,                            // Directory Entries
                        0,                              // Hidden sectors
                        sector_count,                   // Total sectors - Hidden Sectors
                        sector_size,                    // Sector size
                        64,                             // Sectors per cluster
                        1,                              // Heads
                        1);                             // Sectors per track

  if (FX_SUCCESS != res)
  {
    return SSP_ERR_MEDIA_FORMAT_FAILED;
  }

  return SSP_SUCCESS;
}



/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t Возвращает RES_OK если все файлы удалены
-----------------------------------------------------------------------------------------------------*/
uint32_t Delete_all_files_in_current_dir(void)
{
  uint32_t   res;

  CHAR       entry_name[FX_MAX_LONG_NAME_LEN];
  UINT       attributes;
  ULONG      size;
  UINT       year;
  UINT       month;
  UINT       day;
  UINT       hour;
  UINT       minute;
  UINT       second;

  uint32_t   err_cnt = 0;

  // Проходим последовательно по всем файлам
  res = fx_directory_first_full_entry_find(&fat_fs_media,entry_name,&attributes,&size,&year,&month,&day,&hour,&minute,&second);
  while (res == FX_SUCCESS)
  {
    if ((attributes & (FX_DIRECTORY | FX_VOLUME)) == 0)
    {
      res = fx_file_delete(&fat_fs_media, entry_name);
      if (res  != FX_SUCCESS) err_cnt++;
    }
    res = fx_directory_next_full_entry_find(&fat_fs_media,entry_name,&attributes,&size,&year,&month,&day,&hour,&minute,&second);
  }

  fx_media_flush(&fat_fs_media);
  if (err_cnt == 0)
  {
    return RES_OK;
  }
  return RES_ERROR;
}


/*-----------------------------------------------------------------------------------------------------


  \param file_name
  \param ext

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Check_file_extension(char *file_name, const char *const *ext_list)
{
  uint32_t name_len;
  uint32_t ext_pos;
  uint32_t ext_len;
  char const *ext;

  if (ext_list == 0)  return RES_ERROR;
  if (*ext_list == 0) return RES_ERROR;

  name_len = strlen(file_name);

  do
  {
    ext =*ext_list;
    ext_len  = strlen(ext);
    ext_pos = strcspn(file_name, ".");
    if  (name_len > ext_len+1)
    {
      if (ext_pos == (name_len-ext_len-1))
      {
        if (strcmp(&file_name[ext_pos+1], ext) == 0) return RES_OK;
      }
    }
    ext_list++;

  }while  (*ext_list != 0);

  return RES_ERROR;

}


/*-----------------------------------------------------------------------------------------------------
  Получаем строку расширения из имени файла
  Если расширения нет или оно длинее предоставленного буфера - нулевой смвол, то возвращаем пустую строку

  \param file_name
  \param ext
  \param ext_sz

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
void Get_file_extension(char *file_name, char *ext, uint32_t ext_buf_sz)
{
  uint32_t  ext_pos;
  uint32_t  ext_len;
  uint32_t  name_len;

  name_len = strlen(file_name);
  ext_pos  = strcspn(file_name, ".");  //

  if (name_len == ext_pos)
  {
    *ext = 0;
    return;
  }
  ext_pos++;
  ext_len = name_len - ext_pos;
  if (ext_len > (ext_buf_sz-1))
  {
    *ext = 0;
    return;
  }

  memcpy(ext,&file_name[ext_pos], ext_len);
  return;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint64_t
-----------------------------------------------------------------------------------------------------*/
uint64_t Get_media_total_sectors(void)
{
  return fat_fs_media.fx_media_total_sectors;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_media_bytes_per_sector(void)
{
  return fat_fs_media.fx_media_bytes_per_sector;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint64_t
-----------------------------------------------------------------------------------------------------*/
uint64_t Get_media_total_size(void)
{
  return fat_fs_media.fx_media_total_sectors * fat_fs_media.fx_media_bytes_per_sector;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint64_t
-----------------------------------------------------------------------------------------------------*/
uint64_t Get_media_available_size(void)
{
  uint64_t sz;
  fx_media_extended_space_available(&fat_fs_media,&sz);
  return sz;
}

