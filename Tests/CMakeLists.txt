if(SOLIDSYSLOG_POSIX)
    add_subdirectory(Support)
    add_subdirectory(Example)
endif()

set(TEST_SOURCES
    SolidSyslogTest.cpp
    SolidSyslogMetaSdTest.cpp
    SolidSyslogTimeQualitySdTest.cpp
    SolidSyslogNullBufferTest.cpp
    SolidSyslogPrivalTest.cpp
    BufferFakeTest.cpp
    BufferFake.c
    SenderSpyTest.cpp
    SenderSpy.c
    StringFakeTest.cpp
    StringFake.c
    main.cpp
)

if(HAVE_STDATOMIC_H)
    list(APPEND TEST_SOURCES
        SolidSyslogAtomicCounterTest.cpp
    )
endif()

if(SOLIDSYSLOG_POSIX)
    list(APPEND TEST_SOURCES
        SolidSyslogPosixClockTest.cpp
        SolidSyslogPosixMqBufferTest.cpp
        ClockFakeTest.cpp
        SolidSyslogUdpSenderTest.cpp
    )
endif()

add_executable(${PROJECT_NAME}Tests ${TEST_SOURCES})

target_link_libraries(${PROJECT_NAME}Tests PRIVATE ${PROJECT_NAME} CppUTest CppUTestExt pthread)
target_include_directories(${PROJECT_NAME}Tests PRIVATE ${CMAKE_SOURCE_DIR}/Source)

if(SOLIDSYSLOG_POSIX)
    target_link_libraries(${PROJECT_NAME}Tests PRIVATE PosixFakes)
endif()

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
                --rc branch_coverage=1
            COMMAND ${LCOV_EXECUTABLE} --extract ${CMAKE_BINARY_DIR}/coverage.info
                "${CMAKE_SOURCE_DIR}/Source/*"
                --output-file ${CMAKE_BINARY_DIR}/coverage.filtered.info
                --rc branch_coverage=1
            COMMAND ${GENHTML_EXECUTABLE} ${CMAKE_BINARY_DIR}/coverage.filtered.info
                --output-directory ${CMAKE_BINARY_DIR}/coverage_report
                --branch-coverage
            DEPENDS ${PROJECT_NAME}Tests
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Generating coverage report in build/coverage/coverage_report/"
        )
    else()
        message(STATUS "lcov/genhtml not found — 'coverage' target unavailable (requires updated base image)")
    endif()
endif()
