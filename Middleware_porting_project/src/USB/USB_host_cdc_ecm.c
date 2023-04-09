#include   "MC50.h"
#include   "ux_hcd_synergy.h"



/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
uint32_t USB_host_cdc_ecm_init(void)
{
  uint32_t res;

  res =  ux_host_stack_initialize(USB_host_change_function);
  if (res == UX_SUCCESS)
  {
    // Еше возможные варианты
    // ux_host_stack_class_register(_ux_system_host_class_hub_name, ux_host_class_hub_entry);
    // ux_host_stack_class_register(_ux_system_host_class_cdc_acm_name, ux_host_class_cdc_acm_entry);
    // ux_host_stack_class_register(_ux_system_host_class_hid_name, ux_host_class_hid_entry);
    // ux_host_class_hid_clients_register();
    // ux_host_stack_class_register_storage();
    // ux_host_stack_class_register(_ux_system_host_class_video_name, ux_host_class_video_entry);
    // ux_host_stack_class_register(_ux_system_host_class_audio_name, ux_host_class_audio_entry);



    res =  ux_host_stack_class_register(_ux_system_host_class_cdc_ecm_name, ux_host_class_cdc_ecm_entry);
    if (res == UX_SUCCESS)
    {
      res = ux_host_stack_hcd_register((UCHAR *) "g_sf_el_ux_hcd_fs_0", USB_initialize_hcd_transfer_support, R_USBFS_BASE, UX_SYNERGY_CONTROLLER);
      if (res != UX_SUCCESS)
      {
        ECM_DEBUG_LOG("E: ux_host_stack_hcd_register error %04X (%s %n)", res,__FUNCTION__, __LINE__);
      }
      else
      {
        ECM_DEBUG_LOG("USB_host_cdc_ecm_init done successfully!");
      }
    }
    else
    {
      ECM_DEBUG_LOG("E: ux_host_stack_class_register error %04X (%s %n)", res,__FUNCTION__, __LINE__);
    }
  }
  else
  {
    ECM_DEBUG_LOG("E: ux_host_stack_initialize error %04X (%s %n)", res,__FUNCTION__, __LINE__);
  }

  return res;
}


