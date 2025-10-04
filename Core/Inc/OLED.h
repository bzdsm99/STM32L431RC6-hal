// OLED.h
#ifndef __OLED_H__
#define __OLED_H__
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "stm32l4xx_hal.h"
#include "OLED_Data.h"

#define Drive_SH1106
// #define Using_Video_Memory			//使用显存
// #define OLED_USE_Hardware			//使用硬件IIC

#ifdef OLED_USE_Hardware
	#define I2C_PERIPH 1	//I2C1(SCL:PA9 SDA:PA10)
	// #define I2C_PERIPH 2   //I2C2(SCL:PB10 SDA:PB11)
	// #define I2C_PERIPH 3   //I2C3(SCL:PC0 SDA:PC1)
	// #if (I2C_PERIPH == 1)
	// 	//#define AF_I2C1	//重映射I2C1	AF_I2C1(SCL:PB8 SDA:PB9)
	// #endif
#else
	#define OLED_SCL_PORT GPIOA
    #define OLED_SCL_PIN GPIO_PIN_9
	#define OLED_SDA_PORT GPIOA
    #define OLED_SDA_PIN GPIO_PIN_10
#endif

/*初始化函数*/
void OLED_Init(void);
#ifdef Using_Video_Memory
	/*FontSize参数取值*/
	/*此参数值不仅用于判断，而且用于计算横向字符偏移，默认值为字体像素宽度*/
	#define OLED_8X16 8
	#define OLED_6X8 6

	/*IsFilled参数数值*/
	#define OLED_UNFILLED 0
	#define OLED_FILLED 1

	/*更新函数*/
	void OLED_Update(void);
	void OLED_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

	/*显存控制函数*/
	void OLED_Clear(void);
	void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);
	void OLED_Reverse(void);
	void OLED_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height);

	/*显示函数*/
	void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize);
	void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize);
	void OLED_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
	void OLED_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize);
	void OLED_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
	void OLED_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize);
	void OLED_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize);
	void OLED_ShowChinese(int16_t X, int16_t Y, const char *Chinese);
	void OLED_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);
	void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, const char *format, ...);

	/*绘图函数*/
	void OLED_DrawPoint(int16_t X, int16_t Y);
	uint8_t OLED_GetPoint(int16_t X, int16_t Y);
	void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1);
	void OLED_DrawRectangle(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled);
	void OLED_DrawTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint8_t IsFilled);
	void OLED_DrawCircle(int16_t X, int16_t Y, uint8_t Radius, uint8_t IsFilled);
	void OLED_DrawEllipse(int16_t X, int16_t Y, uint8_t A, uint8_t B, uint8_t IsFilled);
	void OLED_DrawArc(int16_t X, int16_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled);

#else
	void OLED_Clear(void);
	void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
	void OLED_ShowString(uint8_t Line, uint8_t Column, const char *String);
	void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
	void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
	void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
	void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
	void OLED_ShowFloatNum(uint8_t Line, uint8_t Column, double Number, uint8_t IntLength, uint8_t FraLength);
	void OLED_ShowImage(int16_t X, int16_t Page, uint8_t Width, uint8_t Height, const uint8_t *Image);
	void OLED_Printf(uint8_t Line, uint8_t Column, const char *format, ...);

#endif

#ifdef __cplusplus
}
#endif
#endif	/*__OLED_H__ */


