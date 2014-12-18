#ifndef _DIGITAL_IO_H_
#define _DIGITAL_IO_H_
#include "types.h"

typedef enum
{
     IO_PORT1, 
     IO_PORT2,
     IO_PORT3,
     IO_PORT4,
     IO_PORT5,
     IO_PORT6,
     IO_PORT7,
     IO_PORT8,
     IO_PORTS_MAX
}IO_PORT;

typedef enum
{
     IO_PIN0, 
     IO_PIN1,
     IO_PIN2, 
     IO_PIN3, 
     IO_PIN4, 
     IO_PIN5, 
     IO_PIN6, 
     IO_PIN7
}IO_PIN;

typedef struct
{
     IO_PORT   Port;
     IO_PIN    Pin;
}GPIO;

BOOL DIO_SetPin(IO_PORT Port, IO_PIN Pin, BOOL Value);
BOOL DIO_GetPin(IO_PORT Port, IO_PIN Pin);
BOOL DIO_TogglePin(IO_PORT Port, IO_PIN Pin);
BOOL DIO_SetGPIO(GPIO Gpio, BOOL Value);
BOOL DIO_GetGPIO(GPIO Gpio);
BOOL DIO_ToggleGPIO(GPIO Gpio);

#endif