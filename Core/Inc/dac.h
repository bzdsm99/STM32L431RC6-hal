// dac.h
#ifndef __DAC_H
#define __DAC_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32l4xx_hal.h"

extern DAC_HandleTypeDef hdac;

void dac_init(void);
void dac_set_voltage_ch1(float voltage);
void dac_set_voltage_ch2(float voltage);

#ifdef __cplusplus
}
#endif
#endif
