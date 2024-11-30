// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

int num_free_pages;  //store number of free pages
uint ref_count[(PHYSTOP-KERNBASE) >> PGSHIFT];

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
  
} kmem;

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  num_free_pages = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
  { 
    ref_count[V2P(p) >> PGSHIFT] = 0;          // initialse the reference count to 0
    kfree(p);
  }
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;
  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");
  // Fill with junk to catch dangling refs.
  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = (struct run*)v;
  if(ref_count[V2P(v) >> PGSHIFT] > 0)         // decrease reference count of page when freed
    ref_count[V2P(v) >> PGSHIFT] = ref_count[V2P(v) >> PGSHIFT] - 1;

  if(ref_count[V2P(v) >> PGSHIFT] == 0){       // Free page only if no references to the page
    memset(v, 1, PGSIZE);     // Fill garbage to catch dangling refs.
    r->next = kmem.freelist;
    num_free_pages = num_free_pages + 1;
    kmem.freelist = r;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;
  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
  {
      kmem.freelist = r->next;
      ref_count[V2P((char*)r) >> PGSHIFT] = 1;     // reference count page = one when allocated
      num_free_pages =num_free_pages - 1;
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

void incr_refcount(uint pa)
{
  if(pa >= PHYSTOP || pa < (uint)V2P(end))
    panic("invalid page no."); 
  acquire(&kmem.lock);
  ref_count[pa >> PGSHIFT] = ref_count[pa >> PGSHIFT] + 1;
  release(&kmem.lock);

}


void decr_refcount(uint pa)
{
  if(pa >= PHYSTOP || pa < (uint)V2P(end))
    panic("invalid page no."); 
  acquire(&kmem.lock);
  ref_count[pa >> PGSHIFT] = ref_count[pa >> PGSHIFT] - 1;
  release(&kmem.lock);

}

uint get_refcount(uint pa)
{
  if( pa >= PHYSTOP || pa < (uint)V2P(end))
    panic("getReferenceCount"); 
  return ref_count[pa >> PGSHIFT];

} 

