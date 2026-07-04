# 智能停车场管理系统 (Smart Parking Management System)

基于 STM32F103 的综合停车场管理系统，覆盖 17 种外设/模块。

## 硬件平台

- **MCU**: STM32F103RCT6 (Cortex-M3, 72MHz)
- **开发板**: 百科荣创 移动互联开发平台 STM32 V2.0
- **IDE**: Keil MDK 5.25 + STM32F1xx DFP 2.1.0

## 功能

| 模块 | 功能 |
|------|------|
| HC-SR04 超声波 | 车辆进出检测 |
| MFRC522 RFID | 刷卡身份识别 |
| 舵机 | 道闸抬杆/落杆 |
| OLED 0.96" | 实时信息显示 |
| BH1750 | 光照度检测 |
| MLX90614 | 红外测温 |
| WS2812 | 车位引导灯 |
| 蜂鸣器 | 提示音/报警 |
| ESP8266 WiFi | MQTT 云端通信 |
| RTC | 停车计时 |
| IWDG | 看门狗保护 |
| USART CLI | PC 串口命令 |

## 快速开始

### 1. 打开工程
用 Keil MDK 打开 `USER/Project.uvprojx`

### 2. 编译
点击 **Build (F7)**，确保 **0 Error(s), 0 Warning(s)**

### 3. 烧录
- 连接 ST-LINK 到开发板
- 点击 **Download (F8)**
- 复位开发板，程序开始运行

### 4. 串口 CLI
- 波特率: 115200 8N1
- 命令: `status` `open` `close` `log` `help` `reset`

## 项目结构

```
├── CORE/          CMSIS 核心文件
├── FWLIB/         STM32 标准外设库 (SPL)
├── SYSTEM/        系统层 (delay/sys/usart)
├── HARDWARE/      硬件驱动层 (15个模块)
│   ├── LED/ KEY/ BEEP/ OLED/
│   ├── BH1750/ MLX90614/ UltrasonicWave/
│   ├── RC522/ SteerGear/ WS2812/
│   ├── ESP8266/ usart3/ MQTT/
│   └── RTC/ IWDG/ ADC/ DAC/ DMA/
├── APP/           应用层
│   ├── parking.c   状态机核心
│   ├── gate.c      道闸控制
│   ├── billing.c   计费逻辑
│   ├── cloud.c     云端通信
│   └── cli.c       串口命令
├── USER/
│   └── main.c     主程序
└── DOC/
    └── 设计文档.md  完整设计文档
```

## 3天开发计划

- **Day 1**: 工程框架 + 在板外设 + 传感器驱动
- **Day 2**: 核心业务逻辑 + WiFi/云通信
- **Day 3**: 联调 + 测试 + 验收

## License

Educational project for STM32 learning.
