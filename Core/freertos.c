#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "delay.h"
#include "LCD.h"

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
    delay_init();
    lcd_init();
    lcd_clear(YELLOW);
    for(;;)
    {
        osDelay(1);
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

