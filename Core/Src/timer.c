//timer.c
#include "timer.h"
#include "borad.h"
#include "usart.h"
#include <stdarg.h>


// 定义定时器配置数组
TimerConfig timer_configs[TIMER_KEY_COUNT] = {0};
static uint32_t g_npwm_remain = 0;

/**
  * @brief 初始化定时器的更新中断功能
  * @param       Timx: 定时器
  * @param       arr: 自动重装载值
  * @param       psc: 预分频系数
  * @note  高级定时器,通用定时器,基本定时器均可使用
  * @retval 无
  */
void Timx_baseStart_Init(TIM_TypeDef *Timx, uint16_t arr, uint16_t psc)
{
    uint8_t key;
    switch ((uint32_t)Timx) //允许所有定时器
    {
        case (uint32_t)TIM1:    //高级定时器
            key = TIM1_KEY;
            break;
        case (uint32_t)TIM2:    //通用定时器
            key = TIM2_KEY;
            break;
        case (uint32_t)TIM6:    //基本定时器
            key = TIM6_KEY;
            break;
        case (uint32_t)TIM7:    //基本定时器
            key = TIM7_KEY;
            break;
        case (uint32_t)TIM15:   //通用定时器
            key = TIM15_KEY;
            break;
        case (uint32_t)TIM16:   //通用定时器
            key = TIM16_KEY;
            break;
        case (uint32_t)LPTIM1:  //低功耗定时器
            key = LPTIM1_KEY;
            break;
        case (uint32_t)LPTIM2:  //低功耗定时器
            key = LPTIM2_KEY;
            break;
    }
    
    timer_configs[key].timx_Handles.Instance = Timx;
    timer_configs[key].timx_Handles.Init.Prescaler = psc;                          /* 设置预分频系数 */
    timer_configs[key].timx_Handles.Init.CounterMode = TIM_COUNTERMODE_UP;         /* 递增计数模式 */
    timer_configs[key].timx_Handles.Init.Period = arr;
    timer_configs[key].timx_Handles.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;   /* 时钟分频 */
    timer_configs[key].timx_Handles.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE; /* 自动重装载寄存器预装载使能 */                             /* 自动装载值 */
    HAL_TIM_Base_Init(&timer_configs[key].timx_Handles);
    HAL_TIM_Base_Start_IT(&timer_configs[key].timx_Handles);    /* 使能定时器x及其更新中断 */
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
        HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 3, 3); /* 抢占1，子优先级3，组2 */
        HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);         /* 开启ITM3中断 */
    }
    else if (htim->Instance == TIM7)
    {
        __HAL_RCC_TIM7_CLK_ENABLE();                     /* 使能TIM时钟 */
        HAL_NVIC_SetPriority(TIM7_IRQn, 3, 3); /* 抢占1，子优先级3，组2 */
        HAL_NVIC_EnableIRQ(TIM7_IRQn);         /* 开启ITM3中断 */
    }
    else if (htim->Instance == TIM1)
    {
        __HAL_RCC_TIM1_CLK_ENABLE();                     /* 使能TIM时钟 */
        HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 2, 3); /* 抢占1，子优先级3，组2 */
        HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);         /* 开启更新中断 */
    }
}

/**
 * @brief       通用定时器及以上TIMX 通道Y PWM输出 初始化函数（使用PWM模式1）
 * @note
 *              通用定时器的时钟来自APB1,当PPRE1 ≥ 2分频的时候
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *              默认初始化为50%的占空比
 * @param       Timx: 定时器
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @param       channel_count: 使用的通道数量
 * @param       可变参数列表，传入使用的通道（如 TIM_CHANNEL_1, TIM_CHANNEL_2 等）
 * 
 * @example     arr: 1000 -1  psc: 80 -1 等于1ms
 * @retval      无
 */
