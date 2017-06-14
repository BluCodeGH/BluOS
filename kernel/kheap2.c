#include "kheap2.h"
#include "panic.h"

heap_header *heap_start;
heap_header *heap_end;

void init_heap(int addr) {
  heap_start = (heap_header *)addr;
  heap_end = heap_start;
}

u32int nkmalloc(u32int sz, u32int *phys) {
  heap_header *p = heap_start;
  while (p < heap_end) { //Will only exit if there is no free heap space
    if (p->used == 0 && p->length >= sz) { //Found a match
      p->used = 1;
      if (p->length > sz + sizeof(heap_header) + sizeof(heap_footer)) { //Break it up
        heap_footer *foot = (heap_footer *)((u8int)p + p->length); //Store the old footer's position

        p->length = sz; //Update the found header

        heap_footer *footn = (heap_footer *)((u8int)p + p->length); //Make a new footer
        footn->pointer = p;

        heap_header *headn = (heap_header *)(footn + 1); //Make a new header
        headn->length = (u8int)foot - (u8int)(headn + 1);
        headn->used = 0;

        foot->pointer = headn; //Update old footer
      }
      return (u32int)(p + 1); //Return
    } else {
      p = (heap_header *)((u8int)p + sizeof(heap_header) + p->length + sizeof(heap_footer));
    }
  }

  heap_end->used = 0; //Expand the heap
  heap_end->length = sz;
  heap_footer *fstart = (heap_footer *)((u8int)heap_end + sizeof(heap_header) + sz);
  fstart->pointer = heap_end;
  heap_end = (heap_header *)(fstart + 1);
  return nkmalloc(sz, u32int *phys); //Recurse (we know there will be space now)
}

u32int nkmalloc_a(u32int sz, u32int *phys) {
  heap_header *p = heap_start;
  while (p < heap_end) { //Will only exit if there is no free heap space
    if (p->used == 0 && p->length >= ((!((u8int)p + sizeof(heap_header)) + 1) & 0x00000FFF) + sz) { //Found a match
      if ((!(u8int)p + 1) & 0x00000FFF > sizeof(heap_header) + sizeof(heap_footer)) { //Break it up
        heap_footer *foot = (heap_footer *)((u8int)p + p->length); //Store final footer's position
        //Update length to 'up to' the page boundary.
        p->length = ((!(u8int)p + 1) & 0x00000FFF) - sizeof(heap_header) - sizeof(heap_footer);

        heap_footer *footp = ((u8int)p + p->length + sizeof(heap_header)); //Make new footer
        footp->pointer = p;

        heap_header *head = (heap_header *)(footp + 1); //Make a new header which is the actual used one
        if ((u8int)foot - (u8int)(head + 1) > sz + sizeof(heap_header) + sizeof(heap_footer)) { //Break it up again
          head->length = sz; //Setup used head
          head->used = 1;

          heap_footer *footr = ((u8int)head + sz + sizeof(heap_header)); //Make new footer
          footr->pointer = head;

          heap_header *headn = (heap_header *)(footr + 1); //Make a new header
          headn->length = (u8int)foot - (u8int)(headn + 1);
          headn->used = 0;

          foot->pointer = headn;
        } else {
          head->length = (u8int)foot - (u8int)(head + 1);
          head->used = 1;

          foot->pointer = head; //Update old footer.
        }
      } else { //Expand previous instead
        //Create new header with data starting at page boundary
        heap_header *head = (heap_header *)((((u8int)p + 0x1000) & 0xFFFFF000) - sizeof(heap_header));

      }
    } else {
      p = (heap_header *)((u8int)p + sizeof(heap_header) + p->length + sizeof(heap_footer));
    }
  }
}

void nkfree(u32int start) {
  heap_header *head = (heap_header *)(start - sizeof(heap_header)); //Get given header and surrounding headers
  heap_footer *foot = (heap_footer *)(start + head->length);
  heap_header *headp = ((heap_footer *)((u8int)head - sizeof(heap_footer)))->pointer;
  heap_header *headn = (heap_header *)((u8int)foot + sizeof(heap_footer));

  if (head > heap_start && headn < heap_end && headp->used == 0 && headn->used == 0) { //Can we expand forwards and backwards
    heap_footer *footn = (heap_footer *)((u8int)headn + headn->length); //Get next footer
    footn->pointer = headp; //Point footer to header
    //Update length
    headp->length = headp->length + head->length + headn->length + 2*sizeof(heap_header) + 2*sizeof(heap_footer);
  } else if (head > heap_start && headp->used == 0) { //Only backwards?
    foot->pointer = headp; //Point footer to header
    headp->length = headp->length + head->length + sizeof(heap_header) + sizeof(heap_footer); //Update length
  } else if (headn < heap_end && headn->used == 0) { //Only forwards?
    heap_footer *footn = (heap_footer *)((u8int)headn + headn->length); //Get next footer
    footn->pointer = head; //Point footer to header
    head->length = head->length + headn->length + sizeof(heap_header) + sizeof(heap_footer); //Update length
  }
  head->used = 0;
}