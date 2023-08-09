// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-09
// 17:56:39
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"
#include   "gx_display.h"

extern GX_STUDIO_DISPLAY_INFO MC50_display_table[];

uint16_t               video_buffer[DISPLAY_1_X_RESOLUTION*DISPLAY_1_Y_RESOLUTION];

GX_WINDOW_ROOT         *root;

char                    gui_strings[GX_STRINGS_NUMBER][GX_STRING_MAX_LEN];
GX_STRING               gx_strings[GX_STRINGS_NUMBER];


/*-----------------------------------------------------------------------------------------------------


  \param str_id
  \param fmt
-----------------------------------------------------------------------------------------------------*/
GX_STRING *GUI_print_str(uint32_t str_id, const char *fmt, ...)
{
  va_list          ap;
  GX_STRING       *p = 0;
  va_start(ap, fmt);
  if (str_id < GX_STRINGS_NUMBER)
  {
    vsnprintf(gui_strings[str_id], GX_STRING_MAX_LEN, (const char *)fmt, ap);
    gx_strings[str_id].gx_string_length = strlen(gui_strings[str_id]);
    p = &gx_strings[str_id];
  }
  va_end(ap);
  return p;
}

/*-----------------------------------------------------------------------------------------------------


  \param str_id
  \param fmt
-----------------------------------------------------------------------------------------------------*/
void GUI_print_to_prompt(uint32_t str_id, GX_PROMPT **prmts, const char *fmt, ...)
{
  va_list          ap;
  GX_STRING       *p = 0;
  va_start(ap, fmt);
  if (str_id < GX_STRINGS_NUMBER)
  {
    vsnprintf(gui_strings[str_id], GX_STRING_MAX_LEN, (const char *)fmt, ap);
    gx_strings[str_id].gx_string_length = strlen(gui_strings[str_id]);
    p = &gx_strings[str_id];
    gx_prompt_text_set_ext(prmts[str_id] , p);
  }
  va_end(ap);
  return;
}


/*-----------------------------------------------------------------------------------------------------
  Переносим веь буфер на дисплей

  \param canvas
  \param dirty
-----------------------------------------------------------------------------------------------------*/
static void _565rgb_buffer_toggle(GX_CANVAS *canvas, GX_RECTANGLE *dirty)
{
  //ITM_EVENT8(1,1);
  TFT_set_display_stream();
  TFT_wr_data_buf(video_buffer, sizeof(video_buffer));
  //ITM_EVENT8(1,0);
}

/*-----------------------------------------------------------------------------------------------------


  \param display

  \return UINT
-----------------------------------------------------------------------------------------------------*/
static UINT _565rgb_driver_setup(GX_DISPLAY *display)
{
  _gx_display_driver_565rgb_setup(display, (VOID *)SCREEN_HANDLE, _565rgb_buffer_toggle);

  TFT_clear();
  TFT_display_on();
  TFT_set_display_stream();

  return (GX_SUCCESS);
}


/*-----------------------------------------------------------------------------------------------------


  \param arg
-----------------------------------------------------------------------------------------------------*/
void GUI_start(void)
{
  TFT_init();
  gx_system_initialize();

  MC50_display_table[0].canvas_memory =  (ULONG *)video_buffer;

  for (uint32_t i=0;i< GX_STRINGS_NUMBER;i++)
  {
    gx_strings[i].gx_string_ptr    = gui_strings[i];
    gx_strings[i].gx_string_length = 0;
  }

  gx_studio_display_configure(DISPLAY_1, _565rgb_driver_setup, LANGUAGE_ENGLISH, DISPLAY_1_THEME_1,&root);

  Init_main_screen();

  gx_widget_show(root);
  gx_system_start();
}


