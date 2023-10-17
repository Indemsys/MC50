#ifndef SOUND_PLAYER_H
  #define SOUND_PLAYER_H

  #include "Sound_list.h"
  #include "Sound_gen.h"
  #include "WAV_reader.h"
  #include "Voice_announcer.h"

  #define DEALAYED_STOP_TIMEOUT_MS    1000


  #define EVT_PLAY_SOUND             BIT( 1 )
  #define EVT_READ_NEXT              BIT( 2 )
  #define EVT_STOP_PLAY              BIT( 3 )
  #define EVT_PLAY_FILE              BIT( 4 )
  #define EVT_DELAYED_STOP           BIT( 5 )
  #define EVENT_PLAYER_MSG_IN_QUEUE  BIT( 6 )

  #define PLAYER_IDLE           0
  #define PLAYING_SOUND         1
  #define PLAYING_FILE          2
  #define PLAY_QUEUE_SZ         32
  #define MAX_FILE_PATH_LEN     128

  #define MAX_SOUND_VOLUME 10


typedef struct
{
  uint32_t en_voice_announcer;
  uint32_t sound_volume;
  uint32_t voice_language;
  uint32_t sound_freq;

} T_sound_player_settings;

typedef struct
{
    uint8_t       mode;       // Режим работы плеера. 0- неактивен, 1 - воспроизведение тона, 2- воспроизведение файла
    uint8_t       bank;       // Индекс текущего сэмпла
    uint32_t      tone_freq;  // Частота тона в Гц
    uint32_t      sample_rate;
    int16_t      *file_ptr;   // Указатель на данные файла
    char          file_name[MAX_FILE_PATH_LEN+1];  // Имя файла
    FX_FILE       sound_file;
    int16_t       tmp_audio_buf[AUDIO_BUF_SAMPLES_NUM];
    uint32_t      block_sz;   // Размер прочитанного их файл блока данных
    uint8_t       file_end;   // Флаг завершения воспроизведения файла

    uint8_t       attenuation;

    int32_t       play_queue[PLAY_QUEUE_SZ]; //  Очередь на воспроизведение файлов
    int32_t       queue_head;
    int32_t       queue_tail;
    int32_t       queue_sz;

} T_sound_player;

typedef struct
{
  uint8_t data[8];
} T_player_msg;



uint32_t     Thread_Sound_Player_create(void);
uint32_t     Player_Send_msg_to_queue(T_player_msg *player_msg);
uint32_t     Player_Get_msg_from_queue(T_player_msg *player_msg);
void         Player_Set_MUTE(uint8_t state);
uint32_t     Player_Set_Volume(uint32_t v);

void         Player_Play_sound(uint32_t tone_freq);
void         Player_Stop(void);
void         Player_Delayed_Stop(void);
void         Player_Enqueue_file(int file);

unsigned int Player_Get_wave_file_sample_rate(void);
void         Player_Set_evt_to_read_next_block(void);


uint32_t     Get_en_voice_announcer   (void);
uint32_t     Get_sound_volume         (void);
uint32_t     Get_voice_language       (void);
uint32_t     Get_sound_freq           (void);
void         Set_en_voice_announcer   (uint32_t val);
void         Set_sound_loudness       (uint32_t val);
void         Set_en_voice_laguage     (uint32_t val);
void         Set_sound_freq           (uint32_t val);

#endif
