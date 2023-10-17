// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2020-01-31
// 11:14:21
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

extern const flash_instance_t flash_cbl;


/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _Get_Flash_protect_ID_buf_string(uint8_t *buf, char *str)
{
  char s[4];
  for (uint32_t i=0; i < 15; i++)
  {
    buf[i] =*(uint8_t *)(ID_ADDRESS+i);
    if (str != 0)
    {
      sprintf(s, "%02X ", buf[i]);
      if (i == 0) str[0] = 0;
      strcat(str, s);
    }
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param buf
  \param sz

  \return uint8_t - 0 если все байты ID равны 0xFF
-----------------------------------------------------------------------------------------------------*/
static uint8_t _Get_protection_flag(uint8_t *buf, uint8_t sz)
{
  for (uint32_t i=0; i < sz; i++)
  {
    if (buf[i] != 0xFF)
    {
      return 1;
    }
  }
  return 0;
}
/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
static uint8_t _Get_protection_byte(void)
{
  return *(uint8_t *)(ID_ADDRESS+15);
}

/*-----------------------------------------------------------------------------------------------------


  \param pb

  \return const char*
-----------------------------------------------------------------------------------------------------*/
static const char* _Get_protection_state_description(uint8_t pb, uint8_t prot_en)
{

  switch (pb & 0xC0)
  {
  case 0xC0:
    if (prot_en)
    {
      return "Protection activated. All erase anabled, matching ID enabled.";
    }
    else
    {
      return "Protection disactivated.";
    }

  case 0x80:
    return "Protection activated. Matching ID enabled.";
  case 0x00:
    return "Protection activated. Matching ID disabled.";
  }
  return "Protection statius undefined";
}


/*-----------------------------------------------------------------------------------------------------


  \param pb

  \return const char*
-----------------------------------------------------------------------------------------------------*/
static flash_id_code_mode_t _Get_flash_mode(uint8_t pb, uint8_t prot_flag)
{
  switch (pb & 0xC0)
  {
  case 0xC0:
    if (prot_flag)
    {
      return FLASH_ID_CODE_MODE_LOCKED_WITH_ALL_ERASE_SUPPORT;
    }
    else
    {
      return FLASH_ID_CODE_MODE_UNLOCKED;
    }
  case 0x80:
    return FLASH_ID_CODE_MODE_LOCKED;
  case 0x00:
    return FLASH_ID_CODE_MODE_LOCKED;
  }
  return FLASH_ID_CODE_MODE_UNLOCKED;

}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void _Print_header(uint8_t  *id_buf, char *str)
{
  GET_MCBL;

  uint8_t prot_en;

  uint8_t  pb;
  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF(" ===  Flash control ===\n\r");
  MPRINTF("ESC - exit, [E] - edit access ID and flash it, [0] - Set protect with 'All erase' anabled\n\r");
  MPRINTF("[1] - Set protect without 'All erase' anabled, [2] - Set full protect without ID matching\n\r");
  MPRINTF("----------------------------------------------------------------------\n\r");
  // Читаем ID и биты защиты
  _Get_Flash_protect_ID_buf_string(id_buf, str);
  prot_en = _Get_protection_flag(id_buf, 15);
  // Выводим ID
  MPRINTF("ID = %s\n\r", str);
  pb = _Get_protection_byte();
  MPRINTF("Protection byte = %02X\n\r", pb);
  MPRINTF("%s\r\n" ,_Get_protection_state_description(pb, prot_en));
  MPRINTF("----------------------------------------------------------------------\n\r\n\r");
}

/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
void Do_Flash_control(uint8_t keycode)
{
  GET_MCBL;
  uint8_t              b;
  uint8_t              id_buf[16];
  char                 str[61];
  char                 edit_str[61];
  uint32_t             ed[16];
  uint8_t              protection_b;
  flash_id_code_mode_t flash_mode;



  _Print_header(id_buf, str);

  if (flash_cbl.p_api->open(flash_cbl.p_ctrl,flash_cbl.p_cfg) != SSP_SUCCESS)
  {
    MPRINTF("Flash open error.\n\r");
    Wait_ms(1000);
    return;
  }


  protection_b = _Get_protection_byte();
  strcpy(edit_str, str);

  do
  {
    if (WAIT_CHAR(&b, 200) == RES_OK)
    {
      switch (b)
      {
      case '0':
        protection_b = 0xFF;
        break;
      case '1':
        protection_b = 0xBF;
        break;
      case '2':
        protection_b = 0x3F;
        break;


      case 'E':
      case 'e':
        {
          int32_t res;
          MPRINTF(VT100_CURSOR_ON);
          res = Edit_string(edit_str, 60, edit_str);
          MPRINTF(VT100_CURSOR_OFF);
          MPRINTF("\r");
          MPRINTF(VT100_CLR_LINE);

          if (res == RES_OK)
          {
            uint32_t n;
            n = sscanf(edit_str,"%X %X %X %X %X %X %X %X %X %X %X %X %X %X %X",&ed[0],&ed[1],&ed[2],&ed[3],&ed[4],&ed[5],&ed[6],&ed[7],&ed[8],&ed[9],&ed[10],&ed[11],&ed[12],&ed[13],&ed[14]);
            if (n != 15)
            {
              MPRINTF("Error in string.\n\r");
              Wait_ms(1000);
            }
            else
            {
              uint8_t  prot_flag;

              for (uint32_t i=0; i < 15; i++) id_buf[i] = (uint8_t)ed[i];
              id_buf[15] = protection_b;
              prot_flag  = _Get_protection_flag(id_buf, 15);

              MPRINTF("New ID           =  %s\n\r", edit_str);
              MPRINTF("Protection byte  =  %02X (%s)\n\r", protection_b, _Get_protection_state_description(protection_b,  prot_flag));
              MPRINTF("Press Y to programm\n\r");

              if (WAIT_CHAR(&b, 10000) == RES_OK)
              {
                if (b == 'Y')
                {
                  ssp_err_t err;
                  // Записать ID во Flash
                  flash_mode = _Get_flash_mode(protection_b, prot_flag);
                  err = flash_cbl.p_api->idCodeSet(flash_cbl.p_ctrl, id_buf, flash_mode);
                  if (err == SSP_SUCCESS)
                  {
                    MPRINTF("ID code set was done Ok!\n\r");
                  }
                  else
                  {
                    MPRINTF("ID code set executing error %d!\n\r", err);
                  }
                  Wait_ms(1000);
                }
              }
            }
          }
          _Print_header(id_buf, str);
        }
        break;

      case VT100_ESC:
        flash_cbl.p_api->close(flash_cbl.p_ctrl);
        return;
      }
    }

  }while (1);




}


