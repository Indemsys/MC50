﻿#ifndef NET_THREAD_H
  #define NET_THREAD_H

#define EVT_MQTT_MSG        BIT(0)

extern uint8_t              g_BSD_initialised;

void  Thread_net_create(void);
UINT  Send_event_to_Net_task(uint32_t event_flag);

#endif // NET_THREAD_H



