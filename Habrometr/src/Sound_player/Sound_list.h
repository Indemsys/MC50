#ifndef SOUND_LIST_H
  #define SOUND_LIST_H


  #define DIR_ENG  "ENG/"
  #define DIR_RUS  "RUS/"



enum T_sounds_id
{
  SND_0                              ,
  SND_1                              ,
  SND_2                              ,
  SND_3                              ,
  SND_4                              ,
  SND_5                              ,
  SND_6                              ,
  SND_7                              ,
  SND_8                              ,
  SND_9                              ,
  SND_ALARM                          ,
  SND_GONG                           ,
  SND_OVERLOAD                       ,
  SND_SILENCE                        ,
  SND_KARMA                          ,
};

typedef struct
{
    enum  T_sounds_id sid;
    const char        *file_name;

} T_sound_file;


typedef void (*T_play_func)(int num);


  #ifndef SOUNDS_LIST

extern const T_sound_file    sound_files_map[];

  #endif


#endif
