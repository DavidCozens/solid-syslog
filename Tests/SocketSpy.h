#ifndef SOCKETSPY_H
#define SOCKETSPY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void SocketSpy_Reset(void);
    int  SocketSpy_SendtoCallCount(void);

#ifdef __cplusplus
}
#endif

#endif /* SOCKETSPY_H */
