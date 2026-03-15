extern void uart_puts(char *s);
extern char uart_getc();
extern void uart_putc(char c);
extern void uart_hex(unsigned int d);
#define BUF_SIZE 64
#define SBI_EXT_SET_TIMER 0x0 
#define SBI_EXT_SHUTDOWN  0x8 
#define SBI_EXT_BASE      0x10 //opensbi info

int strcmp(const char *s1, const char *s2) {
    // s1 not end and s1 = s2
    while (*s1 && (*s1 == *s2)) {  
        s1++;
        s2++;
    }  

    return *(unsigned char *)s1 - *(unsigned char *)s2;
}


enum sbi_ext_base_fid {
    SBI_EXT_BASE_GET_SPEC_VERSION,
    SBI_EXT_BASE_GET_IMP_ID,
    SBI_EXT_BASE_GET_IMP_VERSION,
    SBI_EXT_BASE_PROBE_EXT,
    SBI_EXT_BASE_GET_MVENDORID,
    SBI_EXT_BASE_GET_MARCHID,
    SBI_EXT_BASE_GET_MIMPID,
};

struct sbiret {
    long error;
    long value;
};

struct sbiret sbi_ecall(int ext,
                        int fid,
                        unsigned long arg0,
                        unsigned long arg1,
                        unsigned long arg2,
                        unsigned long arg3,
                        unsigned long arg4,
                        unsigned long arg5) {
    struct sbiret ret;
    register unsigned long a0 asm("a0") = (unsigned long)arg0;
    register unsigned long a1 asm("a1") = (unsigned long)arg1;
    register unsigned long a2 asm("a2") = (unsigned long)arg2;
    register unsigned long a3 asm("a3") = (unsigned long)arg3;
    register unsigned long a4 asm("a4") = (unsigned long)arg4;
    register unsigned long a5 asm("a5") = (unsigned long)arg5;
    register unsigned long a6 asm("a6") = (unsigned long)fid;
    register unsigned long a7 asm("a7") = (unsigned long)ext;
    asm volatile("ecall"
                 : "+r"(a0), "+r"(a1)
                 : "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7)
                 : "memory");
    ret.error = a0;
    ret.value = a1;
    return ret;
}


long sbi_get_spec_version(void) {
    struct sbiret ret = sbi_ecall(SBI_EXT_BASE, SBI_EXT_BASE_GET_SPEC_VERSION, 0, 0, 0, 0, 0, 0);
    //Return: 1 or an extension specific nonzero value if yes, 0 otherwise.
    return (ret.error == 0) ? ret.value : ret.error;
}

long sbi_get_impl_id(void) {
    struct sbiret ret = sbi_ecall(SBI_EXT_BASE, SBI_EXT_BASE_GET_IMP_ID, 0, 0, 0, 0, 0, 0);
    return (ret.error == 0) ? ret.value : ret.error;
}

long sbi_get_impl_version(void) {
    struct sbiret ret = sbi_ecall(SBI_EXT_BASE, SBI_EXT_BASE_GET_IMP_VERSION, 0, 0, 0, 0, 0, 0);
    return (ret.error == 0) ? ret.value : ret.error;
}

void shell() {
    char buffer[BUF_SIZE];
    int index = 0;

    uart_puts("opi-rv2> ");
    
    while (1) {
        char c = uart_getc();

        uart_putc(c);

        // Check for Enter key (which your uart_getc converts to '\n')
        if (c == '\n' || c == '\r') {
            buffer[index] = '\0'; // Null-terminate the string

            // Command Processing
            if (strcmp(buffer, "hello") == 0) {
                uart_puts("Hello world.\n");
            } 
            else if (strcmp(buffer, "help") == 0) {
                uart_puts("Available commands:\n");
                uart_puts("  help  - show all commands.\n");
                uart_puts("  hello - print Hello world.\n");
                uart_puts("  info  - print system info.\n");
            }else if (strcmp(buffer, "info") == 0) {
                uart_puts("System information:\n");
                
                uart_puts("  OpenSBI specification version: ");
                uart_hex(sbi_get_spec_version());
                uart_putc('\n');

                uart_puts("  implementation ID: ");
                uart_hex(sbi_get_impl_id());
                uart_putc('\n');

                uart_puts("  implementation version: ");
                uart_hex(sbi_get_impl_version());
                uart_putc('\n');
            }
            else if (index > 0) {
                uart_puts("Unknown command: ");
                uart_puts(buffer);
                uart_puts("\nUse help to get commands.\n");
            }

            // Reset for next command
            index = 0;
            uart_puts("opi-rv2> ");
        } 
        else {
            // Fill the buffer (prevent overflow)
            if (index < BUF_SIZE - 1) {
                buffer[index++] = c;
            }
        }
    }
}
