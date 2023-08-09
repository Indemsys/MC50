// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.08.06
// 9:30:25
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "ux_hcd_synergy.h"
#include   "ux_device_class_rndis.h"

extern void ux_dcd_initialize(void);

T_usb_app_info             g_uinf;


// Размеры даны для случая с закоментированнными макросами UX_HOST_SIDE_ONLY и UX_DEVICE_SIDE_ONLY
#define USBX_HOST_REGULAR_MEMORY_SIZE      (24000)// 12000 Минимальный размер определенный на основе статистики и тестирования при работе класса CDC ECM
                                                  // 24000 Минимальный размер определенный на основе статистики и тестирования при работе класса RNDIS
#define USBX_HOST_CACHE_SAFE_MEMORY_SIZE   (38000)// 32000 Минимальный размер определенный на основе статистики и тестирования при работе класса CDC ECM
                                                  // 38000 Минимальный размер определенный на основе статистики и тестирования при работе класса RNDIS

uint8_t       usb_mem_regular[USBX_HOST_REGULAR_MEMORY_SIZE] BSP_ALIGN_VARIABLE_V2(64);           // Область для кэшируемой динамической памяти
uint8_t       usb_mem_cache_safe[USBX_HOST_CACHE_SAFE_MEMORY_SIZE] BSP_ALIGN_VARIABLE_V2(64) BSP_PLACE_IN_SECTION_V2(".mempools");


static UX_SLAVE_CLASS_STORAGE_PARAMETER          storage_parms;
static UX_SLAVE_CLASS_CDC_ACM_PARAMETER          cdc0_parms;
static UX_SLAVE_CLASS_CDC_ACM_PARAMETER          cdc1_parms;


//static uint8_t *usbd_hs_framework;
static uint8_t *usbd_fs_framework;


SSP_VECTOR_DEFINE_CHAN(dmac_int_isr, DMAC, INT, 1);
SSP_VECTOR_DEFINE_CHAN(dmac_int_isr, DMAC, INT, 0);
//SSP_VECTOR_DEFINE_UNIT(usbhs_usb_int_resume_isr, USB, HS, USB_INT_RESUME, 0);
SSP_VECTOR_DEFINE_UNIT(usbfs_int_isr, USB, FS, INT, 0);


/*
void                      usbfs_int_isr (void);
static void              *gp_ctrl_USB_FS_0_INT;  // Переменная предназначенная для хранения указателя на структуру с управляющей информацией драйвера
const ssp_vector_t        g_vector_USB_FS_0_INT      __attribute__ ((section(".vector." "USB" "_" "FS" "_" "0" "_" "INT")))      __attribute__ ((__used__))=usbfs_int_isr;
const ssp_vector_info_t   g_vector_info_USB_FS_0_INT __attribute__ ((section(".vector_info." "USB" "_" "FS" "_" "0" "_" "INT"))) __attribute__ ((__used__))=
{
  . event_number = ELC_EVENT_USBFS_INT,
  . ip_id        =  SSP_IP_USB,           // Идентификатор из энумератора e_ssp_ip
  . ip_channel   = (0),
  . ip_unit      = SSP_IP_UNIT_USBFS,     // Идентификатор из энумератора e_ssp_ip_unit (там есть только USB)
  . ip_signal    = SSP_SIGNAL_USB_INT,    // Идентификатор из энумератора e_ssp_signal
  . pp_ctrl      = &gp_ctrl_USB_FS_0_INT  // Ссылка на объявленную ранее переменную содержащю указатель на управляющую информацию драйвера
};;
*/

static dmac_instance_ctrl_t usb_transfer_rx_ctrl;

static transfer_info_t usb_transfer_rx_info =
{
  .dest_addr_mode      = TRANSFER_ADDR_MODE_INCREMENTED,
  .repeat_area         = TRANSFER_REPEAT_AREA_DESTINATION,
  .irq                 = TRANSFER_IRQ_EACH,
  .chain_mode          = TRANSFER_CHAIN_MODE_DISABLED,
  .src_addr_mode       = TRANSFER_ADDR_MODE_FIXED,
  .size                = TRANSFER_SIZE_1_BYTE,
  .mode                = TRANSFER_MODE_BLOCK,
  .p_dest              = (void *)NULL,
  .p_src               = (void const *)NULL,
  .num_blocks          = 0,
  .length              = 0,
};

