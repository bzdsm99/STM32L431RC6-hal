// dac.c
#include "dac.h"
#include "stm32l4xx_hal.h"

DAC_HandleTypeDef hdac;


/**
 * @brief  初始化DAC外设
 * @retval None
 */
void dac_init(void)
{
    __HAL_RCC_DAC1_CLK_ENABLE();
    // 初始化DAC句柄
    hdac.Instance = DAC;
    if (HAL_DAC_Init(&hdac) != HAL_OK)
    {

    }

    // 配置DAC通道1 (PA4)
    DAC_ChannelConfTypeDef sConfig = {0};
    sConfig.DAC_Trigger = DAC_TRIGGER_NONE; // 软件触发模式
    sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
    if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
    {
        // printf("DAC通道1配置失败！");
    }

    // 配置DAC通道2 (PA5)
    if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_2) != HAL_OK)
    {
        // printf("DAC通道2配置失败！");
    }

    // 初始化GPIO引脚为模拟输出模式
    __HAL_RCC_GPIOA_CLK_ENABLE(); // 使能GPIOA时钟
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_4; //PA4
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL; // 模拟输出模式
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_5; //PA5
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 启动DAC通道1和通道2
    if (HAL_DAC_Start(&hdac, DAC_CHANNEL_1) != HAL_OK)
    {
        // printf("DAC通道1启动失败！");
    }
    if (HAL_DAC_Start(&hdac, DAC_CHANNEL_2) != HAL_OK)
    {
        // printf("DAC通道2启动失败！");
    }
}

/**
 * @brief  设置DAC通道1的输出电压 PA4
 * @param  value: 输出值 (0 ~ 4095)
 * @retval None
 */
void dac_set_voltage_ch1(float voltage) {
    // 将电压转换为DAC值（假设参考电压为3.3V，12位分辨率）
    uint16_t dac_value = (uint16_t)((voltage / 3.3f) * 4095);

    // 设置DAC通道1输出值
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_value);
}

/**
 * @brief  设置DAC通道2的输出电压   PA5
 * @param  value: 输出值 (0 ~ 4095)
 * @retval None
 */
void dac_set_voltage_ch2(float voltage) {
    // 将电压转换为DAC值（假设参考电压为3.3V，12位分辨率）
    uint16_t dac_value = (uint16_t)((voltage / 3.3f) * 4095);

    // 设置DAC通道2输出值
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, dac_value);
}
