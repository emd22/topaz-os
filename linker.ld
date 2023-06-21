OUTPUT_FORMAT("binary")
ENTRY(_start)

phys = 0x00100000;
SECTIONS
{
    .text  0x100000 : {
      code = .; _code = .; __code = .;
      *(.text)
    }
    .data  : {
      data = .; _data = .; __data = .;
      *(.data)
    }
    .bss  :
    {
      bss = .; _bss = .; __bss = .;
      *(.bss)
    }
    end = .; _end = .; __end = .;
	/DISCARD/ :
	{
		*(.comment)
		*(.eh_frame)
		*(.note.gnu.build-id)
	}
}