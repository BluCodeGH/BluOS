#include "stddef.h"

typedef struct {
  u32int length:31;
  u32int used:1;
} heap_header;

typedef struct {
  heap_header *pointer;
} heap_footer;

void init_heap();

u32int _nkmalloc(u32int sz, u32int *phys);

u32int _nkmalloc_a(u32int sz, u32int *phys);

u32int nkmalloc_a(u32int sz);

/**
   Allocate a chunk of memory, sz in size. The physical address
   is returned in phys. Phys MUST be a valid pointer to u32int!
**/
u32int nkmalloc_p(u32int sz, u32int *phys);

/**
   Allocate a chunk of memory, sz in size. The physical address 
   is returned in phys. It must be page-aligned.
**/
u32int nkmalloc_ap(u32int sz, u32int *phys);

/**
   General allocation function.
**/
u32int nkmalloc(u32int sz);

void nkfree(u32int start);

void print_heap();