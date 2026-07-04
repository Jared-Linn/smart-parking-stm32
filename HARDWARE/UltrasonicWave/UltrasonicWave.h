#ifndef __UltrasonicWave_H
#define	__UltrasonicWave_H

#include "stm32f10x.h"

#define CSB_TX(X)   GPIO_WriteBit( GPIOA , GPIO_Pin_2 , (BitAction)X) 		// PA8 ·¢ËÍ³¬Éù²¨Òý½Å 


extern uint32_t real_time;
extern uint32_t status;
extern float dis;

void Tran(void);
void Tran_Timerx_Software_Init(uint16_t arr,uint16_t psc);
void Tran_EXTI_Init(void);


#endif /* __UltrasonicWave_H */

