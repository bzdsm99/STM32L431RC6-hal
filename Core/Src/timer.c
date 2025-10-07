//timer.c
#include "timer.h"

TIM_HandleTypeDef timx_Handles[13] = {0};


void Timx_Init(TIM_TypeDef *Timx, uint16_t arr, uint16_t psc)
{
    uint8_t key;
    switch ((uint32_t)Timx)
    {
        case (uint32_t)TIM1:
            key = 1;
            break;
        case (uint32_t)TIM6:
            key = 6;
            break;
        case (uint32_t)TIM7:
            key = 7;
            break;
    }
    
    timx_Handles[key].Instance = Timx;
    timx_Handles[key].Init.Prescaler = psc;                          /* 设置预分频系数 */
    timx_Handles[key].Init.CounterMode = TIM_COUNTERMODE_UP;         /* 递增计数模式 */
    timx_Handles[key].Init.Period = arr;
    timx_Handles[key].Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;   /* 时钟分频 */
    timx_Handles[key].Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; /* 自动重装载寄存器预装载使能 */                             /* 自动装载值 */
    HAL_TIM_Base_Init(&timx_Handles[key]);
    HAL_TIM_Base_Start_IT(&timx_Handles[key]);    /* 使能定时器x及其更新中断 */

    if(key == 1)
    {
        HAL_TIM_IC_Init(&timx_Handles[key]);
        TIM_IC_InitTypeDef timx_ic_cap_chy = {0};
        timx_ic_cap_chy.ICPolarity = TIM_ICPOLARITY_RISING;         /* 上升沿捕获 */
        timx_ic_cap_chy.ICSelection = TIM_ICSELECTION_DIRECTTI;     /* 输入捕获IC1映射为TIMx_CH1 */
        timx_ic_cap_chy.ICPrescaler = TIM_ICPSC_DIV1;               /* 输入捕获IC1预分频系数为不分频 */
        timx_ic_cap_chy.ICFilter = 0;                               /* 输入捕获IC1滤波系数为不滤波 */
        HAL_TIM_IC_ConfigChannel(&timx_Handles[key], &timx_ic_cap_chy, TIM_CHANNEL_1);    /* 配置通道1 */
        __HAL_TIM_ENABLE_IT(&timx_Handles[key], TIM_IT_UPDATE);         /* 使能更新中断 */
        HAL_TIM_IC_Start_IT(&timx_Handles[key], TIM_CHANNEL_1);     /* 开始捕获TIM1的通道1 */
    }

}

/**
 * @brief       定时器底层驱动，开启时钟，设置中断优先级
                此函数会被HAL_TIM_Base_Init()函数调用
 * @param       htim:定时器句柄
 * @retval      无
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6)
    {
        __HAL_RCC_TIM6_CLK_ENABLE();                     /* 使能TIM时钟 */
        HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 1, 3); /* 抢占1，子优先级3，组2 */
        HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);         /* 开启ITM3中断 */
    }
    else if (htim->Instance == TIM7)
    {
        __HAL_RCC_TIM7_CLK_ENABLE();                     /* 使能TIM时钟 */
        HAL_NVIC_SetPriority(TIM7_IRQn, 1, 3); /* 抢占1，子优先级3，组2 */
        HAL_NVIC_EnableIRQ(TIM7_IRQn);         /* 开启ITM3中断 */
    }
    else if (htim->Instance == TIM1)
    {
        __HAL_RCC_TIM1_CLK_ENABLE();                     /* 使能TIM时钟 */
        HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 1, 3); /* 抢占1，子优先级3，组2 */
        HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);         /* 开启更新中断 */

        HAL_NVIC_SetPriority(TIM1_CC_IRQn, 1, 2);       /* 设置捕获中断优先级，抢占1，子优先级2 */
        HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);               /* 开启捕获中断 */
    }
}



