project (libutil)

set(CMAKE_STATIC_LIBRARY_PREFIX "")

add_library(${PROJECT_NAME} 
  src/random_string.cpp
  src/floating_point.cpp)

add_library(skiff::libutil ALIAS ${PROJECT_NAME})

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

if(COMPILE_TESTS)
  add_subdirectory(tests)
endif()
