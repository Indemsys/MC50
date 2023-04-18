// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// 2023-04-11
// 11:39:09
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include   "MC50.h"



static uint32_t                 X_OFFSET;
static uint32_t                 Y_OFFSET;

extern uint16_t                 video_buffer[];
/*-----------------------------------------------------------------------------------------------------


  \param data
-----------------------------------------------------------------------------------------------------*/
void TFT_wr__cmd(uint8_t data)
{
  LCD_DC = 0;
  SPI0_send_byte_to_display(data);
}

/*-----------------------------------------------------------------------------------------------------


  \param data
-----------------------------------------------------------------------------------------------------*/
void TFT_wr_data(uint8_t data)
{
  LCD_DC = 1;
  SPI0_send_byte_to_display(data);
}

/*-----------------------------------------------------------------------------------------------------


  \param data
-----------------------------------------------------------------------------------------------------*/
uint32_t TFT_wr_data_buf(uint16_t *buf, uint32_t buf_sz)
{
  uint32_t  res;

  LCD_DC = 1;
  res = SPI0_send_buff_to_display(buf, buf_sz);
  return res;
}

/*-----------------------------------------------------------------------------------------------------
  Тест прорисовывающий веритикальные линии на экране со сменяющимся цветом
  Предназанчен для отладки коммуникационного канала по SPI

-----------------------------------------------------------------------------------------------------*/
void TFT_test(void)
{
  uint16_t *p_videobuf;
  uint16_t *ptr;
  uint32_t pixels_num = 240 * 240;
  uint32_t vbuf_sz = pixels_num * 2;
  uint32_t n = 0;


  p_videobuf = App_malloc(vbuf_sz);
  if (p_videobuf != NULL)
  {
    do
    {

      ptr = p_videobuf;
      for (uint32_t i=0; i < pixels_num; i++)
      {
        if ((i % 8) == 0)
        {
          switch (n % 3)
          {
          case 0:
            *ptr = 0x07FF;
            break;
          case 1:
            *ptr = 0xF81F;
            break;
          case 2:
            *ptr = 0xFFE0;
            break;
          }
        }
        else
        {
          *ptr = 0xFFFF;
        }
        ptr++;
      }
      TFT_Set_rect(0,0,240,240);
      TFT_wr__cmd(0x002C);
      TFT_wr_data_buf(p_videobuf, vbuf_sz);
      Wait_ms(100);
      n++;
    } while (1);
  }
  App_free(p_videobuf);
}

/*-----------------------------------------------------------------------------------------------------
  Заполнение прямоугольной области дисплея фиксированным словом


  \param data
-----------------------------------------------------------------------------------------------------*/
static void TFT_fill_by_pixel(uint16_t w, uint32_t pixels_num)
{
  uint16_t *ptr;

  ptr = video_buffer;
  for (uint32_t i=0; i < pixels_num; i++)
  {
    *ptr =w;
    ptr++;
  }
}


/*-----------------------------------------------------------------------------------------------------


  \param x
  \param y
-----------------------------------------------------------------------------------------------------*/
void TFT_Set_coordinates(uint32_t x, uint32_t y)
{
  uint16_t dbuf[2];
  uint16_t xaddr;
  uint16_t yaddr;
  uint8_t *p = (uint8_t *)dbuf;

  if (x > LCD_MAX_X) x = LCD_MAX_X;
  if (y > LCD_MAX_Y) y = LCD_MAX_Y;

  xaddr = X_OFFSET + x;
  yaddr = Y_OFFSET + y;

  TFT_wr__cmd(0x2A);
  p[0] =(xaddr >> 8) & 0xFF;
  p[1] =  xaddr & 0xFF;
  p[2] =(MAX_X_SZ >> 8) & 0xFF;
  p[3] = (MAX_X_SZ )&0xFF;
  TFT_wr_data(p[0]);
  TFT_wr_data(p[1]);
  TFT_wr_data(p[2]);
  TFT_wr_data(p[3]);



  TFT_wr__cmd(0x2B);
  p[0] =(yaddr >> 8) & 0xFF;
  p[1] =  yaddr & 0xFF;
  p[2] =(MAX_Y_SZ >> 8) & 0xFF;
  p[3] = (MAX_Y_SZ)&0xFF;
  TFT_wr_data(p[0]);
  TFT_wr_data(p[1]);
  TFT_wr_data(p[2]);
  TFT_wr_data(p[3]);
}

