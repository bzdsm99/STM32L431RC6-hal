**Add rules to help the model understand your coding preferences, including preferred frameworks, coding styles, and other conventions.**
这个项目是主要使用HAL库开发STM32L431RC6，使用的是Embedded IDE插件在VSCode进行开发。
默认有外部高速和低速时钟，系统时钟为最高的80MHz。

用户代码在
.\Core\Inc
.\Core\Src
.\Drivers\SYSTEM
其他都是ST官方提供的驱动代码
HAL库的驱动代码在
.\Drivers\STM32L4xx_HAL_Driver\Inc
.\Drivers\STM32L4xx_HAL_Driver\Src

                         STM32L431RCx  LQFP-64 顶视图
┌-----┐
│  ●  │ 1  = PC13 / TAMP1 / RTC_TS / RTC_OUT / WKUP2
│     │ 2  = PC14 / OSC32_IN
│     │ 3  = PC15 / OSC32_OUT
│     │ 4  = PH0  / OSC_IN
│     │ 5  = PH1  / OSC_OUT
│     │ 6  = NRST
│     │ 7  = PC0  / I2C3_SCL  (AF4)
│     │ 8  = PC1  / I2C3_SDA  (AF4)
│     │ 9  = PC2
│     │10  = PC3
│     │11  = VSSA
│     │12  = VDDA
│     │13  = PA0  / ADC1_IN5 / TIM2_CH1 / WKUP1
│     │14  = PA1  / ADC1_IN6 / TIM2_CH2
│     │15  = PA2  / ADC1_IN7 / TIM2_CH3 / LPUART1_TX
│     │16  = PA3  / ADC1_IN8 / TIM2_CH4 / LPUART1_RX
│     │17  = PA4  / ADC1_IN9 / SPI1_NSS / DAC1_OUT1
│     │18  = PA5  / ADC1_IN10/ SPI1_SCK / DAC1_OUT2
│     │19  = PA6  / ADC1_IN11/ SPI1_MISO / TIM3_CH1
│     │20  = PA7  / ADC1_IN12/ SPI1_MOSI / TIM3_CH2 / I2C3_SCL(AF4)
│     │21  = PB0  / ADC1_IN15/ TIM3_CH3
│     │22  = PB1  / ADC1_IN16/ TIM3_CH4
│     │23  = PB2
│     │24  = PB10 / I2C2_SCL(AF4) / TIM2_CH3
│     │25  = PB11 / I2C2_SDA(AF4) / TIM2_CH4
│     │26  = VSS
│     │27  = VDD
│     │28  = PB12 / SPI2_NSS / I2C2_SMBA(AF4)
│     │29  = PB13 / SPI2_SCK / I2C2_SCL(AF4)
│     │30  = PB14 / SPI2_MISO / I2C2_SDA(AF4)
│     │31  = PB15 / SPI2_MOSI
│     │32  = PC6
│     │33  = PC7
│     │34  = PC8
│     │35  = PC9
│     │36  = PA8  / MCO / TIM1_CH1 / I2C3_SMBA(AF4)
│     │37  = PA9  / USART1_TX / TIM1_CH2 / I2C1_SCL(AF4)
│     │38  = PA10 / USART1_RX / TIM1_CH3 / I2C1_SDA(AF4)
│     │39  = PA11 / CAN_RX / TIM1_CH4 / I2C1_SCL(AF4)
│     │40  = PA12 / CAN_TX / TIM1_ETR / I2C1_SDA(AF4)
│     │41  = PA13 / SWDIO
│     │42  = PA14 / SWCLK
│     │43  = PA15 / JTDI / SPI3_NSS / I2C1_SMBA(AF4)
│     │44  = PC10
│     │45  = PC11
│     │46  = PC12
│     │47  = PD2
│     │48  = PB3  / JTDO / SPI3_SCK
│     │49  = PB4  / NJTRST / SPI3_MISO / I2C3_SDA(AF4)
│     │50  = PB5  / SPI3_MOSI
│     │51  = PB6  / I2C1_SCL(AF4) / TIM4_CH1
│     │52  = PB7  / I2C1_SDA(AF4) / TIM4_CH2
│     │53  = BOOT0
│     │54  = PB8  / I2C1_SCL(AF4) / TIM4_CH3
│     │55  = PB9  / I2C1_SDA(AF4) / TIM4_CH4
│     │56  = VSS
│     │57  = VDD
│     │58  = PE0
│     │59  = PE1
│     │60  = PC13 … (与1脚同一引脚，64 封装无 PE2~PE15)
│     │61  = PC14 …
│     │62  = PC15 …
│     │63  = VBAT
│     │64  = VREF+
└-----┘

请遇到不了解的引脚功能，函数使用方法请联网搜索
注意:使用外设时几乎都要配置外设的时钟源,进行使能.不要忘了!
**Note: This file only applies to the current project, with each file limited to 10,000 characters. If you do not need to commit this file to a remote Git repository, please add it to .gitignore.**
