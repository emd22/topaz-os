#include <TzKern.h>
#include <TzDriverRegistry.h>
#include <ArchX86.h>

#include <Driver/TtyGraphics.h>


void __TzPanic(Char *fname, UInt32 line, Char *str, ...) {
    UInt32 ip = (UInt32)__builtin_return_address(0);
//    __asm__("mov %%esp, %0": "=r"(ip));

    TzDriverRegistryHaltAll();
    TzInterruptsDisable();

    TzBackgroundColorSet(TZ_COLOR_RED);
    TzDisplayClear();

    TzForegroundColorSet(TZ_COLOR_YELLOW);
    TzPrintFormat(">>> PANIC at 0x%x! [ %s : %d ] <<<\n", ip, fname, line);

    TzForegroundColorSet(TZ_COLOR_WHITE);

    TzPrintString("Err: ");

    va_list args;
    va_start(args, str);
    TzPrintFormatVArgs(str, args);
    va_end(args);
    TzPrintString("\n");

    TzHalt();
    while (1);
}