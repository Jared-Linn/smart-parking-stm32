#ifndef __ESP8266_H_
#define __ESP8266_H_

#include "stm32f10x.h"

extern char *ssid;
extern char *password;

uint8_t ESP8266_SCOM(char *sdat,uint8_t mode);
void ESP8266_Software_Init(void);			

#endif
