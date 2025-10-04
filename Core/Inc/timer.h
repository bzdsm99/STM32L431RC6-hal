//timer.h
#ifndef __TIMER_H
#define __TIMER_H
#include "stm32l4xx_hal.h"
#ifdef __cplusplus
extern "C"
{
#endif



#define BTIM_TIMX_INT                       TIM6
#define BTIM_TIMX_INT_IRQn                  TIM6_DAC_IRQn
#define BTIM_TIMX_INT_IRQHandler            TIM6_DAC_IRQHandler
#define BTIM_TIMX_INT_CLK_ENABLE()          do{ __HAL_RCC_TIM6_CLK_ENABLE(); }while(0)   /* TIM6 时钟使能 */

void btim_timx_int_init(uint16_t arr, uint16_t psc);

#ifdef __cplusplus
}
#endif
#endif // !__TIMER_H
