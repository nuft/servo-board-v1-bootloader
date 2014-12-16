#include <stddef.h>

void *memset(void *b, int c, size_t len)
{
    unsigned char *p = (unsigned char *) b;
    while (len--) {
        *p++ = (unsigned char)c;
    }
    return b;
}

void *memcpy(void *dst, const void *src, size_t n)
{
    char *d = dst;
    const char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dst;
}

int strcmp(const char *s1, const char *s2)
{
    const unsigned char *c1 = (const unsigned char *) s1;
    const unsigned char *c2 = (const unsigned char *) s2;
    do {
        if (*c1 > *c2) {
            return 1;
        } else if (*c1 < *c2) {
            return -1;
        }
    } while (*c1++ != 0 && *c2++ != 0);
    return 0;
}

