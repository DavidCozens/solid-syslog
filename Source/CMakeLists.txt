set(SOURCES
    SolidSyslog.c
    SolidSyslogBuffer.c
    SolidSyslogFormat.c
    SolidSyslogMetaSd.c
    SolidSyslogNullBuffer.c
    SolidSyslogSender.c
    SolidSyslogStructuredData.c
    SolidSyslogTimeQualitySd.c
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
        SolidSyslogPosixMqBuffer.c
        SolidSyslogPosixProcId.c
        SolidSyslogUdpSender.c
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
