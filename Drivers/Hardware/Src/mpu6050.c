#include "stm32l4xx_hal.h" // 引入HAL库头文件
#include "mpu6050.h"
#include "delay.h"
// #include "usart.h"
#include "math.h"

// 添加hi2c1的定义
I2C_HandleTypeDef hi2c1;
static uint16_t Gyro_Fsr = 0,Accel_Fsr = 0;


// 初始化I2C1的GPIO引脚 (PB6: SCL, PB7: SDA)
void MPU_I2C_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 使能GPIOB时钟
    __HAL_RCC_GPIOB_CLK_ENABLE();
    
    // 配置GPIO为I2C模式
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7; // I2C1_SCL(PB6), I2C1_SDA(PB7)
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;        // 开漏输出模式
    GPIO_InitStruct.Pull = GPIO_PULLUP;             // 上拉电阻
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; // 高速
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;      // 复用功能AF4 (I2C1)
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

// 使用硬件IIC读取一个字节
uint8_t MPU_Read_Byte(uint8_t reg) {
    uint8_t res;
    if (HAL_I2C_Mem_Read(&hi2c1, MPU_ADDR << 1, reg, I2C_MEMADD_SIZE_8BIT, &res, 1, 100) != HAL_OK) {
        //printf("MPU_Read_Byte: No ACK received for register! reg=0x%02X\r\n", reg);
        return 0xFF; // 返回错误值
    }
    return res;
}

// 使用硬件IIC写入一个字节
uint8_t MPU_Write_Byte(uint8_t reg, uint8_t data) {
    if (HAL_I2C_Mem_Write(&hi2c1, MPU_ADDR << 1, reg, I2C_MEMADD_SIZE_8BIT, &data, 1, 100) != HAL_OK) {
        //printf("MPU_Write_Byte: No ACK received for register! reg=0x%02X\r\n", reg);
        return 1; // 返回错误值
    }
    return 0;
}



// MPU6050初始化
uint8_t MPU_Init(void) {
    uint8_t res;

    // 使能I2C1时钟
    __HAL_RCC_I2C1_CLK_ENABLE();
    
    MPU_I2C_GPIO_Init();
    
    // 配置I2C1外设
    hi2c1.Instance = I2C1;
    hi2c1.Init.Timing = 0x10805E82; // 这是一个针对80MHz APB1时钟的典型值，具体值需要根据实际钟计算
    hi2c1.Init.OwnAddress1 = 0;
    hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.OwnAddress2 = 0;
    hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
        //printf("I2C1初始化失败\r\n");
        return 1;
    }
    
    MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0x80); // 复位MPU6050
    //status ? printf("MPU6050初始化失败\r\n") : printf("MPU6050初始化成功\r\n");
    
    delay_ms(100);
    MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0x00); // 唤醒MPU6050
    //status ? printf("MPU6050唤醒失败\r\n") : printf("MPU6050唤醒成功\r\n");

    MPU_Set_Gyro_Fsr(3);     // 设置陀螺仪量程 ±2000dps
    MPU_Set_Accel_Fsr(0);    // 设置加速度计量程 ±2g
    MPU_Set_Rate(50);        // 设置采样率 50Hz

    MPU_Write_Byte(MPU_INT_EN_REG, 0x00);   // 关闭所有中断
    MPU_Write_Byte(MPU_USER_CTRL_REG, 0x00); // 关闭I2C主模式
    MPU_Write_Byte(MPU_FIFO_EN_REG, 0x00);  // 关闭FIFO
    MPU_Write_Byte(MPU_INTBP_CFG_REG, 0x80); // INT引脚低电平有效
    // 配置DLPF带宽为42Hz
    MPU_Write_Byte(MPU_CONFIG_REG, 0x03); // 0x03 对应 42Hz 带宽
    //MPU_Write_Byte(MPU_CONFIG_REG, 0x05); // 0x05 对应 10Hz 带宽

    res = MPU_Read_Byte(MPU_DEVICE_ID_REG);
    //printf("res = %d,MPU_ADDR = %d\r\n",res,MPU_ADDR);
    if (res == 0x71 || res == 0x70) { // 检查器件ID是否为0x71或0x70
        MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0x01); // 设置CLKSEL, PLL X轴为参考
        MPU_Write_Byte(MPU_PWR_MGMT2_REG, 0x00); // 加速度与陀螺仪都工作
        MPU_Set_Rate(50); // 设置采样率为50Hz
    } else {
        return 1; // 初始化失败
    }

    return 0; // 初始化成功
}

// 设置陀螺仪量程
uint8_t MPU_Set_Gyro_Fsr(uint8_t fsr) {
    switch (fsr)    //+-dps
    {
    case 0:
        Gyro_Fsr = 250;
        break;
    case 1:
        Gyro_Fsr = 500;
        break;
    case 2:
        Gyro_Fsr = 1000;
        break;
    case 3:
        Gyro_Fsr = 2000;
        break;
    }
    return MPU_Write_Byte(MPU_GYRO_CFG_REG, fsr << 3);
}

