#include "dma.h"
#include "usart.h"
#include "sys.h"
#include "stdio.h"  // 添加stdio.h用于printf函数
#include <string.h>

#define DMA_RX_BUFFER_SIZE  64  /* DMA接收缓冲区大小，用于USART2循环接收 */

/* USART2 DMA句柄定义 - 全局变量 */
DMA_HandleTypeDef g_dma_tx_handle;  // USART2发送DMA句柄
DMA_HandleTypeDef g_dma_rx_handle;  // USART2接收DMA句柄
uint8_t g_rx_buffer[DMA_RX_BUFFER_SIZE];  /* DMA接收缓冲区，采用循环模式存储接收到的数据 */

/* 内存到内存DMA句柄 - 静态变量，仅在本文件内使用 */
static DMA_HandleTypeDef g_dma_mem2mem_handle;

/* 函数声明：DMA传输完成回调函数 */
static void dma_mem2mem_transfer_complete_callback(DMA_HandleTypeDef *hdma);
/* 函数声明：DMA传输错误回调函数 */
static void dma_mem2mem_transfer_error_callback(DMA_HandleTypeDef *hdma);

/* 外部引用：UART句柄，由usart.c中定义 */
extern UART_HandleTypeDef g_uart_handle;

extern UART_HandleTypeDef g_uart_handle;

/**
 * @brief       初始化内存到内存DMA通道
 * @details     配置DMA1 Channel3用于内存到内存的数据传输
 *              - 时钟使能：启用DMA1和DMAMUX（如果存在）时钟
 *              - 配置DMA参数：设置为内存到内存模式，地址自动递增，字节对齐
 *              - 中断配置：设置高优先级中断并启用
 *              - 回调注册：注册传输完成和错误回调函数
 * @note        必须在使用内存到内存DMA功能前调用此函数进行初始化
 * @retval      HAL状态值 - HAL_OK表示成功，其他值表示错误
 */
HAL_StatusTypeDef dma_mem2mem_init(void)
{
    /* 使能DMA1控制器时钟，这是使用DMA外设的前提 */
    __HAL_RCC_DMA1_CLK_ENABLE();
    
#if defined(DMAMUX1)
    /* 如果芯片有DMAMUX（DMA多路复用器），也需要使能其时钟 */
    __HAL_RCC_DMAMUX1_CLK_ENABLE();
#endif
    
    /* 配置DMA句柄参数 */
    g_dma_mem2mem_handle.Instance = DMA1_Channel3;                  /* 选择DMA1的Channel3 */
    g_dma_mem2mem_handle.Init.Request = DMA_REQUEST_0;              /* 内存到内存传输使用默认请求通道0 */
    g_dma_mem2mem_handle.Init.Direction = DMA_MEMORY_TO_MEMORY;     /* 传输方向：内存到内存 */
    g_dma_mem2mem_handle.Init.PeriphInc = DMA_PINC_ENABLE;          /* 源地址（外设地址）自增 */
    g_dma_mem2mem_handle.Init.MemInc = DMA_MINC_ENABLE;             /* 目标地址（内存地址）自增 */
    g_dma_mem2mem_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE; /* 源数据宽度：8位（字节） */
    g_dma_mem2mem_handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;    /* 目标数据宽度：8位（字节） */
    g_dma_mem2mem_handle.Init.Mode = DMA_NORMAL;                    /* 传输模式：正常模式（非循环） */
    g_dma_mem2mem_handle.Init.Priority = DMA_PRIORITY_HIGH;         /* 传输优先级：高优先级 */
    
    /* 安全检查：验证DMA实例是否有效 */
    if ((uint32_t)g_dma_mem2mem_handle.Instance == 0) {
        return HAL_ERROR;
    }
    
    /* 调用HAL库函数初始化DMA通道 */
    HAL_StatusTypeDef result = HAL_DMA_Init(&g_dma_mem2mem_handle);
    
    if (result == HAL_OK) {
        /* 初始化成功后，注册中断回调函数 */
        HAL_DMA_RegisterCallback(&g_dma_mem2mem_handle, HAL_DMA_XFER_CPLT_CB_ID, dma_mem2mem_transfer_complete_callback);
        HAL_DMA_RegisterCallback(&g_dma_mem2mem_handle, HAL_DMA_XFER_ERROR_CB_ID, dma_mem2mem_transfer_error_callback);
        
        /* 配置并使能DMA中断 */
        HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 1, 0);  /* 设置中断优先级 */
        HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);          /* 使能中断 */
    } else {
        /* 初始化失败，输出错误信息 */
        printf("HAL_DMA_Init failed with error: %d\r\n", result);
    }
    
    return result;
}

