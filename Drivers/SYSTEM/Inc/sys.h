//sys.h
#ifndef SYS_H
#ifdef __cplusplus
extern "C" {
#endif 
#include "stm32l4xx_hal.h"
#include "stdbool.h"
extern void Error_Handler(void);


void sys_nvic_set_vector_table(uint32_t baseaddr, uint32_t offset);
void sys_wfi_set(void);
void sys_intx_disable(void);
void sys_intx_enable(void);
void sys_msr_msp(uint32_t addr);
void sys_standby(void);
void sys_soft_reset(void);
void sys_stm32_clock_init(uint32_t plln);
void SystemClock_Config(void);

#ifdef __cplusplus
}
#endif
#endif // !SYS_H



