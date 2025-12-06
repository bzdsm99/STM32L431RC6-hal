#include "Matrix_keyboard.h"

char KEY_NUMCHARS[4][4] = {  //按键对应字符
    {'*', '7', '4', '1'},
    {'0', '8', '5', '2'},
    {'#', '9', '6', '3'},
    {'D', 'C', 'B', 'A'}
};

// 初始化函数
void Matrix_keyboard_init(void)
{
    // 配置行输出 推挽输出
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // 行 默认输出高电平,按键按下时拉低
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin = ROW_1.pin;
    HAL_GPIO_Init(ROW_1.GPIOx, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ROW_2.pin;
    HAL_GPIO_Init(ROW_2.GPIOx, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ROW_3.pin;
    HAL_GPIO_Init(ROW_3.GPIOx, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = ROW_4.pin;
    HAL_GPIO_Init(ROW_4.GPIOx, &GPIO_InitStruct);

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;  // 外部上拉电阻，这里用内部上拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Pin = COL_1.pin;
    HAL_GPIO_Init(COL_1.GPIOx, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = COL_2.pin;
    HAL_GPIO_Init(COL_2.GPIOx, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = COL_3.pin;
    HAL_GPIO_Init(COL_3.GPIOx, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = COL_4.pin;
    HAL_GPIO_Init(COL_4.GPIOx, &GPIO_InitStruct);

    // 默认所有行高电平
    HAL_GPIO_WritePin(ROW_1.GPIOx, ROW_1.pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ROW_2.GPIOx, ROW_2.pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ROW_3.GPIOx, ROW_3.pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ROW_4.GPIOx, ROW_4.pin, GPIO_PIN_SET);
}

/**
  * @brief      矩阵键盘扫描
  * @param      无
  * @retval     KEY_NUMCHARS中对键盘定义的字符
  */
char Matrix_keyboard_scan(void)
{
    uint8_t row, col;
    char key = 0; // 默认无按键按下

    // 定义行数组
    struct gpio_pin rows[] = {ROW_1, ROW_2, ROW_3, ROW_4};
    // 定义列数组
    struct gpio_pin cols[] = {COL_1, COL_2, COL_3, COL_4};

    for (row = 0; row < 4; row++) {
        // 设置当前行为低电平
        HAL_GPIO_WritePin(rows[row].GPIOx, rows[row].pin, GPIO_PIN_RESET);

        // 扫描所有列
        for (col = 0; col < 4; col++) {
            if (HAL_GPIO_ReadPin(cols[col].GPIOx, cols[col].pin) == GPIO_PIN_RESET) {
                // 检测到按键按下
                key = KEY_NUMCHARS[row][col]; // 获取按键对应的字符
                // 等待按键释放（可选）
                while (HAL_GPIO_ReadPin(cols[col].GPIOx, cols[col].pin) == GPIO_PIN_RESET);
                break;
            }
        }

        // 恢复当前行为高电平
        HAL_GPIO_WritePin(rows[row].GPIOx, rows[row].pin, GPIO_PIN_SET);

        // 如果检测到按键，退出循环
        if (key != 0) {
            break;
        }
    }

    return key; // 返回按键字符，0 表示无按键按下
}
