cmake_minimum_required (VERSION 3.8)

project(skiff)

set(CMAKE_CXX_STANDARD 20)

#
# Options
#
option(COMPILE_TESTS "Execute unit tests" ON)
option(SKIFF_USE_THREADS "Allow threads"  ON)
option(WITH_ASAN     "Compile with ASAN" OFF)

#
# Find libskiff
#
find_package(libskiff REQUIRED)

#
# Setup build type 'Release vs Debug'
#
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

#
# Locate CPPUTest
#
if(DEFINED ENV{CPPUTEST_HOME})
    message(STATUS "Using CppUTest home: $ENV{CPPUTEST_HOME}")
    set(CPPUTEST_INCLUDE_DIRS $ENV{CPPUTEST_HOME}/include)
    set(CPPUTEST_LIBRARIES $ENV{CPPUTEST_HOME}/lib)
    set(CPPUTEST_LDFLAGS CppUTest CppUTestExt)
else()
    find_package(PkgConfig REQUIRED)
    pkg_search_module(CPPUTEST REQUIRED cpputest>=3.8)
    message(STATUS "Found CppUTest version ${CPPUTEST_VERSION}")
endif()

#
# Setup ASAN
#
if(WITH_ASAN)
  set(CMAKE_CX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
  set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
endif()

#
# Build 
#
include_directories( 
  ${LIBSKIFF_INCLUDE_DIRS} 
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(PROJECT_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/assembler/assemble.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/machine/vm.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/machine/vm_load_binary.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/machine/execution_context.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/machine/memory/memman.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/machine/memory/memory.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/machine/memory/stack.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/machine/system/io_user.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/machine/system/io_disk.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/machine/system/timer.cpp
)

add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
        skiffd.cpp)

if(SKIFF_USE_THREADS)
  add_compile_definitions(SKIFF_USE_THREADS)
  find_package (Threads REQUIRED)
  target_link_libraries(${PROJECT_NAME}
    Threads::Threads
  )
endif()

target_link_libraries(${PROJECT_NAME}
  libskiff
)

#
# Tests
#
if(COMPILE_TESTS)
  add_subdirectory(tests)
endif()
