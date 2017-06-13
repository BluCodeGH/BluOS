#include "stddef.h"

typedef struct {
  u32int used:1;
  u32int length:31;
} heap_header;

typedef struct {
  heap_header *pointer;
} heap_footer;

void init_heap(int addr);

char *nkmalloc(u32int sz);

void nkfree(char *start);