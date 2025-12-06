// pwr.h
#ifndef __PWR_H
#define __PWR_H

#include "sys.h"


/******************************************************************************************/
/* PWR WKUP 按键 引脚和中断 定义 
 * 我们通过WK_UP按键唤醒 MCU,  因此必须定义这个按键及其对应的中断服务函数 
 */

#define PWR_WKUP_GPIO_PORT              GPIOA
#define PWR_WKUP_GPIO_PIN               GPIO_PIN_0
#define PWR_WKUP_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)   /* PA口时钟使能 */
  
#define PWR_WKUP_INT_IRQn               EXTI0_IRQn
#define PWR_WKUP_INT_IRQHandler         EXTI0_IRQHandler

/******************************************************************************************/

void pwr_pvd_init(uint32_t pls); /* PVD电压检测初始化函数 */
void pwr_wkup_key_init(void);    /* 唤醒按键初始化 */

//请不要在HAL_GPIO_EXTI_Callback中调用一下函数，会发生中断嵌套
void pwr_enter_sleep(void);      /* 进入睡眠模式 */
void pwr_enter_stop(void);       /* 进入停止模式 */
void pwr_enter_standby(void);    /* 进入待机模式 */
#endif




















