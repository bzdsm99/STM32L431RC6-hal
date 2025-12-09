//LCD.c
#include "LCD.h"
#include "font.h"
#include "delay.h"
#include "stdio.h"
#include "OLED_Data.h"
#include "string.h"
#include "gpio.h"
// 已移除 stm32l4xx_hal.h 中的 SPI 相关依赖

/* LCD的宽和高定义 */
#if SPI_LCD_TYPE                    /* 2.4寸SPI_LCD屏幕 */
uint16_t spilcd_width  = 240;       /* 屏幕的宽度 240(竖屏) */
uint16_t spilcd_height = 320;       /* 屏幕的高度 320(竖屏) */
#else
uint16_t spilcd_width  = 128;       /* 屏幕的宽度 128(竖屏) */
uint16_t spilcd_height = 160;       /* 屏幕的高度 160(竖屏) */
#endif                              /* 1.8寸ST7735屏幕 */

uint8_t spilcd_dir = 0;             /* 默认横屏(1)、竖屏(0) */

#if USE_LCD_BUF
    #if SPI_LCD_TYPE
    uint16_t lcd_buf[320 * 240];    /* 存放一帧图像数据 */
    #else
    uint16_t lcd_buf[128 * 160];    /* 存放一帧图像数据 */
    #endif
#endif

/* LCD的画笔颜色和背景色 */
uint32_t g_point_color = BLACK;    /* 画笔颜色 */
uint32_t g_back_color  = WHITE;    /* 背景色 */

//课程PPT的lcd接线定义
// struct gpio_pin LCD_SCL_PIN = {GPIOB, GPIO_PIN_8};
// struct gpio_pin LCD_SDA_PIN = {GPIOB, GPIO_PIN_9};
// struct gpio_pin LCD_RES_PIN = {GPIOA, GPIO_PIN_5};
// struct gpio_pin LCD_DC_PIN  = {GPIOC, GPIO_PIN_2};
// struct gpio_pin LCD_CS_PIN  = {GPIOA, GPIO_PIN_0};
// struct gpio_pin LCD_BLK_PIN = {GPIOA, GPIO_PIN_4};

struct gpio_pin LCD_SCL_PIN = {GPIOA, GPIO_PIN_6};
struct gpio_pin LCD_SDA_PIN = {GPIOA, GPIO_PIN_12};
struct gpio_pin LCD_RES_PIN = {GPIOC, GPIO_PIN_9};
struct gpio_pin LCD_DC_PIN  = {GPIOC, GPIO_PIN_4};
struct gpio_pin LCD_CS_PIN  = {GPIOC, GPIO_PIN_5};
struct gpio_pin LCD_BLK_PIN = {GPIOA, GPIO_PIN_8};

//向SPI总线传输一个8位数据
static void SPI_WriteData(uint8_t Data)
{
	unsigned char i=0;
	for(i=8;i>0;i--)
	{
	    if(Data&0x80)	
        LCD_SDA_SET; //输出数据
        else LCD_SDA_CLR;

        LCD_SCL_CLR;       
        LCD_SCL_SET;
        Data<<=1; 
	}
}

/**
 * @brief       往LCD写命令
 * @param       cmd:命令
 * @retval      无
 */
static void lcd_write_cmd(uint8_t cmd)
{
    //SPI 写命令时序开始
    LCD_CS_CLR;
    LCD_RS_CLR;
    SPI_WriteData(cmd);
    LCD_CS_SET;
}

/**
 * @brief       往LCD写数据
 * @param       Data:数据
 * @retval      无
 */
static void lcd_write_data(uint8_t Data)
{
    LCD_CS_CLR;
    LCD_RS_SET;
    SPI_WriteData(Data);
    LCD_CS_SET;
}

//向液晶屏写一个16位数据
static void LCD_WriteData_16Bit(uint16_t Data)
{
    LCD_CS_CLR;
    LCD_RS_SET;
    SPI_WriteData(Data>>8); 	//写入高8位数据
    SPI_WriteData(Data); 		//写入低8位数据
    LCD_CS_SET; 
}

static void Lcd_Reset(void)
{
	LCD_RST_CLR;
	delay_ms(100);
	LCD_RST_SET;
	delay_ms(50);
}


/**
 * @brief       设置LCD行列地址
 * @param       xs: 列起始地址
 *              ys: 行起始地址
 *              xe: 列结束地址
 *              ye: 行结束地址
 * @note        适用于ST7735驱动芯片,1.8寸TFT屏幕,请根据驱动芯片手册进行修改
 * @retval      无
 */
