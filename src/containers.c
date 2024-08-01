#include "containers.h"
#include <stdlib.h>
#include <string.h>

HashTable *HashTable_create(size_t size) {
  HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
  ht->size = size;
  ht->table = malloc(size * sizeof(HTEntry *));

  return ht;
}

size_t HashTable_hash(char *key, size_t size) {
  size_t hashval;

  for (hashval = 0; *key != '\0'; key++)
    hashval = *key + 31 * hashval;

  return hashval % size;
}

void HashTable_append(HashTable *ht, char *key, void *value) {
  HTEntry *entry = malloc(sizeof(HTEntry));
  entry->key = malloc(strlen(key) * sizeof(char));
  strncpy(entry->key, key, strlen(key));
  entry->value = value;
  entry->next = NULL;

  size_t id = HashTable_hash(key, ht->size);

  ht->table[id] = entry;
}

void *HashTable_get(HashTable *ht, char *key) {
  unsigned id = HashTable_hash(key, ht->size);

  HTEntry *entry;

  if ((entry = ht->table[id]) == NULL)
    return NULL;

  return entry->value;
}
