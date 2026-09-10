#ifndef STRING_H
#define STRING_H

typedef unsigned long size_t;

// Memory operations
void* memset(void* dest, int val, size_t count);
void* memcpy(void* dest, const void* src, size_t count);
void* memmove(void* dest, const void* src, size_t count);
int   memcmp(const void* s1, const void* s2, size_t count);

// String operations
size_t strlen(const char* s);
int    strcmp(const char* s1, const char* s2);
int    strncmp(const char* s1, const char* s2, size_t n);
char*  strcpy(char* dest, const char* src);
char*  strncpy(char* dest, const char* src, size_t n);
char*  strcat(char* dest, const char* src);
char*  strncat(char* dest, const char* src, size_t n);
char*  strchr(const char* s, int c);
int    str_ends_with(const char* str, const char* suffix);

// Numerical conversions
void int_to_str(int num, char* str);
void uint_to_str(unsigned long long num, char* str, int base);

#endif // STRING_H
