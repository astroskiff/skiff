
link_directories(${CPPUTEST_LIBRARIES})

include_directories(
  ${CPPUTEST_INCLUDE_DIRS}
  ../
)

add_executable(unit_tests
        ${PROJECT_SOURCES}
        example_tests.cpp
        main.cpp)


target_link_libraries(unit_tests
        ${CPPUTEST_LDFLAGS})

      #add_custom_target(copy-test-files ALL
      # COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/test_files ${CMAKE_BINARY_DIR}/tests/test_files
      #  )

add_custom_command(TARGET unit_tests COMMAND ./unit_tests POST_BUILD)
