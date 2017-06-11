#include "mem.h"
#include "stddef.h"

void memcpy(u8int *source, u8int *dest, u32int no_bytes) {
  int i;
  for (i = 0; i < no_bytes; i++) {
    *(dest + i) = *(source + i);
  }
}

// Write len copies of val into dest.
void memset(u8int *dest, u8int val, u32int len) {
    u8int *temp = (u8int *)dest;
    for (; len != 0; len--) {
      *temp++ = val;
    }
}