#include "stm32l4xx_hal.h"
#include "borad.h"
#include "delay.h"
#include "sys.h"
#include "OLED.h"
#include "usart.h"
#include "Timer.hpp"

Timer timer6 = Timer(TIM6,5000-1,8000-1);

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

        OLED_ShowNum(4,1,elapsed_time,12);
        delay_ms(50);
    }
}

/**
 * @brief       定时器更新中断回调函数
 * @param       htim:定时器句柄
 * @retval      无
 */
extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        HAL_GPIO_TogglePin(LED.GPIOx,LED.pin);
    }
    if (htim->Instance == TIM7)
    {
        HAL_GPIO_TogglePin(LED.GPIOx,LED.pin);
    }
}

/**
 * @brief       定时器TIM6中断服务函数
 * @param       无
 * @retval      无
 */
extern "C" void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&timer6.timx_handle); /* 定时器中断公共处理函数 */
}
