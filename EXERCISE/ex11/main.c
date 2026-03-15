#define UART_BASE 0x10000000UL
#define UART_RBR  (unsigned char*)(UART_BASE + 0x0)
#define UART_THR  (unsigned char*)(UART_BASE + 0x0)
#define UART_LSR  (unsigned char*)(UART_BASE + 0x5)
#define LSR_DR    (1 << 0)
#define LSR_TDRQ  (1 << 5)

char uart_getc() {
    // Wait until the Data Ready (DR) bit is set to 1
    while (!(*UART_LSR & LSR_DR));
    
    // Read the character from the Receiver Buffer Register (RBR)
    return *UART_RBR;
}

void uart_putc(char c) {
    // Wait until the Transmit Data Register Empty (TDRQ) bit is set to 1
    while (!(*UART_LSR & LSR_TDRQ));
    
    // Write the character to the Transmitter Holding Register (THR)
    if(c=='\r') *UART_THR = '\n';
    else *UART_THR = c;
}

void uart_puts(const char* s) {
    while (*s) {
        uart_putc(*s++);
    }
}

void start_kernel() {
    uart_puts("\nStarting\n");
    while (1) {
        uart_putc(uart_getc());
    }
}
