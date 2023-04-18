#include   "MC50.h"

#define   TMP_BUF_SZ        512

#define   MAX_VALUE_STR_SZ  100

#define MTYPE_NONE          0
#define MTYPE_SUBMENU       1
#define MTYPE_SUBTABLE      2
#define MTYPE_PARAMETER     3


static const char *PARAM_EDITOR_HELP = "\033[5C Press digit key to select menu item.\r\n"
                                      "\033[5C <M> - Main menu, <R> - return on prev. level\r\n"
                                      "\033[5C Enter - Accept, Esc - Cancel, ^[H - erase\r\n";


static void     _Goto_to_edit_param(void);
static uint8_t* _Get_mn_caption(void);
static uint8_t  _Get_mn_prevlev(void);

/*-------------------------------------------------------------------------------------------
  Найти название меню по его идентификатору
---------------------------------------------------------------------------------------------*/
static uint8_t* _Get_mn_caption(void)
{
  int i;
  GET_MCBL;
  const T_NV_parameters_instance  *p_pars = mcbl->p_pars;

  for (i = 0; i < p_pars->menu_items_num; i++)
  {
    if (p_pars->menu_items_array[i].currlev == mcbl->current_level) return(uint8_t *)p_pars->menu_items_array[i].name;
  }
  return(uint8_t *)p_pars->menu_items_array[0].name;
}




/*-------------------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------*/
static uint8_t _Get_mn_prevlev(void)
{
  int i;
  GET_MCBL;
  const T_NV_parameters_instance  *p_pars = mcbl->p_pars;

  for (i = 0; i < p_pars->menu_items_num; i++)
  {
    if (p_pars->menu_items_array[i].currlev == mcbl->current_level) return (p_pars->menu_items_array[i].prevlev);
  }
  return (MAIN_PARAMS_ROOT);
}


/*-------------------------------------------------------------------------------------------
  Вывод на экран
---------------------------------------------------------------------------------------------*/
void Show_parameters_menu(void)
{
  int               i;
  uint32_t          n;
  uint8_t           str[MAX_VALUE_STR_SZ];
  uint8_t          *st;
  GET_MCBL;
  const T_NV_parameters_instance  *p_pars = mcbl->p_pars;

  MPRINTF(VT100_CLEAR_AND_HOME);
  // Вывод заголовка меню параметров
  st = _Get_mn_caption();
  VT100_send_str_to_pos(st, 1, Find_str_center(st));
  VT100_send_str_to_pos(DASH_LINE, 2, 0);
  VT100_send_str_to_pos((uint8_t *)PARAM_EDITOR_HELP, 3, 0);
  MPRINTF("\r\n");
  MPRINTF(DASH_LINE);

  //..........................................
  // Вывести список субменю на данном уровне
  //..........................................
  n = 0;
  for (i = 0; i < p_pars->menu_items_num; i++)
  {
    if (p_pars->menu_items_array[i].prevlev != mcbl->current_level) continue;
    sprintf((char *)str, "%1X - %s", n, p_pars->menu_items_array[i].name);

    if ((strlen((char *)str)+ SCR_ITEMS_HOR_OFFS) > COLCOUNT)
    {
      str[COLCOUNT - SCR_ITEMS_HOR_OFFS - 3] = '.';
      str[COLCOUNT - SCR_ITEMS_HOR_OFFS - 2] = '>';
      str[COLCOUNT - SCR_ITEMS_HOR_OFFS - 1] = 0;
    }
    VT100_send_str_to_pos(str, SCR_ITEMS_VERT_OFFS + n, SCR_ITEMS_HOR_OFFS);
    mcbl->item_indexes[n] = i;
    n++;
  }
  mcbl->current_menu_submenus_count = n;

  //..........................................
  // Вывести список всех параметров на данном уровне
  //..........................................
  for (i = 0; i < p_pars->items_num; i++)
  {
    int len;
    if (p_pars->items_array[i].parmnlev != mcbl->current_level) continue;
    VT100_set_cursor_pos(SCR_ITEMS_VERT_OFFS + n, SCR_ITEMS_HOR_OFFS);
    sprintf((char *)str, "%1X - ", n);
    MPRINTF((char *)str);
    len = strlen((char *)str)+ SCR_ITEMS_HOR_OFFS;
    MPRINTF(VT100_REVERSE_ON);
    MPRINTF((char *)p_pars->items_array[i].var_description);
    MPRINTF("= ");
    len = len + strlen((char *)p_pars->items_array[i].var_description)+ 2;
    MPRINTF(VT100_REVERSE_OFF);
    // Преобразовать параметр в строку
    Convert_parameter_to_str(p_pars, str, MAX_VALUE_STR_SZ-1, i);
    if ((strlen((char *)str)+ len) > COLCOUNT)
    {
      str[COLCOUNT - len - 3] = '.';
      str[COLCOUNT - len - 2] = '>';
      str[COLCOUNT - len - 1] = 0;
    }
    MPRINTF((char *)str);
    mcbl->item_indexes[n] = i;
    n++;
  }
  mcbl->current_menu_items_count = n;

}

