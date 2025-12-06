/**
  ******************************************************************************
  * @file    STM32L4OSPI.c
  * @author  MCD Application Team
  * @brief   This file defines the operations of the external loader for
  *          MX25LM51245G OSPI memory of STM32L4P5G-DK.
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

#include "STM32L4OSPI.h"
#include <string.h>





/* Private functions ---------------------------------------------------------*/

/** @defgroup STM32L4P5G_Discovery_OSPI_Private_Functions Private Functions
  * @{
  */

void HAL_Delay(uint32_t Delay)
{
  int i=0;
  for (i=0; i<0x1000; i++);
}
 uint32_t HAL_GetTick(void)
{
  int i;
  for (i=0;i<1024;i++);
  uwTick++;
  return uwTick;
}
/** @defgroup STM32L4P5G_Discovery_OSPI_Exported_Functions Exported Functions
  * @{
  */

 /**
  * @brief  Initializes the OSPI interface.
  * @param  Instance   OSPI Instance
  * @param  Init       OSPI Init structure
  * @retval BSP status
  */
 HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{ 
  return HAL_OK;
}
/**
  * @brief  System initialization.
  * @param  None
  * @retval  1      : Operation succeeded
  * @retval  0      : Operation failed
  */
int Init_OSPI()
{   int32_t result=0;


  /* Zero Init structs */
 
  SystemInit(); 
	   HAL_Init();  
	  /* Configure the system clock  */
   SystemClock_Config();
  
/*Initialaize OSPI*/
   if(BSP_OSPI_NOR_Init() !=0)
    return 0;
/*Configure the OSPI in memory-mapped mode*/
   result=BSP_OSPI_NOR_EnableMemoryMappedMode();  

	 if(result==0)
    return 1; 

   return 0;
}

/**
  * @brief   erase memory.
  * @retval  1      : Operation succeeded
  * @retval  0      : Operation failed
  */

 int MassErase (void){
 /*Initialaize OSPI*/
	 BSP_OSPI_NOR_DeInit();
   BSP_OSPI_NOR_Init();
	
	 /*Erases the entire OSPI memory*/
	 BSP_OSPI_NOR_Erase_Chip();

  /*Reads current status of the OSPI memory*/
	while (BSP_OSPI_NOR_GetStatus()!=0){};

	 return 0;
 
 }
 
 /**
  * @brief   Program memory.
  * @param   Address: page address
  * @param   Size   : size of data
  * @param   buffer : pointer to data buffer
  * @retval  1      : Operation succeeded
  * @retval  0      : Operation failed
  */
 int Write (uint32_t Address, uint32_t Size, uint8_t* buffer)
{
    /*Initialaize OSPI*/
    BSP_OSPI_NOR_DeInit();
    Address = Address & 0x0fffffff;
   BSP_OSPI_NOR_Init();
   
	 /*Writes data to the OSPI memory*/
    BSP_OSPI_NOR_Write(buffer,Address, Size);
  
   return 1;
}


/**
  * @brief   Sector erase.
  * @param   EraseStartAddress :  erase start address
  * @param   EraseEndAddress   :  erase end address
  * @retval  None
  */
 int SectorErase (uint32_t EraseStartAddress ,uint32_t EraseEndAddress)
{
	
  uint32_t BlockAddr;
  EraseStartAddress &= 0x0FFFFFFF;  
  EraseEndAddress &= 0x0FFFFFFF;
  EraseStartAddress = EraseStartAddress -  EraseStartAddress % 0x10000;
   /*Initialaize OSPI*/
	BSP_OSPI_NOR_DeInit();
  BSP_OSPI_NOR_Init();	
	
	  while (EraseEndAddress>=EraseStartAddress)
  {
    BlockAddr = EraseStartAddress;
  /*Erases the specified block of the OSPI memory*/
  BSP_OSPI_NOR_Erase_Block(BlockAddr);
 
   /*Reads current status of the OSPI memory*/
		while (BSP_OSPI_NOR_GetStatus()!=0);

		EraseStartAddress+=0x10000;
		/*Configure the OSPI in memory-mapped mode*/
    BSP_OSPI_NOR_EnableMemoryMappedMode();
  }

 
  return 1;	
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follows :
  *            System Clock source            = PLL (MSI)
  *            SYSCLK(Hz)                     = 110000000
  *            HCLK(Hz)                       = 110000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 1
  *            APB2 Prescaler                 = 1
  *            MSI Frequency(Hz)              = 4000000
  *            PLL_M                          = 1
  *            PLL_N                          = 55
  *            PLL_Q                          = 2
  *            PLL_R                          = 2
  *            PLL_P                          = 7
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  /* Enable voltage range 1 boost mode for frequency above 80 Mhz */
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);
  __HAL_RCC_PWR_CLK_DISABLE();

  /* Enable MSI Oscillator and activate PLL with MSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 55;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLP = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
   
    while(1);
  }

  /* To avoid undershoot due to maximum frequency, select PLL as system clock source */
  /* with AHB prescaler divider 2 as first step */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    while(1);
  }

  /* AHB prescaler divider at 1 as second step */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    
    while(1);
  }
}





/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
