#ifndef TOPAZ_STRING_H
#define TOPAZ_STRING_H

#include <Types.h>

Int TzIntToStr(Int value, Char *buffer, Int radix);
Int TzStringLength(Char *str);
void *TzMemorySet(void *ptr, Int v, UInt32 n);

#endif