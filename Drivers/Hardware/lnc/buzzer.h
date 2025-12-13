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
void buzzer_play_tone(uint16_t frequency, uint16_t duration, uint8_t volume);

void buzzer_play_twinkle_star(void);
void buzzer_play_wind_rises(void);
void buzzer_play_poweron_sound(void);


#ifdef __cplusplus
}
#endif

#endif /* __BUZZER_H */