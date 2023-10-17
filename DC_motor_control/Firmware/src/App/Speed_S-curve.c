// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-10-14
// 12:53:56
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


#define   TRAJ_ACCELR_CONCAVE  0
#define   TRAJ_ACCELR_CONVEX   1
#define   TRAJ_LINEAR          2
#define   TRAJ_DECELR_CONVEX   3
#define   TRAJ_DECELR_CONCAVE  4
#define   TRAJ_FINISH          5

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Speed_Scurve_init(uint8_t mov_dir)
{

  mov_cbl.start_position = adc.smpl_POS_SENS;

  if (mov_dir == NEED_TO_CLOSE)
  {
    // Определяем желательную позицию остановки движения
    mov_cbl.finish_position = wvar.close_position;

    // Определяем позицию немедленной остановки c учетом запаса по углу на закрытие
    if (drv_cbl.opening_direction == OPENING_CLOCKWISE)
    {
      mov_cbl.stop_position = mov_cbl.finish_position - lroundf(wvar.closing_margine * adc.grad_to_samples_scale);
    }
    else
    {
      mov_cbl.stop_position = mov_cbl.finish_position + lroundf(wvar.closing_margine * adc.grad_to_samples_scale);
    }

  }
  else if (mov_dir == NEED_TO_OPEN)
  {
    // Определяем желательную позицию остановки движения
    mov_cbl.finish_position = wvar.open_position;

    // Определяем позицию немедленной остановки c учетом запаса по углу на открытие
    if (drv_cbl.opening_direction == OPENING_CLOCKWISE)
    {
      mov_cbl.stop_position = mov_cbl.finish_position + lroundf(wvar.opening_margine * adc.grad_to_samples_scale);
    }
    else
    {
      mov_cbl.stop_position = mov_cbl.finish_position - lroundf(wvar.opening_margine * adc.grad_to_samples_scale);
    }
  }

  mov_cbl.moving_distance = abs(mov_cbl.start_position - mov_cbl.finish_position);

  if (mov_cbl.start_position > mov_cbl.finish_position)
  {
    mov_cbl.current_speed = -adc.shaft_speed_smoothly;
    mov_cbl.reverse_movement  = 1;
  }
  else
  {
    mov_cbl.current_speed = adc.shaft_speed_smoothly;
    mov_cbl.reverse_movement  = 0;
  }

  // Если движение ещё идет в обратном направлении, то не учитывать его, поскольку не можем создать вращающий момент в противоположном направлении
  if (mov_cbl.current_speed < 0.0f) mov_cbl.current_speed = 0.0f;

  // .....................................................................................................................................
  // Планирование траектории
  // .....................................................................................................................................
  // Ускорение и замедление проводим по S-кривой
  // Свойство S-кривой таково что максимальное ускорение на ней равно удвоенному линейному ускорению между начальной и конечной скоростью
  // Формула вогнутой (concave) части S-кривой записывается так:  V0 + (As/T)*t^2
  // Формулы выпуклой (convex)  части S-кривой записывается так:  Vh + As*t - (As/T)*t^2
  // Где: T  - время нарастания от начальной скорости к конечной
  //      V0 - начальная скорость, V1 - конеченая скорость
  //      Vh - средняя скорсоть = (V1-V0)/2
  //      As - максимальное ускорение на S-кривой. Оно равно 2*(V1-V0)/T,
  //      t  - текущее время


  // Расчет времени ускорения в секундах с учетом текущей скорости
  mov_cbl.accel_time =((wvar.rotation_speed - mov_cbl.current_speed) / wvar.rotation_speed) * wvar.acceleration_time_ms / 1000.0f;
  // Расчет дистанции ускорения в отсчетах АЦП
  mov_cbl.accel_dist =  mov_cbl.accel_time * ((wvar.rotation_speed - mov_cbl.current_speed) / 2.0f + mov_cbl.current_speed) * adc.grad_to_samples_scale;

  // Расчет времени замедления в секундах с учетом текущей скорости
  mov_cbl.decel_time = wvar.deceleration_time_ms / 1000.0f;
  // Расчет дистанции замедления в отсчетах АЦП
  mov_cbl.decel_dist =  mov_cbl.decel_time * ((wvar.rotation_speed - wvar.min_rotation_speed) / 2.0f + wvar.min_rotation_speed) * adc.grad_to_samples_scale;

  mov_cbl.lin_time   = 0; // Продолжительность линейного движения

  float acel_decel_dist = mov_cbl.accel_dist + mov_cbl.decel_dist;

  float rot_accel_speed_delta = wvar.rotation_speed - mov_cbl.current_speed;
  float rot_decel_speed_delta = wvar.rotation_speed - wvar.min_rotation_speed;
  float scale = 1.0f;

  if (acel_decel_dist >= mov_cbl.moving_distance)
  {
    // На требуемой дистанции не удастся развить максимальную скорость
    // Уменьшаем время разгона и торможения пропорционально сокращению дистанции
    scale                  = mov_cbl.moving_distance / acel_decel_dist;
    mov_cbl.accel_time          = mov_cbl.accel_time * scale;
    mov_cbl.decel_time          = mov_cbl.decel_time * scale;
    rot_accel_speed_delta  = rot_accel_speed_delta * scale;
    rot_decel_speed_delta  = rot_decel_speed_delta * scale;
  }
  else
  {
    // Расчет времени линейного движеня в секундах
    mov_cbl.lin_time =((mov_cbl.moving_distance - acel_decel_dist) * adc.samples_to_grad_scale) / wvar.rotation_speed;
  }

  // Находим коэфициенты необходимые для расчета s-кривой на этапе ускорения
  mov_cbl.accel_v0    = mov_cbl.current_speed;
  mov_cbl.accel_vh    = rot_accel_speed_delta / 2.0f;
  mov_cbl.accel_As    = 2 * rot_accel_speed_delta / mov_cbl.accel_time;
  mov_cbl.accel_C2    = mov_cbl.accel_As / mov_cbl.accel_time;
  mov_cbl.accel_halfT = mov_cbl.accel_time / 2.0f;

  // Находим коэфициенты необходимые для расчета s-кривой на этапе замедления
  mov_cbl.decel_v0    = wvar.min_rotation_speed * scale;
  mov_cbl.decel_vh    = rot_decel_speed_delta / 2.0f + wvar.min_rotation_speed * scale;
  mov_cbl.decel_As    = 2 * rot_decel_speed_delta / mov_cbl.decel_time;
  mov_cbl.decel_C2    = mov_cbl.decel_As / mov_cbl.decel_time;
  mov_cbl.decel_halfT = mov_cbl.decel_time / 2.0f;
  float       Jm = 2.0f * mov_cbl.decel_As / mov_cbl.decel_time;
  mov_cbl.decel_concave_dist =(((wvar.min_rotation_speed + powf(mov_cbl.decel_As, 2) / (6.0f * Jm)) * mov_cbl.decel_As) / Jm) * adc.grad_to_samples_scale; // Находим путь пройденный в течении вогнутой части S-кривой на стадии замедления

  mov_cbl.traj_stage = TRAJ_ACCELR_CONCAVE;
  mov_cbl.stage_time = 0;
  mov_cbl.estim_pos_0= mov_cbl.start_position;
}

