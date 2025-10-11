#include "stm32l4xx_hal.h"
#include "borad.h"
#include "delay.h"
#include "sys.h"
#include "OLED.h"
#include "usart.h"
#include "Timer.h"

static uint16_t duty = 0;       // 初始占空比
static int16_t direction;    // 增加方向标志：1 表示增加，-1 表示减少

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
    // timx_pwmStart_init(TIM1,5000-1,80-1);
    timx_pwmStart_init(TIM2,5000-1, 80-1);
    // atim1_npwmStart_init(5000-1 , 80-1);
    Timx_ICStart_Init(TIM1);

    printf("test start...\r\n");

    tick_time = HAL_GetTick();  // 初始化时间为系统启动时间

    while (1)
    {
        elapsed_time = HAL_GetTick() - tick_time;  // 计算经过的时间
        OLED_ShowNum(4,1,elapsed_time,12);
        //OLED_ShowNum(2,1,HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_8),1);
        //printf("TIM1 Counter Value: %lu\r\n", TIM1->CNT); // 打印计数器值
        OLED_ShowNum(1,1,Timx_ICHighTime(),8);
        timx_pwmSetCompare(TIM2,TIM_CHANNEL_1,duty);
        // 更新 duty 值
        duty += direction;
        // 检查边界条件并改变方向
        if (duty >= 5000) {
            direction = -200;  // 开始减少
        } else if (duty <= 0) {
            direction = 200;   // 开始增加
        }
    }
}









