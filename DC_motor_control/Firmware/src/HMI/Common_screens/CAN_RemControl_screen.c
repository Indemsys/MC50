// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-15
// 12:20:40
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"


static GX_WINDOW              *can_rem_control_screen;

static void Change_node_num(void *p);
static uint32_t  node_num = 1;

static void Can_rem_control_open(void *p);
static void Can_rem_control_close(void *p);
static void Can_rem_control_stop(void *p);
static void Can_rem_control_freewheeling(void *p);

static const T_gui_menu_content  canrc_content[] =
{
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_CAN_rem_control.window_CAN_rem_control_prmpt_open          , Can_rem_control_open           },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_CAN_rem_control.window_CAN_rem_control_prmpt_close         , Can_rem_control_close          },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_CAN_rem_control.window_CAN_rem_control_prmpt_stop          , Can_rem_control_stop           },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_CAN_rem_control.window_CAN_rem_control_prmpt_freewheeling  , Can_rem_control_freewheeling   },
  { MENU_ITEM_COMMAND ,(GX_WIDGET *)&window_CAN_rem_control.window_CAN_rem_control_prmt_node_num       , Change_node_num                },
  { MENU_ITEM_RETURN  ,(GX_WIDGET *)&window_CAN_rem_control.window_CAN_rem_control_prmpt_return        , 0                              },
};

#define HMI_CANRC_ITEMS_COUNT (sizeof(canrc_content)/sizeof(canrc_content[0]))


static T_menu_cbl    canrc_cbl;
static T_hmi_func ret_func;

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_CAN_rem_ctrl_screen(void *p)
{
  if (p!=0) ret_func = (T_hmi_func)p;
  if (can_rem_control_screen == 0)
  {
    gx_studio_named_widget_create("window_CAN_rem_control" , 0, (GX_WIDGET **)&can_rem_control_screen);
  }
  canrc_cbl.enc_cnt = Get_encoder_counter();
  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)can_rem_control_screen);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Can_rem_control_open(void *p)
{
  uint8_t  data;
  uint32_t can_id = DMC01_REQ | (node_num << 20);

  data = DMC01_REQ_OPEN;
  CAN0_post_packet_to_send(can_id, &data , 1);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Can_rem_control_close(void *p)
{
  uint8_t  data;
  uint32_t can_id = DMC01_REQ | (node_num << 20);

  data = DMC01_REQ_CLOSE;
  CAN0_post_packet_to_send(can_id, &data , 1);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Can_rem_control_stop(void *p)
{
  uint8_t  data;
  uint32_t can_id = DMC01_REQ | (node_num << 20);

  data = DMC01_REQ_BRAKE;
  CAN0_post_packet_to_send(can_id, &data , 1);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Can_rem_control_freewheeling(void *p)
{
  uint8_t  data;
  uint32_t can_id = DMC01_REQ | (node_num << 20);

  data = DMC01_REQ_FREE_RUN;
  CAN0_post_packet_to_send(can_id, &data , 1);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void Change_node_num(void *p)
{
  node_num++;
  if (node_num > MAX_CAN_NODE_NUM)
  {
    node_num  =0;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Show(void)
{
  gx_widget_style_remove(canrc_content[canrc_cbl.item_num_prev].prompt, GX_STYLE_DRAW_SELECTED);
  gx_widget_style_add(canrc_content[canrc_cbl.item_num].prompt, GX_STYLE_DRAW_SELECTED);

  gx_prompt_text_set_ext(&window_CAN_rem_control.window_CAN_rem_control_prmt_node_num          ,GUI_print_str(0        , "%d", node_num));
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static uint32_t _Encoder_processing(void)
{

  int32_t n     = canrc_cbl.item_num;
  int32_t delta = Get_encoder_counter_delta(&canrc_cbl.enc_cnt);

  if (delta != 0)
  {
    if (delta > 0)
    {
      if (n > 0) n--;
    }
    else
    {
      if (n < (HMI_CANRC_ITEMS_COUNT-1)) n++;
    }
    canrc_cbl.item_num_prev = canrc_cbl.item_num;
    canrc_cbl.item_num = n;
  }

  if (Get_switch_press_signal())
  {
    if (canrc_content[canrc_cbl.item_num].menu_item_type == MENU_ITEM_WINDOW)
    {
      if (canrc_content[canrc_cbl.item_num].item_function)
      {
        gx_system_timer_stop((GX_WIDGET *)can_rem_control_screen, ENCODER_PROC_TIMER_ID);
        gx_widget_detach((GX_WIDGET *)can_rem_control_screen);
        canrc_content[canrc_cbl.item_num].item_function((void*)Init_CAN_rem_ctrl_screen);
        return 0;
      }
    }
    else if (canrc_content[canrc_cbl.item_num].menu_item_type == MENU_ITEM_COMMAND)
    {
      if (canrc_content[canrc_cbl.item_num].item_function) canrc_content[canrc_cbl.item_num].item_function(0);
    }
    else if (canrc_content[canrc_cbl.item_num].menu_item_type == MENU_ITEM_RETURN)
    {
      if (ret_func!=0)
      {
        gx_system_timer_stop((GX_WIDGET *)can_rem_control_screen, ENCODER_PROC_TIMER_ID);
        gx_widget_detach((GX_WIDGET *)can_rem_control_screen);
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
VOID CanRC_draw_callback(GX_WINDOW *window)
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
UINT CanRC_event_callback(GX_WINDOW *window, GX_EVENT *event_ptr)
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