/*-----------------------------------------------------------------------------------------------------


  \param x
  \param y
-----------------------------------------------------------------------------------------------------*/
void TFT_Set_rect(int x0, int y0, int x1, int y1)
{
  uint16_t dbuf[2];
  uint16_t xaddr0;
  uint16_t xaddr1;
  uint16_t yaddr0;
  uint16_t yaddr1;
  uint8_t *p = (uint8_t *)dbuf;

  if (x0 > LCD_MAX_X) x0 = LCD_MAX_X;
  if (y0 > LCD_MAX_Y) y0 = LCD_MAX_Y;

  if (x1 > LCD_MAX_X) x1 = LCD_MAX_X;
  if (y1 > LCD_MAX_Y) y1 = LCD_MAX_Y;

  xaddr0 = X_OFFSET + x0;
  xaddr1 = X_OFFSET + x1;

  yaddr0 = Y_OFFSET + y0;
  yaddr1 = Y_OFFSET + y1;

  TFT_wr__cmd(0x2A);
  p[0] =(xaddr0 >> 8) & 0xFF;
  p[1] = xaddr0 & 0xFF;
  p[2] =(xaddr1 >> 8) & 0xFF;
  p[3] = xaddr1 & 0xFF;
  TFT_wr_data(p[0]);
  TFT_wr_data(p[1]);
  TFT_wr_data(p[2]);
  TFT_wr_data(p[3]);

  TFT_wr__cmd(0x2B);
  p[0] =(yaddr0 >> 8) & 0xFF;
  p[1] = yaddr0 & 0xFF;
  p[2] =(yaddr1 >> 8) & 0xFF;
  p[3] = yaddr1 & 0xFF;
  TFT_wr_data(p[0]);
  TFT_wr_data(p[1]);
  TFT_wr_data(p[2]);
  TFT_wr_data(p[3]);
}


/*-----------------------------------------------------------------------------------------------------


  \param x
-----------------------------------------------------------------------------------------------------*/
void TFT_Set_x(uint32_t x)
{
  uint16_t dbuf[2];
  uint16_t xaddr;
  uint8_t *p = (uint8_t *)dbuf;

  if (x >= LCD_X_SIZE) x = LCD_X_SIZE-1;

  xaddr = X_OFFSET + x;

  TFT_wr__cmd(0x2A);
  p[0] =(xaddr >> 8) & 0xFF;
  p[1] = xaddr & 0xFF;
  p[2] =(MAX_X_SZ >> 8) & 0xFF;
  p[3] = (MAX_X_SZ )&0xFF;
  TFT_wr_data(p[0]);
  TFT_wr_data(p[1]);
  TFT_wr_data(p[2]);
  TFT_wr_data(p[3]);
}

