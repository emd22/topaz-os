#ifndef TOPAZ_DRIVER_REGISTRY_H
#define TOPAZ_DRIVER_REGISTRY_H

#include <Types.h>

#define TZ_DRIVER_NONE 0
#define TZ_MAX_DRIVERS 16

typedef TzStatus (*TzDriverInitHandle)(void *userp);
typedef TzStatus (*TzDriverDestroyHandle)(void *userp);

typedef UInt16 TzDriverId;

typedef struct {
    TzDriverId id;
    TzDriverInitHandle init_handle;
    TzDriverDestroyHandle destroy_handle;
} TzDriver;

void TzDriverRegistryHaltAll();

TzDriver *TzDriverCreate(TzDriverId id, TzDriverInitHandle init_handle, TzDriverDestroyHandle destroy_handle);
TzStatus TzDriverInit(TzDriver *driver, void *userp);
TzStatus TzDriverDestroy(TzDriver *driver, void *userp);

#endif