/**
 * @brief       执行内存到内存DMA传输（非阻塞方式）
 * @details     启动一次内存到内存的DMA数据传输操作
 *              - 参数验证：检查指针有效性及数据长度
 *              - 状态检查：确保DMA不处于忙状态
 *              - 启动传输：使用中断方式启动DMA传输
 * @param       source: 源数据缓冲区地址（32位指针）
 * @param       destination: 目标数据缓冲区地址（32位指针）
 * @param       data_length: 要传输的数据长度（以字节为单位）
 * @retval      HAL状态值 
 *              - HAL_OK: 成功启动传输
 *              - HAL_ERROR: 参数无效
 *              - HAL_BUSY: DMA正在执行其他传输
 *              - 其他值: HAL库内部错误
 * @note        此函数为非阻塞式，立即返回，实际传输在后台进行
 *              传输完成可通过回调函数或轮询状态获知
 */
HAL_StatusTypeDef dma_mem2mem_transfer(uint32_t *source, uint32_t *destination, uint32_t data_length)
{
    /* 参数合法性检查 */
    if ((source == NULL) || (destination == NULL) || (data_length == 0)) {
        printf("Invalid parameters: source=%p, destination=%p, data_length=%lu\r\n", 
               source, destination, data_length);
        return HAL_ERROR;
    }
    
    /* 检查当前DMA通道状态，避免冲突 */
    HAL_DMA_StateTypeDef state = HAL_DMA_GetState(&g_dma_mem2mem_handle);
    if (state == HAL_DMA_STATE_BUSY) {
        printf("DMA is busy, state=%d\r\n", state);
        return HAL_BUSY;
    }
    
    /* 输出调试信息，便于追踪传输过程 */
    printf("Starting DMA transfer: source=0x%08X, dest=0x%08X, length=%lu bytes\r\n", 
           (unsigned int)source, (unsigned int)destination, data_length);
    
    /* 使用中断方式启动DMA传输 */
    HAL_StatusTypeDef result = HAL_DMA_Start_IT(&g_dma_mem2mem_handle, 
                                                (uint32_t)source, 
                                                (uint32_t)destination, 
                                                data_length);
    
    /* 检查启动结果 */
    if (result != HAL_OK) {
        printf("HAL_DMA_Start_IT failed with error: %d\r\n", result);
        uint32_t error_code = HAL_DMA_GetError(&g_dma_mem2mem_handle);
        if (error_code != HAL_DMA_ERROR_NONE) {
            printf("DMA error code: %lu\r\n", error_code);
        }
    } else {
        printf("HAL_DMA_Start_IT succeeded\r\n");
    }
    
    return result;
}

/**
 * @brief       等待内存到内存DMA传输完成（阻塞式）
 * @details     轮询等待DMA传输操作结束，带超时机制
 *              - 记录起始时间，开始轮询DMA状态
 *              - 每1ms检查一次状态，避免CPU过度占用
 *              - 达到超时时间则返回超时错误
 *              - 传输完成后检查最终状态并返回结果
 * @param       timeout_ms: 最大等待时间（毫秒）
 * @retval      HAL状态值
 *              - HAL_OK: 传输成功完成
 *              - HAL_TIMEOUT: 等待超时
 *              - HAL_ERROR: 传输过程中发生错误
 * @note        此函数为阻塞式调用，会占用CPU直到传输完成或超时
 *              建议在必须同步等待的场景下使用
 */
