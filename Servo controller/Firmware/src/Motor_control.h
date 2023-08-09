#ifndef MOTOR_CONTROL_H
  #define MOTOR_CONTROL_H


#define   OPENING_CLOCKWISE                      0      // Константа определющая конфигурацию когда открывается по часовой стрелке при взгляде сверху
                                                        // При открывании скорость поворота положительна. Позиция меняется от меньших значений к большим.
#define   OPENING_COUNTERCLOCKWISE               1      // Константа определющая конфигурацию когда открывается против часовой стрелке при взгляде сверху
                                                        // При открывании скорость поворота отрицательна. Позиция меняется от больших значений к меньшим.


#define   OPENING_CLOCKWISE_SPEED_SIGN         ( 1.0f)  // Знак скорости повората в направлении открывания, когда  открываестя по часовой стрелке при взгляде сверху
#define   OPENING_COUNTERCLOCKWISE_SPEED_SIGN  (-1.0f)  // Знак скорости повората в направлении открывания, когда  открываестя против часовой стрелке при взгляде сверху


#define   USE_FILTERED_CURRENT_IN_CURRENT_LOOP    // Определить макрос если в контуре управления током надо использовать отфильтрованное значение тока


#define   MAX_ALLOWABLE_SHAFT_SPEED_BEFOR_MOVEMENT    (5.0f) // Максимально допустимая скорость вращения вала привода перед началом выполнения команды на движение (град в сек)

#define  NEED_TO_OPEN                               (OPENING_CLOCKWISE)
#define  NEED_TO_CLOSE                              (OPENING_COUNTERCLOCKWISE)

#define  CLOSING_MARGINE                            (10.0f)
#define  OPENING_MARGINE                            (0.0f)


typedef struct
{
    volatile uint8_t   state;                     // Управление моторм включено-1 или выключено-0

    uint8_t            en_current_loop;
    uint8_t            en_speed_loop;

    uint32_t           pwm_freq;
    volatile uint32_t  pwm_val;                   // Установка ШИМ в процентах
    volatile uint32_t  prev_pwm_val;              //
    T_OPSCR            opscr;
    volatile float     mot_rpm;                   // Скорость вращения ротора двигателя в RPM

    float              target_current;
    float              target_rotation_speed;     // Заданая скорость вращения в градусах в секунду
    uint8_t            mot_rotation_target_dir;   // Заданное направление вращения.

    uint8_t            direction;                 // Текущее направление вращения определяемое PID алгоритмом.

    float              speed_sign_on_open_dir;    // Знак скорости поворота в направлении открывания
    uint8_t            opening_direction;         // Направление открывания OPENING_CLOCKWISE или OPENING_COUNTERCLOCKWISE

} T_mot_control_cbl;


#define  TRAJ_ACCELR_CONCAVE  0
#define  TRAJ_ACCELR_CONVEX   1
#define  TRAJ_LINEAR          2
#define  TRAJ_DECELR_CONVEX   3
#define  TRAJ_DECELR_CONCAVE  4
#define  TRAJ_FINISH          5

#define  NO_ORDER             0
#define  ORDER_TO_OPEN        1
#define  ORDER_TO_CLOSE       2


typedef struct
{
    uint8_t          order;                 // Состояние приказа на движение. NO_ORDER,  ORDER_TO_OPEN, ORDER_TO_CLOSE
    T_sys_timestump  start_time;

    uint8_t          reverse_movement;      // Флаг реверсивного движения, когда показания сенсора позиции меняются от больших значений к меньшим

    int32_t          start_position;        // Стартовая позиция начала движения в отсчетах АЦП
    int32_t          finish_position;       // Требуемая позиция прекращения движения в отсчетах АЦП
    int32_t          stop_position;         // Позиция немедленного прекращения движения в отсчетах АЦП

    int32_t          moving_distance;       // Дистанция движения в отсчетах АЦП
    int32_t          current_pos;           // Текущая нормализованная позиция в отсчетах АЦП
    float            current_speed;         // Текущая скрость движения в градусах в секунду

    uint8_t          traj_stage;            // Идентификатор текущего участка траектории S-кривой
    float            stage_time;            // Время пошедшее относительно начала текущего участка траектории
    float            estim_pos;             // Оценка текущей позиции по расчетной S-кривой в отсчетах АЦП. Используется для отладки
    float            estim_pos_0;           // Начальная позиция для оценки текущей позиции в отсчетах АЦП. Используется для отладки

    // Переменные для формирования траектории скорости по s-кривой на стадии ускорения
    float            accel_time;
    float            accel_dist;
    float            accel_v0;
    float            accel_vh;
    float            accel_As;
    float            accel_C2;
    float            accel_halfT;

    float            lin_time;

    // Переменные для формирования траектории скорости по s-кривой на стадии замедления
    float            decel_time;
    float            decel_dist;
    float            decel_concave_dist;
    float            decel_v0;
    float            decel_vh;
    float            decel_As;
    float            decel_C2;
    float            decel_halfT;

} T_movement_cbl;


extern T_movement_cbl mc;

extern T_mot_control_cbl   mot_cbl;
extern uint32_t            moving_pulse_ms;
extern PIDParams           pid_i_loop;
extern PIDParams           pid_s_loop;
extern float               speed_loop_e;
extern float               current_loop_e;


void                Motor_control_thread(void);
void                MC_create_sync_obj(void);

uint32_t            MC_set_open(void);
uint32_t            MC_set_close(void);
uint32_t            MC_set_closing_pulse(void);
uint32_t            MC_set_opening_pulse(void);
uint32_t            MC_set_close_position(void);
uint32_t            MC_set_open_position(void);
uint32_t            MC_set_freewheeling(void);


uint32_t            MC_set_start_motor(void);
uint32_t            MC_set_stop_motor(void);
uint32_t            MC_set_driver_fault(void);
uint32_t            MC_set_moving_pulse(void);

uint32_t            MC_set_reset_tmc6200(void);
uint32_t            MC_set_change_pwm(void);
uint32_t            MC_set_speed_loop_tick(void);
T_mot_control_cbl*  MC_get_cbl(void);
void                MC_do_6_step_commutation(void);
void                MC_current_loop_PID_step(void);
void                MC_speed_loop_PID_step(void);
void                Motor_init_PID_params(void);
void                Determine_opening_direction(void);

void                MC_freewheeling(void);
void                MC_brake_on(void);
void                MC_brake_off(void);

#endif



