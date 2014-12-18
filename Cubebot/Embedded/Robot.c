#include <msp430f5529.h>
#include "Robot.h"
#include "Types.h"
#include "Comm.h"
#include "RobotArm.h"
#include "StepperController.h"
#include "Stepper.h"
#include "EasyDriver.h"
#include "DigitalIO.h"
#include "Comm.h"
#include "Timer.h"

//#define __USE_INVERTED_90

#ifdef __USE_INVERTED_90
     #define OPEN_SWING       (45.0f)
#else
     #define OPEN_SWING       (18.0f)
#endif

#define ALIGN_SWING      (10.0f)

//////////// EASY DRIVER OBJECTS ////////////
static EzControl LeftRotateEz = 
{
     .Step = 
     {
          .Port = IO_PORT4,
          .Pin = IO_PIN3
     },
     .Direction = 
     {
          .Port = IO_PORT4,
          .Pin = IO_PIN0
     }
};


static EzControl RightRotateEz = 
{
     .Step = 
     {
          .Port = IO_PORT1,
          .Pin = IO_PIN3
     },
     .Direction = 
     {
          .Port = IO_PORT1,
          .Pin = IO_PIN2
     }
};

//////////// STEPPER MOTOR OBJECTS ////////////
static Stepper LeftRotateStep = 
{
     .EzDriver = &LeftRotateEz
};

static Stepper RightRotateStep = 
{
     .EzDriver = &RightRotateEz
};

//////////// STEPPER CONTROLLER OBJECTS ////////////
static SteprCtrl LeftRotaterCtrl = 
{
     .Direction = STEPPER_CW,
     .Stepper = &LeftRotateStep,
     .NumOfSteps = 0,
     .Timer = TIMER_LEFT_ROTATE
};

static ServoCtrl LeftClawCtrl = 
{
     .Pwm = TA_CCR3,
     .Center = 1500, 
     .IsBusy = FALSE,
     .Timer = TIMER_LEFT_CLAW
};

static SteprCtrl RightRotaterCtrl = 
{
     .Direction = STEPPER_CW,
     .Stepper = &RightRotateStep,
     .NumOfSteps = 0,
     .Timer = TIMER_RIGHT_ROTATE
};

static ServoCtrl RightClawCtrl = 
{
     .Pwm = TA_CCR4,
     .Center = 1500, 
     .IsBusy = FALSE,
     .Timer = TIMER_RIGHT_CLAW
};

//////////// ROBOT ARM OBJECTS ////////////
static RobotArm LeftArm = 
{
     .Rotater = &LeftRotaterCtrl,
     .Claw = &LeftClawCtrl,
     .ClawOpen = FALSE
};

static RobotArm RightArm = 
{
     .Rotater = &RightRotaterCtrl,
     .Claw = &RightClawCtrl,
     .ClawOpen = FALSE
};

//////////////////////////////////////////
typedef enum
{
     CW,
     CCW,
     Open,
     Close,
     OpenAlign,
     CloseAlign
}InternAction;

typedef struct
{
     ArmId          Arm;
     InternAction   Action;
     ArmRotate      Rotate;
}RobotCommand;

#define CMD_QUEUE_LEN    (32)
RobotCommand CmdQueue[CMD_QUEUE_LEN];

typedef struct
{
     RobotCommand * Queue;
     uint8 NextEmpty;
     uint8 Unload;
}CommandQueue;

typedef struct
{
     BOOL           Busy;
     RobotCommand   CurrentCmd;
     RobotArm  *    LeftArm;
     RobotArm  *    RightArm;
     CommCommand    RespondCmd;
}RobotControl;

static CommandQueue CmdQ = 
{
     .Queue = CmdQueue,
     .NextEmpty = 0,
     .Unload = 0
};

static RobotControl Robot = 
{
     .Busy = FALSE,
     .CurrentCmd = {Left, Open, D90},
     .LeftArm = &LeftArm,
     .RightArm = &RightArm,
     .RespondCmd = CMD_ROBOT
};

static BOOL CmdQ_Push(RobotCommand Data);
static BOOL CmdQ_Pop(RobotCommand * Data);
static void GenerateSequenceFace(RobotCommand Cmd);
static void GenerateSequenceCube(RobotCommand Cmd);
static BOOL ValidateParameters(ArmId Arm, ArmAction Action, ArmRotate Rotate);
static BOOL AreArmsBusy(void);
static void AlignCube(void); 
static void GenerateOpenAlign(void);
static void GenerateCloseAlign(void);

