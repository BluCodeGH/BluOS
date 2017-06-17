#include "stddef.h"

typedef struct {
  u32int length:31;
  u32int used:1;
} heap_header;

typedef struct {
  heap_header *pointer;
} heap_footer;

void init_heap();

u32int _kmalloc(u32int sz, u32int *phys);

u32int _kmalloc_a(u32int sz, u32int *phys);

u32int kmalloc_a(u32int sz);

u32int kmalloc_p(u32int sz, u32int *phys);

u32int kmalloc_ap(u32int sz, u32int *phys);

u32int kmalloc(u32int sz);

void kfree(u32int start);

void print_heap();