// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-15
// 12:20:40
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"


static GX_WINDOW              *manual_DC_ctrl_screen;

static void      Change_DC_pwm_val(void *p);
static int32_t   pwm_dc_val;
static uint8_t   pwm_dc_changer_active = 0;

static void Manual_DC_Ctrl_Rot_CW(void *p);
static void Manual_DC_Ctrl_Rot_CCW(void *p);
static void Manual_DC_Ctrl_hard_stop(void *p);
static void Manual_DC_Ctrl_freewheeling(void *p);

static const T_gui_menu_content  man_DC_ctrl_content[] =
{
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_manual_DC_ctrl.window_manual_DC_ctrl_prmpt_rot_CW        , Manual_DC_Ctrl_Rot_CW         },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_manual_DC_ctrl.window_manual_DC_ctrl_prmpt_rot_CCW       , Manual_DC_Ctrl_Rot_CCW        },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_manual_DC_ctrl.window_manual_DC_ctrl_prmpt_stop          , Manual_DC_Ctrl_hard_stop      },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_manual_DC_ctrl.window_manual_DC_ctrl_prmpt_freewheeling  , Manual_DC_Ctrl_freewheeling   },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_manual_DC_ctrl.window_manual_DC_ctrl_prmt_pwm_val        , Change_DC_pwm_val             },
  { MENU_ITEM_RETURN  ,(GX_WIDGET *)&window_manual_DC_ctrl.window_manual_DC_ctrl_prmpt_return        , 0                             },
};

#define HMI_MAN_DC_CTRL_ITEMS_COUNT (sizeof(man_DC_ctrl_content)/sizeof(man_DC_ctrl_content[0]))


#define   MAN_DC_CTRL_PRMT_PWM_VAL    1
#define   MAN_DC_CTRL_VAL_PWR         2
#define   MAN_DC_CTRL_VAL_MOTI        3
#define   MAN_DC_CTRL_VAL_RPM         4
#define   MAN_DC_CTRL_VAL_TEMP        5



