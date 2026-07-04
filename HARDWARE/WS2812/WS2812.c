#include "WS2812.h"
#include "oled.h"
#include "delay.h"

uint8_t  ws2812_flag;
uint32_t ws2812_RGB = 0xffffff;
/***************************************************************
*函数名：	WS2812_Init
*功  能：	WS2812的IO端口初始化
*参  数：	无
*返回值：	无
***************************************************************/
void WS2812_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//使能PORTE,PORTE时钟
 	RCC_APB2PeriphClockCmd(WS2812_PORT_CLK,ENABLE);		
	
	GPIO_InitStructure.GPIO_Pin   = WS2812_PORT_PIN;
	//设置成上拉输入
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; 	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//初始化GPIOA1
 	GPIO_Init(WS2812_PORT,&GPIO_InitStructure);			
	
	WS2812_Reset();
	
	ws2812_flag = 0;
	
	OLED_Clear();
	OLED_ShowString(110,6,"K1",16);
}

/***************************************************************
*函数名：	WS2812_Delay
*功  能：	WS2812延时函数
*参  数：	无
*返回值：	无
***************************************************************/
void WS2812_Delay(uint8_t nns)
{
	uint8_t i;
	i = nns;
	do
	{
		;
	}while(--i);
}

/***************************************************************
*函数名：	WS2812_Set_1
*功  能：	WS2812输出“1码”
*参  数：	无
*返回值：	无
*注：“1码”的表示方法为850ns±150ns高电平,400ns±150ns低电平
	一定要使用寄存器操作,否则IO速度不够
***************************************************************/
void WS2812_Set_1(void)
{
	WS2812_PORT->BSRR = WS2812_PORT_PIN;
	WS2812_Delay(5);
	WS2812_PORT->BRR = WS2812_PORT_PIN;
}

/***************************************************************
*函数名：	WS2812_Set_0
*功  能：	WS2812输出“0码”
*参  数：	无
*返回值：	无
*注：“0码”的表示方法为850ns±150ns低电平,400ns±150ns高电平
	一定要使用寄存器操作,否则IO速度不够
***************************************************************/
void WS2812_Set_0(void)
{
	WS2812_PORT->BSRR = WS2812_PORT_PIN;
	WS2812_Delay(1);
	WS2812_PORT->BRR = WS2812_PORT_PIN;
	WS2812_Delay(2);
}

/***************************************************************
*函数名：	WS2812_Reset
*功  能：	WS2812复位
*参  数：	无
*返回值：	无
***************************************************************/
void WS2812_Reset(void)
{
	WS2812_PORT->BRR = WS2812_PORT_PIN;
	Delay_us(60);
}
	
/***************************************************************
*函数名：	WS2812_Handle
*功  能：	控制WS2812产生不同的颜色
*参  数：	R8G8B8,从高位到低位按顺序表示：红绿蓝
*返回值：	无
***************************************************************/
void WS2812_Handle(uint32_t R8G8B8)
{
	int8_t   i,tp;
	uint32_t G8R8B8;
	
	ws2812_flag = 0;
	G8R8B8 = ((R8G8B8 & 0x00ff00) << 8) + ((R8G8B8 & 0xff0000) >> 8) + (R8G8B8 & 0x0000ff);
	OLED_ShowString(32,2,"R:",16);
	OLED_ShowString(32,4,"G:",16);
	OLED_ShowString(32,6,"B:",16);
	OLED_ShowInt32Num(56,2,((G8R8B8 >> 8) & 0xff),3,16);
	OLED_ShowInt32Num(56,4,((G8R8B8 >> 16) & 0xff),3,16);
	OLED_ShowInt32Num(56,6,(G8R8B8 & 0xff),3,16);
	
	for(i=23;i>=0;i--)
	{
		tp = (G8R8B8 >> i) & 0x01;
		if(tp == 1)
		{
			WS2812_Set_1();
		}
		else
		{
			WS2812_Set_0();
		}
	}
}
//										endfile
