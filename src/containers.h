#ifndef CONTAINERS_H
#define CONTAINERS_H

#include <stdio.h>
#include <stdlib.h>

typedef struct HTEntry {
  char *key;
  void *value;
  struct HTEntry *next;
} HTEntry;

typedef struct HashTable {
  size_t size;
  HTEntry **table;
} HashTable;

HashTable *HashTable_create(size_t);
size_t HashTable_hash(char *, size_t);
void HashTable_append(HashTable *, char *, void *);
void *HashTable_get(HashTable *, char *);

#endif
