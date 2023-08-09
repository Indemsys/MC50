// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-16
// 10:49:33
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"

static GX_WINDOW  *sdcard_state_screen;


#define  ID_VAL_SD_STATUS         0
#define  ID_VAL_MANUFID           1
#define  ID_VAL_OID               2
#define  ID_VAL_NAME              3
#define  ID_VAL_REV               4
#define  ID_VAL_SN                5
#define  ID_VAL_MANUF_DATA        6
#define  ID_VAL_CARD_CAPACITY     7
#define  ID_VAL_FS_CAPACITY       8
#define  ID_VAL_MISC              9


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_sdcard_state_screen(void)
{
  if (sdcard_state_screen == 0)
  {
    gx_studio_named_widget_create("window_sdcard_state" , 0, (GX_WIDGET **)&sdcard_state_screen);
  }
  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)sdcard_state_screen);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Show_sdcard_state(void)
{
  sdmmc_priv_csd_reg_t *p_csd = Get_csd_reg();
  sdmmc_priv_cid_reg_t *p_cid = Get_cid_reg();

  uint32_t status = Get_FS_init_res()->fx_media_open_result;
  if (status != FX_SUCCESS)
  {
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_SD_status      ,GUI_print_str(ID_VAL_SD_STATUS    , "SD error"));
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_ManufID        ,GUI_print_str(ID_VAL_MANUFID      , ""));
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_OID            ,GUI_print_str(ID_VAL_OID          , ""));
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_NAME           ,GUI_print_str(ID_VAL_NAME         , ""));
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_Rev            ,GUI_print_str(ID_VAL_REV          , ""));
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_SN             ,GUI_print_str(ID_VAL_SN           , ""));
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_Manuf_Data     ,GUI_print_str(ID_VAL_MANUF_DATA   , ""));
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_Card_Capacity  ,GUI_print_str(ID_VAL_CARD_CAPACITY, ""));
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_FS_Capacity    ,GUI_print_str(ID_VAL_FS_CAPACITY  , ""));
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_Misc           ,GUI_print_str(ID_VAL_MISC         , ""));

    gx_prompt_text_color_set(&window_sdcard_state.window_sdcard_state_Val_SD_status,GX_COLOR_ID_RED, GX_COLOR_ID_RED, GX_COLOR_ID_RED);
  }
  else
  {
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_SD_status      ,GUI_print_str(ID_VAL_SD_STATUS    , "SD initialised"));

    // ManufID
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_ManufID      ,GUI_print_str(ID_VAL_MANUFID    , "%02X ", p_cid->cid.mfg_id));

    // OID
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_OID      ,GUI_print_str(ID_VAL_OID    , "%04X ",p_cid->cid.oem_id));


    // NAME
    char c1 = p_cid->cid.product_name1;
    char c2 = p_cid->cid.product_name2;
    char c3 = p_cid->cid.product_name3;
    char c4 = p_cid->cid.product_name4;
    char c5 = p_cid->cid.product_name5;
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_NAME      ,GUI_print_str(ID_VAL_NAME    , "%c%c%c%c%c ",c1, c2, c3, c4 , c5));

    // Rev
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_Rev      ,GUI_print_str(ID_VAL_REV    , "%d ",p_cid->cid.product_revision));

    // Serial Number
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_SN      ,GUI_print_str(ID_VAL_SN    ,  "%08X ",p_cid->cid.serial_number));

    // ManufactDate
    int32_t year  = 2000 +((p_cid->cid.mfg_date >> 4) & 0xFF);
    int32_t month = p_cid->cid.mfg_date & 0x0F;
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_Manuf_Data      ,GUI_print_str(ID_VAL_MANUF_DATA    ,  "%d.%d ",year , month));


    uint64_t sz = (uint64_t)sd_card_ctrl.status.sector_count *(uint64_t)sd_card_ctrl.status.sector_size;
    uint32_t s1 = sz % 1000ll;
    uint32_t s2 =(sz / 1000ll)% 1000;
    uint32_t s3 =(sz / 1000000ll)% 1000;
    uint32_t s4 =(sz / 1000000000ll);
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_Card_Capacity      ,GUI_print_str(ID_VAL_CARD_CAPACITY    ,  "%d %03d %03d %03d",s4, s3,s2, s1));


    sz = Get_media_total_size();
    s1 = sz % 1000ll;
    s2 =(sz / 1000ll)% 1000;
    s3 =(sz / 1000000ll)% 1000;
    s4 =(sz / 1000000000ll);
    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_FS_Capacity      ,GUI_print_str(ID_VAL_FS_CAPACITY    ,  "%d %03d %03d %03d",s4, s3,s2, s1));

    gx_prompt_text_set_ext(&window_sdcard_state.window_sdcard_state_Val_Misc           ,GUI_print_str(ID_VAL_MISC         , ""));
  }

}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Encoder_processing(void)
{
  if (Get_switch_press_signal())
  {
    gx_system_timer_stop((GX_WIDGET *)sdcard_state_screen, ENCODER_PROC_TIMER_ID);
    gx_widget_detach((GX_WIDGET*)sdcard_state_screen);
    Init_dmenu_screen();
    return 0;
  }
  return 1;
}

/*-----------------------------------------------------------------------------------------------------


  \param window
-----------------------------------------------------------------------------------------------------*/
VOID SDcard_state_draw_callback(GX_WINDOW *window)
{
  gx_window_draw(window);
  gx_widget_children_draw(window);
}

/*-----------------------------------------------------------------------------------------------------


  \param window
  \param event_ptr

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT SDcard_state_event_callback(GX_WINDOW *window, GX_EVENT *event_ptr)
{
  UINT status;

  switch (event_ptr->gx_event_type)
  {
  case GX_EVENT_SHOW:
    _Show_sdcard_state();
    gx_system_timer_start((GX_WIDGET *)window, ENCODER_PROC_TIMER_ID, ENCODER_PROC_INTIT_TICKS, ENCODER_PROC_PERIOD_TICKS);
    status = gx_window_event_process(window, event_ptr);
    break;

  case GX_EVENT_TIMER:
    if (event_ptr->gx_event_payload.gx_event_timer_id == ENCODER_PROC_TIMER_ID)
    {
      if (_Encoder_processing()) _Show_sdcard_state();
    }
    break;

  default:
    status = gx_window_event_process(window, event_ptr);
    return status;
  }

  return 0;
}

