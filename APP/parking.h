/**
 * @file    parking.h
 * @brief   停车场核心状态机模块
 * @author  STM32 Learner
 * @date    2026-07-04
 */

#ifndef __PARKING_H__
#define __PARKING_H__

#include <stm32f10x.h>

/* ========== 停车场状态枚举 ========== */
typedef enum {
    PARKING_IDLE = 0,       /* 空闲，等待车辆 */
    PARKING_WAIT_CARD,      /* 检测到车辆，等待刷卡 */
    PARKING_GATE_OPEN,      /* 刷卡成功，道闸打开 */
    PARKING_GATE_CLOSE,     /* 车辆通过，道闸关闭 */
    PARKING_PARKED,         /* 已停车，计时中 */
    PARKING_EXIT_WAIT,      /* 等待出场刷卡 */
    PARKING_EXIT_OPEN,      /* 出场抬杆 */
    PARKING_ALARM           /* 异常报警 */
} ParkingState;

/* ========== 停车记录 ========== */
typedef struct {
    uint8_t  card_id[4];    /* 卡号 (MIFARE UID) */
    uint32_t entry_time;    /* 入场时间戳 (RTC 秒) */
    uint32_t exit_time;     /* 出场时间戳 */
    uint16_t fee_yuan;      /* 费用 (元) */
    uint8_t  valid;         /* 记录有效标志 */
} ParkingRecord;

/* ========== 系统全局状态 ========== */
typedef struct {
    ParkingState state;         /* 当前状态 */
    ParkingRecord current;      /* 当前停车记录 */
    uint8_t  card_uid[4];       /* 当前读取的卡号 */
    uint8_t  card_detected;     /* 检测到卡片标志 */
    uint8_t  car_present;       /* 当前车辆在位标志 */
    uint8_t  car_present_prev;  /* 上一周期车辆在位 */
    uint16_t distance_cm;       /* 超声波测距值 (cm) */
    uint16_t light_lx;          /* 光照度 (Lx) */
    float    temperature;       /* 环境温度 (°C) */
    float    mcu_temp;          /* MCU 内部温度 */
    uint8_t  wifi_connected;    /* WiFi 连接状态 */
    uint8_t  alarm_flag;        /* 报警标志 */
    uint32_t tick_ms;           /* 系统运行毫秒计数 */
} ParkingSystem;

/* ========== 全局实例 ========== */
extern ParkingSystem g_parking;

/* ========== API 函数 ========== */
void Parking_Init(void);                /* 初始化停车场系统 */
void Parking_StateMachine(void);        /* 状态机主循环 (每10ms调用) */
void Parking_SensorUpdate(void);        /* 更新所有传感器数据 */
void Parking_DisplayUpdate(void);       /* 更新 OLED 显示 */
void Parking_KeyHandler(uint8_t key);   /* 按键事件处理 */
const char* Parking_StateName(ParkingState s);  /* 状态名称字符串 */

/* ========== 配置宏 ========== */
#define CAR_DISTANCE_THRESHOLD  50      /* 车辆检测距离阈值 (cm) */
#define LIGHT_THRESHOLD_LOW     100     /* 低光照阈值 (Lx) */
#define TEMP_ALARM_HIGH         40.0f   /* 高温报警阈值 (°C) */
#define GATE_OPEN_ANGLE         90      /* 道闸打开角度 */
#define GATE_CLOSE_ANGLE        0       /* 道闸关闭角度 */

#endif /* __PARKING_H__ */
