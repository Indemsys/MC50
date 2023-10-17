#ifndef QUAD_ENCODER_H
  #define QUAD_ENCODER_H


#define   QENC_ROTATION_CW        1
#define   QENC_ROTATION_CCW      -1


#define   QENC_RAV_FLTR_LEN       32

typedef struct
{
  int32_t                 capt_prev;
  int32_t                 delta;
  int32_t                 delta_aver;
  uint8_t                 bitmask;
  uint8_t                 bitmask_prev;
  T_run_average_int32_N   ra_filtr;
  int32_t                 ra_filtr_arr[QENC_RAV_FLTR_LEN]; // Массив выборок фильтра бегущего среднего скорости вращения сервопотенциометра
  int32_t                 ror_dir;

} T_QuadEncoder_cbl;

extern T_QuadEncoder_cbl  qenc;


void QEnc_reinit(void);
void QEnc_measure_speed_and_direction(void);


#endif



