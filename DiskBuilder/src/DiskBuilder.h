//
// Created by emd22 on 02/06/23.
//

#ifndef TOPAZ_OS_DISKBUILDER_H
#define TOPAZ_OS_DISKBUILDER_H

#include <Types.h>

#define TZ_DELETED_DIR_MARKER 0xE5


typedef struct {
    UChar  oem_name[8];
    UInt16 bytes_per_sector;
    UInt8  sectors_per_cluster;
    UInt16 reserved_sectors_count;
    UInt8  number_of_fats;
    UInt16 root_entries_count;
    UInt16 total_sectors_count_16;
    UInt8  media_type;
    UInt16 sectors_per_fat_1x;
    UInt16 sectors_per_track;
    UInt16 heads_per_cylinder;
    UInt32 hidden_sectors_count;
    UInt32 total_sectors_count_32;
} __attribute__((packed)) TzFatBPB;

typedef struct {
    UInt32 sectors_per_fat32;
    UInt16 extended_flags;
    UInt16 fs_version;
    UInt32 root_dir_cluster_count;
    UInt16 fs_info_sector_count;
    UInt16 backup_boot_sector_count;
    UInt8  _resv0[12];
    UInt8  drive_number;
    UInt8  _resv1;
    UInt8  extended_boot_signature;
    UInt32 volume_serial_number;
    Int8   volume_label[11];
    Int8   file_system_name[8];
} __attribute__((packed)) TzFatEBPB;

typedef struct
{
    TzFatBPB bpb;
    TzFatEBPB ebpb;
} __attribute__((packed)) TzFatInfo;

typedef struct
{
    UChar    jump_bytes[3];
    TzFatInfo info;
    UChar    boot_code_32[0x1A4];
    UInt16   boot_signature;
} __attribute__((packed)) TzFatBootSector;

typedef enum
{
    FAT_DIR_ENTRY_READ_ONLY = 0x01,
    FAT_DIR_ENTRY_HIDDEN    = 0x02,
    FAT_DIR_ENTRY_SYSTEM    = 0x04,
    FAT_DIR_ENTRY_VOLUME_ID = 0x08,
    FAT_DIR_ENTRY_DIRECTORY = 0x10,
    FAT_DIR_ENTRY_ARCHIVE   = 0x20,
    FAT_DIR_ENTRY_LONG_NAME = FAT_DIR_ENTRY_READ_ONLY | FAT_DIR_ENTRY_HIDDEN | FAT_DIR_ENTRY_SYSTEM | FAT_DIR_ENTRY_VOLUME_ID
} TzFatDirEntryAttribute;

typedef struct
{
    Char   name[8];
    Char   extension[3];
    UInt8  attribute;
    UInt8  _resv0;
    UInt8  creation_time_sec_tenths;
    UInt16 creation_time_2_secs;
    UInt16 creation_date;
    UInt16 last_access_date;
    UInt16 first_cluster_hi;
    UInt16 last_write_time;
    UInt16 last_write_date;
    UInt16 first_cluster_lo;
    UInt32 size;
} __attribute__((packed)) TzFatDirEntry;


#endif //TOPAZ_OS_DISKBUILDER_H
