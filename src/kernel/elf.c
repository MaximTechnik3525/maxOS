#include "elf.h"
#include "mmu.h"
#include "pmm.h"
#include "string.h"

// Basic ELF64 headers
typedef struct {
    unsigned char e_ident[16];
    unsigned short e_type;
    unsigned short e_machine;
    unsigned int e_version;
    unsigned long long e_entry;
    unsigned long long e_phoff;
    unsigned long long e_shoff;
    unsigned int e_flags;
    unsigned short e_ehsize;
    unsigned short e_phentsize;
    unsigned short e_phnum;
    unsigned short e_shentsize;
    unsigned short e_shnum;
    unsigned short e_shstrndx;
} Elf64_Ehdr;

typedef struct {
    unsigned int p_type;
    unsigned int p_flags;
    unsigned long long p_offset;
    unsigned long long p_vaddr;
    unsigned long long p_paddr;
    unsigned long long p_filesz;
    unsigned long long p_memsz;
    unsigned long long p_align;
} Elf64_Phdr;

int elf_load(const unsigned char* elf_data, unsigned long* entry_point, unsigned long target_cr3) {
    Elf64_Ehdr* hdr = (Elf64_Ehdr*)elf_data;
    
    // Check magic
    if (hdr->e_ident[0] != 0x7F || hdr->e_ident[1] != 'E' || 
        hdr->e_ident[2] != 'L' || hdr->e_ident[3] != 'F') {
        return -1; // Not an ELF
    }
    
    // Must be 64-bit (Class 2)
    if (hdr->e_ident[4] != 2) return -2;

    Elf64_Phdr* phdr = (Elf64_Phdr*)(elf_data + hdr->e_phoff);
    
    for (int i = 0; i < hdr->e_phnum; i++) {
        if (phdr[i].p_type == 1) { // PT_LOAD
            unsigned long vaddr = phdr[i].p_vaddr;
            unsigned long memsz = phdr[i].p_memsz;
            unsigned long filesz = phdr[i].p_filesz;
            
            // Map the required pages
            unsigned long num_pages = (memsz + 4095) / 4096;
            for (unsigned long p = 0; p < num_pages; p++) {
                unsigned long virt = (vaddr & ~0xFFF) + (p * 4096);
                unsigned long phys = (unsigned long)pmm_alloc_page();
                if (!phys) return -3; // OOM
                mmu_map_page(target_cr3, virt, phys, 1); // User accessible
                
                // Copy data or zero BSS
                unsigned char* dest = (unsigned char*)phys; // In current mapping (kernel), phys == virt roughly, but wait!
                // Ah, kernel runs with 2MB identity mapping, so phys pointer is directly accessible!
                unsigned long offset_in_page = (p == 0) ? (vaddr & 0xFFF) : 0;
                unsigned long copy_size = 4096 - offset_in_page;
                
                unsigned long file_offset = phdr[i].p_offset + (p * 4096) - (vaddr & 0xFFF);
                if (p == 0) file_offset = phdr[i].p_offset;
                
                for (unsigned long b = 0; b < 4096; b++) dest[b] = 0; // Zero page first
                
                if ((p * 4096) < filesz) {
                    unsigned long to_copy = filesz - (p * 4096);
                    if (to_copy > copy_size) to_copy = copy_size;
                    memcpy(dest + offset_in_page, elf_data + file_offset, to_copy);
                }
            }
        }
    }
    
    *entry_point = hdr->e_entry;
    return 0;
}
