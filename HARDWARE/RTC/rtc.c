#include "rtc.h"
#include "stm32f10x.h"
#include "delay.h"
#include "sys.h"

_calendar_obj calendar;//时钟结构体 
 
/**************************************************************
*功  能：RTC中断优先级配置
*参  数: 无
*返回值: 无
**************************************************************/
static void RTC_NVIC_Config(void)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	//RTC全局中断
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	//先占优先级1位,从优先级3位
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	//先占优先级0位,从优先级4位
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	//使能该通道中断
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	//根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
	NVIC_Init(&NVIC_InitStructure);
}



/**************************************************************
*功  能：RTC初始化
*参  数: 无
*返回值: 0 正常 其他:错误代码
*注  意：BKP->DR1用于保存是否第一次配置的设置	
**************************************************************/
uint8_t RTC_Init(void)
{
	
	uint8_t temp=0;//检查是不是第一次配置时钟
	//使能PWR和BKP外设时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	//使能后备寄存器访问	
	PWR_BackupAccessCmd(ENABLE);
	//从指定的后备寄存器中读出数据:读出了与写入的指定数据不相乎
	if (BKP_ReadBackupRegister(BKP_DR1) != 0x5050)		
		{
		//复位备份区域 
		BKP_DeInit();
		//设置外部低速晶振(LSE),使用外设低速晶振
		RCC_LSEConfig(RCC_LSE_ON);
		//检查指定的RCC标志位设置与否,等待低速晶振就绪
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET&&temp<250)	
			{
			temp++;
			Delay_ms(10);
			}
		//初始化时钟失败,晶振有问题	    
		if(temp>=250)return 1; 
		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		//使能RTC时钟  
		RCC_RTCCLKCmd(ENABLE);
		//等待最近一次对RTC寄存器的写操作完成
		RTC_WaitForLastTask();
		//等待RTC寄存器同步  
		RTC_WaitForSynchro();
		//使能RTC秒中断
		RTC_ITConfig(RTC_IT_SEC, ENABLE);
		//等待最近一次对RTC寄存器的写操作完成
		RTC_WaitForLastTask();
		// 允许配置	
		RTC_EnterConfigMode();
		//设置RTC预分频的值
		RTC_SetPrescaler(32767);
		//等待最近一次对RTC寄存器的写操作完成
		RTC_WaitForLastTask();
		//设置时间
		RTC_Set(2015,1,14,17,42,55);
		//退出配置模式
		RTC_ExitConfigMode();
		//向指定的后备寄存器中写入用户程序数据
		BKP_WriteBackupRegister(BKP_DR1, 0X5050);	
		}
		else//系统继续计时
		{
			//等待最近一次对RTC寄存器的写操作完成
			RTC_WaitForSynchro();
			//使能RTC秒中断
			RTC_ITConfig(RTC_IT_SEC, ENABLE);
			//等待最近一次对RTC寄存器的写操作完成
			RTC_WaitForLastTask();	
		}
		//RCT中断分组设置	
		RTC_NVIC_Config();
		//更新时间	
		RTC_Get();
		//ok
		return 0;

}		 				    
 
/**************************************************************
*功  能：RTC中断服务函数
*参  数: 无
*返回值: 无
**************************************************************/
void RTC_IRQHandler(void)
{		 
	//秒钟中断
	if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
	{				
		//更新时间   
		RTC_Get();							
 	}
	//闹钟中断
	if(RTC_GetITStatus(RTC_IT_ALR)!= RESET)	
	{
		//清闹钟中断	
		RTC_ClearITPendingBit(RTC_IT_ALR);
		//更新时间   
	    RTC_Get();							
  	} 
	//清闹钟中断
	RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW);
	RTC_WaitForLastTask();	  	    						 	   	 
}

