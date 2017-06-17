#include "kheap2.h"
#include "panic.h"
#include "screen.h"

extern void *end;
u32int placement_address = (u32int)&end;
heap_header *heap_start;
heap_header *heap_end;

#define PB(x) (((u32int)x + 0xFFF) & 0xFFFFF000)

void init_heap() {
  heap_start = (heap_header *)placement_address; //Start the heap where the old heap ends.
  heap_end = heap_start;
}

u32int _nkmalloc(u32int sz, u32int *phys) {
  if (heap_start != 0) { //Is heap initialised?
    heap_header *p = heap_start;
    while (p < heap_end) { //Will only exit if there is no free heap space
      if (p->used == 0 && p->length >= sz) { //Found a match
        p->used = 1;
        if (p->length > sz + sizeof(heap_header) + sizeof(heap_footer)) { //Break it up
          heap_footer *foot = (heap_footer *)((u32int)p + p->length + sizeof(heap_header)); //Store the old footer's position

          p->length = sz; //Update the found header

          heap_footer *footn = (heap_footer *)((u32int)p + p->length + sizeof(heap_header)); //Make a new footer
          footn->pointer = p;

          heap_header *headn = (heap_header *)(footn + 1); //Make a new header
          headn->length = (u32int)foot - (u32int)(headn + 1);
          headn->used = 0;

          foot->pointer = headn; //Update old footer
        }
        if (phys) {
          *phys = (u32int)(p + 1);
        }
        return (u32int)(p + 1); //Return
      } else {
        p = (heap_header *)((u32int)p + sizeof(heap_header) + p->length + sizeof(heap_footer));
      }
    }
    //Uh oh no free space that works
    heap_end->used = 0; //Expand the heap
    heap_end->length = sz;
    heap_footer *fstart = (heap_footer *)((u32int)heap_end + sizeof(heap_header) + sz);
    fstart->pointer = heap_end;
    heap_end = (heap_header *)(fstart + 1);
    return _nkmalloc(sz, phys); //Recurse (we know there will be space now)
  } else { //Heap not initialised yet
    if (phys) {
      *phys = placement_address;
    }
    u32int tmp = placement_address;
    placement_address += sz;
    return tmp;
  }
}

u32int _nkmalloc_a(u32int sz, u32int *phys) {
  if (heap_start != 0) { //Is heap initialised
    heap_header *p = heap_start;
    while (p < heap_end) { //Will only exit if there is no free heap space
      u32int before = PB(p + 1) - (u32int)(p + 1); //# bytes between p and the next page break
      if (p->length > before && p->length - before >= sz && p->used == 0) {
        //Stage 1
        heap_footer *footn = (heap_footer *)((u32int)p + p->length + sizeof(heap_header)); //Store final footer's position
        //We need these in both cases
        heap_footer *footp = (heap_footer *)(PB(p) - (sizeof(heap_header) + sizeof(heap_footer)));
        heap_header *head = (heap_header *)(footp + 1);

        if (before > sizeof(heap_header) + sizeof(heap_footer)) { //Break?
          p->length = ((u32int)footp - (u32int)p) - sizeof(heap_header); //Update length and footp
          footp->pointer = p;
        } else { //Expand
          //Get to-expand head
          heap_header *headpp = (heap_header *)(((heap_footer *)((u32int)p - sizeof(heap_footer)))->pointer); 
          
          headpp->length = ((u32int)footp - (u32int)headpp) - sizeof(heap_header); //Update header & footer
          footp->pointer = headpp;
        }
        //Stage 2: 'head' is in position
        head->used = 1; //Update head used
        if ((u32int)footn - PB(p + sizeof(heap_header)) >= sz + sizeof(heap_header) + sizeof(heap_footer)) { //Break?
          head->length = sz; //Update head length

          heap_footer *foot = (heap_footer *)((u32int)head + sz + sizeof(heap_header)); //Make new footer
          foot->pointer = head;

          heap_header *headn = (heap_header *)(foot + 1); //Make new header
          headn->used = 0;
          headn->length = (u32int)footn - (u32int)(headn + 1);

          footn->pointer = headn;
        } else {
          head->length = (u32int)footn - PB(p + sizeof(heap_header)); //Update used header and footer
          footn->pointer = head;
        }
        if (phys) {
          *phys = (u32int)(head + 1);
        }
        return (u32int)(head + 1);
      } else {
        p = (heap_header *)((u32int)p + sizeof(heap_header) + p->length + sizeof(heap_footer));
      }
    }
    //Uh oh no free space that works
    heap_end->used = 0; //Expand the heap
    heap_end->length = PB(heap_end + 1) - (u32int)(heap_end + 1) + sz;
    heap_footer *fstart = (heap_footer *)((u32int)heap_end + heap_end->length + sizeof(heap_header));
    fstart->pointer = heap_end;
    heap_end = (heap_header *)(fstart + 1);
    return _nkmalloc_a(sz, phys); //Recurse (we know there will be space now)
  } else {
    placement_address &= 0xFFFFF000; //Align the placement address;
    placement_address += 0x1000;
    if (phys) {
      *phys = placement_address;
    }
    u32int tmp = placement_address;
    placement_address += sz;
    return tmp;
  }
}

u32int nkmalloc_a(u32int sz) {
  return _nkmalloc_a(sz, 0);
}

u32int nkmalloc_p(u32int sz, u32int *phys) {
  return _nkmalloc(sz, phys);
}

u32int nkmalloc_ap(u32int sz, u32int *phys) {
  return _nkmalloc_a(sz, phys);
}

u32int nkmalloc(u32int sz) {
  return _nkmalloc(sz, 0);
}

void nkfree(u32int start) {
  heap_header *head = (heap_header *)(start - sizeof(heap_header)); //Get given header and surrounding headers
  heap_footer *foot = (heap_footer *)(start + head->length);
  heap_header *headp = ((heap_footer *)((u32int)head - sizeof(heap_footer)))->pointer;
  heap_header *headn = (heap_header *)((u32int)foot + sizeof(heap_footer));

  if (head > heap_start && headn < heap_end && headp->used == 0 && headn->used == 0) { //Can we expand forwards and backwards
    heap_footer *footn = (heap_footer *)((u32int)headn + headn->length); //Get next footer
    footn->pointer = headp; //Point footer to header
    //Update length
    headp->length = headp->length + head->length + headn->length + 2*sizeof(heap_header) + 2*sizeof(heap_footer);
  } else if (head > heap_start && headp->used == 0) { //Only backwards?
    foot->pointer = headp; //Point footer to header
    headp->length = headp->length + head->length + sizeof(heap_header) + sizeof(heap_footer); //Update length
  } else if (headn < heap_end && headn->used == 0) { //Only forwards?
    heap_footer *footn = (heap_footer *)((u32int)headn + headn->length); //Get next footer
    footn->pointer = head; //Point footer to header
    head->length = head->length + headn->length + sizeof(heap_header) + sizeof(heap_footer); //Update length
  }
  head->used = 0;
}

void print_heap() { //Generic heap inspector
  heap_header *p = heap_start;
  while (p < heap_end) {
    print_char('[');
    if (p->used == 1) {
      print_char('U');
    } else {
      print_char('E');
    }
    print_dec(p->length);
    print_char(']');
    p = (heap_header *)((u32int)p + sizeof(heap_header) + p->length + sizeof(heap_footer));
  }
  print_char('\n');
}