static const transfer_on_dmac_cfg_t usb_transfer_rx_extend =
{
  .channel     = 1,
  .offset_byte = 0,
};

extern const transfer_instance_t usb_transfer_rx;

static  transfer_cfg_t usb_transfer_rx_cfg =
{
  .p_info              =&usb_transfer_rx_info,
  .activation_source   = ELC_EVENT_USBHS_FIFO_1,
  .auto_enable         = false,
  .p_callback          = NULL,
  .p_context           =&usb_transfer_rx,
  .irq_ipl             =(0),
  .p_extend            =&usb_transfer_rx_extend,
};

const transfer_instance_t usb_transfer_rx =
{
  .p_ctrl        =&usb_transfer_rx_ctrl,
  .p_cfg         =&usb_transfer_rx_cfg,
  .p_api         =&g_transfer_on_dmac
};


static dmac_instance_ctrl_t usb_transfer_tx_ctrl;

static transfer_info_t usb_transfer_tx_info =
{
  .dest_addr_mode      = TRANSFER_ADDR_MODE_FIXED,
  .repeat_area         = TRANSFER_REPEAT_AREA_SOURCE,
  .irq                 = TRANSFER_IRQ_EACH,    // TRANSFER_IRQ_END, - изменение этого параметра ничего не меняет
  .chain_mode          = TRANSFER_CHAIN_MODE_DISABLED,
  .src_addr_mode       = TRANSFER_ADDR_MODE_INCREMENTED,
  .size                = TRANSFER_SIZE_1_BYTE,
  .mode                = TRANSFER_MODE_BLOCK,
  .p_dest              = (void *)NULL,
  .p_src               = (void const *)NULL,
  .num_blocks          = 0,
  .length              = 0,
};

static const transfer_on_dmac_cfg_t usb_transfer_tx_extend =
{
  .channel     = 0,
  .offset_byte = 0,
};

extern const transfer_instance_t usb_transfer_tx;

static  transfer_cfg_t usb_transfer_tx_cfg =
{
  .p_info              =&usb_transfer_tx_info,
  .activation_source   = ELC_EVENT_USBHS_FIFO_0,
  .auto_enable         = false,
  .p_callback          = NULL,
  .p_context           =&usb_transfer_tx,
  .irq_ipl             =(0),
  .p_extend            =&usb_transfer_tx_extend,
};

const transfer_instance_t usb_transfer_tx =
{
  .p_ctrl        =&usb_transfer_tx_ctrl,
  .p_cfg         =&usb_transfer_tx_cfg,
  .p_api         =&g_transfer_on_dmac
};

