set(COMMON_SOURCES
    Common/ExampleAppName.c
    Common/ExampleCommandLine.c
    Common/ExampleServiceThread.c
    Common/ExampleUdpConfig.c
)

add_executable(SolidSyslogExample SingleTask/main.c SingleTask/SolidSyslogExample.c ${COMMON_SOURCES})
target_link_libraries(SolidSyslogExample PRIVATE ${PROJECT_NAME})
target_include_directories(SolidSyslogExample PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/SingleTask
    ${CMAKE_CURRENT_SOURCE_DIR}/Common
)
