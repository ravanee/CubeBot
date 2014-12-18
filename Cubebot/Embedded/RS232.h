#ifndef _RS232_H_
#define _RS232_H_

#include "types.h"
void RS232_Init(void);
void RS232_Task(void);
BOOL RS232_Send(uint8 * Data, uint8 Length);
BOOL RS232_GetRx(uint8 * Data);
BOOL RS232_IsActive(void);
#endif

