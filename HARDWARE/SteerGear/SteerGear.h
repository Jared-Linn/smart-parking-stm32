#ifndef __STEERGEAR_H
#define __STEERGEAR_H

#include "stm32f10x.h"

void TIM2_PWM_Init(uint32_t arr,uint32_t psc);
void SteerGear_PWM(int16_t duty);

#endif