static T_menu_cbl     man_dc_ctrl_cbl;
static T_hmi_func     ret_func;

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_Manual_DC_ctrl_screen(void *p)
{
  if (p!=0) ret_func = (T_hmi_func)p;
  if (manual_DC_ctrl_screen == 0)
  {
    gx_studio_named_widget_create("window_manual_DC_ctrl" , 0, (GX_WIDGET **)&manual_DC_ctrl_screen);
  }
  man_dc_ctrl_cbl.enc_cnt = Get_encoder_counter();
  pwm_dc_val = drv_cbl.pwm_val;
  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)manual_DC_ctrl_screen);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Manual_DC_Ctrl_Rot_CW(void *p)
{
  App_set_flags(DC_ROT_CW_NO_ACCEL);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Manual_DC_Ctrl_Rot_CCW(void *p)
{
  App_set_flags(DC_ROT_CCW_NO_ACCEL);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Manual_DC_Ctrl_hard_stop(void *p)
{
  App_set_flags( MOTOR_HARD_STOP);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Manual_DC_Ctrl_freewheeling(void *p)
{
  App_set_flags(MOTOR_FREEWHEELING);
}

/*-----------------------------------------------------------------------------------------------------
  Эта функция не должна ничего делать
  Изменение PWM происходит в функции _Encoder_processing

  \param void
-----------------------------------------------------------------------------------------------------*/
static void Change_DC_pwm_val(void *p)
{

}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Show(void)
{
  gx_widget_style_remove(man_DC_ctrl_content[man_dc_ctrl_cbl.item_num_prev].prompt, GX_STYLE_DRAW_SELECTED);
  gx_widget_style_add(man_DC_ctrl_content[man_dc_ctrl_cbl.item_num].prompt, GX_STYLE_DRAW_SELECTED);
  gx_prompt_text_set_ext(&window_manual_DC_ctrl.window_manual_DC_ctrl_prmt_pwm_val   ,GUI_print_str(MAN_DC_CTRL_PRMT_PWM_VAL   , "%d"    , pwm_dc_val));
  gx_prompt_text_set_ext(&window_manual_DC_ctrl.window_manual_DC_ctrl_Val_pwr        ,GUI_print_str(MAN_DC_CTRL_VAL_PWR        , "%0.0f" ,(double)adc.input_pwr));
  gx_prompt_text_set_ext(&window_manual_DC_ctrl.window_manual_DC_ctrl_Val_moti       ,GUI_print_str(MAN_DC_CTRL_VAL_MOTI       , "%0.0f" ,(double)adc.filtered_dc_motor_current));
  gx_prompt_text_set_ext(&window_manual_DC_ctrl.window_manual_DC_ctrl_Val_rpm        ,GUI_print_str(MAN_DC_CTRL_VAL_RPM        , "%0.0f" ,(double)drv_cbl.mot_rpm));
  gx_prompt_text_set_ext(&window_manual_DC_ctrl.window_manual_DC_ctrl_Val_temp       ,GUI_print_str(MAN_DC_CTRL_VAL_TEMP       , "%0.1f" ,(double)adc.drv_temp));
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static uint32_t _Encoder_processing(void)
{

  int32_t n     = man_dc_ctrl_cbl.item_num;
  int32_t delta = Get_encoder_counter_delta(&man_dc_ctrl_cbl.enc_cnt);

  if ((man_DC_ctrl_content[man_dc_ctrl_cbl.item_num].item_function == Change_DC_pwm_val) && pwm_dc_changer_active)
  {
    if (delta != 0)
    {
      if (delta > 0)
      {
        if (pwm_dc_val < 100) pwm_dc_val++;
      }
      else
      {
        if (pwm_dc_val > 0) pwm_dc_val--;
      }
      drv_cbl.pwm_val = pwm_dc_val;
    }

    if (Get_switch_press_signal())
    {
      pwm_dc_changer_active = 0;
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
        if (n < (HMI_MAN_DC_CTRL_ITEMS_COUNT-1)) n++;
      }
      man_dc_ctrl_cbl.item_num_prev = man_dc_ctrl_cbl.item_num;
      man_dc_ctrl_cbl.item_num = n;
    }

    if (Get_switch_press_signal())
    {
      if (man_DC_ctrl_content[man_dc_ctrl_cbl.item_num].menu_item_type == MENU_ITEM_WINDOW)
      {
        if (man_DC_ctrl_content[man_dc_ctrl_cbl.item_num].item_function)
        {
          gx_system_timer_stop((GX_WIDGET *)manual_DC_ctrl_screen, ENCODER_PROC_TIMER_ID);
          gx_widget_detach((GX_WIDGET *)manual_DC_ctrl_screen);
          man_DC_ctrl_content[man_dc_ctrl_cbl.item_num].item_function((void*)Init_Manual_DC_ctrl_screen);
          return 0;
        }
      }
      else if (man_DC_ctrl_content[man_dc_ctrl_cbl.item_num].menu_item_type == MENU_ITEM_COMMAND)
      {
        if (man_DC_ctrl_content[man_dc_ctrl_cbl.item_num].item_function == Change_DC_pwm_val)
        {
          pwm_dc_changer_active = 1;
        }
        else
        {
          if (man_DC_ctrl_content[man_dc_ctrl_cbl.item_num].item_function) man_DC_ctrl_content[man_dc_ctrl_cbl.item_num].item_function(0);
        }
      }
      else if (man_DC_ctrl_content[man_dc_ctrl_cbl.item_num].menu_item_type == MENU_ITEM_RETURN)
      {
        if (ret_func!=0)
        {
          gx_system_timer_stop((GX_WIDGET *)manual_DC_ctrl_screen, ENCODER_PROC_TIMER_ID);
          gx_widget_detach((GX_WIDGET *)manual_DC_ctrl_screen);
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
VOID Manual_DC_ctrl_draw_callback(GX_WINDOW *window)
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
UINT Manual_DC_ctrl_event_callback(GX_WINDOW *window, GX_EVENT *event_ptr)
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





