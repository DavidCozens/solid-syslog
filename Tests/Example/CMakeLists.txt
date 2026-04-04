set(EXAMPLE_TEST_SOURCES
    SolidSyslogExampleTest.cpp
    main.cpp
)

add_executable(ExampleTests ${EXAMPLE_TEST_SOURCES})

target_link_libraries(ExampleTests PRIVATE ${PROJECT_NAME} PosixFakes CppUTest CppUTestExt pthread)

target_include_directories(ExampleTests PRIVATE ${CMAKE_SOURCE_DIR}/Example)

target_sources(ExampleTests PRIVATE ${CMAKE_SOURCE_DIR}/Example/SolidSyslogExample.c)
