project (libskiff)

set(CMAKE_STATIC_LIBRARY_PREFIX "")

add_library(${PROJECT_NAME} 
  src/assembler/assemble.cpp
  src/bytecode/floating_point.cpp
  src/bytecode/loader.cpp
  src/generators/binary_generator.cpp
  src/generators/instruction_generator.cpp)

add_library(skiff::libskiff ALIAS ${PROJECT_NAME})

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

if(COMPILE_TESTS)
  add_subdirectory(tests)
endif()
