#include "screen.h"
#include "panic.h"
#include "kb.h"
#include "multiboot.h"
#include "int/descriptor_tables.h"
#include "paging.h"
#include "kheap.h"

void main(multiboot *mbd) {
  u32int memory_size_kb = 0;
  if (!(MULTIBOOT_FLAG_MEM & mbd->flags)) {
    PANIC("Unknown Memory Size");
  } else {
    memory_size_kb = 0x3FF + mbd->mem_upper;
  }
  init_descriptor_tables();
  initialise_paging(memory_size_kb);
  init_heap(); //Must be called after paging enable
  reset_cursor();
  print("Welcome to BluOS. Current Memory Size: ");
  print_dec(memory_size_kb / 1024);
  print(" MiB.\n");
  setup_kb();
  for(;;);
}