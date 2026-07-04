/**
 * @file    parking.c
 * @brief   停车场核心状态机实现
 * @note    主循环每 10ms 调用 Parking_StateMachine()
 */

#include "parking.h"
#include "gate.h"
#include "billing.h"
#include "cloud.h"
#include "cli.h"
#include "led.h"
#include "key.h"
#include "beep.h"
#include "oled_drv.h"
#include "bh1750.h"
#include "mlx90614.h"
#include "UltrasonicWave.h"
#include "RC522.h"
#include "WS2812.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>

/* ========== 全局状态 ========== */
ParkingSystem g_parking;

/* ========== 状态名称表 ========== */
static const char* state_names[] = {
    "IDLE",
    "WAIT_CARD",
    "GATE_OPEN",
    "GATE_CLOSE",
    "PARKED",
    "EXIT_WAIT",
    "EXIT_OPEN",
    "ALARM"
};

const char* Parking_StateName(ParkingState s)
{
    if (s <= PARKING_ALARM)
        return state_names[s];
    return "UNKNOWN";
}

/* ========== 初始化 ========== */
void Parking_Init(void)
{
    memset(&g_parking, 0, sizeof(ParkingSystem));
    g_parking.state = PARKING_IDLE;

    /* 显示启动画面 */
    OLED_Clear();
    OLED_ShowString(0, 0, "Smart Parking");
    OLED_ShowString(16, 2, "System Init...");
    delay_ms(500);

    /* 初始化各个子系统 */
    Gate_Init();        /* 超声波 + 舵机 */
    Billing_Init();     /* RTC + 计费 */
    Cloud_Init();       /* ESP8266 + MQTT */

    /* 初始状态指示 */
    LED1 = 0;   /* 心跳灯灭 */
    LED2 = 0;   /* WiFi 待连接 */
    LED3 = 0;   /* 报警灯灭 */
    LED4 = 0;   /* 车位空闲 */

    WS2812_SetColor(0, 255, 0);  /* 绿灯：车位空闲 */
    BEEP_ShortBeep();

    g_parking.tick_ms = 0;
}

/* ========== 传感器数据更新 ========== */
void Parking_SensorUpdate(void)
{
    /* 超声波测距 */
    g_parking.distance_cm = Ultrasonic_GetDistance();
    g_parking.car_present_prev = g_parking.car_present;
    g_parking.car_present = (g_parking.distance_cm > 0 &&
                             g_parking.distance_cm < CAR_DISTANCE_THRESHOLD);

    /* 光照度 (仅在连接 BH1750 时有效) */
    g_parking.light_lx = BH1750_ReadLight();

    /* 环境温度 (仅在连接 MLX90614 时有效) */
    g_parking.temperature = MLX90614_ReadTemp();

    /* RFID 刷卡检测 */
    if (RC522_CheckCard())
    {
        if (RC522_ReadCardID(g_parking.card_uid))
        {
            g_parking.card_detected = 1;
            BEEP_ShortBeep();  /* 刷卡提示音 */
        }
    }

    /* 报警条件检测 */
    if (g_parking.temperature > TEMP_ALARM_HIGH)
    {
        g_parking.alarm_flag = 1;
    }
}

