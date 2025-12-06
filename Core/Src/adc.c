#include "adc.h"
#include "sys.h"
#include <stdarg.h>

// 存储ADC句柄的全局变量
ADC_HandleTypeDef hadc1;

// 静态全局变量，用于存储ADC通道信息
static adc_channel_info_t g_adc_channels[ADC_MAX_CHANNELS]; //可以尽量的节省内存空间
static uint8_t g_adc_channel_count = 0;

/**
 * @brief  添加ADC通道配置
 * @param  channel: ADC通道号
 * @param  port: GPIO端口
 * @param  pin: GPIO引脚号
 * 
 * @arags:      PA0通道 (ADC_CHANNEL_5)
 * @arags:      PA1通道 (ADC_CHANNEL_6)
 * @arags:      PA2通道 (ADC_CHANNEL_7)
 * @arags:      PA3通道 (ADC_CHANNEL_8)
 * @arags:      PA4通道 (ADC_CHANNEL_9)
 * @arags:      PA5通道 (ADC_CHANNEL_10)
 * @arags:      PA6通道 (ADC_CHANNEL_11)
 * @arags:      PA7通道 (ADC_CHANNEL_12)
 * @arags:      PB0通道 (ADC_CHANNEL_15)
 * @arags:      PB1通道 (ADC_CHANNEL_16)
 * @arags:      PC0通道 (ADC_CHANNEL_1)
 * @arags:      PC1通道 (ADC_CHANNEL_2)
 * @arags:      PC2通道 (ADC_CHANNEL_3)
 * @arags:      PC3通道 (ADC_CHANNEL_4)
 * @retval      None
 */
void adc_add_channel(uint32_t channel, GPIO_TypeDef* port, uint16_t pin) 
{
    if (g_adc_channel_count < ADC_MAX_CHANNELS) {
        g_adc_channels[g_adc_channel_count].channel = channel;
        g_adc_channels[g_adc_channel_count].pin_info.GPIOx = port;
        g_adc_channels[g_adc_channel_count].pin_info.pin = pin;
        g_adc_channel_count++;
    }
}

/**
 * @brief  初始化所有已添加的ADC通道
 * @retval 内部参考电压ADC
 */
uint32_t adc_init_channels(void)
{
    // 初始化ADC句柄
    hadc1.Instance = ADC1;                                    // 选择ADC1作为转换模块
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;     // ADC时钟预分频器配置: PCLK/4
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;               // ADC分辨率配置: 12位分辨率
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;               // ADC数据对齐方式: 右对齐
    hadc1.Init.ScanConvMode = (g_adc_channel_count > 1) ? ENABLE : DISABLE; // ADC扫描转换模式: 多通道时启用
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;            // ADC转换结束选择: 单次转换结束后产生EOC标志
    hadc1.Init.LowPowerAutoWait = DISABLE;                    // ADC低功耗自动等待: 禁用自动等待模式
    hadc1.Init.ContinuousConvMode = DISABLE;                  // ADC连续转换模式: 禁用连续转换模式（单次转换模式）
    hadc1.Init.NbrOfConversion = g_adc_channel_count;         // ADC转换通道数
    hadc1.Init.DiscontinuousConvMode = DISABLE;               // ADC间断模式: 禁用间断模式
    hadc1.Init.NbrOfDiscConversion = 1;                       // ADC间断模式通道数
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;         // ADC外部触发转换源: 软件触发启动转换
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; // ADC外部触发转换边沿: 无外部触发边沿
    hadc1.Init.DMAContinuousRequests = DISABLE;               // ADC DMA连续请求: 禁用DMA连续请求
    hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;              // ADC数据溢出处理: 数据溢出时保留原有数据
    hadc1.Init.OversamplingMode = DISABLE;                    // ADC过采样模式: 禁用过采样模式
    
    // 初始化ADC
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        // 初始化错误处理
        Error_Handler();
    }

    // 配置所有已添加的通道
    for (uint8_t i = 0; i < g_adc_channel_count; i++)
    {
        ADC_ChannelConfTypeDef sConfig = {0};
        
        sConfig.Channel = g_adc_channels[i].channel;
        sConfig.Rank = ADC_REGULAR_RANK_1 + i;
        sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5; // 增加采样时间以提高精度
        sConfig.SingleDiff = ADC_SINGLE_ENDED;
        sConfig.OffsetNumber = ADC_OFFSET_NONE;
        
        if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
        {
            // 通道配置错误处理
            Error_Handler();
        }
    }

    // 在ADC初始化后添加参考电压校准
    return HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
}

