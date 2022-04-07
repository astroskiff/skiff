project (libskiff)

set(CMAKE_STATIC_LIBRARY_PREFIX "")

add_library(${PROJECT_NAME} 
  src/assembler/assemble.cpp
  src/bytecode/executable.cpp
  src/bytecode/floating_point.cpp
  src/generators/binary_generator.cpp
  src/generators/instruction_generator.cpp
  src/machine/execution_context.cpp
  src/machine/stack.cpp
  src/machine/vm_instruction_decode.cpp
  src/machine/vm.cpp)

add_library(skiff::libskiff ALIAS ${PROJECT_NAME})

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

if(COMPILE_TESTS)
  add_subdirectory(tests)
endif()
