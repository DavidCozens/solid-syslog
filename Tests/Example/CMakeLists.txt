set(EXAMPLE_TEST_SOURCES
    SolidSyslogExampleTest.cpp
    ExampleServiceThreadTest.cpp
    main.cpp
)

add_executable(ExampleTests ${EXAMPLE_TEST_SOURCES})

target_link_libraries(ExampleTests PRIVATE ${PROJECT_NAME} PosixFakes CppUTest CppUTestExt pthread)

target_include_directories(ExampleTests PRIVATE
    ${CMAKE_SOURCE_DIR}/Example/SingleTask
    ${CMAKE_SOURCE_DIR}/Example/Common
)

target_sources(ExampleTests PRIVATE
    ${CMAKE_SOURCE_DIR}/Example/SingleTask/SolidSyslogExample.c
    ${CMAKE_SOURCE_DIR}/Example/Common/ExampleAppName.c
    ${CMAKE_SOURCE_DIR}/Example/Common/ExampleCommandLine.c
    ${CMAKE_SOURCE_DIR}/Example/Common/ExampleServiceThread.c
    ${CMAKE_SOURCE_DIR}/Example/Common/ExampleUdpConfig.c
)
