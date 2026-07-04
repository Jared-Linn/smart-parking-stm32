#include <stm32f10x.h>
#include "usart.h"
#include "misc.h"
uint8_t USART_RX_BUF[USART1_RxData_length]; 	//串口1接收数组
uint8_t Rx_num = 0;							//发送数据中间变量
uint8_t USART1_Rx_OK = RESET;				//发送数据标志位

/**************************************************************
*功  能：	串口1初始化
*参  数：	baudrate  波特率
*返回值：	无
**************************************************************/
void USART1_Configure(uint32_t baudrate)
{
	//定义结构体变量
	GPIO_InitTypeDef GPIO_TypeDefStructure;
	USART_InitTypeDef USART_TypeDefStructure;
	
	//开启GPIOA时钟 开启串口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA
						   |RCC_APB2Periph_USART1,ENABLE);
	//TX  复用推挽输出
	GPIO_TypeDefStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_TypeDefStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_TypeDefStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_TypeDefStructure);
	//RX 浮空输入
	GPIO_TypeDefStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_TypeDefStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA,&GPIO_TypeDefStructure);
	
	//波特率
	USART_TypeDefStructure.USART_BaudRate = baudrate;
	//工作方式    接收与发送
	USART_TypeDefStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	//硬件流控制  无
	USART_TypeDefStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	//校验位	  无
	USART_TypeDefStructure.USART_Parity =USART_Parity_No;
	//停止位	  1
	USART_TypeDefStructure.USART_StopBits = USART_StopBits_1;
	//数据长度  8
	USART_TypeDefStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1,&USART_TypeDefStructure);
#if USART1_ITRX_ENABLE  //0 查询模式 1 中断模式
	//中断优先级配置
	NVIC_TypeDefStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_TypeDefStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_TypeDefStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_TypeDefStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_TypeDefStructure);
	//开启串口1接收中断
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
#endif
	
	//打开串口1
	USART_Cmd(USART1,ENABLE);
	
}
/**************************************************************
*功  能：	串口查询模式
*参  数：	无
*返回值：	无
**************************************************************/
void USART1_Rx_Handler(void)
{
	uint8_t Res;
	//判断接收标志
	if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)  
	{
		//读取接收到的数据
		Res =USART_ReceiveData(USART1);	
		//向串口1发送数据
		USART_SendData(USART1, Res);
		//等待发送结束		
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
				 
     } 
}

/**************************************************************
*功  能：	串口中断服务函数
*参  数：	无
*返回值：	无
**************************************************************/
//接收状态标记
uint16_t USART_RX_STA=0;       
void USART1_IRQHandler()
{
	uint8_t Res;
	//接收中断
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)    
	{
		//读取接收到的数据
		Res =USART_ReceiveData(USART1);	
		//向串口1发送数据
		USART_SendData(USART1, Res);
		//等待发送结束   	
		while(USART_GetFlagStatus(USART1,USART_FLAG_TC)!=SET);
	} 
}

/**************************************************************
*功  能：	串口1以字节为单位发送
*参  数：	data 发送的字节
*返回值：	无
**************************************************************/
void USART1_Send_Byte(uint8_t data)
{
	//串口1发送数据
	USART_SendData(USART1,data);
	//等待发送完成
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);

}

/**************************************************************
*功  能：	串口1指定长度发送数据
*参  数：	str 数据地址指针  length_num 数据长度
*返回值：	无
**************************************************************/
void USART1_Send_Data_length(uint8_t *str,uint8_t length_num)
{
	uint8_t Tx_num = 0;
	do
	{
		//指针加一
		USART1_Send_Byte(*(str+Tx_num));	
		Tx_num++;
	}
	//指针小于数据长度
	while(Tx_num < length_num);				
}

/**************************************************************
*功  能：	串口1发送字符串
*参  数：	str 数据地址指针
*返回值：	无
**************************************************************/
void USART1_Send_String(uint8_t *str)
{
	uint8_t Tx_num = 0;
	do
	{
		//指针加一
		USART1_Send_Byte(*(str+Tx_num));	
		Tx_num++;
	}
	//判断该指针对应字符是否为结束符
	while(*(str+Tx_num) != '\0');			
}
//										endfile	


