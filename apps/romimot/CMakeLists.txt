project(CFE_ROMIMOT C)

# Create the app module
add_cfe_app(romimot fsw/src/romimot.c fsw/src/romimot_hw.c)


# Add table
add_cfe_tables(romimot fsw/tables/romimot_tbl.c)

target_include_directories(romimot PUBLIC
  fsw/mission_inc
  fsw/platform_inc
)

# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if (ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif (ENABLE_UNIT_TESTS)
