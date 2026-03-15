
extern void shell(void);

#define SBI_EXT_SET_TIMER 0x0
#define SBI_EXT_SHUTDOWN  0x8
#define SBI_EXT_BASE      0x10



void start_kernel() {
    uart_puts("\nStarting kernel ...\n");

    shell();
}
