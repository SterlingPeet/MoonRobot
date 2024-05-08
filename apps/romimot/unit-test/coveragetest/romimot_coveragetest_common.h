/**
 * @file
 *
 * Common definitions for all romimot coverage tests
 */

#ifndef ROMIMOT_COVERAGETEST_COMMON_H
#define ROMIMOT_COVERAGETEST_COMMON_H

/*
 * Includes
 */

#include "utassert.h"
#include "uttest.h"
#include "utstubs.h"

#include "cfe.h"
#include "romimot_events.h"
#include "romimot.h"
#include "romimot_table.h"

/*
 * Macro to add a test case to the list of tests to execute
 */
#define ADD_TEST(test) UtTest_Add((Test_##test), romimot_UT_Setup, romimot_UT_TearDown, #test)

/*
 * Setup function prior to every test
 */
void romimot_UT_Setup(void);

/*
 * Teardown function after every test
 */
void romimot_UT_TearDown(void);

#endif /* ROMIMOT_COVERAGETEST_COMMON_H */
