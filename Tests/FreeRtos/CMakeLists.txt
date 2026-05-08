# Tests/FreeRtos — host-side unit tests for FreeRTOS adapters under
# Platform/FreeRtos/Source/. Each test executable recompiles the adapter
# source under test with the FreeRtosFakes config and links against the
# fakes — see project_header_configured_platforms memory.

add_executable(SolidSyslogFreeRtosDatagramTest
    SolidSyslogFreeRtosDatagramTest.cpp
    main.cpp
    ${CMAKE_SOURCE_DIR}/Platform/FreeRtos/Source/SolidSyslogFreeRtosDatagram.c
    ${CMAKE_SOURCE_DIR}/Tests/Support/FreeRtosFakes/Source/FreeRtosSocketsFake.c
)

target_link_libraries(SolidSyslogFreeRtosDatagramTest PRIVATE
    ${PROJECT_NAME}
    CppUTest
    CppUTestExt
)

target_include_directories(SolidSyslogFreeRtosDatagramTest PRIVATE
    ${CMAKE_SOURCE_DIR}/Platform/FreeRtos/Interface
    ${CMAKE_SOURCE_DIR}/Core/Interface
    ${CMAKE_SOURCE_DIR}/Core/Source
    ${CMAKE_SOURCE_DIR}/Tests/Support/FreeRtosFakes/Interface
    $ENV{FREERTOS_KERNEL_PATH}/include
    $ENV{FREERTOS_PLUS_TCP_PATH}/source/include
)

add_test(NAME SolidSyslogFreeRtosDatagramTest COMMAND SolidSyslogFreeRtosDatagramTest)
