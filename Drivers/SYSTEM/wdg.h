#ifndef __WDG_H
#define __WDG_H
#ifdef __cplusplus
extern "C"
{
#endif
#include "sys.h"

void iwdg_init(uint8_t prer, uint16_t rlr);
void iwdg_feed(void);

void wwdg_init(uint8_t tr, uint8_t wr, uint32_t fprer);

#ifdef __cplusplus
}
#endif
#endif




























