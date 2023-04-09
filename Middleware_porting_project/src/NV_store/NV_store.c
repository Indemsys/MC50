// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2018.09.03
// 23:06:41
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

extern const flash_instance_t flash_cbl;

#define INI_STR_SIZE 512
#define TMP_BUF_SZ   512

uint32_t   g_setting_recordc_counter;

static uint32_t Restore_settings_from_DataFlash(const T_NV_parameters_instance  *p_pars);

static T_settings_restore_results nvr;

void  _Restore_default_IPs(void);


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return T_file_system_init_results*
-----------------------------------------------------------------------------------------------------*/
T_settings_restore_results* Get_Setting_restoring_res(void)
{
  return &nvr;
}


/*-----------------------------------------------------------------------------------------------------
  Востанавливаем IP адресса по умолчанию

  \param void
-----------------------------------------------------------------------------------------------------*/
void  _Restore_default_IPs(void)
{
//  strcpy((char *)wvar.wifi_dhcp_start_ip, DEFAULT_HOST_IP_ADDRESS);
//  strcpy((char *)wvar.mqtt_server_ip, DEFAULT_HOST_IP_ADDRESS);
//  Restore_default_dyn_tables();
}


/*-------------------------------------------------------------------------------------------
  Восстановление параметров по умолчанию, после сбоев системы или смены версии
---------------------------------------------------------------------------------------------*/
void Return_def_params(const T_NV_parameters_instance *p_pars)
{
  uint16_t  i;
  const     T_NV_parameters    *pp;

  // Загрузить параметры значениями по умолчанию
  for (i = 0; i < p_pars->items_num; i++)
  {
    pp = &p_pars->items_array[i];

    if ((pp->attr & VAL_NOINIT) == 0)
    {
      switch (pp->vartype)
      {
        // tint8u, tint16u, tuint32_t, tfloat, tarrofdouble, tarrofbyte
      case tint8u:
        *(uint8_t *)pp->val = (uint8_t)pp->defval;
        break;
      case tint16u:
        *(uint16_t *)pp->val = (uint16_t)pp->defval;
        break;
      case tint32u:
        *(uint32_t *)pp->val = (uint32_t)pp->defval;
        break;
      case tint32s:
        *(int32_t *)pp->val = (int32_t)pp->defval;
        break;
      case tfloat:
        *(float *)pp->val = (float)pp->defval;
        break;
      case tstring:
        {
          uint8_t *st;

          strncpy((char *)pp->val, (const char *)pp->pdefval, pp->varlen - 1);
          st = (uint8_t *)pp->val;
          st[pp->varlen - 1] = 0;
        }
        break;
      case tarrofbyte:
        memcpy(pp->val, pp->pdefval, pp->varlen);
        break;
      case tarrofdouble:
        break;
      }
    }
  }

  // Востанавливаем IP адресса по умолчанию
  _Restore_default_IPs();

}


