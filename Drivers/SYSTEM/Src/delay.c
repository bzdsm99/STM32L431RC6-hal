#include "delay.h"
#include "stm32l4xx_hal.h"

extern __IO uint32_t uwTick;

void delay_init(void)
{
    // 初始化DWT
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void delay_us(uint32_t us)
{
    // 启用DWT
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    
    // 获取当前cycle count
    uint32_t startCycles = DWT->CYCCNT;
    uint32_t cyclesToWait = us * (SystemCoreClock / 1000000);
    
    // 等待直到达到所需的cycle数
    while ((DWT->CYCCNT - startCycles) < cyclesToWait);
}

void delay_ms(uint32_t ms)
{
    // 检查调度器是否正在运行
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) 
    {
        // 如果调度器正在运行，使用RTOS的延时函数
        vTaskDelay(pdMS_TO_TICKS(ms));
    }
    else 
    {
        // 如果调度器未运行，使用基于uwTick的循环延时
        uint32_t startTick = HAL_GetTick();
        while((HAL_GetTick() - startTick) < ms);
    }
}