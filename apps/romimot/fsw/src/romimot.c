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
 * \file
 *   This file contains the source code for the ROMI Motor Driver App.
 */

/*
** Include Files:
*/
#include "romimot_events.h"
#include "romimot_version.h"
#include "romimot.h"
#include "romimot_hw.h"
#include "romimot_table.h"

#include <string.h>

#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

/*
** global data
*/
ROMIMOT_Data_t ROMIMOT_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void ROMIMOT_Main(void)
{
    int32            status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(ROMIMOT_PERF_ID);

    /*
    ** Perform application specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = ROMIMOT_Init();
    if (status != CFE_SUCCESS)
    {
        ROMIMOT_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** ROMIMOT Runloop
    */
    while (CFE_ES_RunLoop(&ROMIMOT_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(ROMIMOT_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, ROMIMOT_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(ROMIMOT_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            ROMIMOT_ProcessCommandPacket(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(ROMIMOT_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ROMIMOT: SB Pipe Read Error, App Will Exit");

            ROMIMOT_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(ROMIMOT_PERF_ID);

    CFE_ES_ExitApp(ROMIMOT_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 ROMIMOT_Init(void)
{
    int32 status;

    ROMIMOT_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    ROMIMOT_Data.BatteryMillivolts = 0;

    ROMIMOT_Data.MotorsEnabled = 0;

    ROMIMOT_Data.RawLeftEncoder  = 0;
    ROMIMOT_Data.RawRightEncoder = 0;

    ROMIMOT_Data.LeftEncoderDelta = 0;
    ROMIMOT_Data.LeftEncoderDelta = 0;

    ROMIMOT_Data.LeftOdo  = 0;
    ROMIMOT_Data.RightOdo = 0;

    ROMIMOT_Data.LeftMotSpeed  = 0;
    ROMIMOT_Data.RightMotSpeed = 0;

    ROMIMOT_Data.LeftOdoStep  = 0;
    ROMIMOT_Data.RightOdoStep = 0;

    ROMIMOT_Data.LeftOdoTrgt  = 0;
    ROMIMOT_Data.RightOdoTrgt = 0;

    /*
     * Amount to increment/decrement the target to make the wheel turn at constant speed.
     */
    ROMIMOT_Data.TargetDeltaLeft  = 0;
    ROMIMOT_Data.TargetDeltaRight = 0;

    /*
    ** Initialize app command execution counters
    */
    ROMIMOT_Data.CmdCounter    = 0;
    ROMIMOT_Data.ErrCounter    = 0;
    ROMIMOT_Data.I2CErrCounter = 0;

    /*
    ** Initialize app configuration data
    */
    ROMIMOT_Data.PipeDepth = ROMIMOT_PIPE_DEPTH;

    strncpy(ROMIMOT_Data.PipeName, "ROMIMOT_CMD_PIPE", sizeof(ROMIMOT_Data.PipeName));
    ROMIMOT_Data.PipeName[sizeof(ROMIMOT_Data.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ROMI Motor Driver App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    /*
    ** Initialize housekeeping packet (clear user data area).
    */
    CFE_MSG_Init(CFE_MSG_PTR(ROMIMOT_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(ROMIMOT_HK_TLM_MID),
                 sizeof(ROMIMOT_Data.HkTlm));

    /*
    ** Initialize Motor State packet (clear user data area).
    */
    CFE_MSG_Init(CFE_MSG_PTR(ROMIMOT_Data.MotState.TelemetryHeader), CFE_SB_ValueToMsgId(ROMIMOT_STATE_MID),
                 sizeof(ROMIMOT_Data.MotState));

    /*
    ** Create Software Bus message pipe.
    */
    status = CFE_SB_CreatePipe(&ROMIMOT_Data.CommandPipe, ROMIMOT_Data.PipeDepth, ROMIMOT_Data.PipeName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ROMI Motor Driver App: Error creating pipe, RC = 0x%08lX\n", (unsigned long)status);
        return status;
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ROMIMOT_SEND_HK_MID), ROMIMOT_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ROMI Motor Driver App: Error Subscribing to HK request, RC = 0x%08lX\n",
                             (unsigned long)status);
        return status;
    }

    /*
    ** Subscribe to ground command packets
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ROMIMOT_CMD_MID), ROMIMOT_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ROMIMOT: Error Subscribing to Command, RC = 0x%08lX\n", (unsigned long)status);

        return status;
    }

    /*
    ** Subscribe to wakeup command packets
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ROMIMOT_WAKEUP_MID), ROMIMOT_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ROMIMOT: Error Subscribing to wakeup, RC = 0x%08lX\n", (unsigned long)status);

        return status;
    }

    /*
    ** Register Table(s)
    */
    status = CFE_TBL_Register(&ROMIMOT_Data.TblHandles[0], "RomimotTable", sizeof(ROMIMOT_Table_t), CFE_TBL_OPT_DEFAULT,
                              ROMIMOT_TblValidationFunc);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ROMI Motor Driver App: Error Registering Table, RC = 0x%08lX\n", (unsigned long)status);

        return status;
    }
    else
    {
        status = CFE_TBL_Load(ROMIMOT_Data.TblHandles[0], CFE_TBL_SRC_FILE, ROMIMOT_TABLE_FILE);
    }

    CFE_EVS_SendEvent(ROMIMOT_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION, "ROMIMOT Initialized.%s",
                      ROMIMOT_VERSION_STRING);

    // setup I2C

    ROMIMOT_Data.i2c_open = false;

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialize I2C Connection                                                  */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 ROMIMOT_ConnectI2C(void)
{

    if (ROMIMOT_Data.i2c_open == false)
    {
        // setup I2C

        bool fail_flag = false;
        char busname[20];
        snprintf(busname, 20, "/dev/i2c-%d", i2cBusNumber);
        ROMIMOT_Data.i2cfd = open_i2c_device(busname);

        if (ROMIMOT_Data.i2cfd < 0)
        {
            fail_flag = true;
            CFE_EVS_SendEvent(ROMIMOT_I2C_ERR_EID, CFE_EVS_EventType_ERROR, "ROMIMOT: Failed to open I2C bus %s",
                              busname);
            CFE_ES_WriteToSysLog("failed to open I2C bus %20s", busname);
        }

        if (ioctl(ROMIMOT_Data.i2cfd, I2C_SLAVE, romiaddr) < 0)
        {
            fail_flag = true;
            CFE_EVS_SendEvent(ROMIMOT_I2C_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ROMIMOT: Failed to select romi I2C device address 0x%X", romiaddr);
            CFE_ES_WriteToSysLog("failed to select romi I2C device");
        }

        if (fail_flag)
        {
            ROMIMOT_Data.I2CErrCounter++;
            return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        }

        CFE_EVS_SendEvent(ROMIMOT_I2C_ERR_EID, CFE_EVS_EventType_INFORMATION, "ROMIMOT: Opened I2C bus %s", busname);
        CFE_ES_WriteToSysLog("Opened I2C bus %s", busname);

        ROMIMOT_Data.i2c_open = true;
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the ROMIMOT */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void ROMIMOT_ProcessCommandPacket(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case ROMIMOT_CMD_MID:
            ROMIMOT_ProcessGroundCommand(SBBufPtr);
            break;

        case ROMIMOT_SEND_HK_MID:
            ROMIMOT_ReportHousekeeping((CFE_MSG_CommandHeader_t *)SBBufPtr);
            break;

        case ROMIMOT_WAKEUP_MID:
            ROMIMOT_Wakeup((CFE_MSG_CommandHeader_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(ROMIMOT_INVALID_MSGID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ROMIMOT: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* ROMIMOT ground commands                                                 */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void ROMIMOT_ProcessGroundCommand(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    CFE_EVS_SendEvent(ROMIMOT_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "ROMIMOT Process Ground Command called with CommandCode %x", CommandCode);

    /*
    ** Process "known" ROMIMOT ground commands
    */
    switch (CommandCode)
    {
        case ROMIMOT_NOOP_CC:
            if (ROMIMOT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ROMIMOT_NoopCmd_t)))
            {
                ROMIMOT_Noop((ROMIMOT_NoopCmd_t *)SBBufPtr);
            }

            break;

        case ROMIMOT_RESET_COUNTERS_CC:
            if (ROMIMOT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ROMIMOT_ResetCountersCmd_t)))
            {
                ROMIMOT_ResetCounters((ROMIMOT_ResetCountersCmd_t *)SBBufPtr);
            }

            break;

        case ROMIMOT_PROCESS_CC:
            if (ROMIMOT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ROMIMOT_ProcessCmd_t)))
            {
                ROMIMOT_Process((ROMIMOT_ProcessCmd_t *)SBBufPtr);
            }

            break;

        case ROMIMOT_MOT_ENABLE_CC:
            if (ROMIMOT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ROMIMOT_SetEnableCmd_t)))
            {
                if (ROMIMOT_ConnectI2C() == CFE_SUCCESS)
                {
                    ROMIMOT_SetMotEnable((ROMIMOT_SetEnableCmd_t *)SBBufPtr, true);
                }
            }
            break;
        case ROMIMOT_MOT_DISABLE_CC:
            if (ROMIMOT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ROMIMOT_SetEnableCmd_t)))
            {
                if (ROMIMOT_ConnectI2C() == CFE_SUCCESS)
                {
                    ROMIMOT_SetMotEnable((ROMIMOT_SetEnableCmd_t *)SBBufPtr, false);
                }
            }
            break;

        case ROMIMOT_SET_TARGET_CC:
            if (ROMIMOT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ROMIMOT_SetTargetCmd_t)))
            {
                if (ROMIMOT_ConnectI2C() == CFE_SUCCESS)
                {
                    ROMIMOT_SetTarget((ROMIMOT_SetTargetCmd_t *)SBBufPtr);
                }
            }
            break;

        case ROMIMOT_SET_TARGET_DELTA_CC:
            if (ROMIMOT_VerifyCmdLength(&SBBufPtr->Msg, sizeof(ROMIMOT_SetTargetDeltaCmd_t)))
            {
                if (ROMIMOT_ConnectI2C() == CFE_SUCCESS)
                {
                    ROMIMOT_SetTargetDelta((ROMIMOT_SetTargetDeltaCmd_t *)SBBufPtr);
                }
            }
            break;

        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(ROMIMOT_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
                              CommandCode);
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 ROMIMOT_ReportHousekeeping(const CFE_MSG_CommandHeader_t *Msg)
{
    int i;
    // CFE_EVS_SendEvent(ROMIMOT_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION, "ROMIMOT Report HK");
    /*
    ** Get command execution counters...
    */
    ROMIMOT_Data.HkTlm.Payload.CommandErrorCounter  = ROMIMOT_Data.ErrCounter;
    ROMIMOT_Data.HkTlm.Payload.CommandCounter       = ROMIMOT_Data.CmdCounter;
    ROMIMOT_Data.HkTlm.Payload.I2CErrorCounter      = ROMIMOT_Data.I2CErrCounter;
    ROMIMOT_Data.HkTlm.Payload.BatteryMillivolts    = ROMIMOT_Data.BatteryMillivolts;
    ROMIMOT_Data.HkTlm.Payload.MotorsEnabled        = ROMIMOT_Data.MotorsEnabled;
    ROMIMOT_Data.HkTlm.Payload.RawLeftMotorEncoder  = ROMIMOT_Data.RawLeftEncoder;
    ROMIMOT_Data.HkTlm.Payload.RawRightMotorEncoder = ROMIMOT_Data.RawRightEncoder;
    ROMIMOT_Data.HkTlm.Payload.LeftMotorOdometer    = ROMIMOT_Data.LeftOdo;
    ROMIMOT_Data.HkTlm.Payload.RightMotorOdometer   = ROMIMOT_Data.RightOdo;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(ROMIMOT_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(ROMIMOT_Data.HkTlm.TelemetryHeader), true);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    for (i = 0; i < ROMIMOT_NUMBER_OF_TABLES; i++)
    {
        CFE_TBL_Manage(ROMIMOT_Data.TblHandles[i]);
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* ROMIMOT Check I2C Transaction for Errors                                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 ROMIMOT_CheckI2CTransaction(int RetCode)
{
    int32 status = CFE_STATUS_EXTERNAL_RESOURCE_FAIL;

    if (RetCode == ROMIMOT_I2C_SETUP_WR_ERR_EID)
    {
        ROMIMOT_Data.I2CErrCounter++;
        CFE_EVS_SendEvent(ROMIMOT_I2C_ERR_EID, CFE_EVS_EventType_ERROR, "ROMIMOT: I2C setup write operation failed");
    }
    else if (RetCode == ROMIMOT_I2C_DAT_R_ERR_EID)
    {
        ROMIMOT_Data.I2CErrCounter++;
        CFE_EVS_SendEvent(ROMIMOT_I2C_ERR_EID, CFE_EVS_EventType_ERROR, "ROMIMOT: I2C data read operation failed");
    }
    else if (RetCode == ROMIMOT_I2C_DAT_W_ERR_EID)
    {
        ROMIMOT_Data.I2CErrCounter++;
        CFE_EVS_SendEvent(ROMIMOT_I2C_ERR_EID, CFE_EVS_EventType_ERROR, "ROMIMOT: I2C data write operation failed");
    }
    else if (RetCode != 0)
    {
        ROMIMOT_Data.I2CErrCounter++;
        CFE_EVS_SendEvent(ROMIMOT_I2C_ERR_EID, CFE_EVS_EventType_ERROR, "ROMIMOT: I2C [unkown] operation failed 0x%x",
                          RetCode);
    }
    else
    {
        status = CFE_SUCCESS;
    }

    return status;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* ROMIMOT Wakeup and service the hardware                                    */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 ROMIMOT_Wakeup(const CFE_MSG_CommandHeader_t *Msg)
{
    if (ROMIMOT_Data.i2c_open)
    {
        ROMIMOT_Data.CmdCounter++;
        int       i2c_ret = 0;
        uint8_t   buf[4];
        uint16    battVolts;
        MotorPair encVals;
        float     pcoeff = 0.05;

        // Read the buttons on the ROMI, emit an event if pressed.
        i2c_ret = romiRead(ROMIMOT_Data.i2cfd, 3, 3, buf);

        if (ROMIMOT_CheckI2CTransaction(i2c_ret) == CFE_SUCCESS)
        {
            if (buf[0])
            {
                CFE_EVS_SendEvent(ROMIMOT_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION, "ROMIMOT button");
            }
        }

        // Read the battery voltage on the ROMI, store it in the data struct.
        i2c_ret = romiRead(ROMIMOT_Data.i2cfd, 10, 2, (uint8_t *)&battVolts);
        if (ROMIMOT_CheckI2CTransaction(i2c_ret) == CFE_SUCCESS)
        {
            ROMIMOT_Data.BatteryMillivolts = battVolts;
        }

        // Read the motor encoders
        i2c_ret = romiEncoderRead(ROMIMOT_Data.i2cfd, &encVals);
        if (ROMIMOT_CheckI2CTransaction(i2c_ret) == CFE_SUCCESS)
        {
            ROMIMOT_Data.LeftEncoderDelta  = encVals.left - ROMIMOT_Data.RawLeftEncoder;
            ROMIMOT_Data.RightEncoderDelta = encVals.right - ROMIMOT_Data.RawRightEncoder;
            ROMIMOT_Data.RawLeftEncoder    = encVals.left;
            ROMIMOT_Data.RawRightEncoder   = encVals.right;

            ROMIMOT_Data.LeftOdo += ROMIMOT_Data.LeftEncoderDelta;
            ROMIMOT_Data.RightOdo += ROMIMOT_Data.RightEncoderDelta;
        }

        // Write state data for HK payload
        ROMIMOT_Data.MotState.Payload.MotorsEnabled      = ROMIMOT_Data.MotorsEnabled;
        ROMIMOT_Data.MotState.Payload.LeftPower          = ROMIMOT_Data.LeftMotSpeed;
        ROMIMOT_Data.MotState.Payload.RightPower         = ROMIMOT_Data.RightMotSpeed;
        ROMIMOT_Data.MotState.Payload.LeftEncoderDelta   = ROMIMOT_Data.LeftEncoderDelta;
        ROMIMOT_Data.MotState.Payload.RightEncoderDelta  = ROMIMOT_Data.RightEncoderDelta;
        ROMIMOT_Data.MotState.Payload.LeftMotorOdometer  = ROMIMOT_Data.LeftOdo;
        ROMIMOT_Data.MotState.Payload.RightMotorOdometer = ROMIMOT_Data.RightOdo;

        CFE_SB_TimeStampMsg(CFE_MSG_PTR(ROMIMOT_Data.MotState.TelemetryHeader));
        CFE_SB_TransmitMsg(CFE_MSG_PTR(ROMIMOT_Data.MotState.TelemetryHeader), true);

        if (ROMIMOT_Data.MotorsEnabled)
        {
            // Calculate the distance and direction between the Odo Step and the Odo target.
            // If the distance is greater than the target delta, move the Odo step
            // in the direction of the target by the target delta.  Otherwise
            // set the Odo step to the target.
            int32_t leftOdoDistance = ROMIMOT_Data.LeftOdoTrgt - ROMIMOT_Data.LeftOdoStep;
            int32_t leftDir         = leftOdoDistance > 0 ? 1 : -1;
            if (leftOdoDistance * leftDir > ROMIMOT_Data.TargetDeltaLeft)
            {
                ROMIMOT_Data.LeftOdoStep += ROMIMOT_Data.TargetDeltaLeft * leftDir;
            }
            else
            {
                ROMIMOT_Data.LeftOdoStep = ROMIMOT_Data.LeftOdoTrgt;
            }
            int32_t rightOdoDistance = ROMIMOT_Data.RightOdoTrgt - ROMIMOT_Data.RightOdoStep;
            int32_t rightDir         = rightOdoDistance > 0 ? 1 : -1;
            if (rightOdoDistance * rightDir > ROMIMOT_Data.TargetDeltaRight)
            {
                ROMIMOT_Data.RightOdoStep += ROMIMOT_Data.TargetDeltaRight * rightDir;
            }
            else
            {
                ROMIMOT_Data.RightOdoStep = ROMIMOT_Data.RightOdoTrgt;
            }

            // Calculate the speed of the motors based on the difference between
            // the Odo step and the Odo, and the proportional coefficient.
            ROMIMOT_Data.LeftMotSpeed  = pcoeff * (ROMIMOT_Data.LeftOdoStep - ROMIMOT_Data.LeftOdo);
            ROMIMOT_Data.RightMotSpeed = pcoeff * (ROMIMOT_Data.RightOdoStep - ROMIMOT_Data.RightOdo);

            // Cap speed at plus or minus 200
            if (ROMIMOT_Data.LeftMotSpeed > 200)
            {
                ROMIMOT_Data.LeftMotSpeed = 200;
            }
            if (ROMIMOT_Data.LeftMotSpeed < -200)
            {
                ROMIMOT_Data.LeftMotSpeed = -200;
            }
            if (ROMIMOT_Data.RightMotSpeed > 200)
            {
                ROMIMOT_Data.RightMotSpeed = 200;
            }
            if (ROMIMOT_Data.RightMotSpeed < -200)
            {
                ROMIMOT_Data.RightMotSpeed = -200;
            }

            printf("motor set L: %d %d %d, R: %d %d %d\n", ROMIMOT_Data.LeftMotSpeed, ROMIMOT_Data.LeftOdoStep,
                   ROMIMOT_Data.LeftOdoTrgt, ROMIMOT_Data.RightMotSpeed, ROMIMOT_Data.RightOdoStep,
                   ROMIMOT_Data.RightOdoTrgt);
            i2c_ret = romiMotorWrite(ROMIMOT_Data.i2cfd, ROMIMOT_Data.LeftMotSpeed, ROMIMOT_Data.RightMotSpeed);
            ROMIMOT_CheckI2CTransaction(i2c_ret);
        }
        else
        {
            i2c_ret = romiMotorWrite(ROMIMOT_Data.i2cfd, 0, 0);
            ROMIMOT_CheckI2CTransaction(i2c_ret);
        }
    }

    return CFE_SUCCESS;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* ROMIMOT NOOP commands                                                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 ROMIMOT_Noop(const ROMIMOT_NoopCmd_t *Msg)
{
    ROMIMOT_Data.CmdCounter++;

    CFE_EVS_SendEvent(ROMIMOT_COMMANDNOP_INF_EID, CFE_EVS_EventType_INFORMATION, "ROMIMOT: NOOP command %s",
                      ROMIMOT_VERSION);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 ROMIMOT_ResetCounters(const ROMIMOT_ResetCountersCmd_t *Msg)
{
    ROMIMOT_Data.CmdCounter    = 0;
    ROMIMOT_Data.ErrCounter    = 0;
    ROMIMOT_Data.I2CErrCounter = 0;

    CFE_EVS_SendEvent(ROMIMOT_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION, "ROMIMOT: RESET command");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function Process Ground Station Command                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 ROMIMOT_Process(const ROMIMOT_ProcessCmd_t *Msg)
{
    int32            status;
    ROMIMOT_Table_t *TblPtr;
    const char *     TableName = "ROMIMOT.RomimotTable";

    /* ROMI Motor Driver Use of Table */

    status = CFE_TBL_GetAddress((void *)&TblPtr, ROMIMOT_Data.TblHandles[0]);

    if (status < CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ROMI Motor Driver App: Fail to get table address: 0x%08lx", (unsigned long)status);
        return status;
    }

    CFE_ES_WriteToSysLog("ROMI Motor Driver App: Table Value 1: %d  Value 2: %d", TblPtr->Int1, TblPtr->Int2);

    ROMIMOT_GetCrc(TableName);

    status = CFE_TBL_ReleaseAddress(ROMIMOT_Data.TblHandles[0]);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ROMI Motor Driver App: Fail to release table address: 0x%08lx", (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}

int32 ROMIMOT_SetMotEnable(const ROMIMOT_SetEnableCmd_t *Msg, uint8_t enable)
{
    ROMIMOT_Data.MotorsEnabled = enable;

    CFE_EVS_SendEvent(ROMIMOT_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION, "ROMIMOT: Motor Enable command: %d",
                      enable);

    return CFE_SUCCESS;
}
int32 ROMIMOT_SetTarget(const ROMIMOT_SetTargetCmd_t *Msg)
{
    // ROMIMOT_Data.LeftMotSpeed  = Msg->cmdMotLeft;
    // ROMIMOT_Data.RightMotSpeed = Msg->cmdMotRight;
    ROMIMOT_Data.LeftOdoTrgt += Msg->cmdMotLeft;
    ROMIMOT_Data.RightOdoTrgt += Msg->cmdMotRight;

    CFE_EVS_SendEvent(ROMIMOT_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "ROMIMOT: Motor Target set (Absolute) : %d %d", ROMIMOT_Data.LeftOdoTrgt,
                      ROMIMOT_Data.RightOdoTrgt);
    return CFE_SUCCESS;
}
int32 ROMIMOT_SetTargetDelta(const ROMIMOT_SetTargetDeltaCmd_t *Msg)
{
    ROMIMOT_Data.TargetDeltaLeft  = Msg->cmdMotLeft;
    ROMIMOT_Data.TargetDeltaRight = Msg->cmdMotRight;

    CFE_EVS_SendEvent(ROMIMOT_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "ROMIMOT: Motor Target Delta Set : %d %d", ROMIMOT_Data.TargetDeltaLeft,
                      ROMIMOT_Data.TargetDeltaRight);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool ROMIMOT_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(ROMIMOT_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        ROMIMOT_Data.ErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify contents of First Table buffer contents                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 ROMIMOT_TblValidationFunc(void *TblData)
{
    int32            ReturnCode = CFE_SUCCESS;
    ROMIMOT_Table_t *TblDataPtr = (ROMIMOT_Table_t *)TblData;

    /*
    ** ROMI Motor Driver Table Validation
    */
    if (TblDataPtr->Int1 > ROMIMOT_TBL_ELEMENT_1_MAX)
    {
        /* First element is out of range, return an appropriate error code */
        ReturnCode = ROMIMOT_TABLE_OUT_OF_RANGE_ERR_CODE;
    }

    return ReturnCode;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Output CRC                                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void ROMIMOT_GetCrc(const char *TableName)
{
    int32          status;
    uint32         Crc;
    CFE_TBL_Info_t TblInfoPtr;

    status = CFE_TBL_GetInfo(&TblInfoPtr, TableName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ROMI Motor Driver App: Error Getting Table Info");
    }
    else
    {
        Crc = TblInfoPtr.Crc;
        CFE_ES_WriteToSysLog("ROMI Motor Driver App: CRC: 0x%08lX\n\n", (unsigned long)Crc);
    }
}
