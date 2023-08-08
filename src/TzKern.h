#ifndef TOPAZ_KERN_H
#define TOPAZ_KERN_H

#include <Types.h>

#include <stdarg.h>

#define TzPanic(str, ...) \
    { __TzPanic(__FILE_NAME__, __LINE__, str, __VA_ARGS__); }

void __TzPanic(Char *fname, UInt32 line, Char *str, ...);

#endif