ENTRY(_start)

MEMORY
{
    DRAM (rwx) : ORIGIN = 0x00001000, LENGTH = 0x0003f000
}

SECTIONS
{
    .text : {
        KEEP(*(.text.start))
        *(.text*)
        *(.rodata*)
    } > DRAM

    .data : ALIGN(4) {
        *(.sdata*)
        *(.data*)
    } > DRAM

    .maps_data 0x00002000 : ALIGN(4) {
        KEEP(*(.maps_data))
    } > DRAM

    .bss : ALIGN(4) {
        . = ALIGN(4);
        __bss_start = .;
        *(.sbss*)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > DRAM

    . = ALIGN(16);
    . = ORIGIN(DRAM) + LENGTH(DRAM);
    _stack_top = .;
}