// 设置加速度计量程
uint8_t MPU_Set_Accel_Fsr(uint8_t fsr) {
    switch (fsr)    //+-g
    {
    case 0:
        Accel_Fsr = 2;
        break;
    case 1:
        Accel_Fsr = 4;
        break;
    case 2:
        Accel_Fsr = 8;
        break;
    case 3:
        Accel_Fsr = 16;
        break;
    }
    return MPU_Write_Byte(MPU_ACCEL_CFG_REG, fsr << 3);
}

// 设置数字低通滤波器
uint8_t MPU_Set_LPF(uint16_t lpf) {
    uint8_t data = 0;
    if (lpf >= 188) data = 1;
    else if (lpf >= 98) data = 2;
    else if (lpf >= 42) data = 3;
    else if (lpf >= 20) data = 4;
    else if (lpf >= 10) data = 5;
    else data = 6;
    return MPU_Write_Byte(MPU_CFG_REG, data);
}

// 设置采样率
uint8_t MPU_Set_Rate(uint16_t rate) {
    uint8_t data;
    if (rate > 1000) rate = 1000;
    if (rate < 4) rate = 4;
    data = 1000 / rate - 1;
    MPU_Write_Byte(MPU_SAMPLE_RATE_REG, data);
    return MPU_Set_LPF(rate / 2);
}


/**
  * @brief      获取MPU6050温度值
  * @retval     温度值  单位摄氏度
  */
float MPU_Get_Temperature(void) {
    int16_t temperature_raw = (int16_t) (MPU_Read_Byte(MPU_TEMP_OUTH_REG) << 8) + MPU_Read_Byte(MPU_TEMP_OUTL_REG);
    //float temperature = temperature_raw / 340.0f + 36.53f;    //MPU6050
    float temperature = temperature_raw / 333.87f + 21.0f;      //MPU6500
    return temperature;
}

/**
  * @brief      获取陀螺仪数据
  * @param      gx  陀螺仪X轴的旋转度数
  * @param      gy  陀螺仪Y轴的旋转度数
  * @param      gz  陀螺仪Z轴的旋转度数
  */
void MPU_Get_Gyroscope(float *gx, float *gy, float *gz) {
    static float Unit_factor;
    int16_t gx_raw = (int16_t) (MPU_Read_Byte(MPU_GYRO_XOUTH_REG) << 8) + MPU_Read_Byte(MPU_GYRO_XOUTL_REG);
    int16_t gy_raw = (int16_t) (MPU_Read_Byte(MPU_GYRO_YOUTH_REG) << 8) + MPU_Read_Byte(MPU_GYRO_YOUTL_REG);
    int16_t gz_raw = (int16_t) (MPU_Read_Byte(MPU_GYRO_ZOUTH_REG) << 8) + MPU_Read_Byte(MPU_GYRO_ZOUTL_REG);

    Unit_factor = Gyro_Fsr / (pow(2,16) / 2);  // 陀螺仪量程转换因子
    //printf("Unit_factor = %f ,Gyro_Fsr =%d ,target : 6.1035e-2f\r\n",Unit_factor,Gyro_Fsr);
    *gx = gx_raw * Unit_factor;
    *gy = gy_raw * Unit_factor;
    *gz = gz_raw * Unit_factor;
}


/**
  * @brief      获取加速度计数据
  * @param      gx  加速度计X轴的重力加速度
  * @param      gy  加速度计Y轴的重力加速度
  * @param      gz  加速度计Z轴的重力加速度
  */
void MPU_Get_Accelerometer(float *ax, float *ay, float *az) {
    static float Unit_factor;
    int16_t ax_raw = (int16_t) (MPU_Read_Byte(MPU_ACCEL_XOUTH_REG) << 8) + MPU_Read_Byte(MPU_ACCEL_XOUTL_REG);
    int16_t ay_raw = (int16_t) (MPU_Read_Byte(MPU_ACCEL_YOUTH_REG) << 8) + MPU_Read_Byte(MPU_ACCEL_YOUTL_REG);
    int16_t az_raw = (int16_t) (MPU_Read_Byte(MPU_ACCEL_ZOUTH_REG) << 8) + MPU_Read_Byte(MPU_ACCEL_ZOUTL_REG);

    Unit_factor = Accel_Fsr / (pow(2,16) / 2);  // 陀螺仪量程转换因子
    //printf("Unit_factor = %f ,Gyro_Fsr =%d ,target : 6.1035e-5f\r\n",Unit_factor,Accel_Fsr);
    *ax = ax_raw * Unit_factor;
    *ay = ay_raw * Unit_factor;
    *az = az_raw * Unit_factor;
}



// 设置 MPU6050 的 FIFO 功能
// sens: 启用的传感器位掩码 (TEMP_OUT, GYRO_X/Y/Z, ACCEL_X/Y/Z)

/**
  * @brief      设置 MPU6050 的 FIFO 功能
  * @param      sens: 启用的传感器位掩码 (TEMP_OUT, GYRO_X/Y/Z, ACCEL_X/Y/Z)
  * @retval     0, 成功; 其他, 失败
  */
uint8_t MPU_Set_Fifo(uint8_t sens)
{
    return MPU_Write_Byte(MPU_FIFO_EN_REG, sens); // 配置 FIFO 使能寄存器
}






