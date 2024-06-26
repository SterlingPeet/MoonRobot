/************************************************************************
 * NASA Docket No. GSC-18,719-1, and identified as “core Flight System: Bootes”
 *
 * Copyright (c) 2020 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ************************************************************************/

/**
 * @file
 *
 * Main header file for the ROMI Motor Driver application
 */

#ifndef ROMIMOT_H
#define ROMIMOT_H

/*
** Required header files.
*/
#include "cfe.h"
#include "cfe_error.h"
#include "cfe_evs.h"
#include "cfe_sb.h"
#include "cfe_es.h"

#include "romimot_perfids.h"
#include "romimot_msgids.h"
#include "romimot_msg.h"

/***********************************************************************/
#define ROMIMOT_PIPE_DEPTH 32 /* Depth of the Command Pipe for Application */

#define ROMIMOT_NUMBER_OF_TABLES 1 /* Number of Table(s) */

/* Define filenames of default data images for tables */
#define ROMIMOT_TABLE_FILE "/cf/romimot_tbl.tbl"

#define ROMIMOT_TABLE_OUT_OF_RANGE_ERR_CODE -1

#define ROMIMOT_TBL_ELEMENT_1_MAX 10
/************************************************************************
** Type Definitions
*************************************************************************/

/*
** Global Data
*/
typedef struct
{
    /*
    ** Command interface counters...
    */
    uint16 CmdCounter;
    uint8  ErrCounter;
    uint8  I2CErrCounter;

    /*
    ** Housekeeping telemetry packet...
    */
    ROMIMOT_HkTlm_t HkTlm;

    /*
    ** State output packet...
    */
    ROMIMOT_MotorState_t MotState;

    /*
    ** Run Status variable used in the main processing loop
    */
    uint32 RunStatus;

    /*
    ** Battery voltage expressed in millivolts
    */
    uint16 BatteryMillivolts;

    /*
     * Are the motors enabled
     */
    uint8 MotorsEnabled;

    /*
    ** Raw wheel encoder values, stored for next encoder delta calculation
    */
    int16 RawLeftEncoder;
    int16 RawRightEncoder;

    /*
    ** Change in encoder values since the last reading
    */
    int16 LeftEncoderDelta;
    int16 RightEncoderDelta;

    /*
    ** Absolute position of each motor since ROMIMOT app started
    */
    int32 LeftOdo;
    int32 RightOdo;

    /*
    ** Odometer intermediate targets
    */
    int32 LeftOdoStep;
    int32 RightOdoStep;

    /*
    ** Odometer targets
    */
    int32 LeftOdoTrgt;
    int32 RightOdoTrgt;

    /*
     * Motor speed settings
     */
    int16 LeftMotSpeed;
    int16 RightMotSpeed;

    /*
     * amount to increment/decrement the target to make the wheel turn at constant speed.
     */
    int16_t TargetDeltaLeft;
    int16_t TargetDeltaRight;

    /* file desccriptor for I2C bus */
    int i2cfd;

    /* I2C connection status flag */
    bool i2c_open;

    /*
    ** Operational data (not reported in housekeeping)...
    */
    CFE_SB_PipeId_t CommandPipe;

    /*
    ** Initialization data (not reported in housekeeping)...
    */
    char   PipeName[CFE_MISSION_MAX_API_LEN];
    uint16 PipeDepth;

    CFE_TBL_Handle_t TblHandles[ROMIMOT_NUMBER_OF_TABLES];
} ROMIMOT_Data_t;

/****************************************************************************/
/*
** Local function prototypes.
**
** Note: Except for the entry point (ROMIMOT_Main), these
**       functions are not called from any other source module.
*/
void  ROMIMOT_Main(void);
int32 ROMIMOT_Init(void);
int32 ROMIMOT_ConnectI2C(void);
void  ROMIMOT_ProcessCommandPacket(CFE_SB_Buffer_t *SBBufPtr);
void  ROMIMOT_ProcessGroundCommand(CFE_SB_Buffer_t *SBBufPtr);
int32 ROMIMOT_ReportHousekeeping(const CFE_MSG_CommandHeader_t *Msg);
int32 ROMIMOT_CheckI2CTransaction(int32 RetCode);
int32 ROMIMOT_Wakeup(const CFE_MSG_CommandHeader_t *Msg);
int32 ROMIMOT_ResetCounters(const ROMIMOT_ResetCountersCmd_t *Msg);
int32 ROMIMOT_Process(const ROMIMOT_ProcessCmd_t *Msg);
int32 ROMIMOT_SetMotEnable(const ROMIMOT_SetEnableCmd_t *Msg, uint8_t enable);
int32 ROMIMOT_SetTarget(const ROMIMOT_MotCmd_t *Msg);
int32 ROMIMOT_SetTargetDelta(const ROMIMOT_MotCmd_t *Msg);
int32 ROMIMOT_Noop(const ROMIMOT_NoopCmd_t *Msg);
void  ROMIMOT_GetCrc(const char *TableName);

int32 ROMIMOT_TblValidationFunc(void *TblData);

bool ROMIMOT_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength);

#endif /* ROMIMOT_H */
