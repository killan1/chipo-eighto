#include "args.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum { ARG_NO_OPTION, ARG_SHRT_OPTION, ARG_LONG_OPTION } ArgOpt;
typedef enum { ARG_VALUE, ARG_OPTION } ArgType;

typedef struct HTEntry {
  char *key;
  void *value;
  struct HTEntry *next;
} HTEntry;

typedef struct HashTable {
  size_t size;
  HTEntry **table;
} HashTable;

HashTable *createHashTable(size_t size) {
  printf("size=%ld\n", size);
  HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
  ht->size = size;
  ht->table = malloc(size * sizeof(HTEntry *));

  return ht;
}

unsigned hash(char *key, size_t size) {
  unsigned int hashval;

  for (hashval = 0; *key != '\0'; key++)
    hashval = *key + 31 * hashval;

  return hashval % size;
}

void append(HashTable *ht, char *key, void *value) {
  HTEntry *entry = malloc(sizeof(HTEntry));
  entry->key = malloc(strlen(key) * sizeof(char));
  strncpy(entry->key, key, strlen(key));
  entry->value = value;
  entry->next = NULL;

  unsigned int id = hash(key, ht->size);

  ht->table[id] = entry;
}

void *get(HashTable *ht, char *key) {
  unsigned id = hash(key, ht->size);

  HTEntry *entry;

  if ((entry = ht->table[id]) == NULL)
    return NULL;

  return entry->value;
}

int get_arg_type(char *arg) {
  int dashc = 0;

  while (*arg == '-') {
    dashc++;
    arg++;
  }

  return dashc;
}

void parse_args(ArgParseOption *opts, int optc, int argc, char **argv,
                void *dist) {
  char *carg, *narg, *optp, *optv;
  int dashc;
  bool has_eq_char;

  while (argc--) {
    carg = *argv;
    dashc = get_arg_type(carg);
    carg += dashc;
    has_eq_char = false;

    if (dashc) {
      optp = carg;

      if (dashc == 2 && *carg == '\0')
        break;

      while (*carg) {
        if (*carg == '=') {
          has_eq_char = true;
          break;
        }
        carg++;
      }

      char *narg = *(argv + 1);
      if (has_eq_char) {
        optv = carg + 1;
      } else if (narg == NULL || get_arg_type(narg)) {
        optv = NULL;
      } else {
        optv = narg;
      }
      for (int i = 0; i < optc; i++) {
        if (dashc == 1) {
          int chindex = carg - optp;
          while (chindex) {
            char c = *optp++;
            if (chindex > 1)
              optv = NULL;

            if (c == opts[i].ch) {
              opts[i].parse(optp, optv, dist);
            }
            chindex--;
          }
        } else {
          *carg = '\0';
          if (strncmp(opts[i].str, optp, strlen(optp)) == 0) {
            opts[i].parse(optp, optv, dist);
          }
        }
      }
    }

    argv++;
  }
}

void parse_args__(ArgParseOption *options, int optc, int argc, char **argv,
                  void *dist) {
  int c = 1;

  char *p, *pp, *optp;
  ArgType at = ARG_VALUE;
  ArgOpt opt = ARG_NO_OPTION;
  while (c < argc) {
    p = argv[c];

    while (*p == '-') {
      if (opt == ARG_LONG_OPTION)
        opt = 0;
      opt++;
      p++;
      at = ARG_OPTION;
    }

    if (at == ARG_OPTION) {
      if (opt == ARG_LONG_OPTION && *p == '\0')
        c = argc;
      optp = p;
      at = ARG_VALUE;
    } else if (at == ARG_VALUE) {
      if (opt != ARG_NO_OPTION) {
        for (int i = 0; i < optc; i++) {
          switch (opt) {
          case ARG_SHRT_OPTION:
            pp = optp;
            while (*pp) {
              if (*pp++ == options[i].ch)
                options[i].parse(optp, p, dist);
            }
            break;
          case ARG_LONG_OPTION:
            if (strncmp(options[i].str, optp, strlen(optp)) == 0) {
              options[i].parse(optp, p, dist);
            }
            break;
          default:
            break;
          }
        }
      }
      opt = ARG_NO_OPTION;
    }

    c++;
  }
}
