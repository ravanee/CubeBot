#ifndef _UART_H_
#define _UART_H_

#include "Types.h"
void UART_Init(void);
void UART_Task(void);
BOOL UART_Send(uint8 * Data, uint8 Length);
BOOL UART_GetRx(uint8 * Data);
BOOL UART_IsActive(void);

#endif