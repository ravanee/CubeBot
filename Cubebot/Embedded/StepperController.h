#ifndef _STEPPER_CONTROLLER_H_
#define _STEPPER_CONTROLLER_H_

#include "Stepper.h"
#include "Timer.h"
#include "Types.h"

typedef struct
{
     StepDirection  Direction;
     Stepper *      Stepper;
     uint16         NumOfSteps;
     TimerID        Timer;
}SteprCtrl;


void STPCTL_Rotate(SteprCtrl * Stepper, float Degrees);
BOOL STPCTL_IsBusy(SteprCtrl * Stepper);
void STPCTL_Stop(SteprCtrl * Stepper);
#endif