/*-------------------------------------------------------------------------------------------
  Загрузка параметров из файла, после старта системы
---------------------------------------------------------------------------------------------*/
int32_t Restore_settings(const T_NV_parameters_instance *p_pars)
{
  uint32_t res;

  nvr.dataflash_saving_error = SAVED_TO_DATAFLASH_NONE;

  // Сначала запишем в параметры значения по умолчанию
  Return_def_params(p_pars);
  nvr.settings_source = RESTORED_DEFAULT_SETTINGS;


  // Восстанавливаем параметры из DataFlash
//  if (Restore_settings_from_DataFlash(p_pars) != RES_OK)
//  {
//    nvr.dataflash_restoring_error = 1;
//    Return_def_params(p_pars);
//    nvr.settings_source = RESTORED_DEFAULT_SETTINGS;
//    res = Save_settings_to(p_pars, MEDIA_TYPE_DATAFLASH, 0);
//    if (res != RES_OK)
//    {
//      nvr.dataflash_saving_error = SAVED_TO_DATAFLASH_ERROR;
//    }
//    else
//    {
//      nvr.dataflash_saving_error = SAVED_TO_DATAFLASH_OK;
//    }
//  }
//  else
//  {
//    nvr.settings_source = RESTORED_SETTINGS_FROM_DATAFLASH;
//  }

  res = Restore_settings_from_INI_file(p_pars);
  if (res == RES_OK)
  {
    nvr.settings_source = RESTORED_SETTINGS_FROM_INI_FILE;
    res = Save_settings_to(p_pars, MEDIA_TYPE_DATAFLASH, 0);
    if (res != RES_OK)
    {
      nvr.dataflash_saving_error = SAVED_TO_DATAFLASH_ERROR;
    }
    else
    {
      nvr.dataflash_saving_error = SAVED_TO_DATAFLASH_OK;
    }
    fx_file_rename(&fat_fs_media, SETTINGS_INI_FILE_NAME, SETTINGS_USED_INI_FILE_NAME);
    fx_media_flush(&fat_fs_media);
  }
  else
  {
    // Пытаемся восстановить параметры из файла на диске
    res = Restore_settings_from_JSON_file(p_pars, 0);
    if (res == RES_OK)
    {
      nvr.settings_source = RESTORED_SETTINGS_FROM_JSON_FILE;
      res = Save_settings_to(p_pars, MEDIA_TYPE_DATAFLASH, 0);
      if (res != RES_OK)
      {
        nvr.dataflash_saving_error = SAVED_TO_DATAFLASH_ERROR;
      }
      else
      {
        nvr.dataflash_saving_error = SAVED_TO_DATAFLASH_OK;
      }
      Delete_app_settings_file(); // Не оставляем файлов с установками на диске
    }
    else
    {
      if (Restore_settings_from_DataFlash(p_pars) != RES_OK)
      {
        nvr.dataflash_restoring_error = 1;
        Return_def_params(p_pars);
        nvr.settings_source = RESTORED_DEFAULT_SETTINGS;
        res = Save_settings_to(p_pars, MEDIA_TYPE_DATAFLASH, 0);
        if (res != RES_OK)
        {
          nvr.dataflash_saving_error = SAVED_TO_DATAFLASH_ERROR;
        }
        else
        {
          nvr.dataflash_saving_error = SAVED_TO_DATAFLASH_OK;
        }
      }
      else
      {
        nvr.dataflash_saving_error = SAVED_TO_DATAFLASH_NONE;
        nvr.settings_source = RESTORED_DEFAULT_SETTINGS;
      }
    }
  }
  return (RES_OK);
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t  Save_settings(const T_NV_parameters_instance *p_pars)
{
  return  Save_settings_to(p_pars, MEDIA_TYPE_DATAFLASH, 0);
}

/*-------------------------------------------------------------------------------------------
  Восстановить серийный номер, MAC адрес и другие постоянные величины из файла
---------------------------------------------------------------------------------------------*/
uint32_t Restore_settings_from_INI_file(const T_NV_parameters_instance  *p_pars)
{
  FX_FILE            f;
  char              *str;
  char               *inbuf;
  char              *var_alias;
  char              *val;
  int32_t            n;
  uint32_t           pcnt;
  int32_t            scan_res;


  str = App_malloc_pending(INI_STR_SIZE + 1,10);
  if (str == NULL)
  {
    return RES_ERROR;
  }

  inbuf = App_malloc_pending(TMP_BUF_SZ + 1,10);
  if (inbuf == NULL)
  {
    App_free(str);
    return RES_ERROR;
  }
  // Открываем файл

  if (fx_file_open(&fat_fs_media,&f, SETTINGS_INI_FILE_NAME,(FX_OPEN_FOR_READ)) != FX_SUCCESS)
  {
    App_free(str);
    App_free(inbuf);
    return RES_ERROR;
  }

  pcnt = 0;
  do
  {
    // Читаем строку из файла
    if (Scanf_from_file(&f,&scan_res, inbuf, TMP_BUF_SZ, "%s\n", str) == RES_ERROR) break;
    if (scan_res == 1)
    {
      if (str[0] == 0)  continue;
      if (str[0] == ';') continue;

      // Ищем указатель на символ =
      val = strchr(str, '=');
      if (val == NULL) continue; // Если символа не найдено то это не запись параметра
      *val = 0; // Вставляем 0 по адресу симвла = чтобы отделить стороки имени и значения
      val++; // Переходим на первый символ строки значения
      var_alias = str;
      // Найти параметр по аббревиатуре
      var_alias = Trim_and_dequote_str(var_alias);
      n = Find_param_by_alias(p_pars, var_alias);
      if (n >= 0)
      {
        val = Trim_and_dequote_str(val);
        Convert_str_to_parameter(p_pars, (uint8_t *)val, n);
        pcnt++;
      }
    }
  }while (1);


  fx_file_close(&f);
  App_free(str);
  App_free(inbuf);

  return RES_OK;
}

/*-------------------------------------------------------------------------------------------
   Процедура сохранения в ini-файл параметров
---------------------------------------------------------------------------------------------*/
uint32_t Save_settings_to_INI_file(const T_NV_parameters_instance *p_pars)
{
  FX_FILE            f;
  uint32_t           res;
  uint32_t           i,n;
  char               *str;
  char               *inbuf;
  uint32_t           offs;
  char               *tmp_str;
  char               *name;
  char               *prev_name;
  const T_NV_parameters    *pp;

  str = App_malloc_pending(INI_STR_SIZE + 1,10);
  if (str == NULL)
  {
    return RES_ERROR;
  }

  inbuf = App_malloc_pending(TMP_BUF_SZ + 1,10);
  if (inbuf == NULL)
  {
    App_free(str);
    return RES_ERROR;
  }
  // Открываем файл


  fx_file_delete(&fat_fs_media,SETTINGS_INI_FILE_NAME);
  res = fx_file_create(&fat_fs_media,SETTINGS_INI_FILE_NAME);
  if (res != FX_SUCCESS)
  {
    App_free(str);
    App_free(inbuf);
    return RES_ERROR;
  }
  res = fx_file_open(&fat_fs_media,&f, SETTINGS_INI_FILE_NAME,  FX_OPEN_FOR_WRITE);
  if (res != FX_SUCCESS)
  {
    App_free(str);
    App_free(inbuf);
    return RES_ERROR;
  }

  n =0;
  name      = 0;
  prev_name = 0;
  for (i = 0; i < p_pars->items_num; i++)
  {
    pp = &p_pars->items_array[i];

    if ((pp->attr & 1) == 0) // сохраняем только если параметр для записи
    {
      offs = 0;

      // Параметр должен быть сохранен
      name = (char *)Get_mn_name(p_pars, pp->parmnlev);
      if (name != prev_name)
      {
        sprintf(inbuf + offs, ";--------------------------------------------------------\r\n");
        offs = offs + strlen(inbuf + offs);
        sprintf(inbuf + offs, "; %s\r\n", name);
        offs = offs + strlen(inbuf + offs);
        sprintf(inbuf + offs, ";--------------------------------------------------------\r\n\r\n");
        offs = offs + strlen(inbuf + offs);
        prev_name = name;
      }
      sprintf(inbuf + offs, "; N=%03d %s\r\n%s=", n++, pp->var_description, pp->var_alias);
      offs = offs + strlen(inbuf + offs);
      if (pp->vartype == tstring)
      {
        Convert_parameter_to_str(p_pars, (uint8_t *)(str + 1), MAX_PARAMETER_STRING_LEN, i);
        *str = '"';
        tmp_str = str + strlen(str);
        *tmp_str = '"';
        tmp_str++;
      }
      else
      {
        Convert_parameter_to_str(p_pars, (uint8_t *)str, MAX_PARAMETER_STRING_LEN, i);
        tmp_str = str + strlen(str);
      }
      *tmp_str = '\r';
      tmp_str++;
      *tmp_str = '\n';
      tmp_str++;
      *tmp_str = '\r';
      tmp_str++;
      *tmp_str = '\n';
      tmp_str++;
      *tmp_str = '\0';
      sprintf(inbuf + offs, "%s", str);
      offs = offs + strlen(inbuf + offs);

      if (fx_file_write(&f, inbuf, offs) != FX_SUCCESS)
      {
        fx_file_close(&f);
        App_free(str);
        App_free(inbuf);
        return RES_ERROR;
      }
    }
  }
  fx_file_close(&f);
  fx_media_flush(&fat_fs_media);
  App_free(str);
  App_free(inbuf);
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param file_name
  \param bud
  \param buf_sz
-----------------------------------------------------------------------------------------------------*/
static uint32_t Save_settings_buf_to_file(char *file_name, uint8_t *buf, ULONG buf_sz)
{
  uint32_t   res = RES_ERROR;

  FX_FILE    f;
  char      *fname;

  f.fx_file_id = 0;

  fname = file_name;
  // Открываем файл на запись
  if (file_name == 0)
  {
    if (wvar.en_compress_settins) fname = COMPESSED_SETTINGS_JSON_FILE_NAME;
    else fname = SETTINGS_JSON_FILE_NAME;
  }
  res = Recreate_file_for_write(&f, fname);
  if (res != FX_SUCCESS)
  {
    goto EXIT_ON_ERROR;
  }

  if (fx_file_write(&f, buf, buf_sz) != FX_SUCCESS) goto EXIT_ON_ERROR;

  res = RES_OK;
EXIT_ON_ERROR:
  if (f.fx_file_id == FX_FILE_ID) fx_file_close(&f);
  fx_media_flush(&fat_fs_media);

  return res;
}


/*-----------------------------------------------------------------------------------------------------
  Сохраняем настройки в две области памяти DataFlash

   Формат хранения параметров во Flash

  -------------
  4-е байта - размер данных N
  -------------
  4-е байта - счетчик количества модификаций данных сопровождавшихся перезаписью в DataFlash : g_setting_recordc_counter
  -------------
  N байт    - данные в виде сжатого JSON
  -------------
  4-е байта - контрольная сумма
  -------------


  \param buf
  \param buf_sz

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Save_settings_buf_to_DataFlash(uint8_t *buf, uint32_t buf_sz)
{
  uint8_t   *tmp_buf = 0;
  uint32_t   tmp_buf_sz;
  uint16_t   crc;
  uint32_t   csz;

  uint32_t err_step = 1;

  g_setting_recordc_counter++;

  if ((buf_sz > (SETTINGS_AREA_SIZE_IN_DATAFLASH - DATAFLASH_CHECK_BLOCK_SIZE)) || (buf_sz < 8)) goto EXIT_ON_ERROR;
  csz = buf_sz;
  if ((csz & 0x3) != 0) csz =(csz & 0xFFFFFFFC)+ 4; // Выравниваем размер буфера данных по 4

  tmp_buf_sz = csz + DATAFLASH_CHECK_BLOCK_SIZE; // Добавляем размер буфера данных, номер записи и CRC

  err_step = 2;
  tmp_buf = App_malloc_pending(tmp_buf_sz,10);
  if (tmp_buf == NULL) goto EXIT_ON_ERROR;

  // Записываем служебные данные
  memcpy(&tmp_buf[0],&csz, 4);
  memcpy(&tmp_buf[4],&g_setting_recordc_counter, 4);
  memcpy(&tmp_buf[8], buf, buf_sz);
  crc = Get_CRC16_of_block(tmp_buf,tmp_buf_sz - 4, 0xFFFF);
  memcpy(&tmp_buf[8+csz],&crc, 2);
  memcpy(&tmp_buf[10+csz],&crc, 2);


  err_step = 3;
  // Стираем область памяти 1 хранения настроек
  if (S7G2_DataFlash_EraseArea(SETTINGS_START_ADDR1_IN_DATAFLASH,SETTINGS_AREA_SIZE_IN_DATAFLASH) != RES_OK) goto EXIT_ON_ERROR;
  err_step = 4;
  if (S7G2_DataFlash_WriteArea(SETTINGS_START_ADDR1_IN_DATAFLASH, tmp_buf, tmp_buf_sz) != RES_OK) goto EXIT_ON_ERROR;
  err_step = 5;
  // Стираем область памяти 2 хранения настроек
  if (S7G2_DataFlash_EraseArea(SETTINGS_START_ADDR2_IN_DATAFLASH,SETTINGS_AREA_SIZE_IN_DATAFLASH) != RES_OK) goto EXIT_ON_ERROR;
  err_step = 6;
  if (S7G2_DataFlash_WriteArea(SETTINGS_START_ADDR2_IN_DATAFLASH, tmp_buf, tmp_buf_sz) != RES_OK) goto EXIT_ON_ERROR;

  App_free(tmp_buf);
  return RES_OK;
EXIT_ON_ERROR:
  g_setting_recordc_counter--;
  App_free(tmp_buf);
  return err_step;

}
/*-----------------------------------------------------------------------------------------------------
  Запись утстановок в JSON файл опционально с компрессией.
  Если имя файла не задано, то приеняется имена по умолчанию

  \param file_name

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Save_settings_to(const T_NV_parameters_instance *p_pars, uint8_t media_type,  char *file_name)
{
  uint32_t   res;
  char      *json_str = NULL;
  uint32_t   json_str_sz;
  uint8_t   *compessed_data_ptr = 0;
  uint32_t   compessed_data_sz;
  uint8_t   *buf;
  uint32_t   buf_sz;
  uint32_t   flags;

  if (media_type == MEDIA_TYPE_DATAFLASH)
  {
    flags = JSON_COMPACT;
  }
  else if (wvar.en_formated_settings)
  {
    flags = JSON_INDENT(1) | JSON_ENSURE_ASCII;
  }
  else
  {
    flags = JSON_COMPACT;
  }


  if (Serialze_settings_to_mem(p_pars, &json_str,&json_str_sz, flags) != RES_OK) goto EXIT_ON_ERROR;

  if ((wvar.en_compress_settins) || (media_type == MEDIA_TYPE_DATAFLASH))
  {
    // Выделить память для сжатого файла
    compessed_data_ptr = App_malloc_pending(json_str_sz,10);
    if (compessed_data_ptr == NULL) goto EXIT_ON_ERROR;

    compessed_data_sz = json_str_sz;
    res = Compress_mem_to_mem(SIXPACK_ALG, json_str, json_str_sz,compessed_data_ptr,&compessed_data_sz);
    if (res != RES_OK) goto EXIT_ON_ERROR;
    // Добавляем контрольную сумму
    uint16_t crc = Get_CRC16_of_block(compessed_data_ptr,compessed_data_sz, 0xFFFF);
    buf = compessed_data_ptr;
    buf_sz = compessed_data_sz;
    buf[buf_sz] = crc & 0xFF;
    buf[buf_sz+1] =(crc >> 8) & 0xFF;
    buf_sz += 2;
  }
  else
  {
    buf = (uint8_t *)json_str;
    buf_sz = json_str_sz;
  }

  res = RES_OK;
  switch (media_type)
  {
  case MEDIA_TYPE_FILE:
    res = Save_settings_buf_to_file(file_name,buf, buf_sz);
    break;
  case MEDIA_TYPE_DATAFLASH:
    res = Save_settings_buf_to_DataFlash(buf, buf_sz);
    break;
  }

  App_free(compessed_data_ptr);
  if (json_str != 0) App_free(json_str);
  return res;
EXIT_ON_ERROR:
  App_free(compessed_data_ptr);
  if (json_str != 0) App_free(json_str);
  return RES_ERROR;
}

/*-----------------------------------------------------------------------------------------------------
  Восстановление установок из JSON файла, опционально с декомпрессией.
  Если имя файла не задано, то приеняется имена по умолчанию


  \param file_name

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Restore_settings_from_JSON_file(const T_NV_parameters_instance  *p_pars, char *file_name)
{
  uint8_t   *data_buf = NULL;
  uint32_t   data_buf_sz;
  uint8_t   *decompessed_data_ptr = NULL;
  uint32_t   decompessed_data_sz;
  FX_FILE    f;
  uint32_t   res;
  uint32_t   actual_sz;
  uint8_t    compressed = 0;

  // Открываем файл
  if (file_name == 0)
  {
    res = fx_file_open(&fat_fs_media,&f, COMPESSED_SETTINGS_JSON_FILE_NAME,  FX_OPEN_FOR_READ);
    if (res != FX_SUCCESS)
    {
      res = fx_file_open(&fat_fs_media,&f, SETTINGS_JSON_FILE_NAME,  FX_OPEN_FOR_READ);
      if (res != FX_SUCCESS) goto EXIT_ON_ERROR;
    }
    else compressed = 1;
  }
  else
  {
    res = fx_file_open(&fat_fs_media,&f, file_name,  FX_OPEN_FOR_READ);
    if (res != FX_SUCCESS) goto EXIT_ON_ERROR;
  }

  data_buf_sz = f.fx_file_current_file_size;
  data_buf = App_malloc_pending(data_buf_sz + 1,10);
  if (data_buf == NULL) goto EXIT_ON_ERROR;

  res = fx_file_read(&f, data_buf, data_buf_sz,(ULONG *)&actual_sz);
  if ((res != FX_SUCCESS) || (actual_sz != data_buf_sz)) goto EXIT_ON_ERROR;
  fx_file_close(&f);

  if (compressed)
  {
    // Проверка контрольной суммы
    uint16_t crc = Get_CRC16_of_block(data_buf,data_buf_sz-2, 0xFFFF);
    uint16_t ecrc = data_buf[data_buf_sz-2] +(data_buf[data_buf_sz-1]<<8);
    if (crc != ecrc) goto EXIT_ON_ERROR; // Выход если не совпала контрольная сумма

    decompessed_data_sz = data_buf[0] +(data_buf[1]<<8)+(data_buf[2]<<16)+(data_buf[3]<<24);
    if (decompessed_data_sz > 65536) goto EXIT_ON_ERROR; // Выход если после декомпрессии объем данных слишком большой
    decompessed_data_ptr = App_malloc_pending(decompessed_data_sz+1,10);
    if (decompessed_data_ptr == NULL) goto EXIT_ON_ERROR;
    // Декомпрессия
    if (Decompress_mem_to_mem(SIXPACK_ALG, data_buf, data_buf_sz-2, decompessed_data_ptr, decompessed_data_sz) != decompessed_data_sz) goto EXIT_ON_ERROR;

    App_free(data_buf);
    data_buf = 0;
    data_buf = decompessed_data_ptr;
    data_buf_sz = decompessed_data_sz;
    decompessed_data_ptr = 0;
  }

  data_buf[data_buf_sz] = 0; // Дополняем строку завершающим нулем

  // Парсим JSON формат данных
  res = JSON_Deser_settings(p_pars, (char *)data_buf);
  App_free(decompessed_data_ptr);
  App_free(data_buf);
  return res;

EXIT_ON_ERROR:
  if (f.fx_file_id == FX_FILE_ID) fx_file_close(&f);
  App_free(decompessed_data_ptr);
  App_free(data_buf);
  return RES_ERROR;

}

/*-----------------------------------------------------------------------------------------------------

  Формат хранения параметров во Flash

  -------------
  4-е байта - размер данных N
  -------------
  4-е байта - счетчик количества модификаций данных сопровождавшихся перезаписью в DataFlash : g_setting_recordc_counter
  -------------
  N байт    - данные в виде сжатого JSON
  -------------
  4-е байта - контрольная сумма
  -------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Restore_settings_from_DataFlash(const T_NV_parameters_instance  *p_pars)
{
  uint32_t    err = 1;
  uint32_t    sz;
  uint32_t    flash_addr;
  uint8_t    *buf = NULL;
  uint32_t    buf_sz;
  uint16_t    crc1, crc2;
  uint8_t    *decompessed_data_ptr = NULL;
  uint32_t    decompessed_data_sz;


  // Проходим по всем областям в поисках годной
  for (uint32_t i=0; i < 2; i++)
  {
    flash_addr = SETTINGS_START_ADDR1_IN_DATAFLASH + i * SETTINGS_AREA_SIZE_IN_DATAFLASH;
    // Читаем из  DataFlash размер блока данных
    flash_cbl.p_api->read(flash_cbl.p_ctrl, (uint8_t *)&sz  , flash_addr , 4);

    if ((sz > (SETTINGS_AREA_SIZE_IN_DATAFLASH - DATAFLASH_CHECK_BLOCK_SIZE)) || (sz < 8)) continue; // Ошибка - неправильный размер данных

    buf_sz = sz + 8;
    buf = App_malloc_pending(buf_sz,10);
    if (buf != NULL)
    {
      // Читаем данные
      flash_cbl.p_api->read(flash_cbl.p_ctrl, (uint8_t *)buf  , flash_addr  , buf_sz);

      // Читаем записанную контрольную сумму
      flash_cbl.p_api->read(flash_cbl.p_ctrl, (uint8_t *)&crc1  , flash_addr + 8 + sz, 4);
      // Расчитываем фактическую контрольную сумму
      crc2 = Get_CRC16_of_block(buf, buf_sz, 0xFFFF);
      if (crc1 == crc2)
      {
        // Контрольные суммы совпали, значит величины номера записи и количества данных верны
        memcpy(&decompessed_data_sz,&buf[8], 4);
        if (decompessed_data_sz < 65536)
        {
          decompessed_data_ptr = App_malloc_pending(decompessed_data_sz+1,10);
          if (decompessed_data_ptr != NULL)
          {
            // Декомпрессия
            if (Decompress_mem_to_mem(SIXPACK_ALG,&buf[8], sz , decompessed_data_ptr, decompessed_data_sz) == decompessed_data_sz)
            {
              memcpy(&g_setting_recordc_counter,&buf[4],4);
              App_free(buf);
              buf = 0;
              decompessed_data_ptr[decompessed_data_sz] = 0; // Дополняем нулем поскольку строка JSON должна завершаться всегда нулем
              if (JSON_Deser_settings(p_pars, (char *)decompessed_data_ptr) == RES_OK)
              {
                err = 0;
              }
            }
            App_free(decompessed_data_ptr);
          }
        }
      }
      App_free(buf);
    }
    if (err == 0) break;
  }

  if (err != 0)
  {
    return RES_ERROR;
  }
  else
  {
    return RES_OK;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Принять сертификат из файла

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Accept_certificates_from_file(void)
{
  uint8_t   *data_buf = NULL;
  uint32_t   file_size;
  FX_FILE    f;
  uint32_t   res;
  uint32_t   actual_sz;
  uint8_t   *ptr_sz;


  res = fx_file_open(&fat_fs_media,&f, CA_CERTIFICATE_FILE_NAME,  FX_OPEN_FOR_READ);
  if (res != FX_SUCCESS)
  {
    goto EXIT_ON_ERROR;
  }

  file_size = f.fx_file_current_file_size;
  if (file_size > (CA_CERT_AREA_SIZE_IN_DATAFLASH - 4)) goto EXIT_ON_ERROR;
  data_buf = App_malloc_pending(CA_CERT_AREA_SIZE_IN_DATAFLASH,10);
  if (data_buf == NULL) goto EXIT_ON_ERROR;


  res = fx_file_read(&f, data_buf, file_size,(ULONG *)&actual_sz);
  if ((res != FX_SUCCESS) || (actual_sz != file_size)) goto EXIT_ON_ERROR;
  fx_file_close(&f);

  ptr_sz = (uint8_t *)(&data_buf[CA_CERT_AREA_SIZE_IN_DATAFLASH - 4]);
  memcpy(ptr_sz,&file_size, 4);
  if (S7G2_DataFlash_EraseArea(CA_CERT_START_IN_DATAFLASH,CA_CERT_AREA_SIZE_IN_DATAFLASH) != RES_OK) goto EXIT_ON_ERROR;
  if (S7G2_DataFlash_WriteArea(CA_CERT_START_IN_DATAFLASH, data_buf, CA_CERT_AREA_SIZE_IN_DATAFLASH) != RES_OK) goto EXIT_ON_ERROR;

  App_free(data_buf);
  fx_file_delete(&fat_fs_media,CA_CERTIFICATE_FILE_NAME);
  return res;

EXIT_ON_ERROR:
  if (f.fx_file_id == FX_FILE_ID) fx_file_close(&f);
  App_free(data_buf);
  return RES_ERROR;
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Delete_app_settings_file(void)
{
  fx_file_delete(&fat_fs_media, SETTINGS_JSON_FILE_NAME);
  fx_file_delete(&fat_fs_media, COMPESSED_SETTINGS_JSON_FILE_NAME);
  fx_media_flush(&fat_fs_media);
  Wait_ms(10);
}




