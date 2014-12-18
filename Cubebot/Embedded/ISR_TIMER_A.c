#include <msp430f5529.h>
#include "Timer.h"


#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
     // Set boolean to signal that 1ms has passed
     // Handled in the timer task
     TIMER_SetTick();
}

#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void)
{
     
}

