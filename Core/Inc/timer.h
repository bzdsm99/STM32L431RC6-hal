//timer.h
#ifndef __TIMER_H
#define __TIMER_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32l4xx_hal.h"

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


#define TIM1_NPWM_USE                       // 使用定时器1的数量可控NPWM
#define TIMER_KEY_COUNT (LPTIM2_KEY + 1)    // 定时器数量
#define __TIMx_IC_MAX_CHANNLES  4           // 定时器最大通道数 1~4

/*对目前HAL_TIM_IC_CaptureCallback中通道一的高电平捕获实现的二次封装*/
#define TIM1Channel1_ICStart_Init() do{Timx_ICStart_Init(TIM1, 1, TIM_CHANNEL_1);}while(0)
uint16_t Timx_ICHighTime(void);

// 定义定时器通道配置结构体
typedef struct
{
    TIM_TypeDef *Timx;         // 定时器实例
    uint16_t channels[__TIMx_IC_MAX_CHANNLES];      /* 保存定时器使用的通道时加一后保存,防止通道一保存为0 */
    uint8_t channel_count;     // 当前使用的通道数量
    TIM_HandleTypeDef timx_Handles;
} TimerConfig;

extern TimerConfig timer_configs[TIMER_KEY_COUNT];

extern uint8_t g_timxchy_cap_sta;    /* 输入捕获状态 */
extern uint16_t g_timxchy_cap_val;   /* 输入捕获值 */

void Timx_baseStart_Init(TIM_TypeDef *Timx, uint16_t arr, uint16_t psc);
void Timx_ICStart_Init(TIM_TypeDef *Timx, uint8_t channel_count, ...);

void timx_pwmStart_init(TIM_TypeDef *Timx, uint16_t arr, uint16_t psc,
    uint8_t channel_count, ...);
#ifdef TIM1_NPWM_USE
    void atim1_npwmStart_init(uint16_t arr, uint16_t psc);
    void atim1_npwm_chy_set(uint32_t npwm);
#endif
void timx_pwmSetCompare(TIM_TypeDef *Timx,unsigned int TIM_CHANNEL_x,uint16_t pwm_vaule);

#ifdef __cplusplus
}
#endif
#endif // !__TIMER_H