/*-------------------------------------------------------------------------------------------
  Функция периодически вызываемая в качестве обработчика нажатий из главного цикла монитора

  Принимаются коды клавиш:
    от 0 до F - вызывают соответствующий пункт меню
    M и R     - вызываю переход на другой уровень меню
---------------------------------------------------------------------------------------------*/
void Params_editor_press_key_handler(uint8_t b)
{
  GET_MCBL;
  const T_NV_parameters_instance  *p_pars = mcbl->p_pars;

  if (b < 0x30) return; // Игнорируем служебные коды

  if ((b == 'M') || (b == 'm'))
  {
    Goto_main_menu();
    return;
  }
  if ((b == 'R') || (b == 'r'))
  {
    if (mcbl->current_level != MAIN_PARAMS_ROOT)
    {
      mcbl->current_level = _Get_mn_prevlev();
      Show_parameters_menu();
      return;
    }
    else
    {
      // Выход на пункт меню из которого вошли в редактор параметров
      Return_to_prev_menu();
      Display_menu();
      return;
    }

  }

  b = ascii_to_hex(b); // Из кода клавиши получаем 16-и ричную цифру
  if (b >= MAX_ITEMS_COUNT) return;


  if (b >= mcbl->current_menu_items_count) return; // Выход если код меньше чем есть в ниличии пунктов

  // Определяем к какому типу пункта меню относится код клавиши
  uint8_t mtype = MTYPE_NONE;

  if  (b < mcbl->current_menu_submenus_count)
  {
    mtype = MTYPE_SUBMENU;
  }
  else
  {
    mtype = MTYPE_PARAMETER;
  }


  switch (mtype)
  {
  case MTYPE_SUBMENU:

    // Выбран пункт субменю
    mcbl->current_level = p_pars->menu_items_array[mcbl->item_indexes[b]].currlev;
    Show_parameters_menu();
    break;

  case MTYPE_PARAMETER:

    // Выбран параметр для редактирования
    mcbl->current_parameter_indx = mcbl->item_indexes[b];
    _Goto_to_edit_param();
    break;
  }
}