void timx_pwmStart_init(TIM_TypeDef *Timx, uint16_t arr, uint16_t psc, uint8_t channel_count, ...)
{
    TIM_OC_InitTypeDef timx_oc_pwm_chy  = {0};  /* 定时器PWM输出配置 */
    uint8_t key;
    va_list args;
    switch ((uint32_t)Timx) //允许高级定时器,通用定时器，低功耗定时器
    {
        case (uint32_t)TIM1:    //高级定时器
            key = TIM1_KEY;
            break;
        case (uint32_t)TIM2:    //通用定时器
            key = TIM2_KEY;
            break;
        case (uint32_t)TIM15:   //通用定时器
            key = TIM15_KEY;
            break;
        case (uint32_t)TIM16:   //通用定时器
            key = TIM16_KEY;
            break;
        case (uint32_t)LPTIM1:  //低功耗定时器
            key = LPTIM1_KEY;
            break;
        case (uint32_t)LPTIM2:  //低功耗定时器
            key = LPTIM2_KEY;
            break;
    }
                           
    timer_configs[key].timx_Handles.Instance = Timx;                              /* 定时器x */
    timer_configs[key].timx_Handles.Init.Prescaler = psc;                         /* 定时器分频 */
    timer_configs[key].timx_Handles.Init.CounterMode = TIM_COUNTERMODE_UP;        /* 递增计数模式 */
    timer_configs[key].timx_Handles.Init.Period = arr;                            /* 自动重装载值 */
    HAL_TIM_PWM_Init(&timer_configs[key].timx_Handles);                           /* 初始化PWM */

    timx_oc_pwm_chy.OCMode = TIM_OCMODE_PWM1;                           /* 模式选择PWM1 */
    timx_oc_pwm_chy.Pulse = arr / 2;                                    /* 设置比较值,此值用来确定占空比 */
    timx_oc_pwm_chy.OCPolarity = TIM_OCPOLARITY_LOW;                    /* 输出比较极性为低 */

    // 使用可变参数列表获取通道
    va_start(args, channel_count);
    for (int i = 0; i < channel_count; i++)
    {
        uint32_t channel = va_arg(args, uint32_t);

        // 配置通道
        HAL_TIM_PWM_ConfigChannel(&timer_configs[key].timx_Handles, &timx_oc_pwm_chy, channel);

        // 启动 PWM 输出
        HAL_TIM_PWM_Start(&timer_configs[key].timx_Handles, channel);
    }
    va_end(args);
}

/**
  * @brief  修改Timx通道channel_y的捕获比较寄存器的值
  * @note   基本定时器没有该寄存器
  *         请在HAL_TIM_PWM_MspInit中设定引脚或重映射引脚
  * @param  Timx: 定时器
  * @param  TIM_CHANNEL_y: 通道
  * @param  pwm_vaule: 定时器的比较值
  * @retval 无
  */
void timx_pwmSetCompare(TIM_TypeDef *Timx,unsigned int TIM_CHANNEL_y,uint16_t pwm_vaule)
{
    uint8_t key;
    switch ((uint32_t)Timx) //允许高级定时器,通用定时器，低功耗定时器
    {
        case (uint32_t)TIM1:    //高级定时器
            key = TIM1_KEY;
            break;
        case (uint32_t)TIM2:    //通用定时器
            key = TIM2_KEY;
            break;
        case (uint32_t)TIM15:   //通用定时器
            key = TIM15_KEY;
            break;
        case (uint32_t)TIM16:   //通用定时器
            key = TIM16_KEY;
            break;
        case (uint32_t)LPTIM1:  //低功耗定时器
            key = LPTIM1_KEY;
            break;
        case (uint32_t)LPTIM2:  //低功耗定时器
            key = LPTIM2_KEY;
            break;
    }
    __HAL_TIM_SetCompare(&timer_configs[key].timx_Handles, TIM_CHANNEL_y, pwm_vaule);
}


