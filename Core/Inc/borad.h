#ifndef __BOARD_H
#define __BOARD_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32l4xx_hal.h"
#define KEY_EXIT 1  //是否使用外部终端检测按键

struct gpio_pin
{
    GPIO_TypeDef *GPIOx;
    uint16_t pin;
};

const struct gpio_pin LED = {GPIOC, GPIO_PIN_13};   //为了支持C++
#define LED_R ((struct gpio_pin){GPIOB, GPIO_PIN_5})
#define LED_G ((struct gpio_pin){GPIOA, GPIO_PIN_15})
#define LED_B ((struct gpio_pin){GPIOB, GPIO_PIN_2})
#define KEY_1 ((struct gpio_pin){GPIOB, GPIO_PIN_3})
#define KEY_2 ((struct gpio_pin){GPIOB, GPIO_PIN_4})

#if KEY_EXIT
    #define KEY1_INT_IRQn   EXTI3_IRQn
    #define KEY2_INT_IRQn   EXTI4_IRQn
#endif


void led_init(void);
void led_control(struct gpio_pin led, uint8_t state);
void led_pwm(struct gpio_pin led, uint8_t duty);
uint16_t rgb565_encode(uint8_t r, uint8_t g, uint8_t b);
void led_rgb565(uint16_t rgb565);
void LED_Init(void);    // 独立的LED灯
void key_init(void);
#if (KEY_EXIT==0)
uint8_t key_scan(void);
#endif

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */





