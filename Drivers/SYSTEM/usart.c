#include "sys.h"
#include "usart.h"


/* 如果使用os,则包括下面的头文件即可. */
#if SYS_SUPPORT_OS
#include "os.h" /* os 使用 */
#endif

/******************************************************************************************/
/* 加入以下代码, 支持printf函数, 而不需要选择use MicroLIB */

#if 1

#if (__ARMCC_VERSION >= 6010050)            /* 使用AC6编译器时 */
__asm(".global __use_no_semihosting\n\t");  /* 声明不使用半主机模式 */
__asm(".global __ARM_use_no_argv \n\t");    /* AC6下需要声明main函数为无参数格式，否则部分例程可能出现半主机模式 */

#else
/* 使用AC5编译器时, 要在这里定义__FILE 和 不使用半主机模式 */
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */
    /* standard output using printf() for debugging, no file handling */
    /* is required. */
};

#endif

/* 不使用半主机模式，至少需要重定义_ttywrch\_sys_exit\_sys_command_string函数,以同时兼容AC6和AC5模式 */
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

/* 定义_sys_exit()以避免使用半主机模式 */
void _sys_exit(int x)
{
    x = x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}


/* FILE 在 stdio.h里面定义. */
FILE __stdout;

/* MDK下需要重定义fputc函数, printf函数最终会通过调用fputc输出字符串到串口 */
int fputc(int ch, FILE *f)
{
    while ((USART_UX->ISR & UART_FLAG_TXE) == 0); /* 等待上一个字符发送完成 */

    USART_UX->TDR = (uint8_t)ch;                  /* 将要发送的字符 ch 写入到TDR寄存器 */
    return ch;
}
#endif
/******************************************************************************************/



/* 定义环形缓冲区大小 */
#define UART_RX_BUFFER_SIZE 256

/* 环形缓冲区结构体 */
typedef struct {
    uint8_t buffer[UART_RX_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint16_t count;
} uart_rx_buffer_t;

uint8_t g_rx_buffer[1];  /* HAL库使用的串口接收缓冲(只需要1个字节用于中断接收) */

UART_HandleTypeDef g_uart_handle;  /* UART句柄 */

/* 全局环形缓冲区变量 */
static uart_rx_buffer_t g_uart_rx_buf = {0};

/**
 * @brief       串口X初始化函数
 * @param       baudrate: 波特率, 根据自己需要设置波特率值
 * @note        注意: 必须设置正确的时钟源, 否则串口波特率就会设置异常.
 *              这里的USART的时钟源在sys_stm32_clock_init()函数中已经设置过了.
 * @retval      无
 */
void usart_init(uint32_t baudrate)
{
    /*UART 初始化设置*/
    g_uart_handle.Instance = USART_UX;                              /* USART_UX */
    g_uart_handle.Init.BaudRate = baudrate;                         /* 波特率 */
    g_uart_handle.Init.WordLength = UART_WORDLENGTH_8B;             /* 字长为8位数据格式 */
    g_uart_handle.Init.StopBits = UART_STOPBITS_1;                  /* 一个停止位 */
    g_uart_handle.Init.Parity = UART_PARITY_NONE;                   /* 无奇偶校验位 */
    g_uart_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;             /* 无硬件流控 */
    g_uart_handle.Init.Mode = UART_MODE_TX_RX;                      /* 收发模式 */
    g_uart_handle.Init.OverSampling = UART_OVERSAMPLING_16;         /* 过采样设置 */
    g_uart_handle.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;/* 一位采样 */
    
    if (HAL_UART_Init(&g_uart_handle) != HAL_OK)
    {
        Error_Handler();
    }
    
    /* 该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量 */
    HAL_UART_Receive_IT(&g_uart_handle, (uint8_t *)g_rx_buffer, 1); 
}

/**
 * @brief       UART底层初始化函数
 * @param       huart: UART句柄类型指针
 * @note        此函数会被HAL_UART_Init()调用
 *              完成时钟使能，引脚配置，中断配置
 * @retval      无
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio_init_struct;

    if (huart->Instance == USART_UX)                                /* 如果是设定的串口 */
    {
        USART_TX_GPIO_CLK_ENABLE();                                 /* 使能串口TX脚时钟 */
        USART_RX_GPIO_CLK_ENABLE();                                 /* 使能串口RX脚时钟 */
        USART_UX_CLK_ENABLE();                                      /* 使能串口时钟 */

        gpio_init_struct.Pin = USART_TX_GPIO_PIN;                   /* 串口发送引脚号 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;                    /* 复用推挽输出 */
        gpio_init_struct.Pull = GPIO_PULLUP;                        /* 上拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;         /* IO速度设置为高速 */
        gpio_init_struct.Alternate = USART_AF_FUNCTION;             /* 复用功能 */
        HAL_GPIO_Init(USART_TX_GPIO_PORT, &gpio_init_struct);
        
        gpio_init_struct.Pin = USART_RX_GPIO_PIN;                   /* 串口RX脚 模式设置 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;                    /* 复用推挽输入 */
        gpio_init_struct.Pull = GPIO_PULLUP;                        /* 上拉 */
        HAL_GPIO_Init(USART_RX_GPIO_PORT, &gpio_init_struct);       /* 串口RX脚 必须设置成复用模式 */
        
    
        HAL_NVIC_SetPriority(USART_UX_IRQn, 3, 3);                  /* 设置优先级:抢占优先级3，子优先级3 */
        HAL_NVIC_EnableIRQ(USART_UX_IRQn);                          /* 使能USART中断通道 */
    }
}

