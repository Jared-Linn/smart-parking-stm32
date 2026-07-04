/**
 * @file    cloud.h
 * @brief   云端通信模块 - ESP8266 WiFi + MQTT
 */

#ifndef __CLOUD_H__
#define __CLOUD_H__

#include <stm32f10x.h>

/* ========== API ========== */
void Cloud_Init(void);                                      /* 初始化 WiFi + MQTT */
void Cloud_ReportStatus(void);                              /* 定时上报状态 */
void Cloud_ReportEvent(const char* event, uint8_t* card_id);/* 上报事件 */
void Cloud_ProcessCommand(void);                            /* 处理云端命令 */
uint8_t Cloud_IsConnected(void);                            /* 查询连接状态 */

/* ========== 配置 ========== */
#define WIFI_SSID       "BKRC"
#define WIFI_PASSWORD   "12345678"
#define MQTT_BROKER     "192.168.1.100"   /* MQTT 服务器地址 */
#define MQTT_PORT       1883
#define DEVICE_ID       "PARK001"
#define REPORT_INTERVAL 10000   /* 上报间隔 (ms) */

#endif /* __CLOUD_H__ */
