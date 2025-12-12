//LCD_Interface.cpp
#include "gpio.h"
#include "lcd.h"
#include "sys.h"
#include "usart.h"
#include "borad.h"
#include "adc.h"
#include "delay.h"
#include "timer.h"
#include "Matrix_keyboard.h"
#include "LCD_Interface.h"
#include <string.h>
#include <math.h>
#include "semphr.h" 
#include "ImageData.h"
#include "mpu6050.h"

#define Max_ID_Len 16
#define USART_BUFFER_SIZE 30

// 学号信息
const char * Student_ID = "42301717";
const char * Student_Name = "张庭华";
const char * Professional = "智能科学与技术";
const char * Product_Name = "Chinese National Missile Defense (CNMD)        ";
static uint8_t LCD_Scrolling_Line;
static uint8_t page = 0;

static char USART_Buffer[USART_BUFFER_SIZE]; 

static void LCD_LockStaticScreen(void);
static void LCD_TIM6_Callbask(void);
static bool last_digit_student_ID_is_even(void);
static void LED_Startshow(void);
static void lcd_draw_3d_cube(uint16_t x_pos, uint16_t y_pos, uint16_t size, 
    float angle_x, float angle_y, float angle_z, uint16_t color);


void Runing_Page_1(void);
void Runing_Page_2(void);
void Runing_Page_3(void);
void Runing_Page_4(void);
void Runing_Page_5(void);
void Runing_Page_6(void);
void Runing_Page_7(void);
void Runing_Page_8(void);

static const char *LCD_PageName[8] = {"光照强度值","画图","图片","数字","英文字符"
    ,"六轴传感器","自定义","自定义"};
static void (*func_pages[8])(void) = {
    Runing_Page_1,
    Runing_Page_2,
    Runing_Page_3,
    Runing_Page_4,
    Runing_Page_5,
    Runing_Page_6,
    Runing_Page_7,
    Runing_Page_8
};



static void (*LED_blink)(void) = NULL;
static void (*LED_func)(void) = NULL;


SemaphoreHandle_t xLCDFuncMutex = NULL;



//锁屏界面
void LCD_LockScreen(void)
{
    char input[Max_ID_Len] = {0}; // 初始化数组为全0
    char key = 0;
    bool flag = 0;
    uint8_t index = 0;
    uint32_t start_tick;

    LCD_LockStaticScreen();
    start_tick = HAL_GetTick();
    while(1)
    {
        key = Matrix_keyboard_scan();
        if(key != 0)
        {
            if(key == 'D')  // 确认
            {
                input[index] = '\0'; // 确保字符串结束
                printf("输入学号：%s\r\n", input); // 直接打印input数组
                // 比较输入与预设学号
                if(strcmp(input, Student_ID) == 0) 
                {
                    // 学号正确
                    lcd_clear(GREEN);
                    lcd_show_chinese(5,3,"欢迎你！");
                    delay_ms(2000);
                    return; // 退出锁屏函数
                }
                else 
                {
                    // 学号错误
                    lcd_clear(RED);
                    lcd_show_chinese(5,3,"学号错误！");
                    delay_ms(2000);

                    LCD_LockStaticScreen();
                    index = 0;
                    //memset(input, 0, sizeof(input));
                }
            }
            else if(key == 'C') //删除
            {
                if(index > 0)
                {
                    lcd_fill(((index-1)*8), 16*7, (index*8), 16*8, g_back_color); //清除最后一个字符
                    index--;
                    input[index] = 0;
                }
                continue;
            }
            else if(key >= '0' && key <= '9') //数字输入
            {
                lcd_printf(8,index+1,LCD_FONT_16,"%c",key); // 显示位置从1开始
                input[index] = key; // 存储位置从0开始
                index++;
                if(index > Max_ID_Len - 1) 
                {
                    index = 0;
                    lcd_fill(0, 16*7, spilcd_width, spilcd_height-32, g_back_color); //清除输入区域
                }
            }
        }


        // 使用系统滴答计数器实现光标闪烁
        if((HAL_GetTick() - start_tick) >= 500)
        {
            //画光标
            if(flag == 0)
            {
                lcd_draw_line((index*8), 16*8, 8+(index*8), 16*8, g_point_color);
                flag = 1;
            }
            else//去除光标
            {
                lcd_draw_line(0, 16*8, spilcd_width, 16*8, g_back_color);
                flag = 0;
            }
            start_tick = HAL_GetTick();
        }
        delay_ms(20);
    }
}

