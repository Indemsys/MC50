#ifndef HAL_DATA_H_
  #define HAL_DATA_H_
  #include <stdint.h>
  #include "bsp_api.h"
  #include "common_data.h"
  #include "r_flash_hp.h"
  #include "r_flash_api.h"
  #include "r_dmac.h"
  #include "r_transfer_api.h"
  #include "r_sdmmc.h"
  #include "r_sdmmc_api.h"
  #include "r_gpt.h"
  #include "r_timer_api.h"
  #include "r_adc.h"
  #include "r_adc_api.h"
  #include "r_rtc.h"
  #include "r_rtc_api.h"
  #include "r_spi_api.h"
  #include "r_i2c_api.h"
  #include "r_sci_spi.h"
  #include "r_riic.h"
  #include "r_dtc.h"
  #include "r_sce.h"
  #include "r_aes_api.h"
  #include "r_rsa_api.h"
  #include "r_hash_api.h"
  #include "r_ecc_api.h"
  #include "r_trng_api.h"


  extern const flash_instance_t g_flash0;
  extern const rtc_instance_t   g_rtc0;

#endif /* HAL_DATA_H_ */
