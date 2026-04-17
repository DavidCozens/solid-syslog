target_sources(${PROJECT_NAME} PRIVATE
    Source/SolidSyslogWindowsAtomicCounter.c
)

# Interface headers are installed once Winsock stories (S13.4, S13.9) populate
# Platform/Windows/Interface/.
