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
/*
Adds the given key/value pair to the given hash table.
If a the key exists it get overwrited.
The function returns true if the entry was added.
*/
bool tableSet(Table *table, ObjString *key, Value value);
void tableAddAll(Table *from, Table *to);
/*
You pass in a table and a key. 
If it finds an entry with that key, it returns true, otherwise it returns false. If the entry exists, the value output parameter points to the resulting value.
*/
bool tableGet(Table *table, ObjString *key, Value *value);
#endif