static void lcd_set_address(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye) 
{
    lcd_write_cmd(0x2a);
	lcd_write_data(0x00);
	lcd_write_data(xs+2);
	lcd_write_data(0x00);
	lcd_write_data(xe+2);

	lcd_write_cmd(0x2b);
	lcd_write_data(0x00);
	lcd_write_data(ys+1);
	lcd_write_data(0x00);
	lcd_write_data(ye+2);

	lcd_write_cmd(0x2c);
}


/**
 * @brief       设置LCD的自动扫描方向
 * @note        一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
 * @param       dir:0~7,代表8个方向(具体定义见lcd.h)
 * @retval      无
 */
void lcd_scan_dir(uint8_t dir)
{
    uint16_t regval = 0;
    uint16_t temp;

    /* 根据扫描方式 设置 0x36 寄存器 bit 5,6,7 位的值 */
    switch (dir)
    {
        case L2R_U2D:/* 从左到右,从上到下 */
            regval |= (0 << 7) | (0 << 6) | (0 << 5);
            break;

        case L2R_D2U:/* 从左到右,从下到上 */
            regval |= (1 << 7) | (0 << 6) | (0 << 5);
            break;

        case R2L_U2D:/* 从右到左,从上到下 */
            regval |= (0 << 7) | (1 << 6) | (0 << 5);
            break;

        case R2L_D2U:/* 从右到左,从下到上 */
            regval |= (1 << 7) | (1 << 6) | (0 << 5);
            break;

        case U2D_L2R:/* 从上到下,从左到右 */
            regval |= (0 << 7) | (0 << 6) | (1 << 5);
            break;

        case U2D_R2L:/* 从上到下,从右到左 */
            regval |= (0 << 7) | (1 << 6) | (1 << 5);
            break;

        case D2U_L2R:/* 从下到上,从左到右 */
            regval |= (1 << 7) | (0 << 6) | (1 << 5);
            break;

        case D2U_R2L:/* 从下到上,从右到左 */
            regval |= (1 << 7) | (1 << 6) | (1 << 5);
            break;
    }

    LCD_CS_CLR;  
    lcd_write_cmd(0x36);
    lcd_write_data(regval);

    if (regval & 0x20)
    {
        if (spilcd_width < spilcd_height)   /* 交换X,Y */
        {
            temp = spilcd_width;
            spilcd_width = spilcd_height;
            spilcd_height = temp;
        }
    }
    else
    {
        if (spilcd_width > spilcd_height)   /* 交换X,Y */
        {
            temp = spilcd_width;
            spilcd_width = spilcd_height;
            spilcd_height = temp;
        }
    }

    lcd_set_address(0, 0, spilcd_width - 1, spilcd_height - 1);

    LCD_CS_SET;                    
}

/**
 * @brief       设置LCD显示方向
 * @param       dir:0,竖屏; 1,横屏
 * @retval      无
 */
void lcd_display_dir(uint8_t dir)
{
    spilcd_dir = dir;

    if (dir == 0)   /* 竖屏 */
    {
        spilcd_width = 128;
        spilcd_height = 160;   
    }
    else            /* 横屏 */
    {
        spilcd_width = 160; 
        spilcd_height = 128; 
    }

    lcd_scan_dir(DFT_SCAN_DIR); 
}

/**
 * @brief       清屏函数,可使用缓冲区
 * @param       color: 要清屏的颜色
 * @retval      无
 */
void lcd_clear(uint16_t color) 
{
    lcd_set_address(0,0,spilcd_width-1,spilcd_height-1);
    lcd_write_cmd(0x2C);
#if USE_LCD_BUF 
    for (uint32_t i = 0; i < spilcd_width * spilcd_height; i++) {
        lcd_buf[i] = color; // 直接写入颜色值
    }
#else
    uint8_t i,j;
    for(i=0;i<spilcd_width;i++)
    {
        for(j=0;j<spilcd_height;j++)
        {
            LCD_WriteData_16Bit(color); 
        }
    }
#endif
}


