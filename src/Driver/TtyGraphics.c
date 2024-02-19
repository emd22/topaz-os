#include <Driver/TtyGraphics.h>

#include <Types.h>
#include <TzString.h>

#include <stdarg.h>

typedef struct {
    TzTextColor background_color;
    TzTextColor foreground_color;
    Int buffer_offset;
} TzGraphicsState;

static TzGraphicsState graphics_state;

void TzBackgroundColorSet(TzTextColor color) {
    graphics_state.background_color = color;
}

TzTextColor TzGetBackgroundColor() {
    return graphics_state.background_color;
}

void TzForegroundColorSet(TzTextColor color) {
    graphics_state.foreground_color = color;
}

TzTextColor TzGetForegroundColor() {
    return graphics_state.foreground_color;
}

void TzSetPosition(Int x, Int y) {
    graphics_state.buffer_offset = (y * 80) + x;
}

Int TzGetPosition() {
    return graphics_state.buffer_offset;
}

void TzInternPrintChar(char ch, int color) {
    if (ch == '\n') {
        TzSetPosition(0, TzGetPosition() / 80 + 1);
        return;
    }
    unsigned long video_addr = (0xB8000 + (TzGetPosition() * 2));
    char *video_buffer = (char *)video_addr;
    *(video_buffer++) = ch;
    *(video_buffer++) = color;
    graphics_state.buffer_offset++;

}

void TzInternPrintString(const char *_str, int color) {
    int i = 0;
    char ch;
    for (i = 0; (ch = _str[i]); i++) {

        TzInternPrintChar(ch, color);
    }
}


void TzDisplayClear() {
    TzSetPosition(0, 0);
    int i;
    for (i = 0; i < 80 * 25; i++) {
        TzInternPrintChar(' ', 0 | (TzGetBackgroundColor() << 4));
    }
    TzSetPosition(0, 0);

}

void TzPrintChar(Char ch) {
    TzInternPrintChar(ch, (TzGetBackgroundColor() << 4) | (TzGetForegroundColor() & 0x0F));
}


void TzPrintString(Char *str) {
    TzInternPrintString(str, (TzGetBackgroundColor() << 4) | (TzGetForegroundColor() & 0x0F));
}

void TzPrintInt(Int value) {
    Char buffer[16];
    TzIntToStr(value, buffer, 10);
    TzPrintString(buffer);
}

void TzPrintHex(Int value) {
    Char buffer[16];
    TzIntToStr(value, buffer, 16);
    TzPrintString(buffer);
}



void TzPrintFormatVArgs(const char *fmt, va_list args) {
    char ch;

    char tc;
    int ti;
    char *ts;

    while ((ch = *fmt++)) {
        if (ch == '%') {
            switch (ch = *fmt++) {
                case '%':
                    TzPrintChar('%');
                    break;
                case 'c':
                    tc = va_arg(args, int);
                    TzPrintChar(tc);
                    break;
                case 's':
                    ts = va_arg(args, char *);
                    TzPrintString(ts);
                    break;
                case 'd':
                    ti = va_arg(args, Int);
                    TzPrintInt(ti);
                    break;
                case 'x':
                    ti = va_arg(args, Int);
                    TzPrintHex(ti);
                    break;
            }
        }
        else {
            TzPrintChar(ch);
        }
    }
}


void TzPrintFormat(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    TzPrintFormatVArgs(fmt, args);

    va_end(args);
}
