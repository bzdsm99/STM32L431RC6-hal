#ifndef __ADC_H
#define __ADC_H
// STM32L431RCT6该芯片只有一个ADC（ADC1）

#include "gpio.h"

// 最大支持的ADC通道数
#define ADC_MAX_CHANNELS 16

// ADC通道信息结构体
typedef struct {
    uint32_t channel;           // ADC通道号
    struct gpio_pin pin_info;   // 对应的GPIO引脚信息
} adc_channel_info_t;

extern ADC_HandleTypeDef hadc1;

// 新增函数，用于更灵活的ADC管理
void adc_add_channel(uint32_t channel, GPIO_TypeDef* port, uint16_t pin);
uint32_t adc_init_channels(void);
uint32_t adc_get_value(uint32_t channel);
uint32_t adc_get_average_value(uint32_t channel, uint8_t times);
uint32_t adc_get_median_value(uint32_t channel, uint8_t times);
uint32_t adc_get_ewma_value(uint32_t channel, uint32_t last_ewma_value, float alpha);

#endif

