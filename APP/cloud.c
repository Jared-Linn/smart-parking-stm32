/**
 * @file    cloud.c — Day 1 桩 (Day 2: ESP8266 + MQTT)
 */
#include "cloud.h"
#include "parking.h"
#include "led.h"

void Cloud_Init(void)           { g_parking.wifi_connected = 0; LED2 = 0; }
void Cloud_ReportStatus(void)   {}
void Cloud_ReportEvent(const char* e, uint8_t* c) { (void)e; (void)c; }
void Cloud_ProcessCommand(void) {}
uint8_t Cloud_IsConnected(void) { return 0; }
