/**
 * @file    gate.c
 * @brief   道闸控制实现
 * @note    超声波: PA2=TRIG, PA3=ECHO (TIM3计时)
 *          舵机: PA0, TIM2_CH1 PWM 50Hz
 */

#include "gate.h"
#include "UltrasonicWave.h"
#include "SteerGear.h"
#include "delay.h"

static uint8_t gate_angle = 0;   /* 当前角度 */

void Gate_Init(void)
{
    Ultrasonic_Init();   /* PA2=TRIG, PA3=ECHO, TIM3计时 */
    Servo_Init();        /* PA0, TIM2_CH1, 50Hz PWM */
    Gate_Close();        /* 初始关闭 */
}

void Gate_Open(void)
{
    Servo_SetAngle(90);
    gate_angle = 90;
    delay_ms(500);  /* 给舵机时间转动 */
}

void Gate_Close(void)
{
    Servo_SetAngle(0);
    gate_angle = 0;
    delay_ms(500);
}

uint8_t Gate_IsClosed(void)
{
    return (gate_angle == 0);
}

uint8_t Gate_IsOpen(void)
{
    return (gate_angle == 90);
}
