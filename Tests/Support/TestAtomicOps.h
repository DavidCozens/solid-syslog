#ifndef TESTATOMICOPS_H
#define TESTATOMICOPS_H

/* Test helper that picks whichever real AtomicOps the build provides.
   Prefers WindowsAtomicOps when available (the architecture default on
   MSVC/legacy Windows toolchains that lack <stdatomic.h>); otherwise
   falls back to the StdAtomicOps default from Platform/Atomics. */
#if defined(SOLIDSYSLOG_TEST_USE_WINDOWS_ATOMIC_OPS)
#include "SolidSyslogWindowsAtomicOps.h"
#define TestAtomicOps_Create  SolidSyslogWindowsAtomicOps_Create
#define TestAtomicOps_Destroy SolidSyslogWindowsAtomicOps_Destroy
#else
#include "SolidSyslogStdAtomicOps.h"
#define TestAtomicOps_Create  SolidSyslogStdAtomicOps_Create
#define TestAtomicOps_Destroy SolidSyslogStdAtomicOps_Destroy
#endif

#endif /* TESTATOMICOPS_H */