/* ========== 状态机主循环 ========== */
void Parking_StateMachine(void)
{
    /* 先更新传感器 */
    Parking_SensorUpdate();

    switch (g_parking.state)
    {
    /* ---- 空闲：等待车辆靠近 ---- */
    case PARKING_IDLE:
        LED4 = 0;  /* 车位空闲 */
        WS2812_SetColor(0, 255, 0);  /* 绿灯 */

        if (g_parking.car_present && !g_parking.car_present_prev)
        {
            /* 检测到车辆靠近 */
            g_parking.state = PARKING_WAIT_CARD;
            g_parking.card_detected = 0;
        }
        break;

    /* ---- 等待刷卡 ---- */
    case PARKING_WAIT_CARD:
        if (g_parking.card_detected)
        {
            /* 刷卡成功 → 判断是进场还是出场 */
            if (g_parking.state == PARKING_WAIT_CARD)
            {
                /* 进场：记录卡号、入场时间 */
                memcpy(g_parking.current.card_id, g_parking.card_uid, 4);
                g_parking.current.entry_time = Billing_GetTime();
                g_parking.current.valid = 1;

                /* 抬杆 */
                g_parking.state = PARKING_GATE_OPEN;
                Gate_Open();
                WS2812_SetColor(255, 255, 0);  /* 黄灯：通行中 */
            }
        }

        /* 超时：车走了（30秒未刷卡） */
        if (!g_parking.car_present)
        {
            g_parking.state = PARKING_IDLE;
            g_parking.card_detected = 0;
        }
        break;

    /* ---- 道闸打开中 ---- */
    case PARKING_GATE_OPEN:
        LED4 = 1;  /* 车位即将占用 */

        if (!g_parking.car_present && g_parking.car_present_prev)
        {
            /* 车辆已通过 → 关闭道闸 */
            g_parking.state = PARKING_GATE_CLOSE;
            Gate_Close();
        }
        break;

    /* ---- 道闸关闭中 ---- */
    case PARKING_GATE_CLOSE:
        if (Gate_IsClosed())
        {
            /* 道闸已关闭 → 进入停车状态 */
            g_parking.state = PARKING_PARKED;
            LED4 = 1;   /* 车位占用 */
            WS2812_SetColor(255, 0, 0);  /* 红灯：已占用 */

            /* 云端上报：车辆入场 */
            Cloud_ReportEvent("entry", g_parking.current.card_id);
        }
        break;

    /* ---- 已停车，等待出场 ---- */
    case PARKING_PARKED:
        /* 检测到车辆 + 刷卡 → 可能是出场 */
        if (g_parking.card_detected)
        {
            /* 判断是否同一张卡 */
            if (memcmp(g_parking.card_uid, g_parking.current.card_id, 4) == 0)
            {
                g_parking.state = PARKING_EXIT_WAIT;
            }
            else
            {
                /* 非本车卡，可能是管理员或其他车辆 */
                BEEP_Beep(200);  /* 长提示音 */
            }
            g_parking.card_detected = 0;
        }
        break;

    /* ---- 等待出场刷卡 ---- */
    case PARKING_EXIT_WAIT:
        /* 计算费用 */
        g_parking.current.exit_time = Billing_GetTime();
        g_parking.current.fee_yuan = Billing_CalcFee(
            g_parking.current.entry_time,
            g_parking.current.exit_time
        );

        /* 抬杆放行 */
        g_parking.state = PARKING_EXIT_OPEN;
        Gate_Open();
        WS2812_SetColor(255, 255, 0);  /* 黄灯：通行中 */
        break;

    /* ---- 出场抬杆 ---- */
    case PARKING_EXIT_OPEN:
        if (!g_parking.car_present && g_parking.car_present_prev)
        {
            /* 车辆已离开 → 关闭道闸 */
            Gate_Close();
            g_parking.state = PARKING_GATE_CLOSE;

            /* 云端上报：车辆出场 */
            Cloud_ReportEvent("exit", g_parking.current.card_id);

            /* 复位为空闲 */
            g_parking.state = PARKING_IDLE;
            memset(&g_parking.current, 0, sizeof(ParkingRecord));
            LED4 = 0;   /* 车位空闲 */
            WS2812_SetColor(0, 255, 0);  /* 绿灯 */
        }
        break;

    /* ---- 异常报警 ---- */
    case PARKING_ALARM:
        LED3 = ~LED3;  /* 报警灯闪烁 */
        BEEP_Beep(1000);  /* 长鸣报警 */
        Cloud_ReportEvent("alarm", NULL);

        /* 报警恢复：车辆离开且道闸正常 */
        if (!g_parking.car_present && Gate_IsClosed())
        {
            g_parking.alarm_flag = 0;
            g_parking.state = PARKING_IDLE;
            LED3 = 0;
        }
        break;

    default:
        g_parking.state = PARKING_IDLE;
        break;
    }

    /* 自动报警检测 (独立于状态机) */
    if (g_parking.alarm_flag && g_parking.state != PARKING_ALARM)
    {
        g_parking.state = PARKING_ALARM;
    }

    /* 更新显示 */
    Parking_DisplayUpdate();

    /* 清除单次事件标志 */
    g_parking.card_detected = 0;
}

