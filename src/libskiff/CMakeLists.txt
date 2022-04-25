project (libskiff)

set(CMAKE_STATIC_LIBRARY_PREFIX "")

option(LIBSKIFF_USE_THREADS "Enable threads" ON)
option(LIBSKIFF_GENERATE_STATS "Allow the generation of runtime statistics (slows VM)" OFF)

add_library(${PROJECT_NAME} 
  src/assembler/assemble.cpp
  src/bytecode/executable.cpp
  src/bytecode/floating_point.cpp
  src/generators/binary_generator.cpp
  src/generators/instruction_generator.cpp
  src/machine/execution_context.cpp
  src/machine/memory/memory.cpp
  src/machine/memory/memman.cpp
  src/machine/memory/stack.cpp
  src/machine/vm_load_binary.cpp
  src/machine/vm.cpp
  src/machine/system/print.cpp
  src/machine/system/timer.cpp)

add_library(skiff::libskiff ALIAS ${PROJECT_NAME})

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

if(LIBSKIFF_USE_THREADS)
  target_compile_definitions(${PROJECT_NAME} PRIVATE LIBSKIFF_USE_THREADS=1)
endif()

if(LIBSKIFF_GENERATE_STATS)
  target_compile_definitions(${PROJECT_NAME} PRIVATE LIBSKIFF_GENERATE_STATS=1)
endif()

if(COMPILE_TESTS)
  add_subdirectory(tests)
endif()
