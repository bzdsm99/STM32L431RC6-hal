// dac.h
#ifndef __DAC_H
#define __DAC_H

#include "stm32l4xx_hal.h"

extern DAC_HandleTypeDef hdac;

void dac_init(void);
void dac_set_voltage_ch1(float voltage);
void dac_set_voltage_ch2(float voltage);

#endif

