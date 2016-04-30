/*
 * Copyright (c) 2016, Jarkko Hietaniemi <jhi@iki.fi>
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice appear
 * in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

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
