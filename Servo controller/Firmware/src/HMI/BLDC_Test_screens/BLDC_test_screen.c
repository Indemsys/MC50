// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-06-26
// 13:15:10
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"


static GX_WINDOW              *w_mn_mottest_screen;


#define   MAIN_BLDC_VAL_VERSION           1
#define   MAIN_BLDC_SPEED                 2
#define   MAIN_CAN_NODE_NUM               3
#define   MAIN_VAL_POWER_CONSUMTION       4
#define   MAIN_VAL_PWR_SRC_VOLTAGE        5
#define   MAIN_VAL_PWR_SRC_CURRENT        6
#define   MAIN_VAL_MOTOR_CURRENT          7
#define   MAIN_VAL_ROT_SPEED              8
#define   MAIN_VAL_POSITION               9
#define   MAIN_VAL_ERRORS                 10


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_BLDC_Test_screen(void *p)
{


  if (w_mn_mottest_screen == 0)
  {
    gx_studio_named_widget_create("w_mn_mottest" , 0, (GX_WIDGET **)&w_mn_mottest_screen);
  }

  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)w_mn_mottest_screen);

}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Show(void)
{
  char ver_str[64];

  Get_build_date_time(ver_str);
  gx_prompt_text_set_ext(&w_mn_mottest.w_mn_mottest_Val_version           ,GUI_print_str(MAIN_BLDC_VAL_VERSION     , "%s"    , ver_str));
  gx_prompt_text_set_ext(&w_mn_mottest.w_mn_mottest_Val_power_consumtion  ,GUI_print_str(MAIN_VAL_POWER_CONSUMTION , "%0.1f" ,(double)adc.input_pwr));
  gx_prompt_text_set_ext(&w_mn_mottest.w_mn_mottest_Val_pwr_src_voltage   ,GUI_print_str(MAIN_VAL_PWR_SRC_VOLTAGE  , "%0.1f" ,(double)adc.v_pwr));
  gx_prompt_text_set_ext(&w_mn_mottest.w_mn_mottest_Val_pwr_src_current   ,GUI_print_str(MAIN_VAL_PWR_SRC_CURRENT  , "%0.1f" ,(double)adc.i_pwr));
  gx_prompt_text_set_ext(&w_mn_mottest.w_mn_mottest_Val_motor_current     ,GUI_print_str(MAIN_VAL_MOTOR_CURRENT    , "%0.1f" ,(double)adc.filtered_bldc_motor_current));

  uint32_t app_err = App_get_error_flags();

  if (app_err != 0)
  {
    gx_widget_show(&w_mn_mottest.w_mn_mottest_Val_errors);
    gx_widget_show(&w_mn_mottest.w_mn_mottest_lb_errors);
    gx_prompt_text_set_ext(&w_mn_mottest.w_mn_mottest_Val_errors            ,GUI_print_str(MAIN_VAL_ERRORS           , "%04X" ,app_err ));
  }
  else
  {
    gx_widget_hide(&w_mn_mottest.w_mn_mottest_Val_errors);
    gx_widget_hide(&w_mn_mottest.w_mn_mottest_lb_errors);
  }


}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static uint32_t _Encoder_processing(void)
{

//  if (Get_switch_long_press_signal())
//  {
//    gx_system_timer_stop((GX_WIDGET *)w_mn_mottest_screen, ENCODER_PROC_TIMER_ID);
//    gx_widget_detach((GX_WIDGET *)w_mn_mottest_screen);
//    Init_menu_BLDC_screen((void*)Init_main_BLDC_screen);
//    return 0;
//  }

  if (Get_switch_press_signal())
  {
    gx_system_timer_stop((GX_WIDGET *)w_mn_mottest_screen, ENCODER_PROC_TIMER_ID);
    gx_widget_detach((GX_WIDGET *)w_mn_mottest_screen);
    Init_comm_law_search_screen((void*)Init_BLDC_Test_screen);
    return 0;
  }

  return 1;
}

/*-----------------------------------------------------------------------------------------------------
  Вызывается при выполнении функции gx_system_canvas_refresh  в контексте задачи GUIX System Thread

  \param p_widget
-----------------------------------------------------------------------------------------------------*/
VOID Mn_mot_test_draw_callback(GX_WINDOW *window)
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
UINT Mn_mot_test_event_callback(GX_WINDOW *window, GX_EVENT *event_ptr)
{
  UINT status;

  switch (event_ptr->gx_event_type)
  {
  case GX_EVENT_SHOW:
    _Show();
    gx_system_timer_start((GX_WIDGET *)window, ENCODER_PROC_TIMER_ID, ENCODER_PROC_INTIT_TICKS, ENCODER_PROC_PERIOD_TICKS);
    status = gx_window_event_process(window, event_ptr);
    break;

  case GX_EVENT_TIMER:
    if (event_ptr->gx_event_payload.gx_event_timer_id == ENCODER_PROC_TIMER_ID)
    {
      if (_Encoder_processing()) _Show();
    }
    break;

  default:
    status = gx_window_event_process(window, event_ptr);
    return status;
  }

  return 0;
}