/**
 * @brief       串口数据接收回调函数
                数据处理在这里进行
 * @param       huart:串口句柄
 * @retval      无
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART_UX)                    /* 如果是设定的串口 */
    {
        /* 将接收到的数据存入环形缓冲区 */
        if (g_uart_rx_buf.count < UART_RX_BUFFER_SIZE) {
            g_uart_rx_buf.buffer[g_uart_rx_buf.head] = g_rx_buffer[0];
            g_uart_rx_buf.head = (g_uart_rx_buf.head + 1) % UART_RX_BUFFER_SIZE;
            g_uart_rx_buf.count++;
        }
        /* 重新启动接收中断 */
        HAL_UART_Receive_IT(&g_uart_handle, (uint8_t *)g_rx_buffer, 1);
    }
}

/**
 * @brief       从环形缓冲区读取一个字节
 * @note        会移除已经读取的字符
 * @retval      读取到的字节，如果没有数据则返回-1
 */
int16_t uart_rx_read(void)
{
    int16_t data = -1;
    
    if (g_uart_rx_buf.count > 0) {
        data = g_uart_rx_buf.buffer[g_uart_rx_buf.tail];
        g_uart_rx_buf.tail = (g_uart_rx_buf.tail + 1) % UART_RX_BUFFER_SIZE;
        g_uart_rx_buf.count--;
    }
    
    return data;
}

/**
 * @brief       预览环形缓冲区中的数据但不移除
 * @param       index: 要预览的数据索引（从0开始）
 * @retval      数据值，如果索引超出范围则返回-1
 */
int16_t uart_rx_peek(uint16_t index)
{
    int16_t data = -1;
    
    if (index < g_uart_rx_buf.count) {
        uint16_t pos = (g_uart_rx_buf.tail + index) % UART_RX_BUFFER_SIZE;
        data = g_uart_rx_buf.buffer[pos];
    }
    
    return data;
}

/**
 * @brief       获取环形缓冲区中数据的数量
 * @retval      缓冲区中数据的数量
 */
uint16_t uart_rx_available(void)
{
    return g_uart_rx_buf.count;
}

/**
 * @brief       UART错误回调函数
 * @param       huart: UART句柄
 * @retval      无
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART_UX)
    {
        /* 清除错误标志 */
        __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF | UART_CLEAR_NEF | UART_CLEAR_PEF | UART_CLEAR_FEF);
        
        /* 重新启动接收中断 */
        HAL_UART_Receive_IT(&g_uart_handle, (uint8_t *)g_rx_buffer, 1);
    }
}

