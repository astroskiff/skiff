link_directories(${CPPUTEST_LIBRARIES})

find_package(libutil REQUIRED)

include_directories(
  ${CPPUTEST_INCLUDE_DIRS}
  ${LIBSKIFF_INCLUDE_DIRS}
  ${LIBUTIL_INCLUDE_DIRS}
  ../
)

add_executable(libskiff_unit_tests
        ${PROJECT_SOURCES}
        example.cpp
        assembler.cpp
        stack.cpp
        memory.cpp
        memman.cpp
        main.cpp)


target_link_libraries(libskiff_unit_tests
        ${CPPUTEST_LDFLAGS}
        libskiff
        libutil)

add_custom_command(TARGET libskiff_unit_tests COMMAND ./libskiff_unit_tests POST_BUILD)