/*-------------------------------------------------------------------------------------------
  Переход к редактированиб параметра

  Параметры:
             n    - индекс параметра в массиве параметров
             item - номер параметра в меню
---------------------------------------------------------------------------------------------*/
static void _Goto_to_edit_param(void)
{
  int n;
  GET_MCBL;
  const T_NV_parameters_instance  *p_pars = mcbl->p_pars;

  mcbl->rowcount =(MAX_PARAMETER_STRING_LEN + 1) / COLCOUNT;
  if (((MAX_PARAMETER_STRING_LEN + 1)% COLCOUNT) > 0) mcbl->rowcount++;
  if (mcbl->rowcount > ROWCOUNT - 2) return;  // Слишком большая область редактирования
  mcbl->firstrow = ROWCOUNT - mcbl->rowcount + 1;
  VT100_set_cursor_pos(mcbl->firstrow - 2, 0);
  MPRINTF("Edited parameter: '%s'\r\n", p_pars->items_array[mcbl->current_parameter_indx].var_description);
  MPRINTF(DASH_LINE);
  Convert_parameter_to_str(p_pars, mcbl->param_str, MAX_PARAMETER_STRING_LEN, mcbl->current_parameter_indx);
  VT100_set_cursor_pos(mcbl->firstrow, 0);
  MPRINTF(VT100_CLR_FM_CRSR);
  VT100_set_cursor_pos(mcbl->firstrow, 0);
  MPRINTF(">");
  MPRINTF(VT100_CURSOR_ON);
  MPRINTF((char *)mcbl->param_str);
  // Вычислим где сейчас находится курсор
  n = strlen((char *)mcbl->param_str);
  if (n != 0)
  {
    mcbl->current_row =(n + 2) / COLCOUNT;
    mcbl->current_col =(n + 2)% COLCOUNT;
    mcbl->current_pos =  n;
  }
  else
  {
    mcbl->current_row = 0;
    mcbl->current_col = 2;
    mcbl->current_pos = 0;
  }
  VT100_set_cursor_pos(mcbl->firstrow + mcbl->current_row, mcbl->current_col);
  Set_monitor_func(Edit_func);
}

/*-------------------------------------------------------------------------------------------
   Функция редактирования параметра в окне терминала
---------------------------------------------------------------------------------------------*/
void Edit_func(uint8_t b)
{
  GET_MCBL;

  switch (b)
  {
  case 0x08:  // Back Space
    if (mcbl->current_pos > 0)
    {
      mcbl->current_pos--;
      mcbl->param_str[mcbl->current_pos] = 0;

      if (mcbl->current_col < 2)
      {
        mcbl->current_col = COLCOUNT;
        mcbl->current_row--;
      }
      else
      {
        mcbl->current_col--;
      }
    }
    VT100_set_cursor_pos(mcbl->firstrow + mcbl->current_row, mcbl->current_col);
    MPRINTF(" ");
    VT100_set_cursor_pos(mcbl->firstrow + mcbl->current_row, mcbl->current_col);
    break;
  case 0x7E:  // DEL
  case 0x7F:  // DEL
    mcbl->current_row = 0;
    mcbl->current_col = 2;
    mcbl->current_pos = 0;
    mcbl->param_str[mcbl->current_pos] = 0;
    VT100_set_cursor_pos(mcbl->firstrow + mcbl->current_row, mcbl->current_col);
    MPRINTF(VT100_CLR_FM_CRSR);
    break;
  case 0x1B:  // ESC

    MPRINTF(VT100_CURSOR_OFF);
    Show_parameters_menu();
    Set_monitor_func(Params_editor_press_key_handler);

    break;
  case 0x0D:  // Enter
    mcbl->param_str[mcbl->current_pos] = 0;

    Convert_str_to_parameter(mcbl->p_pars,  mcbl->param_str, mcbl->current_parameter_indx);

    // Вызвать на исполнение функцию если таковая есть у параметра
    if (mcbl->p_pars->items_array[mcbl->current_parameter_indx].func != 0) mcbl->p_pars->items_array[mcbl->current_parameter_indx].func();


    Request_save_app_settings();

    MPRINTF(VT100_CURSOR_OFF);
    Show_parameters_menu();
    Set_monitor_func(Params_editor_press_key_handler);
    break;
  default:
    if (isspace(b) || isgraph(b))
    {
      mcbl->param_str[mcbl->current_pos] = b;
      MPRINTF("%c", b);
      if (mcbl->current_pos < (MAX_PARAMETER_STRING_LEN - 1))
      {
        mcbl->current_pos++;
        if (mcbl->current_col == (COLCOUNT))
        {
          mcbl->current_col = 1;
          mcbl->current_row++;
        }
        else mcbl->current_col++;
      }
      VT100_set_cursor_pos(mcbl->firstrow + mcbl->current_row, mcbl->current_col);
    }
    break;

  }

}


