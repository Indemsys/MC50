#ifndef CAN_COMM_H
  #define CAN_COMM_H


typedef struct
{
    uint32_t tx_err_cnt;
}
T_can_statistic;


void     CAN_packet_reciever(T_can_msg  *mailbox_ptr);
void     CAN_send_status(void);

#endif



