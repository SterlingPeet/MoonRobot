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
 * Define ROMIMOT Messages and info
 */

#ifndef ROMIMOT_MSG_H
#define ROMIMOT_MSG_H

/*
** ROMIMOT command codes
*/
#define ROMIMOT_NOOP_CC             0
#define ROMIMOT_RESET_COUNTERS_CC   1
#define ROMIMOT_PROCESS_CC          2
#define ROMIMOT_MOT_ENABLE_CC       3
#define ROMIMOT_MOT_DISABLE_CC      4
#define ROMIMOT_SET_TARGET_CC       5 // uses ROMIMOT_MotCmd_t
#define ROMIMOT_SET_TARGET_DELTA_CC 6 // uses ROMIMOT_MotCmd_t

/*************************************************************************/

/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */
} ROMIMOT_NoArgsCmd_t;

/*
** Type definition for two-arg motor paramater setting commands
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */
    int16                   cmdMotLeft;
    int16                   cmdMotRight;
} ROMIMOT_MotCmd_t;

/*
** The following commands all share the "NoArgs" format
**
** They are each given their own type name matching the command name, which
** allows them to change independently in the future without changing the prototype
** of the handler function
*/
typedef ROMIMOT_NoArgsCmd_t ROMIMOT_NoopCmd_t;
typedef ROMIMOT_NoArgsCmd_t ROMIMOT_ResetCountersCmd_t;
typedef ROMIMOT_NoArgsCmd_t ROMIMOT_ProcessCmd_t;
typedef ROMIMOT_NoArgsCmd_t ROMIMOT_SetEnableCmd_t;

typedef ROMIMOT_MotCmd_t ROMIMOT_SetTargetCmd_t;
typedef ROMIMOT_MotCmd_t ROMIMOT_SetTargetDeltaCmd_t;

/*************************************************************************/
/*
** Type definition (ROMI Motor Driver App housekeeping)
*/

typedef struct
{
    uint16 CommandCounter;
    uint8  CommandErrorCounter;
    uint8  I2CErrorCounter;
    int16  LeftMotorEncoder;
    int16  RightMotorEncoder;
    uint16 BatteryMillivolts;
} ROMIMOT_HkTlm_Payload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief Telemetry header */
    ROMIMOT_HkTlm_Payload_t   Payload;         /**< \brief Telemetry payload */
} ROMIMOT_HkTlm_t;

#endif /* ROMIMOT_MSG_H */
