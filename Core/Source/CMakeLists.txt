set(SOURCES
    SolidSyslog.c
    SolidSyslogSwitchingSender.c
    SolidSyslogBuffer.c
    SolidSyslogFormatter.c
    SolidSyslogMetaSd.c
    SolidSyslogNullBuffer.c
    SolidSyslogSender.c
    SolidSyslogStore.c
    SolidSyslogNullStore.c
    SolidSyslogNullSecurityPolicy.c
    SolidSyslogCrc16.c
    SolidSyslogCrc16Policy.c
    SolidSyslogStructuredData.c
    SolidSyslogTimeQualitySd.c
    SolidSyslogOriginSd.c
    SolidSyslogFile.c
    SolidSyslogFileStore.c
)

if(HAVE_STDATOMIC_H)
    list(APPEND SOURCES SolidSyslogAtomicCounter.c)
endif()

if(SOLIDSYSLOG_POSIX OR SOLIDSYSLOG_WINSOCK)
    list(APPEND SOURCES
        SolidSyslogResolver.c
        SolidSyslogDatagram.c
        SolidSyslogUdpSender.c
    )
endif()

if(SOLIDSYSLOG_POSIX OR SOLIDSYSLOG_OPENSSL)
    list(APPEND SOURCES
        SolidSyslogStream.c
    )
endif()

if(SOLIDSYSLOG_POSIX)
    list(APPEND SOURCES
        SolidSyslogStreamSender.c
    )
endif()

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PUBLIC  ${CMAKE_SOURCE_DIR}/Core/Interface
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)

if(SOLIDSYSLOG_POSIX)
    target_link_libraries(${PROJECT_NAME} PUBLIC rt)
endif()

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/Core/Interface/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)
