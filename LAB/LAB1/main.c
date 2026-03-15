 
extern void shell(void);
extern void uart_puts(char *s);


void start_kernel() {
    uart_puts("\nStarting ...\n");
    shell();
}
