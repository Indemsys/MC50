// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2021-11-07
// 15:40:27
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "nxd_bsd.h"


#define THREAD_NET_STACK_SIZE (1024*4)
#define THREAD_BSD_STACK_SIZE (1024*2)
static uint8_t thread_net_stack[THREAD_NET_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.net_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static uint8_t thread_bsd_stack[THREAD_BSD_STACK_SIZE] BSP_PLACE_IN_SECTION_V2(".stack.bsd_thread")BSP_ALIGN_VARIABLE_V2(BSP_STACK_ALIGNMENT);
static TX_THREAD       net_thread;


uint8_t                       g_BSD_initialised;

static TX_EVENT_FLAGS_GROUP   net_flag;

static void Thread_net(ULONG initial_input);

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Thread_Net_create(void)
{
  uint32_t res;
  if ((wvar.usb_mode != USB_MODE_RNDIS) && (wvar.usb_mode != USB_MODE_HOST_ECM)) return;

  Init_Net();

  res = tx_thread_create(&net_thread, "Net", Thread_net,
    0,
    (void *)thread_net_stack, // stack_start
    THREAD_NET_STACK_SIZE,   // stack_size
    THREAD_NET_PRIORITY,     // priority. Numerical priority of thread. Legal values range from 0 through (TX_MAX_PRIORITES-1), where a value of 0 represents the highest priority.
    THREAD_NET_PRIORITY,     // preempt_threshold. Highest priority level (0 through (TX_MAX_PRIORITIES-1)) of disabled preemption. Only priorities higher than this level are allowed to preempt this thread. This value must be less than or equal to the specified priority. A value equal to the thread priority disables preemption-threshold.
    TX_NO_TIME_SLICE,
    TX_AUTO_START);
  APPLOG("Net task creation result: %d", res);
}

/*-----------------------------------------------------------------------------------------------------


  \param voi
-----------------------------------------------------------------------------------------------------*/
static void Net_task_abnormal_stop(void)
{
  APPLOG("Net task stopped.");
  do
  {
    Wait_ms(100);
  } while (1);
}


/*-----------------------------------------------------------------------------------------------------

  \param msg

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT  Send_event_to_Net_task(uint32_t event_flag)
{
  return  tx_event_flags_set(&net_flag, event_flag, TX_OR);
}
/*-----------------------------------------------------------------------------------------------------


  \param initial_input
-----------------------------------------------------------------------------------------------------*/
static void Thread_net(ULONG initial_input)
{
  UINT      res;
  ULONG     actual_flags;

  res = tx_event_flags_create(&net_flag, "Task_Net");
  if  (res != TX_SUCCESS)
  {
    APPLOG("Event creation error %d.", res);
    Net_task_abnormal_stop();
  }

  if (wvar.usb_mode == USB_MODE_RNDIS)
  {
    RNDIS_init_network_stack();
    if (bsd_initialize(rndis_ip_ptr, &net_packet_pool, (CHAR*)thread_bsd_stack, THREAD_BSD_STACK_SIZE, THREAD_BSD_PRIORITY)==TX_SUCCESS)
    {
      g_BSD_initialised = 1;
    }

  }
  else if (wvar.usb_mode == USB_MODE_HOST_ECM)
  {
    ECM_Host_init_network_stack();
    if (bsd_initialize(ecm_host_ip_ptr, &net_packet_pool, (CHAR*)thread_bsd_stack, THREAD_BSD_STACK_SIZE, THREAD_BSD_PRIORITY)==TX_SUCCESS)
    {
      g_BSD_initialised = 1;
    }
  }

  do
  {
    // Ожидаем сообщения 10 мс
    res =  tx_event_flags_get(&net_flag, 0XFFFFFFFF, TX_OR_CLEAR,&actual_flags,  MS_TO_TICKS(10));
    if (res == TX_SUCCESS)
    {
      if (actual_flags & EVT_MQTT_MSG)
      {
        // Отработка сообщения от MQTT брокера
        MQTTMC_messages_processor();
      }
    }
    else
    {
      if (wvar.usb_mode == USB_MODE_RNDIS)
      {
        RNDIS_network_controller();
      }
      else if (wvar.usb_mode == USB_MODE_HOST_ECM)
      {
        ECM_Host_network_controller();
      }
      DNS_client_controller();
      MQTT_client_controller();
      SNTP_client_controller();  // SNTP клиент создается только один на одном из двух соединений: ECM или RNDIS. При отключении обоих интерфейсов клиент удаляется.

    }

  } while (1);

}

