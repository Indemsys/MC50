#ifndef MC50_USB_DESCRIPTOR_H
  #define MC50_USB_DESCRIPTOR_H


  #define  DEVICE_VID1  0x045C
  #define  DEVICE_PID1  0x0009
  #define  DEVICE_BCD1  0x0001

  #define  DEVICE_VID2  0x045C
  #define  DEVICE_PID2  0x000A
  #define  DEVICE_BCD2  0x0001

  #define USBD_RNDIS_INTERFACE_INDEX                     0

  #pragma pack(1)

/*-----------------------------------------------------------------------------------------------------
  USB Device Descriptor for HS Mode
-----------------------------------------------------------------------------------------------------*/
static const unsigned char device_hs_descriptor[]
= {
  0x12,                           /* 0 bLength */
  UX_DEVICE_DESCRIPTOR_ITEM,      /* 1 bDescriptorType */
  0x00,                           /* 2 bcdUSB BCD(2.0) */
  0x02,                           /* 3 bcdUSB */
  0xEF,                           /* 4 bDeviceClass    : Device Class */
  0x02,                           /* 5 bDeviceSubClass : Common Class(0x02) */
  0x01,                           /* 6 bDeviceProtocol :Interface Association Descriptor(IAD) */
  0x40,                           /* 7 bMaxPacketSize0 */
  (uint8_t)(DEVICE_VID1),              /* 8 idVendor */  // Должен отличаться от аналогичного в дискрипторе RNDIS иначе при каждой смене RNDIS на VCOM и обратно будет необходимо переустановливать драйвер RNDIS
  (uint8_t)(DEVICE_VID1 >> 8),         /* 9 idVendor */
  (uint8_t)(DEVICE_PID1),              /* 10 idProduct */
  (uint8_t)(DEVICE_PID1 >> 8),         /* 11 idProduct */
  (uint8_t)(DEVICE_BCD1),              /* 12 bcdDevice */
  (uint8_t)(DEVICE_BCD1 >> 8),         /* 13 bcdDevice */
  1,                              /* 14 iManufacturer */
  2,                              /* 15 iProduct */
  3,                              /* 16 iSerialNumber */
  0x01,                           /* 17 bNumConfigurations */
  //.................................
  // Device qualifier descriptor
  //.................................
  0x0a, /* 0 bLength */
  UX_DEVICE_QUALIFIER_DESCRIPTOR_ITEM,      /* 1 bDescriptorType */
  0x00,                           /* 2 bcdUSB BCD(2.0) */
  0x02,                           /* 3 bcdUSB */
  0xEF,                           /* 4 bDeviceClass    : Device Class */
  0x02,                           /* 5 bDeviceSubClass : Common Class(0x02) */
  0x01,                           /* 6 bDeviceProtocol : none */
  0x40,                           /* 7 bMaxPacketSize0 */
  0x01,                           /* 8 bNumConfigs : 1 */
  0x00,                           /* 9 Reserve, fixed to 0 */
};


/*-----------------------------------------------------------------------------------------------------
  USB Device Descriptor for FS mode
-----------------------------------------------------------------------------------------------------*/
static const unsigned char device_fs_descriptor[]
= {
  0x12,                           /* 0 bLength */
  UX_DEVICE_DESCRIPTOR_ITEM,      /* 1 bDescriptorType */
  0x10,                           /* 2 bcdUSB BCD(2.0) */
  0x01,                           /* 3 bcdUSB */
  0xEF,                           /* 4 bDeviceClass    : Device Class */
  0x02,                           /* 5 bDeviceSubClass : Common Class(0x02) */
  0x01,                           /* 6 bDeviceProtocol :Interface Association Descriptor(IAD) */
  0x40,                           /* 7 bMaxPacketSize0 */
  (uint8_t)(DEVICE_VID1),              /* 8 idVendor */
  (uint8_t)(DEVICE_VID1 >> 8),         /* 9 idVendor */
  (uint8_t)(DEVICE_PID1),              /* 10 idProduct */
  (uint8_t)(DEVICE_PID1 >> 8),         /* 11 idProduct */
  (uint8_t)(DEVICE_BCD1),              /* 12 bcdDevice */
  (uint8_t)(DEVICE_BCD1 >> 8),         /* 13 bcdDevice */
  1,                              /* 14 iManufacturer */
  2,                              /* 15 iProduct */
  3,                              /* 16 iSerialNumber */
  0x01,                           /* 17 bNumConfigurations */
};

