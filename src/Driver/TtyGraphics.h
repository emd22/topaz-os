#ifndef TOPAZ_TTY_GRAPHICS_H
#define TOPAZ_TTY_GRAPHICS_H

#include <Types.h>

#include <stdarg.h>

typedef enum {
    TZ_COLOR_BLACK,
    TZ_COLOR_BLUE,
    TZ_COLOR_GREEN,
    TZ_COLOR_CYAN,
    TZ_COLOR_RED,
    TZ_COLOR_PURPLE,
    TZ_COLOR_BROWN,
    TZ_COLOR_GRAY,
    TZ_COLOR_DARK_GRAY,
    TZ_COLOR_LIGHT_BLUE,
    TZ_COLOR_LIGHT_GREEN,
    TZ_COLOR_LIGHT_CYAN,
    TZ_COLOR_LIGHT_RED,
    TZ_COLOR_LIGHT_PURPLE,
    TZ_COLOR_YELLOW,
    TZ_COLOR_WHITE
} TzTextColor;

void TzBackgroundColorSet(TzTextColor color);
TzTextColor TzGetBackgroundColor();
void TzForegroundColorSet(TzTextColor color);
TzTextColor TzGetForegroundColor();

void TzSetPosition(Int x, Int y);
Int TzGetPosition();

void TzDisplayClear();
void TzPrintString(Char *str);

void TzPrintInt(Int value);
void TzPrintFormatVArgs(const char *fmt, va_list args);
void TzPrintFormat(const char *fmt, ...);

#endif