// 液晶IO初始化配置
void LCD_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    // 使能GPIO时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // 配置GPIOB的引脚
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;  // 推挽输出模式
    GPIO_InitStructure.Pull = GPIO_PULLDOWN;         // 无上下拉
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH; // 高速模式

    GPIO_InitStructure.Pin = LCD_SCL_PIN.pin;
    HAL_GPIO_Init(LCD_SCL_PIN.GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = LCD_SDA_PIN.pin;
    HAL_GPIO_Init(LCD_SDA_PIN.GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = LCD_RES_PIN.pin;
    HAL_GPIO_Init(LCD_RES_PIN.GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = LCD_DC_PIN.pin;
    HAL_GPIO_Init(LCD_DC_PIN.GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = LCD_CS_PIN.pin;
    HAL_GPIO_Init(LCD_CS_PIN.GPIOx, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = LCD_BLK_PIN.pin;
    HAL_GPIO_Init(LCD_BLK_PIN.GPIOx, &GPIO_InitStructure);

    // 设置背光引脚为高电平
    // HAL_GPIO_WritePin(LCD_BLK_PIN.GPIOx, LCD_BLK_PIN.pin, GPIO_PIN_SET);
}

/**
* @brief       初始化spilcd
* @param       无
* @retval      无
*/
void lcd_init(void)
{

    LCD_GPIO_Init();
    /* 硬件复位 */
    Lcd_Reset();

    /* 对LCD的寄存器进行配置 - ST7735 */
    lcd_write_cmd(0x11);        /* Sleep Out */
    HAL_Delay(120);             /* wait for power stability */

    //ST7735R Frame Rate
	lcd_write_cmd(0xB1); 
	lcd_write_data(0x01); 
	lcd_write_data(0x2C); 
	lcd_write_data(0x2D); 

	lcd_write_cmd(0xB2); 
	lcd_write_data(0x01); 
	lcd_write_data(0x2C); 
	lcd_write_data(0x2D); 

	lcd_write_cmd(0xB3); 
	lcd_write_data(0x01); 
	lcd_write_data(0x2C); 
	lcd_write_data(0x2D); 
	lcd_write_data(0x01); 
	lcd_write_data(0x2C); 
	lcd_write_data(0x2D); 
	
	lcd_write_cmd(0xB4); //Column inversion 
	lcd_write_data(0x07); 
	
	//ST7735R Power Sequence
	lcd_write_cmd(0xC0); 
	lcd_write_data(0xA2); 
	lcd_write_data(0x02); 
	lcd_write_data(0x84); 
	lcd_write_cmd(0xC1); 
	lcd_write_data(0xC5); 

	lcd_write_cmd(0xC2); 
	lcd_write_data(0x0A); 
	lcd_write_data(0x00); 

	lcd_write_cmd(0xC3); 
	lcd_write_data(0x8A); 
	lcd_write_data(0x2A); 
	lcd_write_cmd(0xC4); 
	lcd_write_data(0x8A); 
	lcd_write_data(0xEE); 
	
	lcd_write_cmd(0xC5); //VCOM 
	lcd_write_data(0x0E); 
	
	lcd_write_cmd(0x36); //MX, MY, RGB mode 
	lcd_write_data(0xC0); 
	
	//ST7735R Gamma Sequence
	lcd_write_cmd(0xe0); 
	lcd_write_data(0x0f); 
	lcd_write_data(0x1a); 
	lcd_write_data(0x0f); 
	lcd_write_data(0x18); 
	lcd_write_data(0x2f); 
	lcd_write_data(0x28); 
	lcd_write_data(0x20); 
	lcd_write_data(0x22); 
	lcd_write_data(0x1f); 
	lcd_write_data(0x1b); 
	lcd_write_data(0x23); 
	lcd_write_data(0x37); 
	lcd_write_data(0x00); 	
	lcd_write_data(0x07); 
	lcd_write_data(0x02); 
	lcd_write_data(0x10); 

	lcd_write_cmd(0xe1); 
	lcd_write_data(0x0f); 
	lcd_write_data(0x1b); 
	lcd_write_data(0x0f); 
	lcd_write_data(0x17); 
	lcd_write_data(0x33); 
	lcd_write_data(0x2c); 
	lcd_write_data(0x29); 
	lcd_write_data(0x2e); 
	lcd_write_data(0x30); 
	lcd_write_data(0x30); 
	lcd_write_data(0x39); 
	lcd_write_data(0x3f); 
	lcd_write_data(0x00); 
	lcd_write_data(0x07); 
	lcd_write_data(0x03); 
	lcd_write_data(0x10);  
	
	lcd_write_cmd(0x2a);
	lcd_write_data(0x00);
	lcd_write_data(0x00);
	lcd_write_data(0x00);
	lcd_write_data(0x7f);

	lcd_write_cmd(0x2b);
	lcd_write_data(0x00);
	lcd_write_data(0x00);
	lcd_write_data(0x00);
	lcd_write_data(0x9f);
	
	lcd_write_cmd(0xF0); //Enable test command  
	lcd_write_data(0x01); 
	lcd_write_cmd(0xF6); //Disable ram power save mode 
	lcd_write_data(0x00); 
	
	lcd_write_cmd(0x3A); //65k mode 
	lcd_write_data(0x05); 
	
	
	lcd_write_cmd(0x29);//Display on

    lcd_display_dir(1);         /* 默认为横屏 */
    lcd_clear(g_back_color);    /* 清屏 */
    LCD_LED_SET;                /* 背光打开 */
}


/**
 * @brief       LCD区域填充,,可使用缓冲区
 * @param       xs   : 区域起始X坐标
 *              ys   : 区域起始Y坐标
 *              xe   : 区域终止X坐标
 *              ye   : 区域终止Y坐标
 *              color: 区域填充颜色
 * @retval      无
 */
void lcd_fill(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye, uint16_t color)
{

    uint32_t index = 0;
    uint32_t totalpoint = (xe - xs + 1) * (ye - ys + 1);

    LCD_CS_CLR;
    lcd_set_address(xs, ys, xe, ye);
    LCD_RS_SET;
    for (index = 0; index < totalpoint; index++)
    {
        LCD_WriteData_16Bit(color);
    }
    LCD_CS_SET;
}

/**
 * @brief       LCD画点
 * @param       x    : 待画点的X坐标
 *              y    : 待画点的Y坐标
 *              color: 待画点的颜色
 * @retval      无
 */
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
	lcd_set_address(x,y,x+1,y+1);
	LCD_WriteData_16Bit(color);
}

/**
 * @brief       LCD画线段
 * @param       x1   : 待画线段端点1的X坐标 
 *              y1   : 待画线段端点1的Y坐标 
 *              x2   : 待画线段端点2的X坐标
 *              y2   : 待画线段端点2的Y坐标
 *              color: 待画线段的颜色
 * @note        x: 0 ~ spilcd_width
 *              y: 0 ~ spilcd_height
 *              与方向无关
 * @retval      无
 */
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t x_delta;
    uint16_t y_delta;
    int16_t x_sign;
    int16_t y_sign;
    int16_t error;
    int16_t error2;
    
    x_delta = (x1 < x2) ? (x2 - x1) : (x1 - x2);
    y_delta = (y1 < y2) ? (y2 - y1) : (y1 - y2);
    x_sign = (x1 < x2) ? 1 : -1;
    y_sign = (y1 < y2) ? 1 : -1;
    error = x_delta - y_delta;
    
    lcd_draw_point(x2, y2, color);
    
    while ((x1 != x2) || (y1 != y2))
    {
        lcd_draw_point(x1, y1, color);
        
        error2 = error << 1;
        if (error2 > -y_delta)
        {
            error -= y_delta;
            x1 += x_sign;
        }
      
        if (error2 < x_delta)
        {
            error += x_delta;
            y1 += y_sign;
        }
    }
}

/**
 * @brief       画水平线,可使用缓冲区
 * @param       x,y   : 起点坐标
 * @param       len   : 线长度
 * @param       color : 矩形的颜色
 * @retval      无
 */
void lcd_draw_hline(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
    if ((len == 0) || (x > spilcd_width) || (y > spilcd_height))
    {
        return;
    }

    lcd_fill(x, y, x + len - 1, y, color);
}

/**
 * @brief       LCD画矩形框
 * @param       x1   : 待画矩形框端点1的X坐标
 *              y1   : 待画矩形框端点1的Y坐标
 *              x2   : 待画矩形框端点2的X坐标
 *              y2   : 待画矩形框端点2的Y坐标
 *              color: 待画矩形框的颜色
 * @retval      无
 */
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    lcd_draw_line(x1, y1, x2, y1, color);
    lcd_draw_line(x1, y2, x2, y2, color);
    lcd_draw_line(x1, y1, x1, y2, color);
    lcd_draw_line(x2, y1, x2, y2, color);
}

/**
 * @brief       LCD画圆形框
 * @param       x    : 待画圆形框原点的X坐标
 *              y    : 待画圆形框原点的Y坐标
 *              r    : 待画圆形框的半径
 *              color: 待画圆形框的颜色
 * @retval      无
 */
void lcd_draw_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color)
{
    int32_t x_t;
    int32_t y_t;
    int32_t error;
    int32_t error2;
    
    x_t = -r;
    y_t = 0;
    error = 2 - 2 * r;
    
    do 
    {
        lcd_draw_point(x - x_t, y + y_t, color);
        lcd_draw_point(x + x_t, y + y_t, color);
        lcd_draw_point(x + x_t, y - y_t, color);
        lcd_draw_point(x - x_t, y - y_t, color);
        
        error2 = error;
        if (error2 <= y_t)
        {
            y_t++;
            error = error + (y_t * 2 + 1);
            if ((-x_t == y_t) && (error2 <= x_t))
            {
                error2 = 0;
            }
        }
        
        if (error2 > x_t)
        {
            x_t++;
            error = error + (x_t * 2 + 1);
        }
    } while (x_t <= 0);
}

