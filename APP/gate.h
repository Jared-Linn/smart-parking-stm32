/**
 * @file    gate.h
 * @brief   道闸控制模块 - 超声波+舵机联动
 */

#ifndef __GATE_H__
#define __GATE_H__

#include <stm32f10x.h>

void Gate_Init(void);           /* 初始化: 超声波 + 舵机 */
void Gate_Open(void);           /* 抬杆 (舵机转90°) */
void Gate_Close(void);          /* 落杆 (舵机转0°) */
uint8_t Gate_IsClosed(void);    /* 查询道闸是否关闭 */
uint8_t Gate_IsOpen(void);      /* 查询道闸是否打开 */

#endif /* __GATE_H__ */
