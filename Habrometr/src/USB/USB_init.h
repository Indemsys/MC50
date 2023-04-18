#ifndef MC50_USB_INIT_H
  #define MC50_USB_INIT_H


// Selector description:  USB1 interface mode
  #define USB1_INTF_OFF                             0
  #define USB1_INTF_VIRTUAL_COM_PORT                1
  #define USB1_INTF_MASS_STORAGE_DEVICE             2
  #define USB1_INTF_RNDIS_PORT                      3

// Selector description:  USB2 interface mode
  #define USB2_INTF_OFF                             0
  #define USB2_INTF_VIRTUAL_COM_PORT                1
  #define USB2_INTF_MASS_STORAGE_DEVICE             2

typedef struct
{
    uint8_t                inserted;
    ULONG                  idVendor;
    ULONG                  idProduct;
    ULONG                  dev_state;
    ULONG                  interface_id;
    ULONG                  interface_num;
    UX_HOST_CLASS_CDC_ECM *ecm_class_ptr;

} T_usb_app_info;


extern T_usb_app_info             g_uinf;

extern VOID ux_cdc_device1_instance_activate(VOID *cdc_instance);
extern VOID ux_cdc_device1_instance_deactivate(VOID *cdc_instance);
extern VOID ux_cdc_device0_instance_activate(VOID *cdc_instance);
extern VOID ux_cdc_device0_instance_deactivate(VOID *cdc_instance);



void    Init_USB_stack(void);
uint8_t Get_usb_1_mode(void);
uint8_t Get_usb_2_mode(void);
void    Set_usb_mode(void);
UINT    USB_initialize_hcd_transfer_support(UX_HCD *hcd);
#endif // USB_INIT_H



