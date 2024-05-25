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
 *  The ROMI Motor Driver App header file containing version information
 */

#ifndef ROMIMOT_VERSION_H
#define ROMIMOT_VERSION_H

/* Development Build Macro Definitions */

#define ROMIMOT_BUILD_NUMBER 0 /*!< Development Build: Number of commits since baseline */
#define ROMIMOT_BUILD_BASELINE                                                            \
    "v1.0.0" /*!< Development Build: git tag that is the base for the current development \
              */

/*
 * Version Macros, see \ref cfsversions for definitions.
 */
#define ROMIMOT_MAJOR_VERSION 1 /*!< @brief Major version number. */
#define ROMIMOT_MINOR_VERSION 0 /*!< @brief Minor version number. */
#define ROMIMOT_REVISION      0 /*!< @brief Revision version number. */

/*!
 * @brief Mission revision.
 *
 * Reserved for mission use to denote patches/customizations as needed.
 * Values 1-254 are reserved for mission use to denote patches/customizations as needed. NOTE: Reserving 0 and 0xFF for
 * cFS open-source development use (pending resolution of nasa/cFS#440)
 */
#define ROMIMOT_MISSION_REV 0xFF

#define ROMIMOT_STR_HELPER(x) #x /*!< @brief Helper function to concatenate strings from integer macros */
#define ROMIMOT_STR(x)        ROMIMOT_STR_HELPER(x) /*!< @brief Helper function to concatenate strings from integer macros */

/*! @brief Development Build Version Number.
 * @details Baseline git tag + Number of commits since baseline. @n
 * See @ref cfsversions for format differences between development and release versions.
 */
#define ROMIMOT_VERSION ROMIMOT_BUILD_BASELINE //  "+dev" ROMIMOT_STR(ROMIMOT_BUILD_NUMBER)

/*! @brief Development Build Version String.
 * @details Reports the current development build's baseline, number, and name. Also includes a note about the latest
 * official version. @n See @ref cfsversions for format differences between development and release versions.
 */
#define ROMIMOT_VERSION_STRING " ROMI Motor Driver App " ROMIMOT_VERSION

#endif /* ROMIMOT_VERSION_H */
