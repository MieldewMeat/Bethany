#pragma once

#include <stddef.h>
#include <stdint.h>

size_t strlen(const char *str);

uint64_t strcmp(const char *a, const char *b);

char *strcpy(char *dst, const char *src);

char *strdup(const char *str);

void *memcpy(void *dest, const void *src, size_t n);