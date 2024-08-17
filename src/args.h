#ifndef ARGS_H
#define ARGS_H

#include <stdint.h>
#include <stdlib.h>

typedef enum { ARG_NO_OPTION, ARG_SHRT_OPTION, ARG_LONG_OPTION } ArgOption;
typedef struct ArgParserOption {
  char *lng;
  char shrt;
  char *description;
  void *(*parse)(char *, char *, void *);
  void (*set)(void *, void *);
} ArgParserOption;

typedef struct ArgParserOptions {
  ArgParserOption *options;
  uint8_t count;
  uint8_t max_count;
} ArgParserOptions;

ArgParserOptions *args_init_options(uint8_t);
void args_add_option(ArgParserOptions *options, ArgParserOption option);
void args_add_options(ArgParserOptions *options, int, ...);
void args_parse(ArgParserOptions *options, int argc, char **argv, void *dist);
void args_destroy(ArgParserOptions *);

#endif
