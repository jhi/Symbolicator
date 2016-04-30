/* This example code shows how to get symbolic information using
 * the Symbolicator API for the frames returned with backtrace().
 *
 * The output is in JSON format.
 *
 * See the backtrace_symbols.c for comparison, and see also
 * SymbolicatorTest.c for another example of Symbolicator API.
 */

#include "Symbolicator.h"

#include <execinfo.h> /* backtrace(), backtrace_symbols() */
#include <stdio.h>    /* printf(), fprintf() */
#include <stdint.h>   /* uintptr_t */

int main() {
  void *frames[10];
  int d = backtrace(frames, sizeof(frames) / sizeof(frames[0]));
  Symbolicator symbolicator;
  if (SymbolicatorGetSelf(&symbolicator)) {
    printf("[\n");
    const int last = d - 1;
    SymbolicatorInfo info;
    for (int i = 0; i < d; i++) {
      printf("  {");
      if (SymbolicatorSymbolicate(symbolicator, frames[i], &info)) {
#undef JSON_STR
#undef JSON_NUM
#define JSON_STR(m)                                                            \
  if (info.m != NULL) {                                                        \
    printf("\"" #m "\":\"%s\",", info.m);                                      \
  } else {                                                                     \
    printf("\"" #m "\":null,");                                                \
  }
#define JSON_NUM(m, f) printf("\"" #m "\":\"%" #f "\",", info.m)
        JSON_STR(owner_name);
        JSON_NUM(owner_base, llx);
        JSON_STR(symbol_name);
        JSON_NUM(symbol_base, llx);
        JSON_STR(filename);
        JSON_NUM(linenumber, d);
#undef JSON_STR
#undef JSON_NUM
        printf("\"address\":\"%jx\"", (uintptr_t)frames[i]);
      } else {
        fprintf(stderr, "SymblicatorGetSelf failed for frames[%d] = %jx\n", i,
                (uintptr_t)frames[i]);
      }
      printf("}%s\n", i < last ? "," : "");
    }
    printf("]\n");
  } else {
    fprintf(stderr, "SymblicatorGetSelf failed\n");
  }
}
