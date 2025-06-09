#include "hashutils.h"
#include <stdint.h>
#include <stddef.h>
#include <scanner.h>

uint32_t hashNumber(const void *data, size_t length)
{
    uint32_t hash = 2166136261u; // FNV-1a offset basis
    const uint8_t *bytes = (const uint8_t *)data;
    for (size_t i = 0; i < length; i++)
    {
        hash ^= bytes[i];
        hash *= 16777619u; // FNV-1a prime
    }
    return hash;
}


uint32_t hashToken(const void *key) {
    const Token *token = (const Token *)key;
    const char *chars = token->start;
    int length = token->length;

    // FNV-1a
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++) {
        hash ^= (uint8_t)chars[i];
        hash *= 16777619;
    }
    return hash;
}