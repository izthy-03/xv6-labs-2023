// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

// map count of each physical page
int mapcount[(PHYSTOP - KERNBASE) >> PGSHIFT];

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  // printf("initing kernel mem\n");
  // printf("mapcount locates at %p\n", mapcount);
  // extern char etext[];
  // printf("etext at %p\n", (uint64)etext);
  // printf("end at %p\n", (uint64)end);
  memset(mapcount, 0, sizeof(mapcount));
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
// ***PS***: 
// For user virtual pages, the only way to access kfree() is uvmunmap() in vm.c
// But for kernel structs stored by pa(pagetables, etc.), they're freed by directly calling kfree() 
// instead of uvmunmap(). So just free it without checking mapcount(
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);
  mapcount[COUNTID(pa)] = 0;

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r){
    memset((char*)r, 5, PGSIZE);    // fill with junk
    mapcount[COUNTID(r)] = 1;  // initialize mapcount
  }
  return (void*)r;
}
