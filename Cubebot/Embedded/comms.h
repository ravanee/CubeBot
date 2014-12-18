#ifndef _COMMS_H_
#define _COMMS_H_

#include "Types.h"

typedef void (*RxFunc) (uint8 *, uint8);

typedef enum
{
     CMD_DEBUG = 0,
     CMD_ROBOT,
     CMD_CUBE,
     CMD_MAX_COMMAND
}CommCommand;

void COMM_Init(void);
BOOL COMM_Send(CommCommand Command, uint8 * Data, uint8 Length);
void COMM_RegisterHandler(CommCommand Command, RxFunc Handler);
void COMM_UnregisterHandler(CommCommand Command);
void COMM_Task(void);

#endif