if(SOLIDSYSLOG_POSIX)
    set(COMMON_SOURCES
        Common/ExampleAppName.c
        Common/ExampleCommandLine.c
        Common/ExampleServiceThread.c
        Common/ExampleUdpConfig.c
        Common/ExampleTcpConfig.c
        Common/ExampleInteractive.c
    )

    # Single-task example: NullBuffer, direct send, bare-metal model
    add_executable(SolidSyslogExample SingleTask/main.c SingleTask/SolidSyslogExample.c ${COMMON_SOURCES})
    target_link_libraries(SolidSyslogExample PRIVATE ${PROJECT_NAME})
    target_include_directories(SolidSyslogExample PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/SingleTask
        ${CMAKE_CURRENT_SOURCE_DIR}/Common
    )

    # Threaded example: PosixMessageQueueBuffer, two pthreads (requires POSIX threads and mqueue)
    add_executable(SolidSyslogThreadedExample Threaded/main.c ${COMMON_SOURCES})
    target_link_libraries(SolidSyslogThreadedExample PRIVATE ${PROJECT_NAME} Threads::Threads)
    target_include_directories(SolidSyslogThreadedExample PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Common
    )
elseif(SOLIDSYSLOG_WINSOCK AND HAVE_WINDOWS_PLATFORM)
    # Windows example: NullBuffer + Winsock UDP + Windows clock/hostname/process-id.
    # Output binary is named SolidSyslogExample.exe so BDD scenarios remain platform-agnostic.
    add_executable(SolidSyslogWindowsExample
        Windows/main.c
        Windows/SolidSyslogWindowsExample.c
        Windows/ExampleWindowsCommandLine.c
        Common/ExampleAppName.c
        Common/ExampleInteractive.c
    )
    set_target_properties(SolidSyslogWindowsExample PROPERTIES OUTPUT_NAME "SolidSyslogExample")
    target_link_libraries(SolidSyslogWindowsExample PRIVATE ${PROJECT_NAME})
    target_include_directories(SolidSyslogWindowsExample PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Windows
        ${CMAKE_CURRENT_SOURCE_DIR}/Common
    )
endif()
