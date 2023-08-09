// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2020.07.13
// 16:29:15
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"


const T_vannouncer_item vannouncer_map[] =
{
  { VOAN_MSG_ERROR_               , Play_error_number_msg         , "ERROR MESSAGE       " },  //
  { VOAN_MSG_OVERLOAD             , Play_msg_overload             , "OVERLOAD MESSAGE    " },  //
  { VOAN_MSG_ALARM                , Play_msg_alarm                , "ALARM MESSAGE       " },  //
  { VOAN_MSG_KARMA                , Play_msg_karma                , "KARMA MESSAGE       " },  //                                                                                                            
};

#define  SIZEOF_SOUNDS_MAP   (sizeof(vannouncer_map)/ sizeof(vannouncer_map[0]) )

/*------------------------------------------------------------------------------
  Обработка команд приходящих по CAN
 ------------------------------------------------------------------------------*/
void VAnnouncer_messages_processing(const uint8_t *buf)
{
  uint32_t val;

  if (buf[0] == VOAN_CMD_SETT)
  {
    memcpy(&val,&buf[2], 4);
    switch (buf[1])
    {
    case VOAN_EN_VOICE_ANNOUNCER :
      Set_en_voice_announcer(val);
      LOGs(__FUNCTION__, __LINE__, 0, "Voice announcer mode = %d", val);
      break;
    case VOAN_SOUND_VOLUME  :
      Set_sound_loudness(val);
      LOGs(__FUNCTION__, __LINE__, 0, "Sound attenuation = %d", val);
      break;
    case VOAN_VOICE_LANGUAGE:
      Set_en_voice_laguage(val);
      LOGs(__FUNCTION__, __LINE__, 0, "Voice laguage = %d", val);
      break;
    }
  }
  else if (buf[0] == VOAN_CMD_PLAY)
  {
    uint32_t msg_num;
    uint32_t msg_arg;

    msg_num = buf[1];
    memcpy(&msg_arg,&buf[2], 4);
    Play_msg_by_num(msg_num, msg_arg);
    LOGs(__FUNCTION__, __LINE__, 0, "Play command = %d, arg = %d", msg_num, msg_arg);
  }
  else if (buf[0] == VOAN_CMD_SOUND_FREQ)
  {
    memcpy(&val,&buf[1], 4);
    Set_sound_freq(val);
    LOGs(__FUNCTION__, __LINE__, 0, "Sound frequency = %d", val);
  }
  else if (buf[0] == VOAN_CMD_SOUND_GEN)
  {
    Player_Play_sound(Get_sound_freq());
    LOGs(__FUNCTION__, __LINE__, 0, "Play sound %d Hz", Get_sound_freq());
  }
  else if (buf[0] == VOAN_CMD_STOP)
  {
    Player_Stop();
    LOGs(__FUNCTION__, __LINE__, 0, "Stop play");
  }
}


/*------------------------------------------------------------------------------
   Получить префикс для пути к директории с аудиофайлами
 ------------------------------------------------------------------------------*/
const char* VAnnouncer_get_files_dir(void)
{
  switch (Get_voice_language())
  {
  case 0:
    return DIR_ENG;
  case 1:
    return DIR_RUS;
  default:
    return DIR_ENG;
  }
}


/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
void Play_msg_by_num(unsigned int  msg_num, int msg_arg)
{
  unsigned int i;
  unsigned int n = sizeof(vannouncer_map) / sizeof(vannouncer_map[0]);

  for (i = 0; i < n; i++)
  {
    if (vannouncer_map[i].msg_id == msg_num)
    {
      vannouncer_map[i].play_func(msg_arg);
      return;
    }
  }
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void Play_error_number_msg(int num)
{
  if (Get_en_voice_announcer())
  {
    Player_Enqueue_file(SND_GONG);
    Player_Enqueue_file(SND_SILENCE);
    Player_Enqueue_file((num / 100)% 10);
    Player_Enqueue_file(SND_SILENCE);
    Player_Enqueue_file((num / 10)% 10);
    Player_Enqueue_file(SND_SILENCE);
    Player_Enqueue_file(num % 10);
    Player_Enqueue_file(SND_SILENCE);
  }
}

/*-----------------------------------------------------------------------------------------------------
  
  
  \param num  
-----------------------------------------------------------------------------------------------------*/
void Play_msg_karma(int num)
{
  if (Get_en_voice_announcer())
  {
    Player_Enqueue_file(SND_GONG);
    Player_Enqueue_file(SND_KARMA);
    Player_Enqueue_file(SND_SILENCE);
    Player_Enqueue_file((num / 100)% 10);
    Player_Enqueue_file(SND_SILENCE);
    Player_Enqueue_file((num / 10)% 10);
    Player_Enqueue_file(SND_SILENCE);
    Player_Enqueue_file(num % 10);
    Player_Enqueue_file(SND_SILENCE);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Play_msg_alarm(int num)
{
  if (Get_en_voice_announcer())
  {
    Player_Enqueue_file(SND_ALARM);
  }
  else
  {
    Player_Play_sound(440);
  }
  Player_Delayed_Stop();
}


/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
void Play_msg_overload(int num)
{
  if (Get_en_voice_announcer())
  {
    Player_Enqueue_file(SND_GONG);
    Player_Enqueue_file(SND_OVERLOAD);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param sid

  \return const char*
-----------------------------------------------------------------------------------------------------*/
const char* Get_sound_description(uint32_t sid)
{
  if (sid >= SIZEOF_SOUNDS_MAP)
  {
    return "Undefined";
  }

  return vannouncer_map[sid].description;

}