/*-----------------------------------------------------------------------------------------------------
  Функция возвращает NX_TRUE если к Ethernet-USB адаптеру присоедена сеть Ethernet
  Если сеть Ethernet отсоединена или не была присоеденена, то возвращает NX_FALSE

  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t Is_ECM_usb_link_up(void)
{
  if (g_uinf.ecm_class_ptr == 0)
  {
    return NX_FALSE;
  }
  USB_NETWORK_DEVICE_TYPE *ecm_net_dev_ptr = (USB_NETWORK_DEVICE_TYPE *)g_uinf.ecm_class_ptr->ux_host_class_cdc_ecm_network_handle;
  if (ecm_net_dev_ptr == 0)
  {
    return NX_FALSE;
  }
  return ecm_net_dev_ptr->ux_network_device_usb_link_up;
}

/*-----------------------------------------------------------------------------------------------------
  Функция возвращает NX_TRUE если к Ethernet-USB адаптеру присоедена сеть Ethernet
  Если сеть Ethernet была отсоединена , то возвращает NX_FALSE
  !!! После сброса системы при подключенном адаптере всегда возвращает NX_TRUE, даже если сеть отсоединена.

  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t Is_ECM_usb_network_link_up(void)
{
  if (g_uinf.ecm_class_ptr == 0) return NX_FALSE;
  USB_NETWORK_DEVICE_TYPE *ecm_net_dev_ptr = (USB_NETWORK_DEVICE_TYPE *)g_uinf.ecm_class_ptr->ux_host_class_cdc_ecm_network_handle;
  if (ecm_net_dev_ptr == 0) return NX_FALSE;
  return ecm_net_dev_ptr->ux_network_device_link_status;
}

/*-----------------------------------------------------------------------------------------------------


  \param mac_str

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ECM_Get_MAC(char* mac_str, uint32_t max_str_len)
{
  mac_str[0] = 0;
  if (g_uinf.ecm_class_ptr == 0) return RES_ERROR;
  uint8_t *m = g_uinf.ecm_class_ptr->ux_host_class_cdc_ecm_node_id;
  // К этому моменту уже была выполнена функция _ux_network_driver_activate
  snprintf(mac_str, max_str_len, "%02X:%02X:%02X:%02X:%02X:%02X", m[0],m[1],m[2],m[3],m[4],m[5]);
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param ip_str
  \param max_str_len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ECM_Get_MASK_IP(char* ip_str, char* mask_str,uint32_t max_str_len)
{
  ULONG                    ip_address;
  ULONG                    network_mask;
  USB_NETWORK_DEVICE_TYPE *netdev;

  ip_str[0] = 0;
  mask_str[0] = 0;
  if (g_uinf.ecm_class_ptr == 0) return RES_ERROR;

  netdev = (USB_NETWORK_DEVICE_TYPE *)(g_uinf.ecm_class_ptr->ux_host_class_cdc_ecm_network_handle);
  nx_ip_address_get(netdev->ux_network_device_ip_instance,&ip_address,&network_mask);
  snprintf(ip_str, max_str_len, "%03d.%03d.%03d.%03d", IPADDR(ip_address));
  snprintf(mask_str, max_str_len, "%03d.%03d.%03d.%03d", IPADDR(network_mask));
  return RES_OK;
}

/*-----------------------------------------------------------------------------------------------------


  \param gate_str
  \param max_str_len

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t ECM_Get_Gateway_IP(char* gate_str,uint32_t max_str_len)
{
  ULONG                    ip_address;
  USB_NETWORK_DEVICE_TYPE *netdev;

  gate_str[0] = 0;
  if (g_uinf.ecm_class_ptr == 0) return RES_ERROR;

  netdev = (USB_NETWORK_DEVICE_TYPE *)(g_uinf.ecm_class_ptr->ux_host_class_cdc_ecm_network_handle);
  nx_ip_gateway_address_get(netdev->ux_network_device_ip_instance,&ip_address);
  snprintf(gate_str, max_str_len, "%03d.%03d.%03d.%03d", IPADDR(ip_address));
  return RES_OK;
}


/*-----------------------------------------------------------------------------------------------------
   Функция вызывается по цепочке:
   ux_system_host_enum_thread->_ux_host_stack_rh_change_process->_ux_host_stack_rh_device_insertion->...
   _ux_host_stack_new_device_create->_ux_host_stack_class_interface_scan->_ux_host_stack_configuration_interface_scan->...
   _ux_host_class_cdc_ecm_entry->_ux_host_class_cdc_ecm_activate

  \param event
  \param host_class
  \param instance

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT USB_host_change_function(ULONG event, UX_HOST_CLASS *host_class, VOID *instance)
{

  if (event == UX_DEVICE_INSERTION) /* Check if there is a device insertion. */
  {
    ECM_DEBUG_LOG("USB_host_change_function  event %d: Inserted USB device", event);

    if (_ux_utility_memory_compare(_ux_system_host_class_cdc_ecm_name, host_class->ux_host_class_name ,_ux_utility_string_length_get(_ux_system_host_class_cdc_ecm_name)) == UX_SUCCESS)
    {
      UX_HOST_CLASS_CDC_ECM  *p_class = UX_NULL;

      p_class = (UX_HOST_CLASS_CDC_ECM *)instance;

      g_uinf.idVendor  = p_class->ux_host_class_cdc_ecm_device->ux_device_descriptor.idVendor;
      g_uinf.idProduct = p_class->ux_host_class_cdc_ecm_device->ux_device_descriptor.idProduct;
      g_uinf.dev_state = p_class->ux_host_class_cdc_ecm_device->ux_device_state;

      g_uinf.interface_id  = p_class->ux_host_class_cdc_ecm_interface_data->ux_interface_descriptor.bInterfaceClass;
      g_uinf.interface_num = p_class->ux_host_class_cdc_ecm_interface_data->ux_interface_descriptor.bInterfaceNumber;

      ECM_DEBUG_LOG("Inserted USB ECM device VID=%04X PID=%04X Intf.Class=%04X Intf.Num=%d Dev.State=%d. Class=%s", g_uinf.idVendor , g_uinf.idProduct, g_uinf.interface_id, g_uinf.interface_num, g_uinf.dev_state, host_class->ux_host_class_name);
      APPLOG("Inserted USB ECM device VID=%04X PID=%04X Intf.Class=%04X Intf.Num=%d Dev.State=%d. Class=%s", g_uinf.idVendor , g_uinf.idProduct, g_uinf.interface_id, g_uinf.interface_num, g_uinf.dev_state, host_class->ux_host_class_name);

      uint8_t *m = p_class->ux_host_class_cdc_ecm_node_id;
      // К этому моменту уже была выполнена функция _ux_network_driver_activate

      ECM_DEBUG_LOG("USB ECM MAC ADDRESS: %02X %02X %02X %02X %02X %02X", m[0],m[1],m[2],m[3],m[4],m[5]);
      APPLOG("USB ECM MAC ADDRESS: %02X %02X %02X %02X %02X %02X", m[0],m[1],m[2],m[3],m[4],m[5]);
      // Далее из контекста _ux_host_class_cdc_ecm_thread будет вызвана функция _ux_network_driver_link_up
      // в которой будет установлен флаг usb_network_device_ptr -> ux_network_device_usb_link_up = NX_TRUE;
      // и будет установлен usb_network_device_ptr -> ux_network_device_link_status = NX_TRUE; если перед этим был запущен сетевой стек

      g_uinf.ecm_class_ptr   = p_class;
      g_uinf.inserted = 1;
    }
    else
    {
      ECM_DEBUG_LOG("E: unknown host class %s (%s %n)", host_class->ux_host_class_name,__FUNCTION__, __LINE__);
    }
  }
  else if (event == UX_DEVICE_REMOVAL) /* Check if there is a device removal. */
  {
    ECM_DEBUG_LOG("USB_host_change_function  event %d: Removed USB device", event);
    APPLOG("Removed USB device.");
    g_uinf.inserted = 0;
  }
  else
  {
    ECM_DEBUG_LOG("USB_host_change_function unknown event %d", event);
  }

  return UX_SUCCESS;
}


