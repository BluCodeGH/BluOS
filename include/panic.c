#include "panic.h"

void panic(char *message, char *file, u32int line) {
    // We encountered a massive problem and have to stop.
    asm volatile("cli"); // Disable interrupts.

    print("PANIC(");
    print(message);
    print(") at ");
    print(file);
    print(":");
    print_dec(line);
    print("\n");
    // Halt by going into an infinite loop.
    for(;;);
}

void panic_assert(char *file, u32int line, char *desc) {
    // An assertion failed, and we have to panic.
    asm volatile("cli"); // Disable interrupts.

    print("ASSERTION-FAILED(");
    print(desc);
    print(") at ");
    print(file);
    print(":");
    print_dec(line);
    print("\n");
    // Halt by going into an infinite loop.
    for(;;);
}