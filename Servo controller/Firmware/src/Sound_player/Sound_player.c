#include "MC50.h"


extern int            Read_wav_header_from_file(FX_FILE *fp);
extern T_WAV_pars_err Wavefile_header_parsing(void);

static T_sound_player       ply;
static TX_EVENT_FLAGS_GROUP play_flags;
static uint8_t              f_delayed_stop = 0;
static T_sys_timestump      delayed_stop_start;


#define PLAYER_TASK_STACK_SIZE 2048
static uint8_t player_stacks[PLAYER_TASK_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.player_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static TX_THREAD       player_thread;

#define PLAYER_MESAGGES_QUEUE_SZ  20
TX_QUEUE               player_queue;
T_player_msg           player_queue_buf[PLAYER_MESAGGES_QUEUE_SZ];


T_sound_player_settings   sp_set;

uint32_t Get_en_voice_announcer(void)                   { return  sp_set.en_voice_announcer;}
uint32_t Get_sound_volume(void)                         { return  sp_set.sound_volume;}
uint32_t Get_voice_language(void)                       { return  sp_set.voice_language;}
uint32_t Get_sound_freq(void)                           { return  sp_set.sound_freq;}

void     Set_en_voice_announcer(uint32_t val)           { sp_set.en_voice_announcer = val;}
void     Set_en_voice_laguage(uint32_t val)             { sp_set.voice_language = val;    }
void     Set_sound_freq(uint32_t val)                   { sp_set.sound_freq = val;        }
void     Set_sound_loudness(uint32_t val)
{
  sp_set.sound_volume = val;
  Player_Set_Volume(sp_set.sound_volume);
}

/*-----------------------------------------------------------------------------------------------------


  \param val
-----------------------------------------------------------------------------------------------------*/
void Player_Set_MUTE(uint8_t state)
{
  // Тут надо разрешить или запретить воспроизведение звука
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Player_Set_evt_to_read_next_block(void)
{
  tx_event_flags_set(&play_flags, EVT_READ_NEXT, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Audio_set_evt_to_play_file(void)
{
  tx_event_flags_set(&play_flags, EVT_PLAY_FILE, TX_OR);
}


/*------------------------------------------------------------------------------
   Преобразование адиосэмплов файла в формат кодека
   16-битные отсчеты конвертируем в 12-битные и применяем аттенюацию

   Преобразование 512 отсчетов (1024 байта) длится 57 мкс = 18 мегабайт в сек
 ------------------------------------------------------------------------------*/
static void _Audio_convert_samlpes(int16_t *src, int16_t *dest, uint32_t samples_num, uint8_t attenuation)
{
  uint32_t  i;
  int16_t   sample;

  for (i = 0; i < samples_num; i++)
  {
    sample =*src;
    sample = sample >> 4;
    sample = sample >> attenuation;
    sample = sample +  DAC_MID_LEVEL;
    *dest =  sample;
    dest++;
    src++;
  }
}


/*------------------------------------------------------------------------------
  Остановка воспроизведения очереди запросов
 ------------------------------------------------------------------------------*/
static void _Audio_stop_playing(void)
{
  Player_Set_MUTE(1);
  Sound_DAC_clear_for_silience(audio_buf[0], AUDIO_BUF_SAMPLES_NUM * 2);
  ply.mode = PLAYER_IDLE;
  ply.queue_sz = 0;
  ply.queue_tail = 0;
  ply.queue_head = 0;
}

/*------------------------------------------------------------------------------
   Извлечение из очереди идентификаторов воспроизводимых файлов данных очередного файла

   Процедура поиска, открытия файла и чтения блока данных занимает не менее 5.8 мс
 ------------------------------------------------------------------------------*/
static int _Audio_init_play_file(void)
{
  int             file_num;
  T_WAV_pars_err  res;
  ULONG           actual_size;
  ULONG           request_size;


  if (ply.queue_sz == 0) goto err_exit;

  // Получаем номер записи в массиве с параметрами файла
  file_num = ply.play_queue[ply.queue_tail];

  // Формируем путь к файлу
  strcpy(ply.file_name, VAnnouncer_get_files_dir());
  strncat(ply.file_name, sound_files_map[file_num].file_name, MAX_FILE_PATH_LEN - strlen(ply.file_name));
  // Открываем файл
  if (ply.sound_file.fx_file_id == FX_FILE_ID)
  {
    LOGs(__FUNCTION__, __LINE__, 0, "Close previous file");
    fx_file_close(&ply.sound_file);
  }

  if (fx_file_open(&fat_fs_media,&ply.sound_file,  ply.file_name, FX_OPEN_FOR_READ) != FX_SUCCESS)
  {
    LOGs(__FUNCTION__, __LINE__, 0, "File %s opening error.", ply.file_name);
    goto err_exit;
  }
  else
  {
    LOGs(__FUNCTION__, __LINE__, 0, "File %s opened Ok.", ply.file_name);
  }

  if (Read_wav_header_from_file(&ply.sound_file) == 0)
  {
    LOGs(__FUNCTION__, __LINE__, 0, "Read file %s header error.", ply.file_name);
    goto err_exit;
  }
  res  = Wavefile_header_parsing();
  if (res != Valid_WAVE_File)
  {
    LOGs(__FUNCTION__, __LINE__, 0, "Parsing file %s error %d.", ply.file_name, res);
    goto err_exit;
  }
  ply.sample_rate = Player_Get_wave_file_sample_rate();

  LOGs(__FUNCTION__, __LINE__, 0, "Update sample rate to %d.", ply.sample_rate);
  Sound_DAC_update_sample_rate(ply.sample_rate);

  // Читаем во временный буфер данные из файла
  actual_size  = 0;
  request_size = AUDIO_BUF_SAMPLES_NUM * SAMPLE_SIZE;
  fx_file_read(&ply.sound_file, ply.tmp_audio_buf, request_size,&actual_size);
  ply.block_sz = actual_size;
  ply.block_sz = ply.block_sz / 2;
  if (ply.block_sz <= 0)
  {
    LOGs(__FUNCTION__, __LINE__, 0, "Reading file %s error.", ply.file_name);
    goto err_exit;
  }

  // Передвигаем указатель конца очереди номеров файлов на следующий номер
  ply.queue_tail++;
  if (ply.queue_tail >= PLAY_QUEUE_SZ)
  {
    ply.queue_tail = 0;
  }
  ply.queue_sz--;


  return 1;
err_exit:

  // Передвигаем указатель конца очереди номеров файлов на следующий номер
  ply.queue_tail++;
  if (ply.queue_tail >= PLAY_QUEUE_SZ)
  {
    ply.queue_tail = 0;
  }
  ply.queue_sz--;

  return 0;
}

/*------------------------------------------------------------------------------
   Заполнение аудио-буффера abuf данными из файла

   sz- количество 2-х байтных слов в буфере

   Процедура длится не менее 1110 мкс
 ------------------------------------------------------------------------------*/
static void _Audio_fill_buf_from_file(uint8_t bank, uint32_t position, int samples_num)
{
  int16_t   *buf_ptr;
  ULONG      actual_size  = 0;

  buf_ptr = audio_buf[ply.bank] + position;

  if (samples_num > ply.block_sz)
  {
    // Достигнут конец файла
    _Audio_convert_samlpes(ply.tmp_audio_buf, buf_ptr, ply.block_sz, ply.attenuation); // Длительность 57 мкс для 512 отсчетов
    Player_Set_MUTE(0);
    Sound_DAC_clear_for_silience(buf_ptr + ply.block_sz, samples_num - ply.block_sz);
    ply.block_sz = 0;
    ply.file_end = 1;
  }
  else
  {
    _Audio_convert_samlpes(ply.tmp_audio_buf, buf_ptr, samples_num, ply.attenuation); // Длительность 57 мкс для 512 отсчетов
    Player_Set_MUTE(0);
    ply.file_end = 0;
    if (samples_num == ply.block_sz)
    {
      // Воспроизвели столько же сколько прочитали из файла
      // Читаем следующий блок
      fx_file_read(&ply.sound_file, ply.tmp_audio_buf, samples_num * SAMPLE_SIZE,&actual_size);
      ply.block_sz = actual_size;
      ply.block_sz = ply.block_sz / 2;
      if (ply.block_sz <= 0)
      {
        ply.file_end = 1;
        ply.block_sz = 0;
      }
    }
    else
    {
      // Мы воспроизвели меньше чем прочитали из файла на предыдущем этапе
      // Это прооисходит в начале воспроизведения файла, когда для аудиобуфера нужно меньше данных чем полный размер буфера
      // Здесь часть данных из временного буфера переносим в его начало, и остальное читаем из файла
      memcpy(ply.tmp_audio_buf, ply.tmp_audio_buf + samples_num,(AUDIO_BUF_SAMPLES_NUM - samples_num) * SAMPLE_SIZE);
      // Читаем недостающие данные во временный буфер
      fx_file_read(&ply.sound_file, ply.tmp_audio_buf, samples_num * SAMPLE_SIZE,&actual_size);
      ply.block_sz = actual_size;
      ply.block_sz = ply.block_sz / 2;
      ply.block_sz += AUDIO_BUF_SAMPLES_NUM - samples_num;
    }
  }

}

/*-----------------------------------------------------------------------------------------------------


  \param bank
  \param position
  \param samples_num
-----------------------------------------------------------------------------------------------------*/
static void _Audio_fill_buf_w_sound(uint8_t bank, uint32_t position, int samples_num)
{
  int16_t   *buf_ptr;

  buf_ptr = audio_buf[ply.bank] + position;
  Generate_sound_to_buf(buf_ptr, samples_num, ply.attenuation);
}

/*-----------------------------------------------------------------------------------------------------
  Ожидание события
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Audio_wait_event(uint32_t ticks)
{
  ULONG actual_flags;
  // Ожидаем все возможные события
  if (tx_event_flags_get(&play_flags, 0xFFFFFFFF, TX_OR_CLEAR ,&actual_flags, ticks) == TX_SUCCESS)
  {
    if (actual_flags &  EVT_DELAYED_STOP)
    {
      f_delayed_stop = 1;
      Get_hw_timestump(&delayed_stop_start);
    }
    return actual_flags;
  }
  return 0;
}



/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
uint32_t Player_Send_msg_to_queue(T_player_msg *player_msg)
{
  if (tx_queue_send(&player_queue, player_msg, TX_NO_WAIT)!=TX_SUCCESS)
  {
    return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param player_msg

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t Player_Get_msg_from_queue(T_player_msg *player_msg)
{
  if (tx_queue_receive(&player_queue, player_msg, TX_NO_WAIT)!=TX_SUCCESS)
  {
    return RES_ERROR;
  }
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param v
-----------------------------------------------------------------------------------------------------*/
uint32_t Player_Set_Volume(uint32_t v)
{
  if (v > MAX_SOUND_VOLUME) v = MAX_SOUND_VOLUME;
  ply.attenuation = MAX_SOUND_VOLUME - v;
  return RES_OK;
}


/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void Player_Play_sound(uint32_t tone_freq)
{
  ply.tone_freq = tone_freq;
  tx_event_flags_set(&play_flags, EVT_PLAY_SOUND, TX_OR);
}

/*------------------------------------------------------------------------------
   Воспроизвести файл из постоянной памяти
   file - номер файла в массиве файлов
 ------------------------------------------------------------------------------*/
void Player_Enqueue_file(int file)
{
  __disable_interrupt();
  if (ply.queue_sz < PLAY_QUEUE_SZ)
  {
    ply.play_queue[ply.queue_head] = file;
    ply.queue_head++;
    if (ply.queue_head >= PLAY_QUEUE_SZ)
    {
      ply.queue_head = 0;
    }
    ply.queue_sz++;
  }
  __enable_interrupt();

  _Audio_set_evt_to_play_file();
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void Player_Stop(void)
{
  tx_event_flags_set(&play_flags, EVT_STOP_PLAY, TX_OR);
}

/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
void Player_Delayed_Stop(void)
{
  tx_event_flags_set(&play_flags, EVT_DELAYED_STOP, TX_OR);
}

/*-----------------------------------------------------------------------------------------------------


  \param p_queue
-----------------------------------------------------------------------------------------------------*/
static void Player_send_to_queue_notify(TX_QUEUE *p_queue)
{
  tx_event_flags_set(&play_flags, EVENT_PLAYER_MSG_IN_QUEUE, TX_OR);
}


/*------------------------------------------------------------------------------

 ------------------------------------------------------------------------------*/
static void SoundPlayer_task(ULONG arg)
{
  uint32_t    evt_flags;
  uint8_t     bnk;

  Set_en_voice_announcer(wvar.en_sounds);
  Set_en_voice_laguage(wvar.voice_language);
  Set_sound_freq(440);
  Set_sound_loudness(wvar.sound_loudness);
  
  fx_directory_default_set(&fat_fs_media, "/");

  tx_event_flags_create(&play_flags, "SoundPlay"); // Все события автоматически сбрасываемые
  tx_queue_create(&player_queue, (CHAR *) "Player", sizeof(T_player_msg)/sizeof(uint32_t),player_queue_buf, sizeof(T_player_msg) * PLAYER_MESAGGES_QUEUE_SZ);
  tx_queue_send_notify(&player_queue, Player_send_to_queue_notify);

  ply.sample_rate = PLAYER_DEFAULT_SAMPLE_RATE;
  Sound_DAC_init_player_periphery(ply.sample_rate);

  ply.mode = PLAYER_IDLE;

  do
  {
    evt_flags =  _Audio_wait_event(10);

    // Ожидаем сообщения о проигрывании файла

    if (evt_flags != 0)
    {

      if (evt_flags & EVENT_PLAYER_MSG_IN_QUEUE)
      {
        T_player_msg player_msg;
        while (Player_Get_msg_from_queue(&player_msg) == RES_OK)
        {
          VAnnouncer_messages_processing(player_msg.data);
        }
      }

      // Если получили сигнал воспроизвести файл когда воспроизводился тон, то отменить воспроизведение тона
      if ((ply.mode == PLAYING_SOUND) && ((evt_flags & EVT_PLAY_FILE) != 0))
      {
        _Audio_stop_playing();
      }
      // Если получили сигнал воспроизвести тона когда воспроизводился файл, то отменить воспроизведение файла
      if ((ply.mode == PLAYING_FILE) && ((evt_flags & EVT_PLAY_SOUND) != 0))
      {
        _Audio_stop_playing();
      }

      // Приоритет отдаем воспроизведению тона
      if ((evt_flags & EVT_PLAY_SOUND) && (ply.mode == PLAYER_IDLE))
      {
        // Начинаем воспроизведение звука из состояния тишины

        // Ожидание начала цикла DMA, чтобы было достаточно времени заполнить доступный на запись буфер
        do
        {
          evt_flags =  _Audio_wait_event(10);
        } while ((evt_flags & EVT_READ_NEXT) == 0);


        bnk = Sound_DAC_get_current_buf();    // Определяем из какогор буфера в данный момент идет воспроизведение
        ply.mode = PLAYING_SOUND;
        ply.bank = bnk ^ 1;                    // Устанавливаемся на буфер готовый к перезаписи данных
        Tone_gen_start(ply.tone_freq, ply.sample_rate);
        _Audio_fill_buf_w_sound(ply.bank, 0 , AUDIO_BUF_SAMPLES_NUM);
        Player_Set_MUTE(0);
        //_Audio_set_evt_to_read_next_block(); // Сразу сгенерировать событие на чтение следующего блока данных если файл продолжается
      }
      else if ((evt_flags & EVT_PLAY_FILE) && (ply.mode == PLAYER_IDLE))
      {
        // Начинаем воспроизведение файла из состояния тишины

        // Ожидаем начало цикла DMA, чтобы было достаточно времени заплнить доступный на запись буфер
        do
        {
          evt_flags =  _Audio_wait_event(10);
        } while ((evt_flags & EVT_READ_NEXT) == 0);

        if (ply.queue_sz > 0)  // Извлечь из очереди номер файла и инициализировать воспроизведение
        {
          if (_Audio_init_play_file() != 0)
          {
            bnk = Sound_DAC_get_current_buf();  // Определяем из какогор буфера в данный момент идет воспроизведение.
            ply.mode = PLAYING_FILE;
            ply.bank = bnk ^ 1;                     // Устанавливаемся на буфер готовый к перезаписи данных
            _Audio_fill_buf_from_file(ply.bank, 0, AUDIO_BUF_SAMPLES_NUM);
            //_Audio_set_evt_to_read_next_block();    // Сразу сгенерировать событие на чтение следующего блока данных если файл продолжается
          }
        }
      }
      else if (evt_flags & EVT_READ_NEXT)
      {
        // Обрабатываем событие заполнения следующего аудиобуфера

        bnk = Sound_DAC_get_current_buf();     // Определяем из какогор буфера в данный момент идет воспроизведение
        ply.bank = bnk ^ 1;
        ;                    // Устанавливаемся на буфер готовый к перезаписи данных

        if (ply.mode == PLAYING_SOUND)
        {
          // Если воспроизводим простой звук, то заполняем им следующий аудиобуффер
          _Audio_fill_buf_w_sound(ply.bank, 0, AUDIO_BUF_SAMPLES_NUM);
        }
        else if (ply.mode == PLAYING_FILE)
        {
          if (ply.block_sz == 0)
          {
            if (ply.queue_sz != 0)
            {
              // Начать воспроизводить новый файл из очереди
              if (_Audio_init_play_file() != 0)
              {
                _Audio_fill_buf_from_file(ply.bank,0, AUDIO_BUF_SAMPLES_NUM);
              }
              else
              {
                _Audio_stop_playing();
              }
            }
            else
            {
              if (ply.file_end == 1)
              {
                _Audio_stop_playing();
              }
            }
          }
          else
          {
            _Audio_fill_buf_from_file(ply.bank, 0 , AUDIO_BUF_SAMPLES_NUM);
          }
        }
      }
      else if (evt_flags & EVT_STOP_PLAY)
      {
        _Audio_stop_playing();
      }
    }

    if (f_delayed_stop != 0)
    {
      if (Time_elapsed_msec(&delayed_stop_start) > DEALAYED_STOP_TIMEOUT_MS)
      {
        f_delayed_stop = 0;
        _Audio_stop_playing();
      }
    }

  }while (1);
}


/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
uint32_t  Thread_Sound_Player_create(void)
{
  UINT              err;

  err = tx_thread_create(
                         &player_thread,
                         (CHAR *)"SoundPlayer",
                         SoundPlayer_task,
                         0,
                         player_stacks,
                         PLAYER_TASK_STACK_SIZE,
                         PLAYER_TASK_PRIO,
                         PLAYER_TASK_PRIO,
                         1,
                         TX_AUTO_START
                        );

  if (err == TX_SUCCESS)
  {
    APPLOG("Sound player task created.");
    return RES_OK;
  }
  else
  {
    APPLOG("Sound player creating error %d.", err);
    return RES_ERROR;
  }

}

