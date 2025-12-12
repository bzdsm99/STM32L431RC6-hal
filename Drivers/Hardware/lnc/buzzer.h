#ifndef __BUZZER_H
#define __BUZZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32l4xx_hal.h"
// 蜂鸣器引脚定义
#define BUZZER_PORT GPIOA
#define BUZZER_PIN  GPIO_PIN_11


// 蜂鸣器初始化
void buzzer_init(void);


#ifdef __cplusplus
}
#endif

#endif /* __BUZZER_H */