/*-----------------------------------------------------------------------------------------------------
  USB Device Descriptor for HS Mode
-----------------------------------------------------------------------------------------------------*/
static const unsigned char rndis_device_hs_descriptor[]
= {
  0x12,                           /* 0 bLength */
  UX_DEVICE_DESCRIPTOR_ITEM,      /* 1 bDescriptorType */
  0x00,                           /* 2 bcdUSB BCD(2.0) */
  0x02,                           /* 3 bcdUSB */
  0xEF,                           /* 4 bDeviceClass    : Device Class */
  0x02,                           /* 5 bDeviceSubClass : Common Class(0x02) */
  0x01,                           /* 6 bDeviceProtocol :Interface Association Descriptor(IAD) */
  0x40,                           /* 7 bMaxPacketSize0 */
  (uint8_t)(DEVICE_VID2),              /* 8 idVendor */
  (uint8_t)(DEVICE_VID2 >> 8),         /* 9 idVendor */
  (uint8_t)(DEVICE_PID2),              /* 10 idProduct */
  (uint8_t)(DEVICE_PID2 >> 8),         /* 11 idProduct */
  (uint8_t)(DEVICE_BCD2),              /* 12 bcdDevice */
  (uint8_t)(DEVICE_BCD2 >> 8),         /* 13 bcdDevice */
  1,                              /* 14 iManufacturer */
  2,                              /* 15 iProduct */
  3,                              /* 16 iSerialNumber */
  0x01,                           /* 17 bNumConfigurations */
  //.................................
  // Device qualifier descriptor
  //.................................
  0x0a, /* 0 bLength */
  UX_DEVICE_QUALIFIER_DESCRIPTOR_ITEM,      /* 1 bDescriptorType */
  0x10,                           /* 2 bcdUSB BCD(2.0) */
  0x01,                           /* 3 bcdUSB */
  0xEF,                           /* 4 bDeviceClass    : Device Class */
  0x02,                           /* 5 bDeviceSubClass : Common Class(0x02) */
  0x01,                           /* 6 bDeviceProtocol : none */
  0x40,                           /* 7 bMaxPacketSize0 */
  0x01,                           /* 8 bNumConfigs : 1 */
  0x00,                           /* 9 Reserve, fixed to 0 */
};

/*-----------------------------------------------------------------------------------------------------
  USB Device Descriptor for FS mode
-----------------------------------------------------------------------------------------------------*/
static const unsigned char rndis_device_fs_descriptor[]
= {
  0x12,                           /* 0 bLength */
  UX_DEVICE_DESCRIPTOR_ITEM,      /* 1 bDescriptorType */
  0x10,                           /* 2 bcdUSB BCD(2.0) */
  0x01,                           /* 3 bcdUSB */
  0xEF,                           /* 4 bDeviceClass    : Device Class */
  0x02,                           /* 5 bDeviceSubClass : Common Class(0x02) */
  0x01,                           /* 6 bDeviceProtocol :Interface Association Descriptor(IAD) */
  0x40,                           /* 7 bMaxPacketSize0 */
  (uint8_t)(DEVICE_VID2),              /* 8 idVendor */
  (uint8_t)(DEVICE_VID2 >> 8),         /* 9 idVendor */
  (uint8_t)(DEVICE_PID2),              /* 10 idProduct */
  (uint8_t)(DEVICE_PID2 >> 8),         /* 11 idProduct */
  (uint8_t)(DEVICE_BCD2),              /* 12 bcdDevice */
  (uint8_t)(DEVICE_BCD2 >> 8),         /* 13 bcdDevice */
  1,                              /* 14 iManufacturer */
  2,                              /* 15 iProduct */
  3,                              /* 16 iSerialNumber */
  0x01,                           /* 17 bNumConfigurations */
};


typedef struct
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint16_t wTotalLength;
    uint8_t  bNumInterfaces;
    uint8_t  bConfigurationValue;
    uint8_t  iConfiguration;
    uint8_t  bmAttributes;
    uint8_t  bMaxPower;

} T_USB_config_descriptor;

/*-----------------------------------------------------------------------------------------------------
  USB Configuration Descriptor for HS mode
-----------------------------------------------------------------------------------------------------*/
static T_USB_config_descriptor config_hs_descriptor
  = {
  0x09,                                            /* 0 bLength */
  UX_CONFIGURATION_DESCRIPTOR_ITEM,                /* 1 bDescriptorType */
  0,                                               /* 2 wTotalLength : This will be calculated at run-time. */
  0,                                               /* 4 bNumInterfaces */
  0x01,                                            /* 5 bConfigurationValue : Fixed to 1 since only one configuration is supported. */
  0x00,                                            /* 6 iConfiguration */
  0x80 | (0 << 6) | (0 << 5),                      /* 7 bmAttributes */
  250,                                             /* 8 bMaxPower */
};

/*-----------------------------------------------------------------------------------------------------
  USB Configuration Descriptor for FS mode
-----------------------------------------------------------------------------------------------------*/
static T_USB_config_descriptor config_fs_descriptor
  = {
  0x09,                                            /* 0 bLength */
  UX_CONFIGURATION_DESCRIPTOR_ITEM,                /* 1 bDescriptorType */
  0,                                               /* 2 wTotalLength : This will be calculated at run-time. */
  0,                                               /* 4 bNumInterfaces */
  0x01,                                            /* 5 bConfigurationValue : Fixed to 1 since only one configuration is supported. */
  0x00,                                            /* 6 iConfiguration */
  0x80 | (0 << 6) | (0 << 5),                      /* 7 bmAttributes */
  250,                                             /* 8 bMaxPower */
};