/* ========== OLED 显示更新 ========== */
void Parking_DisplayUpdate(void)
{
    static uint8_t page = 0;
    char buf[21];

    /* 每 500ms 切换显示页面 */
    if (g_parking.tick_ms % 500 == 0)
    {
        page = (page + 1) % 3;
    }

    OLED_Clear();

    switch (page)
    {
    case 0:  /* 主页：状态 + 时间 */
        sprintf(buf, "Status: %s", Parking_StateName(g_parking.state));
        OLED_ShowString(0, 0, (uint8_t*)buf);

        sprintf(buf, "Dist: %d cm", g_parking.distance_cm);
        OLED_ShowString(0, 2, (uint8_t*)buf);

        sprintf(buf, "Time: %02d:%02d:%02d",
                Billing_GetHour(), Billing_GetMinute(), Billing_GetSecond());
        OLED_ShowString(0, 4, (uint8_t*)buf);

        if (g_parking.state == PARKING_PARKED)
        {
            uint32_t elapsed = Billing_GetTime() - g_parking.current.entry_time;
            sprintf(buf, "Parked: %d min", (int)(elapsed / 60));
            OLED_ShowString(0, 6, (uint8_t*)buf);
        }
        break;

    case 1:  /* 传感器数据页 */
        OLED_ShowString(0, 0, "--- Sensors ---");

        sprintf(buf, "Light : %d Lx", g_parking.light_lx);
        OLED_ShowString(0, 2, (uint8_t*)buf);

        sprintf(buf, "Temp  : %.1f C", g_parking.temperature);
        OLED_ShowString(0, 4, (uint8_t*)buf);

        sprintf(buf, "WiFi  : %s", g_parking.wifi_connected ? "OK" : "NO");
        OLED_ShowString(0, 6, (uint8_t*)buf);
        break;

    case 2:  /* 费用/记录页 */
        OLED_ShowString(0, 0, "--- Billing ---");

        if (g_parking.current.valid)
        {
            sprintf(buf, "Card: %02X%02X%02X%02X",
                    g_parking.current.card_id[0],
                    g_parking.current.card_id[1],
                    g_parking.current.card_id[2],
                    g_parking.current.card_id[3]);
            OLED_ShowString(0, 2, (uint8_t*)buf);

            if (g_parking.current.exit_time > 0)
            {
                sprintf(buf, "Fee: %d Yuan", g_parking.current.fee_yuan);
                OLED_ShowString(0, 4, (uint8_t*)buf);
            }
        }
        else
        {
            OLED_ShowString(0, 2, "No record");
        }
        break;
    }
}

/* ========== 按键事件处理 ========== */
void Parking_KeyHandler(uint8_t key)
{
    switch (key)
    {
    case 1:  /* K1: 模拟进场 */
        if (g_parking.state == PARKING_IDLE)
        {
            g_parking.car_present = 1;
            g_parking.state = PARKING_WAIT_CARD;
        }
        break;

    case 2:  /* K2: 模拟出场 */
        if (g_parking.state == PARKING_PARKED)
        {
            /* 模拟刷卡出场 */
            g_parking.card_detected = 1;
            memcpy(g_parking.card_uid, g_parking.current.card_id, 4);
        }
        break;

    case 3:  /* K3: 打印记录 */
        CLI_PrintLog();
        break;

    case 4:  /* K4: 管理员模式 */
        if (g_parking.state == PARKING_ALARM)
        {
            /* 清除报警 */
            g_parking.alarm_flag = 0;
            g_parking.state = PARKING_IDLE;
            LED3 = 0;
        }
        else
        {
            /* 手动开关道闸 */
            if (Gate_IsClosed())
                Gate_Open();
            else
                Gate_Close();
        }
        break;
    }
}

/* ========== 打印停车记录 ========== */
void Parking_PrintRecord(void)
{
    printf("\r\n========== Parking Record ==========\r\n");
    printf("State: %s\r\n", Parking_StateName(g_parking.state));
    printf("Distance: %d cm\r\n", g_parking.distance_cm);
    printf("Car Present: %s\r\n", g_parking.car_present ? "YES" : "NO");
    printf("Light: %d Lx\r\n", g_parking.light_lx);
    printf("Temperature: %.1f C\r\n", g_parking.temperature);

    if (g_parking.current.valid)
    {
        printf("Card: %02X%02X%02X%02X\r\n",
               g_parking.current.card_id[0], g_parking.current.card_id[1],
               g_parking.current.card_id[2], g_parking.current.card_id[3]);
        printf("Entry: %d\r\n", (int)g_parking.current.entry_time);
        printf("Fee: %d Yuan\r\n", g_parking.current.fee_yuan);
    }
    printf("====================================\r\n\r\n");
}
