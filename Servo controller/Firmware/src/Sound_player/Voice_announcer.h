#ifndef VOICE_ANNOUNCER_H
  #define VOICE_ANNOUNCER_H

#define ARRIVAL_ON_FLOOR_SILENCE       0
#define ARRIVAL_ON_FLOOR_GONG          1
#define ARRIVAL_ON_FLOOR_VOICE_MSG     2


// data[0] - Тип команды
  #define VOAN_CMD_SETT                    0 // Настройка параметров проигрывателя
// data[1] - Номер параметра. data[2..5] - значение параметра 4 байта
  #define VOAN_EN_VOICE_ANNOUNCER          2 // Установка значения переменной en_voice_announcer
  #define VOAN_SOUND_VOLUME                3 // Установка значения переменной sound_volume
  #define VOAN_VOICE_LANGUAGE              4 // Установка значения переменной voice_language




// data[0] - Тип команды
  #define VOAN_CMD_PLAY      1 // Команда на проигрывание определенного сообщения
// data[1] - номер сообщения, data[2..5] - аргумент сообщения 4 байта
  #define VOAN_MSG_ERROR_                  0 //
  #define VOAN_MSG_OVERLOAD                1 //
  #define VOAN_MSG_ALARM                   2
  #define VOAN_MSG_KARMA                   3

  #define VOAN_LAST_MSG_ID                 4 //


// data[0] - Тип команды
#define VOAN_CMD_SOUND_FREQ     2 // Установка частоты тона
// data[1...4] - частота тона

// data[0] - Тип команды
#define VOAN_CMD_SOUND_GEN      3 // Команда на генерацию тона

// data[0] - Тип команды
#define VOAN_CMD_STOP           4 // Команда на отсновку генерации звука


typedef struct
{
    uint32_t msg_id;
    T_play_func play_func;
    const char *description;

} T_vannouncer_item;


void        Play_msg_by_num(unsigned int  msg_num, int msg_arg);
void        Play_error_number_msg(int num);
void        Play_msg_karma(int num);
void        Play_msg_overload(int num);
void        Play_msg_alarm(int num);
void        Play_arrival_snd(int num);

void        VAnnouncer_messages_processing(const uint8_t *buf);
const char* VAnnouncer_get_files_dir(void);


const char *Get_sound_description(uint32_t sid);


#endif // VOICE_ANNOUNCER_H



