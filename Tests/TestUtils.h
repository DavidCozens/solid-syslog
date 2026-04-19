#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <stddef.h>

static inline size_t MinSize(size_t a, size_t b)
{
    return (a < b) ? a : b;
}

/* Expected call counts — readable names for CALLED_FUNCTION assertions. */
enum
{
    NEVER  = 0,
    ONCE   = 1,
    TWICE  = 2,
    THRICE = 3
};

/* Assert that a call-count expression equals an expected count.
 * Reads as: CALLED_FUNCTION(SenderFake_SendCount(inner), ONCE); */
/* NOLINTNEXTLINE(cppcoreguidelines-macro-usage) -- CppUTest assertions are macro-based */
#define CALLED_FUNCTION(f, n) LONGS_EQUAL((n), (f))

#endif /* TESTUTILS_H */
