#ifndef JSON_DESERIALIZER_H
  #define JSON_DESERIALIZER_H

uint32_t JSON_Deser_tables(const T_NV_parameters_instance  *p_pars, json_t *root);
uint32_t JSON_Deser_settings(const T_NV_parameters_instance  *p_pars, char *text);
uint32_t JSON_Deser_and_Exec_command(char *text, uint32_t f_long_stream);

#endif



