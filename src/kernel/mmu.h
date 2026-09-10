#ifndef MMU_H
#define MMU_H

void mmu_init(void);
unsigned long mmu_create_address_space(void);
void mmu_map_page(unsigned long cr3, unsigned long virt, unsigned long phys, int user);

#endif
