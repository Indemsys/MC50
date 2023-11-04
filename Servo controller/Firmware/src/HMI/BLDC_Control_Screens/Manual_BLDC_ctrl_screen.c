// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-15
// 12:20:40
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"


static GX_WINDOW              *man_BLDC_ctrl_screen;

static void Man_BLDC_open(void *p);
static void Man_BLDC_close(void *p);
static void Man_BLDC_stop(void *p);
static void Man_BLDC_freewheeling(void *p);

static const T_gui_menu_content  man_bldc_menu_content[] =
{
  { MENU_ITEM_RETURN  ,(GX_WIDGET *)&window_man_BLDC_ctrl.window_man_BLDC_ctrl_prmpt_return              , 0                     },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_man_BLDC_ctrl.window_man_BLDC_ctrl_prmpt_open                , Man_BLDC_open         },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_man_BLDC_ctrl.window_man_BLDC_ctrl_prmpt_close               , Man_BLDC_close        },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_man_BLDC_ctrl.window_man_BLDC_ctrl_prmpt_stop                , Man_BLDC_stop         },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_man_BLDC_ctrl.window_man_BLDC_ctrl_prmpt_freewheeling        , Man_BLDC_freewheeling },
};

#define HMI_MAN_BLDC_MENU_ITEMS_COUNT (sizeof(man_bldc_menu_content)/sizeof(man_bldc_menu_content[0]))


static T_menu_cbl man_bldc_cbl;


#define   MENU_VERSION_STR_ID  0

static T_hmi_func ret_func;
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Man_BLDC_open(void *p)
{
  App_set_flags(BLDC_SERVO_OPEN);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Man_BLDC_close(void *p)
{
  App_set_flags(BLDC_SERVO_CLOSE);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Man_BLDC_stop(void *p)
{
  App_set_flags( MOTOR_HARD_STOP);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Man_BLDC_freewheeling(void *p)
{
  App_set_flags(MOTOR_FREEWHEELING);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_man_ctrl_menu_screen(void *p)
{
  if (p!=0) ret_func = (T_hmi_func)p;
  if (man_BLDC_ctrl_screen == 0)
  {
    gx_studio_named_widget_create("window_man_BLDC_ctrl" , 0, (GX_WIDGET **)&man_BLDC_ctrl_screen);
  }
  man_bldc_cbl.enc_cnt = Get_encoder_counter();
  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)man_BLDC_ctrl_screen);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Show(void)
{
  char ver_str[64];


  Get_build_date_time(ver_str);

  gx_widget_style_remove(man_bldc_menu_content[man_bldc_cbl.item_num_prev].prompt, GX_STYLE_DRAW_SELECTED);
  gx_widget_style_add(man_bldc_menu_content[man_bldc_cbl.item_num].prompt, GX_STYLE_DRAW_SELECTED);


  gx_prompt_text_set_ext(&window_man_BLDC_ctrl.window_man_BLDC_ctrl_Val_shaft_rot_speed    ,GUI_print_str(0   , "%0.1f" ,(double)adc.shaft_speed_smoothly));
  gx_prompt_text_set_ext(&window_man_BLDC_ctrl.window_man_BLDC_ctrl_Val_shaft_position     ,GUI_print_str(1   , "%0.1f" ,(double)adc.shaft_position_grad));
  gx_prompt_text_set_ext(&window_man_BLDC_ctrl.window_man_BLDC_ctrl_prmpt_ver              ,GUI_print_str(2   , "%s"    ,ver_str));

}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static uint32_t _Encoder_processing(void)
{

  int32_t n     = man_bldc_cbl.item_num;
  int32_t delta = Get_encoder_counter_delta(&man_bldc_cbl.enc_cnt);

  if (delta != 0)
  {
    if (delta > 0)
    {
      if (n > 0) n--;
    }
    else
    {
      if (n < (HMI_MAN_BLDC_MENU_ITEMS_COUNT-1)) n++;
    }
    man_bldc_cbl.item_num_prev = man_bldc_cbl.item_num;
    man_bldc_cbl.item_num = n;
  }

  if (Get_switch_press_signal())
  {
    if (man_bldc_menu_content[man_bldc_cbl.item_num].menu_item_type == MENU_ITEM_WINDOW)
    {
      if (man_bldc_menu_content[man_bldc_cbl.item_num].item_function)
      {
        gx_system_timer_stop((GX_WIDGET *)man_BLDC_ctrl_screen, ENCODER_PROC_TIMER_ID);
        gx_widget_detach((GX_WIDGET *)man_BLDC_ctrl_screen);
        man_bldc_menu_content[man_bldc_cbl.item_num].item_function((void*)Init_man_ctrl_menu_screen);
        return 0;
      }
    }
    else if (man_bldc_menu_content[man_bldc_cbl.item_num].menu_item_type == MENU_ITEM_COMMAND)
    {
      if (man_bldc_menu_content[man_bldc_cbl.item_num].item_function) man_bldc_menu_content[man_bldc_cbl.item_num].item_function(0);
    }
    else if (man_bldc_menu_content[man_bldc_cbl.item_num].menu_item_type == MENU_ITEM_RETURN)
    {
      if (ret_func!=0)
      {
        gx_system_timer_stop((GX_WIDGET *)man_BLDC_ctrl_screen, ENCODER_PROC_TIMER_ID);
        gx_widget_detach((GX_WIDGET *)man_BLDC_ctrl_screen);
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





