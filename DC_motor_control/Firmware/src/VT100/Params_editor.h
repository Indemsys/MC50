#ifndef PARAMS_EDITOR_H
  #define PARAMS_EDITOR_H

#define           MAX_PARAMETER_STRING_LEN 1024


void              Edit_func(uint8_t b);
void              Params_editor_press_key_handler(uint8_t b);
void              Save_str_to_logfile(char* str);
void              Close_logfile(void);
void              Convert_parameter_to_str(const T_NV_parameters_instance  *p_pars, uint8_t *buf,uint16_t maxlen, uint16_t indx);
void              Convert_str_to_parameter(const T_NV_parameters_instance  *p_pars, uint8_t *buf,uint16_t indx);
uint8_t*          Get_mn_name(const T_NV_parameters_instance *p_pars, uint32_t menu_lev);
int32_t           Find_param_by_alias(const T_NV_parameters_instance  *p_pars, char* alias);
int32_t           Find_param_by_name(const T_NV_parameters_instance  *p_pars, char *name);
const char*       Convrt_var_type_to_str(enum  vartypes  vartype);
void              Show_parameters_menu(void);
#endif
