#include "iwdg.h"
#include "stm32f10x.h"

/**************************************************************
*功  能：	初始化独立看门狗
*参  数：	prer 预分频值  rlr 重装载值
*返回值：	无
**************************************************************/
void IWDG_Init(uint8_t prer,uint16_t rlr) 
{	
	//使能对寄存器IWDG_PR和IWDG_RLR的写操作
 	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); 
	//设置IWDG预分频值:设置IWDG预分频值为64
	IWDG_SetPrescaler(prer);  
	//设置IWDG重装载值
	IWDG_SetReload(rlr); 
	//按照IWDG重装载寄存器的值重装载IWDG计数器
	IWDG_ReloadCounter();  
	//使能IWDG
	IWDG_Enable();  
}

/**************************************************************
*功  能：	喂狗狗阻止复位
*参  数：	无
*返回值：	无
**************************************************************/
void IWDG_Feed(void)
{   
	//reload	
 	IWDG_ReloadCounter();									   
}
//										endfile
