#ifndef __BEEP_H_
#define __BEEP_H_

#include "sys.h"

#define LED1 PDout(8)
#define LED2 PDout(9)
#define LED3 PDout(10)
#define LED4 PDout(11)

void BEEP_Init(void);
void BEEP_Test(void);

#endif