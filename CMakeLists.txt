# AetherMachO - Mach-O analysis and emulation engine for macOS/iOS
# Copyright (c) 2026 Jesse Liu <neoliu2011@gmail.com>
# SPDX-License-Identifier: Apache License, Version 2.0
# See LICENSE file in the root directory for full license text.

cmake_minimum_required(VERSION 3.20)
project(AetherMachO VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(PROJECT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# find the dependent packages
find_package(AetherBinary CONFIG REQUIRED)
find_package(AetherVM CONFIG REQUIRED)

# include the sub projects
add_subdirectory(apple)

configure_file(
  "${CMAKE_SOURCE_DIR}/include/Version.h.in"
  "${CMAKE_BINARY_DIR}/generated/Version.h"
  @ONLY
)

set(HEADER_FILES
  include/MachOEngine.h
  ${CMAKE_BINARY_DIR}/generated/Version.h
)

set(SOURCE_FILES
  src/MachOEngine.cpp
)

add_library(MachOEngine SHARED
  ${SOURCE_FILES}
  ${HEADER_FILES}
)

set_target_properties(MachOEngine PROPERTIES
  SKIP_BUILD_RPATH TRUE
  SKIP_INSTALL_RPATH TRUE
)

target_link_libraries(MachOEngine PRIVATE
  AetherBinary::AetherBinary
  AetherVM::AetherVM
  MachOLoader
)

target_include_directories(MachOEngine PRIVATE 
  include
  ${CMAKE_BINARY_DIR}
)

target_compile_options(MachOEngine PRIVATE
  -DAETHER_DLLIMPL=1
  -fno-rtti
)

target_link_options(MachOEngine PRIVATE
  -Wl,-rpath,@executable_path
  -Wl,-rpath,@executable_path/../lib
  -Wl,-rpath,@loader_path
  -Wl,-rpath,@loader_path/../lib
  -Wl,-install_name,@rpath/libMachOEngine.dylib
)

target_include_directories(MachOEngine PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/generated>
  $<INSTALL_INTERFACE:include>
)

target_compile_definitions(MachOEngine PRIVATE
  PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
  PROJECT_VERSION_STRING="${PROJECT_VERSION_STRING}"
)

# Install rules

install(TARGETS MachOEngine
  EXPORT MachOEngineTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
)

install(EXPORT MachOEngineTargets
  NAMESPACE MachOEngine::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MachOEngine
)

configure_package_config_file(
  "${CMAKE_SOURCE_DIR}/cmake/MachOEngineConfig.cmake.in"
  "${CMAKE_BINARY_DIR}/generated/MachOEngineConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MachOEngine
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
  "${CMAKE_BINARY_DIR}/generated/MachOEngineConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  "${CMAKE_BINARY_DIR}/generated/MachOEngineConfig.cmake"
  "${CMAKE_BINARY_DIR}/generated/MachOEngineConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/MachOEngine
)
