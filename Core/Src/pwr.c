#include "pwr.h"
#include "usart.h"

/**
 * @brief       初始化PVD电压监视器
 * @param       pls: 电压等级(PWR_PVD_detection_level)
 *   @arg       PWR_PVDLEVEL_0,2.2V;
 *   @arg       PWR_PVDLEVEL_1,2.3V;
 *   @arg       PWR_PVDLEVEL_2,2.4V;
 *   @arg       PWR_PVDLEVEL_3,2.5V;
 *   @arg       PWR_PVDLEVEL_4,2.6V;
 *   @arg       PWR_PVDLEVEL_5,2.7V;
 *   @arg       PWR_PVDLEVEL_6,2.8V;
 *   @arg       PWR_PVDLEVEL_7,2.9V;
 * @retval      无
 */
void pwr_pvd_init(uint32_t pls)
{
    PWR_PVDTypeDef pwr_pvd = {0};

    __HAL_RCC_PWR_CLK_ENABLE();                      /* 使能PWR时钟 */
    
    pwr_pvd.PVDLevel = pls;                          /* 检测电压级别 */
    pwr_pvd.Mode = PWR_PVD_MODE_IT_RISING_FALLING;   /* 使用中断线的上升沿和下降沿双边缘触发 */
    HAL_PWR_ConfigPVD(&pwr_pvd);

    HAL_NVIC_SetPriority(PVD_PVM_IRQn, 2 ,2);
    HAL_NVIC_EnableIRQ(PVD_PVM_IRQn);
    HAL_PWR_EnablePVD();                             /* 使能PVD检测 */

    // 启用硬件滤波（如果支持）
    __HAL_PWR_PVD_EXTI_ENABLE_RISING_FALLING_EDGE();
    __HAL_PWR_PVD_EXTI_ENABLE_IT();
}

/**
 * @brief       PVD中断服务函数
 * @param       无
 * @retval      无
 */
void PVD_PVM_IRQHandler(void)
{
    HAL_PWREx_PVD_PVM_IRQHandler();
}

/**
 * @brief       PVD中断服务回调函数
 * @param       无
 * @retval      无
 */
void HAL_PWR_PVDCallback(void)
{
    if (__HAL_PWR_GET_FLAG(PWR_FLAG_PVDO)) /* 电压比PLS所选电压还低 */
    {
        printf("电压过低.\r\n");
    }
    else    //电压正常
    {
        printf("电压正常.\r\n");
    }
}

/*********************************** 睡眠模式 *******************************************/

/**
 * @brief       低功耗模式下的按键初始化(用于唤醒睡眠模式/停止模式)
 * @param       无
 * @retval      无
 */
void pwr_wkup_key_init(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    PWR_WKUP_GPIO_CLK_ENABLE();                           /* WKUP时钟使能 */

    gpio_init_struct.Pin = PWR_WKUP_GPIO_PIN;             /* WKUP引脚 */
    gpio_init_struct.Mode = GPIO_MODE_IT_FALLING;         /* 中断 */
    gpio_init_struct.Pull = GPIO_PULLUP;                  /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;        /* 高速 */
    HAL_GPIO_Init(PWR_WKUP_GPIO_PORT, &gpio_init_struct); /* WKUP引脚初始化 */

    HAL_NVIC_SetPriority(PWR_WKUP_INT_IRQn, 0, 0);        /* 抢占优先级0，子优先级0 */
    HAL_NVIC_EnableIRQ(PWR_WKUP_INT_IRQn);
}

/**
 * @brief       WK_UP按键 外部中断服务程序
 * @param       无
 * @retval      无
 */
void PWR_WKUP_INT_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(PWR_WKUP_GPIO_PIN);
}

/**
 * @brief       外部中断回调函数
 * @param       GPIO_Pin:中断线引脚
 * @note        此函数会被PWR_WKUP_INT_IRQHandler()调用
 * @retval      无
 */
// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
// {
//     printf("HAL_GPIO_EXTI_Callback() Called.\r\n");
//     if (GPIO_Pin == PWR_WKUP_GPIO_PIN)
//     {
//         /* HAL_GPIO_EXTI_IRQHandler()函数已经为我们清除了中断标志位，所以我们进了回调函数可以不做任何事 */
//         printf("唤醒,已重新开启时钟.\r\n");
//         __HAL_GPIO_EXTI_CLEAR_FLAG(PWR_WKUP_GPIO_PIN);
//     }
// }



/*********************************** 睡眠模式 *******************************************/

/**
 * @brief       进入CPU睡眠模式
 * @param       无
 * @note        设置稳压器为低功耗模式，等待中断唤醒
 * @retval      无
 */
void pwr_enter_sleep(void)
{
    printf("正在进入睡眠模式...\r\n");

    /* 清除可能存在的挂起中断 */
    __HAL_GPIO_EXTI_CLEAR_FLAG(PWR_WKUP_GPIO_PIN);

    /* 显式启用WKUP引脚唤醒功能 */
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    /* 进入睡眠模式前确保所有操作完成 */
    __DSB();
    __ISB();

    HAL_SuspendTick();  /* 暂停滴答时钟，防止通过滴答时钟中断唤醒 */

    /* 执行WFI指令, 进入睡眠模式 */
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);

    HAL_ResumeTick();   /* 恢复滴答时钟 */

    printf("已从睡眠模式唤醒\r\n");
}

/*********************************** 停止模式 *******************************************/

/**
 * @brief       进入停止模式
 * @param       无
 * @note        设置稳压器为低功耗模式，等待中断唤醒
 *              恢复后请重新开始时钟
 * @retval      无
 */
void pwr_enter_stop(void)
{
    printf("进入停止模式...\r\n");
    
    __HAL_RCC_PWR_CLK_ENABLE();     /* 使能电源时钟 */
    
    /* 配置唤醒引脚 */
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    
    /* 确保所有操作完成 */
    __DSB();
    __ISB();
    
    HAL_SuspendTick();  /* 暂停滴答时钟 */
    
    /* 进入停止模式，设置稳压器为低功耗模式，等待中断唤醒 */
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    
    /* 停止模式唤醒后需要重新配置系统时钟 */
    SystemClock_Config();
    
    HAL_ResumeTick();   /* 恢复滴答时钟 */
    
    printf("从停止模式唤醒\r\n");
}

/*********************************** 待机模式 *******************************************/

/**
 * @brief       进入待机模式
 * @param       无
 * @note        关闭时钟,关闭稳压器
 *              进入待机模式后通过 WKUP1(PA0)引脚上升沿或下降沿触发唤醒
 *              恢复后请重新开始时钟
 * @retval      无
 */
void pwr_enter_standby(void)
{
    printf("正在进入待机模式...\r\n");
    
    __HAL_RCC_PWR_CLK_ENABLE();               /* 使能电源时钟 */

    /* 清除所有挂起的中断 */
    NVIC_ClearPendingIRQ(PWR_WKUP_INT_IRQn);
    __HAL_GPIO_EXTI_CLEAR_FLAG(PWR_WKUP_GPIO_PIN);
    
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1); /* 使能KEY_UP引脚的唤醒功能 */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);        /* 需要清此标记，否则将保持唤醒状态 */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);        /* 清除待机标志 */
    
    /* 确保所有内存访问完成 */
    __DSB();
    __ISB();
    
    HAL_Delay(10);  /* 短暂延迟确保配置生效 */
    
    HAL_SuspendTick();                        /* 暂停滴答时钟，防止通过滴答时钟中断唤醒 */
    
    printf("执行待机指令...\r\n");
    HAL_PWR_EnterSTANDBYMode();               /* 进入待机模式 */
    
    /* 待机模式唤醒后会系统复位，不会执行到这里 */
    HAL_ResumeTick();
}


