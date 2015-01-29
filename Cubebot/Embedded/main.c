#include <msp430f5529.h>
#include <string.h>
#include "Types.h"
#include "Comm.h"
#include "Timer.h"
#include "Robot.h"
#include "DigitalIO.h"

// Unified Clock System Init
static void UCS_Init(void);
static void UCS_Init(void)
{
     // Port select XT2
     P5SEL |= BIT2+BIT3; 
     // Set XT2 On
     UCSCTL6 &= ~XT2OFF; 
     // Enable Master, SubMaster, Aux Clocks
     UCSCTL4 = (SELM0 + SELM2) + (SELS0 + SELS2) + (SELA0 + SELA2);
     // Allow settling
     do
     {
          UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);                                               
          SFRIFG1 &= ~OFIFG;  // Clear fault flags
     }while (SFRIFG1&OFIFG);  // Test oscillator fault flag  
}

void LEDs(void * n)
{
     DIO_TogglePin(IO_PORT4, IO_PIN7);
     DIO_TogglePin(IO_PORT1, IO_PIN0);
     TIMER_SetTimer(TIMER_LEDS, 250, LEDs, 0);
}


int main(void)
{
     // Stop Watch Dog Timer
     WDTCTL = WDTPW + WDTHOLD; 
     // Initializing functions
     UCS_Init();
     COMM_Init();
     TIMER_Init();
     ROBOT_Init();
     
     /////////////////  TEST LEDs  ////////////////
     P1DIR |= BIT0;
     P4DIR |= BIT7;
     DIO_SetPin(IO_PORT4, IO_PIN7, TRUE);
     DIO_SetPin(IO_PORT1, IO_PIN0, FALSE);
     
     TIMER_SetTimer(TIMER_LEDS, 250, LEDs, 0);
     /////////////////////////////////////////////

     __enable_interrupt();
     // Task Loop
     
     while(1) 
     {
          COMM_Task();
          TIMER_Task();
          ROBOT_Task();
     }
}