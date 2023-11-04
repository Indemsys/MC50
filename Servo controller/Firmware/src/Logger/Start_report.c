// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-02-06
// 17:23:49
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Write_system_start_report(void)
{
  uint32_t err;

  fmi_unique_id_t uid;
  APPLOG("##############################################################");


  err = g_fmi.p_api->uniqueIdGet(&uid);
  if (err == SSP_SUCCESS)
  {
    APPLOG("Chip ID: %08X %08X %08X %08X", uid.unique_id[0], uid.unique_id[1], uid.unique_id[2], uid.unique_id[3]);
  }
  else
  {
    APPLOG("Failed to read Chip ID");
  }

  APPLOG("RSTSR0 = %02X, RSTSR1 = %04X, RSTSR3 = %02X",rstsr0,rstsr1,rstsr2);

  if (R_SYSTEM->RSTSR0_b.PORF)
  {
    APPLOG("Power-on reset detected");
    R_SYSTEM->RSTSR0_b.PORF = 0;
  }
  if (R_SYSTEM->RSTSR0_b.LVD0RF)
  {
    APPLOG("Voltage monitor 0 reset detected");
    R_SYSTEM->RSTSR0_b.LVD0RF = 0;
  }
  if (R_SYSTEM->RSTSR0_b.LVD1RF)
  {
    APPLOG("Voltage monitor 1 reset detected")
    R_SYSTEM->RSTSR0_b.LVD1RF = 0;
  }
  if (R_SYSTEM->RSTSR0_b.LVD2RF)
  {
    APPLOG("Voltage monitor 2 reset detected");
    R_SYSTEM->RSTSR0_b.LVD2RF = 0;
  }
  if (R_SYSTEM->RSTSR0_b.DPSRSTF)
  {
    APPLOG(" Deep Software Standby mode cancellation requested by an interrupt");
    R_SYSTEM->RSTSR0_b.DPSRSTF = 0;
  }
  if (R_SYSTEM->RSTSR1_b.IWDTRF)
  {
    APPLOG("Independent Watchdog Timer reset detected");
    R_SYSTEM->RSTSR1_b.IWDTRF = 0;
  }
  if (R_SYSTEM->RSTSR1_b.WDTRF)
  {
    APPLOG("Watchdog Timer reset detected");
    R_SYSTEM->RSTSR1_b.WDTRF = 0;
  }
  if (R_SYSTEM->RSTSR1_b.SWRF)
  {
    APPLOG("Software reset detected");
    R_SYSTEM->RSTSR1_b.SWRF = 0;
  }
  if (R_SYSTEM->RSTSR1_b.RPERF)
  {
    APPLOG("SRAM parity error reset detected");
    R_SYSTEM->RSTSR1_b.RPERF = 0;
  }
  if (R_SYSTEM->RSTSR1_b.REERF)
  {
    APPLOG("SRAM DED error reset detected");
    R_SYSTEM->RSTSR1_b.REERF = 0;
  }
  if (R_SYSTEM->RSTSR1_b.BUSSRF)
  {
    APPLOG("Bus slave MPU error reset detected");
    R_SYSTEM->RSTSR1_b.BUSSRF = 0;
  }
  if (R_SYSTEM->RSTSR1_b.BUSMRF)
  {
    APPLOG("Bus master MPU error reset detected");
    R_SYSTEM->RSTSR1_b.BUSMRF = 0;
  }
  if (R_SYSTEM->RSTSR1_b.SPERF)
  {
    APPLOG("SP error reset detected");
    R_SYSTEM->RSTSR1_b.SPERF = 0;
  }

  if (R_SYSTEM->RSTSR2_b.CWSF)
  {
    APPLOG("Warm start");
  }
  else
  {
    APPLOG("Cold start");
  }

  if (g_file_system_ready==0)
  {
    APPLOG("File system on SD card missing.");
  }
  else
  {
    APPLOG("File system on SD card ready.");
  }


  T_settings_restore_results* r = Get_Setting_restoring_res();

  if (r->dataflash_restoring_error != 0 )
  {
    APPLOG("Internal flash memory error.");
  }

  switch (r->settings_source)
  {
  case RESTORED_DEFAULT_SETTINGS:
     APPLOG("Restored default parameters.");
     break;
  case RESTORED_SETTINGS_FROM_DATAFLASH:
     APPLOG("Parameters restored from internal Flash memory.");
     break;
  case RESTORED_SETTINGS_FROM_JSON_FILE:
     APPLOG("Parameters restored from JSON file on SD card.");
     break;
  case RESTORED_SETTINGS_FROM_INI_FILE:
     APPLOG("Parameters restored from INI file on SD card.");
     break;
  }

  switch (r->dataflash_saving_error)
  {
  case SAVED_TO_DATAFLASH_NONE:
    break;
  case SAVED_TO_DATAFLASH_OK:
    APPLOG("Parameters saved to internal flash memory.");
    break;
  case SAVED_TO_DATAFLASH_ERROR:
    APPLOG("Parameters saving to internal flash memory error.");
    break;
  }

  APPLOG("Compile time    : %s",__DATE__" "__TIME__);

  rtc_time_t  curr_time;
  rtc_cbl.p_api->calendarTimeGet(rtc_cbl.p_ctrl,&curr_time);
  curr_time.tm_mon++; //
  APPLOG("Date Time: %04d.%02d.%02d  %02d:%02d:%02d", curr_time.tm_year+1900, curr_time.tm_mon, curr_time.tm_mday, curr_time.tm_hour, curr_time.tm_min, curr_time.tm_sec);
  if (rtc_init_res.RTC_status == ACTIVE_INTERNAL_RTC)
  {
    APPLOG("RTC valid. Internal MCU RTC is used.");
  }
  else if (rtc_init_res.RTC_status == ACTIVE_EXTERNAL_RTC)
  {
    APPLOG("RTC valid. External chip AB1815 is used.");
  }


  APPLOG("Application type = %d", wvar.app_type);


}

