#include <msp430f5529.h>
#include <string.h>
#include "UART.h"
#include "Types.h"

#define NO_INTERRUPT    (0x00)
#define RX_INTERRUPT    (0x02)
#define TX_INTERRUPT    (0x04)

#define RX_QUEUE    (16)

typedef struct
{
     uint8 Queue[RX_QUEUE];
     uint8 NextEmpty;
     uint8 Unload;
}RX_Queue;


typedef struct
{
     BOOL      Start;
     BOOL      Active;
     uint8 *   TxData;
     uint8     DataLength;
     uint8     NextTx;
}TX_Control;

static RX_Queue     RxQue;
static TX_Control   TxCtrl;

static BOOL RxQ_Push(uint8 Data);
static BOOL RxQ_Pop(uint8 * Data);
void UART_Init()
{
     ///// Hardware /////
  
     // Select P3.3 and P3.4 for UART use
     P3SEL |= BIT3 + BIT4;
     // Hold state machine in reset
     UCA0CTL1 |= UCSWRST;          
     // Select SMCLK
     UCA0CTL1 |= UCSSEL_2;                     
     // SMCLK = 4MHz
     // Desired Baud = 9600 bps
     // Using http://processors.wiki.ti.com/index.php/USCI_UART_Baud_Rate_Gen_Mode_Selection
     // UCxBR0UART = 160
     // UCxBR1UART = 1
     // UCxBRS = 6
     // UCxBRF = 0
     UCA0BR0 = 0xA0;                             
     UCA0BR1 = 0x01;    
     UCA0MCTL |= (UCBRS_3 + UCBRF_0);            
     // Release hold on state machine
     UCA0CTL1 &= ~UCSWRST;                   
     // Enable RX interrupr
     UCA0IE = UCRXIE; 
     
     ///// Software /////
     memset(&RxQue,      0, sizeof(RX_Queue));
     memset(&TxCtrl,     0, sizeof(TX_Control));
}

BOOL UART_Send(uint8 * Data, uint8 Length)
{
     // Make sure its not active
     if(!TxCtrl.Active)
     {
          // Signal that we should setup for transmission
          TxCtrl.Start = TRUE;
          // Save Data pointer
          TxCtrl.TxData = Data;
          // Save the length
          TxCtrl.DataLength = Length;
          // Set active to block other TX attempts
          TxCtrl.Active = TRUE;
          // Notify user of success
          return TRUE;
     }    
     return FALSE;
}

BOOL UART_GetRx(uint8 * Data)
{
     // Pop data from the RX queue
     return RxQ_Pop(Data);
}

void UART_Task(void)
{
     // Check to see if we need to setup a TX
     if(TxCtrl.Start)
     {
          // Clear start flag
          TxCtrl.Start = FALSE;
          // Point to first byte of data
          TxCtrl.NextTx = 0;
          // Make sure TX interrupts is clear (just for safety)
          UCA0IE &= ~UCTXIE;
          // Load the first byte into the register
          UCA0TXBUF = TxCtrl.TxData[TxCtrl.NextTx++];
          // Enable TX interrupts
          UCA0IE |= UCTXIE;
     }
}

BOOL UART_IsActive(void)
{
     return TxCtrl.Active;
}

static BOOL RxQ_Push(uint8 Data)
{
     // Check to see if there is room in the queue
     uint8 NextEmpty = (RxQue.NextEmpty + 1) & (RX_QUEUE - 1);
     if(NextEmpty != RxQue.Unload)
     {
          // Save data in queue
          RxQue.Queue[RxQue.NextEmpty] = Data;
          // Update tail pointer
          RxQue.NextEmpty = NextEmpty;
          // Notify user of success
          return TRUE;
     }
     // Operation failed
     return FALSE;
}
static BOOL RxQ_Pop(uint8 * Data)
{
     // Is there something in the queue
     if(RxQue.Unload != RxQue.NextEmpty)
     {
          // Pop data from head
          *Data = RxQue.Queue[RxQue.Unload];
          // Update head pointer
          RxQue.Unload = (RxQue.Unload + 1) & (RX_QUEUE - 1);
          // Notify user of success
          return TRUE;
     }
     // Operation failed
     return FALSE;
}


#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
     switch(__even_in_range(UCA0IV,4))
     {
          case NO_INTERRUPT :
               break;                             
          case RX_INTERRUPT : 
               {
                    uint8 data = UCA0RXBUF;
                    // Push RX data into queue
                    RxQ_Push(data);
                    break;
               }
          case TX_INTERRUPT :
               // Is there more data to TX/
               if(TxCtrl.DataLength > TxCtrl.NextTx)
               {
                    // If yes, load TX buffer
                    UCA0TXBUF = TxCtrl.TxData[TxCtrl.NextTx++];
               }
               else
               {
                    // If no, disable TX interrupts
                    UCA0IE &= ~UCTXIE;
                    // Signal done transmitting
                    TxCtrl.Active = FALSE;
               }
               break;                            
     }
}


