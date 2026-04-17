if(HAVE_WINDOWS_INTERLOCKED)
    target_sources(${PROJECT_NAME} PRIVATE
        Source/SolidSyslogWindowsAtomicCounter.c
    )
endif()

if(SOLIDSYSLOG_WINSOCK)
    target_sources(${PROJECT_NAME} PRIVATE
        Source/SolidSyslogAddress.c
        Source/SolidSyslogWinsockResolver.c
        Source/SolidSyslogWinsockDatagram.c
    )
    target_link_libraries(${PROJECT_NAME} PUBLIC ws2_32)
    target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Interface)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Interface/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
    )
endif()
