/**
 * @file    cli.c
 * @brief   串口命令行实现
 * @note    USART1 (PA9=TX, PA10=RX), 115200 8N1
 *          支持命令: status, open, close, log, help, reset
 */

#include "cli.h"
#include "parking.h"
#include "gate.h"
#include "billing.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

#define CLI_BUF_SIZE    64
#define CLI_PROMPT      "\r\nParking> "

static char  cli_buf[CLI_BUF_SIZE];
static uint8_t cli_idx = 0;
static uint8_t cli_ready = 0;

/* ========== 初始化 ========== */
void CLI_Init(void)
{
    uart_init(115200);  /* USART1 初始化 */
    memset(cli_buf, 0, CLI_BUF_SIZE);
    cli_idx = 0;
    cli_ready = 0;

    printf("\r\n========================================\r\n");
    printf("  Smart Parking Management System\r\n");
    printf("  STM32F103  |  Version 1.0\r\n");
    printf("  Type 'help' for commands\r\n");
    printf("========================================\r\n");
    printf(CLI_PROMPT);
}

/* ========== 主循环处理 ========== */
void CLI_Process(void)
{
    /* 检查串口是否有数据 */
    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE))
    {
        char ch = USART_ReceiveData(USART1);

        /* 回显 */
        USART_SendData(USART1, ch);

        if (ch == '\r' || ch == '\n')
        {
            if (cli_idx > 0)
            {
                cli_buf[cli_idx] = '\0';
                cli_ready = 1;
            }
        }
        else if (ch == '\b' || ch == 0x7F)  /* 退格 */
        {
            if (cli_idx > 0) cli_idx--;
        }
        else if (cli_idx < CLI_BUF_SIZE - 1)
        {
            cli_buf[cli_idx++] = ch;
        }
    }

    /* 处理就绪命令 */
    if (cli_ready)
    {
        cli_ready = 0;
        cli_idx = 0;

        if (strcmp(cli_buf, "help") == 0)
        {
            CLI_PrintHelp();
        }
        else if (strcmp(cli_buf, "status") == 0)
        {
            Parking_PrintRecord();
        }
        else if (strcmp(cli_buf, "open") == 0)
        {
            Gate_Open();
            printf("OK: Gate opened\r\n");
        }
        else if (strcmp(cli_buf, "close") == 0)
        {
            Gate_Close();
            printf("OK: Gate closed\r\n");
        }
        else if (strcmp(cli_buf, "log") == 0)
        {
            CLI_PrintLog();
        }
        else if (strcmp(cli_buf, "reset") == 0)
        {
            printf("OK: Resetting...\r\n");
            NVIC_SystemReset();
        }
        else if (strlen(cli_buf) > 0)
        {
            printf("ERR: Unknown command '%s'\r\n", cli_buf);
            printf("Type 'help' for command list\r\n");
        }

        printf(CLI_PROMPT);
    }
}

/* ========== 帮助信息 ========== */
void CLI_PrintHelp(void)
{
    printf("\r\n");
    printf("Available commands:\r\n");
    printf("  status  - Show system status\r\n");
    printf("  open    - Open parking gate\r\n");
    printf("  close   - Close parking gate\r\n");
    printf("  log     - Print parking records\r\n");
    printf("  reset   - System reset\r\n");
    printf("  help    - Show this help\r\n");
    printf("\r\n");
}

/* ========== 打印记录 ========== */
void CLI_PrintLog(void)
{
    Parking_PrintRecord();
}
