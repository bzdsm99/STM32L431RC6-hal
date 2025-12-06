//真随机数硬件外设
//rng.h
#ifndef __RNG_H
#define __RNG_H

#include "sys.h"

uint8_t rng_init(void);
uint32_t rng_get_random_num(void);
uint32_t rng_get_random_range(int min, int max);

#endif // !__RNG_H

