#ifndef DMENU_SCREEN_H
  #define DMENU_SCREEN_H

void Init_dmenu_screen(void);
VOID Dmenu_draw_callback(GX_WINDOW *widget);
UINT Dmenu_event_callback(GX_WINDOW *widget, GX_EVENT *event_ptr);

#endif