/*-----------------------------------------------------------------------------------------------------


  \param x
-----------------------------------------------------------------------------------------------------*/
void TFT_Set_y(uint32_t y)
{
  uint16_t dbuf[2];
  uint16_t yaddr;
  uint8_t *p = (uint8_t *)dbuf;

  if (y >= LCD_Y_SIZE) y = LCD_Y_SIZE-1;

  yaddr = Y_OFFSET + y;

  TFT_wr__cmd(0x2B);
  p[0] =(yaddr >> 8) & 0xFF;
  p[1] = yaddr & 0xFF;
  p[2] =(MAX_Y_SZ >> 8) & 0xFF;
  p[3] = (MAX_Y_SZ)&0xFF;
  TFT_wr_data(p[0]);
  TFT_wr_data(p[1]);
  TFT_wr_data(p[2]);
  TFT_wr_data(p[3]);
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void TFT_display_on(void)
{
  TFT_wr__cmd(0x29); //display ON
}

/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void TFT_clear(void)
{
  uint16_t w = 0x0000;
  TFT_fill_by_pixel(w, LCD_X_SIZE * LCD_Y_SIZE);
  TFT_set_display_stream();
  TFT_wr_data_buf(video_buffer,LCD_X_SIZE * LCD_Y_SIZE*2);
}


/*-----------------------------------------------------------------------------------------------------


  \param void
-----------------------------------------------------------------------------------------------------*/
void TFT_set_display_stream(void)
{
  TFT_Set_rect(0, 0 , LCD_X_SIZE-1, LCD_Y_SIZE-1);
  TFT_wr__cmd(0x2C);
}


/*-----------------------------------------------------------------------------------------------------


  \param void

  \return uint8_t
-----------------------------------------------------------------------------------------------------*/
uint8_t Get_screen_orientation_word(void)
{
  uint8_t  w;

  switch (wvar.screen_orientation)
  {
  case 0:
    w = 0
       + LSHIFT(0, 7) // MY  - Page Address Order       | 0 = Top to Bottom, 1 = Bottom to Top
       + LSHIFT(0, 6) // MX  - Column Address Order     | 0 = Left to Right, 1 = Right to Left
       + LSHIFT(0, 5) // MV  - Page/Column Order        | 0 = Normal Mode, 1 = Reverse Mode
       + LSHIFT(0, 4) // ML  - Line Address Order       | 0 = LCD Refresh Top to Bottom, 1 = LCD Refresh Bottom to Top
       + LSHIFT(0, 3) // RGB - RGB/BGR Order            | 0 = RGB, 1 = BGR
       + LSHIFT(0, 2) // MH  - Display Data Latch Order | 0 = LCD Refresh Left to Right, 1 = LCD Refresh Right to Left
    ;
    X_OFFSET = 0;
    Y_OFFSET = 0;
    break;
  case 1:
    w = 0
       + LSHIFT(0, 7) // MY  - Page Address Order       | 0 = Top to Bottom, 1 = Bottom to Top
       + LSHIFT(1, 6) // MX  - Column Address Order     | 0 = Left to Right, 1 = Right to Left
       + LSHIFT(1, 5) // MV  - Page/Column Order        | 0 = Normal Mode, 1 = Reverse Mode
       + LSHIFT(0, 4) // ML  - Line Address Order       | 0 = LCD Refresh Top to Bottom, 1 = LCD Refresh Bottom to Top
       + LSHIFT(0, 3) // RGB - RGB/BGR Order            | 0 = RGB, 1 = BGR
       + LSHIFT(0, 2) // MH  - Display Data Latch Order | 0 = LCD Refresh Left to Right, 1 = LCD Refresh Right to Left
    ;
    X_OFFSET = 0;
    Y_OFFSET = 0;
    break;
  case 2:
    w = 0
       + LSHIFT(1, 7) // MY  - Page Address Order       | 0 = Top to Bottom, 1 = Bottom to Top   // 1 не работает 0 не работает
       + LSHIFT(1, 6) // MX  - Column Address Order     | 0 = Left to Right, 1 = Right to Left   // 1             1
       + LSHIFT(0, 5) // MV  - Page/Column Order        | 0 = Normal Mode, 1 = Reverse Mode
       + LSHIFT(0, 4) // ML  - Line Address Order       | 0 = LCD Refresh Top to Bottom, 1 = LCD Refresh Bottom to Top
       + LSHIFT(0, 3) // RGB - RGB/BGR Order            | 0 = RGB, 1 = BGR
       + LSHIFT(0, 2) // MH  - Display Data Latch Order | 0 = LCD Refresh Left to Right, 1 = LCD Refresh Right to Left
    ;
    X_OFFSET = 0;
    Y_OFFSET = 0x50;
    break;
  case 3:
    w = 0
       + LSHIFT(1, 7) // MY  - Page Address Order       | 0 = Top to Bottom, 1 = Bottom to Top
       + LSHIFT(0, 6) // MX  - Column Address Order     | 0 = Left to Right, 1 = Right to Left
       + LSHIFT(1, 5) // MV  - Page/Column Order        | 0 = Normal Mode, 1 = Reverse Mode
       + LSHIFT(0, 4) // ML  - Line Address Order       | 0 = LCD Refresh Top to Bottom, 1 = LCD Refresh Bottom to Top
       + LSHIFT(0, 3) // RGB - RGB/BGR Order            | 0 = RGB, 1 = BGR
       + LSHIFT(0, 2) // MH  - Display Data Latch Order | 0 = LCD Refresh Left to Right, 1 = LCD Refresh Right to Left
    ;
    X_OFFSET = 0x50;
    Y_OFFSET = 0;
    break;
  }
  return w;
}


/*-----------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------*/
void TFT_init(void)
{
  uint8_t  tmp;

  SPI0_init();
  LCD_BLK = 1;
  LCD_RST = 0;
  Wait_ms(10);
  LCD_RST = 1;

  Wait_ms(100);
  TFT_wr__cmd(0x11); //exit SLEEP mode

  Wait_ms(100);

  TFT_wr__cmd(0x36);

  //
  tmp =  Get_screen_orientation_word();


  TFT_wr_data(tmp); //MADCTL: memory data access control

  TFT_wr__cmd(0x3A);
  TFT_wr_data(0x55); //COLMOD: 16 bit/pixel

  TFT_wr__cmd(0xB2);
  TFT_wr_data(0x0C);
  TFT_wr_data(0x0C);
  TFT_wr_data(0x00);
  TFT_wr_data(0x33);
  TFT_wr_data(0x33); //PORCTRK: Porch setting

  TFT_wr__cmd(0xB7);
  TFT_wr_data(0x35); //GCTRL: Gate Control

  TFT_wr__cmd(0xBB);
  TFT_wr_data(0x2B); //VCOMS: VCOM setting

  TFT_wr__cmd(0xC0);
  TFT_wr_data(0x2C); //LCMCTRL: LCM Control

  TFT_wr__cmd(0xC2);
  TFT_wr_data(0x01);
  TFT_wr_data(0xFF); //VDVVRHEN: VDV and VRH Command Enable

  TFT_wr__cmd(0xC3);
  TFT_wr_data(0x11); //VRHS: VRH Set

  TFT_wr__cmd(0xC4);
  TFT_wr_data(0x20); //VDVS: VDV Set

  TFT_wr__cmd(0xC6);
  TFT_wr_data(0x0F); //FRCTRL2: Frame Rate control in normal mode

  TFT_wr__cmd(0xD0);
  TFT_wr_data(0xA4);
  TFT_wr_data(0xA1); //PWCTRL1: Power Control 1

  TFT_wr__cmd(0xE0);
  TFT_wr_data(0xD0);
  TFT_wr_data(0x00);
  TFT_wr_data(0x05);
  TFT_wr_data(0x0E);
  TFT_wr_data(0x15);
  TFT_wr_data(0x0D);
  TFT_wr_data(0x37);
  TFT_wr_data(0x43);
  TFT_wr_data(0x47);
  TFT_wr_data(0x09);
  TFT_wr_data(0x15);
  TFT_wr_data(0x12);
  TFT_wr_data(0x16);
  TFT_wr_data(0x19); //PVGAMCTRL: Positive Voltage Gamma control

  TFT_wr__cmd(0xE1);
  TFT_wr_data(0xD0);
  TFT_wr_data(0x00);
  TFT_wr_data(0x05);
  TFT_wr_data(0x0D);
  TFT_wr_data(0x0C);
  TFT_wr_data(0x06);
  TFT_wr_data(0x2D);
  TFT_wr_data(0x44);
  TFT_wr_data(0x40);
  TFT_wr_data(0x0E);
  TFT_wr_data(0x1C);
  TFT_wr_data(0x18);
  TFT_wr_data(0x16);
  TFT_wr_data(0x19); //NVGAMCTRL: Negative Voltage Gamma control

  TFT_wr__cmd(0x21); // Inverse

  Wait_ms(1);
}


