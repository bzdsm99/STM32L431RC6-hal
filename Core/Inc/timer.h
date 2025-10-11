//timer.h
#ifndef __TIMER_H
#define __TIMER_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32l4xx_hal.h"
#define TIM1_NPWM_USE

typedef enum {
    RTC_KEY = 0,
    TIM1_KEY,
    TIM2_KEY,
    TIM6_KEY,
    TIM7_KEY,
    TIM15_KEY,
    TIM16_KEY,
    LPTIM1_KEY,
    LPTIM2_KEY
} TimerKey_t;



extern TIM_HandleTypeDef timx_Handles[9];
extern uint8_t g_timxchy_cap_sta;    /* 输入捕获状态 */
extern uint16_t g_timxchy_cap_val;   /* 输入捕获值 */


void Timx_baseStart_Init(TIM_TypeDef *Timx, uint16_t arr, uint16_t psc);

void Timx_ICStart_Init(TIM_TypeDef *Timx);
uint16_t Timx_ICHighTime(void);

void timx_pwmStart_init(TIM_TypeDef *Timx, uint16_t arr, uint16_t psc);
#ifdef TIM1_NPWM_USE
    void atim1_npwmStart_init(uint16_t arr, uint16_t psc);
    void atim1_npwm_chy_set(uint32_t npwm);
#endif
void timx_pwmSetCompare(TIM_TypeDef *Timx,unsigned int TIM_CHANNEL_x,uint16_t pwm_vaule);



#ifdef __cplusplus
}
#endif
#endif // !__TIMER_H



