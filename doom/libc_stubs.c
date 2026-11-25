#include <stddef.h>
#include <stdarg.h>

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    return dest;
}

void* memmove(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    if (d < s) {
        for (size_t i = 0; i < n; i++) d[i] = s[i];
    } else if (d > s) {
        for (size_t i = n; i > 0; i--) d[i - 1] = s[i - 1];
    }
    return dest;
}

void* memset(void* s, int c, size_t n) {
    unsigned char* p = (unsigned char*)s;
    for (size_t i = 0; i < n; i++) p[i] = (unsigned char)c;
    return s;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) return p1[i] - p2[i];
    }
    return 0;
}

size_t strlen(const char* s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

char* strncpy(char* dest, const char* src, size_t n) {
    size_t i = 0;
    for (; i < n && src[i]; i++) dest[i] = src[i];
    for (; i < n; i++) dest[i] = '\0';
    return dest;
}

char* strcat(char* dest, const char* src) {
    char* d = dest + strlen(dest);
    while ((*d++ = *src++));
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i]) return (unsigned char)s1[i] - (unsigned char)s2[i];
        if (s1[i] == '\0') return 0;
    }
    return 0;
}

int strcasecmp(const char* s1, const char* s2) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    while (*p1 && *p2) {
        int c1 = (*p1 >= 'A' && *p1 <= 'Z') ? (*p1 + 32) : *p1;
        int c2 = (*p2 >= 'A' && *p2 <= 'Z') ? (*p2 + 32) : *p2;
        if (c1 != c2) return c1 - c2;
        p1++; p2++;
    }
    int c1 = (*p1 >= 'A' && *p1 <= 'Z') ? (*p1 + 32) : *p1;
    int c2 = (*p2 >= 'A' && *p2 <= 'Z') ? (*p2 + 32) : *p2;
    return c1 - c2;
}

int strncasecmp(const char* s1, const char* s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    for (size_t i = 0; i < n; i++) {
        int c1 = (p1[i] >= 'A' && p1[i] <= 'Z') ? (p1[i] + 32) : p1[i];
        int c2 = (p2[i] >= 'A' && p2[i] <= 'Z') ? (p2[i] + 32) : p2[i];
        if (c1 != c2) return c1 - c2;
        if (c1 == 0) return 0;
    }
    return 0;
}

char* strchr(const char* s, int c) {
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return (c == 0) ? (char*)s : NULL;
}

char* strrchr(const char* s, int c) {
    const char* last = NULL;
    do {
        if (*s == (char)c) last = s;
    } while (*s++);
    return (char*)last;
}

char* strstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;
    for (; *haystack; haystack++) {
        const char* h = haystack;
        const char* n = needle;
        while (*h && *n && (*h == *n)) { h++; n++; }
        if (!*n) return (char*)haystack;
    }
    return NULL;
}

char* strdup(const char* s) {
    extern void* malloc(size_t);
    size_t len = strlen(s) + 1;
    char* copy = (char*)malloc(len);
    if (copy) memcpy(copy, s, len);
    return copy;
}

int atoi(const char* s) {
    int res = 0, sign = 1;
    while (*s == ' ' || *s == '\t' || *s == '\n') s++;
    if (*s == '-') { sign = -1; s++; }
    else if (*s == '+') s++;
    while (*s >= '0' && *s <= '9') {
        res = res * 10 + (*s - '0');
        s++;
    }
    return res * sign;
}

int abs(int j) { return (j < 0) ? -j : j; }

