#include "dma.h"

DMA_InitTypeDef DMA_InitStructure;

//保存DMA每次数据传送的长度 
uint16_t DMA1_MEM_LEN;	    
/**************************************************************
*功  能：	开启一次DMA传输
*参  数：	DMA_CHx DMA通道
*返回值：	无
*注  释：
				DMA1的各通道配置
				这里的传输形式是固定的,这点要根据不同的情况来修改
				从存储器->外设模式/8位数据宽度/存储器增量模式
				DMA_CHx:DMA通道CHx
				cpar:外设地址
				cmar:存储器地址
				cndtr:数据传输量 
**************************************************************/
void MYDMA_Config(DMA_Channel_TypeDef* DMA_CHx,uint32_t cpar,uint32_t cmar,uint16_t cndtr)
{
	//使能DMA传输
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	
	//将DMA的通道1寄存器重设为缺省值
	DMA_DeInit(DMA_CHx);   
	//长度
	DMA1_MEM_LEN=cndtr;
	//DMA外设基地址
	DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;
	//DMA内存基地址	
	DMA_InitStructure.DMA_MemoryBaseAddr = cmar; 
	//数据传输方向，从内存读取发送到外设
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST; 
	//DMA通道的DMA缓存的大小	
	DMA_InitStructure.DMA_BufferSize = cndtr;  
	//外设地址寄存器不变
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; 
	//内存地址寄存器递增	
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  
	//数据宽度为8位
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  
	//数据宽度为8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; 
	//工作在正常缓存模式
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; 
	//DMA通道 x拥有中优先级	
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; 
	//DMA通道x没有设置为内存到内存传输	
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  
	//根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器
	DMA_Init(DMA_CHx, &DMA_InitStructure);  
} 

/**************************************************************
*功  能：	开启一次DMA传输
*参  数：	DMA_CHx DMA通道
*返回值：	无
**************************************************************/
void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
{ 
	//关闭USART1 TX DMA1 所指示的通道
	DMA_Cmd(DMA_CHx, DISABLE );  
	//DMA通道的DMA缓存的大小	
 	DMA_SetCurrDataCounter(DMA_CHx,DMA1_MEM_LEN);
	//使能USART1 TX DMA1 所指示的通道 
 	DMA_Cmd(DMA_CHx, ENABLE);  
}	  
//										endfile



