// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-06-26
// 13:15:10
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"


static GX_WINDOW              *angle_calibr_screen;


static uint32_t calibr_stage;
static T_sys_timestump    save_t;

#define VAL_ANGLE_0       0
#define VAL_ANGLE_90      1
#define VAL_CURRENT_VALUE 2
#define TEXT_PROMPT       3


static uint32_t position_0;
static uint32_t position_90;
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_calibration_screen(void)
{
  if (angle_calibr_screen == 0)
  {
    gx_studio_named_widget_create("window_angle_calibr" , 0, (GX_WIDGET **)&angle_calibr_screen);
  }

  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)angle_calibr_screen);

  calibr_stage = 0;
  position_0   = wvar.close_position;
  position_90  = wvar.open_position;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Show_Angle_calibr(void)
{
  gx_prompt_text_set_ext(&window_angle_calibr.window_angle_calibr_Val_angle_0       ,GUI_print_str(VAL_ANGLE_0       , "%d",position_0));
  gx_prompt_text_set_ext(&window_angle_calibr.window_angle_calibr_Val_angle_90      ,GUI_print_str(VAL_ANGLE_90      , "%d",position_90));
  gx_prompt_text_set_ext(&window_angle_calibr.window_angle_calibr_Val_current_value ,GUI_print_str(VAL_CURRENT_VALUE , "%d",adc.smpl_POS_SENS));

  if (calibr_stage == 0)
  {
    gx_widget_fill_color_set(&window_angle_calibr.window_angle_calibr_Val_angle_0, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);
    gx_widget_fill_color_set(&window_angle_calibr.window_angle_calibr_Val_angle_90, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    gx_prompt_text_color_set(&window_angle_calibr.window_angle_calibr_Val_angle_0, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    gx_prompt_text_color_set(&window_angle_calibr.window_angle_calibr_Val_angle_90, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);
    gx_multi_line_text_view_text_set_ext(&window_angle_calibr.window_angle_calibr_text_prompt, GUI_print_str(TEXT_PROMPT, "Step1. Set the lever to position 0 deg and long press to save"));

  }
  else if (calibr_stage == 1)
  {
    gx_widget_fill_color_set(&window_angle_calibr.window_angle_calibr_Val_angle_0, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    gx_widget_fill_color_set(&window_angle_calibr.window_angle_calibr_Val_angle_90, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);
    gx_prompt_text_color_set(&window_angle_calibr.window_angle_calibr_Val_angle_0, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);
    gx_prompt_text_color_set(&window_angle_calibr.window_angle_calibr_Val_angle_90, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    gx_multi_line_text_view_text_set_ext(&window_angle_calibr.window_angle_calibr_text_prompt, GUI_print_str(TEXT_PROMPT, "Step2. Set the lever to position 90 deg and long press to save"));
  }
  else if (calibr_stage == 2)
  {
    gx_widget_fill_color_set(&window_angle_calibr.window_angle_calibr_Val_angle_0, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    gx_widget_fill_color_set(&window_angle_calibr.window_angle_calibr_Val_angle_90, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK, GX_COLOR_ID_BLACK);
    gx_prompt_text_color_set(&window_angle_calibr.window_angle_calibr_Val_angle_0, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);
    gx_prompt_text_color_set(&window_angle_calibr.window_angle_calibr_Val_angle_90, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE, GX_COLOR_ID_WHITE);
    gx_multi_line_text_view_text_set_ext(&window_angle_calibr.window_angle_calibr_text_prompt, GUI_print_str(TEXT_PROMPT, "Step3. Long press to reset"));
  }


  if (Time_elapsed_msec(&save_t)>500)
  {
    gx_widget_hide(&window_angle_calibr.window_angle_calibr_prmpt_Saved);
  }
  else
  {
    gx_widget_show(&window_angle_calibr.window_angle_calibr_prmpt_Saved);
  }

}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static uint32_t _Encoder_processing(void)
{
  if (Get_switch_long_press_signal())
  {
    switch (calibr_stage)
    {
    case 0:
      position_0 = adc.smpl_POS_SENS;
      calibr_stage++;
      break;
    case 1:
      position_90 = adc.smpl_POS_SENS;
      calibr_stage++;
      break;
    case 2:
      wvar.close_position = position_0;
      wvar.open_position  = position_90;
      Request_save_app_settings();
      Wait_ms(200);
      Reset_system();
      break;
    }
    Get_hw_timestump(&save_t);
  }

  if (Get_switch_press_signal())
  {
    gx_system_timer_stop((GX_WIDGET *)angle_calibr_screen, ENCODER_PROC_TIMER_ID);
    gx_widget_detach((GX_WIDGET *)angle_calibr_screen);
    Init_menu_screen();
    return 0;
  }
  return 1;
}

/*-----------------------------------------------------------------------------------------------------
  Вызывается при выполнении функции gx_system_canvas_refresh  в контексте задачи GUIX System Thread

  \param p_widget
-----------------------------------------------------------------------------------------------------*/
VOID Angle_calibr_draw_callback(GX_WINDOW *window)
{
  gx_window_draw(window);
  gx_widget_children_draw(window);
}

/*-----------------------------------------------------------------------------------------------------
  Вызывется при выполнении функции gx_widget_show(root) с gx_event_type = GX_EVENT_SHOW

  \param p_widget
  \param p_event

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT Angle_calibr_event_callback(GX_WINDOW *window, GX_EVENT *event_ptr)
{
  UINT status;

  switch (event_ptr->gx_event_type)
  {
  case GX_EVENT_SHOW:
    Show_Angle_calibr();
    gx_system_timer_start((GX_WIDGET *)window, ENCODER_PROC_TIMER_ID, ENCODER_PROC_INTIT_TICKS, ENCODER_PROC_PERIOD_TICKS);
    status = gx_window_event_process(window, event_ptr);
    break;

  case GX_EVENT_TIMER:
    if (event_ptr->gx_event_payload.gx_event_timer_id == ENCODER_PROC_TIMER_ID)
    {
      if (_Encoder_processing()) Show_Angle_calibr();
    }
    break;

  default:
    status = gx_window_event_process(window, event_ptr);
    return status;
  }

  return 0;
}
