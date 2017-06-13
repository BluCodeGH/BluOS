#include "kheap2.h"
#include "panic.h"

heap_header *heap_start;
heap_header *heap_end;

void init_heap(int addr) {
  heap_start = (heap_header *)addr;
  heap_end = heap_start;
}

char *nkmalloc(u32int sz) {
  heap_header *p = heap_start;
  while (p < heap_end) { //Will only exit if there is no free heap space
    if (p->used == 0 && p->length >= sz) { //Found a match
      p->used = 1;
      if (p->length > sz + (u32int)(char *)sizeof(heap_header) + (u32int)(char *)sizeof(heap_footer)) { //Break it up
        heap_footer *foot = (heap_footer *)((char *)p + p->length); //Store the old footer's position

        p->length = sz; //Update the found header

        heap_footer *footn = (heap_footer *)((char *)p + p->length); //Make a new footer
        footn->pointer = p;

        heap_header *headn = (heap_header *)((char *)footn + 1); //Make a new header
        headn->length = (char *)foot - (char *)(headn + 1);
        headn->used = 0;

        foot->pointer = headn; //Update old footer
      }
      return (char *)(p + 1); //Return
    } else {
      p = (heap_header *)((u32int)p + sizeof(heap_header) + p->length + sizeof(heap_footer));
    }
  }

  heap_end->used = 0; //Expand the heap
  heap_end->length = sz;
  heap_footer *fstart = (heap_footer *)((char *)heap_end + sizeof(heap_header) + sz);
  fstart->pointer = heap_end;
  heap_end = (heap_header *)(fstart + 1);
  return nkmalloc(sz); //Recurse (we know there will be space now)
}

void nkfree(char *start) {
  heap_header *head = (heap_header *)((u32int)start - sizeof(heap_header)); //Get given header and surrounding headers
  heap_footer *foot = (heap_footer *)(start + head->length);
  heap_header *headp = ((heap_footer *)((u32int)head - sizeof(heap_footer)))->pointer;
  heap_header *headn = (heap_header *)((char *)foot + sizeof(heap_footer));

  if (head > heap_start && headn < heap_end && headp->used == 0 && headn->used == 0) { //Can we expand forwards and backwards
    heap_footer *footn = (heap_footer *)((char *)headn + headn->length); //Get next footer
    footn->pointer = headp; //Point footer to header
    //Update length
    headp->length = headp->length + head->length + headn->length + 2*sizeof(heap_header) + 2*sizeof(heap_footer);
  } else if (head > heap_start && headp->used == 0) { //Only backwards?
    foot->pointer = headp; //Point footer to header
    headp->length = headp->length + head->length + sizeof(heap_header) + sizeof(heap_footer); //Update length
  } else if (headn < heap_end && headn->used == 0) { //Only forwards?
    heap_footer *footn = (heap_footer *)((char *)headn + headn->length); //Get next footer
    footn->pointer = head; //Point footer to header
    head->length = head->length + headn->length + sizeof(heap_header) + sizeof(heap_footer); //Update length
  }
  head->used = 0;
}