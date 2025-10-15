#include "borad.h"
#include "delay.h"

// 定义PWM模拟的分辨率
#define PWM_RESOLUTION 100

const struct gpio_pin LED = {GPIOC, GPIO_PIN_13};

void led_init(void)
{
    GPIO_InitTypeDef GPIO_Init_Struct;
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_Init_Struct.Pin = LED_R.pin | LED_B.pin;
    GPIO_Init_Struct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init_Struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_Init_Struct);
    GPIO_Init_Struct.Pin = LED_G.pin;
    HAL_GPIO_Init(GPIOA, &GPIO_Init_Struct);
    //上电默认高电平 LED共阳
    HAL_GPIO_WritePin(GPIOA, LED_G.pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, LED_R.pin | LED_B.pin, GPIO_PIN_SET);
}

// LED控制函数，HIGH为熄灭，LOW为点亮（因为是共阳极）
void led_control(struct gpio_pin led, uint8_t state) {
    HAL_GPIO_WritePin(led.GPIOx, led.pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

// RGB565编码函数
uint16_t rgb565_encode(uint8_t r, uint8_t g, uint8_t b) {
    // 将8位RGB值转换为RGB565格式
    // R: 8位转5位，右移3位
    // G: 8位转6位，右移2位
    // B: 8位转5位，右移3位
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
}

// RGB565解码函数
static void rgb565_decode(uint16_t rgb565, uint8_t *r, uint8_t *g, uint8_t *b) {
    // 从RGB565值中提取RGB分量
    *r = ((rgb565 >> 11) & 0x1F) << 3;  // 提取5位R并扩展到8位
    *g = ((rgb565 >> 5) & 0x3F) << 2;   // 提取6位G并扩展到8位
    *b = (rgb565 & 0x1F) << 3;          // 提取5位B并扩展到8位
}



// 使用RGB565值控制RGB LED
void led_rgb565(uint16_t rgb565) {
    // PWM计数器
    static uint8_t pwm_counter;
    // 当前RGB占空比
    static uint8_t current_r_duty;
    static uint8_t current_g_duty;
    static uint8_t current_b_duty;
    uint8_t r, g, b;

    // 解码RGB565值
    rgb565_decode(rgb565, &r, &g, &b);

    // 将0-255的值转换为0-PWM_RESOLUTION的占空比
    // 注意: 共阴极LED，值越大越亮，0为熄灭，PWM_RESOLUTION为最亮
    current_r_duty = 100 - (r * PWM_RESOLUTION / 255);
    current_g_duty = 100 - (g * PWM_RESOLUTION / 255);
    current_b_duty = 100 - (b * PWM_RESOLUTION / 255);

    // 更新全局PWM计数器
    pwm_counter++;
    if (pwm_counter >= PWM_RESOLUTION) {
        pwm_counter = 0;
    }

    // 同步更新RGB LED状态
    if (pwm_counter < current_r_duty) {
        led_control(LED_R, 1); // 点亮红色LED
    } else {
        led_control(LED_R, 0); // 熄灭红色LED
    }

    if (pwm_counter < current_g_duty) {
        led_control(LED_G, 1); // 点亮绿色LED
    } else {
        led_control(LED_G, 0); // 熄灭绿色LED
    }

    if (pwm_counter < current_b_duty) {
        led_control(LED_B, 1); // 点亮蓝色LED
    } else {
        led_control(LED_B, 0); // 熄灭蓝色LED
    }
}

//PC13上的一个LED灯
void LED_Init()
{
    GPIO_InitTypeDef GPIO_Init_Struct;
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_Init_Struct.Pin = LED.pin;
    GPIO_Init_Struct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init_Struct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_Init_Struct);
    HAL_GPIO_WritePin(GPIOC, LED.pin, GPIO_PIN_RESET);  //高电平点亮
}



void key_init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_Init_Struct;
#if KEY_EXIT
    GPIO_Init_Struct.Pin = KEY_1.pin | KEY_2.pin;
    GPIO_Init_Struct.Mode = GPIO_MODE_IT_FALLING;   //下降沿触发
    GPIO_Init_Struct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_Init_Struct);
    GPIO_Init_Struct.Pin = KEY_2.pin;
    HAL_GPIO_Init(GPIOB, &GPIO_Init_Struct);    //有问题,这里必须二次调用才能上拉
    HAL_NVIC_SetPriority(KEY1_INT_IRQn, 0, 2); //优先级
    HAL_NVIC_EnableIRQ(KEY1_INT_IRQn);
    HAL_NVIC_SetPriority(KEY2_INT_IRQn, 0, 2);
    HAL_NVIC_EnableIRQ(KEY2_INT_IRQn);
