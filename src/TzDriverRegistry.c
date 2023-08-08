#include "TzDriverRegistry.h"
#include "TzKern.h"

static TzDriver driver_registry[TZ_MAX_DRIVERS];


void TzDriverRegistryHaltAll() {
    Int i;
    for (i = 0; i < TZ_MAX_DRIVERS; i++) {
        TzDriver *driver = &driver_registry[i];
        if (driver->id == TZ_DRIVER_NONE)
            continue;
        driver->destroy_handle(NULL);
    }
}

TzDriver *FindFreeDriver() {
    Int i;
    for (i = 0; i < TZ_MAX_DRIVERS; i++) {
        TzDriver *driver = &driver_registry[i];
        if (driver->id == TZ_DRIVER_NONE) {
            return driver;
        }
    }
    TzPanic("No free driver slots! (%d)", TZ_MAX_DRIVERS);
    return NULL;
}



TzDriver *TzDriverCreate(TzDriverId id, TzDriverInitHandle init_handle, TzDriverDestroyHandle destroy_handle) {
    TzDriver *driver = FindFreeDriver();

    driver->id = id;
    driver->init_handle = init_handle;
    driver->destroy_handle = destroy_handle;

    return driver;
}

TzStatus TzDriverInit(TzDriver *driver, void *userp) {
    TzStatus status = driver->init_handle(userp);
    return status;
}

TzStatus TzDriverDestroy(TzDriver *driver, void *userp) {
    TzStatus status = driver->destroy_handle(userp);
    driver->id = TZ_DRIVER_NONE;
    return status;
}