#ifndef __DELAY_H
#define __DELAY_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "stm32l4xx_hal.h"
#include "sys.h"

/*---- SysTick版本(支持ucosii) ----*/
void delay_init(uint16_t sysclk); /* 初始化延迟函数 */
void delay_ms(uint16_t nms);      /* 延时nms */
void delay_us(uint32_t nus);      /* 延时nus */

/*---- 时钟滴答器版本 ----*/
void Delay_us(uint32_t xus);
void Delay_ms(uint32_t xms);
void Delay_s(uint32_t xs);

#if (!SYS_SUPPORT_OS)           /* 如果不支持OS */
void HAL_Delay(uint32_t Delay); /* HAL库的延时函数，HAL库内部用到 */
#endif

#ifdef __cplusplus
}
#endif
#endif
