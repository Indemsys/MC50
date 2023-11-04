// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-15
// 12:20:40
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"


static GX_WINDOW              *w_comm_law_screen;
static uint8_t                 current_changer_active = 0;


static void _Rot_forward(void *p);
static void _Rot_reverse(void *p);
static void _Rot_stop(void *p);
static void _Rot_freewheel(void *p);
static void _Start_search(void *p);
static void _Save_law(void *p);
static void _Change_current(void *p);

static const T_gui_menu_content  comm_law_menu_content[] =
{
  { MENU_ITEM_RETURN  ,(GX_WIDGET *)&w_comm_law.w_comm_law_prmpt_return          , 0               },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&w_comm_law.w_comm_law_prmpt_rot_forward     , _Rot_forward    },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&w_comm_law.w_comm_law_prmpt_rot_reverse     , _Rot_reverse    },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&w_comm_law.w_comm_law_prmpt_stop            , _Rot_stop       },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&w_comm_law.w_comm_law_prmpt_freewheel       , _Rot_freewheel  },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&w_comm_law.w_comm_law_prmpt_search          , _Start_search   },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&w_comm_law.w_comm_law_prmpt_save            , _Save_law       },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&w_comm_law.w_comm_law_Val_current           , _Change_current },
};

#define HMI_COMM_LAW_MENU_ITEMS_COUNT (sizeof(comm_law_menu_content)/sizeof(comm_law_menu_content[0]))


static T_menu_cbl comm_law_cbl;


#define   VAL_CURRENT_STR_ID       0
#define   VAL_COMM_LAW_STR_ID      1
#define   VAL_PWM_STR_ID           2

static T_hmi_func ret_func;
/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Rot_forward(void *p)
{
  App_set_flags(BLDC_ROT_CW);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Rot_reverse(void *p)
{
  App_set_flags(BLDC_ROT_CCW);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Rot_stop(void *p)
{
  App_set_flags(MOTOR_HARD_STOP);
}

/*-----------------------------------------------------------------------------------------------------


  \param p
-----------------------------------------------------------------------------------------------------*/
static void _Rot_freewheel(void *p)
{
  App_set_flags(MOTOR_FREEWHEELING);
}

/*-----------------------------------------------------------------------------------------------------


  \param p
-----------------------------------------------------------------------------------------------------*/
static void _Change_current(void *p)
{

}


/*-----------------------------------------------------------------------------------------------------


  \param p
-----------------------------------------------------------------------------------------------------*/
static void _Save_law(void *p)
{
  Req_to_reset_log_file();
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Start_search(void *p)
{
  App_set_flags(COMMUTATION_LAW_SEARCH);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_comm_law_search_screen(void *p)
{
  if (p != 0) ret_func = (T_hmi_func)p;
  if (w_comm_law_screen == 0)
  {
    gx_studio_named_widget_create("w_comm_law" , 0, (GX_WIDGET **)&w_comm_law_screen);
  }
  comm_law_cbl.enc_cnt = Get_encoder_counter();
  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)w_comm_law_screen);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Show(void)
{
  char ver_str[64];


  Get_build_date_time(ver_str);

  gx_widget_style_remove(comm_law_menu_content[comm_law_cbl.item_num_prev].prompt, GX_STYLE_DRAW_SELECTED);
  gx_widget_style_add(comm_law_menu_content[comm_law_cbl.item_num].prompt, GX_STYLE_DRAW_SELECTED);

  gx_prompt_text_set_ext(&w_comm_law.w_comm_law_Val_current           ,GUI_print_str(VAL_CURRENT_STR_ID     , "%0.1f"    , (double)wvar.bldc_sett_max_current));
  gx_prompt_text_set_ext(&w_comm_law.w_comm_law_Val_comm_law          ,GUI_print_str(VAL_COMM_LAW_STR_ID    , "%06d"     , wvar.bldc_comm_law));
  gx_prompt_text_set_ext(&w_comm_law.w_comm_law_Val_pwm               ,GUI_print_str(VAL_PWM_STR_ID         , "%0.1f"    , (double)Get_test_pwm_percents()));



}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static uint32_t _Encoder_processing(void)
{

  int32_t n     = comm_law_cbl.item_num;
  int32_t delta = Get_encoder_counter_delta(&comm_law_cbl.enc_cnt);

  if ((comm_law_menu_content[comm_law_cbl.item_num].item_function == _Change_current) && current_changer_active)
  {
    if (delta != 0)
    {
      if (delta > 0)
      {
        if (wvar.bldc_sett_max_current < 20.0f) wvar.bldc_sett_max_current += 0.1f;
      }
      else
      {
        if (wvar.bldc_sett_max_current > 0.0f) wvar.bldc_sett_max_current -= 0.1f;
      }

      if (wvar.bldc_sett_max_current < 0.1f)
      {
        wvar.bldc_sett_max_current = 0.1f;
      }
      else if (wvar.bldc_sett_max_current > 20.0f)
      {
        wvar.bldc_sett_max_current = 20.0f;
      }
    }

    if (Get_switch_press_signal())
    {
      current_changer_active = 0;
    }
  }
  else
  {
    if (delta != 0)
    {
      if (delta > 0)
      {
        if (n > 0) n--;
      }
      else
      {
        if (n < (HMI_COMM_LAW_MENU_ITEMS_COUNT-1)) n++;
      }
      comm_law_cbl.item_num_prev = comm_law_cbl.item_num;
      comm_law_cbl.item_num = n;
    }


    if (Get_switch_press_signal())
    {
      if (comm_law_menu_content[comm_law_cbl.item_num].menu_item_type == MENU_ITEM_WINDOW)
      {
        if (comm_law_menu_content[comm_law_cbl.item_num].item_function)
        {
          gx_system_timer_stop((GX_WIDGET *)w_comm_law_screen, ENCODER_PROC_TIMER_ID);
          gx_widget_detach((GX_WIDGET *)w_comm_law_screen);
          comm_law_menu_content[comm_law_cbl.item_num].item_function((void *)Init_man_ctrl_menu_screen);
          return 0;
        }
      }
      else if (comm_law_menu_content[comm_law_cbl.item_num].menu_item_type == MENU_ITEM_COMMAND)
      {
        if (comm_law_menu_content[comm_law_cbl.item_num].item_function == _Change_current)
        {
          current_changer_active = 1;
        }
        else
        {
          if (comm_law_menu_content[comm_law_cbl.item_num].item_function) comm_law_menu_content[comm_law_cbl.item_num].item_function(0);
        }
      }
      else if (comm_law_menu_content[comm_law_cbl.item_num].menu_item_type == MENU_ITEM_RETURN)
      {
        if (ret_func != 0)
        {
          gx_system_timer_stop((GX_WIDGET *)w_comm_law_screen, ENCODER_PROC_TIMER_ID);
          gx_widget_detach((GX_WIDGET *)w_comm_law_screen);
          ret_func(0);
        }
      }
    }
  }
  return 1;
}

/*-----------------------------------------------------------------------------------------------------
  Вызывается при выполнении функции gx_system_canvas_refresh  в контексте задачи GUIX System Thread

  \param p_widget
-----------------------------------------------------------------------------------------------------*/
VOID W_comm_law_draw_callback(GX_WINDOW *window)
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
UINT W_comm_law_event_callback(GX_WINDOW *window, GX_EVENT *event_ptr)
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





