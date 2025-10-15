// Matrix_keyboard.h 矩阵键盘
#ifndef MATRIX_KEYBOARD_H
#define MATRIX_KEYBOARD_H
#ifdef __cplusplus
extern "C" {
#endif

#include "borad.h"

#define ROW_1 ((struct gpio_pin){GPIOC, GPIO_PIN_7})    //第一行
#define ROW_2 ((struct gpio_pin){GPIOC, GPIO_PIN_6})    //第二行
#define ROW_3 ((struct gpio_pin){GPIOB, GPIO_PIN_15})   //第三行
#define ROW_4 ((struct gpio_pin){GPIOB, GPIO_PIN_14})   //第四行

#define COL_1 ((struct gpio_pin){GPIOC, GPIO_PIN_0})    //第一列
#define COL_2 ((struct gpio_pin){GPIOC, GPIO_PIN_1})    //第二列
#define COL_3 ((struct gpio_pin){GPIOC, GPIO_PIN_3})    //第三列
#define COL_4 ((struct gpio_pin){GPIOA, GPIO_PIN_7})    //第四列

//extern char KEY_NUMCHARS[4][4];     //按键对应字符

void Matrix_keyboard_init(void);
char Matrix_keyboard_scan(void); 

#ifdef __cplusplus
}
#endif
#endif

