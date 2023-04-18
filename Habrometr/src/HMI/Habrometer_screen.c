// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-16
// 11:43:37
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"
#include   "MC50_specifications.h"
#include   "MC50_resources.h"
#include   "Habr_Karma_thread.h"

GX_WINDOW              *habrometr_screen;


#define  ID_VAL_USER          0
#define  ID_VAL_VOICES        1
#define  ID_VAL_VIEWS         2
#define  ID_VAL_SUBSCRAIBERS  3
#define  ID_VAL_RATING        4
#define  ID_VAL_KARMA         5
#define  ID_VAL_DATE_TIME     6

GX_PROMPT* prmts[7] =
{
  {&window_habrometr.window_habrometr_Val_user         } ,
  {&window_habrometr.window_habrometr_Val_voices       } ,
  {&window_habrometr.window_habrometr_Val_views        } ,
  {&window_habrometr.window_habrometr_Val_subscraibers } ,
  {&window_habrometr.window_habrometr_Val_rating       } ,
  {&window_habrometr.window_habrometr_Val_karma        } ,
  {&window_habrometr.window_habrometr_Val_date_time    }
};


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void Init_habrometr_screen(void)
{
  if (habrometr_screen == 0)
  {
    gx_studio_named_widget_create("window_habrometr" , 0, (GX_WIDGET **)&habrometr_screen);
  }
  gx_widget_attach((GX_WIDGET *)root, (GX_WIDGET *)habrometr_screen);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static void _Show_habrometr(void)
{
  GUI_print_to_prompt(ID_VAL_USER          ,prmts,"%s"  , wvar.habr_user_name);
  GUI_print_to_prompt(ID_VAL_VOICES        ,prmts,"%.0f", (double)habr_results.number_of_votes);
  GUI_print_to_prompt(ID_VAL_VIEWS         ,prmts,"%.0f", (double)habr_results.number_of_views);
  GUI_print_to_prompt(ID_VAL_SUBSCRAIBERS  ,prmts,"%.0f", (double)habr_results.number_of_subscribers);
  GUI_print_to_prompt(ID_VAL_RATING        ,prmts,"%.1f", (double)habr_results.rating);
  GUI_print_to_prompt(ID_VAL_KARMA         ,prmts,"%.0f", (double)habr_results.karma);


  rtc_time_t  curr_time;
  rtc_cbl.p_api->calendarTimeGet(rtc_cbl.p_ctrl,&curr_time);
  curr_time.tm_mon++;
  GUI_print_to_prompt(ID_VAL_DATE_TIME     ,prmts,"%04d.%02d.%02d  %02d:%02d:%02d", curr_time.tm_year+1900, curr_time.tm_mon, curr_time.tm_mday, curr_time.tm_hour, curr_time.tm_min, curr_time.tm_sec);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
static uint32_t _Encoder_processing(void)
{
  if (Get_switch_press_signal())
  {
    gx_system_timer_stop((GX_WIDGET *)habrometr_screen, ENCODER_PROC_TIMER_ID);
    gx_widget_detach((GX_WIDGET *)habrometr_screen);
    Init_menu_screen();
    return 0;
  }
  return 1;
}

/*-----------------------------------------------------------------------------------------------------


  \param window
-----------------------------------------------------------------------------------------------------*/
VOID Habrometer_draw_callback(GX_WINDOW *window)
{
  gx_window_draw(window);
  gx_widget_children_draw(window);
}


/*-----------------------------------------------------------------------------------------------------


  \param window
  \param event_ptr

  \return UINT
-----------------------------------------------------------------------------------------------------*/
UINT Habrometer_event_callback(GX_WINDOW *window, GX_EVENT *event_ptr)
{
  UINT status;

  switch (event_ptr->gx_event_type)
  {
  case GX_EVENT_SHOW:
    _Show_habrometr();
    gx_system_timer_start((GX_WIDGET *)window, ENCODER_PROC_TIMER_ID, ENCODER_PROC_INTIT_TICKS, ENCODER_PROC_PERIOD_TICKS);
    status = gx_window_event_process(window, event_ptr);
    break;

  case GX_EVENT_TIMER:
    if (event_ptr->gx_event_payload.gx_event_timer_id == ENCODER_PROC_TIMER_ID)
    {
      if (_Encoder_processing()) _Show_habrometr();
    }
    break;

  default:
    status = gx_window_event_process(window, event_ptr);
    return status;
  }

  return 0;
}