int toupper(int c) { return (c >= 'a' && c <= 'z') ? (c - 32) : c; }
int tolower(int c) { return (c >= 'A' && c <= 'Z') ? (c + 32) : c; }
int isspace(int c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f'; }
int isdigit(int c) { return c >= '0' && c <= '9'; }
int isalpha(int c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }

int vsnprintf(char* str, size_t size, const char* format, va_list ap) {
    if (!str || size == 0) return 0;
    size_t pos = 0;
    for (const char* p = format; *p && pos < size - 1; p++) {
        if (*p != '%') {
            str[pos++] = *p;
            continue;
        }
        p++;
        int zero_pad = 0;
        int width = 0;
        if (*p == '0' || *p == '.') {
            zero_pad = 1;
            p++;
        }
        while (*p >= '0' && *p <= '9') {
            width = width * 10 + (*p - '0');
            p++;
        }
        while (*p == '-' || *p == 'l') p++;
        if (*p == 's') {
            const char* s = va_arg(ap, const char*);
            if (!s) s = "(null)";
            while (*s && pos < size - 1) str[pos++] = *s++;
        } else if (*p == 'd' || *p == 'i') {
            int val = va_arg(ap, int);
            char buf[16];
            int idx = 0;
            unsigned int uval = (val < 0) ? -val : val;
            if (val < 0 && pos < size - 1) str[pos++] = '-';
            if (uval == 0) buf[idx++] = '0';
            while (uval > 0) { buf[idx++] = '0' + (uval % 10); uval /= 10; }
            while (zero_pad && idx < width && idx < 15) buf[idx++] = '0';
            while (idx > 0 && pos < size - 1) str[pos++] = buf[--idx];
        } else if (*p == 'x' || *p == 'X') {
            unsigned int val = va_arg(ap, unsigned int);
            char buf[16];
            int idx = 0;
            const char* hex = (*p == 'X') ? "0123456789ABCDEF" : "0123456789abcdef";
            if (val == 0) buf[idx++] = '0';
            while (val > 0) { buf[idx++] = hex[val & 0xF]; val >>= 4; }
            while (zero_pad && idx < width && idx < 15) buf[idx++] = '0';
            while (idx > 0 && pos < size - 1) str[pos++] = buf[--idx];
        } else if (*p == 'c') {
            str[pos++] = (char)va_arg(ap, int);
        } else if (*p == '%') {
            str[pos++] = '%';
        }
    }
    str[pos] = '\0';
    return (int)pos;
}

int snprintf(char* str, size_t size, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vsnprintf(str, size, format, ap);
    va_end(ap);
    return ret;
}

int sprintf(char* str, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vsnprintf(str, 1024, format, ap);
    va_end(ap);
    return ret;
}

int sscanf(const char* str, const char* format, ...) {
    (void)str; (void)format;
    return 0;
}

void* stderr = 0;
void* stdout = 0;
int errno = 0;

long strtol(const char* nptr, char** endptr, int base) {
    while (*nptr == ' ' || *nptr == '\t' || *nptr == '\n') nptr++;
    int sign = 1;
    if (*nptr == '-') { sign = -1; nptr++; }
    else if (*nptr == '+') nptr++;
    if (base == 0) {
        if (*nptr == '0' && (*(nptr+1) == 'x' || *(nptr+1) == 'X')) { base = 16; nptr += 2; }
        else if (*nptr == '0') { base = 8; nptr++; }
        else base = 10;
    } else if (base == 16 && *nptr == '0' && (*(nptr+1) == 'x' || *(nptr+1) == 'X')) {
        nptr += 2;
    }
    long res = 0;
    while (*nptr) {
        int d = -1;
        if (*nptr >= '0' && *nptr <= '9') d = *nptr - '0';
        else if (*nptr >= 'a' && *nptr <= 'z') d = *nptr - 'a' + 10;
        else if (*nptr >= 'A' && *nptr <= 'Z') d = *nptr - 'A' + 10;
        if (d < 0 || d >= base) break;
        res = res * base + d;
        nptr++;
    }
    if (endptr) *endptr = (char*)nptr;
    return res * sign;
}

double strtod(const char* nptr, char** endptr) {
    long val = strtol(nptr, endptr, 10);
    return (double)val;
}

int putc(int c, void* stream) { (void)stream; return c; }
int fputc(int c, void* stream) { (void)stream; return c; }
int fputs(const char* s, void* stream) { (void)s; (void)stream; return 0; }
int system(const char* command) { (void)command; return -1; }
int __isoc99_sscanf(const char* str, const char* format, ...) { (void)str; (void)format; return 0; }

static int upper_table[384];
static const int* upper_ptr = 0;
const int** __ctype_toupper_loc(void) {
    if (upper_ptr == 0) {
        for (int i = 0; i < 384; i++) {
            int ch = i - 128;
            upper_table[i] = (ch >= 'a' && ch <= 'z') ? (ch - 32) : ch;
        }
        upper_ptr = &upper_table[128];
    }
    return &upper_ptr;
}

static unsigned short ctype_table[384];
static const unsigned short* ctype_ptr = 0;
const unsigned short** __ctype_b_loc(void) {
    if (ctype_ptr == 0) {
        for (int i = 0; i < 384; i++) {
            int ch = i - 128;
            unsigned short flags = 0;
            if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') flags |= 0x2000;
            if (ch >= '0' && ch <= '9') flags |= 0x0800;
            if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) flags |= 0x0400;
            ctype_table[i] = flags;
        }
        ctype_ptr = &ctype_table[128];
    }
    return &ctype_ptr;
}

double fabs(double x) { return (x < 0.0) ? -x : x; }