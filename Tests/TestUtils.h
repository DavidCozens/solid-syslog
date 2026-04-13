#ifndef TESTUTILS_H
#define TESTUTILS_H

#include <stddef.h>

static inline size_t MinSize(size_t a, size_t b)
{
    return (a < b) ? a : b;
}

#endif /* TESTUTILS_H */