/**
 * @brief       串口中断服务函数
 * @param       无
 * @retval      无
 */
void USART_UX_IRQHandler(void)
{
#if SYS_SUPPORT_OS                          /* 使用OS */
    OSIntEnter();    
#endif

    HAL_UART_IRQHandler(&g_uart_handle);   /* 调用HAL库中断处理公用函数 */

#if SYS_SUPPORT_OS                          /* 使用OS */
    OSIntExit();
#endif

}

/**
 * @brief       在环形缓冲区中查找命令
 * @param       cmd_buffer: 用于存储命令的缓冲区
 * @param       buffer_size: 命令缓冲区大小
 * @note        请确保cmd_buffer和缓冲区中的数据是UTF-8编码的,以回车换行结尾
 * @retval      1: 找到命令, 0: 未找到命令
 */
bool uart_find_command(char* cmd_buffer, uint16_t buffer_size)
{
    uint16_t i, j;
    uint16_t count = g_uart_rx_buf.count;
    
    // 如果缓冲区中的数据少于最小命令长度，则直接返回
    if (count < 4) {
        // 如果只有少量字符且都是换行符，则清理掉
        if (count > 0) {
            uint16_t temp_tail = g_uart_rx_buf.tail;
            uint8_t only_newlines = 1;
            
            for (i = 0; i < count; i++) {
                uint8_t ch = g_uart_rx_buf.buffer[temp_tail];
                if (ch != '\r' && ch != '\n') {
                    only_newlines = 0;
                    break;
                }
                temp_tail = (temp_tail + 1) % UART_RX_BUFFER_SIZE;
            }
            
            // 如果只有换行符，清理掉它们
            if (only_newlines) {
                g_uart_rx_buf.count = 0;
                g_uart_rx_buf.tail = g_uart_rx_buf.head;
            }
        }
        return 0;  // 最小命令长度为4 ("Num+" 或 "Num-")
    }
    
    // 创建一个临时缓冲区来存储当前环形缓冲区的内容
    uint8_t temp_buffer[UART_RX_BUFFER_SIZE];
    uint16_t temp_tail = g_uart_rx_buf.tail;
    
    // 将环形缓冲区中的数据复制到临时线性缓冲区
    for (i = 0; i < count && i < UART_RX_BUFFER_SIZE; i++) {
        temp_buffer[i] = g_uart_rx_buf.buffer[temp_tail];
        temp_tail = (temp_tail + 1) % UART_RX_BUFFER_SIZE;
    }
    
    // 在临时缓冲区中查找命令结束符
    for (i = 0; i < count && i < UART_RX_BUFFER_SIZE - 1; i++) {
        if (temp_buffer[i] == '\r' || temp_buffer[i] == '\n') {
            // 找到命令结束符，检查前面是否有足够的字符构成有效命令
            if (i >= 4) {  // 至少需要4个字符
                // 复制命令到命令缓冲区
                uint16_t copy_len = (i < buffer_size - 1) ? i : buffer_size - 1;
                for (j = 0; j < copy_len; j++) {
                    cmd_buffer[j] = temp_buffer[j];
                }
                cmd_buffer[copy_len] = '\0';
                
                // 从环形缓冲区中移除已处理的数据（包括结束符）
                for (j = 0; j <= i; j++) {
                    if (g_uart_rx_buf.count > 0) {
                        g_uart_rx_buf.tail = (g_uart_rx_buf.tail + 1) % UART_RX_BUFFER_SIZE;
                        g_uart_rx_buf.count--;
                    }
                }
                return 1;  // 找到命令
            } else {
                // 命令太短，移除无效字符
                for (j = 0; j <= i; j++) {
                    if (g_uart_rx_buf.count > 0) {
                        g_uart_rx_buf.tail = (g_uart_rx_buf.tail + 1) % UART_RX_BUFFER_SIZE;
                        g_uart_rx_buf.count--;
                    }
                }
            }
        }
    }
    
    return 0;  // 未找到完整命令
}



