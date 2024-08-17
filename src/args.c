#include "args.h"
#include "utils.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ArgOption get_arg_type(char *arg) {
  ArgOption arg_opt = ARG_NO_OPTION;

  while (*arg == '-') {
    arg_opt++;
    arg++;
  }

  return arg_opt;
}

ArgParserOptions *args_init_options(uint8_t size) {
  ArgParserOptions *opts = malloc(sizeof(ArgParserOptions));

  if (opts == NULL)
    terminate("Failed to allocate memory");

  opts->options = malloc(size * sizeof(ArgParserOption));

  if (opts == NULL) {
    free(opts);
    terminate("Failed to allocate memory");
  }

  opts->max_count = size;
  opts->count = 0;

  return opts;
}

void args_add_option(ArgParserOptions *opts, ArgParserOption option) {
  if (opts->count >= opts->max_count)
    return;

  opts->options[opts->count++] = option;
}

void args_add_options(ArgParserOptions *opts, int count, ...) {
  va_list args;
  va_start(args, count);

  for (int i = 0; i < count; i++) {
    args_add_option(opts, va_arg(args, ArgParserOption));
  }

  va_end(args);
}

void args_parse(ArgParserOptions *opts, int argc, char **argv, void *dist) {
  char *carg, *narg, *optp, *optv;
  ArgOption arg_opt;
  bool has_eq_char;

  while (argc--) {
    carg = *argv;
    carg += arg_opt = get_arg_type(carg);
    has_eq_char = false;

    if (arg_opt != ARG_NO_OPTION) {
      optp = carg;

      if (arg_opt == ARG_LONG_OPTION && *carg == '\0')
        break;

      while (*carg) {
        if (*carg == '=') {
          has_eq_char = true;
          break;
        }
        carg++;
      }

      narg = *(argv + 1);
      if (has_eq_char) {
        optv = carg + 1;
      } else if (narg == NULL || get_arg_type(narg)) {
        optv = NULL;
      } else {
        optv = narg;
      }

      switch (arg_opt) {
      case ARG_SHRT_OPTION: {
        size_t ch_idx = carg - optp;
        while (ch_idx) {
          char c = *optp++;
          if (ch_idx > 1)
            optv = NULL;
          for (size_t i = 0; i < opts->count; i++) {
            if (opts->options[i].shrt && c == opts->options[i].shrt) {
              void *val = NULL;

              if (opts->options[i].parse != NULL)
                val = opts->options[i].parse(opts->options[i].lng, optv, opts);
              if (opts->options[i].set != NULL)
                opts->options[i].set(val, dist);
              free(val);
            }
          }
          ch_idx--;
        }
        break;
      }
      case ARG_LONG_OPTION:
        *carg = '\0';
        for (int i = 0; i < opts->count; i++) {
          if (strncmp(opts->options[i].lng, optp, strlen(optp)) == 0) {
            void *val = NULL;

            if (opts->options[i].parse != NULL)
              val = opts->options[i].parse(optp, optv, opts);
            if (opts->options[i].set != NULL)
              opts->options[i].set(val, dist);
            free(val);
          }
        }
        break;
      default:
        break;
      }
    }

    argv++;
  }
}

void args_destroy(ArgParserOptions *opts) {
  free(opts->options);
  free(opts);
}
