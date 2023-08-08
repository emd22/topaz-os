#include "PitTimer.h"
#include "Interrupt.h"

#include <TzDriverRegistry.h>

#include <Types.h>
#include <ArchX86.h>

static TzSystemTimerHandle system_timer_handle;

typedef struct {
    Int frequency;
    TzSystemTimerHandle handle;
} TzSystemTimerDriverParams;

void TzSetSystemTimerHandle(TzSystemTimerHandle handle) {
    system_timer_handle = handle;
}

static void InternTimerCallback(TzRegisterList *registers) {
    system_timer_handle(registers);
}


TzStatus TzSystemTimerDriverInit(void *userp) {
    TzSystemTimerDriverParams *params = (TzSystemTimerDriverParams *)userp;

    // register our system interrupt to our internal callback
    TzRegisterIrq(TZ_IRQ0, InternTimerCallback);

    const UInt32 divisor = 1193180 / params->frequency;

    TzOut8(0x43, 0x36);

    UInt8 lo = (UInt8)(divisor & 0xFF);
    UInt8 hi = (UInt8)((divisor >> 8) & 0xFF);

    TzOut8(0x40, lo);
    TzOut8(0x40, hi);

    TzSetSystemTimerHandle(params->handle);

    return TZ_OK;
}

static void InternTimerStub(TzRegisterList *registers) {
    return;
}


TzStatus TzSystemTimerDriverDestroy(void *userp) {
    (void)userp;

    system_timer_handle = InternTimerStub;

    return TZ_OK;
}

void TzSystemTimerInit(Int frequency, TzSystemTimerHandle handle) {
    TzSystemTimerDriverParams params;
    params.frequency = frequency;
    params.handle = handle;

    TzDriver *driver = TzDriverCreate(
        TZ_DRIVER_PIT_TIMER,
        TzSystemTimerDriverInit,
        TzSystemTimerDriverDestroy
    );
    TzDriverInit(driver, (void *)&params);
}








