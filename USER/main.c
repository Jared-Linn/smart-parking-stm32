/************************************************************
 * 项目名称：智能停车场管理系统 — Day 1
 * 硬件平台：移动互联开发平台 STM32 V2.0
 * 核心芯片：STM32F103VC  (Cortex-M3, 72MHz)
 *
 * === Day 1 功能 ===
 *  - LED (PD8-11):  车位占用 / 心跳 / WiFi / 报警
 *  - KEY (PE7-10):  K1=车靠近, K2=刷卡, K3=车离开, K4=复位
 *  - BEEP (PA8):    提示音
 *  - OLED (SPI):    状态显示
 *  - IWDG:          看门狗
 *  - 状态机:        7状态停车场逻辑
 *
 * === Day 2/3 逐步添加 ===
 *  USART1 CLI / 超声波 / 舵机 / RC522 / BH1750 / MLX90614
 *  WS2812 / ESP8266 WiFi / MQTT / RTC / ADC / DAC / DMA
 **************************************************************/

#include <stm32f10x.h>
#include "delay.h"
#include "sys.h"
#include "led.h"
#include "key.h"
#include "beep.h"
#include "oled_drv.h"
#include "iwdg.h"

#include "parking.h"
#include "gate.h"
#include "billing.h"
#include "cloud.h"
#include "cli.h"

/* ========== 系统初始化 ========== */
static void System_Init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    Delay_Init();        /* SysTick 延时 */
    LED_Init();          /* PD8-11 */
    Key_Init();          /* PE7-10 */
    BEEP_Init();         /* PA8 GPIO */
    OLED_Init();         /* SPI */
    IWDG_Init(4, 625);   /* 预分频64, 重载625 → 约1秒 */

    Gate_Init();         /* Day 2 */
    Billing_Init();      /* Day 2 */
    Cloud_Init();        /* Day 2 */
    CLI_Init();          /* Day 2 */

    Parking_Init();      /* 状态机初始化 */

    OLED_Clear();
    OLED_ShowString(16, 0, (uint8_t*)"Smart Parking");
    OLED_ShowString(0,  3, (uint8_t*)"Day 1: Onboard OK");
    OLED_ShowString(0,  5, (uint8_t*)"Press K1 start");
    delay_ms(1500);
}

/* ========== 主函数 ========== */
int main(void)
{
    uint8_t key_val;

    System_Init();

    while (1)
    {
        IWDG_Feed();  /* ★ 喂狗 */

        key_val = KEY_Scan();
        if (key_val) Parking_KeyHandler(key_val);

        Parking_StateMachine();
        Cloud_ReportStatus();
        Cloud_ProcessCommand();
        CLI_Process();

        LED1 = ~LED1;  /* 心跳灯 */
        g_parking.tick_ms += 10;
        delay_ms(10);
    }
}
//                                              endfile
