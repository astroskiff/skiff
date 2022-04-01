project (libskiff)

set(CMAKE_STATIC_LIBRARY_PREFIX "")

add_library(${PROJECT_NAME} 
  src/floating_point.cpp
  src/binary_generator.cpp
  src/instruction_generator.cpp)

add_library(skiff::libskiff ALIAS ${PROJECT_NAME})

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

if(COMPILE_TESTS)
  add_subdirectory(tests)
endif()
