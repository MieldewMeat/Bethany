#include "string.h"
#include "../memory/heap.h"

size_t strlen(const char *str){
    size_t len = 0;

    while(str[len]) len++;

    return len;
}

uint64_t strcmp(const char *a, const char *b){
    while(*a && *b){
        if(*a != *b) return *a-*b;
        a++;
        b++;
    }

    return *a-*b;
}

char *strcpy(char *dst, const char *src){
    char *ret = dst;

    while((*dst++ = *src++));

    return ret;
}

char *strdup(const char *str){
    size_t len = strlen(str);

    char *copy = kmalloc(len +1);

    if(copy == NULL) return NULL;

    strcpy(copy, str);

    return copy;
}

void *memcpy(void *dest, const void *src, size_t n){
    uint8_t *d = dest;
    const uint8_t *s = src;

    for(size_t i = 0; i < n; i++) d[i] = s[i];

    return dest;
}

void *memset(void *ptr, uint8_t value, size_t n){
    uint8_t *p = ptr;

    while(n--) *p++ = (uint8_t)value;

    return ptr;
}
