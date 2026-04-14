cmake_minimum_required(VERSION 3.16)
project(SolidSyslog C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CppUTest REQUIRED)
find_package(Threads REQUIRED)

if(MSVC)
    add_compile_options(/W4 /WX /wd4200 /wd4297)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
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

include(CheckIncludeFile)
check_include_file(stdatomic.h HAVE_STDATOMIC_H)

add_subdirectory(Source)

if(SOLIDSYSLOG_POSIX)
    add_subdirectory(Example)
endif()

enable_testing()
add_subdirectory(Tests)
