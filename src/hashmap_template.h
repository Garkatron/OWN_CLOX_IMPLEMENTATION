#ifndef HASHMAP_TEMPLATE_H
#define HASHMAP_TEMPLATE_H

#define HASHMAP_NAME ValueMap
#define KEY_TYPE Value*
#define VALUE_TYPE Value*
#define HASH_FUNC valueHash
#define EQUALS_FUNC valueEquals

#include "hashmap_impl.h"

#undef HASHMAP_NAME
#undef KEY_TYPE
#undef VALUE_TYPE
#undef HASH_FUNC
#undef EQUALS_FUNC

#endif
