// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2016.07.07
// 15:07:12
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

extern   T_can_statistic             can_stat;

/*-----------------------------------------------------------------------------------------------------

 \param p_rx

 \return int32_t
-----------------------------------------------------------------------------------------------------*/
static int32_t Execute_CAN_cmd(T_can_msg  *can_msg)
{
  int32_t  ret  = 0;
  uint32_t node_mask =(wvar.can_node_number & 0x1F)<< 20;

  switch (can_msg->data[0])
  {
  case DMC01_REQ_SET0GRAD:
    APPLOG("CAN command: SET 0 GRAD");
    MC_set_close_position();
    CAN0_post_packet_to_send(DMC01_ANS | node_mask, can_msg->data,  1);
    break;
  case DMC01_REQ_SET90GRAD:
    APPLOG("CAN command: SET 90 GRAD");
    MC_set_open_position();
    CAN0_post_packet_to_send(DMC01_ANS | node_mask, can_msg->data,  1);
    break;
  case DMC01_REQ_SETOPENANGLE:
    APPLOG("CAN command: SET OPEN ANGLE");
    MC_set_open_position();
    CAN0_post_packet_to_send(DMC01_ANS | node_mask, can_msg->data,  1);
    break;
  case DMC01_REQ_SETCLOSEANGLE:
    APPLOG("CAN command: SET CLOSE ANGLE");
    MC_set_close_position();
    CAN0_post_packet_to_send(DMC01_ANS | node_mask, can_msg->data,  1);
    break;
  case DMC01_REQ_OPEN:
    APPLOG("CAN command: OPEN");
    MC_set_open();
    CAN0_post_packet_to_send(DMC01_ANS | node_mask, can_msg->data,  1);
    break;
  case DMC01_REQ_CLOSE:
    APPLOG("CAN command: CLOSE");
    MC_set_close();
    CAN0_post_packet_to_send(DMC01_ANS | node_mask, can_msg->data,  1);
    break;
  case DMC01_REQ_BRAKE:
    APPLOG("CAN command: BREAK");
    MC_set_stop_motor();
    CAN0_post_packet_to_send(DMC01_ANS | node_mask, can_msg->data,  1);
    break;
  case DMC01_REQ_CLOS_PRESS_PULSE:
    APPLOG("CAN command: CLOSE PRESS IMPULSE");
    MC_set_closing_pulse();
    CAN0_post_packet_to_send(DMC01_ANS | node_mask, can_msg->data,  1);
    break;
  case DMC01_REQ_OPEN_PRESS_PULSE:
    APPLOG("CAN command: OPEN PRESS IMPULSE");
    MC_set_opening_pulse();
    CAN0_post_packet_to_send(DMC01_ANS | node_mask, can_msg->data,  1);
    break;
  case DMC01_REQ_FREE_RUN:
    APPLOG("CAN command: FREE RUN");
    MC_set_freewheeling();
    CAN0_post_packet_to_send(DMC01_ANS | node_mask, can_msg->data,  1);
    break;
  case DMC01_REQ_RESET:
    APPLOG("CAN command: Reset System");
    Wait_ms(50);
    Reset_system();
    break;
  default:
    ret = -1;
    break;
  }
  return ret;
}

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void CAN_send_status(void)
{
  uint8_t  data[8];
  uint32_t flags = App_get_error_flags();
  data[0] = DMC01_REQ_STATUS;
  memcpy(&data[1],&flags, sizeof(flags));
  uint32_t  node_mask =(wvar.can_node_number & 0x1F)<< 20;
  CAN0_post_packet_to_send(DMC01_ANS | node_mask, data, sizeof(flags)+ 1);
}


/*------------------------------------------------------------------------------


 ------------------------------------------------------------------------------*/
void CAN_packet_reciever(T_can_msg  *mailbox_ptr)
{
  uint32_t msg_len;
  uint8_t  *data;
  uint32_t  node_id;


  data   = (uint8_t *)mailbox_ptr->data;
  msg_len = mailbox_ptr->len;

  if (((mailbox_ptr->can_id & 0xFE0FFFFF) == DMC01_REQ) && (msg_len == 1))
  {
    node_id =(mailbox_ptr->can_id >> 20) & 0x1F;

    if (node_id == wvar.can_node_number)
    {

      int32_t ret = Execute_CAN_cmd(mailbox_ptr);

      // Если на запрос не было найдено обработчика в процедуре выше, то на запрос DMC01_REQ_STATUS отвечаем не проверя адрес
      if (ret < 0)
      {
        switch (data[0])
        {
        case DMC01_REQ_STATUS:
          CAN_send_status();
          break;
        }
      }
    }
  }
}