/**
 * @brief       填充实心圆,可使用缓冲区
 * @param       x,y  : 圆中心坐标
 * @param       r    : 半径
 * @param       color: 圆的颜色
 * @retval      无
 */
void lcd_fill_circle(uint16_t x, uint16_t y, uint16_t r, uint16_t color)
{
    uint32_t i;
    uint32_t imax = ((uint32_t)r * 707) / 1000 + 1;
    uint32_t sqmax = (uint32_t)r * (uint32_t)r + (uint32_t)r / 2;
    uint32_t xr = r;

    lcd_draw_hline(x - r, y, 2 * r, color);

    for (i = 1; i <= imax; i++)
    {
        if ((i * i + xr * xr) > sqmax)
        {
            /* draw lines from outside */
            if (xr > imax)
            {
            lcd_draw_hline (x - i + 1, y + xr, 2 * (i - 1), color);
            lcd_draw_hline (x - i + 1, y - xr, 2 * (i - 1), color);
            }

            xr--;
        }

      /* draw lines from inside (center) */
      lcd_draw_hline(x - xr, y + i, 2 * xr, color);
      lcd_draw_hline(x - xr, y - i, 2 * xr, color);
    }
}



/**
 * @brief       LCD显示1个字符
 * @param       x    : 待显示字符的X坐标
 *              y    : 待显示字符的Y坐标
 *              ch   : 待显示字符
 *              font : 待显示字符的字体
 *              mode : 叠加方式(1); 非叠加方式(0)
 *              color: 待显示字符的颜色
 * @retval      无
 */
