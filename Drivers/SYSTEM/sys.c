#include "sys.h"

/**
 * @brief       设置中断向量表偏移地址
 * @param       baseaddr: 基址
 * @param       offset: 偏移量(必须是0, 或者0X100的倍数)
 * @retval      无
 */
void sys_nvic_set_vector_table(uint32_t baseaddr, uint32_t offset)
{
    /* 设置NVIC的向量表偏移寄存器,VTOR低9位保留,即[8:0]保留 */
    SCB->VTOR = baseaddr | (offset & (uint32_t)0xFFFFFE00);
}

/**
 * @brief       执行: WFI指令(执行完该指令进入低功耗状态, 等待中断唤醒)
 * @param       无
 * @retval      无
 */
void sys_wfi_set(void)
{
    __ASM volatile("wfi");
}

/**
 * @brief       关闭所有中断(但是不包括fault和NMI中断)
 * @param       无
 * @retval      无
 */
void sys_intx_disable(void)
{
    __ASM volatile("cpsid i");
}

/**
 * @brief       开启所有中断
 * @param       无
 * @retval      无
 */
void sys_intx_enable(void)
{
    __ASM volatile("cpsie i");
}

/**
 * @brief       设置栈顶地址
 * @note        左侧的红X, 属于MDK误报, 实际是没问题的
 * @param       addr: 栈顶地址
 * @retval      无
 */
void sys_msr_msp(uint32_t addr)
{
    __set_MSP(addr); /* 设置栈顶地址 */
}

/**
 * @brief       进入待机模式
 * @param       无
 * @retval      无
 */
void sys_standby(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();    /* 使能电源时钟 */
    // SET_BIT(PWR->CR, PWR_CR_PDDS); /* 进入待机模式 */
    HAL_PWR_EnterSTANDBYMode(); /* 进入待机模式 */
}

/**
 * @brief       系统软复位
 * @param       无
 * @retval      无
 */
void sys_soft_reset(void)
{
    NVIC_SystemReset();
}

/**
 * @brief       系统时钟初始化函数
 * @param       plln: PLL倍频系数(PLL倍频), 取值范围: 2~20
 *              系统时钟频率 = (8MHz / 1) * plln / 2
                中断向量表位置在启动时已经在SystemInit()中初始化
 * @retval      无
 */
void sys_stm32_clock_init(uint32_t plln)
{
    HAL_StatusTypeDef ret = HAL_ERROR;
    RCC_OscInitTypeDef rcc_osc_init = {0};
    RCC_ClkInitTypeDef rcc_clk_init = {0};

    rcc_osc_init.OscillatorType = RCC_OSCILLATORTYPE_HSE; /* 选择要配置HSE */
    rcc_osc_init.HSEState = RCC_HSE_ON;                   /* 打开HSE */
    rcc_osc_init.PLL.PLLState = RCC_PLL_ON;               /* 打开PLL */
    rcc_osc_init.PLL.PLLSource = RCC_PLLSOURCE_HSE;       /* PLL时钟源选择HSE */
    rcc_osc_init.PLL.PLLM = 1;                            /* VCO输入分频器 */
    rcc_osc_init.PLL.PLLN = plln;                         /* PLL倍频系数，最大支持到20 */
    rcc_osc_init.PLL.PLLR = RCC_PLLR_DIV2;                /* PLLR分频系数 */
    rcc_osc_init.PLL.PLLP = RCC_PLLP_DIV7;                /* PLLP分频系数 */
    rcc_osc_init.PLL.PLLQ = RCC_PLLQ_DIV2;                /* PLLQ分频系数 */
    ret = HAL_RCC_OscConfig(&rcc_osc_init);

    if (ret != HAL_OK)
    {
        while (1)
            ; /* 时钟初始化失败，之后的程序将可能无法正常执行，可以在这里加入自己的处理 */
    }

    /* 选中PLL作为系统时钟源并且配置HCLK,PCLK1和PCLK2*/
    rcc_clk_init.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    rcc_clk_init.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;       /* 设置系统时钟来自PLL */
    rcc_clk_init.AHBCLKDivider = RCC_SYSCLK_DIV1;              /* AHB分频系数为1 */
    rcc_clk_init.APB1CLKDivider = RCC_HCLK_DIV2;               /* APB1分频系数为2 */
    rcc_clk_init.APB2CLKDivider = RCC_HCLK_DIV1;               /* APB2分频系数为1 */
    
    /* 根据频率调整FLASH延时周期，80MHz需要4个等待周期 */
    if (plln > 16) {
        ret = HAL_RCC_ClockConfig(&rcc_clk_init, FLASH_LATENCY_4); /* 设置FLASH延时周期为4WS，也就是5个CPU周期。 */
    } else if (plln > 14) {
        ret = HAL_RCC_ClockConfig(&rcc_clk_init, FLASH_LATENCY_3); /* 设置FLASH延时周期为3WS，也就是4个CPU周期。 */
    } else {
        ret = HAL_RCC_ClockConfig(&rcc_clk_init, FLASH_LATENCY_2); /* 同时设置FLASH延时周期为2WS，也就是3个CPU周期。 */
    }

    if (ret != HAL_OK)
    {
        while (1)
            ; /* 时钟初始化失败，之后的程序将可能无法正常执行，可以在这里加入自己的处理 */
    }

    /* 使能SYSCFG时钟 */
    __HAL_RCC_SYSCFG_CLK_ENABLE();
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
        Error_Handler();
    }

    /** Configure LSE Drive Capability
     */
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.MSIState = RCC_MSI_ON;
    RCC_OscInitStruct.MSICalibrationValue = 0;
    RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
    RCC_OscInitStruct.PLL.PLLM = 1;
    RCC_OscInitStruct.PLL.PLLN = 40;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
    {
        Error_Handler();
    }

    /** Enable MSI Auto calibration
     */
    HAL_RCCEx_EnableMSIPLLMode();
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
        ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

