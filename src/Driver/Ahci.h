//
// Created by emd22 on 05/09/23.
//

#ifndef TOPAZ_OS_AHCI_H
#define TOPAZ_OS_AHCI_H

#include <Types.h>


typedef enum {
    TZ_FIS_TYPE_REG_H2D   = 0x27,
    TZ_FIS_TYPE_REG_D2H   = 0x34,
    TZ_FIS_TYPE_DMA_ACT   = 0x39,
    TZ_FIS_TYPE_DMA_SETUP = 0x41,
    TZ_FIS_TYPE_DATA      = 0x46,
    TZ_FIS_TYPE_BIST      = 0x58,
    TZ_FIS_TYPE_PIO_SETUP = 0x5F,
    TZ_FIS_TYPE_DEV_BITS  = 0xA1,
} TzAhciFisType;


typedef struct {
    UInt8 fis_type;

    UInt8 pmport : 4;
    UInt8 _resv0 : 3;
    UInt8 c : 1;
    UInt8 command;
    UInt8 feature_lo;

    UInt8 lba0;
    UInt8 lba1;
    UInt8 lba2;

    UInt8 device;
    UInt8 lba3;
    UInt8 lba4;
    UInt8 lba5;

    UInt8 feature_hi;

    UInt8 count_lo;
    UInt8 count_hi;
    UInt8 icc;
    UInt8 control;

    UInt8 _resv1[4];
} TzAhciFisRegH2D;

typedef struct {
    UInt8 fis_type;

    UInt8 pmport : 4;
    UInt8 _resv0 : 2;
    UInt8 i : 1;
    UInt8 _resv1: 1;

    UInt8 status;
    UInt8 error;

    UInt8 lba0;
    UInt8 lba1;
    UInt8 lba2;
    UInt8 device;

    UInt8 lba3;
    UInt8 lba4;
    UInt8 lba5;
    UInt8 _resv2;

    UInt8 count_lo;
    UInt8 count_hi;
    UInt8 _resv3[2];

    UInt8 _resv4[4];
} TzAhciFisRegD2H;

typedef struct {
    UInt8 fis_type;
    UInt8 pmport : 4;
    UInt8 _resv0 : 4;

    UInt8 _resv1[2];

    UInt32 data[1];
} TzAhciFisData;


typedef struct {
    UInt8 fis_type;

    UInt8 pmport : 4;
    UInt8 _resv0 : 1;
    UInt8 d : 1;
    UInt8 i : 1;
    UInt8 _resv1 : 1;

    UInt8 status;
    UInt8 error;

    UInt8 lba0;
    UInt8 lba1;
    UInt8 lba2;

    UInt8 device;

    UInt8 lba3;
    UInt8 lba4;
    UInt8 lba5;
    UInt8 _resv2;

    UInt8 count_lo;
    UInt8 count_hi;
    UInt8 _resv3;
    UInt8 e_status;

    UInt16 tc;
    UInt8 _resv4[2];
} TzAhciPioSetup;

typedef struct tagFIS_DMA_SETUP
{
    // DWORD 0
    UInt8  fis_type;	// FIS_TYPE_DMA_SETUP

    UInt8  pmport : 4;	// Port multiplier
    UInt8  _resv0 : 1;		// Reserved
    UInt8  d : 1;		// Data transfer direction, 1 - device to host
    UInt8  i : 1;		// Interrupt bit
    UInt8  a : 1;            // Auto-activate. Specifies if DMA Activate FIS is needed

    UInt8  _resv1[2];       // Reserved

    //DWORD 1&2

    UInt32 dma_buffer_id_lo;    // DMA Buffer Identifier. Used to Identify DMA buffer in host memory.
    // SATA Spec says host specific and not in Spec. Trying AHCI spec might work.
    UInt32 dma_buffer_id_hi;

    //DWORD 3
    UInt32 _resv2;           //More reserved

    //DWORD 4
    UInt32 dma_buffer_offset;   //Byte offset into buffer. First 2 bits must be 0

    //DWORD 5
    UInt32 transfer_count;  //Number of bytes to transfer. Bit 0 must be 0

    //DWORD 6
    UInt32 _resv3;          //Reserved
} TzAhciDmaSetup;



typedef volatile struct {
    UInt32 clb;		// 0x00, command list base address, 1K-byte aligned
    UInt32 clbu;		// 0x04, command list base address upper 32 bits
    UInt32 fb;		// 0x08, FIS base address, 256-byte aligned
    UInt32 fbu;		// 0x0C, FIS base address upper 32 bits
    UInt32 is;		// 0x10, interrupt status
    UInt32 ie;		// 0x14, interrupt enable
    UInt32 cmd;		// 0x18, command and status
    UInt32 rsv0;		// 0x1C, Reserved
    UInt32 tfd;		// 0x20, task file data
    UInt32 sig;		// 0x24, signature
    UInt32 ssts;		// 0x28, SATA status (SCR0:SStatus)
    UInt32 sctl;		// 0x2C, SATA control (SCR2:SControl)
    UInt32 serr;		// 0x30, SATA error (SCR1:SError)
    UInt32 sact;		// 0x34, SATA active (SCR3:SActive)
    UInt32 ci;		// 0x38, command issue
    UInt32 sntf;		// 0x3C, SATA notification (SCR4:SNotification)
    UInt32 fbs;		// 0x40, FIS-based switch control
    UInt32 rsv1[11];	// 0x44 ~ 0x6F, Reserved
    UInt32 vendor[4];	// 0x70 ~ 0x7F, vendor specific
} TzAhciHbaPort;



