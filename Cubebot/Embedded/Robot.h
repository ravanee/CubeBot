#ifndef _ROBOT_H_
#define _ROBOT_H_
#include "Types.h"

typedef enum
{
     Left,
     Right,
     ARM_MAX
}ArmId;

typedef enum
{
     FACE_CW,
     FACE_CCW,
     CUBE_CW,
     CUBE_CCW,
     ACTION_MAX
}ArmAction;

typedef enum
{
     D0,
     D90,
     D180,
     ROTATE_MAX
}ArmRotate;

void ROBOT_Init(void);
void ROBOT_Task(void);
BOOL ROBOT_IssueCommand(ArmId Arm, ArmAction Action, ArmRotate Rotate);
BOOL ROBOT_IsBusy(void);
void ROBOT_SerialDecode(uint8 * Message, uint8 Length);
void ROBOT_Config(uint8 * Message, uint8 Length);
#endif