/*-----------------------------------------------------------------------------------------------------
  USB Mass Storage Class Interface Descriptor for HS mode g_usb_interface_descriptor_storage_0
-----------------------------------------------------------------------------------------------------*/
static const unsigned char interface_msd_hs_descriptor[]
= {
  //.................................
  // Mass Storage Class Interface Descriptor
  //.................................
  0x09, /* 0 bLength */
  UX_INTERFACE_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  0x05, /* 2 bInterfaceNumber */
  0x00, /* 3 bAlternateSetting  : Alternate for SetInterface Request */
  0x02, /* 4 bNumEndpoints      : 2 Endpoints for Interface#1 */
  0x08, /* 5 bInterfaceClass    : Mass Storage Class(0x8) */
  0x06, /* 6 bInterfaceSubClass : SCSI transparent command set(0x6) */
  0x50, /* 7 bInterfaceProtocol : BBB(0x50) */
  0x00, /* 8 iInterface Index */
  //.................................
  // Mass Storage Class Endpoint Descriptor (Bulk-Out)  for Interface#0x05
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM,  /* 1 bDescriptorType */
  (UX_ENDPOINT_OUT | 7), /* 2 bEndpointAddress */
  UX_BULK_ENDPOINT, /* 3 bmAttributes  */
  0x00, /* 4 wMaxPacketSize : 512bytes */
  0x02, /* 5 wMaxPacketSize */
  0x00, /* 6 bInterval */
  //.................................
  // Mass Storage Class Endpoint Descriptor (Bulk-In) for Interface#0x05
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  (UX_ENDPOINT_IN | 8), /* 2 bEndpointAddress */
  UX_BULK_ENDPOINT, /* 3 bmAttributes  */
  0x00, /* 4 wMaxPacketSize : 512bytes */
  0x02, /* 5 wMaxPacketSize */
  0x00, /* 6 bInterval */
};


/*-----------------------------------------------------------------------------------------------------
  USB Mass Storage Class Interface Descriptor  for FS mode g_usb_interface_descriptor_storage_0
-----------------------------------------------------------------------------------------------------*/
static const unsigned char interface_msd_fs_descriptor[]
= {
  //.................................
  // Mass Storage Class Interface Descriptor
  //.................................
  0x09, /* 0 bLength */
  UX_INTERFACE_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  0x05, /* 2 bInterfaceNumber */
  0x00, /* 3 bAlternateSetting  : Alternate for SetInterface Request */
  0x02, /* 4 bNumEndpoints      : 2 Endpoints for Interface#1 */
  0x08, /* 5 bInterfaceClass    : Mass Storage Class(0x8) */
  0x06, /* 6 bInterfaceSubClass : SCSI transparent command set(0x6) */
  0x50, /* 7 bInterfaceProtocol : BBB(0x50) */
  0x00, /* 8 iInterface Index */
  //.................................
  // Mass Storage Class Endpoint Descriptor (Bulk-Out)  for Interface#0x05
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM,  /* 1 bDescriptorType */
  (UX_ENDPOINT_OUT | 7), /* 2 bEndpointAddress */
  UX_BULK_ENDPOINT, /* 3 bmAttributes  */
  0x40, /* 4 wMaxPacketSize 64bytes */
  0x00, /* 5 wMaxPacketSize */
  0x00, /* 6 bInterval */
  //.................................
  // Mass Storage Class Endpoint Descriptor (Bulk-In) for Interface#0x05
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  (UX_ENDPOINT_IN | 8), /* 2 bEndpointAddress */
  UX_BULK_ENDPOINT, /* 3 bmAttributes  */
  0x40, /* 4 wMaxPacketSize 64bytes */
  0x00, /* 5 wMaxPacketSize */
  0x00, /* 6 bInterval */
};


