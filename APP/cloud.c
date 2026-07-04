/**
 * @file    cloud.c
 * @brief   云端通信实现
 * @note    USART3 (PB10=TX, PB11=RX) ↔ ESP8266
 *          使用 AT 指令集 + 简化的 MQTT 上报
 *
 *          ★ 由于完整 MQTT 协议栈较复杂，当前版本使用
 *            简化的 TCP 透传 + JSON 上报方式：
 *            1. ESP8266 连接 WiFi (AT 指令)
 *            2. 建立 TCP 连接到 MQTT Broker
 *            3. 直接 PUBLISH 主题和 JSON 数据
 *
 *            Day 3 进阶：替换为 paho MQTT 嵌入式库
 */

#include "cloud.h"
#include "esp8266.h"
#include "usart3.h"
#include "parking.h"
#include "led.h"
#include <stdio.h>
#include <string.h>

static uint8_t  wifi_ok = 0;
static uint32_t last_report = 0;

/* ========== 初始化 ========== */
void Cloud_Init(void)
{
    USART3_Init(115200);
    ESP8266_Init();

    /* 连接 WiFi */
    if (ESP8266_ConnectAP(WIFI_SSID, WIFI_PASSWORD))
    {
        wifi_ok = 1;
        LED2 = 1;  /* WiFi 状态灯亮 */

        /* 连接 MQTT Broker (TCP) */
        ESP8266_ConnectTCP(MQTT_BROKER, MQTT_PORT);
    }
    else
    {
        wifi_ok = 0;
        LED2 = 0;
    }

    g_parking.wifi_connected = wifi_ok;
}

/* ========== 定时上报状态 ========== */
void Cloud_ReportStatus(void)
{
    char json[256];
    uint32_t now;

    if (!wifi_ok) return;

    now = g_parking.tick_ms;
    if (now - last_report < REPORT_INTERVAL) return;
    last_report = now;

    /* 构造 JSON */
    sprintf(json,
        "{\"device_id\":\"%s\","
        "\"state\":%d,"
        "\"occupied\":%d,"
        "\"temp\":%.1f,"
        "\"light\":%d,"
        "\"dist\":%d}",
        DEVICE_ID,
        g_parking.state,
        (g_parking.state >= PARKING_GATE_CLOSE),
        g_parking.temperature,
        g_parking.light_lx,
        g_parking.distance_cm
    );

    /* 通过 USART3 发送到 ESP8266，ESP8266 透传到 MQTT */
    USART3_SendString(json);
}

/* ========== 上报事件 ========== */
void Cloud_ReportEvent(const char* event, uint8_t* card_id)
{
    char json[256];

    if (!wifi_ok) return;

    if (card_id)
    {
        sprintf(json,
            "{\"event\":\"%s\","
            "\"card\":\"%02X%02X%02X%02X\","
            "\"time\":%d,"
            "\"fee\":%d}",
            event,
            card_id[0], card_id[1], card_id[2], card_id[3],
            (int)g_parking.current.entry_time,
            g_parking.current.fee_yuan
        );
    }
    else
    {
        sprintf(json,
            "{\"event\":\"%s\",\"time\":%d}",
            event,
            (int)g_parking.tick_ms
        );
    }

    USART3_SendString(json);
}

/* ========== 处理云端命令 ========== */
void Cloud_ProcessCommand(void)
{
    /* 检查 USART3 接收缓冲区是否有数据 */
    /* 简化版：直接检查是否有 "open" / "close" / "report" 命令 */
    /* TODO: Day 3 完善 JSON 解析 */
}

/* ========== 查询连接状态 ========== */
uint8_t Cloud_IsConnected(void)
{
    return wifi_ok;
}