/**
 * @brief       高级定时器TIM1 通道Y 输出指定个数PWM 初始化函数(该开发板只有TIM1一个高级定时器)
 * @note
 *              高级定时器的时钟来自APB2, 而PCLK2 = 80Mhz, 我们设置PPRE2不分频, 因此
 *              高级定时器时钟 = 80Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *              
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void atim1_npwmStart_init(uint16_t arr, uint16_t psc)
{
    GPIO_InitTypeDef gpio_init_struct;
    TIM_OC_InitTypeDef timx_oc_npwm_chy;   /* 定时器输出 */
    __HAL_RCC_GPIOA_CLK_ENABLE();  /* TIM1 通道IO口时钟使能 */
    __HAL_RCC_TIM1_CLK_ENABLE();       /* TIM1 时钟使能 */

    timer_configs[TIM1_KEY].timx_Handles.Instance = TIM1;                            /* 定时器 */
    timer_configs[TIM1_KEY].timx_Handles.Init.Prescaler = psc;                       /* 定时器分频 */
    timer_configs[TIM1_KEY].timx_Handles.Init.CounterMode = TIM_COUNTERMODE_UP;      /* 递增计数模式 */
    timer_configs[TIM1_KEY].timx_Handles.Init.Period = arr;                          /* 自动重装载值 */
    timer_configs[TIM1_KEY].timx_Handles.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE; /*使能TIMx_ARR进行缓冲 */
    timer_configs[TIM1_KEY].timx_Handles.Init.RepetitionCounter = 0;                 /* 重复计数器初始值 */
    HAL_TIM_PWM_Init(&timer_configs[TIM1_KEY].timx_Handles);                         /* 初始化PWM */

    gpio_init_struct.Pin = GPIO_PIN_8;                                 /* 通道y的CPIO口 */
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;                           /* 复用推完输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                               /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;                     /* 高速 */
    gpio_init_struct.Alternate = GPIO_AF1_TIM1;
    HAL_GPIO_Init(GPIOA, &gpio_init_struct);

    timx_oc_npwm_chy.OCMode = TIM_OCMODE_PWM1;                         /* 模式选择PWM 1*/
    timx_oc_npwm_chy.Pulse = arr / 2;                                  /* 设置比较值,此值用来确定占空比 */
                                                                       /* 这里默认设置比较值为自动重装载值的一半,即占空比为50% */
    timx_oc_npwm_chy.OCPolarity = TIM_OCPOLARITY_HIGH;                 /* 输出比较极性为高 */
    HAL_TIM_PWM_ConfigChannel(&timer_configs[TIM1_KEY].timx_Handles, &timx_oc_npwm_chy, TIM_CHANNEL_1); /* 配置TIM1通道y */

    HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 1, 3);                   /* 设置中断优先级，抢占优先级1，子优先级3 */
    HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);                           /* 开启TIM1中断 */

    __HAL_TIM_ENABLE_IT(&timer_configs[TIM1_KEY].timx_Handles, TIM_IT_UPDATE);       /* 允许更新中断 */
    HAL_TIM_PWM_Start(&timer_configs[TIM1_KEY].timx_Handles, TIM_CHANNEL_1);    /* 开启对应PWM通道 */
    
}

/**
 * @brief       高级定时器TIM1 NPWM设置PWM个数
 * @param       rcr: PWM的个数, 1~2^32次方个
 * @retval      无
 */
void atim1_npwm_chy_set(uint32_t npwm)
{
    if (npwm == 0) return;

    g_npwm_remain = npwm;                                                   /* 保存脉冲个数 */
    HAL_TIM_GenerateEvent(&timer_configs[TIM1_KEY].timx_Handles, TIM_EVENTSOURCE_UPDATE); /* 产生一次更新事件,在中断里面处理脉冲输出 */
    __HAL_TIM_ENABLE(&timer_configs[TIM1_KEY].timx_Handles);                              /* 使能定时器TIMX */
}

/**
 * @brief       定时器底层驱动，时钟使能，引脚配置
                此函数会被HAL_TIM_PWM_Init()调用
 * @param       htim:定时器句柄
 * @retval      无
 */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        GPIO_InitTypeDef gpio_init_struct = {0};
        __HAL_RCC_TIM1_CLK_ENABLE();                                   /* 使能TIM1时钟 */
        __HAL_RCC_GPIOA_CLK_ENABLE();                                  /* 使能GPIOA时钟 */

        gpio_init_struct.Pin = GPIO_PIN_8;                             /* PA8 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;                       /* 复用推挽输出 */
        gpio_init_struct.Pull = GPIO_NOPULL;                           /* 无上下拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;                 /* 高速 */
        gpio_init_struct.Alternate = GPIO_AF1_TIM1;
        HAL_GPIO_Init(GPIOA, &gpio_init_struct);
    }
    if (htim->Instance == TIM2)
    {
        GPIO_InitTypeDef gpio_init_struct = {0};
        __HAL_RCC_TIM2_CLK_ENABLE();                                   /* 使能TIM2时钟 */
        __HAL_RCC_GPIOA_CLK_ENABLE();                                  /* 使能GPIOA时钟 */

        gpio_init_struct.Pin = GPIO_PIN_0;                             /* PA0 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;                       /* 复用推挽输出 */
        gpio_init_struct.Pull = GPIO_NOPULL;                           /* 无上下拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;                 /* 高速 */
        gpio_init_struct.Alternate = GPIO_AF1_TIM2;                    /* 复用功能为TIM2 */
        HAL_GPIO_Init(GPIOA, &gpio_init_struct);
    }
}

