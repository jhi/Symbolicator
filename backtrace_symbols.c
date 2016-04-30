/* This example code shows how to get symbolic information using
 * the backtrace_symbols() API for the frames returned with backtrace().
 *
 * See the backtrace_json.c for comparison.
 */

#include <execinfo.h> /* backtrace(), backtrace_symbols() */
#include <stdlib.h>   /* free() */
#include <stdio.h>    /* printf(), fprintf() */

int main() {
  void *frames[10];
  int d = backtrace(frames, sizeof(frames) / sizeof(frames[0]));
  printf("backtrace() returned %d\n", d);
  printf("backtrace_symbols:\n");
  char **symbols = backtrace_symbols(frames, d);
  if (symbols) {
    for (int i = 0; i < d; i++) {
      printf("%s\n", symbols[i]);
    }
    free(symbols); /* Please remember to do this. */
  } else {
    fprintf(stderr, "backtrace_symbols() failed\n");
  }
}
