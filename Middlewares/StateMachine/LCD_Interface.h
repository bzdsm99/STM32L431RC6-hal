// LCD_Interface.h
#ifndef LCD_INTERFACE_H
#define LCD_INTERFACE_H
#ifdef __cplusplus
extern "C" {
#endif

#include "LCD.h"

// 学号信息
extern const char * Student_ID;
extern const char * Student_Name;
extern const char * Professional;
extern const char * Product_Name;





void LCD_StartScreen(void);
void LCD_LockScreen(void);
void LCD_RuningScreen(uint32_t keyValue);
void LCD_Scrolling_display(uint8_t Line, const char *str);


void TaskLCDRunfunc(void const *argument);


#ifdef __cplusplus
}
#endif
#endif // LCD_INTERFACE_H



