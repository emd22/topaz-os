#include "Ahci.h"

#include <TzKern.h>
#include "TtyGraphics.h"

#define HBA_PORT_IPM_ACTIVE  1
#define HBA_PORT_DET_PRESENT 3

typedef enum {
    TZ_SATA_SIG_ATA = 0x00000101,
    TZ_SATA_SIG_ATAPI = 0xEB140101,
    TZ_SATA_SIG_SEMB  = 0xC33C0101,
    TZ_SATA_SIG_PM    = 0x96690101
} TzSataDeviceSig;

typedef enum {
    TZ_AHCI_DEV_NULL   = 0,
    TZ_AHCI_DEV_SATA   = 1,
    TZ_AHCI_DEV_SEMB   = 2,
    TZ_AHCI_DEV_PM     = 3,
    TZ_AHCI_DEV_SATAPI = 4
} TzAhciDeviceType;


static int CheckType(TzAhciHbaPort *port) {
    UInt32 ssts = port->ssts;

    UInt8 ipm = (ssts >> 8) & 0x0F;
    UInt8 det = ssts & 0x0F;

    if (det != HBA_PORT_DET_PRESENT || ipm != HBA_PORT_IPM_ACTIVE)
        return TZ_AHCI_DEV_NULL;

    switch (port->sig) {
        case TZ_SATA_SIG_ATAPI:
            return TZ_AHCI_DEV_SATAPI;

        case TZ_SATA_SIG_SEMB:
            return TZ_AHCI_DEV_SEMB;

        case TZ_SATA_SIG_PM:
            return TZ_AHCI_DEV_PM;

        case TZ_SATA_SIG_ATA:
            // fallthrough
        default:
            return TZ_AHCI_DEV_SATA;
    }
}


void TzAhciProbePort(TzAhciHbaMem *abar) {
    // ports implemented
    UInt32 pi = abar->pi;

    UInt32 i;

    for (i = 0; i < 32; i++) {
        if (pi & 0x01) {
            Int dt = CheckType(&abar->ports[i]);
            switch (dt) {
                case TZ_AHCI_DEV_SATA:
                    TzPrintFormat("sata drive found at port %d!\n", i);
                    break;
                case TZ_AHCI_DEV_SATAPI:
                    TzPrintFormat("satapi drive found at port %d!\n", i);
                    break;
                default:
                    break;
            }
        }
        pi >>= 1;
    }
}