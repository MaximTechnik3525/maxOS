#ifndef ELF_H
#define ELF_H

int elf_load(const unsigned char* elf_data, unsigned long* entry_point, unsigned long target_cr3);

#endif