void lcd_show_char(uint16_t x, uint16_t y, char ch, lcd_font_t font, uint8_t mode, uint16_t color)
{
    const uint8_t *ch_code;
    uint8_t ch_width;
    uint8_t ch_height;
    uint8_t ch_size;
    uint8_t ch_offset;
    uint8_t byte_index;
    uint8_t byte_code;
    uint8_t bit_index;
    uint8_t width_index = 0;
    uint8_t height_index = 0;
    
    ch_offset = ch - ' ';   /* 得到偏移后的值（ASCII字库是从空格开始取模，所以-' '就是对应字符的字库） */
    
    switch (font)   /* 获取字体的高度以及宽度 */
    {
#if (FONT_12 != 0)
        case LCD_FONT_12:
        {
            ch_code = font_1206[ch_offset];
            ch_width = FONT_12_CHAR_WIDTH;
            ch_height = FONT_12_CHAR_HEIGHT;
            ch_size = FONT_12_CHAR_SIZE;
            break;
        }
#endif
#if (FONT_16 != 0)
        case LCD_FONT_16:
        {
            ch_code = font_1608[ch_offset];
            ch_width = FONT_16_CHAR_WIDTH;
            ch_height = FONT_16_CHAR_HEIGHT;
            ch_size = FONT_16_CHAR_SIZE;
            break;
        }
#endif
#if (FONT_24 != 0)
        case LCD_FONT_24:
        {
            ch_code = font_2412[ch_offset];
            ch_width = FONT_24_CHAR_WIDTH;
            ch_height = FONT_24_CHAR_HEIGHT;
            ch_size = FONT_24_CHAR_SIZE;
            break;
        }
#endif
#if (FONT_32 != 0)
        case LCD_FONT_32:
        {
            ch_code = font_3216[ch_offset];
            ch_width = FONT_32_CHAR_WIDTH;
            ch_height = FONT_32_CHAR_HEIGHT;
            ch_size = FONT_32_CHAR_SIZE;
            break;
        }
#endif
        default:
        {
            return;
        }
    }
    
    if ((x + ch_width > spilcd_width) || (y + ch_height > spilcd_height))
    {
        return;
    }
    
    for (byte_index = 0; byte_index < ch_size; byte_index++)
    {
        byte_code = ch_code[byte_index];                  /* 获取字符的点阵数据 */

        for (bit_index = 0; bit_index < 8; bit_index++)   /* 一个字节8个点 */
        {
            if ((byte_code & 0x80) != 0)                  /* 有效点,需要显示 */
            {
                lcd_draw_point(x + width_index, y + height_index, color);           /* 画点出来,要显示这个点 */
            }
            else if (mode == 0)
            {
                lcd_draw_point(x + width_index, y + height_index, g_back_color);    /* 画背景色,相当于这个点不显示(注意背景色由全局变量控制) */
            }

            height_index++;

            if (height_index == ch_height)    /* 显示完一列了? */
            {
                height_index = 0;             /* y坐标复位 */
                width_index++;                /* x坐标递增 */
                break;
            }

            byte_code <<= 1;                  /* 移位, 以便获取下一个位的状态 */
        }
    }
}

/**
 * @brief       LCD显示字符串
 * @note        会自动换行
 * @param       x    : 待显示字符串的X坐标
 *              y    : 待显示字符串的Y坐标
 *              font : 待显示字符串的字体
 *              str  : 待显示字符串
 *              color: 待显示字符串的颜色
 * @retval      无
 */
