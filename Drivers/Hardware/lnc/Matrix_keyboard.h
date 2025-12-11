// Matrix_keyboard.h 矩阵键盘
#ifndef MATRIX_KEYBOARD_H
#define MATRIX_KEYBOARD_H
#ifdef __cplusplus
extern "C" {
#endif

#include "borad.h"

#define ROW_1 ((struct gpio_pin){GPIOB, GPIO_PIN_15})    //第一行
#define ROW_2 ((struct gpio_pin){GPIOB, GPIO_PIN_14})    //第二行
#define ROW_3 ((struct gpio_pin){GPIOB, GPIO_PIN_13})   //第三行
#define ROW_4 ((struct gpio_pin){GPIOB, GPIO_PIN_12})   //第四行

#define COL_1 ((struct gpio_pin){GPIOC, GPIO_PIN_6})    //第一列
#define COL_2 ((struct gpio_pin){GPIOC, GPIO_PIN_7})    //第二列
#define COL_3 ((struct gpio_pin){GPIOC, GPIO_PIN_0})    //第三列
#define COL_4 ((struct gpio_pin){GPIOC, GPIO_PIN_1})    //第四列


#define USE_KEYBOARD_EXIT 0 //是否使用矩阵键盘硬件中断，1使用，0不使用


void Matrix_keyboard_init(void);
#if (USE_KEYBOARD_EXIT == 0)
char Matrix_keyboard_scan(void); 
#endif



#ifdef __cplusplus
}

#endif
#endif

