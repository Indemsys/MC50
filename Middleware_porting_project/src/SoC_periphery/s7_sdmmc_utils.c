// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2020.09.01
// 11:20:28
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "sdcard.h"

extern sdmmc_instance_ctrl_t      sd_card_ctrl;
extern sdmmc_cfg_t                sd_card_cfg;

T_csd  sd_csd;

/*-----------------------------------------------------------------------------------------------------


  \param buf
  \param len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SD_get_password(char **buf, uint8_t *len)
{
  char *pass_str;

  pass_str = Get_Flash_password();
  *buf = pass_str;
  *len = strlen(pass_str);
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
uint8_t SD_fill_lock_struct_with_password(T_Lock_Card_Data_Structure *p_lcds)
{
  uint8_t    len;
  char      *buf;
  memset(p_lcds, 0, sizeof(T_Lock_Card_Data_Structure));

  SD_get_password(&buf,&len);

  p_lcds->pwds_len = len;

  memcpy(p_lcds->pass_data, buf, len);

  // Вычисляем длину блока
  len = len + 2;
  if ((len & 1) != 0) len++; // Длина должна быть четной
  return len;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SD_get_card_locked_satus(uint8_t *p_lock_stat)
{

  sdmmc_priv_card_status_t response = {0U};

  // Переволим карту в Transfer режим.
  if (s7_sdmmc_command_send(&sd_card_ctrl, SDMMC_CMD_SEL_DES_CARD, sd_card_ctrl.status.sdhi_rca << 16) == true)
  {
    response.status = sd_card_ctrl.p_reg->SD_RSP10;

    *p_lock_stat = response.r1.card_is_locked;
    return RES_OK;
  }
  else
  {
    s7_sdmmc_command_send(&sd_card_ctrl, SDMMC_CMD_SEL_DES_CARD, 0) ;
    if (s7_sdmmc_command_send(&sd_card_ctrl, SDMMC_CMD_SEL_DES_CARD, sd_card_ctrl.status.sdhi_rca << 16) != true) return RES_ERROR;
    response.status = sd_card_ctrl.p_reg->SD_RSP10;

    *p_lock_stat = response.r1.card_is_locked;
    return RES_OK;
  }
}

/*-----------------------------------------------------------------------------------------------------
  Операции с паролем SD карты

  \param operation
  \param pass_buff
  \param pass_len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SD_password_operations(uint8_t operation, char *pass_buff, uint8_t pass_len)
{
  T_Lock_Card_Data_Structure  lcds;
  uint8_t len;
  uint8_t lock_stat;

  if ((pass_len == 0) || (pass_len > 16)) return RES_ERROR;

  if (SD_get_card_locked_satus(&lock_stat) == RES_ERROR) return RES_ERROR;

  memset(&lcds, 0, sizeof(lcds));

  if (operation == SD_SET_PASSWORD)
  {
    if (lock_stat != 0) return RES_ERROR;
  }
  else if  (operation == SD_UNLOCK)
  {
    if (lock_stat == 0) return RES_ERROR;
  }
  else if  (operation == SD_CLEAR_PASSWORD)
  {

  }


  len = pass_len + 2;
  // длина блока всегда должна быть четной
  if ((len & 1) != 0) len++;
  if (s7_sdmmc_command_send(&sd_card_ctrl, SDMMC_CMD_SET_BLOCKLEN,len) == true)
  {
    if (operation == SD_SET_PASSWORD)
    {
      lcds.flags.set_pwd = 1;     // Устанавливаем пароль
      lcds.flags.lock_unlock = 1; // Закрываем карту
    }
    else if  (operation == SD_UNLOCK)
    {

    }
    else if  (operation == SD_CLEAR_PASSWORD)
    {
      lcds.flags.clr_pwd = 1;
    }

    lcds.pwds_len = pass_len;
    memcpy(lcds.pass_data, pass_buff, pass_len);
    if (S7_sdmmc_command_w_transfer(&sd_card_ctrl, SDMMC_CMD_LOCK_UNLOCK,0, (uint8_t *)&lcds, len) == SSP_SUCCESS)
    {
      return RES_OK;
    }
  }

  return RES_ERROR;
}




/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SD_card_open(void)
{
  ssp_err_t res;
  res = S7_sdmmc_Open(&sd_card_ctrl,&sd_card_cfg);
  if (res != SSP_SUCCESS) return RES_ERROR;
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SD_card_close(void)
{
  ssp_err_t res;
  res = S7_sdmmc_Close(&sd_card_ctrl);
  if (res != SSP_SUCCESS) return RES_ERROR;
  return RES_OK;
}
/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t SD_card_pre_init(void)
{





  // Переволим карту в StanBy режим. Все карты на шине переходят в это состояние.
  s7_sdmmc_command_send(&sd_card_ctrl, SDMMC_CMD_SEL_DES_CARD, 0);
  {
    // Передаем запрос на чтение CSD карте по адресу находящемуся в sd_card_ctrl.status.sdhi_rca
    // Адрес нужно указывать поскольку карта в этот момент находится в состоянии unselected
    if (s7_sdmmc_command_send(&sd_card_ctrl, SDMMC_CMD_SEND_CSD, sd_card_ctrl.status.sdhi_rca << 16) == true)
    {
      /* SDResponseR2 are bits from 8-127, first 8 MSBs are reserved */
      //      sd_csd.reg.sdrsp10 = sd_card_ctrl.p_reg->SD_RSP10;
      //      sd_csd.reg.sdrsp32 = sd_card_ctrl.p_reg->SD_RSP32;
      //      sd_csd.reg.sdrsp54 = sd_card_ctrl.p_reg->SD_RSP54;
      //      sd_csd.reg.sdrsp76 = sd_card_ctrl.p_reg->SD_RSP76;
    }
  }

  // Переволим карту в Transfer режим.
  //s7_sdmmc_command_send(&sd_card_ctrl, SDMMC_CMD_SEL_DES_CARD, sd_card_ctrl.status.sdhi_rca << 16);

  //SD_password_operations(SD_SET_PASSWORD, SD_PASS, 4);


  return RES_OK;
}