void HAL_TIM_IC_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        GPIO_InitTypeDef GPIO_InitStruct;
        __HAL_RCC_TIM1_CLK_ENABLE();                     /* 使能TIM时钟 */
        __HAL_RCC_GPIOA_CLK_ENABLE();                    /* 开启捕获IO时钟 */
        GPIO_InitStruct.Pin = GPIO_PIN_8;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;          /* 复用推挽输出 */
        GPIO_InitStruct.Pull = GPIO_PULLUP;                 /* 上拉 */
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;       /* 高速 */
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;       /* 设置为TIM1复用功能 */
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        HAL_NVIC_SetPriority(TIM1_CC_IRQn, 1, 3); /* 抢占1，子优先级3，组2 */
        HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);         /* 开启ITM3中断 */
    } 
}


/* 输入捕获状态(g_timxchy_cap_sta)
 * [7]  :0,没有成功的捕获;1,成功捕获到一次.
 * [6]  :0,还没捕获到高电平;1,已经捕获到高电平了.
 * [5:0]:捕获高电平后溢出的次数,最多溢出63次,所以最长捕获值 = 63*65536 + 65535 = 4194303
 *       注意:为了通用,我们默认ARR和CCRy都是16位寄存器,对于32位的定时器(如:TIM5),也只按16位使用
 *       按1us的计数频率,最长溢出时间为:4194303 us, 约4.19秒
 *
 *      (说明一下：正常32位定时器来说,1us计数器加1,溢出时间:4294秒)
 */
uint8_t g_timxchy_cap_sta = 0;    /* 输入捕获状态 */
uint16_t g_timxchy_cap_val = 0;   /* 输入捕获值 */


/**
 * @brief       定时器输入捕获中断处理回调函数
 * @param       htim:定时器句柄指针
 * @note        该函数在HAL_TIM_IRQHandler中会被调用
 * @retval      无
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        if ((g_timxchy_cap_sta & 0X80) == 0)                /* 还未成功捕获 */
        {
            if (g_timxchy_cap_sta & 0X40)                   /* 捕获到一个下降沿 */
            {
                g_timxchy_cap_sta |= 0X80;                  /* 标记成功捕获到一次高电平脉宽 */
                
                g_timxchy_cap_val = HAL_TIM_ReadCapturedValue(&timx_Handles[1], TIM_CHANNEL_1);  /* 获取当前的捕获值 */
                TIM_RESET_CAPTUREPOLARITY(&timx_Handles[1], TIM_CHANNEL_1);                      /* 一定要先清除原来的设置 */
                TIM_SET_CAPTUREPOLARITY(&timx_Handles[1], TIM_CHANNEL_1, TIM_ICPOLARITY_RISING); /* 配置通道1上升沿捕获 */
            }
            else /* 还未开始,第一次捕获上升沿 */
            {
                g_timxchy_cap_sta = 0;                              /* 清空 */
                g_timxchy_cap_val = 0;
                g_timxchy_cap_sta |= 0X40;                          /* 标记捕获到了上升沿 */
                __HAL_TIM_DISABLE(&timx_Handles[1]);          /* 关闭定时器 */
                __HAL_TIM_SET_COUNTER(&timx_Handles[1], 0);   /* 定时器5计数器清零 */
                TIM_RESET_CAPTUREPOLARITY(&timx_Handles[1], TIM_CHANNEL_1);   /* 一定要先清除原来的设置！！ */
                TIM_SET_CAPTUREPOLARITY(&timx_Handles[1], TIM_CHANNEL_1, TIM_ICPOLARITY_FALLING); /* 通道1设置为下降沿捕获 */
                __HAL_TIM_ENABLE(&timx_Handles[1]);           /* 使能定时器 */
            }
        }
    }
}


/**
 * @brief       定时器TIM1更新中断服务函数
 * @param       无
 * @retval      无
 */
void TIM1_UP_TIM16_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&timx_Handles[1]); /* 定时器中断公共处理函数 */
}


/**
 * @brief       定时器TIM1中断服务函数
 * @param       无
 * @retval      无
 */
void TIM1_CC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&timx_Handles[1]); /* 定时器中断公共处理函数 */
}

/**
 * @brief       定时器TIM6中断服务函数
 * @param       无
 * @retval      无
 */
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&timx_Handles[6]); /* 定时器中断公共处理函数 */
}

/**
 * @brief       定时器TIM6中断服务函数
 * @param       无
 * @retval      无
 */
void TIM7_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&timx_Handles[7]); /* 定时器中断公共处理函数 */
}

