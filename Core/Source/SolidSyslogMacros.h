#ifndef SOLIDSYSLOGMACROS_H
#define SOLIDSYSLOGMACROS_H

#include <stddef.h>

#include "SolidSyslogError.h"

/* Deviation from MISRA Dir 4.9: type-generic NULL guard cannot be expressed
   as an inline function without void* casts (Rule 11.5). Macro is preferred
   over repeated if-blocks or type-unsafe alternatives. */
/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define ASSIGN_IF_NON_NULL(field, value) \
    do                                   \
    {                                    \
        if ((value) != NULL)             \
        {                                \
            (field) = (value);           \
        }                                \
    } while (0)

/* Same deviation as ASSIGN_IF_NON_NULL — type-generic NULL guard needs a macro.
   Used in _Create paths to install a required collaborator or report exactly
   which slot the integrator left NULL. */
/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define ASSIGN_OR_REPORT(field, value, errorMessage)                     \
    do                                                                   \
    {                                                                    \
        if ((value) != NULL)                                             \
        {                                                                \
            (field) = (value);                                           \
        }                                                                \
        else                                                             \
        {                                                                \
            SolidSyslog_Error(SOLIDSYSLOG_SEVERITY_ERR, (errorMessage)); \
        }                                                                \
    } while (0)

/* Portable compile-time assertion. Uses the negative-array-size trick
   which works from C89 through C23 and all C++ versions — no C11 required. */
/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) */
#define SOLIDSYSLOG_STATIC_ASSERT(cond, msg) typedef char solidsyslog_static_assert_[(cond) ? 1 : -1]

#endif /* SOLIDSYSLOGMACROS_H */