#else
    GPIO_Init_Struct.Pin = KEY_1.pin | KEY_2.pin;
    GPIO_Init_Struct.Mode = GPIO_MODE_INPUT;
    GPIO_Init_Struct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_Init_Struct);
    GPIO_Init_Struct.Pin = KEY_1.pin;
    HAL_GPIO_Init(GPIOB, &GPIO_Init_Struct);    //有问题,这里必须二次调用才能上拉
#endif
}

#if KEY_EXIT
/**
 * @brief  KEY1外部中断服务程序
 * @param  无
 * @retval 无
 */
void EXTI3_IRQHandler(void)
{ 
    HAL_GPIO_EXTI_IRQHandler(KEY_1.pin);    //调用中断处理公用函数 清除KEY1所在中断线 的中断标志位，中断下半部在HAL_GPIO_EXTI_Callback执行
    __HAL_GPIO_EXTI_CLEAR_IT(KEY_1.pin);    //HAL库默认先清中断再处理回调，退出时再清一次中断，避免按键抖动误触发
}

/**
 * @brief  KEY2外部中断服务程序
 * @param  无
 * @retval 无
 */
void EXTI4_IRQHandler(void)
{ 
    HAL_GPIO_EXTI_IRQHandler(KEY_2.pin);    //调用中断处理公用函数 清除KEY1所在中断线 的中断标志位，中断下半部在HAL_GPIO_EXTI_Callback执行
    __HAL_GPIO_EXTI_CLEAR_IT(KEY_2.pin);    //HAL库默认先清中断再处理回调，退出时再清一次中断，避免按键抖动误触发
}

/**
 * @brief  中断服务程序中需要做的事情
           在HAL库中所有的外部中断服务函数都会调用此函数
 * @param  GPIO_Pin:中断引脚号
 * @retval 无
 */
// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
// {
//     if(GPIO_Pin == KEY_1.pin)
//     {
//     }
//     else if(GPIO_Pin == KEY_2.pin)
//     {
//     }
// }
#else
/**
 * @brief  扫描按键是否按下
 * @param  无
 * @note   硬件提供了消抖
 * @retval 无
 */
uint8_t key_scan(void)
{
    if(HAL_GPIO_ReadPin(KEY_1.GPIOx, KEY_1.pin) == GPIO_PIN_RESET)
    {
        //delay_ms(10);   //消抖
        if(HAL_GPIO_ReadPin(KEY_1.GPIOx, KEY_1.pin) == GPIO_PIN_RESET)
        {
            while(HAL_GPIO_ReadPin(KEY_1.GPIOx, KEY_1.pin) == GPIO_PIN_RESET);
            //delay_ms(10);
            return 1;
        }
    }
    if(HAL_GPIO_ReadPin(KEY_2.GPIOx, KEY_2.pin) == GPIO_PIN_RESET)
    {
        //delay_ms(10);   //消抖
        if(HAL_GPIO_ReadPin(KEY_2.GPIOx, KEY_2.pin) == GPIO_PIN_RESET)
        {
            while(HAL_GPIO_ReadPin(KEY_2.GPIOx, KEY_2.pin) == GPIO_PIN_RESET);
            //delay_ms(10);
            return 2;
        }
    }
    return 0;
}
#endif