HAL_StatusTypeDef dma_mem2mem_wait_for_transfer_complete(uint32_t timeout_ms)
{
    uint32_t tickstart = HAL_GetTick();   // 记录开始等待的时间戳
    uint32_t wait_count = 0;              // 等待循环计数器
    
    printf("Waiting for DMA transfer completion (timeout=%lu ms)\r\n", timeout_ms);
    
    /* 循环等待直到DMA不再忙或超时 */
    while (HAL_DMA_GetState(&g_dma_mem2mem_handle) == HAL_DMA_STATE_BUSY) {
        wait_count++;
        
        /* 检查是否超时 */
        if ((HAL_GetTick() - tickstart) > timeout_ms) {
            printf("DMA wait timeout after %lu ms, wait iterations: %lu\r\n", timeout_ms, wait_count);
            return HAL_TIMEOUT;
        }
        
        /* 添加1ms延迟，降低CPU占用率 */
        HAL_Delay(1);
        
        /* 每100次循环输出一次状态信息，用于调试 */
        if (wait_count % 100 == 0) {
            printf("DMA still busy, wait iterations: %lu\r\n", wait_count);
        }
    }
    
    /* 获取最终的DMA状态 */
    HAL_DMA_StateTypeDef state = HAL_DMA_GetState(&g_dma_mem2mem_handle);
    printf("DMA transfer finished with state: %d, wait iterations: %lu\r\n", state, wait_count);
    
    /* 根据状态返回相应结果 */
    if (state == HAL_DMA_STATE_READY) {
        return HAL_OK;  // 传输成功完成
    } else {
        printf("DMA transfer error. State: %d\r\n", state);
        // 特别处理超时情况下的错误码获取
        if (state == HAL_DMA_STATE_TIMEOUT) {
            uint32_t error_code = HAL_DMA_GetError(&g_dma_mem2mem_handle);
            if (error_code != HAL_DMA_ERROR_NONE) {
                printf("DMA error code: %lu\r\n", error_code);
            }
        }
        return HAL_ERROR;
    }
}

/**
 * @brief       同步执行内存到内存DMA传输
 * @details     封装了完整的DMA传输流程：启动传输 + 等待完成
 *              这是一个阻塞式函数，直到传输完成或超时才返回
 * @param       source: 源数据缓冲区地址
 * @param       destination: 目标数据缓冲区地址
 * @param       data_length: 数据长度（以字节为单位）
 * @param       timeout_ms: 等待传输完成的超时时间（毫秒）
 * @retval      HAL状态值
 *              - HAL_OK: 传输成功完成
 *              - HAL_BUSY: DMA忙，无法启动
 *              - HAL_TIMEOUT: 传输超时
 *              - HAL_ERROR: 参数错误或传输失败
 * @note        此函数按顺序调用：
 *              1. dma_mem2mem_transfer() 启动传输
 *              2. dma_mem2mem_wait_for_transfer_complete() 等待完成
 *              适合需要同步完成数据传输的场合
 */
HAL_StatusTypeDef dma_mem2mem_transfer_sync(uint32_t *source, uint32_t *destination, uint32_t data_length, uint32_t timeout_ms)
{
    HAL_StatusTypeDef status;
    
    /* 启动DMA传输 */
    status = dma_mem2mem_transfer(source, destination, data_length);
    if (status != HAL_OK) {
        printf("DMA transfer start failed with status: %d\r\n", status);
        return status;
    }
    
    /* 等待传输完成 */
    status = dma_mem2mem_wait_for_transfer_complete(timeout_ms);
    if (status != HAL_OK) {
        printf("DMA transfer wait failed with status: %d\r\n", status);
        printf("DMA current state: %d\r\n", dma_mem2mem_get_state());
    }
    
    return status;
}

/**
 * @brief       获取内存到内存DMA通道的当前状态
 * @param       无
 * @retval      DMA状态枚举值
 *              可能的值包括：
 *              - HAL_DMA_STATE_RESET: 未初始化
 *              - HAL_DMA_STATE_READY: 准备就绪
 *              - HAL_DMA_STATE_BUSY: 传输进行中
 *              - HAL_DMA_STATE_TIMEOUT: 传输超时
 *              - HAL_DMA_STATE_ERROR: 传输错误
 *              - HAL_DMA_STATE_ABORT: 传输被中止
 * @note        可用于监控DMA传输进度或调试问题
 */
