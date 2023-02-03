#ifndef __LCD_H
#define __LCD_H

#include "stm32f10x.h"
#include <stdint.h>


//#define ST7735
#define SSD1306



#define ST7735_NOP 0x0
#define ST7735_SWRESET 0x01
#define ST7735_RDDID 0x04
#define ST7735_RDDST 0x09

#define ST7735_SLPIN  0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON  0x12
#define ST7735_NORON  0x13

#define ST7735_INVOFF 0x20
#define ST7735_INVON 0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON 0x29
#define ST7735_CASET 0x2A
#define ST7735_RASET 0x2B
#define ST7735_RAMWR 0x2C
#define ST7735_RAMRD 0x2E

#define ST7735_COLMOD 0x3A
#define ST7735_MADCTL 0x36


#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_RDID1 0xDA
#define ST7735_RDID2 0xDB
#define ST7735_RDID3 0xDC
#define ST7735_RDID4 0xDD

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

#define LCD_WIDTH	128
#define LCD_HEIGHT	160




#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2


/* Some colors definitions  */

#define color_green     (0x07E0)
#define color_blue       (0xF800)
//#define color_red      (0x001F)
#define color_red      (0x001F)
#define color_white      (0xFFFF)
#define color_black      (0x0000)
#define color_yellow      (0x0EFF)
#define color_grey      (0x1863)

#define BLACK 0
#define WHITE 1
#define YELLOW 1
#define BLUE 1




extern void lcd_init ( void );
extern void lcd_delay_pool( uint16_t time2delay );


/* LCD function prototype fot the ST7735 */
extern void lcd_backligth_on( void);
extern void lcd_backlight_off( void );
extern uint8_t lcd_spi_send( uint8_t txByte );
extern void lcd_send_commnad(  uint8_t txCommand );
extern void lcd_send_data( uint8_t txData );
extern void lcd_setAddrWindow( uint8_t x, uint8_t y, uint8_t x1, uint8_t y1 );


/* LCD function prototype fot the SSD1306 */
extern void i2c_command( uint8_t command );
extern void i2c_data( uint8_t data );
extern void display(void);

/* Graphic function */
extern void lcd_draw_string( uint16_t x, uint16_t y, char *c, uint16_t color, uint8_t size ) ;
extern void lcd_draw_char( uint16_t x, uint16_t y,  char c, uint16_t color, uint8_t size );
extern void lcd_draw_pixel( uint16_t x, uint16_t y,uint16_t color );
extern void lcd_draw_circle( int16_t x0, int16_t y0, int16_t r, uint16_t color );
extern void lcd_draw_fillrect( uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color ) ;
extern void lcd_draw_rect( uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color ) ;
extern void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
extern void set_background_color ( uint16_t color );


void rcc_lcd_info(void);
void rcc_usart_info(void);


#endif
