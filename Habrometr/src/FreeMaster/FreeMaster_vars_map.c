#include "MC50.h"
#include "freemaster_cfg.h"
#include "freemaster.h"
#include "freemaster_tsa.h"


FMSTR_TSA_TABLE_BEGIN(app_vars)
FMSTR_TSA_RW_VAR(g_cpu_usage                            ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(g_aver_cpu_usage                       ,FMSTR_TSA_UINT32)

FMSTR_TSA_TABLE_END();

// Разделено на две таблицы поскольку помещение всех переменных в одну таблицу вызывало сбои в работе протокола во время считывания таблицы
FMSTR_TSA_TABLE_BEGIN(app_vars2)


FMSTR_TSA_RW_VAR(matlab_time_step                       ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(matlab_time_step_min                   ,FMSTR_TSA_UINT32)
FMSTR_TSA_RW_VAR(matlab_time_step_max                   ,FMSTR_TSA_UINT32)

FMSTR_TSA_TABLE_END();


FMSTR_TSA_TABLE_LIST_BEGIN()

FMSTR_TSA_TABLE(wvar_vars)
FMSTR_TSA_TABLE(app_vars)
FMSTR_TSA_TABLE(app_vars2)

FMSTR_TSA_TABLE_LIST_END()