/*-------------------------------------------------------------------------------------------
   Преобразовать строку в параметр

   Входная строка должна позволять модификацию
---------------------------------------------------------------------------------------------*/
void Convert_str_to_parameter(const T_NV_parameters_instance  *p_pars, uint8_t *in_str, uint16_t indx)
{
  uint8_t  uch_tmp;
  uint16_t uin_tmp;
  uint32_t ulg_tmp;
  int32_t slg_tmp;
  char *end;
  float  d_tmp;


  // Откорректируем преобразование строк True и False в 1 и 0 соответственно
  if (p_pars->items_array[indx].vartype != tstring)
  {
    if (strcmp((char *)in_str, "True") == 0)
    {
      in_str[0] = '1';
      in_str[1] = 0;
    }
    else if (strcmp((char *)in_str, "False") == 0)
    {
      in_str[0] = '0';
      in_str[1] = 0;
    }
  }


  switch (p_pars->items_array[indx].vartype)
  {
  case tint8u:
    uch_tmp = strtol((char *)in_str,&end, 10);
    if (uch_tmp > ((uint8_t)p_pars->items_array[indx].maxval)) uch_tmp = (uint8_t)p_pars->items_array[indx].maxval;
    if (uch_tmp < ((uint8_t)p_pars->items_array[indx].minval)) uch_tmp = (uint8_t)p_pars->items_array[indx].minval;
    *(uint8_t *)p_pars->items_array[indx].val = uch_tmp;
    break;
  case tint16u:
    uin_tmp = strtol((char *)in_str,&end, 10);
    if (uin_tmp > ((uint16_t)p_pars->items_array[indx].maxval)) uin_tmp = (uint16_t)p_pars->items_array[indx].maxval;
    if (uin_tmp < ((uint16_t)p_pars->items_array[indx].minval)) uin_tmp = (uint16_t)p_pars->items_array[indx].minval;
    *(uint16_t *)p_pars->items_array[indx].val = uin_tmp;
    break;
  case tint32u:
    ulg_tmp = strtol((char *)in_str,&end, 10);
    if (ulg_tmp > ((uint32_t)p_pars->items_array[indx].maxval)) ulg_tmp = (uint32_t)p_pars->items_array[indx].maxval;
    if (ulg_tmp < ((uint32_t)p_pars->items_array[indx].minval)) ulg_tmp = (uint32_t)p_pars->items_array[indx].minval;
    *(uint32_t *)p_pars->items_array[indx].val = ulg_tmp;
    break;
  case tint32s:
    slg_tmp = strtol((char *)in_str,&end, 10);
    if (slg_tmp > ((int32_t)p_pars->items_array[indx].maxval)) slg_tmp = (uint32_t)p_pars->items_array[indx].maxval;
    if (slg_tmp < ((int32_t)p_pars->items_array[indx].minval)) slg_tmp = (uint32_t)p_pars->items_array[indx].minval;
    *(uint32_t *)p_pars->items_array[indx].val = slg_tmp;
    break;
  case tfloat:
    d_tmp = strtod((char *)in_str,&end);
    if (d_tmp > ((float)p_pars->items_array[indx].maxval)) d_tmp = (float)p_pars->items_array[indx].maxval;
    if (d_tmp < ((float)p_pars->items_array[indx].minval)) d_tmp = (float)p_pars->items_array[indx].minval;
    *(float *)p_pars->items_array[indx].val = d_tmp;
    break;
  case tstring:
    {
      uint8_t *st;
      strncpy(p_pars->items_array[indx].val, (char *)in_str, p_pars->items_array[indx].varlen - 1);
      st = p_pars->items_array[indx].val;
      st[p_pars->items_array[indx].varlen - 1] = 0;

    }
    break;
  case tarrofbyte:
  case tarrofdouble:
    break;
  }
}

