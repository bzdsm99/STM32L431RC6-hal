#include "cmsis_os.h"
#include "sys.h"
#include "usart.h"
#include "LCD.h"
#include "timer.h"
#include "delay.h"
#include "usart.h"
#include "stdlib.h"


// void MX_FREERTOS_Init(void);

#define MAX_SHAPES 20
#define SHAPE_SIZE 10
#define SPAWN_CHECK_RADIUS (SHAPE_SIZE * 1.5)

// 图形类型枚举
typedef enum {
    CIRCLE,    // 圆形（食物）
    SQUARE,    // 正方形（吃食物）
    TRIANGLE   // 三角形（吃正方形）
} ShapeType;

// 图形结构体
typedef struct {
    uint8_t active;        // 是否激活
    ShapeType type;        // 图形类型
    float x, y;            // 位置
    float dx, dy;          // 移动方向和速度
    uint16_t color;        // 颜色
} Shape;

Shape shapes[MAX_SHAPES];

// 随机生成一个图形
void spawn_shape(ShapeType type, float x, float y) {
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (!shapes[i].active) {
            shapes[i].active = 1;
            shapes[i].type = type;
            shapes[i].x = x;
            shapes[i].y = y;
            
            // 随机速度
            shapes[i].dx = ((float)(rand() % 100) / 100.0f) * 4.0f - 2.0f;  // -2 到 2
            shapes[i].dy = ((float)(rand() % 100) / 100.0f) * 4.0f - 2.0f;  // -2 到 2
            
            // 根据类型设置颜色
            switch (type) {
                case CIRCLE:   shapes[i].color = GREEN; break;   // 绿色圆形（食物）
                case SQUARE:   shapes[i].color = RED; break;     // 红色正方形（吃食物）
                case TRIANGLE: shapes[i].color = BLUE; break;    // 蓝色三角形（吃正方形）
            }
            break;
        }
    }
}

// 检查在指定位置生成圆形是否会与其他圆形重叠
uint8_t can_spawn_circle(float x, float y) {
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active && shapes[i].type == CIRCLE) {
            // 计算距离
            float dx = shapes[i].x - x;
            float dy = shapes[i].y - y;
            float distance_squared = dx*dx + dy*dy;
            
            // 检查是否在检查半径内
            if (distance_squared < (SPAWN_CHECK_RADIUS * SPAWN_CHECK_RADIUS)) {
                return 0; // 有圆形在附近，不能生成
            }
        }
    }
    return 1; // 可以生成
}

// 初始化图形系统
void init_shapes(void) {
    // 初始化所有图形为非激活状态
    for (int i = 0; i < MAX_SHAPES; i++) {
        shapes[i].active = 0;
    }
    
    // 生成一些初始图形
    spawn_shape(CIRCLE, 30, 30);
    spawn_shape(CIRCLE, 80, 50);
    spawn_shape(SQUARE, 50, 80);
    spawn_shape(TRIANGLE, 100, 100);
}

// 检查两个图形是否碰撞（简化版，使用平方距离避免开方运算）
uint8_t check_collision(Shape* s1, Shape* s2) {
    if (!s1->active || !s2->active || s1 == s2) {
        return 0;
    }
    
    // 使用平方距离检测（避免开方运算）
    float dx = s1->x - s2->x;
    float dy = s1->y - s2->y;
    float distance_squared = dx*dx + dy*dy;
    
    return distance_squared < (SHAPE_SIZE * SHAPE_SIZE);
}

