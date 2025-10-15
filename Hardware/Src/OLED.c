//OLED.c
#include "stm32l4xx_hal.h"
#include "OLED.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

/**
  * 数据存储格式：
  * 纵向8点，高位在下，先从左到右，再从上到下
  * 每一个Bit对应一个像素点
  * 
  *      B0 B0                  B0 B0
  *      B1 B1                  B1 B1
  *      B2 B2                  B2 B2
  *      B3 B3  ------------->  B3 B3 --
  *      B4 B4                  B4 B4  |
  *      B5 B5                  B5 B5  |
  *      B6 B6                  B6 B6  |
  *      B7 B7                  B7 B7  |
  *                                    |
  *  -----------------------------------
  *  |   
  *  |   B0 B0                  B0 B0
  *  |   B1 B1                  B1 B1
  *  |   B2 B2                  B2 B2
  *  --> B3 B3  ------------->  B3 B3
  *      B4 B4                  B4 B4
  *      B5 B5                  B5 B5
  *      B6 B6                  B6 B6
  *      B7 B7                  B7 B7
  * 
  * 坐标轴定义：
  * 左上角为(0, 0)点
  * 横向向右为X轴，取值范围：0~127
  * 纵向向下为Y轴，取值范围：0~63
  * 
  *       0             X轴           127 
  *      .------------------------------->
  *    0 |
  *      |
  *      |
  *      |
  *  Y轴 |
  *      |
  *      |
  *      |
  *   63 |
  *      v
  * 
  */


// OLED I2C地址
#define OLED_ADDRESS 0x78

#ifdef OLED_USE_Hardware
    // I2C句柄定义
    #if (I2C_PERIPH == 1)
        I2C_HandleTypeDef hi2c1;
        #define OLED_I2C_HANDLE hi2c1
    #elif (I2C_PERIPH == 2)
        I2C_HandleTypeDef hi2c2;
        #define OLED_I2C_HANDLE hi2c2
    #elif (I2C_PERIPH == 3)
        I2C_HandleTypeDef hi2c3;
        #define OLED_I2C_HANDLE hi2c3
    #elif (I2C_PERIPH == 4)
        I2C_HandleTypeDef hi2c4;
        #define OLED_I2C_HANDLE hi2c4
    #else
        #error "请在OLED.h中定义正确的I2C接口"
    #endif
#endif



#ifdef Using_Video_Memory
	/**
	 * OLED显存数组
	 * 所有的显示函数，都只是对此显存数组进行读写
	 * 随后调用OLED_Update函数或OLED_UpdateArea函数
	 * 才会将显存数组的数据发送到OLED硬件，进行显示
	 */
	uint8_t OLED_DisplayBuf[8][128];
#endif

