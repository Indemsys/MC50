#ifndef MAIN_MONITOR_H
  #define MAIN_MONITOR_H

  #define COLCOUNT 100
  #define ROWCOUNT 24

  #define VT100_CNTLQ      0x11
  #define VT100_CNTLS      0x13
  #define VT100_DEL        0x7F
  #define VT100_BCKSP      0x08
  #define VT100_CR         0x0D  //конфликт имен с регистрами периферии
  #define VT100_LF         0x0A
  #define VT100_ESC        0x1B

/* cursor motion */
  #define VT100_CURSOR_DN   "\033D"  //
  #define VT100_CURSOR_DN_L "\033E"
  #define VT100_CURSOR_UP   "\033M"
  #define VT100_CURSOR_HOME "\033[H"
  #define VT100_CURSOR_N_UP "\033[%dA"  /* printf argument: lines */
  #define VT100_CURSOR_N_RT "\033[%dC"  /* printf argument: cols  */
  #define VT100_CURSOR_N_LT "\033[%dD"  /* printf argument: cols  */
  #define VT100_CURSOR_N_DN "\033[%dB"  /* printf argument: lines */
  #define VT100_CURSOR_SET  "\033[%d;%dH" /* printf arguments: row, col */

/* erasing the screen */
  #define VT100_CLR_FM_CRSR "\033[J"
  #define VT100_CLR_TO_CRSR "\033[1J"
  #define VT100_CLR_SCREEN  "\033[2J"

/* erasing current line */
  #define VT100_CLL_FM_CRSR "\033[K"
  #define VT100_CLL_TO_CRSR "\033[1K"
  #define VT100_CLR_LINE    "\033[2K"

/* inserting and deleting */
  #define VT100_INS_CHARS   "\033[%d"   /* printf argument: cols */
  #define VT100_DEL_CHARS   "\033[%dP"  /* printf argument: cols */
  #define VT100_INS_LINES   "\033[%dL"  /* printf argument: cols */
  #define VT100_DEL_LINES   "\033[%dM"  /* printf argument: cols */

/* character attributes */
  #define VT100_NORMAL      "\033[m"
  #define VT100_ALL_OFF     "\033[0m"
  #define VT100_BOLD_ON     "\033[1m"
  #define VT100_BOLD_OFF    "\033[22m"
  #define VT100_UNDERL_ON   "\033[4m"
  #define VT100_UNDERL_OFF  "\033[24m"
  #define VT100_BLINK_ON    "\033[5m"
  #define VT100_BLINK_OFF   "\033[25m"
  #define VT100_REVERSE_ON  "\033[7m"
  #define VT100_REVERSE_OFF "\033[27m"
  #define VT100_INVIS_ON    "\033[8m"
  #define VT100_INVIS_OFF   "\033[28m"

/* screen attributes */
  #define VT100_ECHO_ON     "\033[12l"
  #define VT100_ECHO_OFF    "\033[12h"
  #define VT100_WRAP_ON     "\033[?7l"
  #define VT100_WRAP_OFF    "\033[?7h"
  #define VT100_CURSOR_ON   "\033[?25h"
  #define VT100_CURSOR_OFF  "\033[?25l"
  #define VT100_ENQ_SIZE    "\033[?92l" /* response: "\033[?%d,%dc" rows, cols */

  #define VT100_CLEAR_AND_HOME "\033[2J\033[H\033[m\033[?25l"
  #define VT100_HOME "\033[H\033[m\033[?25l"


  #define DASH_LINE "----------------------------------------------------------------------\n\r"
  #define SCR_ITEMS_VERT_OFFS 8
  #define SCR_ITEMS_HOR_OFFS  1


  #define GET_MCBL     T_monitor_cbl    *mcbl = (T_monitor_cbl*)(tx_thread_identify()->environment); \
                       T_serial_io_driver *mdrv = (T_serial_io_driver*)(tx_thread_identify()->driver);

  #define MPRINTF      mdrv->_printf
  #define WAIT_CHAR    mdrv->_wait_char
  #define SEND_BUF     mdrv->_send_buf


  #define MN_DRIVER_MARK         0x87654321
  #define MN_RTT0_DRIVER         1
  #define MN_USBFS_VCOM0_DRIVER  2
  #define MN_USBFS_VCOM1_DRIVER  3
  #define MN_WIFI_TELNET_DRIVER  4
  #define MN_RNDIS_TELNET_DRIVER 5


typedef void (*T_menu_func)(uint8_t  keycode);

typedef struct
{
    uint8_t      but;       // Кнопка нажатие которой вызывает данный пункт
    T_menu_func  func;      // Функция вызываемая данным пунктом меню
    void        *psubmenu;  // Аргумент. Для подменю указатель на запись подменю
}
T_VT100_Menu_item;

