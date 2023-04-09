// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2020.09.02
// 9:18:17
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


/*-----------------------------------------------------------------------------------------------------


  \param v

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t SD_card_get_transfer_speed(uint8_t v)
{
  uint32_t  tr = v & 0x7;
  uint32_t  tv =(v >> 3) & 0xF;
  uint32_t  s;

  switch (tr)
  {
  case 0:
    s = 10;
    break;
  case 1:
    s = 100;
    break;
  case 2:
    s = 1000;
    break;
  case 3:
    s = 10000;
    break;
  default:
    s = 100000;
    break;
  }

  switch (tv)
  {
  case 0  :
    return 0  * s;
  case 1  :
    return 10 * s;
  case 2  :
    return 12 * s;
  case 3  :
    return 13 * s;
  case 4  :
    return 15 * s;
  case 5  :
    return 20 * s;
  case 6  :
    return 25 * s;
  case 7  :
    return 30 * s;
  case 8  :
    return 35 * s;
  case 9  :
    return 40 * s;
  case 10 :
    return 45 * s;
  case 11 :
    return 50 * s;
  case 12 :
    return 55 * s;
  case 13 :
    return 60 * s;
  case 14 :
    return 70 * s;
  case 15 :
    return 80 * s;
  }
  return 0;
}

/*-----------------------------------------------------------------------------------------------------


  \param mult
  \param c_size
  \param bl_len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint32_t SD_csd1_card_get_size(uint32_t mult, uint32_t c_size, uint32_t bl_len)
{
  uint32_t m = 0;
  uint32_t bl = 0;

  switch (mult)
  {
  case 0 :
    m = 4;
    break;
  case 1 :
    m = 8;
    break;
  case 2 :
    m = 16;
    break;
  case 3 :
    m = 32;
    break;
  case 4 :
    m = 64;
    break;
  case 5 :
    m = 128;
    break;
  case 6 :
    m = 256;
    break;
  case 7 :
    m = 512;
    break;
  }

  switch (bl_len)
  {
  case 9  :
    bl = 512;
    break;
  case 10 :
    bl = 1024;
    break;
  case 11 :
    bl = 2048;
    break;
  }

  return m * (c_size+1) * bl;
}

/*-----------------------------------------------------------------------------------------------------


  \param mult
  \param c_size
  \param bl_len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
static uint64_t SD_csd2_card_get_size(uint32_t c_size)
{
  return (c_size+1) * 512ll * 1024ll;
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _Print_header(void)
{
  T_sd_unlock_status *p_st;
  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF(" ===  SD card control ===\n\r");
  MPRINTF("ESC - exit,  [Q]- SD card CSD info, [X] - Clear password, [S] - Set password \n\r");
  MPRINTF("----------------------------------------------------------------------\n\r");

  p_st= Get_sd_unlock_status();

  if (p_st->sd_unlock_no_need != 0)
  {
    MPRINTF("On start SD card was not locked\r\n");
  }
  else
  {
    if (p_st->sd_unlock_executed != 0)
    {
      MPRINTF("On start SD was unlocked successfuly\r\n");
    }
    else
    {
      MPRINTF("On start SD card unlock was failed\r\n");
    }
  }
  if (g_file_system_ready == 1)
  {
    MPRINTF("File system on SD card ready\r\n");
  }
  else
  {
    MPRINTF("File system on SD card fail!\r\n");
  }
  MPRINTF("----------------------------------------------------------------------\n\r\n\r");
}

/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
void Do_SD_card_control(uint8_t keycode)
{
  GET_MCBL;
  uint8_t              b;
  uint32_t             res;

  _Print_header();

  // Закрываем медиа
  res = fx_media_close(&fat_fs_media);
  if ((res != FX_SUCCESS) && (res!=FX_MEDIA_NOT_OPEN))
  {
    MPRINTF("Error during media closing.\r\n");
    MPRINTF("Press any key to continue.\r\n");
    WAIT_CHAR(&b,  ms_to_ticks(100000));
  }
  else
  {
    MPRINTF("Media closed successfully.\r\n");
  }

  res = SD_card_open();
  if (res != FX_SUCCESS)
  {
    MPRINTF("Error during SD card opening.\r\n");
    MPRINTF("Press any key to continue.\r\n");
    WAIT_CHAR(&b,  ms_to_ticks(100000));
  }
  else
  {
    MPRINTF("SD card open successfully.\r\n");
  }



  do
  {
    if (WAIT_CHAR(&b, 200) == RES_OK)
    {
      switch (b)
      {
      case 'Q':
      case 'q':
        {
          sdmmc_priv_csd_reg_t *p_csd_reg = Get_csd_reg();
          MPRINTF("SD card info:\r\n");

          if (p_csd_reg->csd_v1_b.csd_structure == 0)
          {
            MPRINTF("CSD Version 1.0\r\n");
            MPRINTF("file_format        = %d  \r\n",p_csd_reg->csd_v1_b.file_format);
            MPRINTF("tmp_write_protect  = %d  \r\n",p_csd_reg->csd_v1_b.tmp_write_protect);
            MPRINTF("perm_write_protect = %d  \r\n",p_csd_reg->csd_v1_b.perm_write_protect);
            MPRINTF("copy               = %d  \r\n",p_csd_reg->csd_v1_b.copy);
            MPRINTF("file_format_grp    = %d  \r\n",p_csd_reg->csd_v1_b.file_format_grp);
            MPRINTF("write_bl_partial   = %d  \r\n",p_csd_reg->csd_v1_b.write_bl_partial);
            MPRINTF("write_bl_len       = %d  \r\n",p_csd_reg->csd_v1_b.write_bl_len);
            MPRINTF("r2w_factor         = %d  \r\n",p_csd_reg->csd_v1_b.r2w_factor);
            MPRINTF("wp_grp_enable      = %d  \r\n",p_csd_reg->csd_v1_b.wp_grp_enable);
            MPRINTF("wp_grp_size        = %d  \r\n",p_csd_reg->csd_v1_b.wp_grp_size);
            MPRINTF("sector_size        = %d  \r\n",p_csd_reg->csd_v1_b.sector_size);
            MPRINTF("erase_blk_en       = %d  \r\n",p_csd_reg->csd_v1_b.erase_blk_en);
            MPRINTF("c_size_mult        = %d  \r\n",p_csd_reg->csd_v1_b.c_size_mult);
            MPRINTF("vdd_w_curr_max     = %d  \r\n",p_csd_reg->csd_v1_b.vdd_w_curr_max);
            MPRINTF("vdd_w_curr_min     = %d  \r\n",p_csd_reg->csd_v1_b.vdd_w_curr_min);
            MPRINTF("vdd_r_curr_max     = %d  \r\n",p_csd_reg->csd_v1_b.vdd_r_curr_max);
            MPRINTF("vdd_r_curr_min     = %d  \r\n",p_csd_reg->csd_v1_b.vdd_r_curr_min);
            MPRINTF("c_size             = %d  (%d bytes)\r\n",p_csd_reg->csd_v1_b.c_size, SD_csd1_card_get_size(p_csd_reg->csd_v1_b.c_size_mult, p_csd_reg->csd_v1_b.c_size, p_csd_reg->csd_v1_b.read_bl_len));
            MPRINTF("dsr_imp            = %d  \r\n",p_csd_reg->csd_v1_b.dsr_imp);
            MPRINTF("read_blk_misalign  = %d  \r\n",p_csd_reg->csd_v1_b.read_blk_misalign);
            MPRINTF("write_blk_misalign = %d  \r\n",p_csd_reg->csd_v1_b.write_blk_misalign);
            MPRINTF("read_bl_partial    = %d  \r\n",p_csd_reg->csd_v1_b.read_bl_partial);
            MPRINTF("read_bl_len        = %d  \r\n",p_csd_reg->csd_v1_b.read_bl_len);
            MPRINTF("ccc                = %04X\r\n",p_csd_reg->csd_v1_b.ccc);
            MPRINTF("tran_speed         = %d  (%d kbit/s) \r\n",p_csd_reg->csd_v1_b.tran_speed, SD_card_get_transfer_speed(p_csd_reg->csd_v1_b.tran_speed));
            MPRINTF("nsac               = %d  \r\n",p_csd_reg->csd_v1_b.nsac);
            MPRINTF("taac               = %d  \r\n",p_csd_reg->csd_v1_b.taac);
            MPRINTF("csd_structure      = %d  \r\n",p_csd_reg->csd_v1_b.csd_structure);
          }
          if (p_csd_reg->csd_v2_b.csd_structure == 1)
          {
            MPRINTF("CSD Version 2.0\r\n");
            MPRINTF("file_format        = %d  \r\n",p_csd_reg->csd_v2_b.file_format);
            MPRINTF("tmp_write_protect  = %d  \r\n",p_csd_reg->csd_v2_b.tmp_write_protect);
            MPRINTF("perm_write_protect = %d  \r\n",p_csd_reg->csd_v2_b.perm_write_protect);
            MPRINTF("copy               = %d  \r\n",p_csd_reg->csd_v2_b.copy);
            MPRINTF("file_format_grp    = %d  \r\n",p_csd_reg->csd_v2_b.file_format_grp);
            MPRINTF("write_bl_partial   = %d  \r\n",p_csd_reg->csd_v2_b.write_bl_partial);
            MPRINTF("write_bl_len       = %d  \r\n",p_csd_reg->csd_v2_b.write_bl_len);
            MPRINTF("r2w_factor         = %d  \r\n",p_csd_reg->csd_v2_b.r2w_factor);
            MPRINTF("wp_grp_enable      = %d  \r\n",p_csd_reg->csd_v2_b.wp_grp_enable);
            MPRINTF("wp_grp_size        = %d  \r\n",p_csd_reg->csd_v2_b.wp_grp_size);
            MPRINTF("sector_size        = %d  \r\n",p_csd_reg->csd_v2_b.sector_size);
            MPRINTF("erase_blk_en       = %d  \r\n",p_csd_reg->csd_v2_b.erase_blk_en);
            MPRINTF("c_size             = %d  (%lld bytes)\r\n",p_csd_reg->csd_v2_b.c_size, SD_csd2_card_get_size(p_csd_reg->csd_v2_b.c_size));
            MPRINTF("dsr_imp            = %d  \r\n",p_csd_reg->csd_v2_b.dsr_imp);
            MPRINTF("read_blk_misalign  = %d  \r\n",p_csd_reg->csd_v2_b.read_blk_misalign);
            MPRINTF("write_blk_misalign = %d  \r\n",p_csd_reg->csd_v2_b.write_blk_misalign);
            MPRINTF("read_bl_partial    = %d  \r\n",p_csd_reg->csd_v2_b.read_bl_partial);
            MPRINTF("read_bl_len        = %d  \r\n",p_csd_reg->csd_v2_b.read_bl_len);
            MPRINTF("ccc                = %d  \r\n",p_csd_reg->csd_v2_b.ccc);
            MPRINTF("tran_speed         = %d  (%d kbit/s) \r\n",p_csd_reg->csd_v2_b.tran_speed, SD_card_get_transfer_speed(p_csd_reg->csd_v2_b.tran_speed));
            MPRINTF("nsac               = %d  \r\n",p_csd_reg->csd_v2_b.nsac);
            MPRINTF("taac               = %d  \r\n",p_csd_reg->csd_v2_b.taac);
            MPRINTF("csd_structure      = %d  \r\n",p_csd_reg->csd_v2_b.csd_structure);
          }
          MPRINTF("\r\nPress any key to continue.\r\n");
          WAIT_CHAR(&b,  ms_to_ticks(100000));
          _Print_header();
        }
        break;

      case 'X':
      case 'x':
        // Очистка пароля
        {
          char   *buf;
          uint8_t len;
          SD_get_password(&buf,&len);
          if (SD_password_operations(SD_CLEAR_PASSWORD, buf, len)== RES_OK)
          {
            MPRINTF("\r\nPassword cleared successfully.\r\n");
          }
          else
          {
            MPRINTF("\r\nError during password clearing.\r\n");
          }
          MPRINTF("\r\nPress any key to continue.\r\n");
          WAIT_CHAR(&b,  ms_to_ticks(100000));
          _Print_header();
        }
        break;

      case 'S':
      case 's':
        // Очистка пароля
        {
          char   *buf;
          uint8_t len;
          SD_get_password(&buf,&len);
          if (SD_password_operations(SD_SET_PASSWORD, buf, len)== RES_OK)
          {
            MPRINTF("\r\nPassword set successfully.\r\n");
          }
          else
          {
            MPRINTF("\r\nError during password setting.\r\n");
          }
          MPRINTF("\r\nPress any key to continue.\r\n");
          WAIT_CHAR(&b,  ms_to_ticks(100000));
          _Print_header();
        }
        break;


      case VT100_ESC:
        return;
      }
    }

  }while (1);




}



