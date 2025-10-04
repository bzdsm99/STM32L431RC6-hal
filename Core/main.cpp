#include "stm32l4xx_hal.h"
#include "borad.h"
#include "delay.h"
#include "sys.h"
#include "OLED.h"
#include "usart.h"



int main(void)
{
    int32_t elapsed_time, tick_time;
    
    HAL_Init();
    sys_stm32_clock_init(20);
    delay_init(80); //80MHZ
    OLED_Init();
    key_init();
    led_init();
    LED_Init();
    usart_init(115200);
    printf("test start...\r\n");

    
    tick_time = HAL_GetTick();  // 初始化时间为系统启动时间
    
    while (1)
    {
        elapsed_time = HAL_GetTick() - tick_time;  // 计算经过的时间

        OLED_ShowNum(4,1,elapsed_time,3);
        delay_ms(50);
    }
}
