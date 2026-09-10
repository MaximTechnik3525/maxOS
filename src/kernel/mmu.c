#include "mmu.h"
#include "debug.h"

__attribute__((aligned(4096)))
static unsigned long pml4[512];

__attribute__((aligned(4096)))
static unsigned long pdpt[512];

__attribute__((aligned(4096)))
static unsigned long pd_tables[4][512];

__attribute__((aligned(4096)))
static unsigned long pt_kernel[512]; // Maps 0 to 2MB using 4KB pages

void mmu_init(void) {
    debug_log("MMU", "Initializing 4KB paging structures in C...\n");

    for (int i = 0; i < 512; i++) {
        pml4[i] = 0;
        pdpt[i] = 0;
        for (int j = 0; j < 4; j++) {
            pd_tables[j][i] = 0;
        }
        pt_kernel[i] = 0;
    }

    // PML4[0] -> PDPT
    pml4[0] = ((unsigned long)pdpt) | 0x07; // Present, RW, User
    
    // PDPT[0..3] -> PD
    for (int i = 0; i < 4; i++) {
        pdpt[i] = ((unsigned long)pd_tables[i]) | 0x07;
    }

    // PD[0][0] -> pt_kernel (Maps 0 - 2MB using 4KB pages)
    pd_tables[0][0] = ((unsigned long)pt_kernel) | 0x07;

    // Map 0 - 2MB using 4KB pages
    // We intentionally unmap the first 4KB page (0x0) for NULL pointer protection
    // BUT wait, BIOS Data Area (BDA) and VGA Text Mode are in the first 1MB.
    // Let's just map everything from 0x1000 to avoid breaking real-mode leftovers if any,
    // actually just map from 0x0 to be safe for now, and protect NULL later if stable.
    for (int i = 0; i < 512; i++) {
        unsigned long addr = i * 4096;
        if (i == 0) {
            // Null pointer protection!
            pt_kernel[i] = 0;
        } else {
            pt_kernel[i] = addr | 0x07; 
        }
    }

    // Map the rest of the 4GB using 2MB huge pages
    for (int i = 1; i < 2048; i++) {
        unsigned long addr = i * 2097152ULL;
        pd_tables[i / 512][i % 512] = addr | 0x87; // Present, RW, User, Huge
    }

    // Load CR3
    asm volatile("mov %0, %%cr3" : : "r" (pml4));
    
    debug_log("MMU", "CR3 loaded. Null pointer protection active.\n");
}
