#include "stm32l4xx_hal.h"
#include "borad.h"
#include "delay.h"
#include "sys.h"
#include "OLED.h"
#include "usart.h"
#include "Timer.h"

uint8_t num = 0;

// void GPIO_PA8_input(void)
// {
//     __GPIOA_CLK_ENABLE();
//     GPIO_InitTypeDef GPIO_InitStruct;
//     GPIO_InitStruct.Pin = GPIO_PIN_8;
//     GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
//     GPIO_InitStruct.Pull = GPIO_NOPULL;
//     HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
// }

int main(void)
{
    uint32_t elapsed_time, tick_time;
    //uint32_t temp;
    HAL_Init();
    sys_stm32_clock_init(20);
    delay_init(80); //80MHZ
    OLED_Init();
    key_init();
    led_init();
    LED_Init();
    usart_init(115200);
    //GPIO_PA8_input();
    Timx_ICStart_Init(TIM1, 0xFFFF, 80-1);

    printf("test start...\r\n");

    tick_time = HAL_GetTick();  // 初始化时间为系统启动时间
    /* 打印 TIM1 相关寄存器的值 */
    printf("TIM1_CR1: 0x%08lX\r\n", TIM1->CR1);
    printf("TIM1_CCMR1: 0x%08lX\r\n", TIM1->CCMR1);
    printf("TIM1_CCER: 0x%08lX\r\n", TIM1->CCER);
    printf("TIM1_SR: 0x%08lX\r\n", TIM1->SR);
    printf("TIM1_CCR1: 0x%08lX\r\n", TIM1->CCR1);
    while (1)
    {
        OLED_ShowString(1,1,"Num: ");
        OLED_ShowNum(1,6,num,4);
        elapsed_time = HAL_GetTick() - tick_time;  // 计算经过的时间
        //OLED_ShowString(3,1,"Time: ");
        OLED_ShowNum(4,1,elapsed_time,12);
        OLED_ShowNum(2,1,HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_8),1);

        // OLED_ShowNum(3,1,g_timxchy_cap_sta,4);
        // OLED_ShowNum(3,6,g_timxchy_cap_val,4);
        // if(g_timxchy_cap_sta & 0X80){
        //     temp = g_timxchy_cap_sta & 0X3F;
        //     temp *= 65536;
        //     temp += g_timxchy_cap_val;
        //     OLED_ShowNum(4,1,temp,12);
        //     g_timxchy_cap_sta = 0;
        // }
        
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
        // if ((g_timxchy_cap_sta & 0X80) == 0)            /* 还未成功捕获 */
        // {
        //     if (g_timxchy_cap_sta & 0X40)               /* 已经捕获到高电平了 */
        //     {
        //         if ((g_timxchy_cap_sta & 0X3F) == 0X3F) /* 高电平太长了 */
        //         {
        //             TIM_RESET_CAPTUREPOLARITY(&timx_Handles[1], TIM_CHANNEL_1);                     /* 一定要先清除原来的设置 */
        //             TIM_SET_CAPTUREPOLARITY(&timx_Handles[1], TIM_CHANNEL_1, TIM_ICPOLARITY_RISING);/* 配置TIM5通道1上升沿捕获 */
        //             g_timxchy_cap_sta |= 0X80;          /* 标记成功捕获了一次 */
        //             g_timxchy_cap_val = 0XFFFF;
        //         }
        //         else      /* 累计定时器溢出次数 */
        //         {
        //             g_timxchy_cap_sta++;
        //         }
        //     }
        // }
        num++;
    }
    
}
