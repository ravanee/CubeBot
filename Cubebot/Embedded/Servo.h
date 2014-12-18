#ifndef _SERVO_H_
#define _SERVO_H_

#include "Timer.h"
#include "Types.h"

#define SERVO_DEFAULT_CENTER  (1500)
#define SERVO_MOVEMENT_TIMEOUT (1000)

typedef struct
{
     PwmID     Pwm;
     uint16    Center; 
     BOOL      IsBusy;
     TimerID   Timer;
}ServoCtrl;

BOOL      SERVO_SetPosition(ServoCtrl * Servo, uint16 Position);
uint16    SERVO_AngleToPosition(ServoCtrl * Servo, float Angle);
BOOL      SERVO_IsBusy(ServoCtrl * Servo);
void      SERVO_SetCenter(ServoCtrl * Servo, uint16 Position);
#endif