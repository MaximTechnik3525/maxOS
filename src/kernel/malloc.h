#ifndef MALLOC_H
#define MALLOC_H

void malloc_init(void);
void* kmalloc(unsigned int size);
void kfree(void* ptr);

#endif
