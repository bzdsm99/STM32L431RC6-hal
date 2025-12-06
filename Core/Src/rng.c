#include "rng.h"
#include "delay.h"

RNG_HandleTypeDef RNG_Handle;

/**
  * @brief 硬件真随机数外设初始化
  * @param 无 
  * @retval 0:初始化成功 1:初始化失败
  */
uint8_t rng_init(void)
{
    uint16_t retry = 0;
    RNG_Handle.Instance = RNG;
    HAL_RNG_Init(&RNG_Handle);

    while((__HAL_RNG_GET_FLAG(&RNG_Handle, RNG_FLAG_DRDY) == RESET)
        && (retry < 10000))
    {
        retry++;
        delay_us(10);
    }

    if(retry >= 10000)
    {
        return 1;
    }

    return 0;
}

/**
  * @brief RNG外设回调函数
  * @param 无 
  * @retval 无
  */
void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng)
{
    // RNG时钟源选择为PLL
    RCC_PeriphCLKInitTypeDef rcc_periphclk_init_struct = {0};
    // RNG时钟源使能
    __HAL_RCC_RNG_CLK_ENABLE();
    rcc_periphclk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_RNG;
    rcc_periphclk_init_struct.RngClockSelection = RCC_RNGCLKSOURCE_PLL; //PLL (Phase Locked Loop)锁相环
    HAL_RCCEx_PeriphCLKConfig(&rcc_periphclk_init_struct);
}

/**
  * @brief 获取随机数
  * @param 无 
  * @retval 随机数
  */
uint32_t rng_get_random_num(void)
{
    uint32_t random_number = 0;
    HAL_RNG_GenerateRandomNumber(&RNG_Handle, &random_number);
    return random_number;
}

/**
  * @brief 获取随机数范围
  * @param min : 最小值
  * @param max : 最大值 
  * @retval 随机数 [min, max]
  */
uint32_t rng_get_random_range(int min, int max)
{
    return (rng_get_random_num() % (max - min + 1)) + min;
}


