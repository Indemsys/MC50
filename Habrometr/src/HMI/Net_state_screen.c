// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-16
// 11:43:20
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"


static GX_WINDOW   *net_state_screen;


#define  ID_VAL_VID            0
#define  ID_VAL_PID            1
#define  ID_VAL_MAC            2
#define  ID_VAL_DHCP           3
#define  ID_VAL_IP             4
#define  ID_VAL_MASK           5
#define  ID_VAL_GATE           6
#define  ID_VAL_CONNECTION     7


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_net_state_screen(void)
{
  if (net_state_screen == 0)
  {
    gx_studio_named_widget_create("window_net_state" , 0, (GX_WIDGET **)&net_state_screen);
  }
  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)net_state_screen);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Show_ECM_link_info(void)
{
  char str[GX_STRING_MAX_LEN+1];
  char str2[GX_STRING_MAX_LEN+1];

  if (g_uinf.inserted == 0)
  {
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_VID          ,GUI_print_str(ID_VAL_VID        , ""));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_PID          ,GUI_print_str(ID_VAL_PID        , ""));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_MAC          ,GUI_print_str(ID_VAL_MAC        , ""));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_DHCP         ,GUI_print_str(ID_VAL_DHCP       , "CDC ECM host mode"));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_IP           ,GUI_print_str(ID_VAL_IP         , ""));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_MASK         ,GUI_print_str(ID_VAL_MASK       , ""));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_GATE         ,GUI_print_str(ID_VAL_GATE       , ""));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_Connection   ,GUI_print_str(ID_VAL_CONNECTION , "DISCONNECTED"));

    gx_prompt_text_color_set(&window_net_state.window_net_state_Val_Connection,GX_COLOR_ID_RED, GX_COLOR_ID_RED, GX_COLOR_ID_RED);
  }
  else
  {
    ECM_Get_MAC(str, GX_STRING_MAX_LEN);
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_MAC          ,GUI_print_str(ID_VAL_MAC      , "%s", str));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_VID          ,GUI_print_str(ID_VAL_VID      , "%04X", g_uinf.idVendor));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_PID          ,GUI_print_str(ID_VAL_PID      , "%04X", g_uinf.idProduct));

    if (Is_ECM_usb_link_up() == NX_FALSE)
    {
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_DHCP         ,GUI_print_str(ID_VAL_DHCP       , "link down"));
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_IP           ,GUI_print_str(ID_VAL_IP         , ""));
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_MASK         ,GUI_print_str(ID_VAL_MASK       , ""));
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_GATE         ,GUI_print_str(ID_VAL_GATE       , ""));
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_Connection   ,GUI_print_str(ID_VAL_CONNECTION , "DISCONNECTED"));

      gx_prompt_text_color_set(&window_net_state.window_net_state_Val_Connection,GX_COLOR_ID_RED, GX_COLOR_ID_RED, GX_COLOR_ID_RED);
    }
    else
    {
      if (wvar.en_dhcp_client)
      {
        gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_DHCP         ,GUI_print_str(ID_VAL_DHCP       , "ECM DHCP client"));
      }
      else
      {
        gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_DHCP         ,GUI_print_str(ID_VAL_DHCP       , "ECM Static IP"));
      }
      ECM_Get_MASK_IP(str, str2, GX_STRING_MAX_LEN);
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_IP     ,GUI_print_str(ID_VAL_IP , "%s", str));
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_MASK   ,GUI_print_str(ID_VAL_MASK , "%s", str2));

      ECM_Get_Gateway_IP(str, GX_STRING_MAX_LEN);
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_GATE   ,GUI_print_str(ID_VAL_GATE , "%s", str));
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_Connection   ,GUI_print_str(ID_VAL_CONNECTION , "CONNECTED"));
      gx_prompt_text_color_set(&window_net_state.window_net_state_Val_Connection,GX_COLOR_ID_GREEN, GX_COLOR_ID_GREEN, GX_COLOR_ID_GREEN);
    }
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void _Show_RNDIS_link_info(void)
{
  char str[GX_STRING_MAX_LEN+1];
  char str2[GX_STRING_MAX_LEN+1];


  if (g_uinf.inserted == 0)
  {
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_VID            ,GUI_print_str(ID_VAL_VID        , ""));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_PID            ,GUI_print_str(ID_VAL_PID        , ""));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_MAC            ,GUI_print_str(ID_VAL_MAC        , ""));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_DHCP           ,GUI_print_str(ID_VAL_DHCP       , "RNDIS device mode"));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_IP             ,GUI_print_str(ID_VAL_IP         , ""));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_MASK           ,GUI_print_str(ID_VAL_MASK       , ""));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_GATE           ,GUI_print_str(ID_VAL_GATE       , ""));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_Connection     ,GUI_print_str(ID_VAL_CONNECTION , "DISCONNECTED"));

    gx_prompt_text_color_set(&window_net_state.window_net_state_Val_Connection,GX_COLOR_ID_RED, GX_COLOR_ID_RED, GX_COLOR_ID_RED);
  }
  else
  {
    RNDIS_Get_MAC(str, GX_STRING_MAX_LEN);
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_MAC            ,GUI_print_str(ID_VAL_MAC      , "%s", str));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_VID            ,GUI_print_str(ID_VAL_VID      , "%04X", g_uinf.idVendor));
    gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_PID            ,GUI_print_str(ID_VAL_PID      , "%04X", g_uinf.idProduct));

    if (Is_RNDIS_network_active() == NX_FALSE)
    {
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_DHCP         ,GUI_print_str(ID_VAL_DHCP       , "link down"));
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_IP           ,GUI_print_str(ID_VAL_IP         , ""));
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_MASK         ,GUI_print_str(ID_VAL_MASK       , ""));
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_GATE         ,GUI_print_str(ID_VAL_GATE       , ""));
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_Connection   ,GUI_print_str(ID_VAL_CONNECTION , "DISCONNECTED"));

      gx_prompt_text_color_set(&window_net_state.window_net_state_Val_Connection,GX_COLOR_ID_RED, GX_COLOR_ID_RED, GX_COLOR_ID_RED);
    }
    else
    {
      if (wvar.rndis_config == RNDIS_CONFIG_PRECONFIGURED_DHCP_SERVER)
      {
        gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_DHCP       ,GUI_print_str(ID_VAL_DHCP       , "RNDIS DHCP server"));
      }
      else
      {
        gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_DHCP       ,GUI_print_str(ID_VAL_DHCP       , "RNDIS Static IP"));
      }

      RNDIS_Get_MASK_IP(str, str2, GX_STRING_MAX_LEN);
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_IP           ,GUI_print_str(ID_VAL_IP , "%s", str));
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_MASK         ,GUI_print_str(ID_VAL_MASK , "%s", str2));


      RNDIS_Get_Gateway_IP(str, GX_STRING_MAX_LEN);
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_GATE         ,GUI_print_str(ID_VAL_GATE , "%s", str));
      gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_Connection   ,GUI_print_str(ID_VAL_CONNECTION , "CONNECTED"));
      gx_prompt_text_color_set(&window_net_state.window_net_state_Val_Connection ,GX_COLOR_ID_GREEN, GX_COLOR_ID_GREEN, GX_COLOR_ID_GREEN);
    }
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void _Show_None_info(void)
{
  gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_VID          ,GUI_print_str(ID_VAL_VID        , ""));
  gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_PID          ,GUI_print_str(ID_VAL_PID        , ""));
  gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_MAC          ,GUI_print_str(ID_VAL_MAC        , ""));
  gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_DHCP         ,GUI_print_str(ID_VAL_DHCP       , ""));
  gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_IP           ,GUI_print_str(ID_VAL_IP         , ""));
  gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_MASK         ,GUI_print_str(ID_VAL_MASK       , ""));
  gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_GATE         ,GUI_print_str(ID_VAL_GATE       , ""));
  gx_prompt_text_set_ext(&window_net_state.window_net_state_Val_Connection   ,GUI_print_str(ID_VAL_CONNECTION , "Wrong USB mode"));

  gx_prompt_text_color_set(&window_net_state.window_net_state_Val_Connection,GX_COLOR_ID_RED, GX_COLOR_ID_RED, GX_COLOR_ID_RED);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Show_net_state(void)
{
  if (wvar.usb_mode == USB_MODE_HOST_ECM)
  {
    _Show_ECM_link_info();
  }
  else if (wvar.usb_mode == USB_MODE_RNDIS)
  {
    _Show_RNDIS_link_info();
  }
  else
  {
    _Show_None_info();
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Encoder_processing(void)
{
  if (Get_switch_press_signal())
  {
    gx_system_timer_stop((GX_WIDGET *)net_state_screen, ENCODER_PROC_TIMER_ID);
    gx_widget_detach((GX_WIDGET*)net_state_screen);
    Init_menu_screen();
    return 0;
  }
  return 1;
}

/*-----------------------------------------------------------------------------------------------------


  \param window
-----------------------------------------------------------------------------------------------------*/
VOID Net_state_draw_callback(GX_WINDOW *window)
{
  gx_window_draw(window);
  gx_widget_children_draw(window);
}



/*-----------------------------------------------------------------------------------------------------


  \param window
  \param event_ptr

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT Net_state_event_callback(GX_WINDOW *window, GX_EVENT *event_ptr)
{
  UINT status;

  switch (event_ptr->gx_event_type)
  {
  case GX_EVENT_SHOW:
    _Show_net_state();
    gx_system_timer_start((GX_WIDGET *)window, ENCODER_PROC_TIMER_ID, ENCODER_PROC_INTIT_TICKS, ENCODER_PROC_PERIOD_TICKS);
    status = gx_window_event_process(window, event_ptr);
    break;

  case GX_EVENT_TIMER:
    if (event_ptr->gx_event_payload.gx_event_timer_id == ENCODER_PROC_TIMER_ID)
    {
      if (_Encoder_processing()) _Show_net_state();
    }
    break;

  default:
    status = gx_window_event_process(window, event_ptr);
    return status;
  }

  return 0;
}

