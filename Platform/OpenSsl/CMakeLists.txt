find_package(OpenSSL REQUIRED)

target_sources(${PROJECT_NAME} PRIVATE
    Source/SolidSyslogTlsStream.c
)

# Headers at compile time; libssl is linked by consumers (real linkage) or
# replaced by OpenSslFake in tests — not declared as a library link here.
target_include_directories(${PROJECT_NAME}
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/Interface
    PRIVATE ${OPENSSL_INCLUDE_DIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Interface/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)
