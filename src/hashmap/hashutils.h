#ifndef HASHUTILS
#define HASHUTILS

#include <stddef.h>
#include <stdint.h>

uint32_t hashNumber(const void *data, size_t length);
uint32_t hashToken(const void *key);

#endif