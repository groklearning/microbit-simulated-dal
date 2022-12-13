# Copyright (C) 2014-2015 ARM Limited. All rights reserved.

cmake_minimum_required(VERSION 2.8)

# default to C99
set(CMAKE_C_FLAGS "-std=c11 -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast -Wno-int-conversion -Wno-incompatible-pointer-types -m32" CACHE STRING "")
set(CMAKE_CXX_FLAGS "-std=gnu++11 -Wno-int-to-pointer-cast -m32 -g" CACHE STRING "")
set(CMAKE_ASM_COMPILER_ARG1 "-m32")

# check that we are actually running on Linux, if we're not then we may pull in
# incorrect dependencies.
if(NOT (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux"))
    message(FATAL_ERROR "This Linux native target will not work on non-Linux platforms (your platform is ${CMAKE_HOST_SYSTEM_NAME}), use `yotta target` to set the target.")
endif()

