# Copyright (C) 2014-2015 ARM Limited. All rights reserved.

cmake_minimum_required(VERSION 3.0)

# default to C99
set(CMAKE_C_FLAGS "-std=c11 -m32 -DMICROPY_NLR_SETJMP=1" CACHE STRING "")
set(CMAKE_CXX_FLAGS "-std=gnu++11 -m32 -fno-exceptions -DMICROPY_NLR_SETJMP=1" CACHE STRING "")
set(CMAKE_ASM_COMPILER_ARG1 "-m32 -DMICROPY_NLR_SETJMP=1")
set(CMAKE_LD_FLAGS "-m32 -fno-exceptions" CACHE STRING "")

# check that we are actually running on Linux, if we're not then we may pull in
# incorrect dependencies.
if(NOT (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux"))
    message(FATAL_ERROR "This Linux native target will not work on non-Linux platforms (your platform is ${CMAKE_HOST_SYSTEM_NAME}), use `yotta target` to set the target.")
endif()

