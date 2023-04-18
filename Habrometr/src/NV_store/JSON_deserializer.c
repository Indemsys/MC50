// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.09.26
// 23:11:19
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


static  uint32_t _JSON_Deser_DT(json_t  *root);
static  uint32_t _JSON_Deser_params(const T_NV_parameters_instance  *p_pars, json_t  *root);
static  uint32_t _JSON_find_array(json_t  *root, json_t  **object, char const *key_name);



/*-----------------------------------------------------------------------------------------------------


  \param root

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t JSON_Deser_tables(const T_NV_parameters_instance  *p_pars, json_t *root)
{
  uint32_t res;
  uint32_t err = 0;

  res = _JSON_Deser_params(p_pars, root);
  if (res != RES_OK) err++;
  //_JSON_Deser_DT(root);

  if (err > 0)
  {
    APPLOG("Deserialization fail count = %d.", err);
    return RES_ERROR;
  }
  else
  {
    APPLOG("Deserialization done sucessfully.");
    return RES_OK;
  }

}

/*-----------------------------------------------------------------------------------------------------


  \param root
  \param params

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _JSON_find_array(json_t  *root, json_t  **object, char const *key_name)
{
  json_t       *item;
  uint32_t      err = 1;
  uint32_t      n   = 0;
  uint32_t      i;

  err = 1;
  n   = json_array_size(root);
  // Проходим по всем элементам макссива root
  for (i = 0; i < n; i++)
  {
    item = json_array_get(root, i); // Получаем элемет массива
    *object = json_object_get(item, key_name); // Ищем в элементе массива объект с заданным именем
    // json_decref(item); Не декременитируем ссылку поскольку она  не инкрементировалась после вызова json_array_get
    if (json_is_array(*object))
    {
      err = 0;
      break;
    }
  }
  if (err == 1)
  {
    APPLOG("Error. Key %s", key_name);
    // json_decref(*object); Не декременитируем ссылку поскольку она  не инкрементировалась после вызова json_object_get
    return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param root

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static  uint32_t _JSON_Deser_params(const T_NV_parameters_instance  *p_pars, json_t  *root)
{
  json_t       *params = 0;
  json_t       *item   = 0;
  int32_t       indx;
  char         *var_name;
  char         *val;
  uint32_t      i;
  uint32_t      err;
  uint32_t      hits_cnt = 0;

  err = _JSON_find_array(root,&params, MAIN_PARAMETERS_KEY);
  if (err != RES_OK) return RES_ERROR;


  for (i = 0; i < json_array_size(params); i++)
  {
    item = json_array_get(params, i);
    if (json_is_array(item))
    {
      if (json_unpack(item, "[s,s]" ,&var_name,&val) == 0)
      {
        indx = Find_param_by_name(p_pars, var_name);
        if (indx >= 0)
        {
          Convert_str_to_parameter(p_pars, (uint8_t *)val , indx);
          hits_cnt++;
        }
      }
      else
      {
        APPLOG("Error.");
      }
    }
    // json_decref(item); Не декременитируем ссылку поскольку она  не инкрементировалась
  }
  if (hits_cnt > 0)
  {
    APPLOG("Updated %d params.",hits_cnt);
  }
  // json_decref(params); Не декременитируем ссылку поскольку она  не инкрементировалась
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param root

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static  uint32_t _JSON_Deser_DT(json_t  *root)
{
  json_t       *params = 0;
  json_t       *item   = 0;
  uint32_t      err = 0;
  rtc_time_t    rt_time = {0};

  err = _JSON_find_array(root,&params, DATETIME_SETTINGS_KEY);
  if (err != RES_OK) return RES_ERROR;


  item = json_array_get(params, 0);
  if (json_is_integer(item)) if (json_unpack(item, "i" ,&rt_time.tm_year) != 0) err++;
  // json_decref(item); Не декременитируем ссылку поскольку она  не инкрементировалась

  item = json_array_get(params, 1);
  if (json_is_integer(item)) if (json_unpack(item, "i" ,&rt_time.tm_mon) != 0) err++;
  // json_decref(item); Не декременитируем ссылку поскольку она  не инкрементировалась

  item = json_array_get(params, 2);
  if (json_is_integer(item)) if (json_unpack(item, "i" ,&rt_time.tm_mday) != 0) err++;
  // json_decref(item); Не декременитируем ссылку поскольку она  не инкрементировалась

  item = json_array_get(params, 3);
  if (json_is_integer(item)) if (json_unpack(item, "i" ,&rt_time.tm_wday) != 0) err++;
  // json_decref(item); Не декременитируем ссылку поскольку она  не инкрементировалась

  item = json_array_get(params, 4);
  if (json_is_integer(item)) if (json_unpack(item, "i" ,&rt_time.tm_hour) != 0) err++;
  // json_decref(item); Не декременитируем ссылку поскольку она  не инкрементировалась

  item = json_array_get(params, 5);
  if (json_is_integer(item)) if (json_unpack(item, "i" ,&rt_time.tm_min) != 0) err++;
  // json_decref(item); Не декременитируем ссылку поскольку она  не инкрементировалась

  item = json_array_get(params, 6);
  if (json_is_integer(item)) if (json_unpack(item, "i" ,&rt_time.tm_sec) != 0) err++;
  // json_decref(item); Не декременитируем ссылку поскольку она  не инкрементировалась

  // json_decref(params); Не декременитируем ссылку поскольку она  не инкрементировалась

  if (err == 0)
  {
    rt_time.tm_year = rt_time.tm_year - 1900;
    rt_time.tm_mon  = rt_time.tm_mon - 1;     // В языке С счет месяцев начинается с 0
    RTC_set_system_DateTime(&rt_time);
    return RES_OK;
  }
  else
  {
    APPLOG("Error.");
    return RES_ERROR;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Десериализация параметров из хранилища

  \param text

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t JSON_Deser_settings(const T_NV_parameters_instance  *p_pars, char *text)
{
  json_t       *root;
  json_error_t  error;
  uint32_t      res;

  root = json_loads(text, 0,&error);

  if (!root)
  {
    APPLOG("JSON decoding error: on line %d: %s", error.line, error.text);
    return RES_ERROR;
  }

  if (!json_is_array(root))
  {
    APPLOG("JSON decoding error: root is not array.");
    json_decref(root);
    return RES_ERROR;
  }

  res = JSON_Deser_tables(p_pars, root);
  json_decref(root);

  return res;

}

/*-----------------------------------------------------------------------------------------------------


  \param root
  \param object
  \param key_name

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t _JSON_find_cmd(json_t  *root, json_t  **object, char const *key_name)
{
  json_t       *item;
  uint32_t      err = 1;
  uint32_t      n   = 0;
  uint32_t      i;

  err = 1;
  n   = json_array_size(root);
  // Проходим по всем элементам макссива root
  for (i = 0; i < n; i++)
  {
    item = json_array_get(root, i); // Получаем элемет массива
    *object = json_object_get(item, key_name); // Ищем в элементе массива объект с заданным именем
    //json_decref(item); Не декременитируем ссылку поскольку она  не инкрементировалась после вызова json_array_get
    if (*object != NULL)
    {
      if (json_is_integer(*object))
      {
        err = 0;
        break;
      }
    }
  }
  if (err == 1)
  {
    //json_decref(*object); Не декременитируем ссылку поскольку она  не инкрементировалась после вызова json_object_get
    *object = NULL;
    return RES_ERROR;
  }
  return RES_OK;
}
/*-----------------------------------------------------------------------------------------------------
  Десериализация и выполнение команды содержимое которой из-за ее размера было сохранено в файл
  Вместо штатного парсера JSON производим собственный построчный разбор JSON структуры из файла
  Это команды:
    OPCODE_GET_FILES_BY_LIST
    OPCODE_DELETE_FILES_BY_LIST

  Список файлов из кодировки JSON переносится обычный список имен файлов заканчивающихся  STR_CRLF
  Этот спиок переносится в файл LIST_OF_FILENAMES и выдается сообщение задаче FTP_sender о необходимости выполнения операции



  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t JSON_Deser_long_command(void)
{
  FX_FILE        *file = NULL;
  char           *str  = NULL;
  int32_t        res;
  ULONG          actual_size;
  char           b;
  uint32_t       indx;
  uint32_t       opid;


  str =  App_malloc_pending(128, 10);
  if (str == NULL) goto EXIT_ON_ERROR;
  file = App_malloc_pending(sizeof(FX_FILE), 10);
  if (file == NULL) goto EXIT_ON_ERROR;
  // Открываем файл
  res = fx_file_open(&fat_fs_media, file, UNCOMPESSED_STREAM_FILE_NAME,  FX_OPEN_FOR_READ);
  if (res != FX_SUCCESS) goto EXIT_ON_ERROR;

  // Читаем стороку символов [{"OpCode":104}

  res= fx_file_read(file,(void *)str,11,&actual_size);
  if (res != FX_SUCCESS) goto EXIT_ON_ERROR;
  if (strncmp(str, "[{\"OpCode\":",11) != 0) goto EXIT_ON_ERROR;

  indx = 0;
  do
  {
    res= fx_file_read(file,(void *)&b,1,&actual_size);
    if (res != FX_SUCCESS) goto EXIT_ON_ERROR;
    str[indx] = b;
    indx++;

  } while ((isdigit(b) != 0) && (indx < 6)); // Up to 5 digits in OpCode are allowed
  if (b != '}') goto EXIT_ON_ERROR;
  str[indx] = 0;

  opid = atoi(str);

  if (!((opid == OPCODE_GET_FILES_BY_LIST) || (opid == OPCODE_DELETE_FILES_BY_LIST))) goto EXIT_ON_ERROR;

  // Читаем символы ,{"List":[
  //res= fx_file_read(file,(void *)str,10,&actual_size);
  //if (res != FX_SUCCESS) goto EXIT_ON_ERROR;
  //
  //if (str[9] != '[') goto EXIT_ON_ERROR;
  //
  //res = RES_ERROR;
  //
  //if (Is_FTP_sender_task_busy() == 0)
  //{
  //  res = MQTTMC_create_files_list_from_JSONchank(RECORDS_DIR_NAME, file);
  //  if (res == RES_OK)
  //  {
  //    if (opid == OPCODE_GET_FILES_BY_LIST)
  //    {
  //      FTP_sender_FLAG_START_FILES_SEND_BY_LIST();
  //    }
  //    else if (opid == OPCODE_DELETE_FILES_BY_LIST)
  //    {
  //      FTP_sender_FLAG_DELETE_FILES_BY_LIST();
  //    }
  //  }
  //}

  MQTTMC_Send_Ack(res);
  fx_file_close(file);
  App_free(str);
  App_free(file);
  return RES_OK;

EXIT_ON_ERROR:
  if (file != 0)
  {
    if (file->fx_file_id == FX_FILE_ID)
    {
      fx_file_close(file);
    }
    App_free(file);
  }
  App_free(str);
  return RES_ERROR;
}
/*-----------------------------------------------------------------------------------------------------
  Декодирование и сохранение параметров из JSON

  \param json

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t JSON_Deser_and_Exec_command(char *text, uint32_t f_long_stream)
{
  json_t       *root;
  json_error_t  error;
  json_t       *opc;
  uint32_t      opcode;
  uint32_t      res = RES_ERROR;


  if (f_long_stream)
  {
    if  (JSON_Deser_long_command() == RES_OK)
    {
      MQTTMC_Send_Ack(RES_OK);
      return RES_OK;
    }
    else
    {
      MQTTMC_Send_Ack(RES_ERROR);
      return RES_ERROR;
    }
  }

  root = json_loads(text, 0,&error);

  if (!root)
  {
    APPLOG("JSON decoding error: on line %d: %s", error.line, error.text);
    return RES_ERROR;
  }

  if (!json_is_array(root))
  {
    APPLOG("JSON decoding error: root is not array.");
    json_decref(root);
    return RES_ERROR;
  }

  res = _JSON_find_cmd(root,&opc, COMMAND_KEY);
  if (res != RES_OK)
  {
    APPLOG("JSON decoding error: 'OpCode' key is not a string.");
    json_decref(root);
    return RES_ERROR;
  }
  opcode = json_integer_value(opc);

  res = RES_ERROR;
  switch (opcode)
  {
  case OPCODE_SAVE_PARAMETERS_TO_RAM:
    APPLOG("MQTT command: SAVE_PARAMETERS_TO_RAM");
    res = JSON_Deser_tables(&wvars_inst, root);
    MQTTMC_Send_Ack(res);
    break;
  case OPCODE_SAVE_PARAMETERS_TO_NV:
    APPLOG("MQTT command: SAVE_PARAMETERS_TO_NV");
    res = JSON_Deser_tables(&wvars_inst,root);
    MQTTMC_Send_Ack(res);
    if (res == RES_OK) Request_save_app_settings();
    break;


  default:
    MQTTMC_set_opcode(opcode); // Записываем код команды для обработки в главной процедуре. Так сделано для того чтобы при обработке не осталось захваченных парсером ресурсов памяти
    res = RES_OK;
    break;
  }
  json_decref(root);
  return res;

}


