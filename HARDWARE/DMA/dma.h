#ifndef __DMA_H
#define __DMA_H
#include "stm32f10x.h"


void MYDMA_Config(DMA_Channel_TypeDef*DMA_CHx,uint32_t cpar,uint32_t cmar,uint16_t cndtr);//≈‰÷√DMA1_CHx

void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx);// πƒ‹DMA1_CHx

#endif


