#include "DiskBuilder.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define TZ_DB_MAX_PATH 256


#define TZ_DB_MEDIA_TYPE_FLOPPY 0xF0
#define TZ_DB_MEDIA_TYPE_HDD 0xF8

typedef struct {
    FILE *output_file;
    char output_path[TZ_DB_MAX_PATH];
    char bootstrap_path[TZ_DB_MAX_PATH];
} DiskBuilderState;

typedef struct {
    char *str;
} ArgParams;

typedef struct {
    char arg_code[32];
    void (*callback)(ArgParams *arg);
    char description[1024];
} ArgEntry;


/*
 * NOTE: keep in mind that static instances in C are automatically
 * zeroed out, therefore everything is initialized.
 * */
static DiskBuilderState state;

void TzError(char *fmt, ...) {
    if (state.output_file != NULL) {
        fclose(state.output_file);
    }
    // remove the output file if it exists
    remove(state.output_path);

    // TODO: add warning, info, debug, etc.
    FILE *stream = stdout;
    const char *error_type_str = "ERROR";

    va_list args;
    va_start(args, fmt);

    fprintf(stream, "[%s]: ", error_type_str);
    vfprintf(stream, fmt, args);
    // NOTE: fputs does not append a newline character like puts.
    fputs("\n", stream);

    va_end(args);

    exit(EXIT_FAILURE);
}


void ChangeOutputPath(ArgParams *value) {
    if (!strncpy(state.output_path, value->str, TZ_DB_MAX_PATH)) {
        TzError("could not change disk output path");
    }
}

void ChangeBootstrapPath(ArgParams *value) {
    if (!strncpy(state.bootstrap_path, value->str, TZ_DB_MAX_PATH)) {
        TzError("could not change bootstrap path");
    }
}


const ArgEntry arg_table[] = {
    {
        "-o",
        ChangeOutputPath,
        "change the output path of the generated disk image"
    },
    {
        "-bs",
        ChangeBootstrapPath,
        "change the path to the bootstrap sector"
    },
};


void PrintCommandHelp(char *program_name) {
    printf("Usage: %s [options] <program file>\n", "DiskBuilder");

    int arg_table_size = sizeof(arg_table) / sizeof(ArgEntry);

    puts("Options:");

    const int arg_column_width = 8;

    int i;
    for (i = 0; i < arg_table_size; i++) {
        ArgEntry *arg = &arg_table[i];

        int padding_amount = arg_column_width - (int)strlen(arg->arg_code);
        printf("\t%s", arg->arg_code);
        while (padding_amount-- > 0) {
            printf(" ");
        }
        printf("%s\n", arg->description);
    }
}

void ParseArgument(char *argname, char *value) {
    int arg_table_size = sizeof(arg_table) / sizeof(ArgEntry);

    int i;
    for (i = 0; i < arg_table_size; i++) {
        ArgEntry *arg = &arg_table[i];

        if (!strcmp(arg->arg_code, argname)) {
            ArgParams params;
            params.str = value;
            arg->callback(&params);
            return;
        }
    }
    TzError("Invalid argument specified [%s]", argname);
}


void ParseArgs(int argc, char **argv) {
    if (argc == 1) {
        PrintCommandHelp(argv[0]);
        return;
    }
    int i = 1;
    for (; i < argc; i++) {
        char *arg = argv[i];
        if (!strcmp(arg, "--help")) {
            PrintCommandHelp(argv[0]);
        }

        ParseArgument(arg, (i + 1 < argc) ? argv[i + 1] : NULL);
    }
}


void TzInitFatValues(TzFatBootSector *bs) {
    // TODO: base values off of disk image size

    // set up bpb
    const char *oem_identifier = "TOPAZ1.0";

    memcpy(bs->info.bpb.oem_name, oem_identifier, 8);

    bs->info.bpb.bytes_per_sector = 512; /* most drives operate on 512 bps */
    bs->info.bpb.sectors_per_cluster = 1; /* TODO: when disk sizes increase, increase sectors per cluster */
    bs->info.bpb.reserved_sectors_count = 1; /* just our bootsector for now */
    bs->info.bpb.number_of_fats = 2;         /* default with FAT32 */
    bs->info.bpb.root_entries_count = 224;   /* 16 entries, 1 512 byte sector */
    bs->info.bpb.total_sectors_count_16 = 0;
    bs->info.bpb.media_type = TZ_DB_MEDIA_TYPE_HDD;
    bs->info.bpb.sectors_per_fat_1x = 0;
    bs->info.bpb.sectors_per_track = 63;
    bs->info.bpb.heads_per_cylinder = 16;
    bs->info.bpb.hidden_sectors_count = 0;
    bs->info.bpb.total_sectors_count_32 = 0; /* increase when we have larger disk sizes */
    // set up ebr bpb
    bs->info.ebpb.drive_number = 0x3F;
    bs->info.ebpb.extended_flags = 0;
    bs->info.ebpb.extended_boot_signature = 0x28;
    bs->info.ebpb.volume_serial_number = 0;
//    bs->info.ebpb.volume_label =

}

void TzInitializeDisk(void) {
    if (state.bootstrap_path[0] == 0)
        return;
    FILE *bs_fp = fopen(state.bootstrap_path, "rb");
    if (!bs_fp) {
        TzError("could not open bootstrap file from path %s", state.bootstrap_path);
    }

    TzFatBootSector boot_sector;

    fread(&boot_sector, sizeof(boot_sector), bs_fp);
    fclose(bs_fp);


    FILE *out_fp = fopen(state.output_path, "wb");


}


int main(int argc, char *argv[]) {
    ParseArgs(argc, argv);
}