//锁屏的静态界面部分
static void LCD_LockStaticScreen(void)
{
    g_back_color = BLACK;
    g_point_color = WHITE;
    lcd_clear(BLACK);
    lcd_show_chinese(5,3,"锁屏界面");
    lcd_show_chinese(7,1,"请输入学号：");
    lcd_show_chinese(10,1,"删除");
    lcd_printf(10,5,LCD_FONT_16,"(C)");
    lcd_show_chinese(10,5,"确认");
    lcd_printf(10,13,LCD_FONT_16,"(D)");
}

// 判断学号最后一位是不是偶数
static bool last_digit_student_ID_is_even(void)
{
    // 获取字符串长度
    uint8_t len = strlen(Student_ID);

    // 获取最后一位字符并转换为数字
    char last_digit_char = Student_ID[len - 1];
    uint8_t last_digit = last_digit_char - '0';
    
    // 判断奇偶性
    return ((last_digit % 2 == 0) ? true : false);
}

// LED灯闪烁-上电开机界面的指示灯
static void LED_Startshow(void)
{
    // 静态变量，用于记录反转次数
    static uint8_t toggle_count = 0;
    
    // 判断学号最后一位是否为偶数
    if (last_digit_student_ID_is_even()) {   
        // 偶数：蓝灯反转两次后绿灯反转两次
        if (toggle_count < 2) {
            led_control_Toggle(LED_B); // 蓝灯反转
        } else {
            led_control_Toggle(LED_G); // 绿灯反转
        }
    } else {   
        // 奇数：红灯反转两次后绿灯反转两次
        if (toggle_count < 2) {
            led_control_Toggle(LED_R); // 红灯反转
        } else {
            led_control_Toggle(LED_G); // 绿灯反转
        }
    }
    toggle_count++;
    if (toggle_count >= 4) toggle_count = 0;
}



//开机界面
void LCD_StartScreen(void)
{
    g_back_color = WHITE;
    g_point_color = BLACK;
    LCD_Scrolling_Line = 4;     //让产品名称在第3行滚动
    lcd_clear(g_back_color);
    lcd_show_chinese(1,1,Student_Name);
    lcd_printf(1,8,LCD_FONT_16,Student_ID);
    lcd_show_chinese(2,1,Professional);
    Timx_Set_TIM6_Callback(LCD_TIM6_Callbask);      //启动TIM6的外部回调
    for(uint8_t i=0;i<5;i++)    //按照要求显示5次
    {
        printf("演示者为 \"%s %s %s\"\r\n",Student_Name,Student_ID,Professional);
        printf("产品名称为: %s\r\n",Product_Name);
    }

    LED_blink = LED_Startshow;
    lcd_show_pic(15,60,100,100,gImage);
}


/**
  * @brief      滚动显示字符,每调用一次向右移动一个字符
  * @param      Line  滚动显示位置的行数
  * @param      str   显示的字符串
  * @retval     无
  */
void LCD_Scrolling_display(uint8_t Line, const char *str)
{
    static uint8_t pos = 0;  // 滚动位置
    uint8_t len = strlen(str);
    char display_str[17] = {0}; // 显示字符串缓冲区(16字符+结束符)
    
    // 清除指定行显示区域
    lcd_fill(0, (Line-1)*16, spilcd_width, Line*16, g_back_color);
    
    if(len == 0) {
        // 空字符串，直接返回
        return;
    }

    else if(len <= 16) {
        // 字符串长度小于等于16，添加空格后滚动显示
        char padded_str[32] = {0}; // 足够容纳原字符串+空格+原字符串
        strcpy(padded_str, str);
        
        // 添加空格分隔
        for(int i = len; i < len + 16; i++) {
            padded_str[i] = ' ';
        }
        
        // 再次添加原字符串以实现无缝滚动
        for(int i = 0; i < len; i++) {
            padded_str[i + len + 16] = str[i];
        }
        
        // 显示滚动文本 (修正lcd_printf的第一个参数，应该直接使用Line)
        for(int i = 0; i < 16; i++) {
            display_str[i] = padded_str[(pos + i) % (len + 16)];
        }
        
        lcd_printf(Line, 1, LCD_FONT_16, "%s", display_str);
        
        // 更新滚动位置
        pos = (pos + 1) % (len + 16);
    } else {
        // 字符串长度大于16，需要滚动显示
        // 复制要显示的部分到display_str
        for(int i = 0; i < 16; i++) {
            display_str[i] = str[(pos + i) % len];
        }
        
        // 显示滚动文本 (修正lcd_printf的第一个参数，应该直接使用Line)
        lcd_printf(Line, 1, LCD_FONT_16, "%s", display_str);
        
        // 更新滚动位置
        pos = (pos + 1) % len;
    }
}


