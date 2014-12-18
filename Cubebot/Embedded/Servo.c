#include "Timer.h"
#include "Types.h"
#include "Servo.h"

#define SERVO_LEFT_LIMIT      (800)
#define SERVO_RIGHT_LIMIT     (2200)
#define SERVO_STEPS_PER_45DEG (500)

static void ServoMoveTimeoutHandler(void * Servo);


BOOL SERVO_SetPosition(ServoCtrl * Servo, uint16 Position)
{
     if(Servo)
     {
          // Make sure the Position is within limits
          if((Position > SERVO_LEFT_LIMIT) && (Position < SERVO_RIGHT_LIMIT))
          {
               TIMER_SetCCR(Servo->Pwm, Position);
               Servo->IsBusy = TRUE;
               // Set busy movement timeout
               TIMER_SetTimer(Servo->Timer, SERVO_MOVEMENT_TIMEOUT, ServoMoveTimeoutHandler, (void *) Servo);
               return TRUE;
          }
     }
     return FALSE;
}
// This function will return the PW position 
// coresponding to the angle from center.
// It will not move the servo.
// To move the servo, use SERVO_SetPosition
// If the angles aren't valid, the function will
// return 0, specifying an invalid PW
uint16 SERVO_AngleToPosition(ServoCtrl * Servo, float Angle)
{
     if(Servo)
     {
          // We only support -60 to +60 from center
          if((Angle < 60.0f) && (Angle > -60.0f))
          {
               float abs = (Angle > 0) ? Angle : (-1.0f * Angle);
               uint16 sign = (Angle > 0) ? 1 : (-1);
               return (Servo->Center + (sign) * ((uint16)(abs * SERVO_STEPS_PER_45DEG / 45.0f)));
          }
     }
     return 0;
}

BOOL SERVO_IsBusy(ServoCtrl * Servo)
{
     if(Servo)
     {
          return Servo->IsBusy;
     }
     return FALSE;
}

static void ServoMoveTimeoutHandler(void * Servo)
{
     // Convert back the servo struct
     ServoCtrl * motor = (ServoCtrl *) Servo;
     if(motor)
     {
          // The servo movement has timedout (done moving)
          motor->IsBusy = FALSE;
     }
}

void SERVO_SetCenter(ServoCtrl * Servo, uint16 Position)
{
     if(Servo)
     {
          // Set the center PW position
          Servo->Center = Position;
     }
}