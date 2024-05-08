/**
 * @file
 *
 *
 * Purpose:
 * Extra scaffolding functions for the romimot unit test
 *
 * Notes:
 * This is an extra UT-specific extern declaration
 * to obtain access to an internal data structure
 *
 * UT often needs to modify internal data structures in ways that
 * actual applications never would (bypassing the normal API) in
 * order to exercise or set up for off-nominal cases.
 */

#ifndef UT_ROMIMOT_H
#define UT_ROMIMOT_H

/*
 * Necessary to include these here to get the definition of the
 * "ROMIMOT_Data_t" typedef.
 */
#include "romimot_events.h"
#include "romimot.h"

/*
 * Allow UT access to the global "ROMIMOT_Data" object.
 */
extern ROMIMOT_Data_t ROMIMOT_Data;

#endif /* UT_ROMIMOT_H */
