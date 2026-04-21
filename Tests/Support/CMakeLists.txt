if(SOLIDSYSLOG_POSIX)
    set(POSIX_FAKES_SOURCES
        ClockFake.c
        SafeStringStandard.c
        SocketFake.c
    )

    if(SOLIDSYSLOG_OPENSSL)
        list(APPEND POSIX_FAKES_SOURCES OpenSslFake.c)
    endif()

    add_library(PosixFakes STATIC ${POSIX_FAKES_SOURCES})

    target_include_directories(PosixFakes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

    target_link_libraries(PosixFakes PUBLIC ${PROJECT_NAME})
endif()

if(SOLIDSYSLOG_WINSOCK)
    add_library(WinsockFakes STATIC WinsockFake.c SafeStringWindows.c)

    target_include_directories(WinsockFakes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

    target_link_libraries(WinsockFakes PUBLIC ${PROJECT_NAME})
endif()