/*-----------------------------------------------------------------------------------------------------
  USB CDC-ACM Interface Descriptor for HS mode g_usb_interface_desc_cdcacm_0
-----------------------------------------------------------------------------------------------------*/
static const unsigned char interface_cdc0_hs_descriptor[]
= {
  //.................................
  // Interface Association Descriptor(IAD)
  //.................................
  0x08, /* 0 bLength */
  UX_INTERFACE_ASSOCIATION_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  0x00, /* 2 bFirstInterface */
  0x02, /* 3 bInterfaceCount */
  0x02, /* 4 bFunctionClass : Communication */
  0x02, /* 5 bFunctionSubClass : Abstract Control Model */
  0x00, /* 6 bFunctionProtocol : Standard or enhanced AT Command set protocol */
  0x00, /* 7 iFunction : String descriptor index */
  //.................................
  // Communication Class Interface Descriptor
  //.................................
  0x09, /* 0 bLength */
  UX_INTERFACE_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  0x00, /* 2 bInterfaceNumber */
  0x00, /* 3 bAlternateSetting  : Alternate for SetInterface Request */
  0x01, /* 4 bNumEndpoints      : 1 Endpoint for Interface#1 */
  0x02, /* 5 bInterfaceClass    : Communications Interface Class(0x2) */
  0x02, /* 6 bInterfaceSubClass : Abstract Control Model(0x2) */
  0x01, /* 7 bInterfaceProtocol : Common AT command(0x01) */
  0x00, /* 8 iInterface Index */
  //.................................
  // Header Functional Descriptor
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x00, /* 2 bDescriptorSubtype : Header Functional Descriptor(0x0) */
  0x10, /* 3 bcdCDC Number  0x0110 == 1.10 */
  0x01, /* 4 bcdCDC */
  //.................................
  // Abstract Control Management Functional Functional Descriptor
  //.................................
  0x04, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x02, /* 2 bDescriptorSubtype : Abstract Control Management Functional Descriptor(0x2) */
  0x06, /* 3 bmCapabilities (Supports SendBreak, GetLineCoding, SetControlLineState, GetLineCoding) */
  //.................................
  // Union Functional Descriptor
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x06, /* 2 bDescriptorSubtype : Union Functional Descriptor(0x6) */
  0x00, /* 3 bMasterInterface */
  0x01, /* 4 bSubordinateInterface0 */
  //.................................
  // Call Management Functional Descriptor
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType */
  0x01, /* 2 bDescriptorSubtype : Call Management Functional Descriptor(0x1) */
  0x03, /* 3 bmCapabilities */
  0x01, /* 4 bDataInterface */
  //.................................
  // CDC-ACM Endpoint descriptor (Interrupt) for Interface#0
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  (UX_ENDPOINT_IN | 3), /* 2 bEndpointAddress */
  UX_INTERRUPT_ENDPOINT, /* 3 bmAttributes  */
  0x08, /* 4 wMaxPacketSize */
  0x00, /* 5 wMaxPacketSize */
  0x0F, /* 6 bInterval */
  //.................................
  // CDC-ACM Data Class Interface Descriptor
  //.................................
  0x09, /* 0 bLength */
  UX_INTERFACE_DESCRIPTOR_ITEM,  /* 1 bDescriptorType */
  0x01, /* 2 bInterfaceNumber */
  0x00, /* 3 bAlternateSetting  : Alternate for SetInterface Request */
  0x02, /* 4 bNumEndpoints      : 2 Endpoints for Interface#0 */
  0x0A, /* 5 bInterfaceClass    : Data Interface Class(0xA) */
  0x00, /* 6 bInterfaceSubClass : Abstract Control Model */
  0x00, /* 7 bInterfaceProtocol : No class specific protocol required */
  0x00, /* 8 iInterface Index   : String descriptor index */
  //.................................
  // CDC-ACM Endpoint Descriptor (Bulk-Out)  for Interface#1
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM,  /* 1 bDescriptorType */
  (UX_ENDPOINT_OUT | 1), /* 2 bEndpointAddress */
  UX_BULK_ENDPOINT, /* 3 bmAttributes  */
  0x00, /* 4 wMaxPacketSize */
  0x02, /* 5 wMaxPacketSize */
  0x00, /* 6 bInterval */
  //.................................
  // CDC-ACM Endpoint Descriptor (Bulk-In) for Interface#1
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  (UX_ENDPOINT_IN | 2), /* 2 bEndpointAddress */
  UX_BULK_ENDPOINT, /* 3 bmAttributes  */
  0x00, /* 4 wMaxPacketSize */
  0x02, /* 5 wMaxPacketSize */
  0x00, /* 6 bInterval */
};

/*-----------------------------------------------------------------------------------------------------
  USB CDC-ACM Interface Descriptor for FS mode g_usb_interface_desc_cdcacm_0
-----------------------------------------------------------------------------------------------------*/
static const unsigned char interface_cdc0_fs_descriptor[]
= {
  //.................................
  // Interface Association Descriptor(IAD)                        *
  //.................................
  0x08, /* 0 bLength */
  UX_INTERFACE_ASSOCIATION_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  0x00, /* 2 bFirstInterface */
  0x02, /* 3 bInterfaceCount */
  0x02, /* 4 bFunctionClass : Communication */
  0x02, /* 5 bFunctionSubClass : Abstract Control Model */
  0x00, /* 6 bFunctionProtocol : Standard or enhanced AT Command set protocol */
  0x00, /* 7 iFunction : String descriptor index */
  //.................................
  // Communication Class Interface Descriptor                     *
  //.................................
  0x09, /* 0 bLength */
  UX_INTERFACE_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  0x00, /* 2 bInterfaceNumber */
  0x00, /* 3 bAlternateSetting  : Alternate for SetInterface Request */
  0x01, /* 4 bNumEndpoints      : 1 Endpoint for Interface#1 */
  0x02, /* 5 bInterfaceClass    : Communications Interface Class(0x2) */
  0x02, /* 6 bInterfaceSubClass : Abstract Control Model(0x2) */
  0x01, /* 7 bInterfaceProtocol : Common AT command(0x01) */
  0x00, /* 8 iInterface Index */
  //.................................
  // Header Functional Descriptor                                 *
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x00, /* 2 bDescriptorSubtype : Header Functional Descriptor(0x0) */
  0x10, /* 3 bcdCDC Number  0x0110 == 1.10 */
  0x01, /* 4 bcdCDC */
  //.................................
  // Abstract Control Management Functional Functional Descriptor *
  //.................................
  0x04, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x02, /* 2 bDescriptorSubtype : Abstract Control Management Functional Descriptor(0x2) */
  0x06, /* 3 bmCapabilities (Supports SendBreak, GetLineCoding, SetControlLineState, GetLineCoding) */
  //.................................
  // Union Functional Descriptor                                  *
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x06, /* 2 bDescriptorSubtype : Union Functional Descriptor(0x6) */
  0x00, /* 3 bMasterInterface */
  0x01, /* 4 bSubordinateInterface0 */
  //.................................
  // Call Management Functional Descriptor                        *
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType */
  0x01, /* 2 bDescriptorSubtype : Call Management Functional Descriptor(0x1) */
  0x03, /* 3 bmCapabilities */
  0x01, /* 4 bDataInterface */
  //.................................
  // CDC-ACM Endpoint descriptor (Interrupt) for Interface#0      *
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  (UX_ENDPOINT_IN | 3), /* 2 bEndpointAddress */
  UX_INTERRUPT_ENDPOINT, /* 3 bmAttributes  */
  0x08, /* 4 wMaxPacketSize */
  0x00, /* 5 wMaxPacketSize */
  0x0F, /* 6 bInterval */
  //.................................
  // CDC-ACM Data Class Interface Descriptor                      *
  //.................................
  0x09, /* 0 bLength */
  UX_INTERFACE_DESCRIPTOR_ITEM,  /* 1 bDescriptorType */
  0x01, /* 2 bInterfaceNumber */
  0x00, /* 3 bAlternateSetting  : Alternate for SetInterface Request */
  0x02, /* 4 bNumEndpoints      : 2 Endpoints for Interface#0 */
  0x0A, /* 5 bInterfaceClass    : Data Interface Class(0xA) */
  0x00, /* 6 bInterfaceSubClass : Abstract Control Model */
  0x00, /* 7 bInterfaceProtocol : No class specific protocol required */
  0x00, /* 8 iInterface Index   : String descriptor index */
  //.................................
  // CDC-ACM Endpoint Descriptor (Bulk-Out)  for Interface#1      *
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM,  /* 1 bDescriptorType */
  (UX_ENDPOINT_OUT | 1), /* 2 bEndpointAddress */
  UX_BULK_ENDPOINT, /* 3 bmAttributes  */
  0x40, /* 4 wMaxPacketSize */
  0x00, /* 5 wMaxPacketSize */
  0x00, /* 6 bInterval */
  //.................................
  // CDC-ACM Endpoint Descriptor (Bulk-In) for Interface#1        *
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  (UX_ENDPOINT_IN | 2), /* 2 bEndpointAddress */
  UX_BULK_ENDPOINT, /* 3 bmAttributes  */
  0x40, /* 4 wMaxPacketSize */
  0x00, /* 5 wMaxPacketSize */
  0x00, /* 6 bInterval */
};


