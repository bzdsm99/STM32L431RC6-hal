/**
  ******************************************************************************
  * @file    STM32L4OSPI.h
  * @author  MCD Application Team
  * @brief   Header file of STM32L4OSPI.c
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


#ifndef __STM32L4OSPI_H
#define __STM32L4OSPI_H
#include "stdint.h"
/* Includes ------------------------------------------------------------------*/
#include "stm32L4xx_hal.h"
#include "stm32l4r9i_discovery_ospi_nor.h"
#include "./mx25lm51245g/mx25lm51245g.h"

#define TIMEOUT 5000U


/* Private function prototypes -----------------------------------------------*/
int Init_OSPI(void);
 int Write (uint32_t Address, uint32_t Size, uint8_t* buffer);
 int SectorErase (uint32_t EraseStartAddress ,uint32_t EraseEndAddress);
 int MassErase ( void);
void SystemClock_Config(void);

#endif
