/* generated common header file - do not edit */
#ifndef COMMON_DATA_H_
  #define COMMON_DATA_H_
  #include <stdint.h>
  #include "bsp_api.h"
  #include "r_cgc.h"
  #include "r_cgc_api.h"
  #include "r_fmi.h"
  #include "r_fmi_api.h"
  #include "r_ioport.h"
  #include "r_ioport_api.h"
  #include "r_elc.h"
  #include "r_elc_api.h"
  #include "r_sce.h"
  #include "r_aes_api.h"
  #include "r_rsa_api.h"
  #include "r_hash_api.h"
  #include "r_ecc_api.h"
  #include "r_trng_api.h"
  #include "nx_crypto_sce_config.h"

  #ifdef __cplusplus
extern "C"
{
  #endif
  extern const cgc_instance_t g_cgc;
  extern const fmi_instance_t g_fmi;
  extern const ioport_instance_t g_ioport;
  extern const elc_instance_t g_elc;

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* COMMON_DATA_H_ */
