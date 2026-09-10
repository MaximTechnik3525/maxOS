#ifndef PMM_H
#define PMM_H

void pmm_init(unsigned int mem_upper_kb);
void* pmm_alloc_page(void);
void pmm_free_page(void* ptr);

#endif
