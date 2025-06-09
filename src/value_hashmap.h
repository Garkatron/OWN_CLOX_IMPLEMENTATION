#ifndef VALUE_HASHMAP_H
#define VALUE_HASHMAP_H

#include "value.h"


void vfreeKey(void *key);
void vfreeValue(void *value);
bool vequal(const void *a_ptr, const void *b_ptr);
uint32_t vhashValue(const void *value);

#endif

