#include "stm32l4xx_hal.h"
#include "borad.h"
#include "delay.h"
#include "sys.h"
#include "OLED.h"
#include "usart.h"
#include "Timer.h"
#include "Matrix_keyboard.h"
uint8_t key_char = 0;
uint16_t led_status = 0;    //RGB565

int main(void)
{
    uint32_t elapsed_time, tick_time;
    HAL_Init();
    sys_stm32_clock_init(20);
    delay_init(80); //80MHZ
    OLED_Init();
    key_init();
    led_init();
    LED_Init();
    usart_init(115200);
    Matrix_keyboard_init();

    printf("test start...\r\n");
    tick_time = HAL_GetTick();  // 初始化时间为系统启动时间
    while (1)
    {
        elapsed_time = HAL_GetTick() - tick_time;  // 计算经过的时间
        OLED_ShowNum(4,1,elapsed_time,12);

        OLED_ShowString(1,1, "you press key:");
        key_char = Matrix_keyboard_scan();
        if(key_char != 0)
        {
            OLED_ShowChar(2,1,key_char);
        }

        //如果按下的是'1'键，就切换红色LED的状态
        if(key_char == '1')
        {
            led_status ^= 0xF100;
        }
        else if(key_char == '2')
        {
            led_status = 0x07E0;
        }
        else if(key_char == '3')
        {
            led_status = 0x001F;
        }
        else if(key_char == '4')
        {
            led_status = 0xFFFF;
        }
        else if(key_char == '5')
        {
            led_status = 0;
        }
        led_rgb565(led_status);
    }
}









