#include "stm32l4xx_hal.h"
#include "borad.h"
#include "delay.h"
#include "sys.h"
#include "OLED.h"
#include "usart.h"
#include "Timer.h"





int main(void)
{
    uint32_t elapsed_time, tick_time;
    uint32_t temp;
    HAL_Init();
    sys_stm32_clock_init(20);
    delay_init(80); //80MHZ
    OLED_Init();
    key_init();
    led_init();
    LED_Init();
    usart_init(115200);
    timx_pwmStart_init(TIM2, 5000-1 , 8000-1);
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
        //OLED_ShowNum(2,1,HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_8),1);
        printf("TIM1 Counter Value: %lu\r\n", TIM1->CNT); // 打印计数器值
        // OLED_ShowNum(3,1,g_timxchy_cap_sta,4);
        // OLED_ShowNum(3,6,g_timxchy_cap_val,4);
        if (g_timxchy_cap_sta & 0X80) {
            temp = (g_timxchy_cap_sta & 0X3F) * 65536 + g_timxchy_cap_val;
            OLED_ShowNum(3, 1, temp, 8); /* 显示高电平持续时间（单位：微秒） */
            g_timxchy_cap_sta = 0;
        }
    }
}



