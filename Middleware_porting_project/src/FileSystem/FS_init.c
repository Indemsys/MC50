// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.08.29
// 18:42:07
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"



FX_MEDIA                                     fat_fs_media;
sdmmc_instance_ctrl_t                        sd_card_ctrl;
static sf_block_media_sdmmc_instance_ctrl_t  sd_card_media_ctrl;
uint8_t                                      fs_memory[FS_MEMORY_SZ] BSP_ALIGN_VARIABLE_V2(64) BSP_PLACE_IN_SECTION_V2(".mempools");



const sdmmc_extended_cfg_t sd_card_ext_cfg =
{
  .block_size      = 512,
  .card_detect     = SDMMC_CARD_DETECT_NONE,
  .write_protect   = SDMMC_WRITE_PROTECT_NONE,
};

sdmmc_cfg_t sd_card_cfg =
{
  .hw =
  {
    .media_type     = SDMMC_MEDIA_TYPE_EMBEDDED,
    .bus_width      = SDMMC_BUS_WIDTH_4_BITS,
    .channel        = 0,
  },
  .p_callback       = NULL,
  .p_extend         = (void *)&sd_card_ext_cfg,
  .p_lower_lvl_transfer   =&DMA_CH2_transfer_instance,
  .access_ipl       =(12),
  .sdio_ipl         = BSP_IRQ_DISABLED,
  .card_ipl         =(BSP_IRQ_DISABLED),
  .dma_req_ipl      =(BSP_IRQ_DISABLED),
};

const sdmmc_instance_t sd_card_cbl =
{
  .p_ctrl           =&sd_card_ctrl,
  .p_cfg            =&sd_card_cfg,
  .p_api            =&g_sdmmc_on_sdmmc //  &g_s7_sdmmc
};

static const sf_block_media_on_sdmmc_cfg_t sd_card_media_ext_cfg =
{
  .p_lower_lvl_sdmmc  =&sd_card_cbl,
};

static sf_block_media_cfg_t sd_card_media_cfg =
{
  .block_size          = 512,
  .p_extend            =&sd_card_media_ext_cfg
};

sf_block_media_instance_t sd_card_media_cbl =
{
  .p_ctrl =&sd_card_media_ctrl,
  .p_cfg =&sd_card_media_cfg,
  .p_api =&g_sf_block_media_on_sdmmc
};


#define g_sf_el_fx0_total_partition       0U

#if (g_sf_el_fx0_total_partition > 1U)
sf_el_fx_media_partition_data_info_t g_sf_el_fx0_partition_data_info[g_sf_el_fx0_total_partition];
#endif

sf_el_fx_instance_ctrl_t g_sf_el_fx0_ctrl;

extern sf_el_fx_t fat_fs_media_cfg;

/** SF_EL_FX interface configuration */
const sf_el_fx_config_t g_sf_el_fx0_config =
{
#if (g_sf_el_fx0_total_partition > 1U)
  .p_partition_data        = (sf_el_fx_media_partition_data_info_t *)g_sf_el_fx0_partition_data_info,
#else
  .p_partition_data = NULL,
#endif
  .p_lower_lvl_block_media =&sd_card_media_cbl,
  .p_context =&fat_fs_media_cfg,
  .p_extend = NULL,
  .total_partitions = g_sf_el_fx0_total_partition,
#if (g_sf_el_fx0_total_partition > 1U)
  .p_callback              = NULL,
#else
  .p_callback = NULL,
#endif
};


sf_el_fx_t fat_fs_media_cfg =
{
  .p_ctrl =&g_sf_el_fx0_ctrl,
  .p_config =&g_sf_el_fx0_config,
};

#define ERROR_CORR_BUF_SZ           0x20000   // Размер буфера для функции коррекции ошибок

static T_file_system_init_results   fs_res;



static char  flash_password[33];
/*-----------------------------------------------------------------------------------------------------
  Получить строку пароля на основе кода ID

  \param str
-----------------------------------------------------------------------------------------------------*/
char* Get_Flash_password(void)
{
  uint8_t b;
  uint8_t flag = 0;
  char    s[4];

  flash_password[0] = 0;
  for (uint32_t i=0; i < 15; i++)
  {
    b =*(uint8_t *)(ID_ADDRESS+i);
    if (((flag == 0) && (b != 0xFF)) || (flag != 0))
    {
      flag = 1;
      sprintf(s, "%02X", b);
      strcat(flash_password, s);
    }
  }
  return flash_password;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return T_file_system_init_results*
-----------------------------------------------------------------------------------------------------*/
T_file_system_init_results* Get_FS_init_res(void)
{
  return &fs_res;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Get_fs_memory_size(void)
{
  return sizeof(fs_memory);
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Init_SD_card_file_system(void)
{
  rtc_time_t   rt_time = {0};

  UCHAR       *buf;

  g_file_system_ready = 0;
  fx_system_initialize();

  rtc_cbl.p_api->calendarTimeGet(rtc_cbl.p_ctrl,&rt_time);
  rt_time.tm_mon++; // Renesas SSP счет месяцев начинается с 0

  fx_system_date_set(rt_time.tm_year+1900 , rt_time.tm_mon , rt_time.tm_mday);
  fx_system_time_set(rt_time.tm_hour, rt_time.tm_min, rt_time.tm_sec);


  fs_res.fx_media_open_result = fx_media_open(&fat_fs_media, (CHAR *)"C:", SF_EL_FX_BlockDriver,&fat_fs_media_cfg, fs_memory, sizeof(fs_memory));
  if (fs_res.fx_media_open_result != FX_SUCCESS)
  {
    return RES_ERROR;
  }

  buf = App_malloc_pending(ERROR_CORR_BUF_SZ, 10);
  if  (buf != NULL) // Выделеям 64 Кб памяти для функции исправления ошибок FAT
  {
    fs_res.fs_check_error_code = fx_media_check(&fat_fs_media,buf, ERROR_CORR_BUF_SZ, FX_FAT_CHAIN_ERROR | FX_DIRECTORY_ERROR | FX_LOST_CLUSTER_ERROR,(ULONG *)&(fs_res.fs_detected_errors));
    App_free(buf);
    if (fs_res.fs_check_error_code != FX_SUCCESS) return RES_ERROR;
  }
  else
  {
    fs_res.fx_corr_buff_alloc_res = RES_ERROR;
  }

  // Получаем строку пути по умолчанию
  fx_directory_default_set(&fat_fs_media, "/");

  // Удаляем служебную директорию Windows
  if (fx_directory_name_test(&fat_fs_media, WINDOWS_DIR) == FX_SUCCESS)
  {
    fx_directory_default_set(&fat_fs_media, WINDOWS_DIR);
    Delete_all_files_in_current_dir();
    fx_directory_default_set(&fat_fs_media, "/");
    fx_directory_delete(&fat_fs_media, WINDOWS_DIR);

  }

  fx_directory_default_set(&fat_fs_media, "/");

  fx_media_flush(&fat_fs_media);

  g_file_system_ready = 1;
  return RES_OK;
}


