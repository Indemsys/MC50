// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2019.08.04
// 21:10:51
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"



UINT ux_device_msc_media_read(VOID *storage, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba, ULONG *media_status)
{
  ssp_err_t     res;
  //uint32_t i;


  res = sd_card_media_cbl.p_api->read(sd_card_media_cbl.p_ctrl, data_pointer , lba, number_blocks);
  if (res != SSP_SUCCESS)
  {
    *media_status = 0x00FFFF02; // UNKNOWN ERROR
    return UX_SUCCESS;
  }


  //  for (i=0; i < number_blocks; i++)
  //  {
  //    res = fx_media_read(&g_fx_media0, lba, data_pointer);
  //
  //    if (res != FX_SUCCESS)
  //    {
  //      return UX_ERROR;
  //    }
  //    lba++;
  //    data_pointer += g_fx_media0.fx_media_bytes_per_sector;
  //  }



  *media_status = 0x00;
  return UX_SUCCESS;
}

UINT ux_device_msc_media_write(VOID *storage, ULONG lun, UCHAR *data_pointer, ULONG number_blocks, ULONG lba, ULONG *media_status)
{
  UINT     res;
  //uint32_t i;

  if (lba == 0)
  {
    *media_status = 0x00002105; // LOGICAL BLOCK ADDRESS OUT OF RANGE
    return UX_SUCCESS;
  }
  res = sd_card_media_cbl.p_api->write(sd_card_media_cbl.p_ctrl, data_pointer , lba, number_blocks);
  if (res != SSP_SUCCESS)
  {
     *media_status = 0x00000303; // WRITE FAULT
    return UX_SUCCESS;
  }



  //  for (i=0; i < number_blocks; i++)
  //  {
  //    res = fx_media_write(&g_fx_media0, lba, data_pointer);
  //    if (res != FX_SUCCESS)
  //    {
  //      if (res == FX_SECTOR_INVALID)
  //      {
  //        *media_status = 0x00002105; // LOGICAL BLOCK ADDRESS OUT OF RANGE
  //        return UX_SUCCESS;
  //      }
  //      *media_status = 0x00000303; // WRITE FAULT
  //      return UX_ERROR;
  //    }
  //    lba++;
  //    data_pointer += g_fx_media0.fx_media_bytes_per_sector;
  //  }

  *media_status = 0x00;
  return UX_SUCCESS;
}


UINT ux_device_msc_media_status(VOID *storage, ULONG lun, ULONG media_id, ULONG *media_status)
{
  //APPLOG("MSD status request (%08X)", *media_status);
  *media_status = 0x00;
  return UX_SUCCESS;
}



