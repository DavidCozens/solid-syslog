cmake_minimum_required(VERSION 3.16)
project(SolidSyslog C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CppUTest REQUIRED)
find_package(Threads REQUIRED)

if(MSVC)
    # /wd4200: char buffer[] (C99 flexible array member) — valid C99, MSVC warns as nonstandard extension
    # /wd4297: test infrastructure throws C++ exceptions from extern "C" functions
    # to propagate assertion failures from C fakes through CppUTest. Permanent suppression.
    add_compile_options(/W4 /WX /wd4200 /wd4297)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion -Werror)
endif()

option(SANITIZE "Enable AddressSanitizer and UndefinedBehaviourSanitizer" OFF)
if(SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

option(COVERAGE "Enable code coverage instrumentation" OFF)
if(COVERAGE)
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

option(ENABLE_CLANG_TIDY "Run clang-tidy on all source files during build" OFF)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)
    set(CMAKE_C_CLANG_TIDY   "${CLANG_TIDY_EXECUTABLE}")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXECUTABLE}")
endif()

option(ENABLE_CPPCHECK "Run cppcheck on all source files during build" OFF)
if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXECUTABLE NAMES cppcheck)
    if(CPPCHECK_EXECUTABLE)
        set(CMAKE_C_CPPCHECK
            "${CPPCHECK_EXECUTABLE}"
            "--enable=warning,style,performance,portability"
            "--error-exitcode=1"
            "--suppress=missingIncludeSystem"
            "--inline-suppr"
            "--std=c11"
        )
        set(CMAKE_CXX_CPPCHECK
            "${CPPCHECK_EXECUTABLE}"
            "--enable=warning,style,performance,portability"
            "--error-exitcode=1"
            "--suppress=missingIncludeSystem"
            "--inline-suppr"
            "--std=c++17"
        )
    else()
        message(WARNING "cppcheck not found — ENABLE_CPPCHECK has no effect")
    endif()
endif()

include(CheckSymbolExists)
check_symbol_exists(sendto "sys/socket.h" HAVE_POSIX_SOCKETS)
option(SOLIDSYSLOG_POSIX "Include POSIX UDP sender" ${HAVE_POSIX_SOCKETS})

set(_SOLIDSYSLOG_PREV_CMAKE_REQUIRED_LIBRARIES "${CMAKE_REQUIRED_LIBRARIES}")
set(CMAKE_REQUIRED_LIBRARIES ws2_32)
check_symbol_exists(sendto "winsock2.h" HAVE_WINSOCK)
set(CMAKE_REQUIRED_LIBRARIES "${_SOLIDSYSLOG_PREV_CMAKE_REQUIRED_LIBRARIES}")
unset(_SOLIDSYSLOG_PREV_CMAKE_REQUIRED_LIBRARIES)
option(SOLIDSYSLOG_WINSOCK "Include Winsock UDP sender" ${HAVE_WINSOCK})

include(CheckIncludeFile)
check_include_file(stdatomic.h HAVE_STDATOMIC_H)
check_symbol_exists(InterlockedIncrement "windows.h" HAVE_WINDOWS_INTERLOCKED)

if(HAVE_STDATOMIC_H OR HAVE_WINDOWS_INTERLOCKED)
    set(HAVE_ATOMIC_COUNTER TRUE)
endif()

add_subdirectory(Source)

if(SOLIDSYSLOG_POSIX)
    add_subdirectory(Platform/Posix)
endif()

if(HAVE_WINDOWS_INTERLOCKED OR SOLIDSYSLOG_WINSOCK)
    add_subdirectory(Platform/Windows)
endif()

if(SOLIDSYSLOG_POSIX)
    add_subdirectory(Example)
endif()

enable_testing()
add_subdirectory(Tests)
