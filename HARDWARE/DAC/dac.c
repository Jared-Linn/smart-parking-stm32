#include <stm32f10x.h>
#include "dac.h"


/**************************************************************
*功  能： 初始化DAC 
*参  数： 无
*返回值： 无
**************************************************************/
void DAC_Configure(void)
{
    
    DAC_InitTypeDef DAC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	//使能DAC时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    
    //无触发
    DAC_InitStructure.DAC_Trigger = DAC_Trigger_None;
    //无波形发生器
    DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    //屏蔽幅值设置
    DAC_InitStructure.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
    //无输出缓存
    DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Disable;
    DAC_Init(DAC_Channel_1,&DAC_InitStructure);
    
    DAC_Cmd(DAC_Channel_1,ENABLE);
    
    DAC_SetChannel1Data(DAC_Align_12b_R,0);
}

/**************************************************************
* 功  能：设置DAC输出值
*       将给定的电压值转换为DAC模块所需的数字值，从而设置DAC的输出。
*       这个转换是通过将输入的毫伏值转换为与DAC分辨率和参考电压兼容的数字值实现的。
* 参  数：value - DAC输出的电压值（mV）
* 返回值：无
**************************************************************/
void DAC_SetOutput(uint16_t value)
{
    double temp = value;
    temp /= 1000;
    temp = temp * 4096/3.3;
    DAC_SetChannel1Data(DAC_Align_12b_R,temp);
}
//										endfile

