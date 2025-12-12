#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32l4xx_hal.h" // HAL库头文件
#include "stdbool.h"
// 用的硬件IIC  I2C1的GPIO引脚 (PB6: SCL, PB7: SDA)
// 软件模拟IIC引脚定义
// #define MPU_IIC_SCL_PIN    GPIO_PIN_6
// #define MPU_IIC_SDA_PIN    GPIO_PIN_7
// #define MPU_IIC_GPIO_PORT  GPIOB

// IO方向设置
#define MPU_SDA_IN()  {MPU_IIC_GPIO_PORT->MODER &= ~(3 << (3 * 2)); MPU_IIC_GPIO_PORT->MODER |= (0 << (3 * 2));}
#define MPU_SDA_OUT() {MPU_IIC_GPIO_PORT->MODER &= ~(3 << (3 * 2)); MPU_IIC_GPIO_PORT->MODER |= (1 << (3 * 2));}

// IO操作宏定义
#define MPU_IIC_SCL(state) HAL_GPIO_WritePin(MPU_IIC_GPIO_PORT, MPU_IIC_SCL_PIN, state)
#define MPU_IIC_SDA(state) HAL_GPIO_WritePin(MPU_IIC_GPIO_PORT, MPU_IIC_SDA_PIN, state)
#define MPU_READ_SDA       HAL_GPIO_ReadPin(MPU_IIC_GPIO_PORT, MPU_IIC_SDA_PIN)

// MPU6050寄存器地址定义
#define MPU_SELF_TESTX_REG      0x0D
#define MPU_SELF_TESTY_REG      0x0E
#define MPU_SELF_TESTZ_REG      0x0F
#define MPU_SELF_TESTA_REG      0x10
#define MPU_SAMPLE_RATE_REG     0x19
#define MPU_CFG_REG             0x1A
#define MPU_GYRO_CFG_REG        0x1B
#define MPU_ACCEL_CFG_REG       0x1C
#define MPU_MOTION_DET_REG      0x1F
#define MPU_FIFO_EN_REG         0x23
#define MPU_I2CMST_CTRL_REG     0x24
#define MPU_I2CSLV0_ADDR_REG    0x25
#define MPU_I2CSLV0_REG         0x26
#define MPU_I2CSLV0_CTRL_REG    0x27
#define MPU_I2CSLV1_ADDR_REG    0x28
#define MPU_I2CSLV1_REG         0x29
#define MPU_I2CSLV1_CTRL_REG    0x2A
#define MPU_I2CSLV2_ADDR_REG    0x2B
#define MPU_I2CSLV2_REG         0x2C
#define MPU_I2CSLV2_CTRL_REG    0x2D
#define MPU_I2CSLV3_ADDR_REG    0x2E
#define MPU_I2CSLV3_REG         0x2F
#define MPU_I2CSLV3_CTRL_REG    0x30
#define MPU_I2CSLV4_ADDR_REG    0x31
#define MPU_I2CSLV4_REG         0x32
#define MPU_I2CSLV4_DO_REG      0x33
#define MPU_I2CSLV4_CTRL_REG    0x34
#define MPU_I2CSLV4_DI_REG      0x35

#define MPU_I2CMST_STA_REG      0x36
#define MPU_INTBP_CFG_REG       0x37
#define MPU_INT_EN_REG          0x38
#define MPU_INT_STA_REG         0x3A

#define MPU_ACCEL_XOUTH_REG     0x3B
#define MPU_ACCEL_XOUTL_REG     0x3C
#define MPU_ACCEL_YOUTH_REG     0x3D
#define MPU_ACCEL_YOUTL_REG     0x3E
#define MPU_ACCEL_ZOUTH_REG     0x3F
#define MPU_ACCEL_ZOUTL_REG     0x40

#define MPU_TEMP_OUTH_REG       0x41
#define MPU_TEMP_OUTL_REG       0x42

#define MPU_GYRO_XOUTH_REG      0x43
#define MPU_GYRO_XOUTL_REG      0x44
#define MPU_GYRO_YOUTH_REG      0x45
#define MPU_GYRO_YOUTL_REG      0x46
#define MPU_GYRO_ZOUTH_REG      0x47
#define MPU_GYRO_ZOUTL_REG      0x48

#define MPU_I2CSLV0_DO_REG      0x63
#define MPU_I2CSLV1_DO_REG      0x64
#define MPU_I2CSLV2_DO_REG      0x65
#define MPU_I2CSLV3_DO_REG      0x66

#define MPU_I2CMST_DELAY_REG    0x67
#define MPU_SIGPATH_RST_REG     0x68
#define MPU_MDETECT_CTRL_REG    0x69
#define MPU_USER_CTRL_REG       0x6A
#define MPU_PWR_MGMT1_REG       0x6B
#define MPU_PWR_MGMT2_REG       0x6C
#define MPU_FIFO_CNTH_REG       0x72
#define MPU_FIFO_CNTL_REG       0x73
#define MPU_FIFO_RW_REG         0x74
#define MPU_DEVICE_ID_REG       0x75
#define MPU_CONFIG_REG          0x1A
// MPU6050 IIC地址
#define MPU_ADDR                0x68


// 函数声明
bool MPU_Init(void);
uint8_t MPU_Write_Byte(uint8_t reg, uint8_t data);
uint8_t MPU_Read_Byte(uint8_t reg);


uint8_t MPU_Set_Gyro_Fsr(uint8_t fsr);
uint8_t MPU_Set_Accel_Fsr(uint8_t fsr);
uint8_t MPU_Set_LPF(uint16_t lpf);
uint8_t MPU_Set_Rate(uint16_t rate);
uint8_t MPU_Set_Fifo(uint8_t sens);

float MPU_Get_Temperature(void);
void MPU_Get_Gyroscope(float *gx, float *gy, float *gz);
void MPU_Get_Accelerometer(float *ax, float *ay, float *az);
//void MPU_Get_Euler_Angles(float *pitch, float *roll, float *yaw);




#endif