/**
  * @brief      初始化定时器的输入捕获功能
  * @param      Timx: 定时器
  * @param      channel_count: 使用的通道数量
  * @param      可变参数列表，传入使用的通道（如 TIM_CHANNEL_1, TIM_CHANNEL_2 等）
  * @note       TIM1 Channel1:PA8 Channel2:PA9
  *             请到 HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
  *             中实现对应通道的处理逻辑
  * @retval     无
  * @example    Timx_ICStart_Init(TIM1, 2, TIM_CHANNEL_1, TIM_CHANNEL_2); // 初始化 TIM1 的通道 1 和通道 2
*/
void Timx_ICStart_Init(TIM_TypeDef *Timx, uint8_t channel_count, ...)
{
    TIM_IC_InitTypeDef timx_ic_cap_chy = {0};
    uint8_t key;
    va_list args;
    //允许高级定时器,通用定时器
    switch ((uint32_t)Timx)
    {
        case (uint32_t)TIM1:    //高级定时器
            key = TIM1_KEY;
            break;
        case (uint32_t)TIM2:    //通用定时器
            key = TIM2_KEY;
            break;
        case (uint32_t)TIM15:   //通用定时器
            key = TIM15_KEY;
            break;
        case (uint32_t)TIM16:   //通用定时器
            key = TIM16_KEY;
            break;
    }
     // 初始化定时器
    timer_configs[key].timx_Handles.Instance = Timx;                                  /* 定时器1 */
    timer_configs[key].timx_Handles.Init.Prescaler = 80-1;                            /* 定时器分频 */
    timer_configs[key].timx_Handles.Init.CounterMode = TIM_COUNTERMODE_UP;            /* 递增计数模式 */
    timer_configs[key].timx_Handles.Init.Period = 0xFFFF;                            /* 自动重装载值 */
    
    // 配置输入捕获参数
    timx_ic_cap_chy.ICPolarity = TIM_ICPOLARITY_RISING;                 /* 上升沿捕获 */
    timx_ic_cap_chy.ICSelection = TIM_ICSELECTION_DIRECTTI;             /* 映射到TI1上 */
    timx_ic_cap_chy.ICPrescaler = TIM_ICPSC_DIV1;                       /* 配置输入分频，不分频 */
    timx_ic_cap_chy.ICFilter = 0;                                       /* 配置输入滤波器，不滤波 */

    __HAL_TIM_ENABLE_IT(&timer_configs[key].timx_Handles, TIM_IT_UPDATE);         /* 使能更新中断 */

    // 使用可变参数列表获取通道
    va_start(args, channel_count);
    for (uint8_t i = 0; i < channel_count; i++)
    {
        uint32_t channel = va_arg(args, uint32_t);
        // 存储通道到全局数组
        if (i < __TIMx_IC_MAX_CHANNLES) // 最大支持 __TIMx_IC_MAX_CHANNLES 个通道
        {
            timer_configs[key].channels[i] = (uint16_t)channel + 1;   //防止通道一初始化值相同
            printf("channel + 1 : %d\r\n",timer_configs[key].channels[i]);
        }
    }
    va_end(args);

    HAL_TIM_IC_Init(&timer_configs[key].timx_Handles);      //要等timer_configs赋值完成才能调用回调函数
    
    // 使用可变参数列表获取通道
    va_start(args, channel_count);
    for (uint8_t i = 0; i < channel_count; i++)
    {
        uint32_t channel = va_arg(args, uint32_t);

        // 配置通道
        HAL_TIM_IC_ConfigChannel(&timer_configs[key].timx_Handles, &timx_ic_cap_chy, channel);

        // 使能对应通道的捕获中断
        switch (channel)
        {
            case TIM_CHANNEL_1:
                __HAL_TIM_ENABLE_IT(&timer_configs[key].timx_Handles, TIM_IT_CC1);
                break;
            case TIM_CHANNEL_2:
                __HAL_TIM_ENABLE_IT(&timer_configs[key].timx_Handles, TIM_IT_CC2);
                break;
            case TIM_CHANNEL_3:
                __HAL_TIM_ENABLE_IT(&timer_configs[key].timx_Handles, TIM_IT_CC3);
                break;
            case TIM_CHANNEL_4:
                __HAL_TIM_ENABLE_IT(&timer_configs[key].timx_Handles, TIM_IT_CC4);
                break;
            default:
                break; // 忽略无效通道
        }

        // 启动捕获
        HAL_TIM_IC_Start_IT(&timer_configs[key].timx_Handles, channel);
    }
    va_end(args);
}


