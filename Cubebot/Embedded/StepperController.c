#include "StepperController.h"
#include "Stepper.h"
#include "Timer.h"
#include "Types.h"
#define MILLISEC_PER_STEP     (2)

static uint16 AngleToSteps(float Angle);
static void StepTheStepper(void * Stepper);

BOOL STPCTL_IsBusy(SteprCtrl * StepMotor)
{
     // Is this pointer valid? (not null)
     if(StepMotor)
     {
          // The stepper is busy if it still has steps to perform
          return (StepMotor->NumOfSteps > 0) ? TRUE : FALSE;
     }
     return FALSE;
}

void STPCTL_Stop(SteprCtrl * StepMotor)
{
     // Is this pointer valid? (not null)
     if(StepMotor)
     {
          // Set steps left as zero
          StepMotor->NumOfSteps = 0;
          // Stop the timer from shooting off again
          TIMER_StopTimer(StepMotor->Timer);
     }
}
void STPCTL_Rotate(SteprCtrl * StepMotor, float Degrees)
{
     // Is this pointer valid? (not null)
     if(StepMotor)
     {
          // Get the direction
          // CW : +degrees
          // CCW : -degrees
          StepMotor->Direction = (Degrees > 0) ? STEPPER_CW : STEPPER_CCW;
          // Convert the angle to steps
          StepMotor->NumOfSteps = AngleToSteps(Degrees);
          // Setup timer for next step
          TIMER_SetTimer(StepMotor->Timer, MILLISEC_PER_STEP, StepTheStepper, (void *) StepMotor);
     }
}
  

static uint16 AngleToSteps(float Angle)
{
     uint16 steps;
     Angle = (Angle < 0.0f) ? ( -1.0f * Angle) : (Angle);
     steps = (uint16)((Angle / 360.0f) * STEPPER_StepsPerRev());
     return steps;
}

static void StepTheStepper(void * StepMotor)
{
     // Convert back the stepper struct
     SteprCtrl * motor = (SteprCtrl *) StepMotor;
     if(motor)
     {
          // If there are steps left, step the motor and reset the timer
          // If not, do set the timer to shoot again
          if(motor->NumOfSteps)
          {
               // Decrement the stepper count
               motor->NumOfSteps--;
               // Step the motor
               STEPPER_Step(motor->Stepper, motor->Direction);
               // Setup timer again
               TIMER_SetTimer(motor->Timer, MILLISEC_PER_STEP, StepTheStepper, StepMotor);
          }
     }
}

