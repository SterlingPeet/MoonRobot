# This example toolchain file describes the cross compiler to use for
# the target architecture indicated in the configuration file.

# In this example, we assume your host system has an apropriate GCC compiler in /usr/bin
# and we assume that you have used something like mk-sbuild to set up a mirror of a raspberry
# pi's rootfs, similar to https://tttapa.github.io/Pages/Raspberry-Pi/C++-Development-Ubuntu/index.html

# Basic cross system configuration
SET(CMAKE_SYSTEM_NAME			Linux)
SET(CMAKE_SYSTEM_VERSION		1)
SET(CMAKE_SYSTEM_PROCESSOR		arm)

# you can get these from https://github.com/tttapa/docker-arm-cross-toolchain
SET(COMPILER_ROOT "$ENV{HOME}/opt/x-tools/aarch64-rpi3-linux-gnu")
MESSAGE( "using compiler root ${COMPILER_ROOT}/bin/aarch64-rpi3-linux-gnu-gcc")
IF( NOT EXISTS ${COMPILER_ROOT}/bin/aarch64-rpi3-linux-gnu-gcc)
  MESSAGE(FATAL_ERROR "You can get the compiler from https://github.com/tttapa/docker-arm-cross-toolchain")
ENDIF()

SET(CMAKE_C_COMPILER			"${COMPILER_ROOT}/bin/aarch64-rpi3-linux-gnu-gcc")
SET(CMAKE_CXX_COMPILER			"${COMPILER_ROOT}/bin/aarch64-rpi3-linux-gnu-g++")

# where is the target environment, not needed unless we're linking to a library in there
# SET(CMAKE_FIND_ROOT_PATH		"/var/lib/schroot/chroots/rpi3-bullseye-arm64")
# MESSAGE(STATUS "Using sysroot path: ${SYSROOT_PATH}")

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM	NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY	ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE	ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# these settings are specific to cFE/OSAL and determines which
# abstraction layers are built when using this toolchain
# Note that "pc-linux" works fine even though this is not technically a "pc"
SET(CFE_SYSTEM_PSPNAME      "pc-linux")
SET(OSAL_SYSTEM_OSTYPE      "posix")

# some other stuff while initially testing

SET(VERBOSE TRUE)
