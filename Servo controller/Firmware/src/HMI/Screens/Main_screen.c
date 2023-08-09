// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-06-26
// 13:15:10
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"


static GX_WINDOW              *main_screen;


#define   MAIN_VAL_SPEED                  1
#define   MAIN_NODE_NUMBER                2
#define   MAIN_VAL_POWER_CONSUMTION       3
#define   MAIN_VAL_PWR_SRC_VOLTAGE        4
#define   MAIN_VAL_PWR_SRC_CURRENT        5
#define   MAIN_VAL_MOTOR_CURRENT          6
#define   MAIN_VAL_ROT_SPEED              7
#define   MAIN_VAL_POSITION               8
#define   MAIN_VAL_ERRORS                 9





/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_main_screen(void)
{
  if (main_screen == 0)
  {
    gx_studio_named_widget_create("window_main" , 0, (GX_WIDGET **)&main_screen);
  }

  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)main_screen);

}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Show_main(void)
{

  gx_prompt_text_set_ext(&window_main.window_main_Val_speed             ,GUI_print_str(MAIN_VAL_SPEED            , "%0.0f" , (double)wvar.rotation_speed));
  gx_prompt_text_set_ext(&window_main.window_main_Val_node_number       ,GUI_print_str(MAIN_NODE_NUMBER          , "%02d"  , wvar.can_node_number));
  gx_prompt_text_set_ext(&window_main.window_main_Val_power_consumtion  ,GUI_print_str(MAIN_VAL_POWER_CONSUMTION , "%0.1f" ,(double)adc.input_pwr));
  gx_prompt_text_set_ext(&window_main.window_main_Val_pwr_src_voltage   ,GUI_print_str(MAIN_VAL_PWR_SRC_VOLTAGE  , "%0.1f" ,(double)adc.v_pwr));
  gx_prompt_text_set_ext(&window_main.window_main_Val_pwr_src_current   ,GUI_print_str(MAIN_VAL_PWR_SRC_CURRENT  , "%0.1f" ,(double)adc.i_pwr));
  gx_prompt_text_set_ext(&window_main.window_main_Val_motor_current     ,GUI_print_str(MAIN_VAL_MOTOR_CURRENT    , "%0.1f" ,(double)adc.filtered_motor_current));
  gx_prompt_text_set_ext(&window_main.window_main_Val_shaft_rot_speed   ,GUI_print_str(MAIN_VAL_ROT_SPEED        , "%0.1f" ,(double)adc.shaft_speed));
  gx_prompt_text_set_ext(&window_main.window_main_Val_shaft_position    ,GUI_print_str(MAIN_VAL_POSITION         , "%0.1f" ,(double)adc.shaft_position_grad));

  uint32_t app_err = App_get_error_flags();

  if (app_err != 0)
  {
    gx_widget_show(&window_main.window_main_Val_errors);
    gx_widget_show(&window_main.window_main_lb_errors);
    gx_prompt_text_set_ext(&window_main.window_main_Val_errors            ,GUI_print_str(MAIN_VAL_ERRORS           , "%04X" ,app_err ));
  }
  else
  {
    gx_widget_hide(&window_main.window_main_Val_errors);
    gx_widget_hide(&window_main.window_main_lb_errors);
  }


}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static uint32_t _Encoder_processing(void)
{

  if (Get_switch_long_press_signal())
  {
    gx_system_timer_stop((GX_WIDGET *)main_screen, ENCODER_PROC_TIMER_ID);
    gx_widget_detach((GX_WIDGET *)main_screen);
    Init_menu_screen();
    return 0;
  }

  if (Get_switch_press_signal())
  {
    gx_system_timer_stop((GX_WIDGET *)main_screen, ENCODER_PROC_TIMER_ID);
    gx_widget_detach((GX_WIDGET *)main_screen);
    Init_qmenu_screen();
    return 0;
  }

  return 1;
}

/*-----------------------------------------------------------------------------------------------------
  Вызывается при выполнении функции gx_system_canvas_refresh  в контексте задачи GUIX System Thread

  \param p_widget
-----------------------------------------------------------------------------------------------------*/
VOID Main_draw_callback(GX_WINDOW *window)
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
UINT Main_event_callback(GX_WINDOW *window, GX_EVENT *event_ptr)
{
  UINT status;

  switch (event_ptr->gx_event_type)
  {
  case GX_EVENT_SHOW:
    Show_main();
    gx_system_timer_start((GX_WIDGET *)window, ENCODER_PROC_TIMER_ID, ENCODER_PROC_INTIT_TICKS, ENCODER_PROC_PERIOD_TICKS);
    status = gx_window_event_process(window, event_ptr);
    break;

  case GX_EVENT_TIMER:
    if (event_ptr->gx_event_payload.gx_event_timer_id == ENCODER_PROC_TIMER_ID)
    {
      if (_Encoder_processing()) Show_main();
    }
    break;

  default:
    status = gx_window_event_process(window, event_ptr);
    return status;
  }

  return 0;
}
