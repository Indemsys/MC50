#ifndef TFT_DRIVER_H
  #define TFT_DRIVER_H


#define      LCD_X_SIZE 240
#define      LCD_Y_SIZE 240

#define      LCD_MAX_X  (LCD_X_SIZE-1)
#define      LCD_MAX_Y  (LCD_Y_SIZE-1)

#define      MAX_X_SZ   319
#define      MAX_Y_SZ   239

void         TFT_wr__cmd(uint8_t data);
void         TFT_wr_data(uint8_t data);
uint32_t     TFT_wr_data_buf(uint16_t *buf, uint32_t buf_sz);
void         TFT_Set_coordinates(uint32_t x, uint32_t y);
void         TFT_Set_rect(int x0, int y0, int x1, int y1);
void         TFT_Set_x(uint32_t x);
void         TFT_Set_y(uint32_t y);
void         TFT_set_display_stream(void);
void         TFT_clear(void);
void         TFT_display_on(void);
void         TFT_init(void);


#endif



