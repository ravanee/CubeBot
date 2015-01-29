#include "RobotArm.h"
#include "Types.h"
#include "StepperController.h"
#include "Servo.h"


static void MoveClaw(RobotArm * Arm, float Degrees);

BOOL ARM_IsActive(RobotArm * Arm)
{
     // Is this pointer valid? (not null)
     if(Arm)
     {
          // If either the claw or rotater has steps left, we are busy
          return (SERVO_IsBusy(Arm->Claw) || STPCTL_IsBusy(Arm->Rotater));
     }
     return FALSE;
}
void ARM_Rotate(RobotArm * Arm, float Degrees)
{
     // Is this pointer valid? (not null)
     if(Arm)
     {
          // Setup stepper to rotate the degrees requested
          // CW : +degreees
          // CCW : -degrees
          STPCTL_Rotate(Arm->Rotater, Degrees);
     }
}

void ARM_MoveClaw(RobotArm * Arm, float Degrees)
{
     // Is this pointer valid? (not null)
     if(Arm)
     {
          // Setup the claw to open
          MoveClaw(Arm, Degrees);
          // Set claw as currently open
          //Arm->ClawOpen = TRUE;
     } 
}
/*
void ARM_CloseClaw(RobotArm * Arm, float Degrees)
{
     // Is this pointer valid? (not null)
     if(Arm)
     {
          // Setup the claw to close
          MoveClaw(Arm, (-1.0f * Degrees));
          // Set claw as currently closed
          Arm->ClawOpen = FALSE;
     }
}
*/
BOOL ARM_IsClawOpen(RobotArm * Arm)
{
     // Is this pointer valid? (not null)
     if(Arm)
     {
          // Return the state of the claw
          return Arm->ClawOpen;
     }
     return FALSE;
}


static void MoveClaw(RobotArm * Arm, float Degrees)
{
     // Rotate the claw based on Open or Closed
     uint16 position = SERVO_AngleToPosition(Arm->Claw, Degrees);
     SERVO_SetPosition(Arm->Claw, position);
}

