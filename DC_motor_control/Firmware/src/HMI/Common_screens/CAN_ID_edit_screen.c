// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-16
// 10:49:33
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"

static GX_WINDOW  *can_id_edit_screen;


static int32_t            enc_cnt;
static T_sys_timestump    save_t;
static T_hmi_func      ret_func;
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_can_id_edit_screen(void *p)
{
  if (p!=0) ret_func = (T_hmi_func)p;
  if (can_id_edit_screen == 0)
  {
    gx_studio_named_widget_create("window_can_id" , 0, (GX_WIDGET **)&can_id_edit_screen);
  }
  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)can_id_edit_screen);
  enc_cnt = Get_encoder_counter();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Show(void)
{
  uint32_t str_id = 0;
  uint32_t can_id;

  sscanf((char*)wvar.can_id, "%x", &can_id);
  gx_prompt_text_set_ext(&window_can_id.window_can_id_prmt_full_id       ,GUI_print_str(str_id++         , "0x%08X", can_id | (wvar.can_node_number << 20) ));
  gx_prompt_text_set_ext(&window_can_id.window_can_id_prmt_node_num      ,GUI_print_str(str_id++         , "%02d"  , wvar.can_node_number));

  if (Time_elapsed_msec(&save_t)>500)
  {
    gx_widget_hide(&window_can_id.window_can_id_prmpt_Saved);
  }
  else
  {
    gx_widget_show(&window_can_id.window_can_id_prmpt_Saved);
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
    int32_t tmp = (int32_t)wvar.can_node_number;
    tmp += delta;
    if (tmp > MAX_CAN_NODE_NUM)
    {
      tmp = MAX_CAN_NODE_NUM;
    }
    else if (tmp < 1)
    {
      tmp = 1;
    }
    wvar.can_node_number = (uint8_t)tmp;
  }


  if (Get_switch_press_signal())
  {
    if (ret_func!=0)
    {
      gx_system_timer_stop((GX_WIDGET *)can_id_edit_screen, ENCODER_PROC_TIMER_ID);
      gx_widget_detach((GX_WIDGET*)can_id_edit_screen);
      ret_func(0);
    }
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
VOID Can_id_draw_callback(GX_WINDOW *window)
{
  gx_window_draw(window);
  gx_widget_children_draw(window);
}

/*-----------------------------------------------------------------------------------------------------


  \param window
  \param event_ptr

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT Can_id_event_callback(GX_WINDOW *window, GX_EVENT *event_ptr)
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

