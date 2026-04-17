if(SOLIDSYSLOG_POSIX)
    set(POSIX_FAKES_SOURCES
        ClockFake.c
        SafeStringStandard.c
        SocketFake.c
    )

    add_library(PosixFakes STATIC ${POSIX_FAKES_SOURCES})

    target_include_directories(PosixFakes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

    target_link_libraries(PosixFakes PUBLIC ${PROJECT_NAME})
endif()

if(SOLIDSYSLOG_WINSOCK)
    add_library(WinsockFakes STATIC WinsockFake.c SafeStringWindows.c)

    target_include_directories(WinsockFakes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

    target_link_libraries(WinsockFakes PUBLIC ${PROJECT_NAME})
endif()
