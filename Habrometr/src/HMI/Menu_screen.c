// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-15
// 12:20:40
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"


static GX_WINDOW              *menu_screen;

typedef struct
{
  GX_WIDGET  *prompt;
   void (*init_screen)(void);

} T_menu_content;

const T_menu_content  menu_content[] =
{
  (GX_WIDGET*)&window_menu.window_menu_sdcard_state  , Init_sdcard_state_screen,
  (GX_WIDGET*)&window_menu.window_menu_network_state , Init_net_state_screen,
  (GX_WIDGET*)&window_menu.window_menu_habrometer    , Init_habrometr_screen,
};

#define HMU_MENU_ITEMS_COUNT (sizeof(menu_content)/sizeof(menu_content[0]))


typedef struct
{
    uint32_t item_num_prev;
    uint32_t item_num;
    int32_t  enc_cnt;

} T_menu_cbl;

T_menu_cbl menu_cbl;


#define   MENU_VERSION_STR_ID  0


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_menu_screen(void)
{
  if (menu_screen == 0)
  {
    gx_studio_named_widget_create("window_menu" , 0, (GX_WIDGET **)&menu_screen);
  }
  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)menu_screen);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Show_menu(void)
{
  GX_STRING *p_gx_str;

  gx_widget_style_remove(menu_content[menu_cbl.item_num_prev].prompt, GX_STYLE_DRAW_SELECTED);
  gx_widget_style_add(menu_content[menu_cbl.item_num].prompt, GX_STYLE_DRAW_SELECTED);

  p_gx_str = GUI_print_str(MENU_VERSION_STR_ID, "Ver: %s", __DATE__" "__TIME__);
  gx_prompt_text_set_ext(&window_menu.window_menu_version      ,p_gx_str);
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static uint32_t _Encoder_processing(void)
{

  int32_t n     = menu_cbl.item_num;
  int32_t delta = Get_encoder_counter_delta(&menu_cbl.enc_cnt);

  if (delta != 0)
  {
    if (delta > 0)
    {
      if (n > 0) n--;
    }
    else
    {
      if (n < (HMU_MENU_ITEMS_COUNT-1)) n++;
    }
    menu_cbl.item_num_prev = menu_cbl.item_num;
    menu_cbl.item_num = n;
  }

  if (Get_switch_press_signal())
  {
    if (menu_content[menu_cbl.item_num].init_screen)
    {
      gx_system_timer_stop((GX_WIDGET *)menu_screen, ENCODER_PROC_TIMER_ID);
      gx_widget_detach((GX_WIDGET*)menu_screen);
      menu_content[menu_cbl.item_num].init_screen();
      return 0;
    }
  }
  return 1;
}

/*-----------------------------------------------------------------------------------------------------
  Вызывается при выполнении функции gx_system_canvas_refresh  в контексте задачи GUIX System Thread

  \param p_widget
-----------------------------------------------------------------------------------------------------*/
VOID Menu_draw_callback(GX_WINDOW *window)
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
UINT Menu_event_callback(GX_WINDOW *window, GX_EVENT *event_ptr)
{
  UINT status;

  //ITM_EVENT32(1,event_ptr->gx_event_type);


  switch (event_ptr->gx_event_type)
  {
  case GX_EVENT_SHOW:
    Show_menu();
    gx_system_timer_start((GX_WIDGET *)window, ENCODER_PROC_TIMER_ID, ENCODER_PROC_INTIT_TICKS, ENCODER_PROC_PERIOD_TICKS);
    status = gx_window_event_process(window, event_ptr);
    break;

  case GX_EVENT_TIMER:
    if (event_ptr->gx_event_payload.gx_event_timer_id == ENCODER_PROC_TIMER_ID)
    {
      if (_Encoder_processing()) Show_menu();
    }
    break;

  default:
    status = gx_window_event_process(window, event_ptr);
    return status;
  }

  return 0;
}