void lcd_show_string(uint16_t x, uint16_t y, lcd_font_t font,const char *str,  uint16_t color)
{
    uint16_t x0 = x;
    uint8_t ch_width;
    uint8_t ch_height;

    switch (font)   /* 获取字体的高度以及宽度 */
    {
#if (FONT_12 != 0)
        case LCD_FONT_12:
        {
            ch_width = FONT_12_CHAR_WIDTH;
            ch_height = FONT_12_CHAR_HEIGHT;
            break;
        }
#endif
#if (FONT_16 != 0)
        case LCD_FONT_16:
        {
            ch_width = FONT_16_CHAR_WIDTH;
            ch_height = FONT_16_CHAR_HEIGHT;
            break;
        }
#endif
#if (FONT_24 != 0)
        case LCD_FONT_24:
        {
            ch_width = FONT_24_CHAR_WIDTH;
            ch_height = FONT_24_CHAR_HEIGHT;
            break;
        }
#endif
#if (FONT_32 != 0)
        case LCD_FONT_32:
        {
            ch_width = FONT_32_CHAR_WIDTH;
            ch_height = FONT_32_CHAR_HEIGHT;
            break;
        }
#endif
        default:
        {
            return;
        }
    }
    
    while ((*str >= ' ') && (*str <= '~'))   /* 判断是不是非法字符! */
    {
        if (x + ch_width > spilcd_width)
        {
            x = x0;
            y += ch_height;
        }
        
        if (y + ch_height > spilcd_height)
        {
            break;
        }
        
        lcd_show_char(x, y, *str, font, 0, color);
        
        x += ch_width;
        str++;
    }
}

/**
 * @brief       平方函数，x^y
 * @param       x: 底数
 *              y: 指数
 * @retval      x^y
 */
static uint32_t lcd_pow(uint8_t x, uint8_t y)
{
    uint8_t loop;
    uint32_t res = 1;
    
    for (loop = 0; loop < y; loop++)
    {
        res *= x;
    }
    
    return res;
}

/**
 * @brief       LCD显示数字，可控制显示高位0
 * @param       x    : 待显示数字的X坐标
 *              y    : 待显示数字的Y坐标
 *              num  : 待显示数字
 *              len  : 待显示数字的位数
 *              mode : NUM_SHOW_NOZERO: 数字高位0不显示
 *                     NUM_SHOW_ZERO  : 数字高位0显示
 *              font : 待显示数字的字体
 *              color: 待显示数字的颜色
 * @retval      无
 */
void lcd_show_num(uint16_t x, uint16_t y, uint32_t num, uint8_t len,  lcd_font_t font, num_mode_t mode, uint16_t color)
{
    uint8_t ch_width;
    uint8_t len_index;
    uint8_t num_index;
    uint8_t first_nozero = 0;
    char pad;
  
    switch (font)
    {
#if (FONT_12 != 0)
        case LCD_FONT_12:
        {
            ch_width = FONT_12_CHAR_WIDTH;
            break;
        }
#endif
#if (FONT_16 != 0)
        case LCD_FONT_16:
        {
            ch_width = FONT_16_CHAR_WIDTH;
            break;
        }
#endif
#if (FONT_24 != 0)
        case LCD_FONT_24:
        {
            ch_width = FONT_24_CHAR_WIDTH;
            break;
        }
#endif
#if (FONT_32 != 0)
        case LCD_FONT_32:
        {
            ch_width = FONT_32_CHAR_WIDTH;
            break;
        }
#endif
        default:
        {
            return;
        }
    }
  
    switch (mode)
    {
        case NUM_SHOW_NOZERO:
        {
            pad = ' ';
            break;
        }
        case NUM_SHOW_ZERO:
        {
            pad = '0';
            break;
        }
        default:
        {
            return;
        }
    }
  
    for (len_index = 0; len_index < len; len_index++)                 /* 按总显示位数循环 */
    {
        num_index = (num / lcd_pow(10, len - len_index - 1)) % 10;    /* 获取对应位的数字 */
        if ((first_nozero == 0) && (len_index < (len - 1)))           /* 没有使能显示,且还有位要显示 */
        {
            if (num_index == 0)
            {
                lcd_show_char(x + ch_width * len_index, y, pad, font, mode & 0x01, color);   /* 高位需要填充0 */
                continue;
            }
            else
            {
                first_nozero = 1;   /* 使能显示 */
            }
        }
        
        lcd_show_char(x + ch_width * len_index, y, num_index + '0', font, mode & 0x01, color);
    }
}


/**
 * @brief       LCD显示十六进制数字
 * @param       x    : 待显示数字的X坐标    LCD_FONT_len/2*(Cloumn-1)
 *              y    : 待显示数字的Y坐标    LCD_FONT_len*(Row-1)
 *              num  : 待显示的十六进制数字
 *              len  : 待显示数字的位数(1~8位)
 *              mode : NUM_SHOW_NOZERO: 十六进制高位0不显示
 *                     NUM_SHOW_ZERO  : 十六进制高位0显示
 *              font : 待显示数字的字体
 *              color: 待显示数字的颜色
 * @retval      无
 */
