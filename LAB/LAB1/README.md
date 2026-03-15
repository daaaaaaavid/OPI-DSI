# Lab 1: Hello World

## Goals
- Bare-metal Programming: Develop code that runs directly on hardware without an underlying OS.
- Peripheral Access: Learn to interact with hardware via Memory-Mapped I/O (MMIO).
- Communication: Establish a serial link between the Orange Pi RV2 and a host machine.

## Basic Initialization
Task: Configure the system state immediately after the bootloader hands over control.
- Stack Pointer: Manually set the stack pointer to a valid memory address.
- BSS Zeroing: Write a routine to zero out the .bss segment to prevent undefined behavior from uninitialized variables.
- Entry Point: Ensure the Program Counter (PC) starts at the correct memory address via a linker script.

## UART Setup
Task: Implement serial communication by directly accessing UART memory-mapped registers.
- Status Polling: Check line status flags to determine if the transmitter is empty or if data is available to read.
- I/O Functions: Implement basic get_char() and put_char() logic.
- Refer to sections 6.2 and 16.3 of https://github.com/nycu-caslab/OSC2026/raw/main/references/K1_User_Manual_(V6.1_2025.08.06).pdf for base addresses and register layouts.

## Simple Shell
Task: Create a minimal interactive interface via the UART connection.
- Commands: Support help, info and hello (print "Hello World!").

## System Information & SBI
Task: Use the Supervisor Binary Interface (SBI) to query firmware-level information.
- sbi_ecall Function: Implement a wrapper using inline assembly to load registers a0–a7 and execute the ecall instruction.
- Info Command: Use SBI_EXT_BASE (ID 0x10) to retrieve and display:
  - OpenSBI Specification Version
  - Implementation ID & Version