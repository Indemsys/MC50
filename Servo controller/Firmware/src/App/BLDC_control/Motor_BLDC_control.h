#ifndef MOTOR_BLDC_CONTROL_H
  #define MOTOR_BLDC_CONTROL_H

// Структура записей в таблице коммутации
typedef struct
{
    int8_t  rv_U;
    int8_t  rv_V;
    int8_t  rv_W;

    int8_t  fw_U;
    int8_t  fw_V;
    int8_t  fw_W;

} T_comm_rec;

typedef struct
{
    int8_t  U;
    int8_t  V;
    int8_t  W;
} T_6step_rec;


typedef struct
{
  uint8_t       out_U_state_req;    // Запрос на переключение  фазы U в отключенное или активное состояние
  uint8_t       out_V_state_req;    // Запрос на переключение  фазы V в отключенное или активное состояние
  uint8_t       out_W_state_req;    // Запрос на переключение  фазы W в отключенное или активное состояние
  int32_t       gpt_U_pwm_val;
  int32_t       gpt_V_pwm_val;
  int32_t       gpt_W_pwm_val;
  uint8_t       comm_law_arr[6];    // Таблица 6-шаговой коммутации
  T_6step_rec   rv_comm[6];
  T_6step_rec   fw_comm[6];
  uint32_t      pwm_indx_to_comp[PWM_STEP_COUNT]; // Таблица конвертации коэфициента заполнения ШИМ в значение компаратора таймера
  uint32_t      gpt_top_val;        // Максимальное значение компаратора таймеров при генерации ШИМ

  float         rotation_freq;
  T_int32_3ph   angle_3ph;
  T_int32_3ph   pwm_3ph_norm;
  T_pwm_3ph     pwm_3ph;
  uint8_t       sin_pwm_type;       // Способ модуляции для получения синусоиды
  uint32_t      norm_angle;         // Текущий нормированный угол синсоиды фазы U при синусоидальном вращении
  int32_t       ampl_scale;         // Коэфиуиент масштабирования амплитуды синусоды при синусоидальном вращении
  uint32_t      angle_delta;
}
T_rotation_3ph;

extern  const T_comm_rec      reference_comm_table[6];

extern T_rotation_3ph         r3ph;
extern  uint8_t               g_comm_tbl_arr[6];
extern  uint8_t               g_check_arr[6];


void     BLDC_ISR_handler(void);

void     BLDC_rotor_hard_stop(void);
void     BLDC_rotor_freewheeling(void);
void     BLDC_set_outputs_state(uint8_t state);
uint32_t BLDC_prepare_6step_tbls(void);

#endif



