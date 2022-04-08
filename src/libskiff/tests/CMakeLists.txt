
link_directories(${CPPUTEST_LIBRARIES})

include_directories(
  ${CPPUTEST_INCLUDE_DIRS}
  ../
)

add_executable(libskiff_unit_tests
        ${PROJECT_SOURCES}
        example.cpp
        assembler.cpp
        instruction_generator.cpp
        floating_point.cpp
        vm_execution_tests.cpp
        stack.cpp
        memory.cpp
        main.cpp)


target_link_libraries(libskiff_unit_tests
        ${CPPUTEST_LDFLAGS}
        skiff::libskiff
        skiff::libutil)

add_custom_command(TARGET libskiff_unit_tests COMMAND ./libskiff_unit_tests POST_BUILD)
