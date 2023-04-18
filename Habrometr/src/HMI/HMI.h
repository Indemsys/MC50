#ifndef HMI_H
  #define HMI_H


#define  SCREEN_HANDLE             0x12345679

#define  GX_STRINGS_NUMBER         14
#define  GX_STRING_MAX_LEN         32

#define ENCODER_PROC_TIMER_ID      20
#define ENCODER_PROC_INTIT_TICKS   1   // Величина выражается в тиках GX_SYSTEM_TIMER_MS
#define ENCODER_PROC_PERIOD_TICKS  1   // Величина выражается в тиках GX_SYSTEM_TIMER_MS

typedef struct
{
    GX_PROMPT *prompt;
} T_prompts;


extern char                    gui_strings[GX_STRINGS_NUMBER][GX_STRING_MAX_LEN];
extern GX_STRING               gx_strings[GX_STRINGS_NUMBER];
extern GX_WINDOW_ROOT         *root;



void       GUI_start(void);
GX_STRING *GUI_print_str(uint32_t str_id, const char *fmt, ...);
void       GUI_print_to_prompt(uint32_t str_id, GX_PROMPT **prmts, const char *fmt, ...);

#include "Menu_screen.h"
#include "SDcard_state_screen.h"
#include "Net_state_screen.h"
#include "Habrometer_screen.h"


#endif



