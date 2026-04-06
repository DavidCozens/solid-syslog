#ifndef SOLIDSYSLOGMACROS_H
#define SOLIDSYSLOGMACROS_H

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

#endif /* SOLIDSYSLOGMACROS_H */
