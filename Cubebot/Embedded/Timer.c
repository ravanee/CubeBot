#include <msp430f5529.h>
#include <string.h>
#include "Types.h"
#include "Timer.h"

typedef struct
{
     BOOL           Active;
     uint16         Remaining;
     ExpireHandler  Handler;
     void *         Data;
}TimerControl;

#define SYSTEM_TIMER_ms     (1)
#define SYSTEM_TIMER_us     (SYSTEM_TIMER_ms * 1000) 
#define SERVO_PWM_PERIOD_us  (20000)

static volatile BOOL TimerExpired;

static TimerControl SoftTimers[TIMER_MAX_TIMERS];

typedef struct
{
     volatile uint16 * Cntl;
     volatile uint16 * Reg;
}CCRs;

// Lookup table for the CCRs
static CCRs TimerA_CCRs[TA_CCR_MAX] = 
{
     {
          .Cntl = &TA0CCTL1,
          .Reg  = &TA0CCR1
     },
     {
          .Cntl = &TA0CCTL2,
          .Reg  = &TA0CCR2
     },
     {
          .Cntl = &TA0CCTL3,
          .Reg  = &TA0CCR3
     },
     {
          .Cntl = &TA0CCTL4,
          .Reg  = &TA0CCR4
     }
};

static void Timer_TA_Init(void);
static void Timer_TB_Init(void);

static void Timer_TA_Init(void)
{
      ///// Hardware /////
     // TASSEL_2 : SMCLK (4MHz)
     // ID_2 : Divide input clock (SMCLK) by 4 (resulting in 1MHz)
     // MC_1 : Put in up mode to CCRO
     TA0CTL = TASSEL_2 + MC_1 + ID_2;
     // Set CCR0 to expire every 1ms
     TA0CCR0 = SERVO_PWM_PERIOD_us - 1; 
     // Clear CCR0 interrupt flag
     TA0CCTL0 &= ~CCIFG; 
     // Allow interrupt
     TA0CCTL0 |= CCIE;
     
     // Clear any pending flags
     TA0CTL &= ~TAIFG;
     // Enable TimerA interrupts
     // TA0CTL |= TAIE; 
}

static void Timer_TB_Init(void)
{
     TB0CTL = TBSSEL_2 + MC_1 + ID_2;
     // Set CCR0 to expire every 1ms
     TB0CCR0 = SYSTEM_TIMER_us - 1; 
     // Clear CCR0 interrupt flag
     TB0CCTL0 &= ~CCIFG; 
     // Allow interrupt
     TB0CCTL0 |= CCIE; 
}

void TIMER_Init(void)
{
     // Initialize HW
     Timer_TA_Init();
     Timer_TB_Init();
     
     // Initialize SW
     memset(SoftTimers, 0, sizeof(TimerControl) * TIMER_MAX_TIMERS);
     TimerExpired = FALSE;
}

void TIMER_Task(void)
{
     // Has 1ms passed?
     if(TimerExpired)
     {
          uint8 index;
          // Clear expired flag
          TimerExpired = FALSE;
          // Update all software timers
          for(index = 0; index < TIMER_MAX_TIMERS; index++)
          {
               // Get timer from list
               TimerControl * timer = &SoftTimers[index];
               // Is this timer active
               if(timer->Active)
               {
                    // Is there time remaining before it expires
                    if(timer->Remaining)
                    {
                         // If so, decrement 1ms
                         timer->Remaining--;
                    }
                    else
                    {
                         // Timer has expired
                         // Set to inactive
                         timer->Active = FALSE;
                         // Call Handler
                         timer->Handler(timer->Data);
                    }
               }
          }
     }
}
BOOL TIMER_SetTimer(TimerID Id, uint16 Delay, ExpireHandler Handler, void * Data)
{
     // Is this a valid timer ID?
     if(Id < TIMER_MAX_TIMERS)
     {
          // Grab the timer control from the list
          TimerControl * timer = &(SoftTimers[Id]);
          if(!timer->Active)
          {
               // Set as active
               timer->Active = TRUE;
               // Load the delay
               timer->Remaining = (Delay / SYSTEM_TIMER_ms);
               // Save the function handler for expires
               if(Handler)
               {
                    // Save expire handler
                    timer->Handler = Handler;
                    // Save data that should be called with handler
                    timer->Data = Data;
                    // return successful
                    return TRUE;
               }
          }
     }
     // return failed
     return FALSE;
}

void TIMER_StopTimer(TimerID Id)
{
     // Is timer valid
     if(Id < TIMER_MAX_TIMERS)
     {
          // Set as inactive
          SoftTimers[Id].Active = FALSE;
     }
}

void TIMER_StartCCR(PwmID Id)
{
     if(Id < TA_CCR_MAX)
     {
          // Set as Reset/Set output mode
          *TimerA_CCRs[Id].Cntl |= OUTMOD_7;
     }
}

void TIMER_SetCCR(PwmID Id, uint16 Value)
{
     if(Id < TA_CCR_MAX)
     {
          // Load the CCR register with this value
          *TimerA_CCRs[Id].Reg = Value;
     }
}

void TIMER_StopCCR(PwmID Id)
{
     if(Id < TA_CCR_MAX)
     {
          // Disable the PWM function for this CCR
          *TimerA_CCRs[Id].Cntl &= ~OUTMOD_7;
     }
}




#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
     
}
#define TA0_NO_INTERRUPT (0)
#define TA0_CCR1         (2)
#define TA0_CCR2         (4)
#define TA0_CCR3         (6)
#define TA0_CCR4         (8)

#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR(void)
{
    /* switch(TA0IV)
     {
          case TA0_NO_INTERRUPT : 
               break;
          case TA0_CCR1 : 
               break;
          case TA0_CCR2 :
               break;
          case TA0_CCR3 :
               break;
          case TA0_CCR4 :
               break;
          default :
               break;
     }*/
}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void TIMER0_B0_ISR(void)
{
     TimerExpired = TRUE;
}

#pragma vector=TIMER0_B1_VECTOR
__interrupt void TIMER0_B1_ISR(void)
{
     
}
