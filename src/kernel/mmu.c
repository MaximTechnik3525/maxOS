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

#include "pmm.h"

unsigned long mmu_create_address_space(void) {
    unsigned long* new_pml4 = (unsigned long*)pmm_alloc_page();
    if (!new_pml4) return 0;
    
    // Copy kernel mappings (from the statically initialized pml4)
    for (int i = 0; i < 512; i++) {
        new_pml4[i] = pml4[i];
    }
    
    return (unsigned long)new_pml4;
}

void mmu_map_page(unsigned long cr3_addr, unsigned long virt, unsigned long phys, int user) {
    unsigned long* pml4_tbl = (unsigned long*)cr3_addr;
    
    int pml4_idx = (virt >> 39) & 0x1FF;
    int pdpt_idx = (virt >> 30) & 0x1FF;
    int pd_idx   = (virt >> 21) & 0x1FF;
    int pt_idx   = (virt >> 12) & 0x1FF;
    
    int flags = 0x03; // Present, RW
    if (user) flags |= 0x04;
    
    if ((pml4_tbl[pml4_idx] & 1) == 0) {
        unsigned long* new_pdpt = (unsigned long*)pmm_alloc_page();
        pml4_tbl[pml4_idx] = ((unsigned long)new_pdpt) | flags;
    }
    
    unsigned long* pdpt_tbl = (unsigned long*)(pml4_tbl[pml4_idx] & ~0xFFF);
    if ((pdpt_tbl[pdpt_idx] & 1) == 0) {
        unsigned long* new_pd = (unsigned long*)pmm_alloc_page();
        pdpt_tbl[pdpt_idx] = ((unsigned long)new_pd) | flags;
    }
    
    unsigned long* pd_tbl = (unsigned long*)(pdpt_tbl[pdpt_idx] & ~0xFFF);
    if ((pd_tbl[pd_idx] & 1) == 0) {
        // If it's a huge page (0x80 bit set), we can't map 4KB inside it!
        // But we only use huge pages in the kernel space. User space is empty.
        unsigned long* new_pt = (unsigned long*)pmm_alloc_page();
        pd_tbl[pd_idx] = ((unsigned long)new_pt) | flags;
    }
    
    unsigned long* pt_tbl = (unsigned long*)(pd_tbl[pd_idx] & ~0xFFF);
    pt_tbl[pt_idx] = (phys & ~0xFFF) | flags;
}
