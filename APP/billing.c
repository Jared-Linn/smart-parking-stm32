/**
 * @file    billing.c — Day 1 桩 (Day 2: RTC)
 */
#include "billing.h"
#include "parking.h"

void Billing_Init(void) {}
uint32_t Billing_GetTime(void) { return g_parking.tick_ms / 1000; }
uint16_t Billing_CalcFee(uint32_t e, uint32_t x) {
    uint32_t d = (x > e) ? (x - e) : 0;
    return (uint16_t)(d / 60 * 2);
}
uint8_t Billing_GetHour(void)   { return 12; }
uint8_t Billing_GetMinute(void) { return 0;  }
uint8_t Billing_GetSecond(void) { return 0;  }
