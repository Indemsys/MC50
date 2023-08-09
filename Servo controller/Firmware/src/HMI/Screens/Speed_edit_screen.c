// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-16
// 10:49:33
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"

static GX_WINDOW  *speed_edit_screen;


static int32_t            enc_cnt;
static T_sys_timestump    save_t;

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_speed_edit_screen(void)
{
  if (speed_edit_screen == 0)
  {
    gx_studio_named_widget_create("window_speed_edit" , 0, (GX_WIDGET **)&speed_edit_screen);
  }
  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)speed_edit_screen);
  enc_cnt = Get_encoder_counter();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Show_Speed_edit(void)
{
  uint32_t str_id = 0;

  gx_prompt_text_set_ext(&window_speed_edit.window_speed_edit_Val_rot_speed       ,GUI_print_str(str_id++, "%0.0f", (double)wvar.rotation_speed));

  if (Time_elapsed_msec(&save_t) > 500)
  {
    gx_widget_hide(&window_speed_edit.window_speed_edit_prmpt_Saved);
  }
  else
  {
    gx_widget_show(&window_speed_edit.window_speed_edit_prmpt_Saved);
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Encoder_processing(void)
{
  int32_t delta = Get_encoder_counter_delta(&enc_cnt);

  if (delta != 0)
  {
    float tmp = (int32_t)wvar.rotation_speed;
    tmp += delta;
    if (tmp < MIN_ROT_SPEED) tmp = MIN_ROT_SPEED;
    if (tmp > MAX_ROT_SPEED) tmp = MAX_ROT_SPEED;
    wvar.rotation_speed = tmp;
  }


  if (Get_switch_press_signal())
  {
    gx_system_timer_stop((GX_WIDGET *)speed_edit_screen, ENCODER_PROC_TIMER_ID);
    gx_widget_detach((GX_WIDGET *)speed_edit_screen);
    Init_menu_screen();
    return 0;
  }
  if (Get_switch_long_press_signal())
  {
    Get_hw_timestump(&save_t);
    Request_save_app_settings();
  }
  return 1;
}

/*-----------------------------------------------------------------------------------------------------


  \param window
-----------------------------------------------------------------------------------------------------*/
VOID Speed_edit_draw_callback(GX_WINDOW *window)
{
  gx_window_draw(window);
  gx_widget_children_draw(window);
}

/*-----------------------------------------------------------------------------------------------------


  \param window
  \param event_ptr

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT Speed_edit_event_callback(GX_WINDOW *window, GX_EVENT *event_ptr)
{
  UINT status;

  switch (event_ptr->gx_event_type)
  {
  case GX_EVENT_SHOW:
    _Show_Speed_edit();
    gx_system_timer_start((GX_WIDGET *)window, ENCODER_PROC_TIMER_ID, ENCODER_PROC_INTIT_TICKS, ENCODER_PROC_PERIOD_TICKS);
    status = gx_window_event_process(window, event_ptr);
    break;

  case GX_EVENT_TIMER:
    if (event_ptr->gx_event_payload.gx_event_timer_id == ENCODER_PROC_TIMER_ID)
    {
      if (_Encoder_processing()) _Show_Speed_edit();
    }
    break;

  default:
    status = gx_window_event_process(window, event_ptr);
    return status;
  }

  return 0;
}