/**
  * @brief 定时器底层驱动，时钟使能，引脚配置
  *        此函数会被HAL_TIM_IC_Init()调用
  * @param       htim:定时器句柄
  * @retval 无
  */
void HAL_TIM_IC_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        __HAL_RCC_TIM1_CLK_ENABLE();                     /* 使能TIM时钟 */
        __HAL_RCC_GPIOA_CLK_ENABLE();                    /* 开启捕获IO时钟 */

        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;       /* 高速 */
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;       /* 设置为TIM1复用功能 */
        for(uint8_t i = 0; i < __TIMx_IC_MAX_CHANNLES && timer_configs[TIM1_KEY].channels[i] != 0; i++)
        {
            switch (timer_configs[TIM1_KEY].channels[i]-1)
            {
                case TIM_CHANNEL_1:
                    GPIO_InitStruct.Pin = GPIO_PIN_8; // PA8 对应 TIM1_CH1
                    break;
                case TIM_CHANNEL_2:
                    GPIO_InitStruct.Pin = GPIO_PIN_9; // PA9 对应 TIM1_CH2
                    break;
                case TIM_CHANNEL_3:
                    GPIO_InitStruct.Pin = GPIO_PIN_10; // PA10 对应 TIM1_CH3
                    break;
                case TIM_CHANNEL_4:
                    GPIO_InitStruct.Pin = GPIO_PIN_11; // PA11 对应 TIM1_CH4
                    break;
                default:
                    continue; // 忽略无效通道
            }
            HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        }
        
        HAL_NVIC_SetPriority(TIM1_CC_IRQn, 2, 3);
        HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);         /* 开启ITM3中断 */

        HAL_NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 2, 2); /* 抢占1，子优先级3，组2 */
        HAL_NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);         /* 开启更新中断 */
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
  * @brief 获取输入捕获的高电平时间
  * @retval 高电平时间,单位us
  */
uint16_t Timx_ICHighTime(void)
{
    static uint16_t time = 0;
    if (g_timxchy_cap_sta & 0X80) {
        time = (g_timxchy_cap_sta & 0X3F) * 65536 + g_timxchy_cap_val;
        g_timxchy_cap_sta = 0;
    }
    return time;
}


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
        if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_CC1) == RESET)
        {
            uint16_t capture_value = HAL_TIM_ReadCapturedValue(&timer_configs[TIM1_KEY].timx_Handles, TIM_CHANNEL_1);
            if ((g_timxchy_cap_sta & 0X80) == 0)                /* 还未成功捕获 */
            {
                if (g_timxchy_cap_sta & 0X40)                   /* 捕获到一个下降沿 */
                {
                    g_timxchy_cap_sta |= 0X80;                  /* 标记成功捕获到一次高电平脉宽 */
                    
                    g_timxchy_cap_val = capture_value;  /* 获取当前的捕获值 */
                    
                    TIM_RESET_CAPTUREPOLARITY(&timer_configs[TIM1_KEY].timx_Handles, TIM_CHANNEL_1);                      /* 一定要先清除原来的设置 */
                    TIM_SET_CAPTUREPOLARITY(&timer_configs[TIM1_KEY].timx_Handles, TIM_CHANNEL_1, TIM_ICPOLARITY_RISING); /* 配置通道1上升沿捕获 */
                }
                else /*第一次捕获到上升沿 */
                {
                    g_timxchy_cap_sta = 0;                              /* 清空 */
                    g_timxchy_cap_val = 0;
                    g_timxchy_cap_sta |= 0X40;                          /* 标记捕获到了上升沿 */
                    __HAL_TIM_DISABLE(&timer_configs[TIM1_KEY].timx_Handles);          /* 关闭定时器 */
                    __HAL_TIM_SET_COUNTER(&timer_configs[TIM1_KEY].timx_Handles, 0);   /* 定时器5计数器清零 */
                    TIM_RESET_CAPTUREPOLARITY(&timer_configs[TIM1_KEY].timx_Handles, TIM_CHANNEL_1);   /* 一定要先清除原来的设置！！ */
                    TIM_SET_CAPTUREPOLARITY(&timer_configs[TIM1_KEY].timx_Handles, TIM_CHANNEL_1, TIM_ICPOLARITY_FALLING); /* 通道1设置为下降沿捕获 */
                    __HAL_TIM_ENABLE(&timer_configs[TIM1_KEY].timx_Handles);           /* 使能定时器 */
                }
            }
        }
        if(__HAL_TIM_GET_FLAG(htim, TIM_FLAG_CC2) == RESET)
        {
            //HAL_GPIO_TogglePin(LED.GPIOx,LED.pin);
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
    #ifdef TIM1_NPWM_USE
        uint16_t npwm = 0;
        /* 以下代码没有使用定时器HAL库共用处理函数来处理，而是直接通过判断中断标志位的方式 */
        if(__HAL_TIM_GET_FLAG(&timer_configs[TIM1_KEY].timx_Handles, TIM_FLAG_UPDATE) != RESET)
        {
            if (g_npwm_remain >= 256)           /* 还有大于256个脉冲需要发送 */
            {
                g_npwm_remain = g_npwm_remain - 256;
                npwm = 256;
            }
            else if (g_npwm_remain % 256)       /* 还有位数（不到256）个脉冲要发送 */
            {
                npwm = g_npwm_remain % 256;
                g_npwm_remain = 0;              /* 没有脉冲了 */
            }

            if (npwm) /* 有脉冲要发送 */
            {
                TIM1->RCR = npwm - 1;                                         /* 设置重复计数寄存器值为npwm-1, 即npwm个脉冲 */
                HAL_TIM_GenerateEvent(&timer_configs[TIM1_KEY].timx_Handles, TIM_EVENTSOURCE_UPDATE); /* 产生一次更新事件,在中断里面处理脉冲输出 */
                __HAL_TIM_ENABLE(&timer_configs[TIM1_KEY].timx_Handles);                              /* 使能定时器TIMX */
            }
            else
            {
                TIM1->CR1 &= ~(1 << 0); /* 关闭定时器TIMX，使用HAL Disable会清除PWM通道信息，此处不用 */
            }

            __HAL_TIM_CLEAR_IT(&timer_configs[TIM1_KEY].timx_Handles, TIM_IT_UPDATE);  /* 清除定时器溢出中断标志位 */
        }
    #else
        HAL_TIM_IRQHandler(&timer_configs[TIM1_KEY].timx_Handles); /* 定时器中断公共处理函数 */
    #endif
}