uint8_t usb_1_interface_type;
uint8_t usb_2_interface_type;


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t Get_usb_1_mode(void)
{
  return usb_1_interface_type;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t Get_usb_2_mode(void)
{
  return usb_2_interface_type;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Set_usb_mode(void)
{
  switch (wvar.usb_mode)
  {
  case USB_MODE_NONE                 :
    usb_1_interface_type = USB1_INTF_OFF;
    usb_2_interface_type = USB2_INTF_OFF;
    break;
  case USB_MODE_VCOM_PORT            :
    usb_1_interface_type = USB1_INTF_VIRTUAL_COM_PORT;
    usb_2_interface_type = USB2_INTF_OFF;
    break;
  case USB_MODE_MASS_STORAGE_        :
    usb_1_interface_type = USB1_INTF_MASS_STORAGE_DEVICE;
    usb_2_interface_type = USB2_INTF_OFF;
    break;
  case USB_MODE_VCOM_AND_MASS_STORAGE:
    usb_1_interface_type = USB1_INTF_VIRTUAL_COM_PORT;
    usb_2_interface_type = USB2_INTF_MASS_STORAGE_DEVICE;
    break;
  case USB_MODE_VCOM_AND_FREEMASTER_PORT   :
    usb_1_interface_type = USB1_INTF_VIRTUAL_COM_PORT;
    usb_2_interface_type = USB2_INTF_VIRTUAL_COM_PORT;
    break;
  case USB_MODE_RNDIS                :
    usb_1_interface_type = USB1_INTF_RNDIS_PORT;
    usb_2_interface_type = USB2_INTF_OFF;
    break;
  case USB_MODE_HOST_ECM:
    usb_1_interface_type = USB1_INTF_OFF;
    usb_2_interface_type = USB2_INTF_OFF;
    break;
  }
}

/*-----------------------------------------------------------------------------------------------------


  \param p_instance
  \param p_data
-----------------------------------------------------------------------------------------------------*/
static void _USB_err_callback(void *p_instance, void *p_data)
{

}


/*-----------------------------------------------------------------------------------------------------
  The definition of wrapper interface for USBX Synergy Port DCD Driver to get a transfer module instance

  \param dcd_io

  \return UINT
-----------------------------------------------------------------------------------------------------*/
static UINT _USB_initialize_dcd_transfer_support(ULONG dcd_io)
{
  UX_DCD_SYNERGY_TRANSFER dcd_transfer;

  // Здесь если установит присвоить &usb_transfer_tx; и &usb_transfer_rx; то VCOM и Mass Storage будут работать, а RNDIS работать не будет

  dcd_transfer.ux_synergy_transfer_tx = NULL; //(transfer_instance_t *)&usb_transfer_tx; //
  dcd_transfer.ux_synergy_transfer_rx = NULL; //(transfer_instance_t *)&usb_transfer_rx; //
  return (UINT)ux_dcd_synergy_initialize_transfer_support(dcd_io, (UX_DCD_SYNERGY_TRANSFER *)&dcd_transfer);
}


/*-----------------------------------------------------------------------------------------------------


  \param hcd

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT USB_initialize_hcd_transfer_support(UX_HCD *hcd)
{
  /* DMA support */
  UX_HCD_SYNERGY_TRANSFER hcd_transfer;
  hcd_transfer.ux_synergy_transfer_tx =  (transfer_instance_t *)&usb_transfer_tx; //
  hcd_transfer.ux_synergy_transfer_rx =  (transfer_instance_t *)&usb_transfer_rx; //
  return (UINT) ux_hcd_synergy_initialize_transfer_support(hcd, (UX_HCD_SYNERGY_TRANSFER *)&hcd_transfer);
}


/*-----------------------------------------------------------------------------------------------------



  \return ULONG
-----------------------------------------------------------------------------------------------------*/
static ULONG _Get_string_descriptor_size(void)
{
  ULONG  size = 0;
  UCHAR   *ptr = (UCHAR *)usb_strings;
  if (NULL != ptr)
  {
    for (INT i = 0; i < 3; i++)
    {
      ptr = ptr + 3;      /* bLength at byte offset 3 */
      /* Counts bLength + Language code(2bytes) + bLength(1byte) */
      size = size +*ptr + 4;
      ptr = ptr +(*ptr)+ 1;
    }
  }
  return size;
}

/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t _USB_get_hs_descr_sz(void)
{
  uint32_t sz = 0;

  sz += sizeof(device_hs_descriptor);
  sz += sizeof(config_hs_descriptor);

  switch (usb_1_interface_type)
  {
  case USB1_INTF_OFF:
    break;
  case USB1_INTF_VIRTUAL_COM_PORT:
    sz += sizeof(interface_cdc0_hs_descriptor);
    break;
  case USB1_INTF_MASS_STORAGE_DEVICE:
    sz += sizeof(interface_msd_hs_descriptor);
    break;
  case USB1_INTF_RNDIS_PORT:
    sz += sizeof(interface_rndis_hs_descriptor);
    break;
  }

  switch (usb_2_interface_type)
  {
  case USB2_INTF_OFF:
    break;
  case USB2_INTF_VIRTUAL_COM_PORT:
    sz += sizeof(interface_cdc1_hs_descriptor);
    break;
  case USB2_INTF_MASS_STORAGE_DEVICE:
    sz += sizeof(interface_msd_hs_descriptor);
    break;
  }
  return sz;
}

/*-----------------------------------------------------------------------------------------------------



  \return uint32_t
-----------------------------------------------------------------------------------------------------*/
uint32_t _USB_get_fs_descr_sz(void)
{
  uint32_t sz = 0;

  sz += sizeof(device_fs_descriptor);
  sz += sizeof(config_fs_descriptor);

  switch (usb_1_interface_type)
  {
  case USB1_INTF_OFF:
    break;
  case USB1_INTF_VIRTUAL_COM_PORT:
    sz += sizeof(interface_cdc0_fs_descriptor);
    break;
  case USB1_INTF_MASS_STORAGE_DEVICE:
    sz += sizeof(interface_msd_fs_descriptor);
    break;
  case USB1_INTF_RNDIS_PORT:
    sz += sizeof(interface_rndis_fs_descriptor);
    break;
  }


  switch (usb_2_interface_type)
  {
  case USB2_INTF_OFF:
    break;
  case USB2_INTF_VIRTUAL_COM_PORT:
    sz += sizeof(interface_cdc1_fs_descriptor);
    break;
  case USB2_INTF_MASS_STORAGE_DEVICE:
    sz += sizeof(interface_msd_fs_descriptor);
    break;
  }
  return sz;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void _USB_prepare_config_hs_descriptor(void)
{
  uint32_t  sz = 0;
  uint32_t  icnt = 0;


  switch (usb_1_interface_type)
  {
  case USB1_INTF_OFF:
    break;
  case USB1_INTF_VIRTUAL_COM_PORT:
    sz += sizeof(interface_cdc0_hs_descriptor);
    icnt += 2;
    break;
  case USB1_INTF_MASS_STORAGE_DEVICE:
    sz += sizeof(interface_msd_hs_descriptor);
    icnt += 1;
    break;
  case USB1_INTF_RNDIS_PORT:
    sz += sizeof(interface_rndis_hs_descriptor);
    icnt += 2;
    break;
  }

  switch (usb_2_interface_type)
  {
  case USB2_INTF_OFF:
    break;
  case USB2_INTF_VIRTUAL_COM_PORT:
    sz += sizeof(interface_cdc1_hs_descriptor);
    icnt += 2;
    break;
  case USB2_INTF_MASS_STORAGE_DEVICE:
    sz += sizeof(interface_msd_hs_descriptor);
    icnt += 1;
    break;
  }

  config_hs_descriptor.wTotalLength = sizeof(config_hs_descriptor)+ sz;
  config_hs_descriptor.bNumInterfaces = icnt;
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void _USB_prepare_config_fs_descriptor(void)
{
  uint32_t  sz = 0;
  uint32_t  icnt = 0;

  switch (usb_1_interface_type)
  {
  case USB1_INTF_OFF:
    break;
  case USB1_INTF_VIRTUAL_COM_PORT:
    sz += sizeof(interface_cdc0_fs_descriptor);
    icnt += 2;
    break;
  case USB1_INTF_MASS_STORAGE_DEVICE:
    sz += sizeof(interface_msd_fs_descriptor);
    icnt += 1;
    break;
  case USB1_INTF_RNDIS_PORT:
    sz += sizeof(interface_rndis_fs_descriptor);
    icnt += 2;
    break;
  }


  switch (usb_2_interface_type)
  {
  case USB2_INTF_OFF:
    break;
  case USB2_INTF_VIRTUAL_COM_PORT:
    sz += sizeof(interface_cdc1_fs_descriptor);
    icnt += 2;
    break;
  case USB2_INTF_MASS_STORAGE_DEVICE:
    sz += sizeof(interface_msd_fs_descriptor);
    icnt += 1;
    break;
  }

  config_fs_descriptor.wTotalLength = sizeof(config_fs_descriptor)+ sz;
  config_fs_descriptor.bNumInterfaces = icnt;
}


/*-----------------------------------------------------------------------------------------------------
   Initialization function that the user can choose to have called automatically during thread entry.
   The user can call this function at a later time if desired using the prototype below.

-----------------------------------------------------------------------------------------------------*/
static void _USB_dev_init(void)
{

  UINT status_ux_device_init;

  /** Calculate the size of USBX String Framework. */
  ULONG string_descr_sz;
  UCHAR *p_string_descr;

  {
    p_string_descr    = (UCHAR *)usb_strings;
    string_descr_sz = _Get_string_descriptor_size();
  }

  /** Calculate the size of USB Language Framework. */
  ULONG lang_descr_sz;
  UCHAR *p_lang_descr;

  p_lang_descr  = (UCHAR *)usb_lang_descr;
  lang_descr_sz = 2;

  /** Initialize the USB Device stack. */

  uint8_t *pmem;
  uint32_t hs_descr_sz = _USB_get_hs_descr_sz();
  uint32_t fs_descr_sz = _USB_get_fs_descr_sz();


//  if (wvar.en_usb_hs)
//  {
//    usbd_hs_framework = App_malloc_pending(hs_descr_sz, 10);
//    if (usbd_hs_framework != NULL)
//    {
//      pmem = usbd_hs_framework;
//
//      switch (usb_1_interface_type)
//      {
//      case USB1_INTF_VIRTUAL_COM_PORT:
//      case USB1_INTF_MASS_STORAGE_DEVICE:
//        memcpy(pmem, device_hs_descriptor, sizeof(device_hs_descriptor));
//        pmem = pmem + sizeof(device_hs_descriptor);
//        break;
//      case USB1_INTF_RNDIS_PORT:
//        memcpy(pmem, rndis_device_hs_descriptor, sizeof(rndis_device_hs_descriptor));
//        pmem = pmem + sizeof(rndis_device_hs_descriptor);
//        break;
//      }
//
//      _USB_prepare_config_hs_descriptor();
//      memcpy(pmem,&config_hs_descriptor, sizeof(config_hs_descriptor));
//      pmem = pmem + sizeof(config_hs_descriptor);
//
//
//      switch (usb_1_interface_type)
//      {
//      case USB1_INTF_OFF:
//        break;
//      case USB1_INTF_VIRTUAL_COM_PORT:
//        memcpy(pmem, interface_cdc0_hs_descriptor, sizeof(interface_cdc0_hs_descriptor));
//        pmem = pmem + sizeof(interface_cdc0_hs_descriptor);
//        break;
//      case USB1_INTF_MASS_STORAGE_DEVICE:
//        memcpy(pmem, interface_msd_hs_descriptor, sizeof(interface_msd_hs_descriptor));
//        pmem = pmem + sizeof(interface_msd_hs_descriptor);
//        break;
//      case USB1_INTF_RNDIS_PORT:
//        memcpy(pmem, interface_rndis_hs_descriptor, sizeof(interface_rndis_hs_descriptor));
//        pmem = pmem + sizeof(interface_rndis_hs_descriptor);
//        break;
//      }
//
//      switch (usb_2_interface_type)
//      {
//      case USB2_INTF_OFF:
//        break;
//      case USB2_INTF_VIRTUAL_COM_PORT:
//        memcpy(pmem, interface_cdc1_hs_descriptor, sizeof(interface_cdc1_hs_descriptor));
//        pmem = pmem + sizeof(interface_cdc1_hs_descriptor);
//        break;
//      case USB2_INTF_MASS_STORAGE_DEVICE:
//        memcpy(pmem, interface_msd_hs_descriptor, sizeof(interface_msd_hs_descriptor));
//        pmem = pmem + sizeof(interface_msd_hs_descriptor);
//        break;
//      }
//    }
//  }

  usbd_fs_framework = App_malloc_pending(fs_descr_sz, 10);
  if (usbd_fs_framework != NULL)
  {
    pmem = usbd_fs_framework;

    switch (usb_1_interface_type)
    {
    case USB1_INTF_VIRTUAL_COM_PORT:
    case USB1_INTF_MASS_STORAGE_DEVICE:
      memcpy(pmem, device_fs_descriptor, sizeof(device_fs_descriptor));
      pmem = pmem + sizeof(device_fs_descriptor);
      break;
    case USB1_INTF_RNDIS_PORT:
      memcpy(pmem, rndis_device_fs_descriptor, sizeof(rndis_device_fs_descriptor));
      pmem = pmem + sizeof(rndis_device_fs_descriptor);
      break;
    }

    _USB_prepare_config_fs_descriptor();
    memcpy(pmem,&config_fs_descriptor, sizeof(config_fs_descriptor));
    pmem = pmem + sizeof(config_fs_descriptor);

    switch (usb_1_interface_type)
    {
    case USB1_INTF_OFF:
      break;
    case USB1_INTF_VIRTUAL_COM_PORT:
      memcpy(pmem, interface_cdc0_fs_descriptor, sizeof(interface_cdc0_fs_descriptor));
      pmem = pmem + sizeof(interface_cdc0_fs_descriptor);
      break;
    case USB1_INTF_MASS_STORAGE_DEVICE:
      memcpy(pmem, interface_msd_fs_descriptor, sizeof(interface_msd_fs_descriptor));
      pmem = pmem + sizeof(interface_msd_fs_descriptor);
      break;
    case USB1_INTF_RNDIS_PORT:
      memcpy(pmem, interface_rndis_fs_descriptor, sizeof(interface_rndis_fs_descriptor));
      pmem = pmem + sizeof(interface_rndis_fs_descriptor);
      break;
    }

    switch (usb_2_interface_type)
    {
    case USB2_INTF_OFF:
      break;
    case USB2_INTF_VIRTUAL_COM_PORT:
      memcpy(pmem, interface_cdc1_fs_descriptor, sizeof(interface_cdc1_fs_descriptor));
      pmem = pmem + sizeof(interface_cdc1_fs_descriptor);
      break;
    case USB2_INTF_MASS_STORAGE_DEVICE:
      memcpy(pmem, interface_msd_fs_descriptor, sizeof(interface_msd_fs_descriptor));
      pmem = pmem + sizeof(interface_msd_fs_descriptor);
      break;
    }
  }



//  if (wvar.en_usb_hs)
//  {
//    status_ux_device_init = ux_device_stack_initialize((UCHAR *)usbd_hs_framework,hs_descr_sz,
//                           (UCHAR *)usbd_fs_framework,fs_descr_sz,
//                           p_string_descr, string_descr_sz,
//                           p_lang_descr, lang_descr_sz, UX_NULL);
//
//  }
//  else
//  {
    status_ux_device_init = ux_device_stack_initialize(NULL, 0,
                           (UCHAR *)usbd_fs_framework,fs_descr_sz,
                           p_string_descr, string_descr_sz,
                           p_lang_descr, lang_descr_sz, UX_NULL);

//  }

  if (UX_SUCCESS != status_ux_device_init)
  {
    _USB_err_callback(NULL,&status_ux_device_init);
  }

}


/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
static void _USB_storage_setup(void)
{
  UINT status;

  /* Auto setup for a simple media storage configuration with single Logical Unit Number (LUN). */
  /* Stores the number of LUN in this device storage instance.  */
  storage_parms.ux_slave_class_storage_parameter_number_lun = 1;
  storage_parms.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_last_lba       = fat_fs_media.fx_media_total_sectors;
  storage_parms.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_block_length   = fat_fs_media.fx_media_bytes_per_sector;
  storage_parms.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_type           = 0;
  storage_parms.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_removable_flag = 0x80;
  storage_parms.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_read           = ux_device_msc_media_read;
  storage_parms.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_write          = ux_device_msc_media_write;
  storage_parms.ux_slave_class_storage_parameter_lun[0].ux_slave_class_storage_media_status         = ux_device_msc_media_status;

  /* Register user callback functions.  */
  storage_parms.ux_slave_class_storage_instance_activate         = NULL;
  storage_parms.ux_slave_class_storage_instance_deactivate       = NULL;
  storage_parms.ux_slave_class_storage_parameter_vendor_id       = (UCHAR *)"NULL";
  storage_parms.ux_slave_class_storage_parameter_product_id      = (UCHAR *)"NULL";
  storage_parms.ux_slave_class_storage_parameter_product_rev     = (UCHAR *)"NULL";
  storage_parms.ux_slave_class_storage_parameter_product_serial  = (UCHAR *)"NULL";

  /* Initializes the device storage class. The class is connected with interface 0 on configuration 1. */
  status =  _ux_device_stack_class_register(_ux_system_slave_class_storage_name, ux_device_class_storage_entry, 1, 0x05, (VOID *)&storage_parms);
  assert(status == UX_SUCCESS);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _USB_cdc0_setup(void)
{
  cdc0_parms.ux_slave_class_cdc_acm_instance_activate   = ux_cdc_device0_instance_activate;
  cdc0_parms.ux_slave_class_cdc_acm_instance_deactivate = ux_cdc_device0_instance_deactivate;
  /* Initializes the device cdc class. */
  ux_device_stack_class_register(_ux_system_slave_class_cdc_acm_name, ux_device_class_cdc_acm_entry, 1, 0x00, (VOID *)&cdc0_parms);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void _USB_cdc1_setup(void)
{
  cdc1_parms.ux_slave_class_cdc_acm_instance_activate   = ux_cdc_device1_instance_activate;
  cdc1_parms.ux_slave_class_cdc_acm_instance_deactivate = ux_cdc_device1_instance_deactivate;
  /* Initializes the device cdc class. */
  ux_device_stack_class_register(_ux_system_slave_class_cdc_acm_name, ux_device_class_cdc_acm_entry, 1, 0x02, (VOID *)&cdc1_parms);
}


/*-----------------------------------------------------------------------------------------------------
  Initializes the USB device controller, enabling DMA transfer if transfer module instances are given.

-----------------------------------------------------------------------------------------------------*/
void ux_dcd_initialize(void)
{
  UINT status;

//  if (wvar.en_usb_hs)
//  {
//    usb_transfer_tx_cfg.activation_source   = ELC_EVENT_USBHS_FIFO_0;
//    usb_transfer_rx_cfg.activation_source  = ELC_EVENT_USBHS_FIFO_1;
//    status = _USB_initialize_transfer_support(R_USBHS_BASE);
//  }
//  else
//  {
    usb_transfer_tx_cfg.activation_source   = ELC_EVENT_USBFS_FIFO_0;
    usb_transfer_rx_cfg.activation_source  = ELC_EVENT_USBFS_FIFO_1;
    status = _USB_initialize_dcd_transfer_support(R_USBFS_BASE);
//  }

  if (UX_SUCCESS != status)
  {
    _USB_err_callback(NULL,&status);
  }
}


/*-----------------------------------------------------------------------------------------------------

  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_USB_stack(void)
{

  ux_system_initialize((CHAR *)usb_mem_regular, USBX_HOST_REGULAR_MEMORY_SIZE, usb_mem_cache_safe, USBX_HOST_CACHE_SAFE_MEMORY_SIZE);

  if (wvar.usb_mode == USB_MODE_HOST_ECM)
  {
     USB_host_cdc_ecm_init();
  }
  else
  {
    _USB_dev_init();

    switch (usb_1_interface_type)
    {
    case USB1_INTF_OFF:
      break;
    case USB1_INTF_VIRTUAL_COM_PORT:
      _USB_cdc0_setup();
      break;
    case USB1_INTF_MASS_STORAGE_DEVICE:
      _USB_storage_setup();
      break;
    case USB1_INTF_RNDIS_PORT:
      Register_rndis_class();
      break;
    }

    switch (usb_2_interface_type)
    {
    case USB2_INTF_OFF:
      break;
    case USB2_INTF_VIRTUAL_COM_PORT:
      _USB_cdc1_setup();
      break;
    case USB2_INTF_MASS_STORAGE_DEVICE:
      _USB_storage_setup();
      break;
    }

    ux_dcd_initialize();  // Инициализируем лрайвер USB девайса
  }
}