void lcd_show_hexNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, lcd_font_t font, num_mode_t mode, uint16_t color)
{
    uint8_t ch_width;
    uint8_t len_index;
    uint8_t hex_index;
    uint8_t first_nozero = 0;
    char pad;
    char hex_char;

    switch (font)
    {
#if (FONT_12 != 0)
        case LCD_FONT_12:
        {
            ch_width = FONT_12_CHAR_WIDTH;
            break;
        }
#endif
#if (FONT_16 != 0)
        case LCD_FONT_16:
        {
            ch_width = FONT_16_CHAR_WIDTH;
            break;
        }
#endif
#if (FONT_24 != 0)
        case LCD_FONT_24:
        {
            ch_width = FONT_24_CHAR_WIDTH;
            break;
        }
#endif
#if (FONT_32 != 0)
        case LCD_FONT_32:
        {
            ch_width = FONT_32_CHAR_WIDTH;
            break;
        }
#endif
        default:
        {
            return;
        }
    }

    switch (mode)
    {
        case NUM_SHOW_NOZERO:
        {
            pad = ' ';
            break;
        }
        case NUM_SHOW_ZERO:
        {
            pad = '0';
            break;
        }
        default:
        {
            return;
        }
    }

    lcd_fill(x,y,x+8*len,y+ch_width*2,g_back_color);    //清空背景

    for (len_index = 0; len_index < len; len_index++)                 /* 按总显示位数循环 */
    {
        hex_index = (num >> ((len - len_index - 1) * 4)) & 0xF;       /* 获取对应位的十六进制数字 */
        if ((first_nozero == 0) && (len_index < (len - 1)))           /* 没有使能显示,且还有位要显示 */
        {
            if (hex_index == 0)
            {
                lcd_show_char(x + ch_width * len_index, y, pad, font, mode & 0x01, color);   /* 高位需要填充0或空格 */
                continue;
            }
            else
            {
                first_nozero = 1;   /* 使能显示 */
            }
        }
        
        if (hex_index < 10)
        {
            hex_char = hex_index + '0';  /* 0-9 */
        }
        else
        {
            hex_char = hex_index - 10 + 'A';  /* A-F */
        }
        
        lcd_show_char(x + ch_width * len_index, y, hex_char, font, mode & 0x01, color);
    }
}


/**
 * @brief       LCD图片显示
 * @note        图片取模方式: 水平扫描、RGB565、高位在前
 * @param       x     : 待显示图片的X坐标
 *              y     : 待显示图片的Y坐标
 *              width : 待显示图片的宽度
 *              height: 待显示图片的高度
 *              pic   : 待显示图片数组首地址
 * @retval      无
 */
void lcd_show_pic(uint16_t x, uint16_t y, uint16_t width, uint16_t height,const uint8_t *pic)
{
    uint32_t index = 0;
    uint32_t totalpoint = width * height;
    
    if ((x + width > spilcd_width) || (y + height > spilcd_height))
    {
        return;
    }

    // 设置显示区域
    lcd_set_address(x, y, x + width - 1, y + height - 1);

    // 逐点绘制
    for (index = 0; index < totalpoint; index++)
    {
        uint16_t color = (pic[index * 2] << 8) | pic[index * 2 + 1];
        lcd_draw_point(x + (index % width), y + (index / width), color);
    }
}








#include <stdarg.h>
/**
  * @brief   LCD使用printf函数打印格式化字符串
  * @param   Line   起始行位置  1 ～ spilcd_height/LCD_FONT_X
  * @param   Column 起始列位置  1 ～ spilcd_width/LCD_FONT_X
  * @param   font   字体大小 LCD_FONT_12 LCD_FONT_16 LCD_FONT_24 LCD_FONT_32
  * @param   format 指定要显示的格式化字符串
  * @param   ...    格式化字符串参数列表
  * @retval  无
 */
void lcd_printf(uint16_t Line, uint16_t Column, lcd_font_t font, const char *format, ...)
{
    char string[256];
    va_list arg;
    va_start(arg, format);
    vsnprintf(string, sizeof(string), format, arg);
    va_end(arg);
    
    uint16_t x, y;
    uint8_t ch_width, ch_height;
    
    // 根据字体类型获取字符宽度和高度
    switch (font)
    {
#if (FONT_12 != 0)
        case LCD_FONT_12:
            ch_width = FONT_12_CHAR_WIDTH;
            ch_height = FONT_12_CHAR_HEIGHT;
            break;
#endif
#if (FONT_16 != 0)
        case LCD_FONT_16:
            ch_width = FONT_16_CHAR_WIDTH;
            ch_height = FONT_16_CHAR_HEIGHT;
            break;
#endif
#if (FONT_24 != 0)
        case LCD_FONT_24:
            ch_width = FONT_24_CHAR_WIDTH;
            ch_height = FONT_24_CHAR_HEIGHT;
            break;
#endif
#if (FONT_32 != 0)
        case LCD_FONT_32:
            ch_width = FONT_32_CHAR_WIDTH;
            ch_height = FONT_32_CHAR_HEIGHT;
            break;
#endif
        default:
            return;
    }
    
    // 计算实际像素位置
    x = (Column - 1) * ch_width;
    y = (Line - 1) * ch_height;
    
    lcd_show_string(x, y, font, string, g_point_color);
}





