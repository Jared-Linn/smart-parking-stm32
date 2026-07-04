#include "i2c.h"
#include "delay.h"  
#include "oled.h"

/**************************************************************
*功  能：	IIC端口初始化
*参  数: 	无
*返回值: 	无 
**************************************************************/
void i2c_Port_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;
	//推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
 
	I2C_SCL=1;
	I2C_SDA=1;
} 

/**************************************************************
*功  能：	IIC输出端口初始化
*参  数: 	无
*返回值: 	无 
**************************************************************/
void SDA_OUT(void)
{  
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //开漏输出
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;	
  GPIO_Init(GPIOA,&GPIO_InitStructure);
}

/**************************************************************
*功  能：	IIC输入端口初始化
*参  数: 	无
*返回值: 	无 
**************************************************************/
void SDA_IN(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //浮空输入
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	
  GPIO_Init(GPIOA,&GPIO_InitStructure);
} 

/**************************************************************
*功  能：	产生IIC起始信号
*参  数: 	无
*返回值: 	无 
**************************************************************/
void I2C_Start(void)
{
	//sda线输出
	SDA_OUT();		
	I2C_SDA=1;	  	  
	I2C_SCL=1;
	Delay_us(4);
	//START:when CLK is high,DATA change form high to low 
 	I2C_SDA=0;		
	Delay_us(4);
	//钳住I2C总线，准备发送或接收数据 
	I2C_SCL=0;		
}	  

/**************************************************************
*功  能：	产生IIC停止信号
*参  数: 	无
*返回值: 	无 
**************************************************************/
void I2C_Stop(void)
{
	//sda线输出
	SDA_OUT();		
	I2C_SCL=0;
	//STOP:when CLK is high DATA change form low to high
	I2C_SDA=0;		
 	Delay_us(4);
	I2C_SCL=1;
    //发送I2C总线结束信号	
	I2C_SDA=1;		
	Delay_us(4);							   	
}

/**************************************************************
*功  能：	等待应答信号到来
*参  数: 	无
*返回值: 	1，接收应答失败
			0，接收应答成功
**************************************************************/
uint8_t I2C_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
    //	SDA_IN();		//SDA设置为输入  
	SDA_OUT();
	I2C_SDA=1;Delay_us(1);	   
	I2C_SCL=1;Delay_us(1);
	//SDA设置为输入	 
	SDA_IN();		
	while(SDA_I)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			I2C_Stop();
			return 1;
		}
	}
	//时钟输出0 
	I2C_SCL=0;			   
	return 0;  
} 

/**************************************************************
*功  能：	产生ACK应答
*参  数: 	无
*返回值: 	无 
**************************************************************/
void I2C_Ack(void)
{
	I2C_SCL=0;
	SDA_OUT();
	I2C_SDA=0;
	Delay_us(2);
	I2C_SCL=1;
	Delay_us(2);
	I2C_SCL=0;
}
		
/**************************************************************
*功  能：	不产生ACK应答
*参  数: 	无
*返回值: 	无 
**************************************************************/
void I2C_NAck(void)
{
	I2C_SCL=0;
	SDA_OUT();
	I2C_SDA=1;
	Delay_us(2);
	I2C_SCL=1;
	Delay_us(2);
	I2C_SCL=0;
}					 				     

/**************************************************************
*功  能：	IIC发送一个字节
*参  数: 	无
*返回值: 	返回从机有无应答，1，有应答；0，无应答	
**************************************************************/
void I2C_Send_Byte(uint8_t txd)
{                        
    uint8_t t;   
	SDA_OUT();
    //拉低时钟开始数据传输	
    I2C_SCL=0;			
    for(t=0;t<8;t++)
    {              
        I2C_SDA=(txd&0x80)>>7;
        txd<<=1;
        //对TEA5767这三个延时都是必须的		
		Delay_us(2);	
		I2C_SCL=1;
		Delay_us(2); 
		I2C_SCL=0;	
		Delay_us(2);
    }	 
} 	    

/**************************************************************
*功  能：	读1个字节
*参  数: 	ack=1时，发送ACK，ack=0，发送nACK 
*返回值: 	   无 
**************************************************************/
uint8_t I2C_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	//SDA设置为输入
	SDA_IN();			
    for(i=0;i<8;i++ )
	{
        I2C_SCL=0; 
        Delay_us(2);
		I2C_SCL=1;
        receive<<=1;
        if(SDA_I)receive++;   
		Delay_us(1); 
    }					 
    if (!ack)
		//发送nACK
        I2C_NAck();		
    else
		//发送ACK   
        I2C_Ack(); 		
    return receive;
}
//										endfile

