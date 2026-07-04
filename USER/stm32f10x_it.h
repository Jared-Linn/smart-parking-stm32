/**
  ******************************************************************************
  * @file    stm32f10x_it.h
  * @brief   中断服务函数头文件
  ******************************************************************************
  */

#ifndef __STM32F10X_IT_H
#define __STM32F10X_IT_H

#include "stm32f10x.h"

/* Cortex-M3 异常处理 */
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);

/* 外设中断处理 */
void TIM3_IRQHandler(void);
void USART1_IRQHandler(void);
void USART3_IRQHandler(void);
void EXTI3_IRQHandler(void);

#endif /* __STM32F10X_IT_H */