// 使用硬件I2C
#ifdef OLED_USE_Hardware
	#include "sys.h"
	/**
	 * @brief：针对STM32L431的硬件I2C配置
	 * @param：无
	 * @retval：无
	 */
	void OLED_GPIO_Init(void)
	{
		// 配置I2C引脚
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		
		// 根据选择的I2C接口配置相应的引脚
		#if (I2C_PERIPH == 1)
			// 启用I2C1时钟和相关GPIO时钟
			__HAL_RCC_I2C1_CLK_ENABLE();
			__HAL_RCC_GPIOA_CLK_ENABLE();
			
			// PA9 (SCL) 和 PA10 (SDA) 配置成"复用开漏"模式
			GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
			
			// 初始化I2C1外设
			OLED_I2C_HANDLE.Instance = I2C1;
			OLED_I2C_HANDLE.Init.Timing = 0x10909CEC;  // 从STM32CubeMX生成的值
			OLED_I2C_HANDLE.Init.OwnAddress1 = 0;
			OLED_I2C_HANDLE.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
			OLED_I2C_HANDLE.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
			OLED_I2C_HANDLE.Init.OwnAddress2 = 0;
			OLED_I2C_HANDLE.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
			OLED_I2C_HANDLE.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
			OLED_I2C_HANDLE.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
			if (HAL_I2C_Init(&OLED_I2C_HANDLE) != HAL_OK)
			{
				Error_Handler();
			}
			
			// 配置模拟滤波器
			if (HAL_I2CEx_ConfigAnalogFilter(&OLED_I2C_HANDLE, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
			{
				Error_Handler();
			}
			
			// 配置数字滤波器
			if (HAL_I2CEx_ConfigDigitalFilter(&OLED_I2C_HANDLE, 0) != HAL_OK)
			{
				Error_Handler();
			}
		#elif (I2C_PERIPH == 2)
			__HAL_RCC_I2C2_CLK_ENABLE();
			__HAL_RCC_GPIOB_CLK_ENABLE();
			GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
			
			// 初始化I2C2外设
			OLED_I2C_HANDLE.Instance = I2C2;
			OLED_I2C_HANDLE.Init.Timing = 0x10909CEC;
			OLED_I2C_HANDLE.Init.OwnAddress1 = 0;
			OLED_I2C_HANDLE.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
			OLED_I2C_HANDLE.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
			OLED_I2C_HANDLE.Init.OwnAddress2 = 0;
			OLED_I2C_HANDLE.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
			OLED_I2C_HANDLE.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
			OLED_I2C_HANDLE.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
			if (HAL_I2C_Init(&OLED_I2C_HANDLE) != HAL_OK)
			{
				Error_Handler();
			}
			
			if (HAL_I2CEx_ConfigAnalogFilter(&OLED_I2C_HANDLE, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
			{
				Error_Handler();
			}
			
			if (HAL_I2CEx_ConfigDigitalFilter(&OLED_I2C_HANDLE, 0) != HAL_OK)
			{
				Error_Handler();
			}
		#elif (I2C_PERIPH == 3)
			// I2C3 使用 PC0 (SCL) 和 PC1 (SDA)
			__HAL_RCC_I2C3_CLK_ENABLE();
			__HAL_RCC_GPIOC_CLK_ENABLE();
			
			// 配置 SCL (PC0)
			GPIO_InitStruct.Pin = GPIO_PIN_0;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
			HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
			
			// 配置 SDA (PC1)
			GPIO_InitStruct.Pin = GPIO_PIN_1;
			HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
			
			// 初始化I2C3外设
			OLED_I2C_HANDLE.Instance = I2C3;
			OLED_I2C_HANDLE.Init.Timing = 0x10909CEC;
			OLED_I2C_HANDLE.Init.OwnAddress1 = 0;
			OLED_I2C_HANDLE.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
			OLED_I2C_HANDLE.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
			OLED_I2C_HANDLE.Init.OwnAddress2 = 0;
			OLED_I2C_HANDLE.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
			OLED_I2C_HANDLE.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
			OLED_I2C_HANDLE.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
			if (HAL_I2C_Init(&OLED_I2C_HANDLE) != HAL_OK)
			{
				Error_Handler();
			}
			
			if (HAL_I2CEx_ConfigAnalogFilter(&OLED_I2C_HANDLE, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
			{
				Error_Handler();
			}
			
			if (HAL_I2CEx_ConfigDigitalFilter(&OLED_I2C_HANDLE, 0) != HAL_OK)
			{
				Error_Handler();
			}
		#endif
	}

	/**
	 * @brief：I2C接收一个字节
	 * @param：RegAddress 要读取的寄存器地址
	 * @retval：接收到的数据
	 */
	uint8_t OLED_I2C_ReceiverByte(uint8_t RegAddress)
	{
		uint8_t Data;
		// 使用HAL库函数读取数据
		HAL_I2C_Master_Receive(&OLED_I2C_HANDLE, OLED_ADDRESS, &Data, 1, 100);
		return Data;		
	}

#else
	// 使用软件I2C实现
	// 软件I2C引脚操作宏定义
	#define OLED_SCL_HIGH()  do { OLED_SCL_PORT->BSRR = OLED_SCL_PIN; } while(0)
	#define OLED_SCL_LOW()   do { OLED_SCL_PORT->BRR  = OLED_SCL_PIN; } while(0)
	#define OLED_SDA_HIGH()  do { OLED_SDA_PORT->BSRR = OLED_SDA_PIN; } while(0)
	#define OLED_SDA_LOW()   do { OLED_SDA_PORT->BRR  = OLED_SDA_PIN; } while(0)
	#define OLED_SDA_READ()  ((OLED_SDA_PORT->IDR & OLED_SDA_PIN) ? 1U : 0U)
	
    void OLED_GPIO_Init(void)
	{
		 GPIO_InitTypeDef GPIO_InitStruct = {0};
        
        // 使能GPIOA时钟
        __HAL_RCC_GPIOA_CLK_ENABLE();
        
        // 配置SCL和SDA引脚
        GPIO_InitStruct.Pin = OLED_SCL_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;  // 开漏输出
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        HAL_GPIO_Init(OLED_SCL_PORT, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = OLED_SDA_PIN;
		HAL_GPIO_Init(OLED_SDA_PORT, &GPIO_InitStruct);

        // 初始化引脚为高电平
        OLED_SCL_HIGH();
        OLED_SDA_HIGH();
	}

	static void OLED_Delay(void) {
		for (volatile int i = 0; i < 20; i++);
	}
    
	/**
      * @brief  OLED软件I2C起始信号
      * @param  无
      * @retval 无
      */
    void OLED_I2C_Start(void)
    {
        OLED_SDA_HIGH();
        OLED_SCL_HIGH();
        OLED_Delay();
        OLED_SDA_LOW();
        OLED_Delay();
        OLED_SCL_LOW();
    }

    /**
      * @brief  OLED软件I2C停止信号
      * @param  无
      * @retval 无
      */
    void OLED_I2C_Stop(void)
    {
        OLED_SCL_LOW();
        OLED_SDA_LOW();
        OLED_Delay();
        OLED_SCL_HIGH();
        OLED_Delay();
        OLED_SDA_HIGH();
    }

    /**
      * @brief  OLED软件I2C发送一个字节
      * @param  Byte 要发送的字节
      * @retval 无
      */
    void OLED_I2C_SendByte(uint8_t Byte)
    {
        uint8_t i;
        OLED_SCL_LOW(); // 确保在发送数据前时钟线为低
        
        for (i = 0; i < 8; i++)
        {
            if (Byte & 0x80)
            {
                OLED_SDA_HIGH();
            }
            else
            {
                OLED_SDA_LOW();
            }
            OLED_Delay();
            OLED_SCL_HIGH();
            OLED_Delay();
            OLED_SCL_LOW();
            if (i == 7)
            {
                OLED_SDA_HIGH(); // 释放SDA线，准备接收ACK
            }
            Byte <<= 1;
            OLED_Delay();
        }
        
        // 接收ACK
        OLED_SCL_HIGH();
        OLED_Delay();
        OLED_SCL_LOW();
        OLED_Delay();
    }
#endif // OLED_USE_Hardware


/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令值，范围：0x00~0xFF
  * @retval 无
  */
void OLED_WriteCommand(uint8_t Command)
{
#ifndef OLED_USE_Hardware
    OLED_I2C_Start();
    OLED_I2C_SendByte(OLED_ADDRESS);      // 发送设备地址
    OLED_I2C_SendByte(0x00);              // 控制字节，0x00表示命令
    OLED_I2C_SendByte(Command);           // 发送命令
    OLED_I2C_Stop();
#else
    uint8_t data[2] = {0x00, Command};
    HAL_I2C_Master_Transmit(&OLED_I2C_HANDLE, OLED_ADDRESS, data, 2, 100);
#endif
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入的数据
  * @retval 无
  */
void OLED_WriteData(uint8_t Data)
{
#ifndef OLED_USE_Hardware
    OLED_I2C_Start();
    OLED_I2C_SendByte(OLED_ADDRESS);      // 发送设备地址
    OLED_I2C_SendByte(0x40);              // 控制字节，0x40表示数据
    OLED_I2C_SendByte(Data);              // 发送数据
    OLED_I2C_Stop();
#else
    uint8_t data[2] = {0x40, Data};
    HAL_I2C_Master_Transmit(&OLED_I2C_HANDLE, OLED_ADDRESS, data, 2, 100);
#endif
}


/**
  * @brief  OLED初始化
  * @param  无
  * @retval 无
  */
void OLED_Init(void)
{
	uint8_t i, j;

	for (i = 0; i < 100; i++)			//上电延时
	{
		for (j = 0; j < 100; j++);
	}

	OLED_GPIO_Init();			//端口初始化

	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始行
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控制
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示
		
	OLED_Clear();				//OLED清屏
	#ifdef Using_Video_Memory
		OLED_Update();				//更新显示，清屏，防止初始化后未显示内容时花屏
	#endif
}
	
	
/**
  * @brief：OLED设置显示光标位置
  * @param：Page 指定光标所在的页，范围：0~7
  * @param：X 指定光标所在的X轴坐标，范围：0~127
  * @retval：无
  * @note：OLED默认的Y轴，只能8个Bit为一组写入，即1页等于8个Y轴坐标
  */
void OLED_SetCursor(uint8_t Page, uint8_t X)
{
	/*如果使用此程序驱动1.3寸的OLED显示屏，则需要解除此注释*/
	/*因为1.3寸的OLED驱动芯片（SH1106）有132列*/
	/*屏幕的起始列接在了第2列，而不是第0列*/
	/*所以需要将X加2，才能正常显示*/
	#ifdef Drive_SH1106
		X += 2;
	#endif
	
	/*通过指令设置页地址和列地址*/
	OLED_WriteCommand(0xB0 | Page);					//设置页位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}




/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;	//结果默认为1
	while (Y--)
	{
		Result *= X;		//累乘Y次
	}
	return Result;			//每次把X累乘到结果上
}



#ifdef Using_Video_Memory
	/**
	  * @brief：判断指定点是否在指定多边形内部
	  * @param：nvert 多边形的顶点数
	  * @param：vertx verty 包含多边形顶点的x和y坐标的数组
	  * @param：testx testy 测试点的X和y坐标
	  * @retval：指定点是否在指定多边形内部，1：在内部，0：不在内部
	  */
	uint8_t OLED_pnpoly(uint8_t nvert, int16_t *vertx, int16_t *verty, int16_t testx, int16_t testy)
	{
		int16_t i, j, c = 0;
		
		/*此算法由W. Randolph Franklin提出*/
		/*参考链接：https://wrfranklin.org/Research/Short_Notes/pnpoly.html*/
		for (i = 0, j = nvert - 1; i < nvert; j = i++)
		{
			if (((verty[i] > testy) != (verty[j] > testy)) &&
				(testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
			{
				c = !c;
			}
		}
		return c;
	}

	/**
	  * @brief：判断指定点是否在指定角度内部
	  * @param：X Y 指定点的坐标
	  * @param：StartAngle EndAngle 起始角度和终止角度，范围：-180~180
	  *           水平向右为0度，水平向左为180度或-180度，下方为正数，上方为负数，顺时针旋转
	  * @retval：指定点是否在指定角度内部，1：在内部，0：不在内部
	  */
	uint8_t OLED_IsInAngle(int16_t X, int16_t Y, int16_t StartAngle, int16_t EndAngle)
	{
		int16_t PointAngle;
		PointAngle = atan2(Y, X) / 3.14 * 180;	//计算指定点的弧度，并转换为角度表示
		if (StartAngle < EndAngle)	//起始角度小于终止角度的情况
		{
			/*如果指定角度在起始终止角度之间，则判定指定点在指定角度*/
			if (PointAngle >= StartAngle && PointAngle <= EndAngle)
			{
				return 1;
			}
		}
		else			//起始角度大于于终止角度的情况
		{
			/*如果指定角度大于起始角度或者小于终止角度，则判定指定点在指定角度*/
			if (PointAngle >= StartAngle || PointAngle <= EndAngle)
			{
				return 1;
			}
		}
		return 0;		//不满足以上条件，则判断判定指定点不在指定角度
	}


	/**
	  * @brief：将OLED显存数组更新到OLED屏幕
	  * @param：无
	  * @retval：无
	  * @note：所有的显示函数，都只是对OLED显存数组进行读写
	  *           随后调用OLED_Update函数或OLED_UpdateArea函数
	  *           才会将显存数组的数据发送到OLED硬件，进行显示
	  *           故调用显示函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_Update(void)
	{
		uint8_t j;
		/*遍历每一页*/
		for (j = 0; j < 8; j ++)
		{
			/*设置光标位置为每一页的第一列*/
			OLED_SetCursor(j, 0);
			/*连续写入128个数据，将显存数组的数据写入到OLED硬件*/
			OLED_WriteData(OLED_DisplayBuf[j], 128);
		}
	}

	/**
	  * @brief：将OLED显存数组部分更新到OLED屏幕
	  * @param：X 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：Width 指定区域的宽度，范围：0~128
	  * @param：Height 指定区域的高度，范围：0~64
	  * @retval：无
	  * @note：此函数会至少更新参数指定的区域
	  *           如果更新区域Y轴只包含部分页，则同一页的剩余部分会跟随一起更新
	  * @note：所有的显示函数，都只是对OLED显存数组进行读写
	  *           随后调用OLED_Update函数或OLED_UpdateArea函数
	  *           才会将显存数组的数据发送到OLED硬件，进行显示
	  *           故调用显示函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
	{
		int16_t j;
		int16_t Page, Page1;
		
		/*负数坐标在计算页地址时需要加一个偏移*/
		/*(Y + Height - 1) / 8 + 1的目的是(Y + Height) / 8并向上取整*/
		Page = Y / 8;
		Page1 = (Y + Height - 1) / 8 + 1;
		if (Y < 0)
		{
			Page -= 1;
			Page1 -= 1;
		}
		
		/*遍历指定区域涉及的相关页*/
		for (j = Page; j < Page1; j ++)
		{
			if (X >= 0 && X <= 127 && j >= 0 && j <= 7)		//超出屏幕的内容不显示
			{
				/*设置光标位置为相关页的指定列*/
				OLED_SetCursor(j, X);
				/*连续写入Width个数据，将显存数组的数据写入到OLED硬件*/
				OLED_WriteData(&OLED_DisplayBuf[j][X], Width);
			}
		}
	}

	/**
	  * @brief：将OLED显存数组全部清零
	  * @param：无
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_Clear(void)
	{
		uint8_t i, j;
		for (j = 0; j < 8; j ++)				//遍历8页
		{
			for (i = 0; i < 128; i ++)			//遍历128列
			{
				OLED_DisplayBuf[j][i] = 0x00;	//将显存数组数据全部清零
			}
		}
	}

	/**
	  * @brief：将OLED显存数组部分清零
	  * @param：X 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：Width 指定区域的宽度，范围：0~128
	  * @param：Height 指定区域的高度，范围：0~64
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
	{
		int16_t i, j;
		
		for (j = Y; j < Y + Height; j ++)		//遍历指定页
		{
			for (i = X; i < X + Width; i ++)	//遍历指定列
			{
				if (i >= 0 && i <= 127 && j >=0 && j <= 63)				//超出屏幕的内容不显示
				{
					OLED_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8));	//将显存数组指定数据清零
				}
			}
		}
	}

	/**
	  * @brief：将OLED显存数组全部取反
	  * @param：无
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_Reverse(void)
	{
		uint8_t i, j;
		for (j = 0; j < 8; j ++)				//遍历8页
		{
			for (i = 0; i < 128; i ++)			//遍历128列
			{
				OLED_DisplayBuf[j][i] ^= 0xFF;	//将显存数组数据全部取反
			}
		}
	}
		
	/**
	  * @brief：将OLED显存数组部分取反
	  * @param：X 指定区域左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定区域左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：Width 指定区域的宽度，范围：0~128
	  * @param：Height 指定区域的高度，范围：0~64
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
	{
		int16_t i, j;
		
		for (j = Y; j < Y + Height; j ++)		//遍历指定页
		{
			for (i = X; i < X + Width; i ++)	//遍历指定列
			{
				if (i >= 0 && i <= 127 && j >=0 && j <= 63)			//超出屏幕的内容不显示
				{
					OLED_DisplayBuf[j / 8][i] ^= 0x01 << (j % 8);	//将显存数组指定数据取反
				}
			}
		}
	}

	/**
	  * @brief：OLED显示一个字符
	  * @param：X 指定字符左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定字符左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：Char 指定要显示的字符，范围：ASCII码可见字符
	  * @param：FontSize 指定字体大小
	  *           范围：OLED_8X16		宽8像素，高16像素
	  *                 OLED_6X8		宽6像素，高8像素
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize)
	{
		if (FontSize == OLED_8X16)		//字体为宽8像素，高16像素
		{
			/*将ASCII字模库OLED_F8x16的指定数据以8*16的图像格式显示*/
			OLED_ShowImage(X, Y, 8, 16, OLED_F8x16[Char - ' ']);
		}
		else if(FontSize == OLED_6X8)	//字体为宽6像素，高8像素
		{
			/*将ASCII字模库OLED_F6x8的指定数据以6*8的图像格式显示*/
			OLED_ShowImage(X, Y, 6, 8, OLED_F6x8[Char - ' ']);
		}
	}

	/**
	  * @brief：OLED显示字符串
	  * @param：X 指定字符串左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定字符串左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：String 指定要显示的字符串，范围：ASCII码可见字符组成的字符串
	  * @param：FontSize 指定字体大小
	  *           范围：OLED_8X16		宽8像素，高16像素
	  *                 OLED_6X8		宽6像素，高8像素
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize)
	{
		uint8_t i;
		for (i = 0; String[i] != '\0'; i++)		//遍历字符串的每个字符
		{
			/*调用OLED_ShowChar函数，依次显示每个字符*/
			OLED_ShowChar(X + i * FontSize, Y, String[i], FontSize);
		}
	}

	/**
	  * @brief：OLED显示数字（十进制，正整数）
	  * @param：X 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：Number 指定要显示的数字，范围：0~4294967295
	  * @param：Length 指定数字的长度，范围：0~10
	  * @param：FontSize 指定字体大小
	  *           范围：OLED_8X16		宽8像素，高16像素
	  *                 OLED_6X8		宽6像素，高8像素
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
	{
		uint8_t i;
		for (i = 0; i < Length; i++)		//遍历数字的每一位							
		{
			/*调用OLED_ShowChar函数，依次显示每个数字*/
			/*Number / OLED_Pow(10, Length - i - 1) % 10 可以十进制提取数字的每一位*/
			/*+ '0' 可将数字转换为字符格式*/
			OLED_ShowChar(X + i * FontSize, Y, Number / OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
		}
	}

	/**
	  * @brief：OLED显示有符号数字（十进制，整数）
	  * @param：X 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：Number 指定要显示的数字，范围：-2147483648~2147483647
	  * @param：Length 指定数字的长度，范围：0~10
	  * @param：FontSize 指定字体大小
	  *           范围：OLED_8X16		宽8像素，高16像素
	  *                 OLED_6X8		宽6像素，高8像素
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize)
	{
		uint8_t i;
		uint32_t Number1;
		
		if (Number >= 0)						//数字大于等于0
		{
			OLED_ShowChar(X, Y, '+', FontSize);	//显示+号
			Number1 = Number;					//Number1直接等于Number
		}
		else									//数字小于0
		{
			OLED_ShowChar(X, Y, '-', FontSize);	//显示-号
			Number1 = -Number;					//Number1等于Number取负
		}
		
		for (i = 0; i < Length; i++)			//遍历数字的每一位								
		{
			/*调用OLED_ShowChar函数，依次显示每个数字*/
			/*Number1 / OLED_Pow(10, Length - i - 1) % 10 可以十进制提取数字的每一位*/
			/*+ '0' 可将数字转换为字符格式*/
			OLED_ShowChar(X + (i + 1) * FontSize, Y, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
		}
	}

	/**
	  * @brief：OLED显示十六进制数字（十六进制，正整数）
	  * @param：X 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：Number 指定要显示的数字，范围：0x00000000~0xFFFFFFFF
	  * @param：Length 指定数字的长度，范围：0~8
	  * @param：FontSize 指定字体大小
	  *           范围：OLED_8X16		宽8像素，高16像素
	  *                 OLED_6X8		宽6像素，高8像素
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
	{
		uint8_t i, SingleNumber;
		for (i = 0; i < Length; i++)		//遍历数字的每一位
		{
			/*以十六进制提取数字的每一位*/
			SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
			
			if (SingleNumber < 10)			//单个数字小于10
			{
				/*调用OLED_ShowChar函数，显示此数字*/
				/*+ '0' 可将数字转换为字符格式*/
				OLED_ShowChar(X + i * FontSize, Y, SingleNumber + '0', FontSize);
			}
			else							//单个数字大于10
			{
				/*调用OLED_ShowChar函数，显示此数字*/
				/*+ 'A' 可将数字转换为从A开始的十六进制字符*/
				OLED_ShowChar(X + i * FontSize, Y, SingleNumber - 10 + 'A', FontSize);
			}
		}
	}

	/**
	  * @brief：OLED显示二进制数字（二进制，正整数）
	  * @param：X 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：Number 指定要显示的数字，范围：0x00000000~0xFFFFFFFF
	  * @param：Length 指定数字的长度，范围：0~16
	  * @param：FontSize 指定字体大小
	  *           范围：OLED_8X16		宽8像素，高16像素
	  *                 OLED_6X8		宽6像素，高8像素
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
	{
		uint8_t i;
		for (i = 0; i < Length; i++)		//遍历数字的每一位	
		{
			/*调用OLED_ShowChar函数，依次显示每个数字*/
			/*Number / OLED_Pow(2, Length - i - 1) % 2 可以二进制提取数字的每一位*/
			/*+ '0' 可将数字转换为字符格式*/
			OLED_ShowChar(X + i * FontSize, Y, Number / OLED_Pow(2, Length - i - 1) % 2 + '0', FontSize);
		}
	}

	/**
	  * @brief：OLED显示浮点数字（十进制，小数）
	  * @param：X 指定数字左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定数字左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：Number 指定要显示的数字，范围：-4294967295.0~4294967295.0
	  * @param：IntLength 指定数字的整数位长度，范围：0~10
	  * @param：FraLength 指定数字的小数位长度，范围：0~9，小数进行四舍五入显示
	  * @param：FontSize 指定字体大小
	  *           范围：OLED_8X16		宽8像素，高16像素
	  *                 OLED_6X8		宽6像素，高8像素
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize)
	{
		uint32_t PowNum, IntNum, FraNum;
		
		if (Number >= 0)						//数字大于等于0
		{
			OLED_ShowChar(X, Y, '+', FontSize);	//显示+号
		}
		else									//数字小于0
		{
			OLED_ShowChar(X, Y, '-', FontSize);	//显示-号
			Number = -Number;					//Number取负
		}
		
		/*提取整数部分和小数部分*/
		IntNum = Number;						//直接赋值给整型变量，提取整数
		Number -= IntNum;						//将Number的整数减掉，防止之后将小数乘到整数时因数过大造成错误
		PowNum = OLED_Pow(10, FraLength);		//根据指定小数的位数，确定乘数
		FraNum = round(Number * PowNum);		//将小数乘到整数，同时四舍五入，避免显示误差
		IntNum += FraNum / PowNum;				//若四舍五入造成了进位，则需要再加给整数
		
		/*显示整数部分*/
		OLED_ShowNum(X + FontSize, Y, IntNum, IntLength, FontSize);
		
		/*显示小数点*/
		OLED_ShowChar(X + (IntLength + 1) * FontSize, Y, '.', FontSize);
		
		/*显示小数部分*/
		OLED_ShowNum(X + (IntLength + 2) * FontSize, Y, FraNum, FraLength, FontSize);
	}

	/**
	  * @brief：OLED显示汉字串
	  * @param：X 指定汉字串左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定汉字串左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：Chinese 指定要显示的汉字串，范围：必须全部为汉字或者全角字符，不要加入任何半角字符
	  *           显示的汉字需要在OLED_Data.c里的OLED_CF16x16数组定义
	  *           未找到指定汉字时，会显示默认图形（一个方框，内部一个问号）
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_ShowChinese(int16_t X, int16_t Y,const char *Chinese)
	{
		uint8_t pChinese = 0;
		uint8_t pIndex;
		uint8_t i;
		char SingleChinese[OLED_CHN_CHAR_WIDTH + 1] = {0};
		
		for (i = 0; Chinese[i] != '\0'; i ++)		//遍历汉字串
		{
			SingleChinese[pChinese] = Chinese[i];	//提取汉字串数据到单个汉字数组
			pChinese ++;							//计次自增
			
			/*当提取次数到达OLED_CHN_CHAR_WIDTH时，即代表提取到了一个完整的汉字*/
			if (pChinese >= OLED_CHN_CHAR_WIDTH)
			{
				pChinese = 0;		//计次归零
				
				/*遍历整个汉字字模库，寻找匹配的汉字*/
				/*如果找到最后一个汉字（定义为空字符串），则表示汉字未在字模库定义，停止寻找*/
				for (pIndex = 0; strcmp(OLED_CF16x16[pIndex].Index, "") != 0; pIndex ++)
				{
					/*找到匹配的汉字*/
					if (strcmp(OLED_CF16x16[pIndex].Index, SingleChinese) == 0)
					{
						break;		//跳出循环，此时pIndex的值为指定汉字的索引
					}
				}
				
				/*将汉字字模库OLED_CF16x16的指定数据以16*16的图像格式显示*/
				OLED_ShowImage(X + ((i + 1) / OLED_CHN_CHAR_WIDTH - 1) * 16, Y, 16, 16, OLED_CF16x16[pIndex].Data);
			}
		}
	}

	/**
	  * @brief：OLED显示图像
	  * @param：X 指定图像左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定图像左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：Width 指定图像的宽度，范围：0~128
	  * @param：Height 指定图像的高度，范围：0~64
	  * @param：Image 指定要显示的图像
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
	{
		uint8_t i = 0, j = 0;
		int16_t Page, Shift;
		
		/*将图像所在区域清空*/
		OLED_ClearArea(X, Y, Width, Height);
		
		/*遍历指定图像涉及的相关页*/
		/*(Height - 1) / 8 + 1的目的是Height / 8并向上取整*/
		for (j = 0; j < (Height - 1) / 8 + 1; j ++)
		{
			/*遍历指定图像涉及的相关列*/
			for (i = 0; i < Width; i ++)
			{
				if (X + i >= 0 && X + i <= 127)		//超出屏幕的内容不显示
				{
					/*负数坐标在计算页地址和移位时需要加一个偏移*/
					Page = Y / 8;
					Shift = Y % 8;
					if (Y < 0)
					{
						Page -= 1;
						Shift += 8;
					}
					
					if (Page + j >= 0 && Page + j <= 7)		//超出屏幕的内容不显示
					{
						/*显示图像在当前页的内容*/
						OLED_DisplayBuf[Page + j][X + i] |= Image[j * Width + i] << (Shift);
					}
					
					if (Page + j + 1 >= 0 && Page + j + 1 <= 7)		//超出屏幕的内容不显示
					{					
						/*显示图像在下一页的内容*/
						OLED_DisplayBuf[Page + j + 1][X + i] |= Image[j * Width + i] >> (8 - Shift);
					}
				}
			}
		}
	}

	/**
	  * @brief：OLED使用printf函数打印格式化字符串
	  * @param：X 指定格式化字符串左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定格式化字符串左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：FontSize 指定字体大小
	  *           范围：OLED_8X16		宽8像素，高16像素
	  *                 OLED_6X8		宽6像素，高8像素
	  * @param：format 指定要显示的格式化字符串，范围：ASCII码可见字符组成的字符串
	  * @param：... 格式化字符串参数列表
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize,const char *format, ...)
	{
		char String[256];						//定义字符数组
		va_list arg;							//定义可变参数列表数据类型的变量arg
		va_start(arg, format);					//从format开始，接收参数列表到arg变量
		vsprintf(String, format, arg);			//使用vsprintf打印格式化字符串和参数列表到字符数组中
		va_end(arg);							//结束变量arg
		OLED_ShowString(X, Y, String, FontSize);//OLED显示字符数组（字符串）
	}

	/**
	  * @brief：OLED在指定位置画一个点
	  * @param：X 指定点的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定点的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_DrawPoint(int16_t X, int16_t Y)
	{
		if (X >= 0 && X <= 127 && Y >=0 && Y <= 63)		//超出屏幕的内容不显示
		{
			/*将显存数组指定位置的一个Bit数据置1*/
			OLED_DisplayBuf[Y / 8][X] |= 0x01 << (Y % 8);
		}
	}

	

	/**
	  * @brief：OLED获取指定位置点的值
	  * @param：X 指定点的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定点的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @retval：指定位置点是否处于点亮状态，1：点亮，0：熄灭
	  */
	uint8_t OLED_GetPoint(int16_t X, int16_t Y)
	{
		if (X >= 0 && X <= 127 && Y >=0 && Y <= 63)		//超出屏幕的内容不读取
		{
			/*判断指定位置的数据*/
			if (OLED_DisplayBuf[Y / 8][X] & 0x01 << (Y % 8))
			{
				return 1;	//为1，返回1
			}
		}
		
		return 0;		//否则，返回0
	}

	/**
	  * @brief：OLED画线
	  * @param：X0 指定一个端点的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y0 指定一个端点的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：X1 指定另一个端点的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y1 指定另一个端点的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1)
	{
		int16_t x, y, dx, dy, d, incrE, incrNE, temp;
		int16_t x0 = X0, y0 = Y0, x1 = X1, y1 = Y1;
		uint8_t yflag = 0, xyflag = 0;
		
		if (y0 == y1)		//横线单独处理
		{
			/*0号点X坐标大于1号点X坐标，则交换两点X坐标*/
			if (x0 > x1) {temp = x0; x0 = x1; x1 = temp;}
			
			/*遍历X坐标*/
			for (x = x0; x <= x1; x ++)
			{
				OLED_DrawPoint(x, y0);	//依次画点
			}
		}
		else if (x0 == x1)	//竖线单独处理
		{
			/*0号点Y坐标大于1号点Y坐标，则交换两点Y坐标*/
			if (y0 > y1) {temp = y0; y0 = y1; y1 = temp;}
			
			/*遍历Y坐标*/
			for (y = y0; y <= y1; y ++)
			{
				OLED_DrawPoint(x0, y);	//依次画点
			}
		}
		else	//斜线
		{
			/*使用Bresenham算法画直线，可以避免耗时的浮点运算，效率更高*/
			/*参考文档：https://www.cs.montana.edu/courses/spring2009/425/dslectures/Bresenham.pdf*/
			/*参考教程：https://www.bilibili.com/video/BV1364y1d7Lo*/
			
			if (x0 > x1)	//0号点X坐标大于1号点X坐标
			{
				/*交换两点坐标*/
				/*交换后不影响画线，但是画线方向由第一、二、三、四象限变为第一、四象限*/
				temp = x0; x0 = x1; x1 = temp;
				temp = y0; y0 = y1; y1 = temp;
			}
			
			if (y0 > y1)	//0号点Y坐标大于1号点Y坐标
			{
				/*将Y坐标取负*/
				/*取负后影响画线，但是画线方向由第一、四象限变为第一象限*/
				y0 = -y0;
				y1 = -y1;
				
				/*置标志位yflag，记住当前变换，在后续实际画线时，再将坐标换回来*/
				yflag = 1;
			}
			
			if (y1 - y0 > x1 - x0)	//画线斜率大于1
			{
				/*将X坐标与Y坐标互换*/
				/*互换后影响画线，但是画线方向由第一象限0~90度范围变为第一象限0~45度范围*/
				temp = x0; x0 = y0; y0 = temp;
				temp = x1; x1 = y1; y1 = temp;
				
				/*置标志位xyflag，记住当前变换，在后续实际画线时，再将坐标换回来*/
				xyflag = 1;
			}
			
			/*以下为Bresenham算法画直线*/
			/*算法要求，画线方向必须为第一象限0~45度范围*/
			dx = x1 - x0;
			dy = y1 - y0;
			incrE = 2 * dy;
			incrNE = 2 * (dy - dx);
			d = 2 * dy - dx;
			x = x0;
			y = y0;
			
			/*画起始点，同时判断标志位，将坐标换回来*/
			if (yflag && xyflag){OLED_DrawPoint(y, -x);}
			else if (yflag)		{OLED_DrawPoint(x, -y);}
			else if (xyflag)	{OLED_DrawPoint(y, x);}
			else				{OLED_DrawPoint(x, y);}
			
			while (x < x1)		//遍历X轴的每个点
			{
				x ++;
				if (d < 0)		//下一个点在当前点东方
				{
					d += incrE;
				}
				else			//下一个点在当前点东北方
				{
					y ++;
					d += incrNE;
				}
				
				/*画每一个点，同时判断标志位，将坐标换回来*/
				if (yflag && xyflag){OLED_DrawPoint(y, -x);}
				else if (yflag)		{OLED_DrawPoint(x, -y);}
				else if (xyflag)	{OLED_DrawPoint(y, x);}
				else				{OLED_DrawPoint(x, y);}
			}	
		}
	}

	/**
	  * @brief：OLED矩形
	  * @param：X 指定矩形左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定矩形左上角的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：Width 指定矩形的宽度，范围：0~128
	  * @param：Height 指定矩形的高度，范围：0~64
	  * @param：IsFilled 指定矩形是否填充
	  *           范围：OLED_UNFILLED		不填充
	  *                 OLED_FILLED			填充
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_DrawRectangle(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled)
	{
		int16_t i, j;
		if (!IsFilled)		//指定矩形不填充
		{
			/*遍历上下X坐标，画矩形上下两条线*/
			for (i = X; i < X + Width; i ++)
			{
				OLED_DrawPoint(i, Y);
				OLED_DrawPoint(i, Y + Height - 1);
			}
			/*遍历左右Y坐标，画矩形左右两条线*/
			for (i = Y; i < Y + Height; i ++)
			{
				OLED_DrawPoint(X, i);
				OLED_DrawPoint(X + Width - 1, i);
			}
		}
		else				//指定矩形填充
		{
			/*遍历X坐标*/
			for (i = X; i < X + Width; i ++)
			{
				/*遍历Y坐标*/
				for (j = Y; j < Y + Height; j ++)
				{
					/*在指定区域画点，填充满矩形*/
					OLED_DrawPoint(i, j);
				}
			}
		}
	}

	/**
	  * @brief：OLED三角形
	  * @param：X0 指定第一个端点的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y0 指定第一个端点的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：X1 指定第二个端点的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y1 指定第二个端点的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：X2 指定第三个端点的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y2 指定第三个端点的纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：IsFilled 指定三角形是否填充
	  *           范围：OLED_UNFILLED		不填充
	  *                 OLED_FILLED			填充
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_DrawTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint8_t IsFilled)
	{
		int16_t minx = X0, miny = Y0, maxx = X0, maxy = Y0;
		int16_t i, j;
		int16_t vx[] = {X0, X1, X2};
		int16_t vy[] = {Y0, Y1, Y2};
		
		if (!IsFilled)			//指定三角形不填充
		{
			/*调用画线函数，将三个点用直线连接*/
			OLED_DrawLine(X0, Y0, X1, Y1);
			OLED_DrawLine(X0, Y0, X2, Y2);
			OLED_DrawLine(X1, Y1, X2, Y2);
		}
		else					//指定三角形填充
		{
			/*找到三个点最小的X、Y坐标*/
			if (X1 < minx) {minx = X1;}
			if (X2 < minx) {minx = X2;}
			if (Y1 < miny) {miny = Y1;}
			if (Y2 < miny) {miny = Y2;}
			
			/*找到三个点最大的X、Y坐标*/
			if (X1 > maxx) {maxx = X1;}
			if (X2 > maxx) {maxx = X2;}
			if (Y1 > maxy) {maxy = Y1;}
			if (Y2 > maxy) {maxy = Y2;}
			
			/*最小最大坐标之间的矩形为可能需要填充的区域*/
			/*遍历此区域中所有的点*/
			/*遍历X坐标*/		
			for (i = minx; i <= maxx; i ++)
			{
				/*遍历Y坐标*/	
				for (j = miny; j <= maxy; j ++)
				{
					/*调用OLED_pnpoly，判断指定点是否在指定三角形之中*/
					/*如果在，则画点，如果不在，则不做处理*/
					if (OLED_pnpoly(3, vx, vy, i, j)) {OLED_DrawPoint(i, j);}
				}
			}
		}
	}

	/**
	  * @brief：OLED画圆
	  * @param：X 指定圆的圆心横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定圆的圆心纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：Radius 指定圆的半径，范围：0~255
	  * @param：IsFilled 指定圆是否填充
	  *           范围：OLED_UNFILLED		不填充
	  *                 OLED_FILLED			填充
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_DrawCircle(int16_t X, int16_t Y, uint8_t Radius, uint8_t IsFilled)
	{
		int16_t x, y, d, j;
		
		/*使用Bresenham算法画圆，可以避免耗时的浮点运算，效率更高*/
		/*参考文档：https://www.cs.montana.edu/courses/spring2009/425/dslectures/Bresenham.pdf*/
		/*参考教程：https://www.bilibili.com/video/BV1VM4y1u7wJ*/
		
		d = 1 - Radius;
		x = 0;
		y = Radius;
		
		/*画每个八分之一圆弧的起始点*/
		OLED_DrawPoint(X + x, Y + y);
		OLED_DrawPoint(X - x, Y - y);
		OLED_DrawPoint(X + y, Y + x);
		OLED_DrawPoint(X - y, Y - x);
		
		if (IsFilled)		//指定圆填充
		{
			/*遍历起始点Y坐标*/
			for (j = -y; j < y; j ++)
			{
				/*在指定区域画点，填充部分圆*/
				OLED_DrawPoint(X, Y + j);
			}
		}
		
		while (x < y)		//遍历X轴的每个点
		{
			x ++;
			if (d < 0)		//下一个点在当前点东方
			{
				d += 2 * x + 1;
			}
			else			//下一个点在当前点东南方
			{
				y --;
				d += 2 * (x - y) + 1;
			}
			
			/*画每个八分之一圆弧的点*/
			OLED_DrawPoint(X + x, Y + y);
			OLED_DrawPoint(X + y, Y + x);
			OLED_DrawPoint(X - x, Y - y);
			OLED_DrawPoint(X - y, Y - x);
			OLED_DrawPoint(X + x, Y - y);
			OLED_DrawPoint(X + y, Y - x);
			OLED_DrawPoint(X - x, Y + y);
			OLED_DrawPoint(X - y, Y + x);
			
			if (IsFilled)	//指定圆填充
			{
				/*遍历中间部分*/
				for (j = -y; j < y; j ++)
				{
					/*在指定区域画点，填充部分圆*/
					OLED_DrawPoint(X + x, Y + j);
					OLED_DrawPoint(X - x, Y + j);
				}
				
				/*遍历两侧部分*/
				for (j = -x; j < x; j ++)
				{
					/*在指定区域画点，填充部分圆*/
					OLED_DrawPoint(X - y, Y + j);
					OLED_DrawPoint(X + y, Y + j);
				}
			}
		}
	}

	/**
	  * @brief：OLED画椭圆
	  * @param：X 指定椭圆的圆心横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定椭圆的圆心纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：A 指定椭圆的横向半轴长度，范围：0~255
	  * @param：B 指定椭圆的纵向半轴长度，范围：0~255
	  * @param：IsFilled 指定椭圆是否填充
	  *           范围：OLED_UNFILLED		不填充
	  *                 OLED_FILLED			填充
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_DrawEllipse(int16_t X, int16_t Y, uint8_t A, uint8_t B, uint8_t IsFilled)
	{
		int16_t x, y, j;
		int16_t a = A, b = B;
		float d1, d2;
		
		/*使用Bresenham算法画椭圆，可以避免部分耗时的浮点运算，效率更高*/
		/*参考链接：https://blog.csdn.net/myf_666/article/details/128167392*/
		
		x = 0;
		y = b;
		d1 = b * b + a * a * (-b + 0.5);
		
		if (IsFilled)	//指定椭圆填充
		{
			/*遍历起始点Y坐标*/
			for (j = -y; j < y; j ++)
			{
				/*在指定区域画点，填充部分椭圆*/
				OLED_DrawPoint(X, Y + j);
				OLED_DrawPoint(X, Y + j);
			}
		}
		
		/*画椭圆弧的起始点*/
		OLED_DrawPoint(X + x, Y + y);
		OLED_DrawPoint(X - x, Y - y);
		OLED_DrawPoint(X - x, Y + y);
		OLED_DrawPoint(X + x, Y - y);
		
		/*画椭圆中间部分*/
		while (b * b * (x + 1) < a * a * (y - 0.5))
		{
			if (d1 <= 0)		//下一个点在当前点东方
			{
				d1 += b * b * (2 * x + 3);
			}
			else				//下一个点在当前点东南方
			{
				d1 += b * b * (2 * x + 3) + a * a * (-2 * y + 2);
				y --;
			}
			x ++;
			
			if (IsFilled)	//指定椭圆填充
			{
				/*遍历中间部分*/
				for (j = -y; j < y; j ++)
				{
					/*在指定区域画点，填充部分椭圆*/
					OLED_DrawPoint(X + x, Y + j);
					OLED_DrawPoint(X - x, Y + j);
				}
			}
			
			/*画椭圆中间部分圆弧*/
			OLED_DrawPoint(X + x, Y + y);
			OLED_DrawPoint(X - x, Y - y);
			OLED_DrawPoint(X - x, Y + y);
			OLED_DrawPoint(X + x, Y - y);
		}
		
		/*画椭圆两侧部分*/
		d2 = b * b * (x + 0.5) * (x + 0.5) + a * a * (y - 1) * (y - 1) - a * a * b * b;
		
		while (y > 0)
		{
			if (d2 <= 0)		//下一个点在当前点东方
			{
				d2 += b * b * (2 * x + 2) + a * a * (-2 * y + 3);
				x ++;
				
			}
			else				//下一个点在当前点东南方
			{
				d2 += a * a * (-2 * y + 3);
			}
			y --;
			
			if (IsFilled)	//指定椭圆填充
			{
				/*遍历两侧部分*/
				for (j = -y; j < y; j ++)
				{
					/*在指定区域画点，填充部分椭圆*/
					OLED_DrawPoint(X + x, Y + j);
					OLED_DrawPoint(X - x, Y + j);
				}
			}
			
			/*画椭圆两侧部分圆弧*/
			OLED_DrawPoint(X + x, Y + y);
			OLED_DrawPoint(X - x, Y - y);
			OLED_DrawPoint(X - x, Y + y);
			OLED_DrawPoint(X + x, Y - y);
		}
	}

	/**
	  * @brief：OLED画圆弧
	  * @param：X 指定圆弧的圆心横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Y 指定圆弧的圆心纵坐标，范围：-32768~32767，屏幕区域：0~63
	  * @param：Radius 指定圆弧的半径，范围：0~255
	  * @param：StartAngle 指定圆弧的起始角度，范围：-180~180
	  *           水平向右为0度，水平向左为180度或-180度，下方为正数，上方为负数，顺时针旋转
	  * @param：EndAngle 指定圆弧的终止角度，范围：-180~180
	  *           水平向右为0度，水平向左为180度或-180度，下方为正数，上方为负数，顺时针旋转
	  * @param：IsFilled 指定圆弧是否填充，填充后为扇形
	  *           范围：OLED_UNFILLED		不填充
	  *                 OLED_FILLED			填充
	  * @retval：无
	  * @note：调用此函数后，要想真正地呈现在屏幕上，还需调用更新函数
	  */
	void OLED_DrawArc(int16_t X, int16_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled)
	{
		int16_t x, y, d, j;
		
		/*此函数借用Bresenham算法画圆的方法*/
		
		d = 1 - Radius;
		x = 0;
		y = Radius;
		
		/*在画圆的每个点时，判断指定点是否在指定角度内，在，则画点，不在，则不做处理*/
		if (OLED_IsInAngle(x, y, StartAngle, EndAngle))	{OLED_DrawPoint(X + x, Y + y);}
		if (OLED_IsInAngle(-x, -y, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y - y);}
		if (OLED_IsInAngle(y, x, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y + x);}
		if (OLED_IsInAngle(-y, -x, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y - x);}
		
		if (IsFilled)	//指定圆弧填充
		{
			/*遍历起始点Y坐标*/
			for (j = -y; j < y; j ++)
			{
				/*在填充圆的每个点时，判断指定点是否在指定角度内，在，则画点，不在，则不做处理*/
				if (OLED_IsInAngle(0, j, StartAngle, EndAngle)) {OLED_DrawPoint(X, Y + j);}
			}
		}
		
		while (x < y)		//遍历X轴的每个点
		{
			x ++;
			if (d < 0)		//下一个点在当前点东方
			{
				d += 2 * x + 1;
			}
			else			//下一个点在当前点东南方
			{
				y --;
				d += 2 * (x - y) + 1;
			}
			
			/*在画圆的每个点时，判断指定点是否在指定角度内，在，则画点，不在，则不做处理*/
			if (OLED_IsInAngle(x, y, StartAngle, EndAngle)) {OLED_DrawPoint(X + x, Y + y);}
			if (OLED_IsInAngle(y, x, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y + x);}
			if (OLED_IsInAngle(-x, -y, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y - y);}
			if (OLED_IsInAngle(-y, -x, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y - x);}
			if (OLED_IsInAngle(x, -y, StartAngle, EndAngle)) {OLED_DrawPoint(X + x, Y - y);}
			if (OLED_IsInAngle(y, -x, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y - x);}
			if (OLED_IsInAngle(-x, y, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y + y);}
			if (OLED_IsInAngle(-y, x, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y + x);}
			
			if (IsFilled)	//指定圆弧填充
			{
				/*遍历中间部分*/
				for (j = -y; j < y; j ++)
				{
					/*在填充圆的每个点时，判断指定点是否在指定角度内，在，则画点，不在，则不做处理*/
					if (OLED_IsInAngle(x, j, StartAngle, EndAngle)) {OLED_DrawPoint(X + x, Y + j);}
					if (OLED_IsInAngle(-x, j, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y + j);}
				}
				
				/*遍历两侧部分*/
				for (j = -x; j < x; j ++)
				{
					/*在填充圆的每个点时，判断指定点是否在指定角度内，在，则画点，不在，则不做处理*/
					if (OLED_IsInAngle(-y, j, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y + j);}
					if (OLED_IsInAngle(y, j, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y + j);}
				}
			}
		}
	}