void ROBOT_Init(void)
{   
     // Set Easy Driver Pins as output
     // Left Rotate Step
     P4DIR |= BIT3;
     // Left Rotate Dir
     P4DIR |= BIT0; 
     // Right Rotate Step
     P1DIR |= BIT3;
     // Right Rotate Dir
     P1DIR |= BIT2;
     
     // Right Claw Timer output 
     P1DIR |= BIT5;
     // Right Claw Timer peripheral as PWM
     P1SEL |= BIT5;
     
     // Left Claw Timer output 
     P1DIR |= BIT4;
     // Left Claw Timer peripheral as PWM
     P1SEL |= BIT4;
          
     // Start PWM on these CCRs
     TIMER_StartCCR(TA_CCR3);
     TIMER_StartCCR(TA_CCR4);
     
     // First Open the Claw
     GenerateOpenAlign();
     Robot.Busy = TRUE;
    
     // Initialize Serial Communication Receive
     COMM_RegisterHandler(CMD_ROBOT,         ROBOT_SerialDecode); 
     COMM_RegisterHandler(CMD_ROBOT_CONFIG,  ROBOT_Config); 
}

void ROBOT_Task(void)
{
     // Only go into task if there is something to do
     if(Robot.Busy)
     {
          // Check the progress of the arms
          if(!AreArmsBusy())
          {
               // Get a new command
               if(CmdQ_Pop(&Robot.CurrentCmd))
               {
                    // See what arm we are dealing with
                    RobotArm * arm;
                    switch(Robot.CurrentCmd.Arm)
                    {
                         case Left :
                              arm = Robot.LeftArm;
                              break;
                         case Right :
                              arm = Robot.RightArm;
                              break;
                    }
                    // See what action we need
                    switch(Robot.CurrentCmd.Action)
                    {
                         case CW :
                              {
                                   // Clockwise is +degrees
                                   float degrees = (90.0f) * Robot.CurrentCmd.Rotate;
                                   // Rotate the arm
                                   ARM_Rotate(arm, degrees);
                              }
                              break;
                         case CCW :
                              {
                                   // Counter clockwise is -degrees
                                   float degrees = (-90.0f) * Robot.CurrentCmd.Rotate;
                                   // Rotate the arm
                                   ARM_Rotate(arm, degrees);
                              }
                              break;
                         case Open :
                              // Open the arm's claw
                              ARM_OpenClaw(arm, OPEN_SWING);
                              break;
                         case Close :
                              // Close the arm's claw
                              ARM_CloseClaw(arm, OPEN_SWING);
                              break;
                         case OpenAlign :
                              // Open the arm's claw
                              ARM_OpenClaw(arm, ALIGN_SWING);
                              break;
                         case CloseAlign :
                              // Close the arm's claw
                              ARM_CloseClaw(arm, ALIGN_SWING);
                              break;

                    }
               }
               else
               {
                    // Done with serial command
                    // Ack the command __RAVE__ may change message
                    uint8 ack = 0x01;
                    COMM_Send(Robot.RespondCmd, &ack, 1);
                    // Allow the robot to accept next command
                    Robot.Busy = FALSE;
               }
          }
     }
}

BOOL ROBOT_IssueCommand(ArmId Arm, ArmAction Action, ArmRotate Rotate)
{
     // Only accept a command if we aren't doing something already
     if(!Robot.Busy)
     {
          RobotCommand cmd;
          // Make sure the values make sense
          if(ValidateParameters(Arm, Action, Rotate))
          {
               // Save values
               cmd.Arm = Arm;
               cmd.Rotate = Rotate;
               switch(Action)
               {
                    case FACE_CW :
                         cmd.Action = CW;
                         GenerateSequenceFace(cmd);
                         break;
                    case FACE_CCW :
                         cmd.Action = CCW;
                         GenerateSequenceFace(cmd);
                         break;
                    case CUBE_CW :
                         cmd.Action = CW;
                         GenerateSequenceCube(cmd);
                         break;
                    case CUBE_CCW :
                         cmd.Action = CCW;
                         GenerateSequenceCube(cmd);
                         break;
               }

               // Set robot as busy
               Robot.Busy = TRUE;
               return TRUE;
          }
     }
     return FALSE;
}

