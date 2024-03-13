#ifndef TOPAZ_KERN_H
#define TOPAZ_KERN_H

#include <Types.h>
#include <ArchX86.h>

#include <TzString.h>

#include <stdarg.h>

#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif

#define TzPanic(str, ...) \
    { __TzPanic(__FILE_NAME__, __LINE__, str, __VA_ARGS__); }

#define TzAssert(cond) \
    { if (!(cond)) { __TzAssertError(#cond, __FILE_NAME__, __LINE__); } }


void __TzAssertError(Char *cond, Char *fname, UInt32 line);
void __TzPanic(Char *fname, UInt32 line, Char *str, ...);

#endif