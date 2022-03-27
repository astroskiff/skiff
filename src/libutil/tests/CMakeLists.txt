
link_directories(${CPPUTEST_LIBRARIES})

include_directories(
  ${CPPUTEST_INCLUDE_DIRS}
  ../
)

add_executable(libutil_unit_tests
        ${PROJECT_SOURCES}
        generate_random.cpp
        random_string.cpp
        random_entry.cpp
        main.cpp)


target_link_libraries(libutil_unit_tests
        ${CPPUTEST_LDFLAGS}
        skiff::libutil)

add_custom_command(TARGET libutil_unit_tests COMMAND ./libutil_unit_tests POST_BUILD)
