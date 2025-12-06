//LCD.h TFT彩屏
#ifndef __SPILCD_H
#define __SPILCD_H

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_spi.h"

/* LCD的宽和高定义 */
#define SPI_LCD_TYPE    0           /* SPI接口屏幕类型（1：2.4寸SPILCD  0：1.8寸ST7735） */  
#define USE_LCD_BUF     0           /* 默认使用lcd_buf,使用lcd_buf刷屏速度会大幅度提升,牺牲空间提高速度,内存不够时,置0即可 */

/* LCD的宽和高定义 */
extern uint16_t spilcd_width;   /* 屏幕的宽度 128(竖屏) */
extern uint16_t spilcd_height;  /* 屏幕的高度 1.8寸:160(竖屏)*/
extern uint8_t  spilcd_dir;  /* 默认横屏(1)、竖屏(0) */

/* LCD的画笔颜色和背景色 */
extern uint32_t g_point_color;    /* 画笔颜色 默认红色*/
extern uint32_t g_back_color;    /* 背景色 默认白色*/


/* 定义LCD启用的字体 */
#define FONT_12             1
#define FONT_16             1
#define FONT_24             1
#define FONT_32             1

/* 默认启用12号字体 */
#if ((FONT_12 == 0) && (FONT_16 == 0) && (FONT_24 == 0) && (FONT_32 == 0))
#undef FONT_12
#define FONT_12 1
#endif

/* LCD显示字体（正方形大小）枚举 */
typedef enum
{
#if (FONT_12 != 0)
  LCD_FONT_12,             /* 12号字体 6*12 */
#endif
#if (FONT_16 != 0)
  LCD_FONT_16,             /* 16号字体 8*16 */
#endif
#if (FONT_24 != 0)
  LCD_FONT_24,             /* 24号字体 12*24*/
#endif
#if (FONT_32 != 0)
  LCD_FONT_32,             /* 32号字体 16*32*/
#endif
} lcd_font_t;

/* LCD显示数字模式枚举 */
typedef enum
{
  NUM_SHOW_NOZERO = 0x00,  /* 数字高位0不显示 */
  NUM_SHOW_ZERO,           /* 数字高位0显示 */
} num_mode_t;


/******************************************************************************************/
/* LCD扫描方向和颜色 定义 */

/* 扫描方向定义 */
#define L2R_U2D         0           /* 从左到右,从上到下 */
#define L2R_D2U         1           /* 从左到右,从下到上 */
#define R2L_U2D         2           /* 从右到左,从上到下 */
#define R2L_D2U         3           /* 从右到左,从下到上 */

#define U2D_L2R         4           /* 从上到下,从左到右 */
#define U2D_R2L         5           /* 从上到下,从右到左 */
#define D2U_L2R         6           /* 从下到上,从左到右 */
#define D2U_R2L         7           /* 从下到上,从右到左 */

#define DFT_SCAN_DIR    R2L_D2U     /* 默认的扫描方向 */

/* 画笔颜色 */
#define WHITE               0xFFFF
#define BLACK               0x0000
#define BLUE                0x001F  
#define BRED                0XF81F
#define GRED                0XFFE0
#define GBLUE               0X07FF
#define RED                 0xF800
#define MAGENTA             0xF81F
#define GREEN               0x07E0
#define CYAN                0x7FFF
#define YELLOW              0xFFE0
#define BROWN               0XBC40      /* 棕色 */
#define BRRED               0XFC07      /* 棕红色 */
#define GRAY                0X8430      /* 灰色 */
#define DARKBLUE            0X01CF      /* 深蓝色 */
#define LIGHTBLUE           0X7D7C      /* 浅蓝色 */
#define GRAYBLUE            0X5458      /* 灰蓝色 */
/* 以上三色为PANEL的颜色  */
#define LIGHTGREEN          0X841F      /* 浅绿色 */
#define LGRAY               0XC618      /* 浅灰色(PANNEL),窗体背景色 */

#define LGRAYBLUE           0XA651      /* 浅灰蓝色(中间层颜色) */
#define LBBLUE              0X2B12      /* 浅棕蓝色(选择条目的反色) */


