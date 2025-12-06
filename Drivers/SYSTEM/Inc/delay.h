#ifndef __DELAY_H
#define __DELAY_H

#include "stdint.h"
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"

void delay_init(void);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

#endif /* __DELAY_H */