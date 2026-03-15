#define UART_BASE  0xD4017000UL  // 0x10000000UL D4017300UL check address
#define UART_RBR  (unsigned char*)(UART_BASE + 0x0) //kryboard to cpu (read)
#define UART_THR  (unsigned char*)(UART_BASE + 0x0) //cpu to screen (write)
#define UART_LSR  (unsigned char*)(UART_BASE + 0x14) //state 
/*#define UART_BASE 0x10000000UL
#define UART_RBR  (unsigned char*)(UART_BASE + 0x0)
#define UART_THR  (unsigned char*)(UART_BASE + 0x0)
#define UART_LSR  (unsigned char*)(UART_BASE + 0x5)*/
#define LSR_DR    (1 << 0) //if 1 ,Data Ready in RBR
#define LSR_TDRQ  (1 << 5) //if 1 ,thr empty


//keyboard to cpu
char uart_getc() {
    while ((*UART_LSR & LSR_DR) == 0); // polling
    char c = (char)*UART_RBR;
    return c == '\r' ? '\n' : c;
}

//cpu to screen
void uart_putc(char c) {
    if (c == '\n')
        uart_putc('\r'); // ('\n' + '\r')

    while ((*UART_LSR & LSR_TDRQ) == 0); // polling

    *UART_THR = c; // display
}

//string
void uart_puts(const char* s) {
    while (*s)
        uart_putc(*s++);
}

//bin to hex
void uart_hex(unsigned long h) {
    uart_puts("0x");
    unsigned long n;
    for (int c = 60; c >= 0; c -= 4) {
        n = (h >> c) & 0xf;
        n += n > 9 ? 0x57 : '0'; //ABCDEF
        uart_putc(n);
    }
}