extern struct gpio_pin LCD_SCL_PIN;
extern struct gpio_pin LCD_SDA_PIN;
extern struct gpio_pin LCD_RES_PIN;
extern struct gpio_pin LCD_DC_PIN;
extern struct gpio_pin LCD_CS_PIN;
extern struct gpio_pin LCD_BLK_PIN;

//液晶控制口置1操作语句宏定义
#define	LCD_SCL_SET  	LCD_SCL_PIN.GPIOx->BSRR=LCD_SCL_PIN.pin
#define	LCD_SDA_SET  	LCD_SDA_PIN.GPIOx->BSRR=LCD_SDA_PIN.pin
#define	LCD_CS_SET  	LCD_CS_PIN.GPIOx->BSRR=LCD_CS_PIN.pin
  
#define	LCD_LED_SET  	LCD_BLK_PIN.GPIOx->BSRR=LCD_BLK_PIN.pin
#define	LCD_RS_SET  	LCD_DC_PIN.GPIOx->BSRR=LCD_DC_PIN.pin
#define	LCD_RST_SET  	LCD_RES_PIN.GPIOx->BSRR=LCD_RES_PIN.pin
//液晶控制口置0操作语句宏定义
#define	LCD_SCL_CLR  	LCD_SCL_PIN.GPIOx->BRR=LCD_SCL_PIN.pin
#define	LCD_SDA_CLR  	LCD_SDA_PIN.GPIOx->BRR=LCD_SDA_PIN.pin
#define	LCD_CS_CLR  	LCD_CS_PIN.GPIOx->BRR=LCD_CS_PIN.pin

#define	LCD_LED_CLR  	LCD_BLK_PIN.GPIOx->BRR=LCD_BLK_PIN.pin
#define	LCD_RS_CLR  	LCD_DC_PIN.GPIOx->BRR=LCD_DC_PIN.pin
#define	LCD_RST_CLR  	LCD_RES_PIN.GPIOx->BRR=LCD_RES_PIN.pin

#define LCD_DATAOUT(x) LCD_DATA->ODR=x; //数据输出
#define LCD_DATAIN     LCD_DATA->IDR;   //数据输入

#define LCD_WR_DATA(data){\
LCD_RS_SET;\
LCD_CS_CLR;\
LCD_DATAOUT(data);\
LCD_WR_CLR;\
LCD_WR_SET;\
LCD_CS_SET;\
} 

/* 函数声明 */
/* lcd驱动函数 */
void lcd_init(void);                                                                      /* lcd初始化函数 */
void lcd_clear(uint16_t color);                                                           /* lcd清屏函数 */                                                       
void lcd_fill(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color);        /* lcd区域填充 */
void lcd_display_dir(uint8_t dir);
void lcd_scan_dir(uint8_t dir);

/* lcd功能函数 */
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);                                  /* lcd画点 */
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);       /* lcd画线段 */
void lcd_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint16_t color);                    /* lcd画水平线 */
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);  /* lcd画矩形框 */
void lcd_draw_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color);                     /* lcd画圆形框 */
void lcd_fill_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color);                     /* 填充实心圆 */

void lcd_show_char(uint16_t x, uint16_t y, char ch, lcd_font_t font, uint8_t mode, uint16_t color); /* lcd显示1个字符 */
void lcd_show_string(uint16_t x, uint16_t y, lcd_font_t font,const char *str,  uint16_t color);     /* lcd显示字符串 */
void lcd_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len,                                /* lcd显示数字，可控制显示高位0 */
    lcd_font_t font, num_mode_t mode, uint16_t color);   
void lcd_show_hexNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len,                             /* lcd显示16进制数字，可控制显示高位0 */
    lcd_font_t font, num_mode_t mode, uint16_t color);                                                              
void lcd_show_pic(uint16_t x, uint16_t y, uint16_t width, uint16_t height,const uint8_t *pic);      /* lcd图片显示 */

void lcd_show_chinese(uint16_t x, uint16_t y, const char *chn);                               /* lcd显示中文 */

void lcd_printf(uint16_t Line, uint16_t Column, lcd_font_t font, const char *format, ...);

#endif