/*-----------------------------------------------------------------------------------------------------
  USB CDC-ACM Interface Descriptor for HS mode g_usb_interface_desc_cdcacm_1
-----------------------------------------------------------------------------------------------------*/
static const unsigned char interface_cdc1_hs_descriptor[]
= {
  //.................................
  // Interface Association Descriptor(IAD)
  //.................................
  0x08, /* 0 bLength */
  UX_INTERFACE_ASSOCIATION_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  0x02, /* 2 bFirstInterface */
  0x02, /* 3 bInterfaceCount */
  0x02, /* 4 bFunctionClass : Communication */
  0x02, /* 5 bFunctionSubClass : Abstract Control Model */
  0x00, /* 6 bFunctionProtocol : Standard or enhanced AT Command set protocol */
  0x00, /* 7 iFunction : String descriptor index */
  //.................................
  // Communication Class Interface Descriptor
  //.................................
  0x09, /* 0 bLength */
  UX_INTERFACE_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  0x02, /* 2 bInterfaceNumber */
  0x00, /* 3 bAlternateSetting  : Alternate for SetInterface Request */
  0x01, /* 4 bNumEndpoints      : 1 Endpoint for Interface#1 */
  0x02, /* 5 bInterfaceClass    : Communications Interface Class(0x2) */
  0x02, /* 6 bInterfaceSubClass : Abstract Control Model(0x2) */
  0x01, /* 7 bInterfaceProtocol : Common AT command(0x01) */
  0x00, /* 8 iInterface Index */
  //.................................
  // Header Functional Descriptor
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x00, /* 2 bDescriptorSubtype : Header Functional Descriptor(0x0) */
  0x10, /* 3 bcdCDC Number  0x0110 == 1.10 */
  0x01, /* 4 bcdCDC */
  //.................................
  // Abstract Control Management Functional Functional Descriptor
  //.................................
  0x04, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x02, /* 2 bDescriptorSubtype : Abstract Control Management Functional Descriptor(0x2) */
  0x06, /* 3 bmCapabilities (Supports SendBreak, GetLineCoding, SetControlLineState, GetLineCoding) */
  //.................................
  // Union Functional Descriptor
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x06, /* 2 bDescriptorSubtype : Union Functional Descriptor(0x6) */
  0x02, /* 3 bMasterInterface */
  0x03, /* 4 bSubordinateInterface0 */
  //.................................
  // Call Management Functional Descriptor
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType */
  0x01, /* 2 bDescriptorSubtype : Call Management Functional Descriptor(0x1) */
  0x03, /* 3 bmCapabilities */
  0x03, /* 4 bDataInterface */
  //.................................
  // CDC-ACM Endpoint descriptor (Interrupt) for Interface#0
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  (UX_ENDPOINT_IN | 6), /* 2 bEndpointAddress */
  UX_INTERRUPT_ENDPOINT, /* 3 bmAttributes  */
  0x08, /* 4 wMaxPacketSize */
  0x00, /* 5 wMaxPacketSize */
  0x0F, /* 6 bInterval */
  //.................................
  // CDC-ACM Data Class Interface Descriptor
  //.................................
  0x09, /* 0 bLength */
  UX_INTERFACE_DESCRIPTOR_ITEM,  /* 1 bDescriptorType */
  0x03, /* 2 bInterfaceNumber */
  0x00, /* 3 bAlternateSetting  : Alternate for SetInterface Request */
  0x02, /* 4 bNumEndpoints      : 2 Endpoints for Interface#0 */
  0x0A, /* 5 bInterfaceClass    : Data Interface Class(0xA) */
  0x00, /* 6 bInterfaceSubClass : Abstract Control Model */
  0x00, /* 7 bInterfaceProtocol : No class specific protocol required */
  0x00, /* 8 iInterface Index   : String descriptor index */
  //.................................
  // CDC-ACM Endpoint Descriptor (Bulk-Out)  for Interface#1
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM,  /* 1 bDescriptorType */
  (UX_ENDPOINT_OUT | 4), /* 2 bEndpointAddress */
  UX_BULK_ENDPOINT, /* 3 bmAttributes  */
  0x00, /* 4 wMaxPacketSize */
  0x02, /* 5 wMaxPacketSize */
  0x00, /* 6 bInterval */
  //.................................
  // CDC-ACM Endpoint Descriptor (Bulk-In) for Interface#1
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  (UX_ENDPOINT_IN | 5), /* 2 bEndpointAddress */
  UX_BULK_ENDPOINT, /* 3 bmAttributes  */
  0x00, /* 4 wMaxPacketSize */
  0x02, /* 5 wMaxPacketSize */
  0x00, /* 6 bInterval */
};

