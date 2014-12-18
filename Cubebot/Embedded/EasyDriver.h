#ifndef _EASY_DRIVER_H_
#define _EASY_DRIVER_H_
#include "DigitalIO.h"
#include "Types.h"

typedef enum
{
	EZ_CW,
	EZ_CCW
}EzDirection;

typedef struct
{
	GPIO	Step;
	GPIO	Direction;
}EzControl;

uint16 EZ_GetMicroStepMult(void);
void EZ_Step(EzControl * EzObj, EzDirection Direction);
#endif





