#include "buzzer.h"
#include "timer.h"
#include "sys.h"
#include "gpio.h"

/**
 * @brief 初始化蜂鸣器
 */
void buzzer_init(void) {
    // 初始化蜂鸣器引脚为普通输出模式
    GPIO_InitTypeDef gpio_init_struct;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    gpio_init_struct.Pin = BUZZER_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;  // 推挽输出
    gpio_init_struct.Pull = GPIO_PULLUP;
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(BUZZER_PORT, &gpio_init_struct);
    
    // 初始状态为高电平（不响）
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
}



