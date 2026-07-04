/**
 * @file    billing.h
 * @brief   计费模块 - RTC 计时 + 阶梯费率
 */

#ifndef __BILLING_H__
#define __BILLING_H__

#include <stm32f10x.h>

/* ========== API ========== */
void     Billing_Init(void);                         /* 初始化 RTC */
uint32_t Billing_GetTime(void);                      /* 获取当前时间戳 (秒) */
uint16_t Billing_CalcFee(uint32_t entry, uint32_t exit); /* 计算费用 (元) */
uint8_t  Billing_GetHour(void);                      /* 当前小时 */
uint8_t  Billing_GetMinute(void);                    /* 当前分钟 */
uint8_t  Billing_GetSecond(void);                    /* 当前秒 */

/* ========== 费率配置 ========== */
#define RATE_FREE_MINUTES   30      /* 前30分钟免费 */
#define RATE_LOW_PER_HOUR   2       /* 30min~2h: 2元/时 */
#define RATE_MID_PER_HOUR   3       /* 2h~6h: 3元/时 */
#define RATE_HIGH_PER_HOUR  5       /* 6h+: 5元/时 */
#define RATE_MAX_PER_DAY    30      /* 每日封顶30元 */

#endif /* __BILLING_H__ */
