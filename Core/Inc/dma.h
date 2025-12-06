#ifndef __DMA_H
#define __DMA_H

#include "stm32l4xx_hal.h"

// DMA接收缓冲区大小定义
#define DMA_RX_BUFFER_SIZE 64

// USART2 DMA句柄声明
extern DMA_HandleTypeDef g_dma_tx_handle;
extern DMA_HandleTypeDef g_dma_rx_handle;

// DMA接收缓冲区
extern uint8_t g_rx_buffer[DMA_RX_BUFFER_SIZE];  /* DMA接收缓冲区 */

/* USART2 DMA相关函数 */
void dma_usart2_init(void);
void dma_usart2_transmit(uint8_t *data, uint16_t size);
void dma_usart2_start_receive(uint8_t *buffer, uint16_t size);
uint16_t dma_usart2_get_receive_count(void);
uint16_t dma_usart2_get_new_data(uint8_t *buffer, uint16_t max_len);



/* 内存到内存DMA相关函数 */

/**
 * @brief       初始化内存到内存DMA
 * @note        使用DMA1 Channel3
 * @retval      HAL状态值
 */
HAL_StatusTypeDef dma_mem2mem_init(void);

/**
 * @brief       执行内存到内存DMA传输,异步执行,CPU启动DMA传输后立即返回，继续执行其他任务
 * @param       source: 源地址
 * @param       destination: 目标地址
 * @param       data_length: 数据长度（以字节为单位）
 * @retval      HAL状态值
 */
HAL_StatusTypeDef dma_mem2mem_transfer(uint32_t *source, uint32_t *destination, uint32_t data_length);

/**
 * @brief       等待内存到内存的异步DMA传输完成
 * @param       timeout_ms: 超时时间（毫秒）
 * @retval      HAL状态值
 */
HAL_StatusTypeDef dma_mem2mem_wait_for_transfer_complete(uint32_t timeout_ms);

/**
 * @brief       同步执行内存到内存DMA传输（初始化+传输+等待完成）,CPU会等待传输完成
 * @param       source: 源地址
 * @param       destination: 目标地址
 * @param       data_length: 数据长度（以字为单位）
 * @param       timeout_ms: 超时时间（毫秒）
 * @retval      HAL状态值
 */
HAL_StatusTypeDef dma_mem2mem_transfer_sync(uint32_t *source, uint32_t *destination, uint32_t data_length, uint32_t timeout_ms);

/**
 * @brief       获取内存到内存DMA状态
 * @retval      DMA状态
 */
HAL_DMA_StateTypeDef dma_mem2mem_get_state(void);

/* 中断处理函数声明 */
void DMA1_Channel7_IRQHandler(void);
void DMA1_Channel6_IRQHandler(void);
void DMA1_Channel3_IRQHandler(void);

#endif



