#ifndef JSON_SERIALIZER_H
  #define JSON_SERIALIZER_H



uint32_t    Serialze_settings_schema_to_JSON_file(const T_NV_parameters_instance *p_pars, char* filename, size_t flags);
uint32_t    Serialze_settings_to_JSON_file(const T_NV_parameters_instance *p_pars, char *filename, size_t flags);
uint32_t    Serialze_settings_to_mem(const T_NV_parameters_instance *p_pars, char **mem, uint32_t *str_size, uint32_t  flags);

uint32_t    Write_json_str_to_file(FX_FILE *p_file, size_t flags, char *str);
uint32_t    Serialize_device_description_to_file(FX_FILE *p_file, size_t flags, char *obj_name);
uint32_t    Serialze_device_state_to_mem(char **mem, uint32_t *str_size);
uint32_t    Serialze_Ack_message(char **mem, uint32_t *str_size, uint32_t ack_code, const char *ack_str);

#endif



