// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-15
// 12:20:40
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"


static GX_WINDOW              *dmenu_screen;


static const T_gui_menu_content  dmenu_content[] =
{
  { MENU_ITEM_WINDOW  ,(GX_WIDGET*)&window_dmenu.window_dmenu_prmpt_can_rem_ctrl        , Init_CanRC_screen        },
  { MENU_ITEM_WINDOW  ,(GX_WIDGET*)&window_dmenu.window_dmenu_prmpt_sdcard_state        , Init_sdcard_state_screen },
  { MENU_ITEM_WINDOW  ,(GX_WIDGET*)&window_dmenu.window_dmenu_prmpt_communicationt_intf , Init_net_state_screen    },
  { MENU_ITEM_WINDOW  ,(GX_WIDGET*)&window_dmenu.window_dmenu_prmpt_return              , Init_menu_screen         },
};

#define HMI_DMENU_ITEMS_COUNT (sizeof(dmenu_content)/sizeof(dmenu_content[0]))


static T_menu_cbl dmenu_cbl;


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_dmenu_screen(void)
{
  if (dmenu_screen == 0)
  {
    gx_studio_named_widget_create("window_dmenu" , 0, (GX_WIDGET **)&dmenu_screen);
  }
  dmenu_cbl.enc_cnt = Get_encoder_counter();
  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)dmenu_screen);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Show_dmenu(void)
{
  gx_widget_style_remove(dmenu_content[dmenu_cbl.item_num_prev].prompt, GX_STYLE_DRAW_SELECTED);
  gx_widget_style_add(dmenu_content[dmenu_cbl.item_num].prompt, GX_STYLE_DRAW_SELECTED);
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static uint32_t _Encoder_processing(void)
{

  int32_t n     = dmenu_cbl.item_num;
  int32_t delta = Get_encoder_counter_delta(&dmenu_cbl.enc_cnt);

  if (delta != 0)
  {
    if (delta > 0)
    {
      if (n > 0) n--;
    }
    else
    {
      if (n < (HMI_DMENU_ITEMS_COUNT-1)) n++;
    }
    dmenu_cbl.item_num_prev = dmenu_cbl.item_num;
    dmenu_cbl.item_num = n;
  }

  if (Get_switch_press_signal())
  {
    if (dmenu_content[dmenu_cbl.item_num].menu_item_type == MENU_ITEM_WINDOW)
    {
      if (dmenu_content[dmenu_cbl.item_num].item_function)
      {
        gx_system_timer_stop((GX_WIDGET *)dmenu_screen, ENCODER_PROC_TIMER_ID);
        gx_widget_detach((GX_WIDGET *)dmenu_screen);
        dmenu_content[dmenu_cbl.item_num].item_function();
        return 0;
      }
    }
    else if (dmenu_content[dmenu_cbl.item_num].menu_item_type == MENU_ITEM_COMMAND)
    {
      if (dmenu_content[dmenu_cbl.item_num].item_function) dmenu_content[dmenu_cbl.item_num].item_function();
    }
  }
  return 1;
}

/*-----------------------------------------------------------------------------------------------------
  Вызывается при выполнении функции gx_system_canvas_refresh  в контексте задачи GUIX System Thread

  \param p_widget
-----------------------------------------------------------------------------------------------------*/
VOID Dmenu_draw_callback(GX_WINDOW *window)
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
UINT Dmenu_event_callback(GX_WINDOW *window, GX_EVENT *event_ptr)
{
  UINT status;

  switch (event_ptr->gx_event_type)
  {
  case GX_EVENT_SHOW:
    Show_dmenu();
    gx_system_timer_start((GX_WIDGET *)window, ENCODER_PROC_TIMER_ID, ENCODER_PROC_INTIT_TICKS, ENCODER_PROC_PERIOD_TICKS);
    status = gx_window_event_process(window, event_ptr);
    break;

  case GX_EVENT_TIMER:
    if (event_ptr->gx_event_payload.gx_event_timer_id == ENCODER_PROC_TIMER_ID)
    {
      if (_Encoder_processing()) Show_dmenu();
    }
    break;

  default:
    status = gx_window_event_process(window, event_ptr);
    return status;
  }

  return 0;
}