/**
 * @brief       定时器TIM1中断服务函数
 * @param       无
 * @retval      无
 */
void TIM1_CC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&timer_configs[TIM1_KEY].timx_Handles); /* 定时器中断公共处理函数 */
}

/**
 * @brief       定时器TIM6中断服务函数
 * @param       无
 * @retval      无
 */
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&timer_configs[TIM6_KEY].timx_Handles); /* 定时器中断公共处理函数 */
}

/**
 * @brief       定时器TIM6中断服务函数
 * @param       无
 * @retval      无
 */
void TIM7_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&timer_configs[TIM7_KEY].timx_Handles); /* 定时器中断公共处理函数 */
}





/**
 * @brief       定时器更新中断回调函数
 * @param       htim:定时器句柄
 * @retval      无
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        if ((g_timxchy_cap_sta & 0X80) == 0)            /* 还未成功捕获 */
        {
            if (g_timxchy_cap_sta & 0X40)               /* 已经捕获到高电平了 */
            {
                if ((g_timxchy_cap_sta & 0X3F) == 0X3F) /* 高电平太长了 */
                {
                    TIM_RESET_CAPTUREPOLARITY(&timer_configs[TIM1_KEY].timx_Handles, TIM_CHANNEL_1);                     /* 一定要先清除原来的设置 */
                    TIM_SET_CAPTUREPOLARITY(&timer_configs[TIM1_KEY].timx_Handles, TIM_CHANNEL_1, TIM_ICPOLARITY_RISING);/* 配置TIM5通道1上升沿捕获 */
                    g_timxchy_cap_sta |= 0X80;          /* 标记成功捕获了一次 */
                    g_timxchy_cap_val = 0XFFFF;
                }
                else      /* 累计定时器溢出次数 */
                {
                    g_timxchy_cap_sta++;
                }
            }
        }
    }

    if (htim->Instance == TIM2)
    {
        HAL_GPIO_TogglePin(LED.GPIOx,LED.pin);
    }
    if (htim->Instance == TIM6)
    {
        HAL_GPIO_TogglePin(LED.GPIOx,LED.pin);
    }
    if (htim->Instance == TIM7)
    {
        HAL_GPIO_TogglePin(LED.GPIOx,LED.pin);
    }
}
