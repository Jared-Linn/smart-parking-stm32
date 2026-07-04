/**
 * @file    parking.c
 * @brief   停车场状态机 — Day 1: 按键模拟 + LED + OLED
 */

#include "parking.h"
#include "led.h"
#include "key.h"
#include "beep.h"
#include "oled_drv.h"
#include "delay.h"
#include <stdio.h>
#include <string.h>

ParkingSystem g_parking;

static const char* state_names[] = {
    "IDLE","WAIT_CARD","GATE_OPEN","GATE_CLOSE",
    "PARKED","EXIT_WAIT","EXIT_OPEN","ALARM"
};

const char* Parking_StateName(ParkingState s) {
    if (s <= PARKING_ALARM) return state_names[s];
    return "???";
}

void Parking_Init(void) {
    memset(&g_parking, 0, sizeof(g_parking));
    g_parking.state = PARKING_IDLE;
    OLED_Clear();
    OLED_ShowString(16, 0, (uint8_t*)"Smart Parking");
    OLED_ShowString(8, 2,  (uint8_t*)"Day1: Onboard");
    OLED_ShowString(0, 4,  (uint8_t*)"Press K1 to start");
    delay_ms(1000);
}

/* ========== 传感器更新 (Day 1: 按键模拟) ========== */
static void SensorUpdate(void) {
    g_parking.car_present_prev = g_parking.car_present;
    /* Day 2: 替换为超声波真实数据 */
    /* Day 2: 替换为 BH1750/MLX90614 真实数据 */
    g_parking.wifi_connected = 0;
}

/* ========== 状态机 ========== */
void Parking_StateMachine(void) {
    SensorUpdate();

    switch (g_parking.state) {
    case PARKING_IDLE:
        LED4 = 0;
        if (g_parking.car_present && !g_parking.car_present_prev) {
            g_parking.state = PARKING_WAIT_CARD;
            g_parking.card_detected = 0;
        }
        break;

    case PARKING_WAIT_CARD:
        if (g_parking.card_detected) {
            memset(&g_parking.current, 0, sizeof(ParkingRecord));
            g_parking.current.entry_time = g_parking.tick_ms / 1000;
            g_parking.current.valid = 1;
            g_parking.state = PARKING_GATE_OPEN;
            LED4 = 1;
        }
        if (!g_parking.car_present) {
            g_parking.state = PARKING_IDLE;
            g_parking.card_detected = 0;
        }
        break;

    case PARKING_GATE_OPEN:
        if (!g_parking.car_present && g_parking.car_present_prev) {
            g_parking.state = PARKING_GATE_CLOSE;
        }
        break;

    case PARKING_GATE_CLOSE:
        g_parking.state = PARKING_PARKED;
        LED4 = 1;
        break;

    case PARKING_PARKED:
        if (g_parking.card_detected) {
            g_parking.state = PARKING_EXIT_WAIT;
            g_parking.card_detected = 0;
        }
        break;

    case PARKING_EXIT_WAIT:
        g_parking.current.exit_time = g_parking.tick_ms / 1000;
        {
            uint32_t dur = g_parking.current.exit_time - g_parking.current.entry_time;
            g_parking.current.fee_yuan = (dur / 60) * 2;  /* 简化: 每分钟2元 */
        }
        g_parking.state = PARKING_EXIT_OPEN;
        break;

    case PARKING_EXIT_OPEN:
        if (!g_parking.car_present && g_parking.car_present_prev) {
            g_parking.state = PARKING_IDLE;
            LED4 = 0;
            memset(&g_parking.current, 0, sizeof(ParkingRecord));
        }
        break;

    case PARKING_ALARM:
        LED3 = ~LED3;
        if (g_parking.alarm_flag == 0) {
            g_parking.state = PARKING_IDLE;
            LED3 = 0;
        }
        break;

    default:
        g_parking.state = PARKING_IDLE;
        break;
    }

    g_parking.card_detected = 0;
    Parking_DisplayUpdate();
}

/* ========== OLED 显示 ========== */
void Parking_DisplayUpdate(void) {
    char buf[21];
    OLED_Clear();
    sprintf(buf, "State: %s", Parking_StateName(g_parking.state));
    OLED_ShowString(0, 0, (uint8_t*)buf);
    sprintf(buf, "Tick: %d", (int)g_parking.tick_ms / 100);
    OLED_ShowString(0, 2, (uint8_t*)buf);

    if (g_parking.current.valid && g_parking.state == PARKING_PARKED) {
        uint32_t elap = (g_parking.tick_ms / 1000) - g_parking.current.entry_time;
        sprintf(buf, "Parked: %d s", (int)elap);
        OLED_ShowString(0, 4, (uint8_t*)buf);
    }
    if (g_parking.current.fee_yuan > 0) {
        sprintf(buf, "Fee: %d Yuan", g_parking.current.fee_yuan);
        OLED_ShowString(0, 6, (uint8_t*)buf);
    }
}

/* ========== 按键处理 ========== */
void Parking_KeyHandler(uint8_t key) {
    switch (key) {
    case 1:  /* K1: 模拟车辆靠近 + 刷卡进场 */
        if (g_parking.state == PARKING_IDLE) {
            g_parking.car_present = 1;
            g_parking.card_detected = 0;
            BEEP_Test();  /* beep.c 自带 500ms 提示 */
        }
        break;
    case 2:  /* K2: 模拟刷卡 (进场/出场) */
        g_parking.card_detected = 1;
        GPIO_SetBits(GPIOA, GPIO_Pin_8);
        Delay_ms(50);
        GPIO_ResetBits(GPIOA, GPIO_Pin_8);
        break;
    case 3:  /* K3: 模拟车辆离开 */
        g_parking.car_present = 0;
        break;
    case 4:  /* K4: 清除报警/复位 */
        if (g_parking.state == PARKING_ALARM) {
            g_parking.alarm_flag = 0;
        }
        NVIC_SystemReset();
        break;
    }
}

/* ========== 串口打印 ========== */
void Parking_PrintRecord(void) {
    /* Day 2: USART1 printf */
}
