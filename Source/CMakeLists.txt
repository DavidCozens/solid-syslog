set(SOURCES
    SolidSyslog.c
    SolidSyslogBuffer.c
    SolidSyslogNullBuffer.c
    SolidSyslogSender.c
)

if(SOLIDSYSLOG_POSIX)
    list(APPEND SOURCES
        SolidSyslogPosixClock.c
        SolidSyslogPosixHostname.c
        SolidSyslogPosixProcId.c
        SolidSyslogUdpSender.c
    )
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/Interface)

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/Interface/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)
