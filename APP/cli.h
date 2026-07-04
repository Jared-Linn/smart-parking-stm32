/**
 * @file    cli.h
 * @brief   串口命令行接口 - USART1 交互
 */

#ifndef __CLI_H__
#define __CLI_H__

#include <stm32f10x.h>

void CLI_Init(void);            /* 初始化串口 CLI */
void CLI_Process(void);         /* 处理串口命令 (主循环中调用) */
void CLI_PrintLog(void);        /* 打印停车记录 */
void CLI_PrintHelp(void);       /* 打印帮助信息 */

#endif /* __CLI_H__ */
