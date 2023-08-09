#ifndef MC50_USB_STORAGE_H
  #define MC50_USB_STORAGE_H

extern UINT ux_device_msc_media_read(VOID *storage, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba, ULONG *media_status);
extern UINT ux_device_msc_media_write(VOID *storage, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba, ULONG *media_status);
extern UINT ux_device_msc_media_status(VOID *storage, ULONG lun, ULONG media_id, ULONG *media_status);




#endif