HAL_DMA_StateTypeDef dma_mem2mem_get_state(void)
{
    return HAL_DMA_GetState(&g_dma_mem2mem_handle);
}

/* DMA传输完成回调函数 */
static void dma_mem2mem_transfer_complete_callback(DMA_HandleTypeDef *hdma)
{
    printf("DMA transfer complete callback called\r\n");
}

/* DMA传输错误回调函数 */
static void dma_mem2mem_transfer_error_callback(DMA_HandleTypeDef *hdma)
{
    printf("DMA transfer error callback called\r\n");
}

/**
 * @brief  This function handles DMA1 Channel3 interrupt request.
 * @param  None
 * @retval None
 */
void DMA1_Channel3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&g_dma_mem2mem_handle);
}

/**
 * @brief       初始化USART2的DMA功能
 * @details     配置USART2发送和接收使用的DMA通道
 *              - TX: 使用DMA1 Channel7，内存到外设，正常模式
 *              - RX: 使用DMA1 Channel6，外设到内存，循环模式
 *              - 配置中断优先级并关联到UART句柄
 * @note        必须在使用USART2 DMA功能前调用
 *              接收采用循环模式，配合dma_usart2_get_new_data()实现高效接收
 * @retval      无
 */
void dma_usart2_init(void)
{
    /* 使能DMA1时钟 */
    __HAL_RCC_DMA1_CLK_ENABLE();
    
    /* 配置TX DMA */
    g_dma_tx_handle.Instance = DMA1_Channel7;
    g_dma_tx_handle.Init.Request = DMA_REQUEST_2;          /* USART2_TX */
    g_dma_tx_handle.Init.Direction = DMA_MEMORY_TO_PERIPH; /* 内存到外设 */
    g_dma_tx_handle.Init.PeriphInc = DMA_PINC_DISABLE;     /* 外设地址不自增 */
    g_dma_tx_handle.Init.MemInc = DMA_MINC_ENABLE;         /* 内存地址自增 */
    g_dma_tx_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE; /* 数据宽度8位 */
    g_dma_tx_handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    g_dma_tx_handle.Init.Mode = DMA_NORMAL;                /* 正常模式 */
    g_dma_tx_handle.Init.Priority = DMA_PRIORITY_MEDIUM;   /* 中等优先级 */
    
    if(HAL_DMA_Init(&g_dma_tx_handle) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* 配置RX DMA */
    g_dma_rx_handle.Instance = DMA1_Channel6;
    g_dma_rx_handle.Init.Request = DMA_REQUEST_2;          /* USART2_RX */
    g_dma_rx_handle.Init.Direction = DMA_PERIPH_TO_MEMORY; /* 外设到内存 */
    g_dma_rx_handle.Init.PeriphInc = DMA_PINC_DISABLE;     /* 外设地址不自增 */
    g_dma_rx_handle.Init.MemInc = DMA_MINC_ENABLE;         /* 内存地址自增 */
    g_dma_rx_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE; /* 数据宽度8位 */
    g_dma_rx_handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    g_dma_rx_handle.Init.Mode = DMA_CIRCULAR;              /* 循环模式 */
    g_dma_rx_handle.Init.Priority = DMA_PRIORITY_HIGH;     /* 高优先级 */
    
    if(HAL_DMA_Init(&g_dma_rx_handle) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* 关联DMA句柄到UART句柄 */
    __HAL_LINKDMA(&g_uart_handle, hdmatx, g_dma_tx_handle);
    __HAL_LINKDMA(&g_uart_handle, hdmarx, g_dma_rx_handle);
    
    /* 配置DMA中断优先级 */
    HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 1, 1);  /* DMA接收中断 */
    HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
    
    HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 1, 2);  /* DMA发送中断 */
    HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
}

/**
 * @brief       使用DMA发送数据通过USART2
 * @details     通过DMA方式异步发送数据，提高CPU效率
 *              - 先等待前一次传输完成
 *              - 调用HAL库DMA发送函数
 * @param       data: 指向要发送数据的指针
 * @param       size: 要发送的数据字节数
 * @retval      无
 * @note        此函数为非阻塞式，立即返回
 *              发送完成可通过中断或轮询判断
 */