// 运行时的静态界面页数显示部分-渲染在右下角
static void lCD_RuningPageStaticScreen(void)
{
    lcd_show_chinese(10,6,"第");
    lcd_show_chinese(10,8,"页");
    lcd_printf(10,13,LCD_FONT_16,"%d",page);
    lcd_show_chinese(2,1,LCD_PageName[page-1]);

}

static void LCD_RuningPagePrintf(void)
{
    portENTER_CRITICAL();       // 进入临界区
    for(uint8_t i = 0; i < (8 - page); i++)
    {
        printf("演示者为“%s %s %s”，正在展示第 %u 页\r\n",Student_Name,Student_ID,Professional,page);
        if(page == 1)   printf("，当前光照强度值为 %d\r\n",adc_get_value(ADC_CHANNEL_5));
    }

    portEXIT_CRITICAL();        // 退出临界区
}




// 运行时的静态界面部分-渲染在右下角
static void lCD_RuningErrorStaticScreen(void)
{
    vTaskSuspendAll();
    
    g_back_color = RED;
    lcd_init();
    lcd_clear(RED);
    delay_ms(20);
    lcd_clear(RED);
    delay_ms(20);

    lcd_show_chinese(4,1,"提示：");
    lcd_show_chinese(5,3,"无效按键");
    for(uint8_t i=0;i<3;i++)    printf("提示：无效按键\r\n");
    // 关闭三色LED灯
    led_control(LED_R,1);
    led_control(LED_G,1);
    led_control(LED_B,1);
    for(uint8_t i=0;i<16;i++){       //红灯闪烁8次
        led_control_Toggle(LED_R);
        delay_ms(100);
    }

    g_back_color = WHITE;
    lcd_clear(g_back_color);
    delay_ms(20);
    lcd_clear(g_back_color);
    delay_ms(20);

    xTaskResumeAll();
}








/**
  * @brief      接收键盘监控任务通知的按键值
  * @note       该函数接收TaskMatrix_keyboard任务的队列消息
  *             并根据按键值进行相应的处理
  * @retval     无
  */
void LCD_RuningScreen(uint32_t keyValue)
{
    LED_blink = NULL;   //取消闪烁
    LCD_Scrolling_Line = 1;     //让产品名称在第一行滚动

    
    g_back_color = WHITE;
    g_point_color = BLACK;

    
    if(keyValue == Student_ID[page])
    {

        LED_func = NULL;    //先暂停子任务
        lcd_init();
        lcd_clear(g_back_color);
        delay_ms(20);
        LCD_RuningPagePrintf();
        lcd_clear(g_back_color);
        delay_ms(20);
        LED_func = func_pages[page];
        page++;
    }
    else
    {
        lCD_RuningErrorStaticScreen();
    }
}




void Runing_Page_1(void)
{
    static uint8_t num = 0;

    lCD_RuningPageStaticScreen();
    lcd_printf(4,1,LCD_FONT_24,"ADC: %d   ",adc_get_value(ADC_CHANNEL_5));
    //printf("Runing_Page_1 runing %d\r\n", num);
    // 关闭三色LED灯
    led_control(LED_R,1);
    led_control(LED_G,1);
    led_control(LED_B,1);
    last_digit_student_ID_is_even() ? led_control(LED_R,0) : led_control(LED_B,0);
    num++;
    if(num >= 20)  //为什么这么写，因为见鬼了
    {
        lcd_init();
        num = 0;
    }
    vTaskDelay(pdMS_TO_TICKS(500));

}


