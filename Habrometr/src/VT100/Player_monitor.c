// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-13
// 14:29:28
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


typedef struct
{
    uint32_t               voan_sound_volume;
    uint32_t               voan_voice_language;
    uint32_t               voan_play_sound_id;
    uint32_t               voan_play_sound_arg_id;
    uint32_t               voan_sound_freq;
} T_sound_diagn_cbl;

T_sound_diagn_cbl  scbl;


/*-----------------------------------------------------------------------------------------------------


  \param row
  \param col
-----------------------------------------------------------------------------------------------------*/
static void Show_Sound_dignostics_header(uint32_t row, uint32_t col)
{
  GET_MCBL;

  scbl.voan_sound_volume                    =    Get_sound_volume();
  scbl.voan_voice_language                  =    Get_voice_language();
  scbl.voan_sound_freq                      =    Get_sound_freq();

  MPRINTF(VT100_CURSOR_SET,row,col);
  MPRINTF(VT100_CLR_LINE"----------------------------------------\n\r");
  MPRINTF(VT100_CLR_LINE"Enable voice announcer = %d\n\r", Get_en_voice_announcer());
  MPRINTF(VT100_CLR_LINE"Voice language ID      = %d\n\r", scbl.voan_voice_language);
  MPRINTF(VT100_CLR_LINE"Sound volume           = %d\n\r", scbl.voan_sound_volume);
  MPRINTF(VT100_CLR_LINE"Sound ID               = %d (%s)\n\r", scbl.voan_play_sound_id, Get_sound_description(scbl.voan_play_sound_id));

  MPRINTF(VT100_CLR_LINE"Sound argument ID      = %d\n\r", scbl.voan_play_sound_arg_id);
  MPRINTF(VT100_CLR_LINE"Sound frequency        = %d\n\r", scbl.voan_sound_freq);
  MPRINTF(VT100_CLR_LINE"----------------------------------------\n\r");

}



/*-----------------------------------------------------------------------------------------------------


  \param keycode
-----------------------------------------------------------------------------------------------------*/
void Do_Sound_dignostics(uint8_t keycode)
{

  uint32_t               row;
  uint32_t               col;
  uint8_t                b;

  GET_MCBL;

  MPRINTF(VT100_CLEAR_AND_HOME);
  MPRINTF(VT100_CLR_LINE"Player diagnostics. ESC-exit\n\r");
  MPRINTF(VT100_CLR_LINE"2 - toggle voice announcer, 4 - toggle language\n\r");
  MPRINTF(VT100_CLR_LINE"5 - dec. volume, 6 - inc. volume,  7 - change sound id, 8 - change sound arg. id, < - dec.freq, > - inc.freq. \n\r");
  MPRINTF(VT100_CLR_LINE"9 - Play sound, 0 - Gen sound, S - Stop sound\n\r");

  row = 5;
  col = 1;

  Show_Sound_dignostics_header(row, col);

  do
  {


    if (WAIT_CHAR(&b, 10) == RES_OK)
    {
      switch (b)
      {
      case '2':
        {
          if (Get_en_voice_announcer() == 0)
          {
            Set_en_voice_announcer(1);
          }
          else
          {
            Set_en_voice_announcer(0);
          }
        }
        break;
      case '4':
        {
          T_player_msg msg = {VOAN_CMD_SETT, VOAN_VOICE_LANGUAGE, 1, 0, 0, 0};
          scbl.voan_voice_language++;
          if (scbl.voan_voice_language > 3) scbl.voan_voice_language = 0;
          memcpy(&msg.data[2],&scbl.voan_voice_language, 4);
          Player_Send_msg_to_queue(&msg);
        }
        break;
      case '5':
        {
          T_player_msg msg = {VOAN_CMD_SETT, VOAN_SOUND_VOLUME, 0, 0, 0, 0};
          if (scbl.voan_sound_volume == 0) scbl.voan_sound_volume = MAX_SOUND_VOLUME;
          else scbl.voan_sound_volume--;
          memcpy(&msg.data[2],&scbl.voan_sound_volume, 4);
          Player_Send_msg_to_queue(&msg);
        }
        break;
      case '6':
        {
          T_player_msg msg = {VOAN_CMD_SETT, VOAN_SOUND_VOLUME, 0, 0, 0, 0};
          scbl.voan_sound_volume++;
          if (scbl.voan_sound_volume > MAX_SOUND_VOLUME) scbl.voan_sound_volume = 0;
          memcpy(&msg.data[2],&scbl.voan_sound_volume, 4);
          Player_Send_msg_to_queue(&msg);
        }
        break;
      case '7':
        {
          scbl.voan_play_sound_id++;
          if (scbl.voan_play_sound_id > VOAN_LAST_MSG_ID) scbl.voan_play_sound_id = 0;
        }
        break;
      case '8':
        {
          scbl.voan_play_sound_arg_id++;
          if (scbl.voan_play_sound_arg_id > 999) scbl.voan_play_sound_arg_id = 0;
        }
        break;
      case '9':
        {
          T_player_msg msg = {VOAN_CMD_PLAY, 0, 0, 0, 0, 0};
          memcpy(&msg.data[1],&scbl.voan_play_sound_id, 1);
          memcpy(&msg.data[2],&scbl.voan_play_sound_arg_id, 4);
          Player_Send_msg_to_queue(&msg);
        }
        break;
      case '0':
        {
          T_player_msg msg = {VOAN_CMD_SOUND_GEN, 0, 0, 0, 0, 0};
          memcpy(&msg.data[1],&scbl.voan_sound_freq, 4);
          Player_Send_msg_to_queue(&msg);
        }
        break;
      case 'S':
      case 's':
        {
          T_player_msg msg = {VOAN_CMD_STOP, 0, 0, 0, 0, 0};
          Player_Send_msg_to_queue(&msg);
        }
        break;
      case '>':
        {
          T_player_msg msg = {VOAN_CMD_SOUND_FREQ, 0, 0, 0, 0, 0};
          scbl.voan_sound_freq += 100;
          if (scbl.voan_sound_freq > 4000) scbl.voan_sound_freq = 4000;
          memcpy(&msg.data[1],&scbl.voan_sound_freq, 4);
          Player_Send_msg_to_queue(&msg);
        }
        break;
      case '<':
        {
          T_player_msg msg = {VOAN_CMD_SOUND_FREQ, 0, 0, 0, 0, 0};
          if (scbl.voan_sound_freq == 0) scbl.voan_sound_freq = 4000;
          else scbl.voan_sound_freq -=100;
          memcpy(&msg.data[1],&scbl.voan_sound_freq, 4);
          Player_Send_msg_to_queue(&msg);
        }
        break;
      case VT100_ESC:
        return;
      default:
        break;
      }
    }
    Show_Sound_dignostics_header(row, col);

  }while (1);
}

