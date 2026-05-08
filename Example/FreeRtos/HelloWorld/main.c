#include "CmsdkUart.h"

#include <FreeRTOS.h>
#include <task.h>

#include <stdint.h>
#include <stdio.h>

#define CMSDK_UART0_BASE_ADDRESS UINT32_C(0x40004000)

static uint32_t MmioRead32(uintptr_t address)
{
    // NOLINTNEXTLINE(performance-no-int-to-ptr) -- mapping the CMSDK UART MMIO address into a 32-bit volatile pointer.
    return *(volatile uint32_t*) address;
}

static void MmioWrite32(uintptr_t address, uint32_t value)
{
    // NOLINTNEXTLINE(performance-no-int-to-ptr) -- mapping the CMSDK UART MMIO address into a 32-bit volatile pointer.
    *(volatile uint32_t*) address = value;
}

static void RtosSleep(int milliseconds)
{
    vTaskDelay(pdMS_TO_TICKS((TickType_t) milliseconds));
}

static const CmsdkUartMemoryAccess MMIO_ACCESS = {MmioRead32, MmioWrite32, RtosSleep};

static void HelloTask(void* argument)
{
    (void) argument;
    printf("hello from FreeRTOS on QEMU mps2-an385\n");
    fflush(stdout);

    /* Block this task forever; the scheduler keeps running so QEMU stays
     * alive and is GDB-attachable. */
    for (;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void)
{
    CmsdkUart_Init(&MMIO_ACCESS, CMSDK_UART0_BASE_ADDRESS);

    if (xTaskCreate(HelloTask, "hello", configMINIMAL_STACK_SIZE * 4, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        /* Heap exhausted before the scheduler started — trap. */
        for (;;)
        {
        }
    }

    vTaskStartScheduler();

    /* Reached only if vTaskStartScheduler returned (idle / timer task could
     * not be allocated). */
    for (;;)
    {
    }
    return 0;
}

/* FreeRTOS hook: called from heap_4.c when pvPortMalloc fails. */
void vApplicationMallocFailedHook(void)
{
    for (;;)
    {
    }
}

/* FreeRTOS hook: called when a task overflows its stack
 * (configCHECK_FOR_STACK_OVERFLOW = 2). */
void vApplicationStackOverflowHook(TaskHandle_t task, char* taskName)
{
    (void) task;
    (void) taskName;
    for (;;)
    {
    }
}
