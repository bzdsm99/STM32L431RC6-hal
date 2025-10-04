//BaseTimer.cpp
#include "Timer.hpp"



Timer::Timer(TIM_TypeDef *Timx, uint16_t arr, uint16_t psc)
:timx(Timx)
{

    timx_handle.Instance = Timx;
    timx_handle.Init.Prescaler = psc;                          /* 设置预分频系数 */
    timx_handle.Init.CounterMode = TIM_COUNTERMODE_UP;         /* 递增计数模式 */
    timx_handle.Init.Period = arr;                             /* 自动装载值 */
    HAL_TIM_Base_Init(&timx_handle);
    HAL_TIM_Base_Start_IT(&timx_handle);    /* 使能定时器x及其更新中断 */
}

/**
 * @brief       定时器底层驱动，开启时钟，设置中断优先级
                此函数会被HAL_TIM_Base_Init()函数调用
 * @param       htim:定时器句柄
 * @retval      无
 */
extern "C" void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        __HAL_RCC_TIM6_CLK_ENABLE();                     /* 使能TIM时钟 */
        HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 1, 3); /* 抢占1，子优先级3，组2 */
        HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);         /* 开启ITM3中断 */
    }
    else if (htim->Instance == TIM7)
    {
        __HAL_RCC_TIM7_CLK_ENABLE();                     /* 使能TIM时钟 */
        HAL_NVIC_SetPriority(TIM7_IRQn, 1, 3); /* 抢占1，子优先级3，组2 */
        HAL_NVIC_EnableIRQ(TIM7_IRQn);         /* 开启ITM3中断 */
    }
}