/*-----------------------------------------------------------------------------------------------------
  USB CDC-ACM Interface Descriptor for FS mode g_usb_interface_desc_cdcacm_1
-----------------------------------------------------------------------------------------------------*/
static const unsigned char interface_cdc1_fs_descriptor[]
= {
  //.................................
  // Interface Association Descriptor(IAD)
  //.................................
  0x08, /* 0 bLength */
  UX_INTERFACE_ASSOCIATION_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  0x02, /* 2 bFirstInterface */
  0x02, /* 3 bInterfaceCount */
  0x02, /* 4 bFunctionClass : Communication */
  0x02, /* 5 bFunctionSubClass : Abstract Control Model */
  0x00, /* 6 bFunctionProtocol : Standard or enhanced AT Command set protocol */
  0x00, /* 7 iFunction : String descriptor index */
  //.................................
  // Communication Class Interface Descriptor
  //.................................
  0x09, /* 0 bLength */
  UX_INTERFACE_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  0x02, /* 2 bInterfaceNumber */
  0x00, /* 3 bAlternateSetting  : Alternate for SetInterface Request */
  0x01, /* 4 bNumEndpoints      : 1 Endpoint for Interface#1 */
  0x02, /* 5 bInterfaceClass    : Communications Interface Class(0x2) */
  0x02, /* 6 bInterfaceSubClass : Abstract Control Model(0x2) */
  0x01, /* 7 bInterfaceProtocol : Common AT command(0x01) */
  0x00, /* 8 iInterface Index */
  //.................................
  // Header Functional Descriptor
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x00, /* 2 bDescriptorSubtype : Header Functional Descriptor(0x0) */
  0x10, /* 3 bcdCDC Number  0x0110 == 1.10 */
  0x01, /* 4 bcdCDC */
  //.................................
  // Abstract Control Management Functional Functional Descriptor *
  //.................................
  0x04, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x02, /* 2 bDescriptorSubtype : Abstract Control Management Functional Descriptor(0x2) */
  0x06, /* 3 bmCapabilities (Supports SendBreak, GetLineCoding, SetControlLineState, GetLineCoding) */
  //.................................
  // Union Functional Descriptor
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x06, /* 2 bDescriptorSubtype : Union Functional Descriptor(0x6) */
  0x02, /* 3 bMasterInterface */
  0x03, /* 4 bSubordinateInterface0 */
  //.................................
  // Call Management Functional Descriptor
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType */
  0x01, /* 2 bDescriptorSubtype : Call Management Functional Descriptor(0x1) */
  0x03, /* 3 bmCapabilities */
  0x03, /* 4 bDataInterface */
  //.................................
  // CDC-ACM Endpoint descriptor (Interrupt) for Interface#0
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  (UX_ENDPOINT_IN | 6), /* 2 bEndpointAddress */
  UX_INTERRUPT_ENDPOINT, /* 3 bmAttributes  */
  0x08, /* 4 wMaxPacketSize */
  0x00, /* 5 wMaxPacketSize */
  0x0F, /* 6 bInterval */
  //.................................
  // CDC-ACM Data Class Interface Descriptor
  //.................................
  0x09, /* 0 bLength */
  UX_INTERFACE_DESCRIPTOR_ITEM,  /* 1 bDescriptorType */
  0x03, /* 2 bInterfaceNumber */
  0x00, /* 3 bAlternateSetting  : Alternate for SetInterface Request */
  0x02, /* 4 bNumEndpoints      : 2 Endpoints for Interface#0 */
  0x0A, /* 5 bInterfaceClass    : Data Interface Class(0xA) */
  0x00, /* 6 bInterfaceSubClass : Abstract Control Model */
  0x00, /* 7 bInterfaceProtocol : No class specific protocol required */
  0x00, /* 8 iInterface Index   : String descriptor index */
  //.................................
  // CDC-ACM Endpoint Descriptor (Bulk-Out)  for Interface#1
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM,  /* 1 bDescriptorType */
  (UX_ENDPOINT_OUT | 4), /* 2 bEndpointAddress */
  UX_BULK_ENDPOINT, /* 3 bmAttributes  */
  0x40, /* 4 wMaxPacketSize */
  0x00, /* 5 wMaxPacketSize */
  0x00, /* 6 bInterval */
  //.................................
  // CDC-ACM Endpoint Descriptor (Bulk-In) for Interface#1
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  (UX_ENDPOINT_IN | 5), /* 2 bEndpointAddress */
  UX_BULK_ENDPOINT, /* 3 bmAttributes  */
  0x40, /* 4 wMaxPacketSize */
  0x00, /* 5 wMaxPacketSize */
  0x00, /* 6 bInterval */
};


