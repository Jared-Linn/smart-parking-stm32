#include "UltrasonicWave.h"
#include "timer.h"
#include "delay.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_tim.h"
#include "oled.h"
#include "stdio.h"


unsigned long time_count;
uint8_t display_flag = 0;
char str[100];
// 计数值
uint32_t   status=0;
// 读回值
uint32_t   real_time;
// 距离计算值
float	dis;							


//定时器3中断服务程序	 
void TIM3_IRQHandler(void)
{
	//判断定时器3更新中断触发
	if(TIM_GetITStatus(TIM3,TIM_IT_Update) == SET)      
	{
		status++;
	}
	//清除定时器3更新中断触发
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);	
}


//通用定时器中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器3!
void Tran_Timerx_Software_Init(uint16_t arr,uint16_t psc)
{
	//定义结构体变量
	TIM_TimeBaseInitTypeDef TIM_TimeBaseTypeDefStructure;
	NVIC_InitTypeDef NVIC_TypeDefStructure;
	
	//开启定时器3时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);	
	
	//定时器预装初值
	TIM_TimeBaseTypeDefStructure.TIM_Period = arr;  
	//预分频系数
	TIM_TimeBaseTypeDefStructure.TIM_Prescaler = psc; 
	//计数模式 自动重载
	TIM_TimeBaseTypeDefStructure.TIM_CounterMode = TIM_CounterMode_Down;
	//时钟分频因子
	TIM_TimeBaseTypeDefStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	//重复计数设置
	TIM_TimeBaseTypeDefStructure.TIM_RepetitionCounter = 0;
	//初始化定时器
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseTypeDefStructure);

	//选择中断线
	NVIC_TypeDefStructure.NVIC_IRQChannel = TIM3_IRQn;
	//抢占优先级
	NVIC_TypeDefStructure.NVIC_IRQChannelPreemptionPriority = 1;
	//响应优先级
	NVIC_TypeDefStructure.NVIC_IRQChannelSubPriority = 3;  
	// 中断线使能
	NVIC_TypeDefStructure.NVIC_IRQChannelCmd = ENABLE;
	//中断线优先级初始化
	NVIC_Init(&NVIC_TypeDefStructure);
	
	//开启定时器中断
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	//关闭定时器3
	TIM_Cmd(TIM3,DISABLE);
}


//外部中断初始化程序
void Tran_EXTI_Init(void)
{
	//定义结构体变量
	GPIO_InitTypeDef GPIO_TypeStructure;
	EXTI_InitTypeDef EXTI_TypeDefStructure;
	NVIC_InitTypeDef NVIC_TypeDefStructure;
	
	//开启GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA
							|RCC_APB2Periph_AFIO,ENABLE);
	
	//选择端口
	GPIO_TypeStructure.GPIO_Pin = GPIO_Pin_2;
	//选择工作模式  通用推挽输出
	GPIO_TypeStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	//选择输出最大速度  50MHz
	GPIO_TypeStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_TypeStructure);
	
	
	GPIO_TypeStructure.GPIO_Pin = GPIO_Pin_3;
	//选择工作模式  上拉输入
	GPIO_TypeStructure.GPIO_Mode = GPIO_Mode_IPU;
	//选择输出最大速度  50MHz
	GPIO_TypeStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//初始化GPIOA
	GPIO_Init(GPIOA,&GPIO_TypeStructure);
	
	

	
	//GPIO与中断线关联
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource3);
	
	//外部中断配置
	//中断线
	EXTI_TypeDefStructure.EXTI_Line = EXTI_Line3;				
	
	//请求方式
	EXTI_TypeDefStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	//下降沿触发
	EXTI_TypeDefStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	//使能中断线
	EXTI_TypeDefStructure.EXTI_LineCmd = ENABLE;				
	EXTI_Init(&EXTI_TypeDefStructure);	
	
	//外部中断向量 EXTI9_5_IRQ优先级配置
	//中断向量选择
	NVIC_TypeDefStructure.NVIC_IRQChannel = EXTI3_IRQn;	
	//抢占优先级
	NVIC_TypeDefStructure.NVIC_IRQChannelPreemptionPriority = 2;
	//响应优先级
	NVIC_TypeDefStructure.NVIC_IRQChannelSubPriority  = 2;
	//使能中断向量
	NVIC_TypeDefStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_Init(&NVIC_TypeDefStructure);	
}

//外部中断0服务程序
void EXTI3_IRQHandler(void)
{

	//判断相应中断线触发中断
	if(EXTI_GetITStatus(EXTI_Line3) == SET)		
	{
		// 关使能定时器3
		TIM_Cmd(TIM3,DISABLE);					
		real_time = status ;
		// 计算距离	定时10us，S=Vt/2（减2是误差补尝）单位：mm
		dis=(float)real_time*1.7-2;      		 	
	}
	//清除相应中断线中断标志位
	EXTI_ClearITPendingBit(EXTI_Line3);		   
}



// 左发生超声波函数
void Tran(void)
{
	uint8_t i;
	TIM_Cmd(TIM3,ENABLE);

	// 定时器清零
	status  = 0;			
	
	for(i=0;i<8;i++)
	{
		// 发送信号
		CSB_TX( Bit_SET );			
		Delay_us(12);    
		CSB_TX( Bit_RESET ); 
		Delay_us(12);		
	}
}
//										endfile

