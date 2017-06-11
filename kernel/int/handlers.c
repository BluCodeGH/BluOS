#include "handlers.h"
#include "stddef.h"
#include "screen.h"

isr_t interrupt_handlers[256];

void register_interrupt_handler(u8int n, isr_t handler) {
  interrupt_handlers[n] = handler;
}

// This gets called from our ASM interrupt handler stub.
void isr_handler(registers_t regs) {
  if (interrupt_handlers[regs.int_no] != 0) {
    isr_t handler = interrupt_handlers[regs.int_no];
    handler(regs);
  } else {
    print("Unhandled Interrupt: ");
    print_dec(regs.int_no);
    print_char('\n');
  }
}

// This gets called from our ASM interrupt handler stub.
void irq_handler(registers_t regs) {
  // Send an EOI (end of interrupt) signal to the PICs.
  // If this interrupt involved the slave.
  if (regs.int_no >= 40) {
    // Send reset signal to slave.
    bout(0xA0, 0x20);
  }
  // Send reset signal to master. (As well as slave, if necessary).
  bout(0x20, 0x20);

  if (interrupt_handlers[regs.int_no] != 0) {
    isr_t handler = interrupt_handlers[regs.int_no];
    handler(regs);
  }
}