typedef struct
{
    const uint8_t            *menu_header; // Заголовок вверху экрана посередине
    const uint8_t            *menu_body;   // Содержание меню
    const T_VT100_Menu_item  *menu_items;  // Массив структур с аттрибутами пунктов меню
}
T_VT100_Menu;

  #define MENU_MAX_DEPTH   20
  #define MAX_ITEMS_COUNT  16
  #define MONIT_STR_MAXLEN 127

typedef struct
{
    const uint32_t   mark;
    const int        driver_type;
    int              (*_init)(void **pcbl, void *pdrv);
    int              (*_send_buf)(const void *buf, unsigned int len);
    int              (*_wait_char)(unsigned char *b,  int ticks); // ticks - время ожидания выражается в тиках (если 0 то без ожидания)
    int              (*_printf)(const char *, ...);              // Возвращает неопределенный результат
    int              (*_deinit)(void **pcbl);
    void              *pdrvcbl;                                   // Указатель на управляющую структуру необходимую для работы драйвера
} T_serial_io_driver;


typedef struct
{
    T_VT100_Menu                    *menu_trace[MENU_MAX_DEPTH];
    uint32_t                         menu_nesting;
    uint32_t                         curr_pos;                      // Текущая позиция в области редактирования
    uint32_t                         beg_pos;                       // Начальная позиция области редактирования
    void                             (*Monitor_func)(unsigned char);

    uint32_t                         item_indexes[MAX_ITEMS_COUNT]; // Массив индексов пунктов меню. Предназначены для быстрого поиска пунктов в списках
    uint32_t                         current_menu_submenus_count;   // Количество субменю в текущем меню
    uint32_t                         current_menu_items_count;      // Общее количество пунктов всех типов в текущем меню
    uint32_t                         current_level;
    int32_t                          current_parameter_indx;        // Индекс редактируемого параметра
    uint8_t                          param_str[MONIT_STR_MAXLEN+1]; // Строка хранения параметра
    uint8_t                          out_str[MONIT_STR_MAXLEN+1];   // Буфферная строка для вывода
    uint32_t                         firstrow;                      // Позиция первой  строки области редактирования переменной
    uint32_t                         rowcount;                      // Количество строк в области редактирования
    uint32_t                         current_row;
    uint32_t                         current_col;
    uint32_t                         current_pos;
    int32_t                          g_access_to_spec_menu;
    const T_NV_parameters_instance  *p_pars;                       // Указатель на структуру параметров
    T_serial_io_driver                *pdrv;                         // Указатель на драйвер

} T_monitor_cbl;

typedef struct
{
    uint32_t   row;
    uint8_t    b;
    char      *str1;
    char      *str2;
    uint32_t   key_mode;
    uint32_t   keys_mask;

}
T_sys_diagn_term;


typedef struct
{
    uint32_t       allocated;
    TX_THREAD      VT100_thread;
    uint32_t       initial_data;
    T_monitor_cbl  mcbl;
    char           task_name[32];

}  T_VT100_task_cbl;

  #define        VT100_TASKS_MAX_NUM    2

extern T_VT100_task_cbl        vt100_task_cbls[VT100_TASKS_MAX_NUM];
extern const T_VT100_Menu      MENU_MAIN;


uint32_t     Task_VT100_create(ULONG initial_data, int32_t *alloc_indx);
uint32_t     Task_VT100_delete(int32_t *alloc_indx);

void         VT100_clr_screen(void);
uint8_t      Find_str_center(uint8_t *str);
void         VT100_send_str_to_pos(uint8_t *str, uint8_t row, uint8_t col);

void         VT100_set_cursor_pos(uint8_t row, uint8_t col);

void         Set_monitor_func(void (*func)(unsigned char));
void         Goto_main_menu(void);
void         Return_to_prev_menu(void);
void         Menu_press_key_handler(uint8_t b);
void         Display_menu(void);

int32_t      Edit_string_in_pos(char *buf, int buf_len, int row, char *instr);
int32_t      Edit_string(char *buf, uint32_t buf_len, char *instr);
void         Mon_print_dump(uint32_t addr, void *buf, uint32_t buf_len, uint8_t sym_in_str);

void         Edit_uinteger_val(uint32_t row, uint32_t *value, uint32_t minv, uint32_t maxv);
void         Edit_integer_val(uint32_t row, int32_t *value, int32_t minv, int32_t maxv);
uint32_t     Edit_integer_hex_val(uint32_t row, uint32_t *value, uint32_t minv, uint32_t maxv, const char *fmt);
void         Edit_float_val(uint32_t row, float *value, float minv, float maxv);

#endif