typedef struct tagFIS_DEV_BITS
{
    // DWORD 0
    UInt8  fis_type;	// FIS_TYPE_DMA_SETUP (A1h)

    UInt8  rsv0:5;		// Reserved
    UInt8  r0:1;
    UInt8  i:1;
    UInt8  n:1;

    UInt8 statusl:3;
    UInt8 r1:1;
    UInt8 statush:3;

    UInt8 r2:1;
    UInt8 error;

    // DWORD 1
    UInt32 act;

} TzAhciFisDevBits;


typedef volatile struct {
    // 0x00 - 0x2B, Generic Host Control
    UInt32 cap;		// 0x00, Host capability
    UInt32 ghc;		// 0x04, Global host control
    UInt32 is;		// 0x08, Interrupt status
    UInt32 pi;		// 0x0C, Port implemented
    UInt32 vs;		// 0x10, Version
    UInt32 ccc_ctl;	// 0x14, Command completion coalescing control
    UInt32 ccc_pts;	// 0x18, Command completion coalescing ports
    UInt32 em_loc;		// 0x1C, Enclosure management location
    UInt32 em_ctl;		// 0x20, Enclosure management control
    UInt32 cap2;		// 0x24, Host capabilities extended
    UInt32 bohc;		// 0x28, BIOS/OS handoff control and status

    // 0x2C - 0x9F, Reserved
    UInt8  rsv[0xA0-0x2C];

    // 0xA0 - 0xFF, Vendor specific registers
    UInt8  vendor[0x100-0xA0];

    // 0x100 - 0x10FF, Port control registers
    TzAhciHbaPort	ports[1];	// 1 ~ 32
} TzAhciHbaMem;

typedef volatile struct tagHBA_FIS
{
    // 0x00
    TzAhciDmaSetup 	dsfis;		// DMA Setup FIS
    UInt8         pad0[4];

    // 0x20
    TzAhciPioSetup 	psfis;		// PIO Setup FIS
    UInt8         pad1[12];

    // 0x40
    TzAhciFisRegD2H rfis;		// Register – Device to Host FIS
    UInt8         pad2[4];

    // 0x58
    TzAhciFisDevBits sdbfis;		// Set Device Bit FIS

    // 0x60
    UInt8 ufis[64];

    // 0xA0
    UInt8 rsv[0x100-0xA0];
} TzAhciHbaFis;

typedef struct tagHBA_CMD_HEADER
{
    // DW0
    UInt8  cfl:5;		// Command FIS length in DWORDS, 2 ~ 16
    UInt8  a:1;		// ATAPI
    UInt8  w:1;		// Write, 1: H2D, 0: D2H
    UInt8  p:1;		// Prefetchable

    UInt8  r:1;		// Reset
    UInt8  b:1;		// BIST
    UInt8  c:1;		// Clear busy upon R_OK
    UInt8  rsv0:1;		// Reserved
    UInt8  pmp:4;		// Port multiplier port

    UInt16 prdtl;		// Physical region descriptor table length in entries

    // DW1
    volatile UInt32 prdbc;		// Physical region descriptor byte count transferred

    // DW2, 3
    UInt32 ctba;		// Command table descriptor base address
    UInt32 ctbau;		// Command table descriptor base address upper 32 bits

    // DW4 - 7
    UInt32 rsv1[4];	// Reserved
} TzAhciHbaCmdHeader;


typedef struct tagHBA_PRDT_ENTRY
{
    UInt32 dba;		// Data base address
    UInt32 dbau;		// Data base address upper 32 bits
    UInt32 rsv0;		// Reserved

    // DW3
    UInt32 dbc:22;		// Byte count, 4M max
    UInt32 rsv1:9;		// Reserved
    UInt32 i:1;		// Interrupt on completion
} TzHbaPrdtEntry;

typedef struct tagHBA_CMD_TBL
{
    // 0x00
    UInt8  cfis[64];	// Command FIS

    // 0x40
    UInt8  acmd[16];	// ATAPI command, 12 or 16 bytes

    // 0x50
    UInt8  rsv[48];	// Reserved

    // 0x80
    TzHbaPrdtEntry	prdt_entry[1];	// Physical region descriptor table entries, 0 ~ 65535
} TzHbaCmdTable;

void TzAhciProbePort(TzAhciHbaMem *abar);


#endif //TOPAZ_OS_AHCI_H
