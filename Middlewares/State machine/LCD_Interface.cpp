//LCD_Interface.cpp
#include "LCD_Interface.hpp"
#include "gpio.h"
#include "lcd.h"
#include "sys.h"
#include "usart.h"
#include "borad.h"
#include "dac.h"
#include "delay.h"
#include "Matrix_keyboard.h"

    // MX_FREERTOS_Init();
    // osKernelStart();


    // g_back_color = BLACK;
    // g_point_color = WHITE;
    
    // Lock_screen();

    // g_back_color = WHITE;

    // g_back_color = WHITE;
    // g_point_color = BLACK;
    // Start_up();
    
void LCD_Interface::Interface_Init(void)
{
    Matrix_keyboard_init();
    dac_init();
    lcd_init();
    usart_init(115200);
}

LCD_Interface::LCD_Interface(void)
{
    Product_Name = "CNMBD";
    Interface_Init();
}


// 锁屏界面
void LCD_Interface::Lock_screen(void)
{
    lcd_show_chinese(4, 3, "锁屏状态");
}

// 上电开机界面
void LCD_Interface::Start_up(void)
{
    lcd_show_chinese(1, 1, "张庭华");
    lcd_show_chinese(2, 1, "第");
    lcd_printf(2, 3, LCD_FONT_16, "%d", 1);
    lcd_show_chinese(2, 3, "页");
    lcd_show_chinese(3, 1, "光照强度值");
}










