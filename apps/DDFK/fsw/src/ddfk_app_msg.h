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
 * Define DDFK_APP Messages and info
 */

#ifndef DDFK_APP_MSG_H
#define DDFK_APP_MSG_H

/*
** DDFK_APP command codes
*/
#define DDFK_APP_NOOP_CC           0
#define DDFK_APP_RESET_COUNTERS_CC 1
#define DDFK_APP_PROCESS_CC        2

/*************************************************************************/

/*
** Type definition (generic "no arguments" command)
*/
typedef struct
{
    CFE_MSG_CommandHeader_t CmdHeader; /**< \brief Command header */
} DDFK_APP_NoArgsCmd_t;

/*
** The following commands all share the "NoArgs" format
**
** They are each given their own type name matching the command name, which
** allows them to change independently in the future without changing the prototype
** of the handler function
*/
typedef DDFK_APP_NoArgsCmd_t DDFK_APP_NoopCmd_t;
typedef DDFK_APP_NoArgsCmd_t DDFK_APP_ResetCountersCmd_t;
typedef DDFK_APP_NoArgsCmd_t DDFK_APP_ProcessCmd_t;

/*************************************************************************/
/*
** Type definition (Differential Drive Forward Kinematics App housekeeping)
*/

typedef struct
{
    uint8 CommandErrorCounter;
    uint8 CommandCounter;
    uint8 spare[2];
} DDFK_APP_HkTlm_Payload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader; /**< \brief Telemetry header */
    DDFK_APP_HkTlm_Payload_t  Payload;         /**< \brief Telemetry payload */
} DDFK_APP_HkTlm_t;

#endif /* DDFK_APP_MSG_H */
