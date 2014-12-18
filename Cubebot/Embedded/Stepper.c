#include "Stepper.h"
#include "Types.h"
#include "EasyDriver.h"

#define STEPS_PER_REV    (200)

void STEPPER_Step(Stepper * Stepper, StepDirection Direction)
{
     // Is this pointer valid? (not null)
     if(Stepper)
     {
          // Tell the Easy Driver to step
          EZ_Step(Stepper->EzDriver, (EzDirection) Direction);
     }
}

uint16 STEPPER_StepsPerRev(void)
{
     // Return how many steps are in 1 revolution of the stepper motor
     return STEPS_PER_REV * EZ_GetMicroStepMult();    
}