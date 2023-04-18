#ifndef OUTPUTS_CONTROL_H
  #define OUTPUTS_CONTROL_H


  typedef  void (*T_Set_pin)(int32_t val);



  #define OUTP_LED_GREEN      0
  #define OUTP_LED_RED        1
  #define OUTP_LCD_LIGHT      2
  #define OUTP_LED_CAN        3
  #define OUTP_LED_RS485      4

  #define DIG_OUTPUTS_NUM     5


// Управляющая структура машины состояний управляемой шаблоном
typedef struct
{
    uint8_t          active;
    uint32_t         init_state;
    T_sys_timestump  lastt;               // Время последнего вызова автомата состояний
    uint32_t         duration;            // Длительность состояний в мс
    int32_t          *pattern_start_ptr;  // Указатель на массив констант являющийся цепочкой состояний (шаблоном)
                                          // Если значение в массиве = 0xFFFFFFFF, то процесс обработки завершается
                                          // Если значение в массиве = 0x00000000, то вернуть указатель на начало цепочки
    int32_t          *pttn_ptr;           // Текущая позиция в цепочке состояний
    uint8_t          repeat;              // Флаг принудительного повторения сигнала

} T_outs_ptrn;


int32_t Outputs_mutex_create(void);

void    Set_output_blink(uint32_t out_num);
void    Set_output_on(uint32_t out_num);
void    Set_output_off(uint32_t out_num);
void    Set_output_blink_undef(uint32_t out_num);
void    Set_output_blink_3(uint32_t out_num);
void    Set_output_off_blink_3(uint32_t out_num);
void    Set_output_can_active_blink(uint32_t out_num);

void    Outputs_state_automat(void);
void    Outputs_set_pattern(const int32_t *pttn, uint32_t n);

#endif



