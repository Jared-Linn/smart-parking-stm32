#include <stm32f10x.h>
#include "delay.h"
#include "beep.h"
/**************************************************************
*功  能：	蜂鸣器引脚初始化
*参  数：	无
*返回值：	无
**************************************************************/
void BEEP_Init(void)
{
	
	GPIO_InitTypeDef GPIO_TypeStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO_TypeStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_TypeStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_TypeStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_TypeStructure);
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);
}

/**************************************************************
*功  能：	实现蜂鸣器尖端鸣叫效果
*参  数：	无
*返回值：	无
**************************************************************/
void BEEP_Test(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_8);
	Delay_ms(500); 
	GPIO_ResetBits(GPIOA,GPIO_Pin_8);
	Delay_ms(500);
}
//										endfile	