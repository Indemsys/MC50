#ifndef MOTOR_CONTROL_H
  #define MOTOR_CONTROL_H


#define   OPENING_CLOCKWISE                        0      // Константа определющая конфигурацию когда открывается по часовой стрелке при взгляде сверху
                                                          // При открывании скорость поворота положительна. Позиция меняется от меньших значений к большим.
#define   OPENING_COUNTERCLOCKWISE                 1      // Константа определющая конфигурацию когда открывается против часовой стрелке при взгляде сверху
                                                          // При открывании скорость поворота отрицательна. Позиция меняется от больших значений к меньшим.


#define   OPENING_CLOCKWISE_SPEED_SIGN             ( 1.0f)  // Знак скорости повората в направлении открывания, когда  открываестя по часовой стрелке при взгляде сверху
#define   OPENING_COUNTERCLOCKWISE_SPEED_SIGN      (-1.0f)  // Знак скорости повората в направлении открывания, когда  открываестя против часовой стрелке при взгляде сверху


#define   USE_FILTERED_CURRENT_IN_CURRENT_LOOP    // Определить макрос если в контуре управления током надо использовать отфильтрованное значение тока


#define   MAX_ALLOWABLE_SHAFT_SPEED_BEFOR_MOVEMENT    (5.0f) // Максимально допустимая скорость вращения вала привода перед началом выполнения команды на движение (град в сек)

#define   NO_ORDER                                  0
#define   ORDER_TO_OPEN                             1
#define   ORDER_TO_CLOSE                            2

#define   NEED_TO_OPEN                              (OPENING_CLOCKWISE)
#define   NEED_TO_CLOSE                             (OPENING_COUNTERCLOCKWISE)


#define   RPM_MEASURE_OVERFL_TIME                  (0.1f)  // Максимальная длительность импульса при измерении скорости вращени при превышении которого скорость приравнивается нулю

#define   MOTOR_HARD_STOP                          BIT(1)
#define   MOTOR_FREEWHEELING                       BIT(12)

#define   BLDC_MOVING_PULSE_HSTOP                  BIT(3)
#define   BLDC_DRIVER_FAULT                        BIT(4)
#define   BLDC_MOVING_PULSE_FREE                   BIT(5)
#define   BLDC_SERVO_OPEN                          BIT(6)
#define   BLDC_SERVO_CLOSE                         BIT(7)
#define   BLDC_SERVO_CLOSING_PULSE                 BIT(8)
#define   BLDC_SERVO_OPENING_PULSE                 BIT(9)
#define   BLDC_SERVO_FIX_CLOSE_POS                 BIT(10)
#define   BLDC_SERVO_FIX_OPEN_POS                  BIT(11)

// Флаги при управлении DC мотором
#define   DC_ROT_CW                                BIT(13)
#define   DC_ROT_CCW                               BIT(14)
#define   DC_ROT_CW_NO_ACCEL                       BIT(15)  //  Старт вращения без стадии разгона и замедления
#define   DC_ROT_CCW_NO_ACCEL                      BIT(16)
#define   DC_ROT_DECELER                           BIT(17)  // Завершение вращения с замедлением


#define   MC_SPEED_LOOP_TICK                       BIT(31)



typedef struct
{
    volatile uint8_t   state;                     // Управление моторм включено-1 или выключено-0

    uint8_t            en_PID_current_control;
    uint8_t            en_PID_speed_control;
    uint8_t            en_ADRC_control;

    uint32_t           pwm_freq;
    volatile uint32_t  pwm_val;                   // Установка ШИМ в процентах
    volatile uint32_t  prev_pwm_val;              //
    volatile float     mot_rpm;                   // Скорость вращения ротора двигателя в RPM

    float              target_current;
    float              target_rotation_speed;     // Заданая скорость вращения в градусах в секунду
    uint8_t            mot_rotation_target_dir;   // Заданное направление вращения.

    uint8_t            direction;                 // Текущее направление вращения определяемое PID алгоритмом.

    float              speed_sign_on_open_dir;    // Знак скорости поворота в направлении открывания
    uint8_t            opening_direction;         // Направление открывания OPENING_CLOCKWISE или OPENING_COUNTERCLOCKWISE

    uint32_t           speed_loop_divider;
    uint32_t           current_loop_divider;

    float              speed_loop_e;
    float              current_loop_e;

} T_drv_control_cbl;


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

    volatile uint8_t traj_stage;            // Идентификатор текущего участка траектории S-кривой
    float            stage_time;            // Время пошедшее относительно начала текущего участка траектории. Изиеряется в тактах цикла управления скоростью
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

    uint32_t         moving_pulse_acc_ms;   // Параметры для выпонения пульсирующего движения
    uint32_t         moving_pulse_ms;       //
    uint32_t         moving_pulse_dec_ms;   //



} T_movement_cbl;


extern T_movement_cbl      mov_cbl;
extern T_drv_control_cbl   drv_cbl;



#endif



