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

    if(SOLIDSYSLOG_OPENSSL)
        # OpenSslFake.c and any test that includes <openssl/ssl.h> needs the
        # OpenSSL headers on the compiler include path. On POSIX these usually
        # come from /usr/include implicitly; on Windows+vcpkg they live under
        # the vcpkg install tree and must be declared explicitly.
        target_include_directories(PosixFakes PUBLIC ${OPENSSL_INCLUDE_DIR})
    endif()
endif()

if(SOLIDSYSLOG_WINSOCK)
    add_library(WinsockFakes STATIC WinsockFake.c SafeStringWindows.c)

    target_include_directories(WinsockFakes PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

    target_link_libraries(WinsockFakes PUBLIC ${PROJECT_NAME})
endif()