void Runing_Page_2(void)
{
    //printf("Runing_Page_2 runing\r\n");
    lCD_RuningPageStaticScreen();

    static float x=30,y=45,z=60;
    
    // 使用连续递增的角度值，使旋转更平滑
    x += 1.5f;
    y += 1.5f;
    z += 1.5f;
    
    // 保持角度在0-360范围内
    if (x >= 360.0f) x -= 360.0f;
    if (y >= 360.0f) y -= 360.0f;
    if (z >= 360.0f) z -= 360.0f;

    lcd_draw_3d_cube(65, 95, 50, x, y, z, RED);
}


void Runing_Page_3(void)
{
    //lcd_init();
    lCD_RuningPageStaticScreen();

    for(uint8_t i = 0; i < Frame_number; i++)
    {
        lcd_show_pic(15,40,100,100,gImageArray[i]);
        delay_ms(200);
    }
    for(int8_t i = Frame_number-1; i >= 0; i--)
    {
        lcd_show_pic(15,40,100,100,gImageArray[i]);
        delay_ms(200);
    }
}


void Runing_Page_4(void)
{
    static bool flag = true;
    if(flag)
    {
        lcd_init();
        lcd_clear(g_back_color);
        delay_ms(20);
        flag = false;
    }

    lCD_RuningPageStaticScreen();
    
    // 处理串口数据，判断是否全部为数字
    if(uart_read_line(USART_Buffer, USART_BUFFER_SIZE) && strlen(USART_Buffer) ) {
        printf("Received string: %s, len: %d\r\n", USART_Buffer,strlen(USART_Buffer));
        int i = 0;
        int is_all_digits = 1; // 假设全是数字
        
        // 使用while循环判断从第一个字符到'\0'是否都是数字
        while(USART_Buffer[i] != '\0') {
            if(USART_Buffer[i] < '0' || USART_Buffer[i] > '9') {
                is_all_digits = 0; // 发现非数字字符
                break;
            }
            i++;
        }
        
        if(is_all_digits && i > 0) { // 确保字符串不为空且全是数字
            // 在LCD上显示数字
            lcd_printf(4, 1, LCD_FONT_24, "Number:        ");
            lcd_printf(5, 1, LCD_FONT_24, "%s         ", USART_Buffer);
        } else {
            lcd_printf(4, 1, LCD_FONT_24, "is not a Number");
        }
    }
}

void Runing_Page_5(void)
{
    lCD_RuningPageStaticScreen();
    
    if(uart_read_line(USART_Buffer, USART_BUFFER_SIZE) && strlen(USART_Buffer) ) {
        printf("Received string: %s, len: %d\r\n", USART_Buffer,strlen(USART_Buffer));
        lcd_printf(4, 1, LCD_FONT_24, "string: ", USART_Buffer);
        lcd_printf(5, 1, LCD_FONT_24, "%s         ", USART_Buffer);
    }
}

void Runing_Page_6(void)
{
    float temperature,gx,gy,gz,ax,ay,az;
    lCD_RuningPageStaticScreen();
    temperature = MPU_Get_Temperature();
    MPU_Get_Gyroscope(&gx, &gy, &gz);
    MPU_Get_Accelerometer(&ax, &ay, &az);
    lcd_printf(6, 1, LCD_FONT_12, "Temperatute: %.2f  ", temperature);

    lcd_printf(7, 1, LCD_FONT_12, "gx: %.2f   ", gx);
    lcd_printf(7, 12, LCD_FONT_12, "ax: %.2f  ", ax);

    lcd_printf(8, 1, LCD_FONT_12, "gy: %.2f   ", gy);
    lcd_printf(8, 12, LCD_FONT_12, "gy: %.2f  ", gy);

    lcd_printf(9, 1, LCD_FONT_12, "gz: %.2f   ", gz);
    lcd_printf(9, 12, LCD_FONT_12, "az: %.2f  ", az);

    delay_ms(500);
}



void Runing_Page_7(void)
{
    lCD_RuningPageStaticScreen();
    
}


void Runing_Page_8(void)
{
    lCD_RuningPageStaticScreen();
    
}



