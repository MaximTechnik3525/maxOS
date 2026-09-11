#include "malloc.h"
#include "pmm.h"

// Very simple block allocator over PMM
#define HEAP_PAGES 8192 // 32 MB kernel heap
static unsigned char* heap_base = 0;
static unsigned int heap_ptr = 0;

void malloc_init(void) {
    // Actually just use a simple bump allocator for kmalloc for now
    // A real allocator would use free lists
    heap_base = (unsigned char*)pmm_alloc_page();
    for (int i = 1; i < HEAP_PAGES; i++) {
        pmm_alloc_page(); // Allocates contiguous physical memory sequentially in our simple PMM
    }
}

// Simple bump allocator. Free is a no-op unless we implement a real free list.
void* kmalloc(unsigned int size) {
    if (size == 0) return 0;
    
    // Align to 8 bytes
    if (size % 8 != 0) {
        size += 8 - (size % 8);
    }
    
    if (heap_ptr + size > HEAP_PAGES * 4096) {
        return 0; // Out of heap memory
    }
    
    void* ptr = (void*)(heap_base + heap_ptr);
    heap_ptr += size;
    return ptr;
}

void kfree(void* ptr) {
    (void)ptr;
    // No-op for now
}
