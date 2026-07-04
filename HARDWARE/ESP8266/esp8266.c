#include "stm32f10x.h"
#include "esp8266.h"
#include "string.h"
#include "stdio.h"
#include "usart3.h"
#include "delay.h"
#include "mqtt_handle.h"
#include "oled.h"

char* ssid = "BKRC_TEST";		//连接的路由器（热点）的名字
char* password = "12345678";	//连接的路由器（热点）的密码

uint8_t ESP8266_CheckOK(uint16_t wait_time)
{
	while(wait_time--)
	{
		if(usart3_length)
		{
			Delay_ms(5);
			if(usart3_length>3)
			{
				if((usart3_buffer[usart3_length-4] == 'O') && (usart3_buffer[usart3_length-3]=='K')\
					 && (usart3_buffer[usart3_length-2] == 0x0d) && (usart3_buffer[usart3_length-1]==0x0a))		
				{
					return 0;				//ok
				}
			}
			if(usart3_length>6)
			{
				if((usart3_buffer[usart3_length-7] == 'O') && (usart3_buffer[usart3_length-6]=='K')\
					 && (usart3_buffer[usart3_length-5] == 0x0d) && (usart3_buffer[usart3_length-1]==0x3e))		
				{
					return 0;				//ok
				}
			}
			if(usart3_length>3)
			{
				if((usart3_buffer[usart3_length-4] == 'O') && (usart3_buffer[usart3_length-3]=='R')\
					 && (usart3_buffer[usart3_length-2] == 0x0d) && (usart3_buffer[usart3_length-1]==0x0a))
				{
					if( (usart3_buffer[0] == 'A') && (usart3_buffer[1] == 'L') && (usart3_buffer[2] == 'R') \
						&& (usart3_buffer[3] == 'E') && (usart3_buffer[4] == 'A') && (usart3_buffer[5] == 'D') )
					{
						return 0; 			//ALREADY_OK
					}
					return 1;				//error
				}	
			}
			usart3_length = 0;			
		}
		Delay_ms(10);
	}
	return 2;							//other,overtime
}

static char edat[128]={0};
uint8_t ESP8266_SCOM(char *sdat,uint8_t mode)
{
	char st[4]={0x41,0x54,0x2B};		//AT+
	char et[3]={0x0D,0x0A};				//回车换行
	while(1)
	{
		usart3_length = 0;
		sprintf(edat,"%s%s%s",st,sdat,et);
		USART3_Send_Length((uint8_t *)edat,strlen(edat));	
		if(!ESP8266_CheckOK(1000))		//等待接收成功，break退出返回1
		{
			break;
		}
		else
		{
			if(!mode)					//失败返回0
			{
				return 0;
			}
		}
	}
	return 1;
}

static char sdat[64]={0};

void ESP8266_Software_Init(void)
{
	uint8_t connect_state = 0;
	uint8_t outmode[3] = {"+++"};
	uint8_t edat [6] = { 0x41,0x54,0x45,0x30,0x0D,0x0A };
	OLED_Show_line(0);
	while(1)
	{	
		if(!usart3_length)							
		{
			USART3_Send_Length(outmode,3);									//退出ESP8266可能已经进入的透传模式(对其他都不影响)
			Delay_ms(100);
			USART3_Send_Length(edat,6);
			Delay_ms(100);
		}
		else
		{
			Delay_ms(10);
			if(usart3_length>4)
			{
				Delay_ms(5);
				if((usart3_buffer[2] == 0x4f) && (usart3_buffer[3]==0x4b)\
				 && (usart3_buffer[4] == 0x0d) && (usart3_buffer[5]==0x0a))		//判断ESP8266有效格式
				{
					usart3_length = 0;
					break;
				}
			}
			usart3_length = 0;
		}
	}
	ESP8266_SCOM("CWMODE=1",1);								//作为从机（连路由器或热点）
	sprintf(sdat,"CWJAP_DEF=\"%s\",\"%s\"",ssid,password);	//合成字符
	connect_state = ESP8266_SCOM(sdat,0);
	if(connect_state)
	{
		OLED_Show_line(1);	//设置进度条
        OLED_Show_Str(16,6,"WiFi连接成功",16);
		esp8266_state = 1;
		Delay_ms(500);
	}
	else
	{
		OLED_Show_Str(16,6,"WiFi连接失败",16);
		ESP8266_Software_Init();
	}
}






