.section ".text.boot"
.global _start
_start:
    # 1. Set up the stack pointer
    # We point sp to _end (the top of our 16KB allocation)
    la sp, _end

    # 2. Clear the .bss section
    la t0, __bss_start
    la t1, __bss_stop
    bgeu t0, t1, 2f        # If start >= stop, skip
1:
    sb zero, (t0)          # Store 0 at address in t0
    addi t0, t0, 1         # Increment pointer
    bltu t0, t1, 1b        # Loop until we reach __bss_stop
2:

    # 3. Jump to the C entry point
    tail start_kernel
