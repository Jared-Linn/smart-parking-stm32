#include "stm32f10x.h"
#include "mqtt_handle.h"
#include "MQTTPacket.h"
#include "esp8266.h"
#include "string.h"
#include "stdio.h"
#include "usart3.h"
#include "string.h"
#include "timer.h"
#include "Delay.h"
#include "cjson.h"
#include "stdlib.h"
#include "WS2812.h"
#include "oled.h"

MQTT_Parameter  BDTG_Device;

#define BUFMAX  400
uint8_t mqtt_buf[200] = {0};
uint8_t mqtt_data[200] = {0};  //用于存储数据

uint8_t esp8266_state = 0;
char* mqtt_server = 	"115.28.209.116";		//服务器设备型的地址
char* mqtt_server_port = 	"1883";					//服务器对应端口号

char* platform_client_id  = "1de852406fc3ae96ff6d75aa1e486b72";								//客户端ID
char* platform_user_name = "bkrc";							//MQTT账号
char* platform_password = "88888888";						//MQTT密码
char* platform_topic_SubName = "device/312d26f8412c8a5f/down";		//订阅主题
char* platform_topic_PusName = "device/312d26f8412c8a5f/up";			//发布主题



char json_string[0x300]	=	{0};									//提取出的json格式字符串
uint8_t mqtt_publish_state = 0;
uint8_t mqtt_connect_state = 0;
static char idat[128] = {0};

void MQTT_Software_Init(void)										//软件初始化
{
    MQTT_Parameter_Init();
	if(esp8266_state)
	{
		OLED_Show_line(2);	//设置进度条
		OLED_Show_Str(8,6,"云平台连接成功",16);
	}
	else
	{
		OLED_Show_Str(16,6,"WiFi连接失败",16);
	}
	sprintf(idat,"CIPSTART=\"TCP\",\"%s\",%s",mqtt_server,mqtt_server_port);	
	ESP8266_SCOM(idat,1);											//连接网站
	ESP8266_SCOM("CIPMODE=1",1);									//设置透传模式
	ESP8266_SCOM("CIPSEND",1);										//进入透传模式
	while(MQTT_Connect());											//连接服务器
	if(MQTT_Connect())
	{
		Delay_ms(500);
		OLED_Show_Str(8,6,"云平台连接失败",16);
	}
	else	
	{
		OLED_Show_line(3);	//设置进度条
		OLED_Show_Str(8,6,"云平台连接成功",16);
	}
	Delay_ms(500);
	Delay_ms(500);
	OLED_Show_Str(8,6,"订阅云平台服务",16);	
	OLED_Show_line(4);	//设置进度条	
	Delay_ms(500);
	Delay_ms(500);
	while(MQTT_Subscribe());			//订阅频道
	Delay_ms(500);	
	Delay_ms(500);					
	if(MQTT_Subscribe())
	{
		OLED_Show_Str(8,6,"订阅云平台失败",16);
	}
	else	
	{
		OLED_Show_Str(8,6,"订阅云平台成功",16);
		OLED_Show_line(5);	//设置进度条
		Delay_ms(500);
		Delay_ms(500);
		TIM_Cmd(TIM3, ENABLE);											//开启定时器3，允许发布
		OLED_Clear();
	}
}

uint8_t MQTT_Date_Conversion(void)									//将订阅接收数组中的数据提取入json_string[]数组,失败返回0,成功返回1
{
    uint16_t k_length = usart3_length, c_length = 0, rk_state = 0, rk_length = 0, s_length = 0;

    for(c_length = 0; c_length < k_length; c_length++)
    {
        if(usart3_buffer[c_length] == '{')
        {
            rk_state = 1;
            rk_length++;
        }
        else if(usart3_buffer[c_length] == '}')
        {
            rk_length--;
        }

        if(rk_state)
        {
            if(rk_length)											//{}内有效数据
            {
                json_string[s_length++] = usart3_buffer[c_length];
            }
            else
            {
                json_string[s_length++] = '}';
                json_string[s_length++] = '\0';
                rk_state = 2;
                break;
            }
        }
    }

    if(rk_state == 2)												//完成转换
    {
        return 1;
    }

    return 0;														//转换失败
}

