/*******************************************************************************/
/*  This file is auto-generated by Azure RTOS GUIX Studio. Do not edit this    */
/*  file by hand. Modifications to this file should only be made by running    */
/*  the Azure RTOS GUIX Studio application and re-generating the application   */
/*  specification file(s). For more information please refer to the Azure RTOS */
/*  GUIX Studio User Guide, or visit our web site at azure.com/rtos            */
/*                                                                             */
/*  GUIX Studio Revision 6.2.1.4                                               */
/*  Date (dd.mm.yyyy): 17.10.2023   Time (hh:mm): 15:23                        */
/*******************************************************************************/


#ifndef _MC50_SPECIFICATIONS_H_
#define _MC50_SPECIFICATIONS_H_

#include "gx_api.h"

/* Determine if C++ compiler is being used, if so use standard C.              */
#ifdef __cplusplus
extern   "C" {
#endif

/* Define widget ids                                                           */



/* Define animation ids                                                        */

#define GX_NEXT_ANIMATION_ID 1


/* Define user event ids                                                       */

#define GX_NEXT_USER_EVENT_ID GX_FIRST_USER_EVENT


/* Declare properties structures for each utilized widget type                 */

typedef struct GX_STUDIO_WIDGET_STRUCT
{
   GX_CHAR *widget_name;
   USHORT  widget_type;
   USHORT  widget_id;
   #if defined(GX_WIDGET_USER_DATA)
   INT   user_data;
   #endif
   ULONG style;
   ULONG status;
   ULONG control_block_size;
   GX_RESOURCE_ID normal_fill_color_id;
   GX_RESOURCE_ID selected_fill_color_id;
   GX_RESOURCE_ID disabled_fill_color_id;
   UINT (*create_function) (GX_CONST struct GX_STUDIO_WIDGET_STRUCT *, GX_WIDGET *, GX_WIDGET *);
   void (*draw_function) (GX_WIDGET *);
   UINT (*event_function) (GX_WIDGET *, GX_EVENT *);
   GX_RECTANGLE size;
   GX_CONST struct GX_STUDIO_WIDGET_STRUCT *next_widget;
   GX_CONST struct GX_STUDIO_WIDGET_STRUCT *child_widget;
   ULONG control_block_offset;
   GX_CONST void *properties;
} GX_STUDIO_WIDGET;

typedef struct
{
    GX_CONST GX_STUDIO_WIDGET *widget_information;
    GX_WIDGET        *widget;
} GX_STUDIO_WIDGET_ENTRY;

typedef struct
{
    GX_RESOURCE_ID normal_pixelmap_id;
    GX_RESOURCE_ID selected_pixelmap_id;
} GX_ICON_PROPERTIES;

typedef struct
{
    GX_RESOURCE_ID string_id;
    GX_RESOURCE_ID font_id;
    GX_RESOURCE_ID normal_text_color_id;
    GX_RESOURCE_ID selected_text_color_id;
    GX_RESOURCE_ID disabled_text_color_id;
} GX_PROMPT_PROPERTIES;

typedef struct
{
    GX_RESOURCE_ID wallpaper_id;
} GX_WINDOW_PROPERTIES;

typedef struct
{
    GX_RESOURCE_ID string_id;
    GX_RESOURCE_ID font_id;
    GX_RESOURCE_ID normal_text_color_id;
    GX_RESOURCE_ID selected_text_color_id;
    GX_RESOURCE_ID disabled_text_color_id;
    GX_UBYTE       whitespace;
    GX_BYTE        line_space;
} GX_ML_TEXT_VIEW_PROPERTIES;


/* Declare top-level control blocks                                            */

typedef struct WINDOW_CAN_REM_CONTROL_CONTROL_BLOCK_STRUCT
{
    GX_WINDOW_MEMBERS_DECLARE
    GX_PROMPT window_CAN_rem_control_prmpt_close;
    GX_PROMPT window_CAN_rem_control_prmpt_stop;
    GX_PROMPT window_CAN_rem_control_lb_menu;
    GX_PROMPT window_CAN_rem_control_prmpt_open;
    GX_PROMPT window_CAN_rem_control_prmpt_freewheeling;
    GX_PROMPT window_CAN_rem_control_prmpt_return;
    GX_PROMPT window_CAN_rem_control_prmt_node_num;
} WINDOW_CAN_REM_CONTROL_CONTROL_BLOCK;

typedef struct WINDOW_SDCARD_STATE_CONTROL_BLOCK_STRUCT
{
    GX_WINDOW_MEMBERS_DECLARE
    GX_PROMPT window_sdcard_state_Val_SD_status;
    GX_PROMPT window_sdcard_state_Val_ManufID;
    GX_PROMPT window_sdcard_state_Val_OID;
    GX_PROMPT window_sdcard_state_Val_NAME;
    GX_PROMPT window_sdcard_state_Lb1;
    GX_PROMPT window_sdcard_state_Lb2;
    GX_PROMPT window_sdcard_state_Lb3;
    GX_PROMPT window_sdcard_state_Lb4;
    GX_PROMPT window_sdcard_state_Val_Rev;
    GX_PROMPT window_sdcard_state_Lb5;
    GX_PROMPT window_sdcard_state_Val_SN;
    GX_PROMPT window_sdcard_state_Lb6;
    GX_PROMPT window_sdcard_state_Val_Manuf_Data;
    GX_PROMPT window_sdcard_state_Lb7;
    GX_PROMPT window_sdcard_state_Val_Card_Capacity;
    GX_PROMPT window_sdcard_state_Lb8;
    GX_PROMPT window_sdcard_state_Val_FS_Capacity;
    GX_PROMPT window_sdcard_state_Val_Misc;
} WINDOW_SDCARD_STATE_CONTROL_BLOCK;

typedef struct WINDOW_NET_STATE_CONTROL_BLOCK_STRUCT
{
    GX_WINDOW_MEMBERS_DECLARE
    GX_PROMPT window_net_state_Val_VID;
    GX_PROMPT window_net_state_lb_VID;
    GX_PROMPT window_net_state_lb_PID;
    GX_PROMPT window_net_state_Val_PID;
    GX_PROMPT window_net_state_Val_MAC;
    GX_PROMPT window_net_state_lb_MAC;
    GX_PROMPT window_net_state_Val_DHCP;
    GX_PROMPT window_net_state_Val_IP;
    GX_PROMPT window_net_state_Val_MASK;
    GX_PROMPT window_net_state_Val_GATE;
    GX_PROMPT window_net_state_lb_IP;
    GX_PROMPT window_net_state_lb_MASK;
    GX_PROMPT window_net_state_lb_GATE;
    GX_PROMPT window_net_state_Val_Connection;
    GX_PROMPT window_net_state_prmpt_Long_press;
} WINDOW_NET_STATE_CONTROL_BLOCK;

typedef struct WINDOW_DIAGNOSTIC_MENU_CONTROL_BLOCK_STRUCT
{
    GX_WINDOW_MEMBERS_DECLARE
    GX_PROMPT window_diagnostic_menu_prmpt_sdcard_state;
    GX_PROMPT window_diagnostic_menu_prmpt_communicationt_intf;
    GX_PROMPT window_diagnostic_menu_lb_menu;
    GX_PROMPT window_diagnostic_menu_prmpt_can_rem_ctrl;
    GX_PROMPT window_diagnostic_menu_prmpt_return;
    GX_PROMPT window_diagnostic_menu_Val_version;
} WINDOW_DIAGNOSTIC_MENU_CONTROL_BLOCK;

typedef struct WINDOW_CAN_ID_CONTROL_BLOCK_STRUCT
{
    GX_WINDOW_MEMBERS_DECLARE
    GX_PROMPT window_can_id_lb_caption;
    GX_PROMPT window_can_id_prmt_full_id;
    GX_PROMPT window_can_id_prmt_node_num;
    GX_PROMPT window_can_id_prmpt_Long_press;
    GX_PROMPT window_can_id_prmpt_Saved;
} WINDOW_CAN_ID_CONTROL_BLOCK;

typedef struct WINDOW_MENU_DC_CONTROL_BLOCK_STRUCT
{
    GX_WINDOW_MEMBERS_DECLARE
    GX_PROMPT window_menu_DC_prmpt_diagnostic;
    GX_PROMPT window_menu_DC_prmpt_reset;
    GX_PROMPT window_menu_DC_prmpt_can_id_edit;
    GX_PROMPT window_menu_DC_prmpt_return;
    GX_PROMPT window_menu_DC_lb_menu;
} WINDOW_MENU_DC_CONTROL_BLOCK;

typedef struct WINDOW_MANUAL_DC_CTRL_CONTROL_BLOCK_STRUCT
{
    GX_WINDOW_MEMBERS_DECLARE
    GX_PROMPT window_manual_DC_ctrl_prmpt_rot_CCW;
    GX_PROMPT window_manual_DC_ctrl_prmpt_stop;
    GX_PROMPT window_manual_DC_ctrl_lb_menu;
    GX_PROMPT window_manual_DC_ctrl_prmpt_rot_CW;
    GX_PROMPT window_manual_DC_ctrl_prmpt_freewheeling;
    GX_PROMPT window_manual_DC_ctrl_prmpt_return;
    GX_PROMPT window_manual_DC_ctrl_prmt_pwm_val;
    GX_PROMPT window_manual_DC_ctrl_lb_pwr;
    GX_PROMPT window_manual_DC_ctrl_lb_rpm;
    GX_PROMPT window_manual_DC_ctrl_lb_curr;
    GX_PROMPT window_manual_DC_ctrl_lb_rpm_1;
    GX_PROMPT window_manual_DC_ctrl_Val_pwr;
    GX_PROMPT window_manual_DC_ctrl_Val_moti;
    GX_PROMPT window_manual_DC_ctrl_Val_rpm;
    GX_PROMPT window_manual_DC_ctrl_Val_temp;
    GX_PROMPT window_manual_DC_ctrl_lb_pwm;
    GX_PROMPT window_manual_DC_ctrl_lb_perc;
} WINDOW_MANUAL_DC_CTRL_CONTROL_BLOCK;

typedef struct WINDOW_MAIN_DC_CONTROL_BLOCK_STRUCT
{
    GX_WINDOW_MEMBERS_DECLARE
    GX_PROMPT window_main_DC_Val_power_consumtion;
    GX_PROMPT window_main_DC_lb_power_consumption;
    GX_PROMPT window_main_DC_Val_pwr_src_voltage;
    GX_PROMPT window_main_DC_lb_pwr_src_voltage;
    GX_PROMPT window_main_DC_lb_pwr_src_current;
    GX_PROMPT window_main_DC_Val_pwr_src_current;
    GX_PROMPT window_main_DC_Val_node_number;
    GX_PROMPT window_main_DC_lb_node_number;
    GX_PROMPT window_main_DC_lb_A1;
    GX_PROMPT window_main_DC_lb_W;
    GX_PROMPT window_main_DC_lb_V;
    GX_PROMPT window_main_DC_lb_motor_current;
    GX_PROMPT window_main_DC_Val_motor_current;
    GX_PROMPT window_main_DC_lb_A2;
    GX_ICON window_main_DC_icon;
    GX_PROMPT window_main_DC_lb_rot_speed;
    GX_PROMPT window_main_DC_Val_mot_rpm;
    GX_PROMPT window_main_DC_lb_grad_s;
    GX_PROMPT window_main_DC_lb_errors;
    GX_PROMPT window_main_DC_Val_errors;
    GX_PROMPT window_main_DC_lb_speed;
    GX_PROMPT window_main_DC_Val_pcb_temper;
    GX_PROMPT window_main_DC_lb_driver_temp;
    GX_PROMPT window_main_DC_lb_grad_s_1;
    GX_PROMPT window_main_DC_Val_version;
} WINDOW_MAIN_DC_CONTROL_BLOCK;

typedef struct WINDOW_MAN_BLDC_CTRL_CONTROL_BLOCK_STRUCT
{
    GX_WINDOW_MEMBERS_DECLARE
    GX_PROMPT window_man_BLDC_ctrl_prmpt_freewheeling;
    GX_PROMPT window_man_BLDC_ctrl_prmpt_open;
    GX_PROMPT window_man_BLDC_ctrl_prmpt_close;
    GX_PROMPT window_man_BLDC_ctrl_prmpt_stop;
    GX_PROMPT window_man_BLDC_ctrl_lb_shaft_rot_speed;
    GX_PROMPT window_man_BLDC_ctrl_lb_shaft_position;
    GX_PROMPT window_man_BLDC_ctrl_Val_shaft_rot_speed;
    GX_PROMPT window_man_BLDC_ctrl_Val_shaft_position;
    GX_PROMPT window_man_BLDC_ctrl_lb_grad_s;
    GX_PROMPT window_man_BLDC_ctrl_lb_grad;
    GX_PROMPT window_man_BLDC_ctrl_prmpt_return;
    GX_PROMPT window_man_BLDC_ctrl_prmpt_ver;
} WINDOW_MAN_BLDC_CTRL_CONTROL_BLOCK;

typedef struct WINDOW_SPEED_EDIT_CONTROL_BLOCK_STRUCT
{
    GX_WINDOW_MEMBERS_DECLARE
    GX_PROMPT window_speed_edit_lb_caption;
    GX_PROMPT window_speed_edit_Val_rot_speed;
    GX_PROMPT window_speed_edit_prmpt_Long_press;
    GX_PROMPT window_speed_edit_prmpt_Saved;
    GX_PROMPT window_speed_edit_lb_degr_sec;
} WINDOW_SPEED_EDIT_CONTROL_BLOCK;

typedef struct WINDOW_ANGLE_CALIBR_CONTROL_BLOCK_STRUCT
{
    GX_WINDOW_MEMBERS_DECLARE
    GX_PROMPT window_angle_calibr_lb_Caption;
    GX_PROMPT window_angle_calibr_Val_angle_0;
    GX_PROMPT window_angle_calibr_prmpt_Saved;
    GX_PROMPT window_angle_calibr_lb_angle_0;
    GX_PROMPT window_angle_calibr_lb_angle_90;
    GX_PROMPT window_angle_calibr_Val_angle_90;
    GX_MULTI_LINE_TEXT_VIEW window_angle_calibr_text_prompt;
    GX_PROMPT window_angle_calibr_Val_current_value;
    GX_PROMPT window_angle_calibr_lb_current_val;
} WINDOW_ANGLE_CALIBR_CONTROL_BLOCK;

typedef struct WINDOW_MAIN_BLDC_CONTROL_BLOCK_STRUCT
{
    GX_WINDOW_MEMBERS_DECLARE
    GX_PROMPT window_main_BLDC_Val_power_consumtion;
    GX_PROMPT window_main_BLDC_lb_power_consumption;
    GX_PROMPT window_main_BLDC_Val_pwr_src_voltage;
    GX_PROMPT window_main_BLDC_lb_pwr_src_voltage;
    GX_PROMPT window_main_BLDC_lb_pwr_src_current;
    GX_PROMPT window_main_BLDC_Val_pwr_src_current;
    GX_PROMPT window_main_BLDC_Val_node_number;
    GX_PROMPT window_main_BLDC_lb_node_number;
    GX_PROMPT window_main_BLDC_lb_A1;
    GX_PROMPT window_main_BLDC_lb_W;
    GX_PROMPT window_main_BLDC_lb_V;
    GX_PROMPT window_main_BLDC_lb_motor_current;
    GX_PROMPT window_main_BLDC_Val_motor_current;
    GX_PROMPT window_main_BLDC_lb_A2;
    GX_ICON window_main_BLDC_icon;
    GX_PROMPT window_main_BLDC_lb_shaft_rot_speed;
    GX_PROMPT window_main_BLDC_Val_shaft_rot_speed;
    GX_PROMPT window_main_BLDC_lb_grad_s;
    GX_PROMPT window_main_BLDC_lb_shaft_position;
    GX_PROMPT window_main_BLDC_Val_shaft_position;
    GX_PROMPT window_main_BLDC_lb_grad;
    GX_PROMPT window_main_BLDC_lb_errors;
    GX_PROMPT window_main_BLDC_Val_errors;
    GX_PROMPT window_main_BLDC_lb_speed;
    GX_PROMPT window_main_BLDC_Val_speed;
    GX_PROMPT window_main_BLDC_Val_version;
} WINDOW_MAIN_BLDC_CONTROL_BLOCK;

typedef struct WINDOW_MENU_BLDC_CONTROL_BLOCK_STRUCT
{
    GX_WINDOW_MEMBERS_DECLARE
    GX_PROMPT window_menu_BLDC_prmpt_diagnostic;
    GX_PROMPT window_menu_BLDC_prmpt_reset;
    GX_PROMPT window_menu_BLDC_prmpt_can_id_edit;
    GX_PROMPT window_menu_BLDC_prmpt_return;
    GX_PROMPT window_menu_BLDC_lb_menu;
    GX_PROMPT window_menu_BLDC_prmpt_angle_calibration;
    GX_PROMPT window_menu_BLDC_prmpt_rotation_speed;
} WINDOW_MENU_BLDC_CONTROL_BLOCK;


/* extern statically defined control blocks                                    */

#ifndef GUIX_STUDIO_GENERATED_FILE
extern WINDOW_CAN_REM_CONTROL_CONTROL_BLOCK window_CAN_rem_control;
extern WINDOW_SDCARD_STATE_CONTROL_BLOCK window_sdcard_state;
extern WINDOW_NET_STATE_CONTROL_BLOCK window_net_state;
extern WINDOW_DIAGNOSTIC_MENU_CONTROL_BLOCK window_diagnostic_menu;
extern WINDOW_CAN_ID_CONTROL_BLOCK window_can_id;
extern WINDOW_MENU_DC_CONTROL_BLOCK window_menu_DC;
extern WINDOW_MANUAL_DC_CTRL_CONTROL_BLOCK window_manual_DC_ctrl;
extern WINDOW_MAIN_DC_CONTROL_BLOCK window_main_DC;
extern WINDOW_MAN_BLDC_CTRL_CONTROL_BLOCK window_man_BLDC_ctrl;
extern WINDOW_SPEED_EDIT_CONTROL_BLOCK window_speed_edit;
extern WINDOW_ANGLE_CALIBR_CONTROL_BLOCK window_angle_calibr;
extern WINDOW_MAIN_BLDC_CONTROL_BLOCK window_main_BLDC;
extern WINDOW_MENU_BLDC_CONTROL_BLOCK window_menu_BLDC;
#endif

/* Declare event process functions, draw functions, and callback functions     */

UINT CanRC_event_callback(GX_WINDOW *widget, GX_EVENT *event_ptr);
VOID CanRC_draw_callback(GX_WINDOW *widget);
UINT SDcard_state_event_callback(GX_WINDOW *widget, GX_EVENT *event_ptr);
VOID SDcard_state_draw_callback(GX_WINDOW *widget);
UINT Net_state_event_callback(GX_WINDOW *widget, GX_EVENT *event_ptr);
VOID Net_state_draw_callback(GX_WINDOW *widget);
UINT Diagnostic_menu_event_callback(GX_WINDOW *widget, GX_EVENT *event_ptr);
VOID Diagnostic_menu_draw_callback(GX_WINDOW *widget);
UINT Can_id_event_callback(GX_WINDOW *widget, GX_EVENT *event_ptr);
VOID Can_id_draw_callback(GX_WINDOW *widget);
UINT Menu_DC_event_callback(GX_WINDOW *widget, GX_EVENT *event_ptr);
VOID Menu_DC_draw_callback(GX_WINDOW *widget);
UINT Manual_DC_ctrl_event_callback(GX_WINDOW *widget, GX_EVENT *event_ptr);
VOID Manual_DC_ctrl_draw_callback(GX_WINDOW *widget);
UINT Main_DC_event_callback(GX_WINDOW *widget, GX_EVENT *event_ptr);
VOID Main_DC_draw_callback(GX_WINDOW *widget);
UINT Qmenu_event_callback(GX_WINDOW *widget, GX_EVENT *event_ptr);
VOID Qmenu_draw_callback(GX_WINDOW *widget);
UINT Speed_edit_event_callback(GX_WINDOW *widget, GX_EVENT *event_ptr);
VOID Speed_edit_draw_callback(GX_WINDOW *widget);
UINT Angle_calibr_event_callback(GX_WINDOW *widget, GX_EVENT *event_ptr);
VOID Angle_calibr_draw_callback(GX_WINDOW *widget);
UINT Main_event_callback(GX_WINDOW *widget, GX_EVENT *event_ptr);
VOID Main_draw_callback(GX_WINDOW *widget);
UINT Menu_BLDC_event_callback(GX_WINDOW *widget, GX_EVENT *event_ptr);
VOID Menu_BLDC_draw_callback(GX_WINDOW *widget);

/* Declare the GX_STUDIO_DISPLAY_INFO structure                                */


typedef struct GX_STUDIO_DISPLAY_INFO_STRUCT 
{
    GX_CONST GX_CHAR *name;
    GX_CONST GX_CHAR *canvas_name;
    GX_CONST GX_THEME **theme_table;
    GX_CONST GX_STRING **language_table;
    USHORT   theme_table_size;
    USHORT   language_table_size;
    UINT     string_table_size;
    UINT     x_resolution;
    UINT     y_resolution;
    GX_DISPLAY *display;
    GX_CANVAS  *canvas;
    GX_WINDOW_ROOT *root_window;
    GX_COLOR   *canvas_memory;
    ULONG      canvas_memory_size;
    USHORT     rotation_angle;
} GX_STUDIO_DISPLAY_INFO;


/* Declare Studio-generated functions for creating top-level widgets           */

UINT gx_studio_icon_create(GX_CONST GX_STUDIO_WIDGET *info, GX_WIDGET *control_block, GX_WIDGET *parent);
UINT gx_studio_prompt_create(GX_CONST GX_STUDIO_WIDGET *info, GX_WIDGET *control_block, GX_WIDGET *parent);
UINT gx_studio_window_create(GX_CONST GX_STUDIO_WIDGET *info, GX_WIDGET *control_block, GX_WIDGET *parent);
UINT gx_studio_multi_line_text_view_create(GX_CONST GX_STUDIO_WIDGET *info, GX_WIDGET *control_block, GX_WIDGET *parent);
GX_WIDGET *gx_studio_widget_create(GX_BYTE *storage, GX_CONST GX_STUDIO_WIDGET *definition, GX_WIDGET *parent);
UINT gx_studio_named_widget_create(char *name, GX_WIDGET *parent, GX_WIDGET **new_widget);
UINT gx_studio_display_configure(USHORT display, UINT (*driver)(GX_DISPLAY *), GX_UBYTE language, USHORT theme, GX_WINDOW_ROOT **return_root);

/* Determine if a C++ compiler is being used.  If so, complete the standard
  C conditional started above.                                                 */
#ifdef __cplusplus
}
#endif

#endif                                       /* sentry                         */
