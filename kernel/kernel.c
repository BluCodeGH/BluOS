#include "screen.h"
#include "panic.h"
#include "kb.h"
#include "multiboot.h"
#include "int/descriptor_tables.h"
#include "paging.h"
#include "kheap2.h"

void main(multiboot *mbd) {
  u32int memory_size_kb = 0;
  if (!(MULTIBOOT_FLAG_MEM & mbd->flags)) {
    PANIC("Unknown Memory Size");
  } else {
    memory_size_kb = 0x3FF + mbd->mem_upper;
  }

  init_heap(0x1000000);
  init_descriptor_tables();
  initialise_paging(memory_size_kb);
  reset_cursor();
  print("Welcome to BluOS. Current Memory Size: ");
  print_dec(memory_size_kb / 1024);
  print(" MiB.\n");
  setup_kb();
  nkmalloc(8);
  print_heap();
  u32int a = nkmalloc_a(32);
  print_heap();
  nkmalloc(4);
  print_heap();
  nkfree(a);
  print_heap();
  nkmalloc_a(8);
  print_heap();
  for(;;);
}