/*
功能：MQTT参数初始化
参数：device 代初始化的设备
*/
void MQTT_Parameter_Init(void)
{
    MQTT_Parameter *mqtt_device;

    mqtt_device = &BDTG_Device;
    mqtt_device->client_id = (char *)platform_client_id;
    mqtt_device->user_name = (char *)platform_user_name;
    mqtt_device->password = (char *)platform_password;
    mqtt_device->topic_SubName = (char *)platform_topic_SubName;
    mqtt_device->topic_PusName = (char *)platform_topic_PusName;
}


/*
功能：MQTT数据发送
参数：*str 数据   len  长度
说明：封装串口发送函数
*/
void MQTT_Data_Send(uint8_t *str, uint16_t len)
{
    USART3_Send_Length(str, len);
}


/*
功能：检查MQTT数据反馈标志
说明：通过串口消息队列中断数据判断
*/
uint8_t MQTT_Check_Ack(uint8_t mode_ack)
{
    uint8_t ack_flag = 0;

    if(usart3_length)
    {
        Delay_ms(10);

        switch(mode_ack)      //选择反馈数据类型
        {
            case CONNACK:
                if(usart3_buffer[1] == 0x02 || usart3_buffer[2] == 0x00 || usart3_buffer[3] == 0x00) //0x00 表示连接成功
                {
                    ack_flag = CONNACK;
                }

                break;

            case PUBACK:
                ack_flag = PUBACK;
                break;

            case SUBACK:
                if(usart3_buffer[1] == 0x03 || usart3_buffer[2] == 0x00 || usart3_buffer[3] == 0x01) //0x01 表示订阅成功
                {
                    ack_flag = SUBACK;
                }

                break;

            case UNSUBACK:

                break;

            default:
                break;
        }
    }

    return ack_flag;
}


/*
功能：连接服务器
参数：device 待连接的设备
*/
char connect_buf[150];
uint16_t connect_len = 0;
uint8_t MQTT_Connect(void)
{
    uint8_t wait_time = 5;
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    MQTT_Parameter *mqtt_parameter;     //MQTT参数指针
    int buflen = sizeof(connect_buf);

    MQTT_Parameter_Init();
    mqtt_parameter = &BDTG_Device;
    data.clientID.cstring = mqtt_parameter->client_id;	//客户端标识符 ID payload
    data.keepAliveInterval = 60;					  	//keep Alive 单位s
    data.cleansession = 1;							  	//清理会话标志置位
    data.username.cstring = mqtt_parameter->user_name;	//用户名
    data.password.cstring = mqtt_parameter->password;	//用户密码
    data.MQTTVersion = 4;
    connect_len = MQTTSerialize_connect((uint8_t *)connect_buf, buflen, &data);

    do
    {
        usart3_length = 0;
        MQTT_Data_Send((uint8_t *)connect_buf, connect_len);   //MQTT数据发送
        Delay_ms(2000);
        wait_time--;

        if(!wait_time)
        {
            return 1;		//超时退出
        }
    }
    while(MQTT_Check_Ack(CONNACK) != CONNACK);

    return 0;
}


/*
功能：消息订阅
参数：*topic   订阅的主题
返回值：0订阅成功  1订阅失败
*/
unsigned char sub_buf[300];
uint8_t MQTT_Subscribe(void)
{
    uint8_t wait_time = 10;
    int buflen = sizeof(sub_buf);
    int msgid = 1;
    int req_qos = 0;
    uint16_t len;
    MQTTString topicString = MQTTString_initializer;
    MQTT_Parameter *mqtt_parameter;   //MQTT参数指针

    MQTT_Parameter_Init();
    mqtt_parameter = &BDTG_Device;
    topicString.cstring = mqtt_parameter->topic_SubName;
    len = MQTTSerialize_subscribe(sub_buf, buflen, 0, msgid, 1, &topicString, &req_qos);

    do
    {
        usart3_length = 0;
        MQTT_Data_Send((uint8_t *)sub_buf, len);   //MQTT数据发送
        Delay_ms(1000);
        wait_time--;

        if(!wait_time)
        {
            return 1;		//超时退出
        }
    }
    while(MQTT_Check_Ack(SUBACK) != SUBACK);

    return 0;
}



