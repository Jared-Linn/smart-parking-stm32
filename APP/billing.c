/**
 * @file    billing.c
 * @brief   计费模块实现 - 基于 RTC 外设
 * @note    RTC 使用内部 40kHz LSI，提供秒级计时
 *          实际项目中建议使用外部 32.768kHz 晶振以提高精度
 */

#include "billing.h"
#include "rtc.h"
#include <time.h>

/* ========== 初始化 ========== */
void Billing_Init(void)
{
    RTC_Init();

    /* 如果 RTC 未配置，设置默认时间 2026-07-04 08:00:00 */
    /* RTC 初始化在上电时由 rtc.c 完成 */
}

/* ========== 获取当前时间戳 (从RTC计数器换算为秒) ========== */
uint32_t Billing_GetTime(void)
{
    /* RTC_GetCounter() 返回 RTC 计数器值
     * 这里简化处理：直接返回计数器值作为"秒"计数
     * 实际项目中需要根据 RTC 预分频配置计算
     */
    return RTC_GetCounter();
}

/* ========== 获取当前时分秒 ========== */
uint8_t Billing_GetHour(void)
{
    uint32_t t = Billing_GetTime();
    return (t / 3600) % 24;
}

uint8_t Billing_GetMinute(void)
{
    uint32_t t = Billing_GetTime();
    return (t / 60) % 60;
}

uint8_t Billing_GetSecond(void)
{
    uint32_t t = Billing_GetTime();
    return t % 60;
}

/* ========== 计算停车费用 ========== */
uint16_t Billing_CalcFee(uint32_t entry, uint32_t exit)
{
    uint32_t duration_sec;
    uint16_t duration_min;
    uint16_t fee = 0;

    if (exit <= entry) return 0;  /* 时间异常 */

    duration_sec = exit - entry;
    duration_min = duration_sec / 60;

    /* 前30分钟免费 */
    if (duration_min <= RATE_FREE_MINUTES)
        return 0;

    duration_min -= RATE_FREE_MINUTES;

    /* 阶梯计费 */
    if (duration_min <= 90)  /* 30分钟~2小时 (已减30分钟，剩余最多90分钟) */
    {
        fee = RATE_LOW_PER_HOUR * ((duration_min + 59) / 60);  /* 向上取整 */
    }
    else if (duration_min <= 330)  /* 2小时~6小时 (已减30分钟，剩余最多330分钟) */
    {
        /* 前2小时按低费率 */
        fee = RATE_LOW_PER_HOUR * 2;
        duration_min -= 120;
        fee += RATE_MID_PER_HOUR * ((duration_min + 59) / 60);
    }
    else  /* 超过6小时 */
    {
        /* 前2小时低费率 + 中间4小时中费率 */
        fee = RATE_LOW_PER_HOUR * 2 + RATE_MID_PER_HOUR * 4;
        duration_min -= 360;
        fee += RATE_HIGH_PER_HOUR * ((duration_min + 59) / 60);
    }

    /* 日封顶 */
    if (fee > RATE_MAX_PER_DAY)
        fee = RATE_MAX_PER_DAY;

    return fee;
}
