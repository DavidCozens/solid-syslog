#include "LwipUdpFake.h"

#include <stddef.h>

#include "lwip/udp.h"

static unsigned udpNewCallCount = 0;
static struct udp_pcb fakePcb;
static struct udp_pcb* lastUdpNewReturned = NULL;
static bool udpNewFails = false;

static unsigned udpRemoveCallCount = 0;
static struct udp_pcb* lastUdpRemovePcb = NULL;

void LwipUdpFake_Reset(void)
{
    udpNewCallCount = 0;
    lastUdpNewReturned = NULL;
    udpNewFails = false;
    udpRemoveCallCount = 0;
    lastUdpRemovePcb = NULL;
}

void LwipUdpFake_SetUdpNewFails(bool fails)
{
    udpNewFails = fails;
}

unsigned LwipUdpFake_UdpNewCallCount(void)
{
    return udpNewCallCount;
}

struct udp_pcb* LwipUdpFake_LastUdpNewReturned(void)
{
    return lastUdpNewReturned;
}

unsigned LwipUdpFake_UdpRemoveCallCount(void)
{
    return udpRemoveCallCount;
}

struct udp_pcb* LwipUdpFake_LastUdpRemovePcb(void)
{
    return lastUdpRemovePcb;
}

struct udp_pcb* udp_new(void)
{
    ++udpNewCallCount;
    lastUdpNewReturned = udpNewFails ? NULL : &fakePcb;
    return lastUdpNewReturned;
}

void udp_remove(struct udp_pcb* pcb)
{
    ++udpRemoveCallCount;
    lastUdpRemovePcb = pcb;
}
