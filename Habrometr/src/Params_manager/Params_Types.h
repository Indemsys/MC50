#ifndef __PARAMS_TYPES_H
  #define __PARAMS_TYPES_H


  #define VAL_LOCAL_EDITED 0x01  //
  #define VAL_READONLY     0x02  // Можно только читать
  #define VAL_PROTECT      0x04  // Защишено паролем
  #define VAL_UNVISIBLE    0x08  // Не выводится на дисплей
  #define VAL_NOINIT       0x10  // Не инициализируется


enum vartypes
{
 tint8u               = 1,
 tint16u              = 2,
 tint32u              = 3,
 tfloat               = 4,
 tarrofdouble         = 5,
 tstring              = 6,
 tarrofbyte           = 7,
 tint32s              = 8,
};


typedef struct 
{
  uint32_t    prevlev;
  uint32_t    currlev;
  const char* name;
  const char* shrtname;
  const char  visible;
}
T_parmenu;


typedef struct
{
  const uint8_t*     var_name;        // Имя параметра
  const uint8_t*     var_description; // Строковое описание
  const uint8_t*     var_alias;       // Короткая аббревиатура
  void*              val;          // Указатель на значение переменной в RAM
  enum  vartypes     vartype;      // Идентификатор типа переменной
  float              defval;       // Значение по умолчанию
  float              minval;       // Минимальное возможное значение
  float              maxval;       // Максимальное возможное значение  
  uint8_t            attr;         // Аттрибуты переменной
  unsigned int       parmnlev;     // Подгруппа к которой принадлежит параметр
  const  void*       pdefval;      // Указатель на данные для инициализации
  const  char*       format;       // Строка форматирования при выводе на дисплей
  void               (*func)(void);// Указатель на функцию выполняемую после редактирования
  uint16_t           varlen;       // Длинна переменной
  uint32_t           menu_pos;     // Позиция в меню
  uint32_t           selector_id;  // Идентификатор селектора
} T_NV_parameters;


typedef struct
{
  uint32_t           val;
  const uint8_t*     caption;
  int32_t            img_indx;
} T_selector_items;


typedef struct
{
  const uint8_t*           name;
  uint32_t                 items_cnt;
  const T_selector_items*  items_list;
} T_selectors_list;


typedef struct
{
  uint32_t                  items_num;
  const T_NV_parameters    *items_array;
  uint32_t                  menu_items_num;
  const T_parmenu          *menu_items_array;
  uint32_t                  selectors_num;
  const T_selectors_list   *selectors_array;
}
T_NV_parameters_instance;


#endif


