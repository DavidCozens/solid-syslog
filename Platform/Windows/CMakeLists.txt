target_sources(${PROJECT_NAME} PRIVATE
    Source/SolidSyslogWindowsAtomicCounter.c
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Interface/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
    OPTIONAL
)
