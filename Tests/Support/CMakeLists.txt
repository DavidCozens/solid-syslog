set(SUPPORT_SOURCES
    BufferFake.c
    SenderSpy.c
    StringFake.c
)

if(SOLIDSYSLOG_POSIX)
    list(APPEND SUPPORT_SOURCES
        ClockFake.c
        SocketSpy.c
    )
endif()

add_library(TestSupport STATIC ${SUPPORT_SOURCES})

target_include_directories(TestSupport PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(TestSupport PUBLIC ${PROJECT_NAME})
