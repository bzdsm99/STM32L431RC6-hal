#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

struct gpio_pin
{
    GPIO_TypeDef *GPIOx;
    uint16_t pin;
};

void MX_GPIO_Init(void);



#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

