// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.08.28
// 18:55:03
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

#define JSON_DECREF(x)     {if (x!=0) {json_decref(x);x=0;}}


#define CLEAR_DEVDESCR_SERIALIZER  {JSON_DECREF(item_obj);JSON_DECREF(main_obj)}
#define SERIALIZE_DEVDESCR_ERROR    {APPLOG("json error.");CLEAR_DEVDESCR_SERIALIZER;return RES_ERROR;}

/*-----------------------------------------------------------------------------------------------------


  \param main_obj
  \param obj_name

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Serialize_device_description_to_obj(json_t *obj, char *obj_name)
{
  json_t    *main_obj = 0;
  json_t    *item_obj = 0;

  item_obj = json_object();
  if (!item_obj) SERIALIZE_DEVDESCR_ERROR;

  if (json_object_set_new(item_obj, "CPU_ID", json_string(g_cpu_id_str)) != 0) SERIALIZE_DEVDESCR_ERROR;
  if (json_object_set_new(item_obj, "SW_Ver", json_string((char*)wvar.manuf_date)) != 0) SERIALIZE_DEVDESCR_ERROR;
  if (json_object_set_new(item_obj, "HW_Ver", json_string((char*)wvar.name)) != 0) SERIALIZE_DEVDESCR_ERROR;
  if (json_object_set_new(item_obj, "CompDate", json_string(__DATE__)) != 0) SERIALIZE_DEVDESCR_ERROR;
  if (json_object_set_new(item_obj, "CompTime", json_string(__TIME__)) != 0) SERIALIZE_DEVDESCR_ERROR;

  if (json_object_set(obj,obj_name, item_obj) != 0) SERIALIZE_DEVDESCR_ERROR;
  JSON_DECREF(item_obj);

  CLEAR_DEVDESCR_SERIALIZER;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param p_file
  \param flags
  \param obj_name

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Serialize_device_description_to_file(FX_FILE *p_file, size_t flags, char *obj_name)
{
  json_t    *main_obj = 0;
  json_t    *item_obj = 0;

  main_obj = json_object();
  if (!main_obj) SERIALIZE_DEVDESCR_ERROR;

  if (Serialize_device_description_to_obj(main_obj, obj_name) != RES_OK) SERIALIZE_DEVDESCR_ERROR;

  // Записываем созданный объект и закрывавем его чтобы освободить динамическую память
  if (json_dumpf(main_obj,p_file, flags) != 0) SERIALIZE_DEVDESCR_ERROR;
  JSON_DECREF(main_obj);

  CLEAR_DEVDESCR_SERIALIZER;
  return RES_OK;
}
#undef CLEAR_DEVDESCR_SERIALIZER
#undef SERIALIZE_DEVDESCR_ERROR


#define CLEAR_PARAM_SERIALIZER  {App_free(str);JSON_DECREF(item_obj);JSON_DECREF(jarray); JSON_DECREF(main_obj)}
#define SERIALIZE_PARS_ERROR    {APPLOG("json error. Indx %d ",i);CLEAR_PARAM_SERIALIZER;return RES_ERROR;}
/*-----------------------------------------------------------------------------------------------------


  \param obj
  \param obj_name

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Serialize_main_params_schema_to_obj(const T_NV_parameters_instance *p_pars, json_t *obj, char *obj_name)
{
  int32_t                   i = 0;
  const T_NV_parameters    *pp;
  char                     *str      = 0;
  json_t                   *main_obj = 0;
  json_t                   *jarray   = 0;
  json_t                   *item_obj = 0;

  str = App_malloc_pending(MAX_PARAMETER_STRING_LEN, 10);
  if (str == NULL) SERIALIZE_PARS_ERROR;
  jarray = json_array();
  if (!jarray) SERIALIZE_PARS_ERROR;

  // Создаем массив описывающий  параметры

  for (i=0; i < p_pars->items_num; i++)
  {
    pp = &p_pars->items_array[i];
    item_obj = json_object();
    if (json_object_set_new(item_obj, "name", json_string((char const *)pp->var_name)) != 0) SERIALIZE_PARS_ERROR;
    if (json_object_set_new(item_obj, "alias", json_string((char const *)pp->var_alias)) != 0) SERIALIZE_PARS_ERROR;
    if (json_object_set_new(item_obj, "descr", json_string((char const *)pp->var_description)) != 0) SERIALIZE_PARS_ERROR;

    Convert_parameter_to_str(p_pars, (uint8_t *)str, MAX_PARAMETER_STRING_LEN, i);
    if (json_object_set_new(item_obj, "value", json_string(str)) != 0) SERIALIZE_PARS_ERROR;
    if (json_object_set_new(item_obj, "type", json_string(Convrt_var_type_to_str(pp->vartype))) != 0) SERIALIZE_PARS_ERROR;
    if (json_object_set_new(item_obj, "max_len",  json_integer(pp->varlen)) != 0) SERIALIZE_PARS_ERROR;
    if (json_object_set_new(item_obj, "max_value",  json_real((double)pp->maxval)) != 0) SERIALIZE_PARS_ERROR;
    if (json_object_set_new(item_obj, "min_value",  json_real((double)pp->minval)) != 0) SERIALIZE_PARS_ERROR;
    if (json_object_set_new(item_obj, "c_format",    json_string(pp->format)) != 0) SERIALIZE_PARS_ERROR;
    if (json_object_set_new(item_obj, "attributes", json_integer(pp->attr)) != 0) SERIALIZE_PARS_ERROR;
    if (json_object_set_new(item_obj, "level",      json_integer(pp->parmnlev)) != 0) SERIALIZE_PARS_ERROR;
    if (json_object_set_new(item_obj, "num",   json_integer(pp->menu_pos)) != 0) SERIALIZE_PARS_ERROR;
    if (json_object_set_new(item_obj, "selector",   json_integer(pp->selector_id)) != 0) SERIALIZE_PARS_ERROR;


    if (json_array_insert(jarray, i, item_obj) != 0) SERIALIZE_PARS_ERROR;
    JSON_DECREF(item_obj);
  }
  if (json_object_set(obj,obj_name, jarray) != 0) SERIALIZE_PARS_ERROR;
  JSON_DECREF(jarray);
  CLEAR_PARAM_SERIALIZER;
  return RES_OK;
}

#undef SERIALIZE_PARS_ERROR
#define SERIALIZE_PARS_ERROR    {APPLOG("json error.");CLEAR_PARAM_SERIALIZER;return RES_ERROR;}
/*-----------------------------------------------------------------------------------------------------


  \param p_file
  \param flags

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Serialize_main_params_schema_to_file(const T_NV_parameters_instance *p_pars, FX_FILE *p_file, size_t flags, char *obj_name)
{
  char      *str      = 0;
  json_t    *main_obj = 0;
  json_t    *jarray   = 0;
  json_t    *item_obj = 0;


  main_obj = json_object();
  if (!main_obj) SERIALIZE_PARS_ERROR;

  if (Serialize_main_params_schema_to_obj(p_pars, main_obj, obj_name) != RES_OK) SERIALIZE_PARS_ERROR;
  // Записываем созданный объект и закрывавем его чтобы освободить динамическую память
  if (json_dumpf(main_obj,p_file, flags) != 0) SERIALIZE_PARS_ERROR;
  JSON_DECREF(main_obj);

  CLEAR_PARAM_SERIALIZER;
  return RES_OK;
}
#undef SERIALIZE_PARS_ERROR
#undef CLEAR_PARAM_SERIALIZER

#define CLEAR_PARAM_VALS_SERIALIZER  {App_free(str);JSON_DECREF(item_obj);JSON_DECREF(jarray); JSON_DECREF(main_obj)}
#define SERIALIZE_PARS_VALS_ERROR    {APPLOG("json error.");CLEAR_PARAM_VALS_SERIALIZER;return RES_ERROR;}
/*-----------------------------------------------------------------------------------------------------


  \param p_file
  \param flags
  \param obj_name

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Serialize_params_vals_to_obj(const T_NV_parameters_instance *p_pars, json_t *obj, char *obj_name)
{
  const T_NV_parameters    *pp;
  char                     *str      = 0;
  json_t                   *main_obj = 0;
  json_t                   *jarray   = 0;
  json_t                   *item_obj = 0;

  str = App_malloc_pending(MAX_PARAMETER_STRING_LEN,0);
  if (str == NULL) SERIALIZE_PARS_VALS_ERROR;

  jarray = json_array();
  if (!jarray) SERIALIZE_PARS_VALS_ERROR;

  // Создаем массив описывающий  параметры

  for (int32_t i=0; i < p_pars->items_num; i++)
  {
    pp = &p_pars->items_array[i];
    item_obj = json_array();
    if (json_array_append_new(item_obj, json_string((char const *)pp->var_name)) != 0) SERIALIZE_PARS_VALS_ERROR;

    Convert_parameter_to_str(p_pars, (uint8_t *)str, MAX_PARAMETER_STRING_LEN, i);
    if (json_array_append_new(item_obj, json_string(str)) != 0) SERIALIZE_PARS_VALS_ERROR;

    if (json_array_insert(jarray, i, item_obj) != 0) SERIALIZE_PARS_VALS_ERROR;
    JSON_DECREF(item_obj);
  }
  if (json_object_set(obj, obj_name, jarray) != 0) SERIALIZE_PARS_VALS_ERROR;
  JSON_DECREF(jarray);

  CLEAR_PARAM_VALS_SERIALIZER;
  return RES_OK;
}
/*-----------------------------------------------------------------------------------------------------


  \param p_file
  \param flags

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Serialize_params_vals_to_file(const T_NV_parameters_instance *p_pars, FX_FILE *p_file, size_t flags, char *obj_name)
{
  char      *str      = 0;
  json_t    *main_obj = 0;
  json_t    *jarray   = 0;
  json_t    *item_obj = 0;



  main_obj = json_object();
  if (!main_obj) SERIALIZE_PARS_VALS_ERROR;

  if (Serialize_params_vals_to_obj(p_pars, main_obj, obj_name) != RES_OK) SERIALIZE_PARS_VALS_ERROR;
  // Записываем созданный объект и закрывавем его чтобы освободить динамическую память
  if (json_dumpf(main_obj,p_file, flags) != 0) SERIALIZE_PARS_VALS_ERROR;
  JSON_DECREF(main_obj);

  CLEAR_PARAM_VALS_SERIALIZER;
  return RES_OK;
}
#undef SERIALIZE_PARS_VALS_ERROR
#undef CLEAR_PARAM_VALS_SERIALIZER


#define CLEAR_PARAMS_TREE_SERIALIZER  {JSON_DECREF(item_obj);JSON_DECREF(jarray); JSON_DECREF(main_obj)}
#define SERIALIZE_PARAMS_TREE_ERROR    {APPLOG("json error.");CLEAR_PARAMS_TREE_SERIALIZER;return RES_ERROR;}
/*-----------------------------------------------------------------------------------------------------


  \param obj
  \param obj_name

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Serialize_params_tree_to_obj(const T_NV_parameters_instance *p_pars, json_t *obj, char *obj_name)
{
  const T_parmenu          *pm;
  json_t                   *main_obj = 0;
  json_t                   *jarray   = 0;
  json_t                   *item_obj = 0;

  jarray = json_array();
  if (!jarray) SERIALIZE_PARAMS_TREE_ERROR;

  // Создаем массив описывающий дерево параметров

  for (int32_t i=0; i < p_pars->menu_items_num; i++)
  {
    pm = &p_pars->menu_items_array[i];
    item_obj = json_object();
    if (json_object_set_new(item_obj, "level",  json_integer(pm->currlev)) != 0) SERIALIZE_PARAMS_TREE_ERROR;
    if (json_object_set_new(item_obj, "parent",  json_integer(pm->prevlev)) != 0) SERIALIZE_PARAMS_TREE_ERROR;
    if (json_object_set_new(item_obj, "name",  json_string(pm->name)) != 0) SERIALIZE_PARAMS_TREE_ERROR;
    if (json_object_set_new(item_obj, "attributes",  json_integer(pm->visible)) != 0) SERIALIZE_PARAMS_TREE_ERROR;

    if (json_array_insert(jarray, i, item_obj) != 0) SERIALIZE_PARAMS_TREE_ERROR;
    JSON_DECREF(item_obj);

  }
  if (json_object_set(obj,obj_name, jarray) != 0) SERIALIZE_PARAMS_TREE_ERROR;
  JSON_DECREF(jarray);


  CLEAR_PARAMS_TREE_SERIALIZER
  return RES_OK;
}
/*-----------------------------------------------------------------------------------------------------


  \param p_file
  \param flags

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Serialize_params_tree_to_file(const T_NV_parameters_instance *p_pars, FX_FILE *p_file, size_t flags, char *obj_name)
{
  json_t    *main_obj = 0;
  json_t    *jarray   = 0;
  json_t    *item_obj = 0;

  main_obj = json_object();
  if (!main_obj) SERIALIZE_PARAMS_TREE_ERROR;

  if (Serialize_params_tree_to_obj(p_pars, main_obj, obj_name) != RES_OK) SERIALIZE_PARAMS_TREE_ERROR;
  // Записываем созданный объект и закрывавем его чтобы освободить динамическую память
  if (json_dumpf(main_obj,p_file, flags) != 0) SERIALIZE_PARAMS_TREE_ERROR;
  JSON_DECREF(main_obj);


  CLEAR_PARAMS_TREE_SERIALIZER
  return RES_OK;
}
#undef CLEAR_PARAMS_TREE_SERIALIZER
#undef SERIALIZE_PARAMS_TREE_ERROR


#define CLEAR_SEL_SERIALIZER  {JSON_DECREF(jarray_items);JSON_DECREF(item_obj1);JSON_DECREF(item_obj2);JSON_DECREF(jarray); JSON_DECREF(main_obj)}
#define SERIALIZE_SEL_ERROR    {APPLOG("json error.");CLEAR_SEL_SERIALIZER;return RES_ERROR;}
/*-----------------------------------------------------------------------------------------------------


  \param obj
  \param obj_name

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Serialize_selectors_to_obj(const T_NV_parameters_instance *p_pars, json_t *obj, char *obj_name)
{
  const T_selectors_list  *sl;

  json_t *main_obj     = 0;
  json_t *jarray       = 0;
  json_t *item_obj1    = 0;
  json_t *item_obj2    = 0;
  json_t *jarray_items = 0;

  jarray = json_array();
  if (!jarray) SERIALIZE_SEL_ERROR;

  for (int32_t i=0; i < p_pars->selectors_num; i++)
  {
    sl = &p_pars->selectors_array[i];
    item_obj1 = json_object();
    if (!item_obj1) SERIALIZE_SEL_ERROR;

    if (json_object_set_new(item_obj1, "name",  json_string((char const *)sl->name)) != 0) SERIALIZE_SEL_ERROR;
    int32_t num = sl->items_cnt;
    if (json_object_set_new(item_obj1, "count",  json_integer(num)) != 0) SERIALIZE_SEL_ERROR;
    if (num > 0)
    {
      // Вставляем массив значений и названий пунктов селектора
      jarray_items = json_array();
      if (!jarray_items) SERIALIZE_SEL_ERROR;

      for (int32_t j=0; j < num; j++)
      {
        item_obj2 = json_object();
        if (!item_obj2) SERIALIZE_SEL_ERROR;

        if (json_object_set_new(item_obj2, "v",  json_integer(sl->items_list[j].val)) != 0) SERIALIZE_SEL_ERROR;
        if (json_object_set_new(item_obj2, "n",  json_string((char const *)sl->items_list[j].caption)) != 0) SERIALIZE_SEL_ERROR;

        json_array_insert(jarray_items, j, item_obj2);
        JSON_DECREF(item_obj2);
      }
      json_object_set(item_obj1,"items", jarray_items);
      JSON_DECREF(jarray_items);


      uint8_t all_same = 1;

      // Преверяем все ли индексы рисунков одинаковы
      int32_t first = sl->items_list[0].img_indx;
      for (int32_t j=0; j < num; j++)
      {
        if (sl->items_list[j].img_indx != first)
        {
          all_same = 0;
          break;
        }
      }

      if (all_same == 0)
      {

        // Вставляем массив индексов пиктограмм пунктов селектора если есть разные индексы
        jarray_items = json_array();
        if (!jarray_items) SERIALIZE_SEL_ERROR;

        for (int32_t j=0; j < num; j++)
        {
          if (json_array_insert_new(jarray_items, j, json_integer(sl->items_list[j].img_indx)) != 0) SERIALIZE_SEL_ERROR;
        }
        json_object_set(item_obj1,"imgs", jarray_items);
        JSON_DECREF(jarray_items);
      }
      else
      {
        // Если все индексы одинаковы то вставляет только один объект индекса
        if (json_object_set_new(item_obj1,"imgs", json_integer(first)) != 0) SERIALIZE_SEL_ERROR;
      }

    }
    if (json_array_insert(jarray, i, item_obj1) != 0) SERIALIZE_SEL_ERROR;
    JSON_DECREF(item_obj1);
  }
  if (json_object_set(obj, obj_name, jarray) != 0) SERIALIZE_SEL_ERROR;
  JSON_DECREF(jarray);


  CLEAR_SEL_SERIALIZER;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t Serialize_selectors_to_file(const T_NV_parameters_instance *p_pars, FX_FILE *p_file, size_t flags, char *obj_name)
{
  json_t *main_obj     = 0;
  json_t *jarray       = 0;
  json_t *item_obj1    = 0;
  json_t *item_obj2    = 0;
  json_t *jarray_items = 0;

  main_obj = json_object();
  if (!main_obj) SERIALIZE_SEL_ERROR;

  if (Serialize_selectors_to_obj(p_pars, main_obj, obj_name) != RES_OK) SERIALIZE_SEL_ERROR;
  // Записываем созданный объект и закрывавем его чтобы освободить динамическую память
  if (json_dumpf(main_obj,p_file, flags) != 0) SERIALIZE_SEL_ERROR;
  JSON_DECREF(main_obj);

  CLEAR_SEL_SERIALIZER;
  return RES_OK;
}
#undef SERIALIZE_SEL_ERROR
#undef CLEAR_SEL_SERIALIZER


/*-----------------------------------------------------------------------------------------------------


  \param p_file
  \param flags
  \param str
-----------------------------------------------------------------------------------------------------*/
uint32_t  Write_json_str_to_file(FX_FILE *p_file, size_t flags, char *str)
{
  uint32_t sz;
  sz = strlen(str);
  if (fx_file_write(p_file, str, sz) != FX_SUCCESS) return RES_ERROR;
  if ((flags & JSON_MAX_INDENT) != 0)
  {
    if (fx_file_write(p_file, "\n", 1) != FX_SUCCESS) return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param filename

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Serialze_settings_schema_to_JSON_file(const T_NV_parameters_instance *p_pars, char *filename, size_t flags)
{
  uint32_t             res;
  FX_FILE              f;

  // Открываем файл на запись
  res = Recreate_file_for_write(&f, (CHAR *)filename);
  if (res != FX_SUCCESS)
  {
    APPLOG("Error %d",res);
    return RES_ERROR;
  }

  do
  {
    res = RES_ERROR;

    if (Write_json_str_to_file(&f,flags,"[") != RES_OK) break;

    // Создаем JSON c массивом описывающим параметры
    if (Serialize_device_description_to_file(&f, flags, DEVICE_HEADER_KEY) != RES_OK) break;
    if (Write_json_str_to_file(&f,flags,",") != RES_OK) break;

    if (Serialize_main_params_schema_to_file(p_pars, &f, flags, MAIN_PARAMETERS_KEY) != RES_OK) break;
    if (Write_json_str_to_file(&f,flags,",") != RES_OK) break;

    if (Serialize_params_tree_to_file(p_pars, &f, flags, PARAMETERS_TREE_KEY) != RES_OK) break;
    if (Write_json_str_to_file(&f,flags,",") != RES_OK) break;

    // Создаем JSON c массивом описывающим селекторы в параметрах
    if (Serialize_selectors_to_file(p_pars, &f, flags, "Selectors") != RES_OK) break;

    //if (Serialize_din_list(&f, flags, TYPE_RECORDS_LIST, "RECORDS_LIST") != RES_OK) break;
    if (Write_json_str_to_file(&f,flags,"]") != RES_OK) break;

    res = RES_OK;
    break;

  } while (0);


  if (fx_file_close(&f) != RES_OK) return RES_ERROR;

  return res;
}

/*-----------------------------------------------------------------------------------------------------


  \param filename

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Serialze_settings_to_JSON_file(const T_NV_parameters_instance *p_pars, char *filename, size_t flags)
{
  uint32_t             res;
  FX_FILE              f;

  // Открываем файл на запись
  res = Recreate_file_for_write(&f, (CHAR *)filename);
  if (res != FX_SUCCESS)
  {
    APPLOG("json error.");
    return res;
  }

  res = RES_ERROR;
  Write_json_str_to_file(&f,flags,"[");

  do
  {
    // Создаем JSON c массивом описывающим параметры
    if (Serialize_device_description_to_file(&f, flags, DEVICE_HEADER_KEY) != RES_OK) break;
    if (Write_json_str_to_file(&f,flags,",") != RES_OK) break;

    if (Serialize_params_vals_to_file(p_pars, &f, flags, MAIN_PARAMETERS_KEY) != RES_OK) break;
    if (Write_json_str_to_file(&f,flags,",") != RES_OK) break;

    res = RES_OK;
    break;

  } while (0);

  Write_json_str_to_file(&f,flags,"]");

  fx_file_close(&f);
  return RES_OK;
}


#define CLEAR_VAL_TO_MEM_SERIALIZER             {JSON_DECREF(main_arr);JSON_DECREF(arr_item)}
#define SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR    {APPLOG("json error.");App_free(ptr);CLEAR_VAL_TO_MEM_SERIALIZER;return RES_ERROR;}
/*-----------------------------------------------------------------------------------------------------
  Сериализвать параметры в память
  Применяется при редактировании установок и их сохранении

  \param mem

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Serialze_settings_to_mem(const T_NV_parameters_instance *p_pars, char **mem, uint32_t *str_size, uint32_t  flags)
{
  char      *ptr      = NULL;
  json_t    *main_arr = 0;
  json_t    *arr_item = 0;


  main_arr = json_array();
  if (!main_arr) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;

  arr_item = json_object();
  if (!arr_item) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;
  if (Serialize_device_description_to_obj(arr_item, DEVICE_HEADER_KEY) != RES_OK) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;
  if (json_array_append(main_arr, arr_item) != 0) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;
  JSON_DECREF(arr_item);

  arr_item = json_object();
  if (!arr_item) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;
  if (Serialize_params_vals_to_obj(p_pars, arr_item, MAIN_PARAMETERS_KEY) != RES_OK) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;
  if (json_array_append(main_arr, arr_item) != 0) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;
  JSON_DECREF(arr_item);

  // Подсчитать сколько места займет строка после сериализации
  size_t size = json_dumpb(main_arr, NULL, 0, flags);
  if (size == 0) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;

  *mem = NULL;
  // Выделить необходимый объем памяти
  ptr = App_malloc_pending(size,10);
  if (ptr == NULL) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;

  // Сериализировать
  size = json_dumpb(main_arr, ptr, size, flags);

  *str_size = size;
  *mem = ptr;
  CLEAR_VAL_TO_MEM_SERIALIZER;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------
  Сериализвать текущее состояние устройства в память

  \param mem

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Serialze_device_state_to_mem(char **mem, uint32_t *str_size)
{
  char      *ptr      = NULL;
  json_t    *main_arr = 0;
  json_t    *arr_item = 0;
  uint32_t  flags;


  main_arr = json_array();
  if (!main_arr) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;

  arr_item = json_object();
  if (!arr_item) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;
  if (Serialize_device_description_to_obj(arr_item, DEVICE_HEADER_KEY) != RES_OK) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;
  if (json_array_append(main_arr, arr_item) != 0) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;
  JSON_DECREF(arr_item);

  arr_item = json_object();


  arr_item = json_object();
  uint32_t up_time = _tx_time_get() / TX_TIMER_TICKS_PER_SECOND;
  if (json_object_set_new(arr_item, "UpTime_sec", json_integer(up_time)) != 0) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;
  if (json_array_append(main_arr, arr_item) != 0) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;
  JSON_DECREF(arr_item);


  flags = JSON_ENSURE_ASCII | JSON_COMPACT;

  // Подсчитать сколько места займет строка после сериализации
  size_t size = json_dumpb(main_arr, NULL, 0, flags);
  if (size == 0) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;

  // Выделить необходимый объем памяти
  *mem = NULL;
  ptr = App_malloc_pending(size,10);
  if (ptr == NULL) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;

  // Сериализировать
  size = json_dumpb(main_arr, ptr, size, flags);

  *str_size = size;
  *mem = ptr;
  CLEAR_VAL_TO_MEM_SERIALIZER;
  return RES_OK;

}

/*-----------------------------------------------------------------------------------------------------


  \param mem
  \param str_size
  \param ack_code       0 - команда выпонена

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Serialze_Ack_message(char **mem, uint32_t *str_size, uint32_t ack_code, const char *ack_str)
{
  char      *ptr      = NULL;
  json_t    *main_arr = 0;
  json_t    *arr_item = 0;
  uint32_t  flags;


  main_arr = json_array();
  if (!main_arr) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;

  arr_item = json_object();
  if (!arr_item) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;
  if (Serialize_device_description_to_obj(arr_item, DEVICE_HEADER_KEY) != RES_OK) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;
  if (json_array_append(main_arr, arr_item) != 0) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;
  JSON_DECREF(arr_item);


  arr_item = json_object();
  uint32_t up_time = _tx_time_get() / TX_TIMER_TICKS_PER_SECOND;
  if (json_object_set_new(arr_item, ack_str, json_integer(ack_code)) != 0) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;
  if (json_array_append(main_arr, arr_item) != 0) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;
  JSON_DECREF(arr_item);

  flags = JSON_ENSURE_ASCII | JSON_COMPACT;

  // Подсчитать сколько места займет строка после сериализации
  size_t size = json_dumpb(main_arr, NULL, 0, flags);
  if (size == 0) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;

  // Выделить необходимый объем памяти
  *mem = NULL;
  ptr = App_malloc_pending(size,10);
  if (ptr == NULL) SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR;

  // Сериализировать
  size = json_dumpb(main_arr, ptr, size, flags);

  *str_size = size;
  *mem = ptr;
  CLEAR_VAL_TO_MEM_SERIALIZER;
  return RES_OK;
}

#undef CLEAR_VAL_TO_MEM_SERIALIZER
#undef SERIALIZE_PARAMS_VALUES_TO_MEM_ERROR




