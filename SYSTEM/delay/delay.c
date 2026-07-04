#include <stm32f10x.h>

static uint8_t  fas_us = 0;//us延时倍乘数
static uint16_t fas_ms = 0;//ms延时倍乘数
/**********************************************************
*功  能：	延时初始化
*参  数：	无
*返回值：	无
**********************************************************/
void Delay_Init()
{
	//使用外部时钟
	SysTick->CTRL &= ~(1<<2);
	//微秒定时器初值
	fas_us = SystemCoreClock/8000000;
	//毫秒延时定时器初值
	fas_ms = fas_us*1000;				
}
/**********************************************************
*功  能：	微秒级延时
*参  数：	nus = 1 即 1us
*返回值：	无
**********************************************************/
void Delay_us(uint32_t nus)
{
	uint32_t temp = 0;

	//定时器预装初值
	SysTick->LOAD = nus*fas_us;
	//计数器清零
	SysTick->VAL = 0;
	//使能定时器
	SysTick->CTRL = 0x01;				
	do
	{
		//读状态
		temp = SysTick->CTRL;				
	}
	//判断状态
	while((temp&0x01)&&!(temp&(1<<16)));
	//关闭定时器
	SysTick->CTRL = 0x00;					
}
/**********************************************************
*功  能：	毫秒级别延时
*参  数：	num = 1 即 1ms   num <= 1864
*返回值：	无
**********************************************************/
void Delay_ms(uint16_t num)
{
	uint32_t temp = 0;

	//毫秒延时定时器预装初值
	SysTick->LOAD = (uint32_t)num*fas_ms;
	//计数器清零
	SysTick->VAL = 0;
	//使能定时器
	SysTick->CTRL = 0x01;				
	do
	{
		//读状态
		temp = SysTick->CTRL;			
	}
	//判断状态
	while((temp&0x01)&&!(temp&(1<<16)));
	//关闭定时器
	SysTick->CTRL = 0x00;				
}
//										endfile	
