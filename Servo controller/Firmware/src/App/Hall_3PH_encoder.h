#ifndef HALL_3PH_ENCODER_H
  #define HALL_3PH_ENCODER_H


typedef struct
{
  int32_t     U_capt_prev[8];
  int32_t     V_capt_prev[8];
  int32_t     W_capt_prev[8];

  int32_t     U_capt[8];
  int32_t     V_capt[8];
  int32_t     W_capt[8];

  uint32_t    U_edg_n;
  uint32_t    V_edg_n;
  uint32_t    W_edg_n;

  uint8_t     U_bit;
  uint8_t     V_bit;
  uint8_t     W_bit;

  uint8_t     bitmask;     // Переменная содержащая маску состояния битов сигналов с датчиков Холла.
  uint32_t    turn_capt;   // Время полного оборота в отсчетах таймера

  int32_t     rot_dir;

} T_Hall_3PH_cbl;


extern T_Hall_3PH_cbl    h3;


void          Hall_3PH_capture_bitmask(void);
void          Hall_3PH_measure_speed_and_direction(void);
void          Hall_3PH_reinit(void);



#endif



