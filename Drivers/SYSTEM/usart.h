#ifndef __USART_H
#define __USART_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "stm32l4xx_hal.h"
#include "stdio.h"
#include "stdbool.h"

	 
/* 选择USART编号 (使用数字定义，避免预处理器无法比较复杂宏的问题) */
#define USART1_SEL    1
#define USART2_SEL    2
#define USART3_SEL    3
#define UART4_SEL     4
#define UART5_SEL     5

/* 选择要使用的USART编号 */
//#define USART_SEL    USART1_SEL
#define USART_SEL    USART2_SEL     //RXD:PA2 TXD:PA3 
//#define USART_SEL    USART3_SEL
//#define USART_SEL    UART4_SEL
//#define USART_SEL    UART5_SEL
#define USART_REC_LEN   200     /* 定义最大接收字节数 200 */

extern UART_HandleTypeDef g_uart_handle;        /* HAL UART句柄 */

void usart_init(uint32_t bound);                /* 串口初始化函数 */
int16_t uart_rx_read(void);
int16_t uart_rx_peek(uint16_t index);
uint16_t uart_rx_available(void);
bool uart_find_command(char* cmd_buffer, uint16_t buffer_size);

// HAL_UART_Transmit(&g_uart_handle, &ch, 1, 10);
// while(HAL_UART_GetState(&g_uart_handle) == HAL_UART_STATE_BUSY_TX);

#if (USART_SEL == USART1_SEL)
#define USART_UX                            USART1
#define USART_TX_GPIO_PORT                  GPIOA
#define USART_TX_GPIO_PIN                   GPIO_PIN_9
#define USART_RX_GPIO_PORT                  GPIOA
#define USART_RX_GPIO_PIN                   GPIO_PIN_10
#define USART_TX_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)
#define USART_RX_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)
#define USART_UX_CLK_ENABLE()               do{ __HAL_RCC_USART1_CLK_ENABLE(); }while(0)
#define USART_UX_IRQn                       USART1_IRQn
#define USART_UX_IRQHandler                 USART1_IRQHandler
#define USART_AF_FUNCTION                   GPIO_AF7_USART1

#elif (USART_SEL == USART2_SEL)
#define USART_UX                            USART2
#define USART_TX_GPIO_PORT                  GPIOA
#define USART_TX_GPIO_PIN                   GPIO_PIN_2
#define USART_RX_GPIO_PORT                  GPIOA
#define USART_RX_GPIO_PIN                   GPIO_PIN_3
#define USART_TX_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)
#define USART_RX_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)
#define USART_UX_CLK_ENABLE()               do{ __HAL_RCC_USART2_CLK_ENABLE(); }while(0)
#define USART_UX_IRQn                       USART2_IRQn
#define USART_UX_IRQHandler                 USART2_IRQHandler
#define USART_AF_FUNCTION                   GPIO_AF7_USART2

#elif (USART_SEL == USART3_SEL)
#define USART_UX                            USART3
#define USART_TX_GPIO_PORT                  GPIOC
#define USART_TX_GPIO_PIN                   GPIO_PIN_10
#define USART_RX_GPIO_PORT                  GPIOC
#define USART_RX_GPIO_PIN                   GPIO_PIN_11
#define USART_TX_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)
#define USART_RX_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)
#define USART_UX_CLK_ENABLE()               do{ __HAL_RCC_USART3_CLK_ENABLE(); }while(0)
#define USART_UX_IRQn                       USART3_IRQn
#define USART_UX_IRQHandler                 USART3_IRQHandler
#define USART_AF_FUNCTION                   GPIO_AF7_USART3

#elif (USART_SEL == UART4_SEL)
#define USART_UX                            UART4
#define USART_TX_GPIO_PORT                  GPIOC
#define USART_TX_GPIO_PIN                   GPIO_PIN_10
#define USART_RX_GPIO_PORT                  GPIOC
#define USART_RX_GPIO_PIN                   GPIO_PIN_11
#define USART_TX_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)
#define USART_RX_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)
#define USART_UX_CLK_ENABLE()               do{ __HAL_RCC_UART4_CLK_ENABLE(); }while(0)
#define USART_UX_IRQn                       UART4_IRQn
#define USART_UX_IRQHandler                 UART4_IRQHandler
#define USART_AF_FUNCTION                   GPIO_AF8_UART4

#elif (USART_SEL == UART5_SEL)
#define USART_UX                            UART5
#define USART_TX_GPIO_PORT                  GPIOC
#define USART_TX_GPIO_PIN                   GPIO_PIN_12
#define USART_RX_GPIO_PORT                  GPIOD
#define USART_RX_GPIO_PIN                   GPIO_PIN_2
#define USART_TX_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)
#define USART_RX_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)
#define USART_UX_CLK_ENABLE()               do{ __HAL_RCC_UART5_CLK_ENABLE(); }while(0)
#define USART_UX_IRQn                       UART5_IRQn
#define USART_UX_IRQHandler                 UART5_IRQHandler
#define USART_AF_FUNCTION                   GPIO_AF8_UART5

#endif /*USART_SEL */

#ifdef __cplusplus
}
#endif
#endif