#else

	/**
	  * @brief  OLED清屏
	  * @param  无
	  * @retval 无
	  */
	void OLED_Clear(void)
	{  
		uint8_t i, j;
		for (j = 0; j < 8; j++)
		{
			OLED_SetCursor(j, 0);
			for(i = 0; i < 128; i++)
			{
				OLED_WriteData(0x00);
			}
		}
	}

	/**
	  * @brief  OLED显示一个字符
	  * @param  Line 行位置，范围：1~4
	  * @param  Column 列位置，范围：1~16
	  * @param  Char 要显示的一个字符，范围：ASCII可见字符
	  * @retval 无
	  */
	void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
	{      	
		uint8_t i;
		OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);		//设置光标位置在上半部分
		for (i = 0; i < 8; i++)
		{
			OLED_WriteData(OLED_F8x16[Char - ' '][i]);			//显示上半部分内容
		}
		OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);	//设置光标位置在下半部分
		for (i = 0; i < 8; i++)
		{
			OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);		//显示下半部分内容
		}
	}

	/**
	  * @brief  OLED显示字符串
	  * @param  Line 起始行位置，范围：1~4
	  * @param  Column 起始列位置，范围：1~16
	  * @param  String 要显示的字符串，范围：ASCII可见字符
	  * @retval 无
	  */
	void OLED_ShowString(uint8_t Line, uint8_t Column,const char *String)
	{
		uint8_t i;
		for (i = 0; String[i] != '\0'; i++)
		{
			OLED_ShowChar(Line, Column + i, String[i]);
		}
	}



	/**
	  * @brief  OLED显示数字（十进制，正数）
	  * @param  Line 起始行位置，范围：1~4
	  * @param  Column 起始列位置，范围：1~16
	  * @param  Number 要显示的数字，范围：0~4294967295
	  * @param  Length 要显示数字的长度，范围：1~10
	  * @retval 无
	  */
	void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
	{
		uint8_t i;
		for (i = 0; i < Length; i++)							
		{
			OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
		}
	}

	/**
	  * @brief  OLED显示数字（十进制，带符号数）
	  * @param  Line 起始行位置，范围：1~4
	  * @param  Column 起始列位置，范围：1~16
	  * @param  Number 要显示的数字，范围：-2147483648~2147483647
	  * @param  Length 要显示数字的长度，范围：1~10
	  * @retval 无
	  */
	void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
	{
		uint8_t i;
		uint32_t Number1;
		if (Number >= 0)
		{
			OLED_ShowChar(Line, Column, '+');
			Number1 = Number;
		}
		else
		{
			OLED_ShowChar(Line, Column, '-');
			Number1 = -Number;
		}
		for (i = 0; i < Length; i++)							
		{
			OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
		}
	}

	/**
	  * @brief  OLED显示数字（十六进制，正数）
	  * @param  Line 起始行位置，范围：1~4
	  * @param  Column 起始列位置，范围：1~16
	  * @param  Number 要显示的数字，范围：0~0xFFFFFFFF
	  * @param  Length 要显示数字的长度，范围：1~8
	  * @retval 无
	  */
	void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
	{
		uint8_t i, SingleNumber;
		for (i = 0; i < Length; i++)							
		{
			SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
			if (SingleNumber < 10)
			{
				OLED_ShowChar(Line, Column + i, SingleNumber + '0');
			}
			else
			{
				OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
			}
		}
	}

	/**
	  * @brief  OLED显示数字（二进制，正数）
	  * @param  Line 起始行位置，范围：1~4
	  * @param  Column 起始列位置，范围：1~16
	  * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
	  * @param  Length 要显示数字的长度，范围：1~16
	  * @retval 无
	  */
	void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
	{
		uint8_t i;
		for (i = 0; i < Length; i++)							
		{
			OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
		}
	}

	/**
	  * @brief：OLED显示浮点数字（十进制，小数）
	  * @param：Line 行数 1-4
	  * @param：Column 列数 1-16
	  * @param：Number 指定要显示的数字，范围：-4294967295.0~4294967295.0
	  * @param：IntLength 指定数字的整数位长度，范围：0~10
	  * @param：FraLength 指定数字的小数位长度，范围：0~9，小数进行四舍五入显示
	  * @retval：无
	  */
	void OLED_ShowFloatNum(uint8_t Line, uint8_t Column, double Number, uint8_t IntLength, uint8_t FraLength)
	{
		uint32_t PowNum, IntNum, FraNum;
		
		if (Number >= 0)                        //数字大于等于0
		{
			OLED_ShowChar(Line, Column, '+');   //显示+号
		}
		else                                    //数字小于0
		{
			OLED_ShowChar(Line, Column, '-');   //显示-号
			Number = -Number;                   //Number取负
		}
		
		/*提取整数部分和小数部分*/
		IntNum = Number;                        //直接赋值给整型变量，提取整数
		Number -= IntNum;                       //将Number的整数减掉，防止之后将小数乘到整数时因数过大造成错误
		PowNum = OLED_Pow(10, FraLength);       //根据指定小数的位数，确定乘数
		FraNum = round(Number * PowNum);        //将小数乘到整数，同时四舍五入，避免显示误差
		IntNum += FraNum / PowNum;              //若四舍五入造成了进位，则需要再加给整数
		
		/*显示整数部分*/
		OLED_ShowNum(Line, Column + 1, IntNum, IntLength);
		/*显示小数点*/
		OLED_ShowChar(Line, Column + 1 + IntLength, '.');
		/*显示小数部分*/
		OLED_ShowNum(Line, Column + 1 + IntLength + 1, FraNum, FraLength);
	}

	/**
	  * @brief：OLED显示图像
	  * @param：X 指定图像左上角的横坐标，范围：-32768~32767，屏幕区域：0~127
	  * @param：Page 指定图像左上角的横坐标从哪一页开始，范围：-32768~32767，屏幕区域：0~7
	  * @param：Width 指定图像的宽度，范围：0~128
	  * @param：Height 指定图像的高度，范围：0~64
	  * @param：Image 指定要显示的图像
	  * @retval：无
	  * @note：只能从某页的起始位置开始画
	  */
	void OLED_ShowImage(int16_t X, int16_t Page, uint8_t Width, uint8_t Height, const uint8_t *Image)
	{
		uint8_t i = 0, j = 0;
		for(j=0; j < Height/8; j++)
		{
			OLED_SetCursor((Page+j), X);
			for(i=0; i < Width; i++)
			{
				OLED_WriteData(Image[j * Width + i]);
			}
		}
		
		
	}


	/**
	  * @brief：OLED使用printf函数打印格式化字符串
	  * @param  Line 起始行位置，范围：1~4
	  * @param  Column 起始列位置，范围：1~16
	  * @param：format 指定要显示的格式化字符串，范围：ASCII码可见字符组成的字符串
	  * @param：... 格式化字符串参数列表
	  * @retval：无
	  */
	void OLED_Printf(uint8_t Line, uint8_t Column,const char *format, ...)
	{
		char String[256];						//定义字符数组
		va_list arg;							//定义可变参数列表数据类型的变量arg
		va_start(arg, format);					//从format开始，接收参数列表到arg变量
		vsprintf(String, format, arg);			//使用vsprintf打印格式化字符串和参数列表到字符数组中
		va_end(arg);							//结束变量arg
		OLED_ShowString(Line, Column, String);//OLED显示字符数组（字符串）
	}
	
#endif






