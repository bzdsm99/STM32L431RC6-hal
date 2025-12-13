#include "cmsis_os.h"
#include "sys.h"
#include "usart.h"
#include "LCD.h"

void MX_FREERTOS_Init(void);

int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(20);
    usart_init(115200);
    lcd_init();
    MX_FREERTOS_Init();
    osKernelStart();
    while (1)
    {

    }
}   


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

#ifdef  USE_FULL_ASSERT
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