/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static const unsigned char interface_rndis_hs_descriptor[]
= {
  //.................................
  // Interface Association Descriptor(IAD)
  //.................................
  0x08, /* 0 bLength */
  UX_INTERFACE_ASSOCIATION_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  0x00, /* 2 bFirstInterface */
  0x02, /* 3 bInterfaceCount */
  0xEF, /* 4 bFunctionClass : Communication */
  0x04, /* 5 bFunctionSubClass : Abstract Control Model */
  0x01, /* 6 bFunctionProtocol :  */
  0x00, /* 7 iFunction : String descriptor index */
  //.................................
  // Communication Class Interface Descriptor
  //.................................
  0x09, /* 0 bLength */
  UX_INTERFACE_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  0x00, /* 2 bInterfaceNumber */
  0x00, /* 3 bAlternateSetting  : Alternate for SetInterface Request */
  0x01, /* 4 bNumEndpoints      : 1 Endpoint for Interface#1 */
  0x02, /* 5 bInterfaceClass    : Communications Interface Class(0x2) */
  0x02, /* 6 bInterfaceSubClass : Abstract Control Model(0x2) */
  0x00, /* 7 bInterfaceProtocol :  */
  0x00, /* 8 iInterface Index */
  //.................................
  // Header Functional Descriptor
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x00, /* 2 bDescriptorSubtype : Header Functional Descriptor(0x0) */
  0x10, /* 3 bcdCDC Number  0x0110 == 1.10 */
  0x01, /* 4 bcdCDC */
  //.................................
  // Abstract Control Management Functional Functional Descriptor
  //.................................
  0x04, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x02, /* 2 bDescriptorSubtype : Abstract Control Management Functional Descriptor(0x2) */
  0x00, /* 3 bmCapabilities */
  //.................................
  // Union Functional Descriptor
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x06, /* 2 bDescriptorSubtype : Union Functional Descriptor(0x6) */
  0x00, /* 3 bMasterInterface */
  0x01, /* 4 bSubordinateInterface0 */
  //.................................
  // Call Management Functional Descriptor
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType */
  0x01, /* 2 bDescriptorSubtype : Call Management Functional Descriptor(0x1) */
  0x00, /* 3 bmCapabilities */
  0x01, /* 4 bDataInterface */
  //.................................
  // CDC-ACM Endpoint descriptor (Interrupt) for Interface#0
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  (UX_ENDPOINT_IN | 3), /* 2 bEndpointAddress */
  UX_INTERRUPT_ENDPOINT, /* 3 bmAttributes  */
  0x08, /* 4 wMaxPacketSize */
  0x00, /* 5 wMaxPacketSize */
  0x0F, /* 6 bInterval */
  //.................................
  // CDC-ACM Data Class Interface Descriptor
  //.................................
  0x09, /* 0 bLength */
  UX_INTERFACE_DESCRIPTOR_ITEM,  /* 1 bDescriptorType */
  0x01, /* 2 bInterfaceNumber */
  0x00, /* 3 bAlternateSetting  : Alternate for SetInterface Request */
  0x02, /* 4 bNumEndpoints      : 2 Endpoints for Interface#0 */
  0x0A, /* 5 bInterfaceClass    : Data Interface Class(0xA) */
  0x00, /* 6 bInterfaceSubClass : Abstract Control Model */
  0x00, /* 7 bInterfaceProtocol : No class specific protocol required */
  0x00, /* 8 iInterface Index   : String descriptor index */
  //.................................
  // CDC-ACM Endpoint Descriptor (Bulk-Out)  for Interface#1
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM,  /* 1 bDescriptorType */
  (UX_ENDPOINT_OUT | 1), /* 2 bEndpointAddress */
  UX_BULK_ENDPOINT, /* 3 bmAttributes  */
  0x00, /* 4 wMaxPacketSize */
  0x02, /* 5 wMaxPacketSize */
  0x00, /* 6 bInterval */
  //.................................
  // CDC-ACM Endpoint Descriptor (Bulk-In) for Interface#1
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  (UX_ENDPOINT_IN | 2), /* 2 bEndpointAddress */
  UX_BULK_ENDPOINT, /* 3 bmAttributes  */
  0x00, /* 4 wMaxPacketSize */
  0x02, /* 5 wMaxPacketSize */
  0x00, /* 6 bInterval */
};