/*-------------------------------------------------------------------------------------------
   Преобразовать параметр в строку
---------------------------------------------------------------------------------------------*/
void Convert_parameter_to_str(const T_NV_parameters_instance  *p_pars, uint8_t *buf, uint16_t maxlen, uint16_t indx)
{
  void *val;

  // tint8u, tint16u, tint32u, tdouble, tstring, tarrofdouble, tarrofbyte
  val = p_pars->items_array[indx].val;
  switch (p_pars->items_array[indx].vartype)
  {
  case tint8u:
    snprintf((char *)buf, maxlen, p_pars->items_array[indx].format,*(uint8_t *)val);
    break;
  case tint16u:
    snprintf((char *)buf, maxlen, p_pars->items_array[indx].format,*(uint16_t *)val);
    break;
  case tint32u:
    snprintf((char *)buf, maxlen, p_pars->items_array[indx].format,*(uint32_t *)val);
    break;
  case tint32s:
    snprintf((char *)buf, maxlen, p_pars->items_array[indx].format,*(int32_t *)val);
    break;
  case tfloat:
    {
      float f;
      f =*((float *)val);
      snprintf((char *)buf, maxlen, p_pars->items_array[indx].format, (double)f);
      break;
    }
  case tstring:
    {
      int len;
      if (p_pars->items_array[indx].varlen > maxlen)
      {
        len = maxlen - 1;
      }
      else
      {
        len = p_pars->items_array[indx].varlen - 1;
      }
      strncpy((char *)buf, (char *)val, len);
      buf[len] = 0;
    }
    break;
  default:
    break;
  }
}

/*-------------------------------------------------------------------------------------------
  Найти индекс параметра по псевдониму
---------------------------------------------------------------------------------------------*/
int32_t Find_param_by_alias(const T_NV_parameters_instance  *p_pars, char *alias)
{
  int i;
  for (i = 0; i < p_pars->items_num; i++)
  {
    if (strcmp((char *)p_pars->items_array[i].var_alias, alias) == 0) return (i);
  }
  return (-1);
}

/*-------------------------------------------------------------------------------------------
  Найти индекс параметра по аббревиатуре
---------------------------------------------------------------------------------------------*/
int32_t Find_param_by_name(const T_NV_parameters_instance  *p_pars, char *name)
{
  int i;
  for (i = 0; i < p_pars->items_num; i++)
  {
    if (strcmp((char *)p_pars->items_array[i].var_name, name) == 0) return (i);
  }
  return (-1);
}

/*-----------------------------------------------------------------------------------------------------



  \return const char*
-----------------------------------------------------------------------------------------------------*/
const char* Convrt_var_type_to_str(enum  vartypes  vartype)
{
  switch (vartype)
  {
  case tint8u        :
    return "tint8u";
  case tint16u       :
    return "tint16u";
  case tint32u       :
    return "tint32u";
  case tfloat        :
    return "tfloat";
  case tarrofdouble  :
    return "tarrofdouble";
  case tstring       :
    return "tstring";
  case tarrofbyte    :
    return "tarrofbyte";
  case tint32s       :
    return "tint32s";
  default:
    return "unknown";
  }
}


/*-------------------------------------------------------------------------------------------
  Найти название меню по его идентификатору
---------------------------------------------------------------------------------------------*/
uint8_t* Get_mn_name(const T_NV_parameters_instance *p_pars, uint32_t menu_lev)
{
  uint16_t i;


  for (i = 0; i < p_pars->menu_items_num; i++)
  {
    if (p_pars->menu_items_array[i].currlev == menu_lev) return(uint8_t *)p_pars->menu_items_array[i].name;
  }
  return(uint8_t *)p_pars->menu_items_array[0].name;
}