BOOL ROBOT_IsBusy(void)
{
     // Return true if robot is currently moving
     return (Robot.Busy);
}

void ROBOT_SerialDecode(uint8 * Message, uint8 Length)
{
     // Is the message valid? (not null)
     if(Message)
     {
          // If the robot is busy, nak the message
          if(Robot.Busy)
          {
               uint8 nack = 0x00;
               COMM_Send((CommCommand)Message[0], &nack, 1); 
          }
          else
          {
               // Otherwise issue the command
               if(ROBOT_IssueCommand((ArmId) Message[1], (ArmAction) Message[2], (ArmRotate) Message[3]))
               {
                    Robot.RespondCmd = CMD_ROBOT;
               }
               else
               {
                    // If we get here, most likely not a valid command (failed due to function ValidateParameters)
                    uint8 nack = 0x00;
                    COMM_Send(CMD_ROBOT, &nack,  sizeof(nack));
               }
          }
     }
}
                        
void ROBOT_Config(uint8 * Message, uint8 Length)
{
     // Is the message valid? (not null)
     if(Message)
     {
          // If the robot is busy, nak the message
          if(Robot.Busy)
          {
               uint8 nack = 0x00;
               COMM_Send((CommCommand)Message[0], &nack, sizeof(nack)); 
          }
          else
          {
               // If not busy, issue the command
               switch((ArmId)Message[1])
               {
                    case Left :
                         switch(Message[2])
                         {
                              case 0 : // Rotater
                                   if(Message[3] == 0)
                                   {
                                        // Clockwise or Counter clockwise
                                        int8 sign = (int8)Message[4]; // -1 or 1
                                        // Get Steps
                                        uint16 value = ((uint16)Message[5] << 8) | ((uint16)Message[6]);
                                        // Convert to angle
                                        float angle = (float)sign * (((float)value / (float)STEPPER_StepsPerRev()) * 360.0f);
                                        // Move stepper
                                        STPCTL_Rotate(&LeftRotaterCtrl, angle);
                                        // Respond to command
                                        Robot.RespondCmd = CMD_ROBOT_CONFIG;
                                        // Tell robot to go
                                        Robot.Busy = TRUE;
                                   }
                                   break;
                              case 1 : // Claw
                                   if(Message[3] == 0)
                                   {
                                        // Get Pulse Width in us
                                        uint16 value = ((uint16)Message[5] << 8) | ((uint16)Message[6]);
                                        // Set as Center for this serv0
                                        SERVO_SetCenter(&LeftClawCtrl, value);
                                        // Respond to command
                                        Robot.RespondCmd = CMD_ROBOT_CONFIG;
                                        // Setting this does not require movement
                                        Robot.Busy = FALSE; 
                                   }
                                   else if(Message[3] == 1)
                                   {
                                        // Get Pulse Width in us
                                        uint16 position = ((uint16)Message[5] << 8) | ((uint16)Message[6]);
                                        // Move to that absolute angle (center +/- the PW angle)
                                        SERVO_SetPosition(&LeftClawCtrl, position);
                                        // Respond to command
                                        Robot.RespondCmd = CMD_ROBOT_CONFIG;
                                        // Tell robot to g
                                        Robot.Busy = TRUE;
                                   }
                                   break;
                         }
                         break;
                    case Right :
                         switch(Message[2])
                         {
                              case 0 : // Rotater
                                   STPCTL_Rotate(&RightRotaterCtrl, (Message[3]) ? (-1.0f * (360.0f / (STEPPER_StepsPerRev()))) : (360.0f / (STEPPER_StepsPerRev())));
                                   Robot.RespondCmd = CMD_ROBOT_CONFIG;
                                   Robot.Busy = TRUE;
                                   break;
                              case 1 : // Claw
                                   if(Message[3] == 0)
                                   {
                                        uint16 value = ((uint16)Message[5] << 8) | ((uint16)Message[6]);
                                        SERVO_SetCenter(&RightClawCtrl, value);
                                        Robot.RespondCmd = CMD_ROBOT_CONFIG;
                                        Robot.Busy = FALSE; // Setting this does not require movement
                                   }
                                   else if(Message[3] == 1)
                                   {
                                        uint16 position = ((uint16)Message[5] << 8) | ((uint16)Message[6]);
                                        SERVO_SetPosition(&RightClawCtrl, position);
                                        Robot.RespondCmd = CMD_ROBOT_CONFIG;
                                        Robot.Busy = TRUE;
                                   }
                                   break;
                         }
                         break;
                    case 3 : // Open and Close Align
                         {
                              if(Message[2] == 0)
                              {
                                   GenerateOpenAlign();
                              }
                              else if(Message[2] == 1)
                              {
                                   GenerateCloseAlign();
                              } 
                         }
                         break;
                    default :
                         {
                              uint8 nack = 0x00;
                              COMM_Send(CMD_ROBOT_CONFIG, &nack, 1);
                         }
                         break;
               }
          }
     }
}

