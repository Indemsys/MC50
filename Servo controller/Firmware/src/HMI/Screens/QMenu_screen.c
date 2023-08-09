// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-15
// 12:20:40
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"


static GX_WINDOW              *qmenu_screen;

static void Qmenu_open(void);
static void Qmenu_close(void);
static void Qmenu_stop(void);
static void Qmenu_freewheeling(void);

static const T_gui_menu_content  qmenu_content[] =
{
  { MENU_ITEM_WINDOW  ,(GX_WIDGET *)&window_qmenu.window_qmenu_prmpt_return              , Init_main_screen   },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_qmenu.window_qmenu_prmpt_open                , Qmenu_open         },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_qmenu.window_qmenu_prmpt_close               , Qmenu_close        },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_qmenu.window_qmenu_prmpt_stop                , Qmenu_stop         },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_qmenu.window_qmenu_prmpt_freewheeling        , Qmenu_freewheeling },
};

#define HMI_QMENU_ITEMS_COUNT (sizeof(qmenu_content)/sizeof(qmenu_content[0]))


static T_menu_cbl qmenu_cbl;


#define   MENU_VERSION_STR_ID  0

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Qmenu_open(void)
{
  MC_set_open();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Qmenu_close(void)
{
  MC_set_close();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Qmenu_stop(void)
{
  MC_set_stop_motor();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Qmenu_freewheeling(void)
{
  MC_set_freewheeling();
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_qmenu_screen(void)
{
  if (qmenu_screen == 0)
  {
    gx_studio_named_widget_create("window_qmenu" , 0, (GX_WIDGET **)&qmenu_screen);
  }
  qmenu_cbl.enc_cnt = Get_encoder_counter();
  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)qmenu_screen);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Show_qmenu(void)
{
  char ver_str[64];


  Get_build_date_time(ver_str);

  gx_widget_style_remove(qmenu_content[qmenu_cbl.item_num_prev].prompt, GX_STYLE_DRAW_SELECTED);
  gx_widget_style_add(qmenu_content[qmenu_cbl.item_num].prompt, GX_STYLE_DRAW_SELECTED);


  gx_prompt_text_set_ext(&window_qmenu.window_qmenu_Val_shaft_rot_speed    ,GUI_print_str(0   , "%0.1f" ,(double)adc.shaft_speed));
  gx_prompt_text_set_ext(&window_qmenu.window_qmenu_Val_shaft_position     ,GUI_print_str(1   , "%0.1f" ,(double)adc.shaft_position_grad));
  gx_prompt_text_set_ext(&window_qmenu.window_qmenu_prmpt_ver              ,GUI_print_str(2   , "%s"    ,ver_str));

}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static uint32_t _Encoder_processing(void)
{

  int32_t n     = qmenu_cbl.item_num;
  int32_t delta = Get_encoder_counter_delta(&qmenu_cbl.enc_cnt);

  if (delta != 0)
  {
    if (delta > 0)
    {
      if (n > 0) n--;
    }
    else
    {
      if (n < (HMI_QMENU_ITEMS_COUNT-1)) n++;
    }
    qmenu_cbl.item_num_prev = qmenu_cbl.item_num;
    qmenu_cbl.item_num = n;
  }

  if (Get_switch_press_signal())
  {
    if (qmenu_content[qmenu_cbl.item_num].menu_item_type == MENU_ITEM_WINDOW)
    {
      if (qmenu_content[qmenu_cbl.item_num].item_function)
      {
        gx_system_timer_stop((GX_WIDGET *)qmenu_screen, ENCODER_PROC_TIMER_ID);
        gx_widget_detach((GX_WIDGET *)qmenu_screen);
        qmenu_content[qmenu_cbl.item_num].item_function();
        return 0;
      }
    }
    else if (qmenu_content[qmenu_cbl.item_num].menu_item_type == MENU_ITEM_COMMAND)
    {
      if (qmenu_content[qmenu_cbl.item_num].item_function) qmenu_content[qmenu_cbl.item_num].item_function();
    }
  }
  return 1;
}

/*-----------------------------------------------------------------------------------------------------
  Вызывается при выполнении функции gx_system_canvas_refresh  в контексте задачи GUIX System Thread

  \param p_widget
-----------------------------------------------------------------------------------------------------*/
VOID Qmenu_draw_callback(GX_WINDOW *window)
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
UINT Qmenu_event_callback(GX_WINDOW *window, GX_EVENT *event_ptr)
{
  UINT status;

  switch (event_ptr->gx_event_type)
  {
  case GX_EVENT_SHOW:
    Show_qmenu();
    gx_system_timer_start((GX_WIDGET *)window, ENCODER_PROC_TIMER_ID, ENCODER_PROC_INTIT_TICKS, ENCODER_PROC_PERIOD_TICKS);
    status = gx_window_event_process(window, event_ptr);
    break;

  case GX_EVENT_TIMER:
    if (event_ptr->gx_event_payload.gx_event_timer_id == ENCODER_PROC_TIMER_ID)
    {
      if (_Encoder_processing()) Show_qmenu();
    }
    break;

  default:
    status = gx_window_event_process(window, event_ptr);
    return status;
  }

  return 0;
}





