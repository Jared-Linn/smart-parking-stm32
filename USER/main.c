/************************************************************
 * 项目名称：智能停车场管理系统
 * 硬件平台：移动互联开发平台（STM32）V2.0
 * 核心芯片：STM32F103RCT6  (Cortex-M3, 72MHz)
 * 外设晶振：8MHz HSE
 *
 * === 功能清单 ===
 *  1.  HC-SR04 超声波   — 车辆进出检测
 *  2.  MFRC522 RFID     — 刷卡身份识别 (SPI)
 *  3.  舵机 (TIM2 CH1)   — 道闸抬杆/落杆
 *  4.  OLED 0.96"        — 信息显示 (SPI)
 *  5.  BH1750            — 光照度检测 → 自动照明
 *  6.  MLX90614          — 红外测温 → 高温报警
 *  7.  WS2812 RGB        — 车位引导灯
 *  8.  蜂鸣器 (TIM1 CH1)  — 提示音/报警
 *  9.  ESP8266 WiFi      — MQTT 云端上报 (USART3)
 *  10. RTC               — 停车计时
 *  11. IWDG              — 看门狗 (1.5s)
 *  12. ADC1 CH16         — 内部温度监测
 *  13. DAC CH1 (PA4)     — 模拟传感器输出
 *  14. DMA1 CH4          — USART1 TX DMA 传输
 *  15. 串口 CLI (USART1)  — PC 端命令控制
 *  16. 4×LED + 4×KEY     — 状态指示 + 本地控制
 *
 * === 引脚分配 ===
 *  PA0  : TIM2_CH1 (舵机 PWM 50Hz)
 *  PA2  : 超声波 TRIG / BH1750 SCL (分时复用)
 *  PA3  : 超声波 ECHO / BH1750 SDA (分时复用)
 *  PA4  : RC522 MISO / DAC CH1
 *  PA5  : RC522 MOSI
 *  PA6  : RC522 SCK
 *  PA7  : RC522 NSS
 *  PA8  : TIM1_CH1 (蜂鸣器 PWM)
 *  PA9  : USART1 TX
 *  PA10 : USART1 RX
 *  PB8  : OLED D1 (MOSI)
 *  PB9  : OLED D0 (SCK)
 *  PB10 : USART3 TX (ESP8266)
 *  PB11 : USART3 RX (ESP8266)
 *  PD8  : LED0 (车位占用)
 *  PD9  : LED1 (系统心跳)
 *  PD10 : LED2 (WiFi 状态)
 *  PD11 : LED3 (报警指示)
 *  PE1  : OLED DC
 *  PE2  : OLED RST
 *  PE3  : OLED CS
 *  PE7  : KEY1 (进场)
 *  PE8  : KEY2 (出场)
 *  PE9  : KEY3 (查询)
 *  PE10 : KEY4 (管理)
 *
 * 编写时间：2026.07.04
 **************************************************************/

#include <stm32f10x.h>
#include "delay.h"
#include "sys.h"
#include "led.h"
#include "key.h"
#include "beep.h"
#include "oled_drv.h"
#include "iwdg.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"

#include "parking.h"
#include "gate.h"
#include "billing.h"
#include "cloud.h"
#include "cli.h"

/* ========== 系统初始化 ========== */
static void System_Init(void)
{
    /* ----- 第1层：内核配置 ----- */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  /* 2位抢占, 2位响应 */
    Delay_Init();        /* SysTick → delay_us / delay_ms */

    /* ----- 第2层：在板外设 ----- */
    LED_Init();          /* PD8-11 */
    Key_Init();          /* PE7-10 */
    BEEP_Init();         /* PA8 TIM1_CH1 */
    OLED_Init();         /* SPI: PE1-3, PB8-9 */
    IWDG_Init();         /* 预分频64, 重载625 → 1秒超时 */
    ADC1_Init();         /* PA0 内部温度传感器通道 */
    DAC1_Init();         /* PA4 输出 */
    DMA_Init();          /* DMA1 CH4 → USART1 TX */

    /* ----- 第3层：通信接口 ----- */
    CLI_Init();          /* USART1 115200 (串口助手 CLI) */
    /* Cloud_Init() 在 Parking_Init() 中调用 */

    /* ----- 第4层：应用初始化 ----- */
    Parking_Init();      /* 停车场系统 (包含 Gate + Billing + Cloud) */

    /* 启动完成 */
    OLED_Clear();
    OLED_ShowString(16, 0, "Smart Parking");
    OLED_ShowString(8, 2,  "System Ready!");
    OLED_ShowString(0, 4,  "STM32F103 V1.0");
    OLED_ShowString(0, 6,  "2026-07-04");
    BEEP_ShortBeep();
    delay_ms(1500);
}

/* ========== 主函数 ========== */
int main(void)
{
    uint8_t key_val;

    /* 初始化 */
    System_Init();

    /* ====== 主循环 ====== */
    while (1)
    {
        /* ★ 喂狗 (1.5秒内必须喂，否则复位) */
        IWDG_Feed();

        /* 扫描按键 (10ms 去抖) */
        key_val = KEY_Scan();
        if (key_val)
        {
            Parking_KeyHandler(key_val);
        }

        /* 停车场状态机 (核心) */
        Parking_StateMachine();

        /* 云端定时上报 */
        Cloud_ReportStatus();

        /* 云端命令处理 */
        Cloud_ProcessCommand();

        /* 串口 CLI 命令处理 */
        CLI_Process();

        /* 智能照明：光照不足自动开灯 */
        if (g_parking.light_lx < LIGHT_THRESHOLD_LOW &&
            g_parking.light_lx > 0)
        {
            WS2812_SetColor(255, 255, 255);  /* 白光照明 */
        }

        /* 系统心跳 */
        LED1 = ~LED1;

        /* 系统运行计数 */
        g_parking.tick_ms += 10;
        delay_ms(10);
    }
}

//                                              endfile
