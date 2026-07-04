#include "stm32f10x.h"
#include "usart3.h"

//储存串口一接收数据的数组
uint8_t usart3_buffer[USART3_RX_SIZE] = { 0 };
//储存串口一接收数据数组的长度
uint16_t usart3_length = 0;

/*******************************************************
功　能：串口3配置在PA9和PA30上并设置波特率
参　数：baudrate -> 波特率 一般为335200
返回值：无
********************************************************/
void USART3_Hardware_Init(uint32_t baudrate)
{
	GPIO_InitTypeDef GPIO_TypeDefStructure;
	USART_InitTypeDef USART_TypeDefStructure;
	NVIC_InitTypeDef NVIC_TypeDefStructure;
	
	//开启GPIOB时钟和串口时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	//TX  复用推挽输出
	GPIO_TypeDefStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_TypeDefStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_TypeDefStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_TypeDefStructure);
	//RX 浮空输入
	GPIO_TypeDefStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_TypeDefStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB,&GPIO_TypeDefStructure);
	
	//波特率
	USART_TypeDefStructure.USART_BaudRate = baudrate;
	//工作方式    接收与发送
	USART_TypeDefStructure.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
	//硬件流控制  无
	USART_TypeDefStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	//校验位	  无
	USART_TypeDefStructure.USART_Parity =USART_Parity_No;
	//停止位	  3
	USART_TypeDefStructure.USART_StopBits = USART_StopBits_1;
	//数据位长度    8
	USART_TypeDefStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART3,&USART_TypeDefStructure);
	
	//串口3中断响应优先级设置
	NVIC_TypeDefStructure.NVIC_IRQChannel = USART3_IRQn;		//选择中断通道
	NVIC_TypeDefStructure.NVIC_IRQChannelPreemptionPriority = 0;//抢占优先级0
	NVIC_TypeDefStructure.NVIC_IRQChannelSubPriority = 6;		//子优先级7
	NVIC_TypeDefStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_TypeDefStructure);			//根据指定的参数初始化NVIC寄存器 
	
	//开启串口3接收中断
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
	//打开串口3
	USART_Cmd(USART3,ENABLE);
}
/*******************************************************
功　能：串口3中断接收数据
使用此接收函数时，可以通过判断usart3_length的长度，来确定是
否接收完成，接收完成后，记得使用usart3_buffer_Clear();清零哦！
参　数：无
返回值：无
********************************************************/
void USART3_IRQHandler(void)
{
	//判断串口接收中断标志位
    if(USART_GetITStatus(USART3,USART_IT_RXNE) == SET)
    {
		//读取串口数据
		usart3_buffer[usart3_length++] = USART_ReceiveData(USART3);
		//如果储存超出长度
		if(usart3_length >= USART3_RX_SIZE)
		{
			//重新从数组0开始储存字节
			usart3_length = 0;
		}
    }
	//清除串口中断接收标志位
	USART_ClearITPendingBit(USART3,USART_IT_RXNE);
}
/*******************************************************
功　能：清空接收的数据
参　数：无
返回值：无
********************************************************/
void USART3_Clear_Buffer(void)
{	
	while(usart3_length)      //清空接收到的数据
	{
		usart3_length--;
		//数据清零
		usart3_buffer[usart3_length] = 0;
	}
}
/*******************************************************
功　能：通过串口3发送一个字节，如0x32、0xff等
参　数：hex -> 字节
返回值：无
********************************************************/
void USART3_Send_Byte(uint8_t byte)
{
    USART_SendData(USART3,byte);
    while(USART_GetFlagStatus(USART3,USART_FLAG_TXE) == RESET);
}

/*******************************************************
功　能：通过串口3发送一个数组
参　数：*buf -> 指针指向一个数组
		 length -> 数组的长度
返回值：无
********************************************************/
void USART3_Send_Length(uint8_t *buf,uint8_t length)
{
	uint8_t len = 0;
	for(len = 0; len < length; len++)
	{
		USART3_Send_Byte(buf[len]);
	}
}

/*******************************************************
功　能：通过串口3发送一串字符
参　数：*str -> 指针指向一串字符,若发送数组时,请保证数组
数据中没有0x00,否则将会把0x00后的数据全部丢掉,发送数组推荐
使用USART3_Send_Length(*buf,length);函数
返回值：无
********************************************************/
void USART3_Send_String(char *str)
{
    uint16_t Tx_cut = 0;
	while(*(str+Tx_cut) != '\0')
    {
        USART3_Send_Byte(*(str+Tx_cut));
		Tx_cut++;
    }
}

