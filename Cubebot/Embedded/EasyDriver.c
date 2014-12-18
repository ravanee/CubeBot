#include "EasyDriver.h"
#include "DigitalIO.h"
#include "Types.h"

#define   EZ_MICROSTEP_MULTIPLIER (8)

uint16 EZ_GetMicroStepMult(void)
{
     return EZ_MICROSTEP_MULTIPLIER;   
}

void EZ_Step(EzControl * EzObj, EzDirection Direction)
{
     if(EzObj)
     {
          // Set Direction
          DIO_SetGPIO(EzObj->Direction, (BOOL) Direction);
           // Create low to high transition
          DIO_SetGPIO(EzObj->Step, FALSE);
          // _RAVE_ Possibly add delay in here
          DIO_SetGPIO(EzObj->Step, TRUE);
     }
}