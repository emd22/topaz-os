#ifndef TOPAZ_OS_TYPES_H
#define TOPAZ_OS_TYPES_H

typedef enum {
    TZ_OK,
    TZ_ERROR,
} TzStatus;

#define NULL (void *)0

#define TRUE 1
#define FALSE 0

#define true TRUE
#define false FALSE

typedef unsigned long UInt32;
typedef UInt32 UInt;
typedef unsigned short UInt16;
typedef unsigned char  UInt8;
typedef unsigned char    UChar;

typedef long Int;
typedef long Int32;
typedef short Int16;
typedef char  Int8;
typedef Int8  Char;

typedef Int8 Bool;

#define TZ_PACK __attribute__((packed))

#endif