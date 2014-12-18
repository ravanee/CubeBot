#ifndef _ROBOT_ARM_H_
#define _ROBOT_ARM_H_
#include "Types.h"
#include "StepperController.h"
#include "Servo.h"


typedef struct
{
     SteprCtrl *    Rotater;
     ServoCtrl *    Claw;
     BOOL           ClawOpen;
}RobotArm;


void ARM_Rotate(RobotArm * Arm, float Degrees);
void ARM_OpenClaw(RobotArm * Arm, float Degrees);
void ARM_CloseClaw(RobotArm * Arm, float Degrees);
BOOL ARM_IsClawOpen(RobotArm * Arm);
BOOL ARM_IsActive(RobotArm * Arm);


void ARM_DEBUG_SetClawSwing(RobotArm * Arm, float Degrees);

#endif
