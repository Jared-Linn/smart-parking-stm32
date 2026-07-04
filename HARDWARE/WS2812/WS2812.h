#ifndef __WS2812_H
#define __WS2812_H

#include "sys.h"

#define WS2812_PORT_CLK		RCC_APB2Periph_GPIOA
#define WS2812_PORT			GPIOA
#define WS2812_PORT_PIN		GPIO_Pin_0


void WS2812_Init(void);
void WS2812_Set_1(void);
void WS2812_Set_0(void);
void WS2812_Reset(void);
void WS2812_Handle(uint32_t G8R8B8);


#endif
