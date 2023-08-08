#ifndef TOPAZ_DRIVER_PIT_TIMER_H
#define TOPAZ_DRIVER_PIT_TIMER_H

#include <ArchX86.h>

#define TZ_DRIVER_PIT_TIMER 0x0001

typedef void (*TzSystemTimerHandle)(TzRegisterList *);

void TzSetSystemTimerHandle(TzSystemTimerHandle handle);
static void InternTimerCallback(TzRegisterList *registers);
void TzSystemTimerInit(Int frequency, TzSystemTimerHandle handle);

#endif