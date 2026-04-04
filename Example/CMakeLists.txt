add_executable(SolidSyslogExample main.c SolidSyslogExample.c)
target_link_libraries(SolidSyslogExample PRIVATE ${PROJECT_NAME})
target_include_directories(SolidSyslogExample PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
