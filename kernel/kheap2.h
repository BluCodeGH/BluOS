#include "stddef.h"

typedef struct {
  u32int length:31;
  u32int used:1;
} heap_header;

typedef struct {
  heap_header *pointer;
} heap_footer;

void init_heap(int addr);

u32int nkmalloc(u32int sz);

u32int nkmalloc_a(u32int sz);

void nkfree(u32int start);

void print_heap();