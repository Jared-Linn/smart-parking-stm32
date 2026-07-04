#ifndef __USART3_H
#define __USART3_H

#include "stm32f10x.h"

//接收一次数据的最大长度
#define USART3_RX_SIZE 	200

extern uint8_t usart3_buffer[];						
extern uint16_t usart3_length;

void USART3_Hardware_Init(uint32_t baudrate);		//串口3初始化
void USART3_Send_Byte(uint8_t byte);				//通过串口3发送1个字节
void USART3_Send_String(char *src);					//通过串口3发送1串字符
void USART3_Send_Length(uint8_t *buf,uint8_t length);//通过串口3发送指定长度的数据
void USART3_Clear_Buffer(void);						//串口3接收数组清空

#endif

