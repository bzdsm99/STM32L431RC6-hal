//freertos.cpp
#include "stm32l4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "LCD_Interface.h"
#include "Matrix_keyboard.h"
#include "usart.h"
#include "delay.h"
#include "timer.h"
#include "adc.h"


//#include "Middlewares\State machine\LCD_Interface_C_API.h"
//FreeRTOS配置为使用静态内存分配时（即 configSUPPORT_STATIC_ALLOCATION 设置为1）
// 添加静态变量以支持空闲任务内存分配
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];


// 定义队列句柄，用于TaskMatrix_keyboard和TaskLed之间的通信
QueueHandle_t xKeyQueue;

// 任务内存分配函数
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
    *ppxIdleTaskStackBuffer = &xIdleStack[0];
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

osThreadId taskDefulatLedHandle;
osThreadId taskMatrix_keyboardHandle;
osThreadId taskLCDRunfuncHandle;


// 任务函数声明
void TaskDefulat(void const * argument);
void TaskMatrix_keyboard(void const * argument);




void MX_FREERTOS_Init(void) {
    Matrix_keyboard_init();

    g_back_color = BLACK;
    g_point_color = WHITE;
    lcd_init();
    // LCD_LockScreen();   // 进入锁屏界面
    led_init();
    LED_Init();
    Timx_baseStart_Init(TIM6,5000 - 1,8000 - 1);    // TIM6 500ms调用一次
    g_back_color = WHITE;
    g_point_color = BLACK;
    lcd_clear(g_back_color);
    delay_ms(20);
    lcd_clear(g_back_color);
    LCD_StartScreen();    // 上电开机界面
    adc_add_channel(ADC_CHANNEL_5,GPIOA,GPIO_PIN_0);    //PA0
    adc_init_channels();
    delay_ms(20);
    // 创建队列，用于按键任务和LED任务之间的通信
    // 队列只存放1个字符，新的值会覆盖旧的值
    xKeyQueue = xQueueCreate(1, sizeof(char));
    if (xKeyQueue == NULL) {
        // 队列创建失败
        printf("Failed to create key queue!\r\n");
    }

    // 创建状态机任务
    osThreadDef(defulat, TaskDefulat, osPriorityNormal, 0, 256);
    taskDefulatLedHandle = osThreadCreate(osThread(defulat), NULL);

    // 创建矩阵键盘任务
    osThreadDef(Matrix_keyboard, TaskMatrix_keyboard, osPriorityNormal, 0, 128);
    taskMatrix_keyboardHandle = osThreadCreate(osThread(Matrix_keyboard), NULL);

    // 创建LCD子任务
    osThreadDef(LCD_func,TaskLCDRunfunc, osPriorityNormal, 0, 256);
    taskLCDRunfuncHandle = osThreadCreate(osThread(LCD_func), NULL);
}


void TaskDefulat(void const * argument)
{
    char keyValue = 0;
    portENTER_CRITICAL();       // 进入临界区
    printf("LED Task Started\r\n");
    portEXIT_CRITICAL();        // 退出临界区

    for (;;)
    {

        // 从队列接收按键值，使用较短的超时时间以提高响应性
        if (xQueueReceive(xKeyQueue, &keyValue, pdMS_TO_TICKS(10)) == pdTRUE) {
            // 成功接收到按键值，可以通过串口打印或其他方式处理
            printf("Received key: %c\r\n", keyValue);
            LCD_RuningScreen(keyValue);
        }


        // 使用vTaskDelay替代delay_ms以符合FreeRTOS规范
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}


void TaskMatrix_keyboard(void const * argument)
{
    char key = 0;

    vTaskSuspendAll();      //挂起调度器
    printf("Matrix keyboard Task Started\r\n");
    xTaskResumeAll();       //恢复调度器
    
    for (;;)
    {
        key = Matrix_keyboard_scan();
        if(key != 0) {
            // 使用队列将按键值发送给LED任务
            // 如果队列已满，先取出旧值再放入新值，确保总是保存最新的按键值
            if (uxQueueSpacesAvailable(xKeyQueue) == 0) {
                // 队列已满，先取出旧值
                char dummy;
                xQueueReceive(xKeyQueue, &dummy, 0);
            }
            // 将新值放入队列
            xQueueSend(xKeyQueue, &key, 0);
        }
        
        // 使用vTaskDelay替代osDelay以符合FreeRTOS规范
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}