/*-----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------------------------------------*/
static const unsigned char interface_rndis_fs_descriptor[]
= {
  //.................................
  // Interface Association Descriptor(IAD)
  //.................................
  0x08, /* 0 bLength */
  UX_INTERFACE_ASSOCIATION_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  0x00, /* 2 bFirstInterface */
  0x02, /* 3 bInterfaceCount */
  0xEF, /* 4 bFunctionClass : Communication */
  0x04, /* 5 bFunctionSubClass : Abstract Control Model */
  0x01, /* 6 bFunctionProtocol : Standard or enhanced AT Command set protocol */
  0x00, /* 7 iFunction : String descriptor index */
  //.................................
  // Communication Class Interface Descriptor
  //.................................
  0x09, /* 0 bLength */
  UX_INTERFACE_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  0x00, /* 2 bInterfaceNumber */
  0x00, /* 3 bAlternateSetting  : Alternate for SetInterface Request */
  0x01, /* 4 bNumEndpoints      : 1 Endpoint for Interface#1 */
  0x02, /* 5 bInterfaceClass    : Communications Interface Class(0x2) */
  0x02, /* 6 bInterfaceSubClass : Abstract Control Model(0x2) */
  0x00, /* 7 bInterfaceProtocol :  */
  0x00, /* 8 iInterface Index */
  //.................................
  // Header Functional Descriptor
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x00, /* 2 bDescriptorSubtype : Header Functional Descriptor(0x0) */
  0x10, /* 3 bcdCDC Number  0x0110 == 1.10 */
  0x01, /* 4 bcdCDC */
  //.................................
  // Abstract Control Management Functional Functional Descriptor
  //.................................
  0x04, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x02, /* 2 bDescriptorSubtype : Abstract Control Management Functional Descriptor(0x2) */
  0x00, /* 3 bmCapabilities */
  //.................................
  // Union Functional Descriptor
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType : CS_INTERFACE(24h) */
  0x06, /* 2 bDescriptorSubtype : Union Functional Descriptor(0x6) */
  0x00, /* 3 bMasterInterface */
  0x01, /* 4 bSubordinateInterface0 */
  //.................................
  // Call Management Functional Descriptor
  //.................................
  0x05, /* 0 bFunctionLength */
  0x24, /* 1 bDescriptorType */
  0x01, /* 2 bDescriptorSubtype : Call Management Functional Descriptor(0x1) */
  0x00, /* 3 bmCapabilities */
  0x01, /* 4 bDataInterface */
  //.................................
  // CDC-ACM Endpoint descriptor (Interrupt) for Interface#0
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  (UX_ENDPOINT_IN | 3), /* 2 bEndpointAddress */
  UX_INTERRUPT_ENDPOINT, /* 3 bmAttributes  */
  0x08, /* 4 wMaxPacketSize */
  0x00, /* 5 wMaxPacketSize */
  0x0F, /* 6 bInterval */
  //.................................
  // CDC-ACM Data Class Interface Descriptor
  //.................................
  0x09, /* 0 bLength */
  UX_INTERFACE_DESCRIPTOR_ITEM,  /* 1 bDescriptorType */
  0x01, /* 2 bInterfaceNumber */
  0x00, /* 3 bAlternateSetting  : Alternate for SetInterface Request */
  0x02, /* 4 bNumEndpoints      : 2 Endpoints for Interface#0 */
  0x0A, /* 5 bInterfaceClass    : Data Interface Class(0xA) */
  0x00, /* 6 bInterfaceSubClass : Abstract Control Model */
  0x00, /* 7 bInterfaceProtocol : No class specific protocol required */
  0x00, /* 8 iInterface Index   : String descriptor index */
  //.................................
  // CDC-ACM Endpoint Descriptor (Bulk-Out)  for Interface#1
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM,  /* 1 bDescriptorType */
  (UX_ENDPOINT_OUT | 1), /* 2 bEndpointAddress */
  UX_BULK_ENDPOINT, /* 3 bmAttributes  */
  0x40, /* 4 wMaxPacketSize */
  0x00, /* 5 wMaxPacketSize */
  0x00, /* 6 bInterval */
  //.................................
  // CDC-ACM Endpoint Descriptor (Bulk-In) for Interface#1
  //.................................
  0x07, /* 0 bLength */
  UX_ENDPOINT_DESCRIPTOR_ITEM, /* 1 bDescriptorType */
  (UX_ENDPOINT_IN | 2), /* 2 bEndpointAddress */
  UX_BULK_ENDPOINT, /* 3 bmAttributes  */
  0x40, /* 4 wMaxPacketSize */
  0x00, /* 5 wMaxPacketSize */
  0x00, /* 6 bInterval */
};

/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static volatile const UCHAR usb_lang_descr[] =
{
  (uint8_t)(0x0409),     /* Supported Language Code */
  (uint8_t)(0x0409 >> 8) /* US English as the default */
};


// Строки использумые дескриптором USB устройства
//  String Device Framework:
// Byte 0 and 1: Word containing the language ID: 0x0904 for US
// Byte 2 : Byte containing the index of the descriptor
// Byte 3 : Byte containing the length of the descriptor string
//
static UCHAR usb_strings[]=
{
  /* Manufacturer string descriptor: Index 1 */
  0x09,0x04,
  0x01,
  0x03,
  'I',
  'N',
  'D',

  /* Product string descriptor: Index 2 */
  // Выводится в окне устройств
  0x09,0x04,
  0x02,
  0x04,
  'M',
  'C',
  '5',
  '0',


  /* Serial Number string descriptor: Index 3 */
  0x09,0x04,
  0x03,
  0x03,
  '0',
  '0',
  '1'

};
  #pragma pack()


#endif
