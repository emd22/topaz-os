#include <TzString.h>

Int TzIntToStr(Int value, Char *buffer, Int radix) {
    Char tmp[16];
    Char *tp = tmp;
    Int i;

    Bool sign = (radix == 10 && value < 0);

    UInt v = (UInt)value;

    if (sign) {
        v = -value;
    }

    while (v || tp == tmp) {
        i = v % radix;
        v /= radix;
        if (i < 10)
            *tp++ = i + '0';
        else
            *tp++ = i + 'A' - 10;
    }

    int len = tp - tmp;

    if (sign) {
        *buffer++ = '-';
        len++;
    }

    while (tp > tmp)
        *buffer++ = *--tp;

    *buffer++ = 0;

    return len;
}

Int TzStringLength(Char *str) {
    Int len = 0;

    while (*(str++))
        len++;

    return len;
}



void *TzMemorySet(void *ptr, Int v, UInt32 n) {
    if (!n)
        return ptr;

    Char *p = ptr;
    do {
        *p++ = v;
    } while(--n);

    return ptr;
}

void *TzMemoryCopy(void *dest, const void *src, UInt32 count) {
    char *pdest = (Char *)dest;
    const char *psrc = (const char *)src;

    if ((pdest != NULL) && (psrc != NULL)) {
        while (count) {
            *(pdest++) = *(psrc++);
            --count;
        }
    }
    return dest;
}