#ifndef SOLIDSYSLOGWINDOWSPROCESSIDINTERNAL_H
#define SOLIDSYSLOGWINDOWSPROCESSIDINTERNAL_H

/* Library-internal test seam. Tests replace this function pointer via
   CppUTest's UT_PTR_SET to inject a deterministic PID (MSVC does not
   support GCC's weak/strong symbol override trick used by the POSIX fakes). */

#include "ExternC.h"

#include <windows.h>

EXTERN_C_BEGIN

    typedef DWORD(WINAPI * WindowsGetCurrentProcessIdFn)(void);

    extern WindowsGetCurrentProcessIdFn WindowsProcessId_GetCurrentProcessId;

EXTERN_C_END

#endif /* SOLIDSYSLOGWINDOWSPROCESSIDINTERNAL_H */
