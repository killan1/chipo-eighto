#ifndef ARGS_H
#define ARGS_H

typedef enum { ARG_NO_OPTION, ARG_SHRT_OPTION, ARG_LONG_OPTION } ArgOption;
typedef struct {
  char *str;
  char ch;
  void (*parse)(char *, char *, void *);
} ArgParseOption;

void parse_args(ArgParseOption *options, int opts_count, int argc, char **argv,
                void *dist);

#endif
