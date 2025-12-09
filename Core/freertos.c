#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "delay.h"
#include "LCD.h"
#include "dac.h"
#include "stdbool.h"

osThreadId defaultTaskHandle;
osThreadId ledToggleTaskHandle;


void StartDefaultTask(void const * argument);
void StartLed(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = &xIdleStack[0];
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}



void MX_FREERTOS_Init(void) {
    osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
    defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

    osThreadDef(ledToggleTask, StartLed, osPriorityNormal, 0, 128);
    ledToggleTaskHandle = osThreadCreate(osThread(ledToggleTask), NULL);

}


void StartDefaultTask(void const * argument)
{
    static float voltage = 0.0f;  // 输出电压
    static bool increasing = true; // 标志位：电压是否在增加

    lcd_clear(YELLOW);
    g_back_color = YELLOW;

    for (;;)
    {
        // 设置 DAC 输出电压
        dac_set_voltage_ch2(voltage);

        // 在 LCD 上显示当前电压
        lcd_printf(1, 1, LCD_FONT_16, "PA5: %.2f V    ", voltage);

        // 根据标志位调整电压
        if (increasing)
        {
            voltage += 0.05f; // 增加电压
            if (voltage >= 3.3f)
            {
                voltage = 3.3f; // 防止超出上限
                increasing = false; // 切换为递减模式
            }
        }
        else
        {
            voltage -= 0.05f; // 减少电压
            if (voltage <= 0.0f)
            {
                voltage = 0.0f; // 防止低于下限
                increasing = true; // 切换为递增模式
            }
        }
    }
}




void StartLed(void const * argument)
{

    for(;;)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        delay_ms(500);
    }

}