static BOOL CmdQ_Push(RobotCommand Data)
{
     // Check to see if there is room in the queue
     uint8 NextEmpty = (CmdQ.NextEmpty + 1) & (CMD_QUEUE_LEN - 1);
     if(NextEmpty != CmdQ.Unload)
     {
          // Save data in queue
          CmdQ.Queue[CmdQ.NextEmpty] = Data;
          // Update tail pointer
          CmdQ.NextEmpty = NextEmpty;
          // Notify user of success
          return TRUE;
     }
     // Operation failed
     return FALSE;
}

static BOOL CmdQ_Pop(RobotCommand * Data)
{
     // Is there something in the queue
     if(CmdQ.Unload != CmdQ.NextEmpty)
     {
          // Pop data from head
          *Data = CmdQ.Queue[CmdQ.Unload];
          // Update head pointer
          CmdQ.Unload = (CmdQ.Unload + 1) & (CMD_QUEUE_LEN - 1);
          // Notify user of success
          return TRUE;
     }
     // Operation failed
     return FALSE;
}


static void GenerateSequenceFace(RobotCommand Cmd)
{
     RobotCommand tempCmd;
#ifdef __USE_INVERTED_90     
     if(Cmd.Rotate == D90)
     {
          tempCmd.Arm = (Cmd.Arm == Left) ? Right : Left;
          tempCmd.Action = Open;
          tempCmd.Rotate = D0; // Don't care
          CmdQ_Push(tempCmd);
          
          tempCmd.Arm = (Cmd.Arm == Left) ? Right : Left;
          tempCmd.Action = CW;
          tempCmd.Rotate = D180; // Don't care
          CmdQ_Push(tempCmd);
       
          tempCmd.Arm = (Cmd.Arm == Left) ? Right : Left;
          tempCmd.Action = Close;
          tempCmd.Rotate = D0; // Don't care
          CmdQ_Push(tempCmd);
     }
#endif
     
     // First open the claw
     tempCmd.Arm = Cmd.Arm;
     tempCmd.Action = Open;
     tempCmd.Rotate = D0; // Don't care
     CmdQ_Push(tempCmd);
     
     // Rotate opposite direction;
     tempCmd.Arm = Cmd.Arm;
     tempCmd.Action = (Cmd.Action == CW) ? CCW : CW;;
     tempCmd.Rotate = Cmd.Rotate; 
     CmdQ_Push(tempCmd);
     
     // Close the claw
     tempCmd.Arm = Cmd.Arm;
     tempCmd.Action = Close;
     tempCmd.Rotate = D0; 
     CmdQ_Push(tempCmd);
     
     // Rotate correct direction
     tempCmd.Arm = Cmd.Arm;
     tempCmd.Action = Cmd.Action;
     tempCmd.Rotate = Cmd.Rotate; 
     CmdQ_Push(tempCmd);
     
#ifdef __USE_INVERTED_90     
     if(Cmd.Rotate == D90)
     {
          tempCmd.Arm = (Cmd.Arm == Left) ? Right : Left;
          tempCmd.Action = Open;
          tempCmd.Rotate = D0; // Don't care
          CmdQ_Push(tempCmd);
          
          tempCmd.Arm = (Cmd.Arm == Left) ? Right : Left;
          tempCmd.Action = CCW;
          tempCmd.Rotate = D180; // Don't care
          CmdQ_Push(tempCmd);
       
          tempCmd.Arm = (Cmd.Arm == Left) ? Right : Left;
          tempCmd.Action = Close;
          tempCmd.Rotate = D0; // Don't care
          CmdQ_Push(tempCmd);
     }
#endif
     
     AlignCube();
     // The robot will be returned to home after this sequence is performed
}

