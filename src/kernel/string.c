#include "string.h"

void* memset(void* dest, int val, size_t count) {
    unsigned char* d = (unsigned char*)dest;
    unsigned char v = (unsigned char)val;
    for (size_t i = 0; i < count; i++) {
        d[i] = v;
    }
    return dest;
}

void* memcpy(void* dest, const void* src, size_t count) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    for (size_t i = 0; i < count; i++) {
        d[i] = s[i];
    }
    return dest;
}

void* memmove(void* dest, const void* src, size_t count) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    if (d == s || count == 0) {
        return dest;
    }
    if (d < s) {
        for (size_t i = 0; i < count; i++) {
            d[i] = s[i];
        }
    } else {
        for (size_t i = count; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
    return dest;
}

int memcmp(const void* s1, const void* s2, size_t count) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    for (size_t i = 0; i < count; i++) {
        if (p1[i] != p2[i]) {
            return (int)p1[i] - (int)p2[i];
        }
    }
    return 0;
}

size_t strlen(const char* s) {
    size_t len = 0;
    while (s && s[len] != '\0') {
        len++;
    }
    return len;
}

int strcmp(const char* s1, const char* s2) {
    if (!s1 && !s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (int)(unsigned char)(*s1) - (int)(unsigned char)(*s2);
}

int strncmp(const char* s1, const char* s2, size_t n) {
    if (n == 0) return 0;
    if (!s1 && !s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;
    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i] || s1[i] == '\0') {
            return (int)(unsigned char)s1[i] - (int)(unsigned char)s2[i];
        }
    }
    return 0;
}

char* strcpy(char* dest, const char* src) {
    if (!dest) return dest;
    if (!src) {
        *dest = '\0';
        return dest;
    }
    size_t i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
    return dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    if (!dest || n == 0) return dest;
    size_t i = 0;
    if (src) {
        while (i < n && src[i] != '\0') {
            dest[i] = src[i];
            i++;
        }
    }
    while (i < n) {
        dest[i] = '\0';
        i++;
    }
    return dest;
}

char* strcat(char* dest, const char* src) {
    if (!dest) return dest;
    size_t dlen = strlen(dest);
    strcpy(dest + dlen, src);
    return dest;
}

char* strncat(char* dest, const char* src, size_t n) {
    if (!dest || n == 0) return dest;
    size_t dlen = strlen(dest);
    size_t i = 0;
    if (src) {
        while (i < n && src[i] != '\0') {
            dest[dlen + i] = src[i];
            i++;
        }
    }
    dest[dlen + i] = '\0';
    return dest;
}

char* strchr(const char* s, int c) {
    if (!s) return 0;
    char ch = (char)c;
    while (*s != '\0') {
        if (*s == ch) return (char*)s;
        s++;
    }
    if (ch == '\0') return (char*)s;
    return 0;
}

int str_ends_with(const char* str, const char* suffix) {
    if (!str || !suffix) return 0;
    size_t slen = strlen(str);
    size_t suflen = strlen(suffix);
    if (suflen > slen) return 0;
    return (strcmp(str + slen - suflen, suffix) == 0);
}

void int_to_str(int num, char* str) {
    if (!str) return;
    int i = 0;
    int is_neg = 0;

    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (num < 0) {
        is_neg = 1;
        // Handle INT_MIN safely without overflow
        if (num == -2147483648) {
            strcpy(str, "-2147483648");
            return;
        }
        num = -num;
    }

    while (num > 0) {
        str[i++] = (num % 10) + '0';
        num /= 10;
    }

    if (is_neg) {
        str[i++] = '-';
    }
    str[i] = '\0';

    // Reverse
    int len = i;
    for (int j = 0; j < len / 2; j++) {
        char tmp = str[j];
        str[j] = str[len - 1 - j];
        str[len - 1 - j] = tmp;
    }
}

void uint_to_str(unsigned long long num, char* str, int base) {
    if (!str || base < 2 || base > 16) return;
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    char buf[65];
    int p = 0;
    while (num > 0) {
        int rem = (int)(num % base);
        buf[p++] = (rem < 10) ? ('0' + rem) : ('A' + rem - 10);
        num /= base;
    }
    int out = 0;
    while (p > 0) {
        str[out++] = buf[--p];
    }
    str[out] = '\0';
}
