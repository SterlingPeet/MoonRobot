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
 *   This file contains the source code for the Differential Drive Forward Kinematics App.
 */

/*
** Include Files:
*/
#include "ddfk_app_events.h"
#include "ddfk_app_version.h"
#include "ddfk_app.h"
#include "ddfk_app_table.h"

#include <string.h>

/*
** global data
*/
DDFK_APP_Data_t DDFK_APP_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/*                                                                            */
/* Application entry point and main process loop                              */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void DDFK_APP_Main(void)
{
    int32            status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(DDFK_APP_PERF_ID);

    /*
    ** Perform application specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = DDFK_APP_Init();
    if (status != CFE_SUCCESS)
    {
        DDFK_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** DDFK_APP Runloop
    */
    while (CFE_ES_RunLoop(&DDFK_APP_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(DDFK_APP_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, DDFK_APP_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(DDFK_APP_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            DDFK_APP_ProcessCommandPacket(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(DDFK_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "DDFK_APP: SB Pipe Read Error, App Will Exit");

            DDFK_APP_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(DDFK_APP_PERF_ID);

    CFE_ES_ExitApp(DDFK_APP_Data.RunStatus);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* Initialization                                                             */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 DDFK_APP_Init(void)
{
    int32 status;

    DDFK_APP_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app command execution counters
    */
    DDFK_APP_Data.CmdCounter = 0;
    DDFK_APP_Data.ErrCounter = 0;

    /*
    ** Initialize app configuration data
    */
    DDFK_APP_Data.PipeDepth = DDFK_APP_PIPE_DEPTH;

    strncpy(DDFK_APP_Data.PipeName, "DDFK_APP_CMD_PIPE", sizeof(DDFK_APP_Data.PipeName));
    DDFK_APP_Data.PipeName[sizeof(DDFK_APP_Data.PipeName) - 1] = 0;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Differential Drive Forward Kinematics App: Error Registering Events, RC = 0x%08lX\n",
                             (unsigned long)status);
        return status;
    }

    /*
    ** Initialize housekeeping packet (clear user data area).
    */
    CFE_MSG_Init(CFE_MSG_PTR(DDFK_APP_Data.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(DDFK_APP_HK_TLM_MID),
                 sizeof(DDFK_APP_Data.HkTlm));

    /*
    ** Create Software Bus message pipe.
    */
    status = CFE_SB_CreatePipe(&DDFK_APP_Data.CommandPipe, DDFK_APP_Data.PipeDepth, DDFK_APP_Data.PipeName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Differential Drive Forward Kinematics App: Error creating pipe, RC = 0x%08lX\n",
                             (unsigned long)status);
        return status;
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(DDFK_APP_SEND_HK_MID), DDFK_APP_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog(
            "Differential Drive Forward Kinematics App: Error Subscribing to HK request, RC = 0x%08lX\n",
            (unsigned long)status);
        return status;
    }

    /*
    ** Subscribe to ground command packets
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(DDFK_APP_CMD_MID), DDFK_APP_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Differential Drive Forward Kinematics App: Error Subscribing to Command, RC = 0x%08lX\n",
                             (unsigned long)status);

        return status;
    }

    /*
    ** Register Table(s)
    */
    status = CFE_TBL_Register(&DDFK_APP_Data.TblHandles[0], "DDFKAppTable", sizeof(DDFK_APP_Table_t),
                              CFE_TBL_OPT_DEFAULT, DDFK_APP_TblValidationFunc);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Differential Drive Forward Kinematics App: Error Registering Table, RC = 0x%08lX\n",
                             (unsigned long)status);

        return status;
    }
    else
    {
        status = CFE_TBL_Load(DDFK_APP_Data.TblHandles[0], CFE_TBL_SRC_FILE, DDFK_APP_TABLE_FILE);
    }

    CFE_EVS_SendEvent(DDFK_APP_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION, "DDFK_APP Initialized.%s",
                      DDFK_APP_VERSION_STRING);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the DDFK_APP */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void DDFK_APP_ProcessCommandPacket(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case DDFK_APP_CMD_MID:
            DDFK_APP_ProcessGroundCommand(SBBufPtr);
            break;

        case DDFK_APP_SEND_HK_MID:
            DDFK_APP_ReportHousekeeping((CFE_MSG_CommandHeader_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(DDFK_APP_INVALID_MSGID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "DDFK_APP: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* DDFK_APP ground commands                                                 */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void DDFK_APP_ProcessGroundCommand(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process "known" DDFK_APP ground commands
    */
    switch (CommandCode)
    {
        case DDFK_APP_NOOP_CC:
            if (DDFK_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(DDFK_APP_NoopCmd_t)))
            {
                DDFK_APP_Noop((DDFK_APP_NoopCmd_t *)SBBufPtr);
            }

            break;

        case DDFK_APP_RESET_COUNTERS_CC:
            if (DDFK_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(DDFK_APP_ResetCountersCmd_t)))
            {
                DDFK_APP_ResetCounters((DDFK_APP_ResetCountersCmd_t *)SBBufPtr);
            }

            break;

        case DDFK_APP_PROCESS_CC:
            if (DDFK_APP_VerifyCmdLength(&SBBufPtr->Msg, sizeof(DDFK_APP_ProcessCmd_t)))
            {
                DDFK_APP_Process((DDFK_APP_ProcessCmd_t *)SBBufPtr);
            }

            break;

        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(DDFK_APP_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR, "Invalid ground command code: CC = %d",
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
int32 DDFK_APP_ReportHousekeeping(const CFE_MSG_CommandHeader_t *Msg)
{
    int i;

    /*
    ** Get command execution counters...
    */
    DDFK_APP_Data.HkTlm.Payload.CommandErrorCounter = DDFK_APP_Data.ErrCounter;
    DDFK_APP_Data.HkTlm.Payload.CommandCounter      = DDFK_APP_Data.CmdCounter;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(DDFK_APP_Data.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(DDFK_APP_Data.HkTlm.TelemetryHeader), true);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    for (i = 0; i < DDFK_APP_NUMBER_OF_TABLES; i++)
    {
        CFE_TBL_Manage(DDFK_APP_Data.TblHandles[i]);
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* DDFK_APP NOOP commands                                                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 DDFK_APP_Noop(const DDFK_APP_NoopCmd_t *Msg)
{
    DDFK_APP_Data.CmdCounter++;

    CFE_EVS_SendEvent(DDFK_APP_COMMANDNOP_INF_EID, CFE_EVS_EventType_INFORMATION, "DDFK_APP: NOOP command %s",
                      DDFK_APP_VERSION);

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 DDFK_APP_ResetCounters(const DDFK_APP_ResetCountersCmd_t *Msg)
{
    DDFK_APP_Data.CmdCounter = 0;
    DDFK_APP_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(DDFK_APP_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION, "DDFK_APP: RESET command");

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function Process Ground Station Command                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 DDFK_APP_Process(const DDFK_APP_ProcessCmd_t *Msg)
{
    int32             status;
    DDFK_APP_Table_t *TblPtr;
    const char       *TableName = "DDFK_APP.DDFKAppTable";

    /* Differential Drive Forward Kinematics Use of Table */

    status = CFE_TBL_GetAddress((void *)&TblPtr, DDFK_APP_Data.TblHandles[0]);

    if (status < CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Differential Drive Forward Kinematics App: Fail to get table address: 0x%08lx",
                             (unsigned long)status);
        return status;
    }

    CFE_ES_WriteToSysLog("Differential Drive Forward Kinematics App: Table Value 1: %d  Value 2: %d", TblPtr->Int1,
                         TblPtr->Int2);

    DDFK_APP_GetCrc(TableName);

    status = CFE_TBL_ReleaseAddress(DDFK_APP_Data.TblHandles[0]);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Differential Drive Forward Kinematics App: Fail to release table address: 0x%08lx",
                             (unsigned long)status);
        return status;
    }

    return CFE_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* Verify command packet length                                               */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool DDFK_APP_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
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

        CFE_EVS_SendEvent(DDFK_APP_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        DDFK_APP_Data.ErrCounter++;
    }

    return result;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Verify contents of First Table buffer contents                  */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 DDFK_APP_TblValidationFunc(void *TblData)
{
    int32             ReturnCode = CFE_SUCCESS;
    DDFK_APP_Table_t *TblDataPtr = (DDFK_APP_Table_t *)TblData;

    /*
    ** Differential Drive Forward Kinematics Table Validation
    */
    if (TblDataPtr->Int1 > DDFK_APP_TBL_ELEMENT_1_MAX)
    {
        /* First element is out of range, return an appropriate error code */
        ReturnCode = DDFK_APP_TABLE_OUT_OF_RANGE_ERR_CODE;
    }

    return ReturnCode;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* Output CRC                                                      */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void DDFK_APP_GetCrc(const char *TableName)
{
    int32          status;
    uint32         Crc;
    CFE_TBL_Info_t TblInfoPtr;

    status = CFE_TBL_GetInfo(&TblInfoPtr, TableName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Differential Drive Forward Kinematics App: Error Getting Table Info");
    }
    else
    {
        Crc = TblInfoPtr.Crc;
        CFE_ES_WriteToSysLog("Differential Drive Forward Kinematics App: CRC: 0x%08lX\n\n", (unsigned long)Crc);
    }
}