static void GenerateSequenceCube(RobotCommand Cmd)
{
     RobotCommand tempCmd;
     
#ifdef __USE_INVERTED_90    
     if(Cmd.Rotate == D90)
     {
          tempCmd.Arm = (Cmd.Arm == Left) ? Right : Left;
          tempCmd.Action = Open;
          tempCmd.Rotate = D0; // Don't care
          CmdQ_Push(tempCmd);
          
          tempCmd.Arm = (Cmd.Arm == Left) ? Right : Left;
          tempCmd.Action = CW;
          tempCmd.Rotate = D180;
          CmdQ_Push(tempCmd);
       
          tempCmd.Arm = (Cmd.Arm == Left) ? Right : Left;
          tempCmd.Action = Close;
          tempCmd.Rotate = D0; // Don't care
          CmdQ_Push(tempCmd);
     }
#endif
     
     // First Open the Claw
     tempCmd.Arm = Cmd.Arm;
     tempCmd.Action = Open;
     tempCmd.Rotate = D0; // Don't care
     CmdQ_Push(tempCmd);
     
     // Rotate opposite direction
     tempCmd.Arm = Cmd.Arm;
     tempCmd.Action = (Cmd.Action == CW) ? CCW : CW;
     tempCmd.Rotate = Cmd.Rotate;
     CmdQ_Push(tempCmd);
     
     // Close claw
     tempCmd.Arm = Cmd.Arm;
     tempCmd.Action = Close;
     tempCmd.Rotate = D0;
     CmdQ_Push(tempCmd);
     
     // Open opposite claw
     tempCmd.Arm = (Cmd.Arm == Left) ? Right : Left;
     tempCmd.Action = Open;
     tempCmd.Rotate = D0;
     CmdQ_Push(tempCmd);
     
     // Rotate correct direction
     tempCmd.Arm = Cmd.Arm;
     tempCmd.Action = Cmd.Action;
     tempCmd.Rotate = Cmd.Rotate;
     CmdQ_Push(tempCmd);
     
     // Close opposite claw
     tempCmd.Arm = (Cmd.Arm == Left) ? Right : Left;;
     tempCmd.Action = Close;
     tempCmd.Rotate = D0;
     CmdQ_Push(tempCmd);

#ifdef __USE_INVERTED_90     
     if(Cmd.Rotate == D90)
     {
          tempCmd.Arm = (Cmd.Arm == Left) ? Right : Left;
          tempCmd.Action = Open;
          tempCmd.Rotate = D0; // Don't care
          CmdQ_Push(tempCmd);
          
          tempCmd.Arm = (Cmd.Arm == Left) ? Right : Left;
          tempCmd.Action = CCW;
          tempCmd.Rotate = D180;
          CmdQ_Push(tempCmd);
       
          tempCmd.Arm = (Cmd.Arm == Left) ? Right : Left;
          tempCmd.Action = Close;
          tempCmd.Rotate = D0; // Don't care
          CmdQ_Push(tempCmd);
     }
#endif 
     
     AlignCube();
     // The robot will be returned to home after this sequence is performed
}
   
static void AlignCube(void)
{
     GenerateOpenAlign();
     GenerateCloseAlign();
}

static void GenerateOpenAlign(void)
{
     RobotCommand tempCmd;
     
     // First Open the Claw
     tempCmd.Arm = Left;
     tempCmd.Action = OpenAlign;
     tempCmd.Rotate = D0; // Don't care
     CmdQ_Push(tempCmd);
     
     tempCmd.Arm = Right;
     tempCmd.Action = OpenAlign;
     tempCmd.Rotate = D0; // Don't care
     CmdQ_Push(tempCmd);
}

static void GenerateCloseAlign(void)
{
     RobotCommand tempCmd;
     
     tempCmd.Arm = Left;
     tempCmd.Action = CloseAlign;
     tempCmd.Rotate = D0; // Don't care
     CmdQ_Push(tempCmd);
     
     tempCmd.Arm = Right;
     tempCmd.Action = CloseAlign;
     tempCmd.Rotate = D0; // Don't care
     CmdQ_Push(tempCmd);
}

static BOOL ValidateParameters(ArmId Arm, ArmAction Action, ArmRotate Rotate)
{
     return ((Arm < ARM_MAX) && (Action < ACTION_MAX) && (Rotate < ROTATE_MAX));
}

static BOOL AreArmsBusy(void)
{
     // See if the arms are currently moving
     return (ARM_IsActive(Robot.LeftArm) || ARM_IsActive(Robot.RightArm));
}
