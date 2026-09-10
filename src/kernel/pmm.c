#include "pmm.h"
#include "debug.h"

// Bitmap for physical memory. Max 4GB = 1M pages = 131072 bytes (128 KB bitmap)
static unsigned char pmm_bitmap[131072];
static unsigned long pmm_total_pages = 0;
static unsigned long pmm_free_pages = 0;

void pmm_init(unsigned int mem_upper_kb) {
    // mem_upper_kb starts from 1MB. Total memory = (mem_upper_kb + 1024) * 1024 bytes.
    unsigned long total_memory_bytes = (unsigned long)(mem_upper_kb + 1024) * 1024;
    pmm_total_pages = total_memory_bytes / 4096;
    if (pmm_total_pages > 1048576) {
        pmm_total_pages = 1048576; // Max 4GB for now
    }

    // Mark everything as used initially
    for (unsigned int i = 0; i < 131072; i++) {
        pmm_bitmap[i] = 0xFF;
    }

    // Free pages from 16MB up to total memory
    // 16MB / 4096 = 4096th page.
    unsigned long start_page = 4096;
    
    for (unsigned long i = start_page; i < pmm_total_pages; i++) {
        pmm_bitmap[i / 8] &= ~(1 << (i % 8));
        pmm_free_pages++;
    }
    
    debug_log("PMM", "Physical Memory Manager initialized.\n");
}

void* pmm_alloc_page(void) {
    for (unsigned long i = 4096; i < pmm_total_pages; i++) {
        if ((pmm_bitmap[i / 8] & (1 << (i % 8))) == 0) {
            pmm_bitmap[i / 8] |= (1 << (i % 8));
            pmm_free_pages--;
            
            // Clear the allocated page
            unsigned char* ptr = (unsigned char*)(i * 4096);
            for (int j = 0; j < 4096; j++) {
                ptr[j] = 0;
            }
            
            return (void*)(i * 4096);
        }
    }
    debug_log("PMM", "OUT OF PHYSICAL MEMORY!\n");
    return 0; // Null
}

void pmm_free_page(void* ptr) {
    unsigned long addr = (unsigned long)ptr;
    unsigned long i = addr / 4096;
    if (i >= 4096 && i < pmm_total_pages) {
        if (pmm_bitmap[i / 8] & (1 << (i % 8))) {
            pmm_bitmap[i / 8] &= ~(1 << (i % 8));
            pmm_free_pages++;
        }
    }
}
