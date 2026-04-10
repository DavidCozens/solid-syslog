set(SOURCES
    SolidSyslog.c
    SolidSyslogBuffer.c
    SolidSyslogFormat.c
    SolidSyslogMetaSd.c
    SolidSyslogNullBuffer.c
    SolidSyslogSender.c
    SolidSyslogStore.c
    SolidSyslogNullStore.c
    SolidSyslogStructuredData.c
    SolidSyslogTimeQualitySd.c
    SolidSyslogOriginSd.c
    SolidSyslogFileApi.c
    SolidSyslogFileStore.c
)

if(HAVE_STDATOMIC_H)
    list(APPEND SOURCES
        SolidSyslogAtomicCounter.c
    )
endif()

if(SOLIDSYSLOG_POSIX)
    list(APPEND SOURCES
        SolidSyslogPosixClock.c
        SolidSyslogPosixHostname.c
        SolidSyslogPosixMessageQueueBuffer.c
        SolidSyslogPosixProcessId.c
        SolidSyslogUdpSender.c
        SolidSyslogTcpSender.c
        SolidSyslogPosixFileApi.c
    )
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/Interface)

if(SOLIDSYSLOG_POSIX)
    target_link_libraries(${PROJECT_NAME} PUBLIC rt)
endif()

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/Interface/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)
