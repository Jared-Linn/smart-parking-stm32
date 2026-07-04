#ifndef __IWDG_H

#define __IWDG_H

#include "stm32f10x.h"
void IWDG_Init(uint8_t prer,uint16_t rlr) ;//独立看门狗初始化
void IWDG_Feed(void);//喂狗

#endif