// 更新图形位置和处理碰撞
void update_shapes(void) {
    // 更新位置
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active) {
            shapes[i].x += shapes[i].dx;
            shapes[i].y += shapes[i].dy;
            
            // 边界检测和反弹
            if (shapes[i].x <= SHAPE_SIZE/2 || shapes[i].x >= spilcd_width - SHAPE_SIZE/2) {
                shapes[i].dx = -shapes[i].dx;
                shapes[i].x += shapes[i].dx;  // 防止卡在边界
            }
            
            if (shapes[i].y <= SHAPE_SIZE/2 || shapes[i].y >= spilcd_height - SHAPE_SIZE/2) {
                shapes[i].dy = -shapes[i].dy;
                shapes[i].y += shapes[i].dy;  // 防止卡在边界
            }
        }
    }
    
    // 碰撞检测
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (!shapes[i].active) continue;
        
        for (int j = 0; j < MAX_SHAPES; j++) {
            if (!shapes[j].active || i == j) continue;
            
            if (check_collision(&shapes[i], &shapes[j])) {
                // 处理不同类型图形间的碰撞
                if (shapes[i].type == CIRCLE && shapes[j].type == CIRCLE) {
                    // 两个圆形相撞 - 两个都保留，并在中间生成新圆形（如果可以的话）
                    float mid_x = (shapes[i].x + shapes[j].x) / 2;
                    float mid_y = (shapes[i].y + shapes[j].y) / 2;
                    
                    // 检查是否可以在该位置生成新圆形
                    if (can_spawn_circle(mid_x, mid_y)) {
                        spawn_shape(CIRCLE, mid_x, mid_y);
                    }
                }
                else if ((shapes[i].type == SQUARE && shapes[j].type == CIRCLE) ||
                         (shapes[i].type == CIRCLE && shapes[j].type == SQUARE)) {
                    // 正方形碰到圆形 - 圆形消失，在圆心处生成新正方形（如果可以的话）
                    Shape* circle = (shapes[i].type == CIRCLE) ? &shapes[i] : &shapes[j];
                    float circle_x = circle->x;
                    float circle_y = circle->y;
                    
                    circle->active = 0;
                    
                    // 检查是否可以在该位置生成新正方形
                    if (can_spawn_circle(circle_x, circle_y)) {
                        spawn_shape(SQUARE, circle_x, circle_y);
                    }
                }
                else if ((shapes[i].type == TRIANGLE && shapes[j].type == SQUARE) ||
                         (shapes[i].type == SQUARE && shapes[j].type == TRIANGLE)) {
                    // 三角形碰到正方形 - 正方形消失
                    Shape* square = (shapes[i].type == SQUARE) ? &shapes[i] : &shapes[j];
                    square->active = 0;
                    // 这里可以添加触发彩色特效的代码
                }
            }
        }
    }
}

// 绘制圆形
void draw_circle(float x, float y, uint16_t color) {
    lcd_draw_circle((uint16_t)x, (uint16_t)y, SHAPE_SIZE/2, color);
    lcd_fill_circle((uint16_t)x, (uint16_t)y, SHAPE_SIZE/2, color);
}

// 绘制正方形
void draw_square(float x, float y, uint16_t color) {
    uint16_t half_size = SHAPE_SIZE/2;
    uint16_t x_pos = (uint16_t)x;
    uint16_t y_pos = (uint16_t)y;
    lcd_fill(x_pos - half_size, y_pos - half_size, x_pos + half_size, y_pos + half_size, color);
}

// 绘制三角形
void draw_triangle(float x, float y, uint16_t color) {
    uint16_t half_size = SHAPE_SIZE/2;
    uint16_t x_pos = (uint16_t)x;
    uint16_t y_pos = (uint16_t)y;
    
    // 绘制一个简单的三角形（等边三角形）
    lcd_draw_line(x_pos, y_pos - half_size, x_pos - half_size, y_pos + half_size, color);
    lcd_draw_line(x_pos - half_size, y_pos + half_size, x_pos + half_size, y_pos + half_size, color);
    lcd_draw_line(x_pos + half_size, y_pos + half_size, x_pos, y_pos - half_size, color);
}

// 绘制所有激活的图形
void draw_shapes(void) {
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active) {
            switch (shapes[i].type) {
                case CIRCLE:
                    draw_circle(shapes[i].x, shapes[i].y, shapes[i].color);
                    break;
                case SQUARE:
                    draw_square(shapes[i].x, shapes[i].y, shapes[i].color);
                    break;
                case TRIANGLE:
                    draw_triangle(shapes[i].x, shapes[i].y, shapes[i].color);
                    break;
            }
        }
    }
}

int main(void)
{
    HAL_Init();
    sys_stm32_clock_init(20);
    usart_init(115200);
    lcd_init();
    init_shapes();
    
    // MX_FREERTOS_Init();
    // osKernelStart();

    while (1)
    {
        // 清屏
        lcd_clear(WHITE);
        
        // 更新图形位置和处理碰撞
        update_shapes();
        
        // 绘制所有图形
        draw_shapes();
        
        // 延时
        HAL_Delay(50);  // 约20 FPS
    }
}


/**
    * @brief  This function is executed in case of error occurrence.
    * @retval None
    */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
    * @brief  Reports the name of the source file and the source line number
    * @param  file: pointer to the source file name
    * @param  line: assert_param error line source number
    * @retval None
    */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
        ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */