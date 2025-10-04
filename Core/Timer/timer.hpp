//BaseTimer.hpp
#pragma once

#include "stm32l4xx_hal.h"
extern TIM_HandleTypeDef *timer_handles[13];

class Timer{
public:

    TIM_HandleTypeDef timx_handle;
    TIM_TypeDef * timx;
    Timer() {};
    Timer(TIM_TypeDef *timx,uint16_t arr, uint16_t psc);

};



