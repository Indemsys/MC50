#include "common_data.h"


//........................................................................................
//  Организация обмена с криптографическим модулем
//........................................................................................

sce_interface_get_api_interfaces_t g_sce_selected_api_interfaces =
{
  .aes128ecb_on_sce     = (uint32_t)&g_aes128ecb_on_sce,
  .aes128cbc_on_sce     = (uint32_t)&g_aes128cbc_on_sce,
  .aes128ctr_on_sce     = (uint32_t)&g_aes128ctr_on_sce,
  .aes128gcm_on_sce     = (uint32_t)&g_aes128gcm_on_sce,
  .aes128xts_on_sce     = (uint32_t)&g_aes128xts_on_sce,
  .aes192ecb_on_sce     = (uint32_t)&g_aes192ecb_on_sce,
  .aes192cbc_on_sce     = (uint32_t)&g_aes192cbc_on_sce,
  .aes192ctr_on_sce     = (uint32_t)&g_aes192ctr_on_sce,
  .aes192gcm_on_sce     = (uint32_t)&g_aes192gcm_on_sce,
  .aes256ecb_on_sce     = (uint32_t)&g_aes256ecb_on_sce,
  .aes256cbc_on_sce     = (uint32_t)&g_aes256cbc_on_sce,
  .aes256ctr_on_sce     = (uint32_t)&g_aes256ctr_on_sce,
  .aes256gcm_on_sce     = (uint32_t)&g_aes256gcm_on_sce,
  .aes256xts_on_sce     = (uint32_t)&g_aes256xts_on_sce,
  .aes128ecb_on_sceHrk  = (uint32_t)&g_aes128ecb_on_sceHrk,
  .aes128cbc_on_sceHrk  = (uint32_t)&g_aes128cbc_on_sceHrk,
  .aes128ctr_on_sceHrk  = (uint32_t)&g_aes128ctr_on_sceHrk,
  .aes128gcm_on_sceHrk  = (uint32_t)&g_aes128gcm_on_sceHrk,
  .aes128xts_on_sceHrk  = (uint32_t)&g_aes128xts_on_sceHrk,
  .aes192ecb_on_sceHrk  = (uint32_t)&g_aes192ecb_on_sceHrk,
  .aes192cbc_on_sceHrk  = (uint32_t)&g_aes192cbc_on_sceHrk,
  .aes192ctr_on_sceHrk  = (uint32_t)&g_aes192ctr_on_sceHrk,
  .aes192gcm_on_sceHrk  = (uint32_t)&g_aes192gcm_on_sceHrk,
  .aes256ecb_on_sceHrk  = (uint32_t)&g_aes256ecb_on_sceHrk,
  .aes256cbc_on_sceHrk  = (uint32_t)&g_aes256cbc_on_sceHrk,
  .aes256ctr_on_sceHrk  = (uint32_t)&g_aes256ctr_on_sceHrk,
  .aes256gcm_on_sceHrk  = (uint32_t)&g_aes256gcm_on_sceHrk,
  .aes256xts_on_sceHrk  = (uint32_t)&g_aes256xts_on_sceHrk,
  .rsa1024_on_sce       = (uint32_t)&g_rsa1024_on_sce,
  .rsa2048_on_sce       = (uint32_t)&g_rsa2048_on_sce,
  .rsa1024_on_sce_hrk   = (uint32_t)&g_rsa1024_on_sce_hrk,
  .rsa2048_on_sce_hrk   = (uint32_t)&g_rsa2048_on_sce_hrk,
  .md5_hash_on_sce      = (uint32_t)&g_md5_hash_on_sce,
  .sha1_hash_on_sce     = (uint32_t)&g_sha1_hash_on_sce,
  .sha256_hash_on_sce   = (uint32_t)&g_sha256_hash_on_sce,
  .ecc192_on_sce        = (uint32_t)&g_ecc192_on_sce,
  .ecc256_on_sce        = (uint32_t)&g_ecc256_on_sce,
  .ecc192_on_sce_hrk    = (uint32_t)&g_ecc192_on_sce_hrk,
  .ecc256_on_sce_hrk    = (uint32_t)&g_ecc256_on_sce_hrk,
  .trng_on_sce          = (uint32_t)&g_trng_on_sce
};

crypto_ctrl_t g_sce_0_ctrl;

crypto_cfg_t  g_sce_0_cfg =
{
  .p_sce_long_plg_start_callback = NULL,
  .p_sce_long_plg_end_callback   = NULL,
  .endian_flag = CRYPTO_WORD_ENDIAN_LITTLE,
  .p_sce_api_interfaces =&g_sce_selected_api_interfaces
};

const crypto_instance_t g_sce_0 =
{
  .p_ctrl =&g_sce_0_ctrl ,
  .p_cfg  =&g_sce_0_cfg  ,
  .p_api  =&g_sce_crypto_api
};

const crypto_instance_t *p_lower_lvl_hw_crypto = (crypto_instance_t *)&g_sce_0;


const cgc_instance_t g_cgc =
{
  .p_api =&g_cgc_on_cgc,
  .p_cfg = NULL
};

const fmi_instance_t g_fmi =
{
  .p_api =&g_fmi_on_fmi
};

const ioport_instance_t g_ioport =
{
  .p_api =&g_ioport_on_ioport,
  .p_cfg = NULL };

const elc_instance_t g_elc =
{
  .p_api =&g_elc_on_elc,
  .p_cfg = NULL
};


