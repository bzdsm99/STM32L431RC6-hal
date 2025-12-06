/**
  ******************************************************************************
  * @file    STM32L4QSPI.c
  * @author  MCD Application Team
  * @brief   This file defines the operations of the external loader for
  *          N25Q128A QSPI memory of STM32L476G-DK.
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

#include "STM32L4QSPI.h"




  QSPI_CommandTypeDef      sCommand;
  QSPI_MemoryMappedTypeDef sMemMappedCfg;
 



/* Private functions ---------------------------------------------------------*/

/** @defgroup STM32L476G_Disco_QSPI_Private_Functions Private Functions
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
/** @defgroup STM32L476G_Disco_QSPI_Exported_Functions Exported Functions
  * @{
  */
 
/**
  * @brief  Initializes the QSPI interface.
  * @param  Instance   QSPI Instance
  * @param  Init       QSPI Init structure
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
int Init_QSPI()
{
	int32_t result=0;
  /* Zero Init structs */
  HAL_Init();
  SystemInit(); 
	
  /* Configure the system clock  */
   SystemClock_Config();

/*Initialaize QSPI*/
   if(BSP_QSPI_Init() !=0)
    return 0;
	 
  /*Configure the QSPI in memory-mapped mode*/
   result=BSP_QSPI_EnableMemoryMappedMode();  
  
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
    
	 /*Initialaize QSPI*/
    BSP_QSPI_Init();
	 /*Erases the entire QSPI memory*/
	  BSP_QSPI_Erase_Chip();
	 /* Reads current status of the QSPI memory*/
    while (BSP_QSPI_GetStatus()!=0){};

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
     Address = Address & 0x0fffffff;
	/*Initialaize QSPI*/
     BSP_QSPI_Init();
	/* Writes an amount of data to the QSPI memory*/
     BSP_QSPI_Write(buffer,Address, Size);
  
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

	/*Initialaize QSPI*/
     BSP_QSPI_Init();
	
	  while (EraseEndAddress>=EraseStartAddress)
  {
    BlockAddr = EraseStartAddress>>16;
    /*Erases the specified sector of the QSPI memory*/
    BSP_QSPI_Erase_Sector(BlockAddr );
  /* Reads current status of the QSPI memory*/
	  while (BSP_QSPI_GetStatus()!=0){};
 
		EraseStartAddress+=0x10000;
   
  }
	/*Configure the QSPI in memory-mapped mode*/
   BSP_QSPI_EnableMemoryMappedMode();
 
  return 1;	
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 400000000 (Cortex-M7 CPU Clock)
  *            HCLK(Hz)                       = 200000000 (Cortex-M4 CPU, Bus matrix Clocks)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 5
  *            PLL_N                          = 160
  *            PLL_P                          = 2
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */

static int SystemClock_Config(void)
{
RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  /* MSI is enabled after System reset, activate PLL with MSI as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLP = 7;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  
  if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }
return 1;

}





/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
