/**
  ******************************************************************************
  * @file    stm32f10x_it.c
  * @brief   中断服务函数
  * @note    本项目的所有中断处理函数集中在此文件中
  *          - SysTick    → delay 延时基准
  *          - TIM3       → 超声波微秒计时
  *          - USART1     → 串口 CLI 接收
  *          - USART3     → ESP8266 WiFi 接收
  *          - EXTI3      → 超声波 ECHO 回响
  ******************************************************************************
  */

#include "stm32f10x_it.h"

/* ========== Cortex-M3 异常处理 ========== */

void NMI_Handler(void) {}
void HardFault_Handler(void)  { while(1); }
void MemManage_Handler(void)  { while(1); }
void BusFault_Handler(void)   { while(1); }
void UsageFault_Handler(void) { while(1); }
void SVC_Handler(void)        {}
void DebugMon_Handler(void)   {}
void PendSV_Handler(void)     {}

/**
  * @brief  SysTick 中断 — delay.c 的计时基准
  */
void SysTick_Handler(void)
{
    /* delay.c 使用轮询模式，此中断处理函数保持为空 */
}

/* ========== STM32F10x 外设中断 ========== */

/**
  * @brief  TIM3 中断 — 超声波测距计时
  */
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        /* 超声波驱动中的溢出计数 */
    }
}

/**
  * @brief  USART1 中断 — 串口 CLI 接收
  */
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        /* 数据由 CLI 模块在 main 循环中轮询读取 */
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

/**
  * @brief  USART3 中断 — ESP8266 WiFi 接收
  */
void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        /* ESP8266 响应数据接收 */
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}

/**
  * @brief  EXTI3 中断 — 超声波 ECHO 引脚 (PA3)
  */
void EXTI3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line3) != RESET)
    {
        /* 超声波回响信号边沿捕获 */
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
