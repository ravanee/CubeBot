#include "Comm.h"
#include "Types.h"
#include <string.h>

#include "RS232.h"

static void comm_RxTask(void);
static void comm_TxTask(void);
static void comm_DecodeRx(void);
static void comm_DispatchMessage(uint8 * Message, uint8 Length);
static uint8 comm_CalcChecksum(uint8 * Data, uint8 Length);

// General Message related structures
#define MESSAGE_HEADER1   (0xC6)  
#define MESSAGE_HEADER2   (0xA2) 

// Transmit related structures
#define MAX_TX_LENGTH    (128)
static uint8 TxMessage[MAX_TX_LENGTH];

// Decode related structures
#define MAX_RX_LENGTH    (128)
static uint8 RxMessage[MAX_RX_LENGTH];
typedef enum
{
     FIND_HEADER1,
     FIND_HEADER2,
     FIND_LENGTH,
     FIND_PAYLOAD,
     FIND_CHECKSUM
}DecodeStates;

typedef struct 
{
     DecodeStates   State;
     uint8          Length;
     uint8          DataDelay;
     uint8          Checksum;
}DecodeControl;

static DecodeControl DecodeCtrl;
static RxFunc RxHandlers[CMD_MAX_COMMAND];
/////////////////////////////

void COMM_Init(void)
{
     RS232_Init();  
     /////////////////////////////////////////////////////////
     DecodeCtrl.State = FIND_HEADER1;
     memset(&TxMessage,  0, sizeof(MAX_TX_LENGTH));
     memset(&RxMessage,  0, sizeof(MAX_RX_LENGTH));
     memset(&RxHandlers, 0, sizeof(RxFunc) * CMD_MAX_COMMAND);
}

BOOL COMM_Send(CommCommand Command, uint8 * Data, uint8 Length)
{
     // Check to see if RS232 is already active
     if(!RS232_IsActive())
     {
          enum
          {
               HEADER1,
               HEADER2,
               LENGTH,
               COMMAND,
               CHECKSUM,
               MESSAGE_OVERHEAD
          };
          #define DATA_START (COMMAND + 1)
          if((Length + MESSAGE_OVERHEAD) < MAX_TX_LENGTH)
          {
               // Stuff Header, Length, and Command into buffer
               TxMessage[HEADER1] = MESSAGE_HEADER1;
               TxMessage[HEADER2] = MESSAGE_HEADER2;
               TxMessage[LENGTH] = Length + 1;
               TxMessage[COMMAND] = (uint8) Command;
               
               // Copy data into rest of buffer
               memcpy(&(TxMessage[DATA_START]), Data, Length); 
               
               // Calculate checksum
               TxMessage[(Length + CHECKSUM)] = comm_CalcChecksum(TxMessage, Length + 4); // 4 = Header1, Header2, Length, Command
               
               // See if RS232 will accept
               return RS232_Send(TxMessage, (Length + MESSAGE_OVERHEAD));
          }
     }
     // Opereation failed
     return FALSE;
}

void COMM_Task(void)
{
     // Give CPU time to Rx and Tx communication tasks
     comm_RxTask();
     comm_TxTask();
}

void COMM_RegisterHandler(CommCommand Command, RxFunc Handler)
{
     // Make sure this is a valid command to register
     if(Command < CMD_MAX_COMMAND)
     {
          // Save handler
          RxHandlers[Command] = Handler;
     }
}
void COMM_UnregisterHandler(CommCommand Command)
{
     // Make sure this is a valid command to register
     if(Command < CMD_MAX_COMMAND)
     {
          // Save handler
          RxHandlers[Command] = NULL;
     }
}

static void comm_RxTask(void)
{
     // Decode any information off RS232 bus
     comm_DecodeRx();
}

static void comm_TxTask(void)
{
     // Let RS232 run
     RS232_Task();
}
     
static void comm_DecodeRx(void)
{
     uint8 rxData;
     // Does RS232 have unread data?
     if(RS232_GetRx(&rxData))
     {
          // Where are we in the decode state machine?
           switch(DecodeCtrl.State)
          {
               case FIND_HEADER1 :
                    // Does header1 match?
                    if(rxData == MESSAGE_HEADER1)
                    {
                         DecodeCtrl.State = FIND_HEADER2;
                    }
                    break;
               case FIND_HEADER2 :
                    // Does header2 match?
                    if(rxData == MESSAGE_HEADER2)
                    {
                         DecodeCtrl.State = FIND_LENGTH;
                    }
                    else
                    {
                         DecodeCtrl.State = FIND_HEADER1;
                    }
                    break;
               case FIND_LENGTH :
                    // Is the message too long or too short?
                    if((rxData > MAX_RX_LENGTH) || (rxData > 0))
                    {
                        DecodeCtrl.State = FIND_HEADER1;
                    }
                    else
                    {
                         DecodeCtrl.Length = rxData;
                         DecodeCtrl.DataDelay = 0;
                         DecodeCtrl.State = FIND_PAYLOAD;
                    }
                    break;
               case FIND_PAYLOAD :
                    // Collect command and data
                    // Length can never be zero
                    if(DecodeCtrl.DataDelay < DecodeCtrl.Length)
                    {
                         RxMessage[DecodeCtrl.DataDelay] = rxData;
                         DecodeCtrl.DataDelay++;
                         DecodeCtrl.State = (DecodeCtrl.DataDelay < DecodeCtrl.Length) ? FIND_PAYLOAD : FIND_CHECKSUM;
                    }
                    break;
               case FIND_CHECKSUM :
                    {
                         uint8 index;
                         uint8 checksum = 0;
                         checksum = checksum + MESSAGE_HEADER1;
                         checksum = checksum + MESSAGE_HEADER2;
                         checksum = checksum + DecodeCtrl.Length;
                         for(index = 0; index < DecodeCtrl.Length; index++)
                         {
                              checksum = checksum + RxMessage[index];
                         }
                         checksum = (~checksum);
                         if(checksum == rxData)
                         {
                              comm_DispatchMessage(RxMessage, DecodeCtrl.Length);
                         }
                         // Start searching again
                         DecodeCtrl.State = FIND_HEADER1;
                    }
                    break;
          }
     }
}

static uint8 comm_CalcChecksum(uint8 * Data, uint8 Length)
{
     uint8 index;
     uint8 checksum = 0;
     for(index = 0; index < Length; index++)
     {
          checksum = checksum + Data[index];
     }
     return (~checksum);
}

static void comm_DispatchMessage(uint8 * Message, uint8 Length)
{
     CommCommand command = (CommCommand) Message[0];
     if(command < CMD_MAX_COMMAND)
     {
          RxFunc handler = RxHandlers[command];
          if(handler)
          {
               handler(Message, Length);
          }
     }
}
     