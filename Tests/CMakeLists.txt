set(TEST_SOURCES
    SolidSyslogTest.cpp
    SenderSpy.c
    PosixUdpSenderTest.cpp
    SocketSpy.c
    main.cpp
)

add_executable(${PROJECT_NAME}Tests ${TEST_SOURCES})

target_link_libraries(${PROJECT_NAME}Tests PRIVATE ${PROJECT_NAME} CppUTest CppUTestExt pthread)

# Standard CppUTest console output via ctest -V
add_test(NAME ${PROJECT_NAME}Tests COMMAND ${PROJECT_NAME}Tests)

# JUnit XML output for CI and the VS Code test explorer.
# Uses a cmake script so test failures (non-zero exit) don't abort the target.
add_custom_target(junit
    COMMAND ${CMAKE_COMMAND}
        -DEXECUTABLE=$<TARGET_FILE:${PROJECT_NAME}Tests>
        -DDIR=${CMAKE_BINARY_DIR}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/run_junit.cmake
    DEPENDS ${PROJECT_NAME}Tests
    COMMENT "Running tests with JUnit XML output"
)

# Code coverage report using lcov/genhtml.
# Requires the coverage preset: cmake --preset coverage && cmake --build --preset coverage --target coverage
# lcov and genhtml must be installed (available in both container images).
if(COVERAGE)
    find_program(LCOV_EXECUTABLE lcov)
    find_program(GENHTML_EXECUTABLE genhtml)

    if(LCOV_EXECUTABLE AND GENHTML_EXECUTABLE)
        add_custom_target(coverage
            COMMAND ${LCOV_EXECUTABLE} --zerocounters --directory ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND}
                -DEXECUTABLE=$<TARGET_FILE:${PROJECT_NAME}Tests>
                -DDIR=${CMAKE_BINARY_DIR}
                -P ${CMAKE_CURRENT_SOURCE_DIR}/run_junit.cmake
            COMMAND ${LCOV_EXECUTABLE} --capture --directory ${CMAKE_BINARY_DIR}
                --output-file ${CMAKE_BINARY_DIR}/coverage.info
                --ignore-errors inconsistent
                --rc branch_coverage=1
            COMMAND ${LCOV_EXECUTABLE} --remove ${CMAKE_BINARY_DIR}/coverage.info
                "/usr/*" "/home/cpputest/*" "${CMAKE_SOURCE_DIR}/Tests/*"
                --output-file ${CMAKE_BINARY_DIR}/coverage.filtered.info
                --ignore-errors unused,empty
                --rc branch_coverage=1
            COMMAND ${GENHTML_EXECUTABLE} ${CMAKE_BINARY_DIR}/coverage.filtered.info
                --output-directory ${CMAKE_BINARY_DIR}/coverage_report
                --branch-coverage
                --ignore-errors empty
            DEPENDS ${PROJECT_NAME}Tests
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating coverage report in build/coverage/coverage_report/"
        )
    else()
        message(STATUS "lcov/genhtml not found — 'coverage' target unavailable (requires updated base image)")
    endif()
endif()