/*
功能：消息发布
参数：*topic 发布的主题
       *str  发布的内容
返回值：0发布失败 1发布成功
*/
unsigned char buf[0x800];
uint8_t MQTT_Publish(char *str)
{
    int buflen = sizeof(buf);
    uint16_t payloadlen, len;
    uint8_t *payload = (uint8_t *)str;
    MQTTString topicString = MQTTString_initializer;
    payloadlen = strlen(str);
    MQTT_Parameter *mqtt_parameter;   //MQTT参数指针

    mqtt_parameter = &BDTG_Device;

    topicString.cstring = mqtt_parameter->topic_PusName;

    len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, (uint8_t *)payload, payloadlen);

    MQTT_Data_Send(buf, len);

    return 1;
}


/*
功能：以字节类型上传数据到OneNET (只适合OneNET平台，其他平台请使用MQTT_Publish（）)
参数：*topic 发布的主题
       *id   id 格式：char * id = "{\"ds_id\":\"data\"}
       *data  数据；
       datalen：数据的长度
返回值：无
*/
uint8_t MQTT_PublishHex(char *topic, char *id, uint8_t *data, uint32_t datalen)
{

    uint32_t link = 0;			//数据拼接标识
    uint32_t linkdata = 0;
    uint32_t idLength = 0;		//数据id长度


    uint8_t payloadBuf[BUFMAX - 3] = {0};	//存放拼接后的数组,因为OneNET需要添加三位标识所以减去3

    uint8_t buf[BUFMAX];					//存放序列化后的数据

    uint32_t buflen = sizeof(buf);	    //序列化数组长度

    uint32_t payloadlen, len;				//有效负载的长度，MQTT包的总长度

    uint8_t *payload;					//有效负载数据

    MQTTString topicString = MQTTString_initializer;

    idLength = strlen(id);

    payloadlen = idLength + datalen + 3 + 4;

    payloadBuf[0] = 0x02;				//上传类型2 二进制格式
    payloadBuf[1] = (idLength >> 8) & 0xff; //上传数据长度  高八位
    payloadBuf[2] = idLength & 0xff;	    //上传数据长度  低八位

    for(; link < idLength; link++)
    {
        payloadBuf[3 + link] = id[link];	//拼接字符
    }

    payloadBuf[3 + link] = (datalen >> 24) & 0xff;

    payloadBuf[4 + link] = (datalen >> 16) & 0xff;

    payloadBuf[5 + link] = (datalen >> 8) & 0xff;

    payloadBuf[6 + link] = datalen & 0xff;


    for(; linkdata < idLength; linkdata++)
    {
        payloadBuf[7 + link + linkdata] = data[linkdata]; //拼接字符
    }

    payload = (uint8_t *)payloadBuf;
    topicString.cstring = topic;		  //上传的主题

    len = MQTTSerialize_publish(buf, buflen, 0, 0, 0, 0, topicString, payload, payloadlen); //序列化数据。返回数据包的总长度

    MQTT_Data_Send(buf, len);

    return 1;
}

void MQTT_Heartbeat(void)
{
    uint8_t buf[] = {0xC0, 0x00};

    if(mqtt_connect_state)
    {
        MQTT_Data_Send(buf, 2);
        mqtt_connect_state = 0;
    }
}

void Platform_Auto_Login(void)
{
    ESP8266_Software_Init();
    MQTT_Software_Init();
}

static cJSON  *bkrc_id, *data, *testmodel, *message,*bkrc_idReceive,*messdata;;
char *mess_str;
static char *out;
static char sdat[0x600] = { 0 };

