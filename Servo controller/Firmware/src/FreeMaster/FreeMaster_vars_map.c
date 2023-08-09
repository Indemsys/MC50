﻿#include "MC50.h"
#include "freemaster_cfg.h"
#include "freemaster.h"
#include "freemaster_tsa.h"


FMSTR_TSA_TABLE_BEGIN(app_vars)
FMSTR_TSA_RW_VAR(g_cpu_usage                            ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(g_aver_cpu_usage                       ,FMSTR_TSA_UINT32)

FMSTR_TSA_RW_VAR(adc.smpl_V_IU                          ,FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adc.smpl_V_IV                          ,FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adc.smpl_V_IW                          ,FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adc.smpl_V_IPWR                        ,FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adc.smpl_VREF_R                        ,FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adc.smpl_POS_SENS                      ,FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adc.smpl_EXT_TEMP                      ,FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adc.smpl_MISC                          ,FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adc.smpl_TEMP                          ,FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adc.smpl_V_VPWR                        ,FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adc.smpl_CPU_temp                      ,FMSTR_TSA_UINT16)
FMSTR_TSA_RW_VAR(adc.smpl_INT_REF_V                     ,FMSTR_TSA_UINT16)

FMSTR_TSA_RW_VAR(adc.smpl_V_IU_offs                     ,FMSTR_TSA_SINT32)
FMSTR_TSA_RW_VAR(adc.smpl_V_IV_offs                     ,FMSTR_TSA_SINT32)
FMSTR_TSA_RW_VAR(adc.smpl_V_IW_offs                     ,FMSTR_TSA_SINT32)
FMSTR_TSA_RW_VAR(adc.smpl_V_IU_norm                     ,FMSTR_TSA_SINT32)
FMSTR_TSA_RW_VAR(adc.smpl_V_IV_norm                     ,FMSTR_TSA_SINT32)
FMSTR_TSA_RW_VAR(adc.smpl_V_IW_norm                     ,FMSTR_TSA_SINT32)

FMSTR_TSA_RW_VAR(adc.smpl_V_IU_norm_avr                 ,FMSTR_TSA_SINT32)
FMSTR_TSA_RW_VAR(adc.smpl_V_IV_norm_avr                 ,FMSTR_TSA_SINT32)
FMSTR_TSA_RW_VAR(adc.smpl_V_IW_norm_avr                 ,FMSTR_TSA_SINT32)

FMSTR_TSA_RW_VAR(adc.instant_motor_current              ,FMSTR_TSA_SINT32)
FMSTR_TSA_RW_VAR(adc.instant_motor_current_fltr         ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.filtered_motor_current             ,FMSTR_TSA_FLOAT)

FMSTR_TSA_RW_VAR(adc.adc_scale                          ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.current_shunt_scale                ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.current_pwr_scale                  ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.voltage_pwr_scale                  ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.i_u                                ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.i_v                                ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.i_w                                ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.i_pwr                              ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.pos_sens                           ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.ext_temp                           ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.misc                               ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.drv_temp                           ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.v_pwr                              ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.cpu_temp                           ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.input_pwr                          ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.shaft_speed                        ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.shaft_abs_speed                    ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.samples_ps_to_grad_ps              ,FMSTR_TSA_FLOAT)
FMSTR_TSA_RW_VAR(adc.samples_to_grad_scale              ,FMSTR_TSA_FLOAT)

FMSTR_TSA_RW_VAR(mot_cbl.state                          ,FMSTR_TSA_UINT8 )
FMSTR_TSA_RW_VAR(mot_cbl.pwm_val                        ,FMSTR_TSA_SINT32)
FMSTR_TSA_RW_VAR(mot_cbl.mot_rpm                        ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(mot_cbl.pwm_freq                       ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(mot_cbl.target_current                 ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(mot_cbl.mot_rotation_target_dir        ,FMSTR_TSA_UINT8 )
FMSTR_TSA_RW_VAR(mot_cbl.target_rotation_speed          ,FMSTR_TSA_FLOAT )

FMSTR_TSA_RW_VAR(mot_cbl.opening_direction              ,FMSTR_TSA_UINT8 )
FMSTR_TSA_RW_VAR(mot_cbl.speed_sign_on_open_dir         ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(mot_cbl.en_current_loop                ,FMSTR_TSA_UINT8 )
FMSTR_TSA_RW_VAR(mot_cbl.en_speed_loop                  ,FMSTR_TSA_UINT8 )

FMSTR_TSA_RW_VAR(mot_cbl.direction                      ,FMSTR_TSA_UINT8 )

FMSTR_TSA_RW_VAR(speed_loop_e                           ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(current_loop_e                         ,FMSTR_TSA_FLOAT )

FMSTR_TSA_RW_VAR(hall_u_capt                            ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(hall_v_capt                            ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(hall_w_capt                            ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(one_turn_period                        ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(rotating_direction                     ,FMSTR_TSA_SINT32)

//FMSTR_TSA_RW_VAR(hall_capt                            ,FMSTR_TSA_UINT32)

FMSTR_TSA_RW_VAR(current_hall_state                     ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(hall_u                                 ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(hall_v                                 ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(hall_w                                 ,FMSTR_TSA_UINT8)

FMSTR_TSA_RW_VAR(tmc6200_cbl.u_gstat                    ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(tmc6200_cbl.u_inputs                   ,FMSTR_TSA_UINT32)

FMSTR_TSA_RW_VAR(hall_state                             ,FMSTR_TSA_UINT8)
FMSTR_TSA_RW_VAR(skip_adc_res                           ,FMSTR_TSA_UINT8)

FMSTR_TSA_RW_VAR(tmc6200_cnts.reset                     ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(tmc6200_cnts.drv_otpw                  ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(tmc6200_cnts.drv_ot                    ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(tmc6200_cnts.uv_cp                     ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(tmc6200_cnts.shortdet_u                ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(tmc6200_cnts.s2gu                      ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(tmc6200_cnts.s2vsu                     ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(tmc6200_cnts.shortdet_v                ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(tmc6200_cnts.s2gv                      ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(tmc6200_cnts.s2vsv                     ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(tmc6200_cnts.shortdet_w                ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(tmc6200_cnts.s2gw                      ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(tmc6200_cnts.s2vsw                     ,FMSTR_TSA_UINT32)

FMSTR_TSA_RW_VAR(matlab_time_step                       ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(matlab_time_step_min                   ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(matlab_time_step_max                   ,FMSTR_TSA_UINT32)

FMSTR_TSA_RW_VAR(moving_pulse_ms                        ,FMSTR_TSA_UINT32)

FMSTR_TSA_RW_VAR(pid_i_loop.Kp                          ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(pid_i_loop.Ki                          ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(pid_i_loop.Kd                          ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(pid_i_loop.Kn                          ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(pid_i_loop.OutUpLim                    ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(pid_i_loop.OutLoLim                    ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(pid_i_loop.OutRateLim                  ,FMSTR_TSA_FLOAT )

FMSTR_TSA_RW_VAR(pid_s_loop.Kp                          ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(pid_s_loop.Ki                          ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(pid_s_loop.Kd                          ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(pid_s_loop.Kn                          ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(pid_s_loop.OutUpLim                    ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(pid_s_loop.OutLoLim                    ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(pid_s_loop.OutRateLim                  ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(pid_s_loop.smpl_time                   ,FMSTR_TSA_FLOAT )

FMSTR_TSA_RW_VAR(mc.traj_stage                          ,FMSTR_TSA_UINT8 )
FMSTR_TSA_RW_VAR(mc.stage_time                          ,FMSTR_TSA_FLOAT )
FMSTR_TSA_RW_VAR(mc.estim_pos                           ,FMSTR_TSA_FLOAT )

FMSTR_TSA_TABLE_END();


FMSTR_TSA_TABLE_LIST_BEGIN()

FMSTR_TSA_TABLE(wvar_tbl)
FMSTR_TSA_TABLE(app_vars)

FMSTR_TSA_TABLE_LIST_END()