void dma_usart2_transmit(uint8_t *data, uint16_t size)
{
    /* 等待上一次传输完成 */
    while(HAL_UART_GetState(&g_uart_handle) == HAL_UART_STATE_BUSY_TX);
    
    if(HAL_UART_Transmit_DMA(&g_uart_handle, data, size) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief       启动USART2的DMA接收功能
 * @details     配置DMA接收缓冲区并启动循环接收模式
 *              一旦启动，DMA将持续接收数据填入缓冲区
 * @param       buffer: 接收数据缓冲区指针
 * @param       size:   缓冲区大小（字节数）
 * @retval      无
 * @note        通常只需要调用一次，在系统初始化时
 *              使用循环模式，结合dma_usart2_get_new_data()获取新数据
 */
void dma_usart2_start_receive(uint8_t *buffer, uint16_t size)
{
    /* 启动DMA接收 */
    if(HAL_UART_Receive_DMA(&g_uart_handle, buffer, size) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief       获取当前已接收的数据量
 * @param       无
 * @retval      已经接收到的数据字节数
 * @note        基于DMA剩余计数值计算得出
 *              在循环模式下，该值可能会超过缓冲区大小
 */
uint16_t dma_usart2_get_receive_count(void)
{
    return (sizeof(g_rx_buffer) - __HAL_DMA_GET_COUNTER(&g_dma_rx_handle));
}

/**
 * @brief       获取DMA接收的新数据
 * @details     从循环接收缓冲区中提取自上次调用以来的新接收数据
 *              处理了数据在缓冲区末尾环绕的情况
 * @param       buffer: 用户提供的缓冲区，用于存放获取到的数据
 * @param       max_len: 用户缓冲区的最大容量
 * @retval      实际成功获取的数据字节数
 * @note        这是高效获取串口数据的关键函数
 *              需要配合静态变量last_pos记录上一次读取位置
 */
uint16_t dma_usart2_get_new_data(uint8_t *buffer, uint16_t max_len)
{
    static uint16_t last_pos = 0;
    uint16_t curr_pos;
    uint16_t data_len = 0;
    
    /* 获取当前DMA写入位置 */
    curr_pos = DMA_RX_BUFFER_SIZE - __HAL_DMA_GET_COUNTER(&g_dma_rx_handle);
    
    /* 计算新接收到的数据长度 */
    if (curr_pos != last_pos) {
        if (curr_pos > last_pos) {
            /* 数据在缓冲区中是连续的 */
            data_len = curr_pos - last_pos;
            if (data_len > max_len) {
                data_len = max_len;
            }
            memcpy(buffer, &g_rx_buffer[last_pos], data_len);
        } else {
            /* 数据环绕缓冲区末端 */
            uint16_t first_part_len = DMA_RX_BUFFER_SIZE - last_pos;
            if (first_part_len > max_len) {
                data_len = max_len;
                memcpy(buffer, &g_rx_buffer[last_pos], data_len);
            } else {
                data_len = first_part_len;
                memcpy(buffer, &g_rx_buffer[last_pos], data_len);
                
                uint16_t second_part_len = curr_pos;
                uint16_t remaining_len = max_len - data_len;
                if (second_part_len > remaining_len) {
                    second_part_len = remaining_len;
                }
                
                if (second_part_len > 0) {
                    memcpy(&buffer[data_len], &g_rx_buffer[0], second_part_len);
                    data_len += second_part_len;
                }
            }
        }
        
        /* 更新last_pos */
        last_pos = curr_pos;
    }
    
    return data_len;
}

/**
 * @brief  DMA1 Channel7中断服务函数
 * @note   用于处理USART2发送DMA中断
 *         调用HAL库的DMA中断处理函数
 * @param  None
 * @retval None
 */
void DMA1_Channel7_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&g_dma_tx_handle);
}

/**
 * @brief  DMA1 Channel6中断服务函数
 * @note   用于处理USART2接收DMA中断
 *         调用HAL库的DMA中断处理函数
 * @param  None
 * @retval None
 */
void DMA1_Channel6_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&g_dma_rx_handle);
}