void MQTT_Pub(uint32_t dat)
{
	static uint32_t Last_Val = 0;
    if(mqtt_publish_state)
    {
        /*
        	{
        			"sign": "f3774a48f0c2a061",
        			"type": 1,
        			"data": {
        				"模块标识": {
        					"模型标识": 23,
        					"模型标识": 83"
        				}
        			}
        		}
        */
        bkrc_id = cJSON_CreateObject();
        cJSON_AddStringToObject(bkrc_id, "sign", "312d26f8412c8a5f");
        cJSON_AddStringToObject(bkrc_id, "type", "1");

        cJSON_AddItemToObject(bkrc_id, "data", data = cJSON_CreateObject());
        cJSON_AddItemToObject(data, "Am_Light", testmodel = cJSON_CreateObject());

		cJSON_AddNumberToObject(testmodel, "RLED", ((dat >> 16) & 0xff));
		cJSON_AddNumberToObject(testmodel, "GLED", ((dat >> 8) & 0xff));
		cJSON_AddNumberToObject(testmodel, "BLED", dat & 0xff);
		Last_Val=dat;
        out = cJSON_Print(bkrc_id);									//转化为字符串
        sprintf(sdat, "%s", out);									//提取到静态内存
        cJSON_Delete(bkrc_id);										//释放
        free(out);													//释放
        MQTT_Publish(sdat);											//将提取内容发布
        mqtt_publish_state = 0;
    }
	else
	if(Last_Val!=dat){
		bkrc_id = cJSON_CreateObject();
        cJSON_AddStringToObject(bkrc_id, "sign", "312d26f8412c8a5f");
        cJSON_AddStringToObject(bkrc_id, "type", "1");

        cJSON_AddItemToObject(bkrc_id, "data", data = cJSON_CreateObject());
        cJSON_AddItemToObject(data, "Am_Light", testmodel = cJSON_CreateObject());

		cJSON_AddNumberToObject(testmodel, "RLED", ((dat >> 16) & 0xff));
		cJSON_AddNumberToObject(testmodel, "GLED", ((dat >> 8) & 0xff));
		cJSON_AddNumberToObject(testmodel, "BLED", dat & 0xff);
		Last_Val=dat;

        out = cJSON_Print(bkrc_id);									//转化为字符串
        sprintf(sdat, "%s", out);									//提取到静态内存
        cJSON_Delete(bkrc_id);										//释放
        free(out);													//释放
        MQTT_Publish(sdat);											//将提取内容发布
	}
}
void MQTT_Receive(void)
{
	char *str_1 = "msg";
	char *str_2;
    uint8_t RLED_val = 0, GLED_val = 0, BLED_val = 0;

	if (usart3_length)//判断串口数据接收
	{
		Delay_ms(50);
		if (MQTT_Date_Conversion())                        //提取有效数据,无效则直接退出
		{
			bkrc_idReceive = cJSON_Parse(json_string);            //提取数据中的特定格式，去掉数据的头

			if (bkrc_idReceive != NULL)
			{
				/******有效的Json数据，从中提取需要的RGB的值

				"{
				"message": "{
				"RLED ":255 ,
				"GLED ":255 ,
				"BLED ":255 
				
				}",
				"type": "msg"
				}"
				*/
				message      = cJSON_GetObjectItem(bkrc_idReceive, "message");
				str_2 = cJSON_GetObjectItem(bkrc_idReceive, "type")->valuestring;
				if(strcmp(str_1, str_2) == 0)           //判断是否是正常的下发数据，而不是平台错误返回的数据
				{
					mess_str = message->valuestring;      //提取字符串
					messdata = cJSON_Parse(mess_str);      //提取数据中的特定格式，去掉数据的头
					if(messdata != NULL)
					{
						cJSON *RLED_item = cJSON_GetObjectItem(messdata, "RLED");
						cJSON *GLED_item = cJSON_GetObjectItem(messdata, "GLED");
						cJSON *BLED_item = cJSON_GetObjectItem(messdata, "BLED");

						if (RLED_item != NULL && cJSON_IsNumber(RLED_item))
						{
							RLED_val = RLED_item->valueint;
						}
						if (GLED_item != NULL && cJSON_IsNumber(GLED_item))
						{
							GLED_val = GLED_item->valueint;
						}
						if (BLED_item != NULL && cJSON_IsNumber(BLED_item))
						{
							BLED_val = BLED_item->valueint;
						}
						ws2812_RGB = RLED_val<<16|GLED_val<<8|BLED_val;
						// 调用相应函数处理 RGB 值
						WS2812_Handle(ws2812_RGB);
					}
				}
				cJSON_Delete(bkrc_idReceive); //提取数据完成清空内存
			}

		}
		usart3_length = 0;
	}
}
