#include "args.h"
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

void parse_args(ArgParserOption *opts, int optc, int argc, char **argv,
                void *dist) {
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
          for (int i = 0; i < optc; i++) {
            if (opts[i].ch && c == opts[i].ch) {
              void *val = NULL;

              if (opts[i].parse != NULL)
                val = opts[i].parse(opts[i].str, optv);
              if (opts[i].set != NULL)
                opts[i].set(val, dist);
              free(val);
            }
          }
          ch_idx--;
        }
        break;
      }
      case ARG_LONG_OPTION:
        *carg = '\0';
        for (int i = 0; i < optc; i++) {
          if (strncmp(opts[i].str, optp, strlen(optp)) == 0) {
            void *val = NULL;

            if (opts[i].parse != NULL)
              val = opts[i].parse(optp, optv);
            if (opts[i].set != NULL)
              opts[i].set(val, dist);
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
