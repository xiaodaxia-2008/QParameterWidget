# Copyright © 2023 Zen Shawn. All rights reserved.
# Created at 11:49:57, August 18, 2023

cmake_minimum_required(VERSION 3.21)

option(BUILD_SHARED_LIBS "build shared libs" OFF)
option(BUILD_EXAMPLES "build example" OFF)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "build type, Release/Debug/MinSizeRel/RelWithDebInfo")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/out" CACHE PATH "install prefix")

project(QParameterWidget VERSION 0.0.0 DESCRIPTION "Introduction about QParameterWidget")

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)

if(MSVC)
  add_compile_options(/W3)
else()
  add_compile_options(-Wall -Wextra)
endif()

if(UNIX)
  find_program(CCACHE_PROGRAM ccache)

  if(CCACHE_PROGRAM)
    message(STATUS "Found ccache")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
  endif()
endif()

find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Qt6 CONFIG COMPONENTS Widgets UiTools LinguistTools)

if(NOT Qt6_FOUND)
  find_package(Qt5 CONFIG REQUIRED COMPONENTS Widgets LinguistTools)
endif()

include(GNUInstallDirs)
include(GenerateExportHeader)
include(InstallRequiredSystemLibraries)

add_subdirectory(Src)

if(BUILD_EXAMPLES)
  add_subdirectory(Examples)
endif()

include(CPack)