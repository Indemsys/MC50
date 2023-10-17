// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-15
// 12:20:40
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"


static GX_WINDOW              *diagnostic_menu_screen;


static const T_gui_menu_content  diagnostic_menu_content[] =
{
  { MENU_ITEM_WINDOW  ,(GX_WIDGET*)&window_diagnostic_menu.window_diagnostic_menu_prmpt_can_rem_ctrl        , Init_CAN_rem_ctrl_screen   },
  { MENU_ITEM_WINDOW  ,(GX_WIDGET*)&window_diagnostic_menu.window_diagnostic_menu_prmpt_sdcard_state        , Init_sdcard_state_screen   },
  { MENU_ITEM_WINDOW  ,(GX_WIDGET*)&window_diagnostic_menu.window_diagnostic_menu_prmpt_communicationt_intf , Init_net_state_screen      },
  { MENU_ITEM_RETURN  ,(GX_WIDGET*)&window_diagnostic_menu.window_diagnostic_menu_prmpt_return              , 0                          },
};

#define HMI_DIAGNOSTIC_MENU_ITEMS_COUNT (sizeof(diagnostic_menu_content)/sizeof(diagnostic_menu_content[0]))


static T_menu_cbl     diagn_menu_cbl;
static T_hmi_func     ret_func;

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_diagnostic_menu_screen(void *p)
{
  if (p!=0) ret_func = (T_hmi_func)p;
  if (diagnostic_menu_screen == 0)
  {
    gx_studio_named_widget_create("window_diagnostic_menu" , 0, (GX_WIDGET **)&diagnostic_menu_screen);
  }
  diagn_menu_cbl.enc_cnt = Get_encoder_counter();
  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)diagnostic_menu_screen);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Show(void)
{
  char ver_str[64];

  Get_build_date_time(ver_str);
  gx_prompt_text_set_ext(&window_diagnostic_menu.window_diagnostic_menu_Val_version           ,GUI_print_str(0          , "%s"    , ver_str));


  gx_widget_style_remove(diagnostic_menu_content[diagn_menu_cbl.item_num_prev].prompt, GX_STYLE_DRAW_SELECTED);
  gx_widget_style_add(diagnostic_menu_content[diagn_menu_cbl.item_num].prompt, GX_STYLE_DRAW_SELECTED);
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static uint32_t _Encoder_processing(void)
{

  int32_t n     = diagn_menu_cbl.item_num;
  int32_t delta = Get_encoder_counter_delta(&diagn_menu_cbl.enc_cnt);

  if (delta != 0)
  {
    if (delta > 0)
    {
      if (n > 0) n--;
    }
    else
    {
      if (n < (HMI_DIAGNOSTIC_MENU_ITEMS_COUNT-1)) n++;
    }
    diagn_menu_cbl.item_num_prev = diagn_menu_cbl.item_num;
    diagn_menu_cbl.item_num = n;
  }

  if (Get_switch_press_signal())
  {
    if (diagnostic_menu_content[diagn_menu_cbl.item_num].menu_item_type == MENU_ITEM_WINDOW)
    {
      if (diagnostic_menu_content[diagn_menu_cbl.item_num].item_function)
      {
        gx_system_timer_stop((GX_WIDGET *)diagnostic_menu_screen, ENCODER_PROC_TIMER_ID);
        gx_widget_detach((GX_WIDGET *)diagnostic_menu_screen);
        diagnostic_menu_content[diagn_menu_cbl.item_num].item_function((void*)Init_diagnostic_menu_screen);
        return 0;
      }
    }
    else if (diagnostic_menu_content[diagn_menu_cbl.item_num].menu_item_type == MENU_ITEM_COMMAND)
    {
      if (diagnostic_menu_content[diagn_menu_cbl.item_num].item_function) diagnostic_menu_content[diagn_menu_cbl.item_num].item_function(0);
    }
    else if (diagnostic_menu_content[diagn_menu_cbl.item_num].menu_item_type == MENU_ITEM_RETURN)
    {
      if (ret_func!=0)
      {
        gx_system_timer_stop((GX_WIDGET *)diagnostic_menu_screen, ENCODER_PROC_TIMER_ID);
        gx_widget_detach((GX_WIDGET *)diagnostic_menu_screen);
        ret_func(0);
      }
    }

  }
  return 1;
}

/*-----------------------------------------------------------------------------------------------------
  Вызывается при выполнении функции gx_system_canvas_refresh  в контексте задачи GUIX System Thread

  \param p_widget
-----------------------------------------------------------------------------------------------------*/
VOID Diagnostic_menu_draw_callback(GX_WINDOW *window)
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
UINT Diagnostic_menu_event_callback(GX_WINDOW *window, GX_EVENT *event_ptr)
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





