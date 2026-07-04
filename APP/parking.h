/**
 * @file    parking.h
 * @brief   停车场核心状态机 — Day 1 精简版 (在板外设)
 */

#ifndef __PARKING_H__
#define __PARKING_H__

#include <stm32f10x.h>

/* ========== 状态枚举 ========== */
typedef enum {
    PARKING_IDLE = 0,
    PARKING_WAIT_CARD,
    PARKING_GATE_OPEN,
    PARKING_GATE_CLOSE,
    PARKING_PARKED,
    PARKING_EXIT_WAIT,
    PARKING_EXIT_OPEN,
    PARKING_ALARM
} ParkingState;

/* ========== 停车记录 ========== */
typedef struct {
    uint8_t  card_id[4];
    uint32_t entry_time;
    uint32_t exit_time;
    uint16_t fee_yuan;
    uint8_t  valid;
} ParkingRecord;

/* ========== 系统状态 ========== */
typedef struct {
    ParkingState state;
    ParkingRecord current;
    uint8_t  card_uid[4];
    uint8_t  card_detected;
    uint8_t  car_present;
    uint8_t  car_present_prev;
    uint16_t distance_cm;
    uint16_t light_lx;
    float    temperature;
    uint8_t  wifi_connected;
    uint8_t  alarm_flag;
    uint32_t tick_ms;
} ParkingSystem;

extern ParkingSystem g_parking;

/* ========== API ========== */
void Parking_Init(void);
void Parking_StateMachine(void);
void Parking_DisplayUpdate(void);
void Parking_KeyHandler(uint8_t key);
void Parking_PrintRecord(void);
const char* Parking_StateName(ParkingState s);

/* ========== 阈值 ========== */
#define CAR_DISTANCE_THRESHOLD  50
#define LIGHT_THRESHOLD_LOW     100
#define TEMP_ALARM_HIGH         40.0f
#define GATE_OPEN_ANGLE         90
#define GATE_CLOSE_ANGLE        0

#endif