/*-----------------------------------------------------------------------------------------------------
  Получение значения целевой скорости в градусах в сек в текущий момент

  \param start_pos
  \param finish_pos

  \return float
-----------------------------------------------------------------------------------------------------*/
float Speed_Scurve_step(void)
{
  float t;
  float rotation_speed = 0.0f;

  // Движение безотносительно направления движения и стороны подвеса  начинается с нуля и заканчивается в точке  mc.distance
  // Здесь нормализуем в соотвествии с этим условием значения позиции и скорости
  if (mov_cbl.reverse_movement)
  {
    mov_cbl.current_pos   = mov_cbl.start_position - adc.smpl_POS_SENS;
    mov_cbl.current_speed = -adc.shaft_speed_smoothly;
  }
  else
  {
    mov_cbl.current_pos   = adc.smpl_POS_SENS - mov_cbl.start_position;
    mov_cbl.current_speed = adc.shaft_speed_smoothly;
  }

  switch (mov_cbl.traj_stage)
  {
  case TRAJ_ACCELR_CONCAVE:
    t = mov_cbl.stage_time;
    rotation_speed = mov_cbl.accel_v0 + mov_cbl.accel_C2 * powf(t,2);
    if (mov_cbl.reverse_movement)
    {
      mov_cbl.estim_pos   = -(mov_cbl.accel_v0 * t + mov_cbl.accel_C2 * powf(t,3) / 3.0f) * adc.grad_to_samples_scale + mov_cbl.estim_pos_0;
    }
    else
    {
      mov_cbl.estim_pos   =(mov_cbl.accel_v0 * t + mov_cbl.accel_C2 * powf(t,3) / 3.0f) * adc.grad_to_samples_scale + mov_cbl.estim_pos_0;
    }
    mov_cbl.stage_time += 1.0f / (float)(SPEED_LOOP_FREQ_HZ);
    if (mov_cbl.stage_time >= mov_cbl.accel_halfT)
    {
      mov_cbl.estim_pos_0 = adc.smpl_POS_SENS;
      mov_cbl.traj_stage++;
      mov_cbl.stage_time = 0.0f;
    }
    break;
  case TRAJ_ACCELR_CONVEX :
    t = mov_cbl.stage_time;
    rotation_speed = mov_cbl.accel_vh +  mov_cbl.accel_As * t -  mov_cbl.accel_C2 * powf(t,2);
    if (mov_cbl.reverse_movement)
    {
      mov_cbl.estim_pos   = -(mov_cbl.accel_vh * t +  mov_cbl.accel_As * powf(t,2) / 2.0f -  mov_cbl.accel_C2 * powf(t,3) / 3.0f) * adc.grad_to_samples_scale + mov_cbl.estim_pos_0;
    }
    else
    {
      mov_cbl.estim_pos   =(mov_cbl.accel_vh * t +  mov_cbl.accel_As * powf(t,2) / 2.0f -  mov_cbl.accel_C2 * powf(t,3) / 3.0f) * adc.grad_to_samples_scale + mov_cbl.estim_pos_0;
    }
    mov_cbl.stage_time += 1.0f / (float)(SPEED_LOOP_FREQ_HZ);
    if (mov_cbl.stage_time >= mov_cbl.accel_halfT)
    {
      mov_cbl.estim_pos_0 = adc.smpl_POS_SENS;
      mov_cbl.traj_stage++;
      mov_cbl.stage_time = 0.0f;
      if (mov_cbl.lin_time == 0) mov_cbl.traj_stage++; // Пропускаем этап линейной скорости если его продолжительность нулевая
    }
    break;
  case TRAJ_LINEAR        :
    t = mov_cbl.stage_time;
    rotation_speed = wvar.rotation_speed;
    mov_cbl.stage_time += 1.0f / (float)(SPEED_LOOP_FREQ_HZ);

    if (mov_cbl.reverse_movement)
    {
      mov_cbl.estim_pos   = -(wvar.rotation_speed * t) * adc.grad_to_samples_scale + mov_cbl.estim_pos_0;
    }
    else
    {
      mov_cbl.estim_pos   =(wvar.rotation_speed * t) * adc.grad_to_samples_scale + mov_cbl.estim_pos_0;
    }

    // Переходим к замедлению если пересекли точку замедления
    if (mov_cbl.reverse_movement)
    {
      if (adc.smpl_POS_SENS <= (mov_cbl.finish_position + mov_cbl.decel_dist))
      {
        mov_cbl.estim_pos_0 = adc.smpl_POS_SENS;
        mov_cbl.traj_stage++;
        mov_cbl.stage_time = 0.0f;
      }
    }
    else
    {
      if (adc.smpl_POS_SENS >= (mov_cbl.finish_position - mov_cbl.decel_dist))
      {
        mov_cbl.estim_pos_0 = adc.smpl_POS_SENS;
        mov_cbl.traj_stage++;
        mov_cbl.stage_time = 0.0f;
      }
    }

    break;
  case TRAJ_DECELR_CONVEX :
    t = mov_cbl.decel_halfT - mov_cbl.stage_time;
    rotation_speed = mov_cbl.decel_vh +  mov_cbl.decel_As * t -  mov_cbl.decel_C2 * (powf(t,2));
    if (mov_cbl.reverse_movement)
    {
      mov_cbl.estim_pos   = -((mov_cbl.decel_dist-mov_cbl.decel_concave_dist)-(mov_cbl.decel_vh * t +  mov_cbl.decel_As * powf(t,2) / 2.0f -  mov_cbl.decel_C2 * powf(t,3) / 3.0f) * adc.grad_to_samples_scale)+ mov_cbl.estim_pos_0;
    }
    else
    {
      mov_cbl.estim_pos   =(mov_cbl.decel_dist-mov_cbl.decel_concave_dist)-(mov_cbl.decel_vh * t +  mov_cbl.decel_As * powf(t,2) / 2.0f -  mov_cbl.decel_C2 * powf(t,3) / 3.0f) * adc.grad_to_samples_scale + mov_cbl.estim_pos_0;
    }
    mov_cbl.stage_time += 1.0f / (float)(SPEED_LOOP_FREQ_HZ);
    if (mov_cbl.stage_time >= mov_cbl.decel_halfT)
    {
      mov_cbl.estim_pos_0 = adc.smpl_POS_SENS;
      mov_cbl.traj_stage++;
      mov_cbl.stage_time = 0.0f;
    }
    break;
  case TRAJ_DECELR_CONCAVE :
    t = mov_cbl.decel_halfT - mov_cbl.stage_time;
    rotation_speed = mov_cbl.decel_v0 + mov_cbl.decel_C2 * (powf(t,2));
    if (mov_cbl.reverse_movement)
    {
      mov_cbl.estim_pos   = -(mov_cbl.decel_concave_dist -(mov_cbl.decel_v0 * t + mov_cbl.decel_C2 * powf(t,3) / 3.0f) * adc.grad_to_samples_scale)+ mov_cbl.estim_pos_0;
    }
    else
    {
      mov_cbl.estim_pos   = mov_cbl.decel_concave_dist -(mov_cbl.decel_v0 * t + mov_cbl.decel_C2 * powf(t,3) / 3.0f) * adc.grad_to_samples_scale + mov_cbl.estim_pos_0;
    }
    mov_cbl.stage_time += 1.0f / (float)(SPEED_LOOP_FREQ_HZ);
    if (mov_cbl.stage_time >= mov_cbl.decel_halfT)
    {
      mov_cbl.estim_pos_0 = adc.smpl_POS_SENS;
      mov_cbl.traj_stage++;
      mov_cbl.stage_time = 0.0f;
    }
    break;
  case TRAJ_FINISH:
    rotation_speed = wvar.min_rotation_speed;
    break;
  }

  return rotation_speed;
}



