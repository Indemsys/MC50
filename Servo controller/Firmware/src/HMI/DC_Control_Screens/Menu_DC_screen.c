﻿// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-15
// 12:20:40
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"


static GX_WINDOW              *menu_DC_screen;

static void Do_Reset_system(void *p);

static const T_gui_menu_content  menu_DC_content[] =
{
  { MENU_ITEM_WINDOW  ,(GX_WIDGET*)&window_menu_DC.window_menu_DC_prmpt_can_id_edit         , Init_can_id_edit_screen       },
  { MENU_ITEM_WINDOW  ,(GX_WIDGET*)&window_menu_DC.window_menu_DC_prmpt_diagnostic          , Init_diagnostic_menu_screen   },
  { MENU_ITEM_RETURN  ,(GX_WIDGET*)&window_menu_DC.window_menu_DC_prmpt_return              , 0                             },
  { MENU_ITEM_COMMAND ,(GX_WIDGET*)&window_menu_DC.window_menu_DC_prmpt_reset               , Do_Reset_system               },
};

#define HMI_MENU_DC_ITEMS_COUNT (sizeof(menu_DC_content)/sizeof(menu_DC_content[0]))


static T_menu_cbl     menu_dc_cbl;
static T_hmi_func  ret_func;

#define   MENU_VERSION_STR_ID  0


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_menu_DC_screen(void *p)
{
  if (p!=0) ret_func = (T_hmi_func)p;
  if (menu_DC_screen == 0)
  {
    gx_studio_named_widget_create("window_menu_DC" , 0, (GX_WIDGET **)&menu_DC_screen);
  }
  menu_dc_cbl.enc_cnt = Get_encoder_counter();
  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)menu_DC_screen);

}

/*-----------------------------------------------------------------------------------------------------


  \param p
-----------------------------------------------------------------------------------------------------*/
static void Do_Reset_system(void *p)
{
  Reset_system();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Show(void)
{
  gx_widget_style_remove(menu_DC_content[menu_dc_cbl.item_num_prev].prompt, GX_STYLE_DRAW_SELECTED);
  gx_widget_style_add(menu_DC_content[menu_dc_cbl.item_num].prompt, GX_STYLE_DRAW_SELECTED);
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static uint32_t _Encoder_processing(void)
{

  int32_t n     = menu_dc_cbl.item_num;
  int32_t delta = Get_encoder_counter_delta(&menu_dc_cbl.enc_cnt);

  if (delta != 0)
  {
    if (delta > 0)
    {
      if (n > 0) n--;
    }
    else
    {
      if (n < (HMI_MENU_DC_ITEMS_COUNT-1)) n++;
    }
    menu_dc_cbl.item_num_prev = menu_dc_cbl.item_num;
    menu_dc_cbl.item_num = n;
  }

  if (Get_switch_press_signal())
  {
    if (menu_DC_content[menu_dc_cbl.item_num].menu_item_type == MENU_ITEM_WINDOW)
    {
      if (menu_DC_content[menu_dc_cbl.item_num].item_function)
      {
        gx_system_timer_stop((GX_WIDGET *)menu_DC_screen, ENCODER_PROC_TIMER_ID);
        gx_widget_detach((GX_WIDGET *)menu_DC_screen);
        menu_DC_content[menu_dc_cbl.item_num].item_function((void*)Init_menu_DC_screen);
        return 0;
      }
    }
    else if (menu_DC_content[menu_dc_cbl.item_num].menu_item_type == MENU_ITEM_COMMAND)
    {
      if (menu_DC_content[menu_dc_cbl.item_num].item_function) menu_DC_content[menu_dc_cbl.item_num].item_function(0);
    }
    else if (menu_DC_content[menu_dc_cbl.item_num].menu_item_type == MENU_ITEM_RETURN)
    {
      if (ret_func!=0)
      {
        gx_system_timer_stop((GX_WIDGET *)menu_DC_screen, ENCODER_PROC_TIMER_ID);
        gx_widget_detach((GX_WIDGET *)menu_DC_screen);
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
VOID Menu_DC_draw_callback(GX_WINDOW *window)
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
UINT Menu_DC_event_callback(GX_WINDOW *window, GX_EVENT *event_ptr)
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




