project(CFE_DDFK_APP C)

# Create the app module
add_cfe_app(ddfk_app fsw/src/ddfk_app.c)



# Add table
add_cfe_tables(ddfk_app fsw/tables/ddfk_app_tbl.c)

target_include_directories(ddfk_app PUBLIC
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
