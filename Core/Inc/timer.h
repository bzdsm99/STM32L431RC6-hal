//timer.h
#ifndef __TIMER_H
#define __TIMER_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32l4xx_hal.h"

extern TIM_HandleTypeDef timx_Handles[13];
extern uint8_t g_timxchy_cap_sta;    /* 输入捕获状态 */
extern uint16_t g_timxchy_cap_val;   /* 输入捕获值 */

void Timx_Init(TIM_TypeDef *Timx, uint16_t arr, uint16_t psc);

#ifdef __cplusplus
}
#endif
#endif // !__TIMER_H