/**************************************************************
*功  能：判断是否是闰年函数
*参  数: year 年份
*返回值: 0 不是  1 是
*注  意：月份   1  2  3  4  5  6  7  8  9  10 11 12
		     闰年   31 29 31 30 31 30 31 31 30 31 30 31
		     非闰年 31 28 31 30 31 30 31 31 30 31 30 31
**************************************************************/
uint8_t Is_Leap_Year(uint16_t year)
{		
	//必须能被4整除
	if(year%4==0) 
	{ 
		if(year%100==0) 
		{ 
			//如果以00结尾,还要能被400整除 
			if(year%400==0)return 1;	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}	

/**************************************************************
*功  能：时间设置
*参  数: 输入 年月日 时分秒
*返回值: 0 成功  其他:错误代码
*注  意：把输入的时钟转换为秒钟
		     以1970年1月1日为基准
		     1970~2099年为合法年份
	       月份数据表	
**************************************************************/	
//月修正数据表	 
uint8_t const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5};  
//平年的月份日期表
const uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
uint8_t RTC_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{
	uint16_t t;
	uint32_t seccount=0;
	if(syear<1970||syear>2099)return 1;
		//把所有年份的秒钟相加
	for(t=1970;t<syear;t++)
	{
		//闰年的秒钟数
		if(Is_Leap_Year(t))seccount+=31622400;
		//平年的秒钟数
		else seccount+=31536000;			  
	}
	smon-=1;
	//把前面月份的秒钟数相加
	for(t=0;t<smon;t++)	   
	{
		//月份秒钟数相加
		seccount+=(uint32_t)mon_table[t]*86400;
		//闰年2月份增加一天的秒钟数	
		if(Is_Leap_Year(syear)&&t==1)seccount+=86400;   
	}
	//把前面日期的秒钟数相加 
	seccount+=(uint32_t)(sday-1)*86400;
	//小时秒钟数
	seccount+=(uint32_t)hour*3600;
	//分钟秒钟数	
    seccount+=(uint32_t)min*60;
	//最后的秒钟加上去
	seccount+=sec;						
    //使能PWR和BKP外设时钟 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	//使能RTC和后备寄存器访问
	PWR_BackupAccessCmd(ENABLE);
	//设置RTC计数器的值
	RTC_SetCounter(seccount);	    
	//等待最近一次对RTC寄存器的写操作完成 
	RTC_WaitForLastTask();			 	
	return 0;	    
}

/**************************************************************
*功  能：初始化闹钟
*参  数: 输入 年月日 时分秒
*返回值: 0 成功  其他:错误代码
*注  意：以1970年1月1日为基准
		 1970~2099年为合法年份     
**************************************************************/	
uint8_t RTC_Alarm_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{
	uint16_t t;
	uint32_t seccount=0;
	if(syear<1970||syear>2099)return 1;
	//把所有年份的秒钟相加
	for(t=1970;t<syear;t++)	
	{
		//闰年的秒钟数
		if(Is_Leap_Year(t))seccount+=31622400;
		//平年的秒钟数
		else seccount+=31536000;			  
	}
	smon-=1;
	//把前面月份的秒钟数相加
	for(t=0;t<smon;t++)	                             
	{
		//月份秒钟数相加
		seccount+=(uint32_t)mon_table[t]*86400;
		//闰年2月份增加一天的秒钟数
		if(Is_Leap_Year(syear)&&t==1)seccount+=86400;	   
	}
	//把前面日期的秒钟数相加 
	seccount+=(uint32_t)(sday-1)*86400;
	//小时秒钟数
	seccount+=(uint32_t)hour*3600;
	//分钟秒钟数
    seccount+=(uint32_t)min*60;
	//最后的秒钟加上去 
	seccount+=sec;								    
	//设置时钟
	//使能PWR和BKP外设时钟  
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	//使能后备寄存器访问
	PWR_BackupAccessCmd(ENABLE);	  
	//上面三步是必须的!
	
	RTC_SetAlarm(seccount);
 
	//等待最近一次对RTC寄存器的写操作完成  
	RTC_WaitForLastTask();				
	
	return 0;	    
}

/**************************************************************
*功  能：得到当前的时间
*参  数: 无
*返回值: 0 成功  其他:错误代码 
**************************************************************/
uint8_t RTC_Get(void)
{
	static uint16_t daycnt=0;
	uint32_t timecount=0; 
	uint32_t temp=0;
	uint16_t temp1=0;	  
    timecount=RTC_GetCounter();
	//得到天数(秒钟数对应的)
 	temp=timecount/86400;
	//超过一天了
	if(daycnt!=temp)        
	{	  
		daycnt=temp;
		//从1970年开始
		temp1=1970;			
		while(temp>=365)
		{		
			//是闰年
			if(Is_Leap_Year(temp1))  
			{
				//闰年的秒钟数
				if(temp>=366)temp-=366;
				else {temp1++;break;}  
			}
			//平年 
			else temp-=365;			
			temp1++;  
		}  
		//得到年份
		calendar.w_year=temp1;     
		temp1=0;
		//超过了一个月
		while(temp>=28)            
		{
			//当年是不是闰年/2月份
			if(Is_Leap_Year(calendar.w_year)&&temp1==1)
			{
				//闰年的秒钟数
				if(temp>=29)temp-=29;
				else break; 
			}
			else 
			{
				//平年
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];
				else break;
			}
			temp1++;  
		}
		//得到月份
		calendar.w_month=temp1+1;
		//得到日期 
		calendar.w_date=temp+1;  	
	}
	//得到秒钟数   
	temp=timecount%86400;
	//小时
	calendar.hour=temp/3600;
	//分钟	
	calendar.min=(temp%3600)/60;
	//秒钟
	calendar.sec=(temp%3600)%60;
	//获取星期 
	calendar.week=RTC_Get_Week(calendar.w_year,calendar.w_month,calendar.w_date);  
	return 0;
}	 

/**************************************************************
*功  能：输入公历日期得到星期(只允许1901-2099年)
*参  数: 年月日
*返回值: 星期号 
**************************************************************/																					 
uint8_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day)
{	
	uint16_t temp2;
	uint8_t yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 
	// 如果为21世纪,年份数加100  
	if (yearH>19)yearL+=100;
	// 所过闰年数只算1900年之后的  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
}	
//										endfile
