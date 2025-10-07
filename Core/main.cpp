#include "stm32l4xx_hal.h"
#include "borad.h"
#include "delay.h"
#include "sys.h"
#include "OLED.h"
#include "usart.h"
#include "Timer.h"

uint8_t num = 0;

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

    Timx_Init(TIM1, 0xFFFF, 80-1);

    printf("test start...\r\n");

    tick_time = HAL_GetTick();  // 初始化时间为系统启动时间
    while (1)
    {
        OLED_ShowString(1,1,"Num: ");
        OLED_ShowNum(1,6,num,4);
        elapsed_time = HAL_GetTick() - tick_time;  // 计算经过的时间
        //OLED_ShowString(3,1,"Time: ");
        OLED_ShowNum(4,1,elapsed_time,12);


        OLED_ShowNum(2, 1, g_timxchy_cap_sta, 3);
        OLED_ShowNum(3, 1, g_timxchy_cap_val, 5);
        
        delay_ms(50);
    }
}


/**
 * @brief       定时器更新中断回调函数
 * @param       htim:定时器句柄
 * @retval      无
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        HAL_GPIO_TogglePin(LED.GPIOx,LED.pin);
    }
    if (htim->Instance == TIM7)
    {
        HAL_GPIO_TogglePin(LED.GPIOx,LED.pin);
    }
    if (htim->Instance == TIM1)
    {
        HAL_GPIO_TogglePin(LED.GPIOx,LED.pin);
        if ((g_timxchy_cap_sta & 0X80) == 0)            /* 还未成功捕获 */
        {
            if (g_timxchy_cap_sta & 0X40)               /* 已经捕获到高电平了 */
            {
                if ((g_timxchy_cap_sta & 0X3F) == 0X3F) /* 高电平太长了 */
                {
                    TIM_RESET_CAPTUREPOLARITY(&timx_Handles[1], TIM_CHANNEL_1);                     /* 一定要先清除原来的设置 */
                    TIM_SET_CAPTUREPOLARITY(&timx_Handles[1], TIM_CHANNEL_1, TIM_ICPOLARITY_RISING);/* 配置TIM5通道1上升沿捕获 */
                    g_timxchy_cap_sta |= 0X80;          /* 标记成功捕获了一次 */
                    g_timxchy_cap_val = 0XFFFF;
                }
                else      /* 累计定时器溢出次数 */
                {
                    g_timxchy_cap_sta++;
                }
            }
        }
    }
    num++;
}
