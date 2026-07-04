#ifndef __MQTT_HANDLE_H_
#define __MQTT_HANDLE_H_

#include "stm32f10x.h"

//连接云平台所需的参数
typedef struct _MQTT_Parameter_{
	char *client_id;     //设备名（待连接物影子名称）
	char *user_name;     //用户名（创建物影子时平台生成）
	char *password;      //密码（创建物影子时平台生成）
    char *topic_SubName; //订阅主题的名称（要收到那个主题发布的消息） 
	char *topic_PusName; //发布主题的名称（发布消息的主题）
}MQTT_Parameter;

extern char* mqtt_server;//服务器设备型的地址 
extern char* mqtt_server_port;//服务器对应端口号
extern uint8_t fan_value;
extern uint8_t esp8266_state;
extern char* platform_client_id;
extern char* platform_user_name;
extern char* platform_password;	
extern char* platform_topic_SubName;
extern char* platform_topic_PusName;

extern uint8_t mqtt_publish_state;
extern uint8_t mqtt_connect_state;
extern char json_string[];
void MQTT_Software_Init(void);
uint8_t MQTT_Date_Conversion(void);

void MQTT_Data_Receive(uint8_t dat);
void MQTT_Parameter_Init(void);
uint8_t MQTT_Connect(void);
uint8_t MQTT_Subscribe(void);
uint8_t MQTT_Publish(char *str);
uint8_t MQTT_PublishHex(char *topic, char *id, uint8_t *data, uint32_t datalen);
void MQTT_Heartbeat(void);
void Platform_Auto_Login(void);
void MQTT_Pub(uint32_t dat);
void MQTT_Receive(void);
#endif