void TaskLCDRunfunc(void const * argument)
{
    portENTER_CRITICAL();       // 进入临界区
    printf("TaskLCDRunfunc Start\r\n");
    if(xLCDFuncMutex == NULL)
    {
        xLCDFuncMutex = xSemaphoreCreateMutex();
        if(xLCDFuncMutex == NULL)
        {
            printf("Failed to create xLCDFuncMutex\r\n");
        }
    }
    portEXIT_CRITICAL();        // 退出临界区

    for(;;) {
        if(LED_func != NULL)
        {
            // 尝试获取互斥量，最多等待10ms
            if(xSemaphoreTake(xLCDFuncMutex, pdMS_TO_TICKS(10)) == pdTRUE)
            {
                LED_func();
                xSemaphoreGive(xLCDFuncMutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}




//TIM6外部回调函数  TIM6 500ms执行一次
static void LCD_TIM6_Callbask(void)
{
    if(LED_blink != NULL)
    {
        LED_blink();
    }
    LCD_Scrolling_display(LCD_Scrolling_Line, Product_Name);
}





static void lcd_draw_3d_cube(uint16_t x_pos, uint16_t y_pos, uint16_t size, 
    float angle_x, float angle_y, float angle_z, uint16_t color)
{
    lcd_fill(x_pos - size + 5 , y_pos - size + 5, x_pos + size - 5, y_pos + size - 5, g_back_color);

    // 立方体的8个顶点 (x, y, z)，以中心为原点
    float vertices[8][3] = {
        {-size / 2, -size / 2, -size / 2},  // 0: 左下后
        {size / 2, -size / 2, -size / 2},   // 1: 右下后
        {size / 2, size / 2, -size / 2},    // 2: 右上后
        {-size / 2, size / 2, -size / 2},   // 3: 左上后
        {-size / 2, -size / 2, size / 2},   // 4: 左下前
        {size / 2, -size / 2, size / 2},    // 5: 右下前
        {size / 2, size / 2, size / 2},     // 6: 右上前
        {-size / 2, size / 2, size / 2}     // 7: 左上前
    };

    // 转换角度为弧度
    float rad_x = angle_x * 3.1415926f / 180.0f;
    float rad_y = angle_y * 3.1415926f / 180.0f;
    float rad_z = angle_z * 3.1415926f / 180.0f;

    // 计算旋转矩阵
    float cos_x = cosf(rad_x), sin_x = sinf(rad_x);
    float cos_y = cosf(rad_y), sin_y = sinf(rad_y);
    float cos_z = cosf(rad_z), sin_z = sinf(rad_z);

    // 存储旋转后的二维投影点
    int projected[8][2];

    for (int i = 0; i < 8; i++)
    {
        // 初始顶点
        float x = vertices[i][0];
        float y = vertices[i][1];
        float z = vertices[i][2];

        // 绕X轴旋转
        float new_y = y * cos_x - z * sin_x;
        float new_z = y * sin_x + z * cos_x;

        // 绕Y轴旋转
        float new_x = x * cos_y + new_z * sin_y;
        new_z = -x * sin_y + new_z * cos_y;

        // 绕Z轴旋转
        float final_x = new_x * cos_z - new_y * sin_z;
        float final_y = new_x * sin_z + new_y * cos_z;

        // 投影到屏幕 (简单平行投影并平移到指定位置)
        projected[i][0] = (int)(final_x) + x_pos;
        projected[i][1] = (int)(final_y) + y_pos;
    }

    // 绘制立方体的12条边
    // 后面 (z = -size/2)
    lcd_draw_line(projected[0][0], projected[0][1], projected[1][0], projected[1][1], color);
    lcd_draw_line(projected[1][0], projected[1][1], projected[2][0], projected[2][1], color);
    lcd_draw_line(projected[2][0], projected[2][1], projected[3][0], projected[3][1], color);
    lcd_draw_line(projected[3][0], projected[3][1], projected[0][0], projected[0][1], color);
    
    // 前面 (z = size/2)
    lcd_draw_line(projected[4][0], projected[4][1], projected[5][0], projected[5][1], color);
    lcd_draw_line(projected[5][0], projected[5][1], projected[6][0], projected[6][1], color);
    lcd_draw_line(projected[6][0], projected[6][1], projected[7][0], projected[7][1], color);
    lcd_draw_line(projected[7][0], projected[7][1], projected[4][0], projected[4][1], color);
    
    // 连接前后面
    lcd_draw_line(projected[0][0], projected[0][1], projected[4][0], projected[4][1], color);
    lcd_draw_line(projected[1][0], projected[1][1], projected[5][0], projected[5][1], color);
    lcd_draw_line(projected[2][0], projected[2][1], projected[6][0], projected[6][1], color);
    lcd_draw_line(projected[3][0], projected[3][1], projected[7][0], projected[7][1], color);
}





