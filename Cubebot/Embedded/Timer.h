#ifndef _TIMER_H_
#define _TIMER_H_

#include "Types.h"
typedef void (*ExpireHandler) (void *);

typedef enum
{
     TIMER_TEST = 0,
     TIMER_LEDS,
     TIMER_LEFT_ROTATE,
     TIMER_LEFT_CLAW,
     TIMER_RIGHT_ROTATE,
     TIMER_RIGHT_CLAW,
     TIMER_MAX_TIMERS
}TimerID;

typedef enum
{
     TA_CCR1 = 0,
     TA_CCR2,
     TA_CCR3,
     TA_CCR4,
     TA_CCR_MAX
}PwmID;

void TIMER_Init(void);
void TIMER_Task(void);
BOOL TIMER_SetTimer(TimerID Id, uint16 Delay, ExpireHandler Handler, void * Data);
void TIMER_StopTimer(TimerID Id);

void TIMER_StartCCR(PwmID Id);
void TIMER_SetCCR(PwmID Id, uint16 Value);
void TIMER_StopCCR(PwmID Id);

#endif