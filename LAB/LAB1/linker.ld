OUTPUT_ARCH(riscv)
ENTRY(_start)
SECTIONS
{
    . =  0x00200000; 
    _start = .;
    .text : {
        KEEP(*(.text.boot))
        . = ALIGN(8);    /* <--- 強制之後的 C 程式碼（如 start_kernel）對齊到 8-byte */
        *(.text)
        . = ALIGN(8);
    }
    .rodata : { . = ALIGN(8); *(.rodata) }
    .data   : { . = ALIGN(8); *(.data) }
    .bss    : {
        . = ALIGN(8);
        __bss_start = .;
        *(.sbss) *(.bss)
        . = ALIGN(8);
        __bss_stop = .;
    }
    . = ALIGN(16);
    . = . + 0x4000;
    _end = .;
}
