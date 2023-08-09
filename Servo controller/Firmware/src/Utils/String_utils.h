#ifndef STRING_UTILS_H
  #define STRING_UTILS_H

#define MAX_MARKER_SIZE  32

// Структура контрольного блока при поиске маркированного числа с плавающей запятой в потоке текста тела HTML страницы
// Число должно быть окружено левым и правым  маркером. Маркеры - это заданые фрагменты текста
typedef struct
{
  int32_t     step;
  uint32_t    cnt;
  const char *left_mark;
  uint32_t    left_mark_sz;
  const char *right_mark;
  uint32_t    right_mark_sz;
  char        fragment[MAX_MARKER_SIZE+1];
  uint32_t    fragment_sz;    // Длина текста с найденным числом
  uint8_t     next_sym_pos;   // Позиция следующего символа в блоке текста после правлго маркера после обнаружения искомого числа
  float       number;         // Найденное число

} T_marked_num_finder;



uint8_t        BCD2ToBYTE(uint8_t val);
uint8_t        BYTEToBCD2(uint8_t val);
uint8_t        ascii_to_hex(uint8_t c);
uint8_t        hex_to_ascii(uint8_t c);
char*          Trim_and_dequote_str(char *str);
int            Read_cstring_from_buf(char **buf, char *str, uint32_t len);
uint8_t*       Isolate_string_in_buf(uint8_t **buf, uint32_t *buf_len);
void           Get_nums_from_date(char *date_str, uint32_t *pmonts, uint32_t *pday, uint32_t *pyear);
void           Get_build_date_time(char *ver_str);
#endif



