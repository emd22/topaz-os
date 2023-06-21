void printc(char ch, int offset, int color);
void prints(const char *str, int offset, int color);
void kmain(void);

// start of our 32 bit bootloader
extern void _start(void) {
    kmain();

    __asm__("cli");
    __asm__("hlt");
}

void kmain(void) {
    const char *str = "hello, world!";
    prints(str, 0, 12);
}


void printc(char ch, int offset, int color) {
    unsigned long video_addr = (0xB8000 + (offset * 2));
    char *video_buffer = (char *)video_addr;
    *(video_buffer++) = ch;
    *(video_buffer++) = color;
}

void prints(const char *_str, int offset, int color) {
    int i = 0;
    char ch;
    for (i = 0; (ch = _str[i]); i++) {
        printc(ch, offset + i, color);
    }
}