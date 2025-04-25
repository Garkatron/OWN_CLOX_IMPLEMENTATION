#ifndef clox_table_h
#define clox_table_h

#include "common.h"
#include "value.h"

// Key/Value pair.
typedef struct {
    ObjString *key;
    Value value;
} Entry;
/*
A Hash-Table is an array of entries.
We track of both allocated size of the array (capacity) and the
number of key/value pairs currently stored in it (count).
The ratio of count to capacity is exactly the load factor of the hash table.
*/
typedef struct 
{
    int count;
    int capacity;
    Entry *entries;
} Table;

void initTable(Table *table);
void freeTable(Table *table);

#endif
