#ifndef _STEPPER_H_
#define _STEPPER_H_
#include "Types.h"
#include "EasyDriver.h"

typedef enum
{
     STEPPER_CW,
     STEPPER_CCW,
}StepDirection;

typedef struct
{
     EzControl *    EzDriver;  
}Stepper;

void STEPPER_Step(Stepper * Stepper, StepDirection Direction);
uint16 STEPPER_StepsPerRev(void);

#endif