/**
 * @brief  ADC MSP初始化回调函数
 * @param  hadc: ADC句柄
 * @retval None
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    if(hadc->Instance==ADC1)
    {
        __HAL_RCC_ADC_CLK_ENABLE();
        
        // 根据已添加的通道信息初始化对应的GPIO引脚
        // 先统计使用的GPIO端口
        uint8_t gpio_a_used = 0, gpio_b_used = 0, gpio_c_used = 0;
        uint32_t gpio_a_pins = 0, gpio_b_pins = 0, gpio_c_pins = 0;
        
        for (uint8_t i = 0; i < g_adc_channel_count; i++) {
            GPIO_TypeDef* port = g_adc_channels[i].pin_info.GPIOx;
            uint16_t pin = g_adc_channels[i].pin_info.pin;
            
            if (port == GPIOA) {
                gpio_a_used = 1;
                gpio_a_pins |= pin;
            } else if (port == GPIOB) {
                gpio_b_used = 1;
                gpio_b_pins |= pin;
            } else if (port == GPIOC) {
                gpio_c_used = 1;
                gpio_c_pins |= pin;
            }
        }
        
        // 使能使用的GPIO端口时钟
        if (gpio_a_used) {
            __HAL_RCC_GPIOA_CLK_ENABLE();
        }
        if (gpio_b_used) {
            __HAL_RCC_GPIOB_CLK_ENABLE();
        }
        if (gpio_c_used) {
            __HAL_RCC_GPIOC_CLK_ENABLE();
        }
        
        // 初始化所有ADC引脚为模拟模式
        if (gpio_a_pins) {
            GPIO_InitStruct.Pin = gpio_a_pins;
            GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        }
        
        if (gpio_b_pins) {
            GPIO_InitStruct.Pin = gpio_b_pins;
            GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
        }
        
        if (gpio_c_pins) {
            GPIO_InitStruct.Pin = gpio_c_pins;
            GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
            GPIO_InitStruct.Pull = GPIO_NOPULL;
            HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        }
    }
}

/**
 * @brief  ADC MSP反初始化回调函数
 * @param  hadc: ADC句柄
 * @retval None
 */
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance==ADC1)
    {
        /* 关闭ADC时钟 */
        __HAL_RCC_ADC_CLK_DISABLE();
        
        // 反初始化所有ADC引脚
        for (uint8_t i = 0; i < g_adc_channel_count; i++) {
            HAL_GPIO_DeInit(g_adc_channels[i].pin_info.GPIOx, g_adc_channels[i].pin_info.pin);
        }
    }
}

/**
 * @brief  获取指定通道的ADC转换值
 * @param  channel: ADC通道
 * @retval 转换结果
 */
uint32_t adc_get_value(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};
    
    sConfig.Channel = channel;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5; // 增加采样时间以提高精度
    sConfig.SingleDiff = ADC_SINGLE_ENDED;
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
    
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100); // 设置超时时间为100ms
    
    uint32_t result = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    
    return result;
}

/**
 * @brief  获取指定通道多次采样的平均值
 * @param  channel: ADC通道
 * @param  times: 采样次数
 * @retval 平均转换结果
 */
uint32_t adc_get_average_value(uint32_t channel, uint8_t times)
{
    uint32_t sum = 0;
    
    for(uint8_t i = 0; i < times; i++)
    {
        sum += adc_get_value(channel);
        HAL_Delay(5); // 添加小延时确保采样稳定
    }
    
    return sum / times;
}

#define ADC_MAX_SAMPLES 15

/**
 * @brief  中值滤波法
 * @param  channel: ADC通道
 * @param  times: 采样次数（建议为奇数）
 * @retval 中值转换结果
 */
uint32_t adc_get_median_value(uint32_t channel, uint8_t times)
{
    uint32_t samples[ADC_MAX_SAMPLES]; // 存储采样值
    uint8_t i, j;
    uint32_t temp;

    // 确保采样次数不超过最大限制
    if (times > ADC_MAX_SAMPLES) {
        times = ADC_MAX_SAMPLES;
    }

    // 获取多次采样值
    for (i = 0; i < times; i++) {
        samples[i] = adc_get_value(channel);
        HAL_Delay(5); // 添加小延时确保采样稳定
    }

    // 对采样值进行排序（冒泡排序）
    for (i = 0; i < times - 1; i++) {
        for (j = 0; j < times - i - 1; j++) {
            if (samples[j] > samples[j + 1]) {
                temp = samples[j];
                samples[j] = samples[j + 1];
                samples[j + 1] = temp;
            }
        }
    }

    // 返回中值
    return samples[times / 2];
}

/**
 * @brief  指数加权移动平均法
 * @param  channel: ADC通道
 * @param  last_ewma_value: 上一次的EWMA值
 * @param  alpha: 平滑系数（范围：0~1，通常为0.1~0.3）
 * @retval 滤波后的转换结果
 */
uint32_t adc_get_ewma_value(uint32_t channel, uint32_t last_ewma_value, float alpha)
{
    uint32_t current_value = adc_get_value(channel);

    // 计算指数加权移动平均值
    uint32_t ewma_value = (uint32_t)(alpha * current_value + (1 - alpha) * last_ewma_value);

    return ewma_value;
}


