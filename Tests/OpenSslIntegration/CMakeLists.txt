# OpenSSL-backed integration tests: links the real libssl and exercises
# SolidSyslogTlsStream end-to-end against an in-process server. Separate binary
# from SolidSyslogTests because unit tests link OpenSslFakes in place of libssl
# — the two cannot share one executable without fake/real symbol collision.
add_executable(OpenSslIntegrationTests
    BioPairStream.c
    TlsTestCert.c
    TlsTestServer.c
    SolidSyslogTlsStreamIntegrationTest.cpp
    main.cpp
)

target_link_libraries(OpenSslIntegrationTests PRIVATE
    ${PROJECT_NAME}
    CppUTest
    CppUTestExt
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

target_include_directories(OpenSslIntegrationTests PRIVATE
    ${CMAKE_SOURCE_DIR}/Core/Source
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_test(NAME OpenSslIntegrationTests COMMAND OpenSslIntegrationTests)