/**
 * @brief       LCD显示单个汉字
 * @param       x     : 待显示汉字的X坐标
 *              y     : 待显示汉字的Y坐标
 *              index : 汉字在字模库中的索引
 *              color : 待显示汉字的颜色
 * @retval      无
 */
void lcd_show_chinese_char(uint16_t x, uint16_t y, uint8_t index)
{
    uint8_t i, j;
    uint8_t dataH, dataL;
    
    /* 检查坐标是否在屏幕范围内 */
    if ((x + 16 > spilcd_width) || (y + 16 > spilcd_height)) {
        return;
    }
    
    if(spilcd_dir == 0)
    {
        for (i = 0; i < 16; i++) {              /* 列 */
            for (j = 0; j < 16; j++) {          /* 行 */
                /* 每行由两个字节组成：Data[row] 为列0..7 的位，Data[row+16] 为列8..15 的位 */
                if (i < 8) {
                    dataH = OLED_CF16x16[index].Data[j];
                    if ((dataH >> (7 - i)) & 0x01) {
                        lcd_draw_point(x + i,spilcd_height - (y + j), g_point_color);
                    }
                } 
                else {
                    dataL = OLED_CF16x16[index].Data[j + 16];
                    if ((dataL >> (7 - (i - 8))) & 0x01) {
                        lcd_draw_point(x + i,spilcd_height - (y + j), g_point_color);
                    }
                }
            }
        }
    }
    else{   // 竖屏显示
        for (i = 0; i < 16; i++) {              /* 列 */
            for (j = 0; j < 16; j++) {          /* 行 */
                /* 每行由两个字节组成：Data[row] 为列0..7 的位，Data[row+16] 为列8..15 的位 */
                if (i < 8) {
                    dataH = OLED_CF16x16[index].Data[j];
                    if ((dataH >> (7 - i)) & 0x01) {
                        lcd_draw_point(y + j,x + i, g_point_color);
                    }
                } 
                else {
                    dataL = OLED_CF16x16[index].Data[j + 16];
                    if ((dataL >> (7 - (i - 8))) & 0x01) {
                        lcd_draw_point(y + j,x + i, g_point_color);
                    }
                }
            }
        }
    }
    
}


/**
 * @brief       LCD显示汉字串
 * @param       Line : 待显示汉字串的行数
 *              Column : 待显示汉字串的列数
 *              chn  : 待显示的汉字串
 * @retval      无
 */
void lcd_show_chinese(uint16_t Line, uint16_t Column, const char *chn)
{
    uint8_t pChn = 0;
    uint8_t pIndex;
    uint8_t x,y,i;
    // 汉字字符的大小为16x16
    if(spilcd_dir == 0)
    {
        x = (Column - 1) * 16;
        y = (Line - 1) * 16;
    }
    else
    {
        y = (Column - 1) * 16;
        x = (Line - 1) * 16;
    }
    char singleChn[3 + 1] = {0};  // GB2312编码下一个汉字占3个字节（UTF-8）
    
    for (i = 0; chn[i] != '\0'; i++) {
        singleChn[pChn] = chn[i];
        pChn++;
        
        /* 当提取次数到达3时，即代表提取到了一个完整的汉字 */
        if (pChn >= 3) {
            pChn = 0;  /* 计次归零 */
            
            /* 遍历整个汉字字模库，寻找匹配的汉字 */
            /* 如果找到最后一个汉字（定义为空字符串），则表示汉字未在字模库定义，停止寻找 */
            for (pIndex = 0; strcmp(OLED_CF16x16[pIndex].Index, "") != 0; pIndex++) {
                /* 找到匹配的汉字 */
                if (strcmp(OLED_CF16x16[pIndex].Index, singleChn) == 0) {
                    break;  /* 跳出循环，此时pIndex的值为指定汉字的索引 */
                }
            }
            spilcd_dir ?
                lcd_show_chinese_char(x , y + ((i + 1) / 3 - 1) * 16, pIndex):
                lcd_show_chinese_char(x + ((i + 1) / 3 - 1) * 16, y, pIndex);
        }
    }
}

