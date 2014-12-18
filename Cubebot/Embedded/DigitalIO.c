#include <msp430f5529.h>
#include "DigitalIO.h"

volatile uint8 * const PORTS_OUT[IO_PORTS_MAX] = 
{
     &P1OUT,
     &P2OUT,
     &P3OUT,
     &P4OUT,
     &P5OUT,
     &P6OUT,
     &P7OUT,
     &P8OUT
};

BOOL DIO_SetGPIO(GPIO Gpio, BOOL Value)
{
     return DIO_SetPin(Gpio.Port, Gpio.Pin, Value);
}

BOOL DIO_GetGPIO(GPIO Gpio)
{
     return DIO_GetPin(Gpio.Port, Gpio.Pin);
}

BOOL DIO_ToggleGPIO(GPIO Gpio)
{
     return DIO_TogglePin(Gpio.Port, Gpio.Pin);
}

BOOL DIO_SetPin(IO_PORT Port, IO_PIN Pin, BOOL Value)
{
     // Make sure port is valid
     if(Port < IO_PORTS_MAX)
     {
          if(Value)
          {
               // If they want a High, set pin
               *PORTS_OUT[Port] |= (1 << Pin);
          }
          else
          {
               // If they want a Low, clear pin
               *PORTS_OUT[Port] &= ~(1 << Pin);
          }    
          return TRUE;
     }
     return FALSE;
}

BOOL DIO_GetPin(IO_PORT Port, IO_PIN Pin)
{
     // Make sure port is valid
     if(Port < IO_PORTS_MAX)
     {
          // Get the status of the pin
          return (*PORTS_OUT[Port] & (1 << Pin));
     }
     return FALSE;
}

BOOL DIO_TogglePin(IO_PORT Port, IO_PIN Pin)
{
     // Make sure the port is valid
     if(Port < IO_PORTS_MAX)
     {
          // Toggle only that pin
          *PORTS_OUT[Port] ^= (1 << Pin);
          return TRUE;
     }
     return FALSE;
}