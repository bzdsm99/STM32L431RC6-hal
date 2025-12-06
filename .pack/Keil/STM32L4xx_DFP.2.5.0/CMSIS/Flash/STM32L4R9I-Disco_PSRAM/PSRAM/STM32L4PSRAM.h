/**
  ******************************************************************************
  * @file    STM32L4PSRAM.h
  * @author  MCD Application Team
  * @brief   Header file of STM32L4PSRAM.c
  *           
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
	
#ifndef __STM32L4PSRAM_H
#define __STM32L4PSRAM_H


/* Includes ------------------------------------------------------------------*/
#include "stm32L4xx_hal.h"
#include "stm32l4r9i_discovery_psram.h"
#include <string.h>
#include "stdint.h"
#define TIMEOUT 5000U



/* Private function prototypes -----------------------------------------------*/
int Init_PSRAM(void);
int Write (uint32_t Address, uint32_t Size, uint8_t* buffer);
static void SystemClock_Config(void);
static void Error_Handler(void);
#endif
