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

#include "Symbolicator.h"

#include <stdio.h>  /* printf(), fprintf() */
#include <string.h> /* strcmp() */
#include <stdlib.h> /* strtoul() */
#include <limits.h> /* ULONG_MAX */
#include <stdarg.h> /* va_list, va_start(), va_end() */
#include <errno.h>  /* errno */

static bool TestSymbolicator(Symbolicator symbolicator,
                             SymbolicatorInfo *info) {
  return SymbolicatorSymbolicate(symbolicator, (void *)TestSymbolicator, info);
}

static void ShowInfo(SymbolicatorInfo *info) {
  printf("owner_name  = %s\n", info->owner_name);
  printf("owner_base  = 0x%llx\n", info->owner_base);
  printf("symbol_name = %s\n", info->symbol_name);
  printf("symbol_base = 0x%llx\n", info->symbol_base);
  printf("filename    = %s\n", info->filename);
  printf("linenumber  = %d\n", info->linenumber);
}

int main(int argc, const char *argv[]) {
  (void)argc;
  Symbolicator symbolicator;
  if (!SymbolicatorGetSelf(&symbolicator)) {
    fprintf(stderr, "%s: SymbolicatorGetSelf failed\n", argv[0]);
  }
  SymbolicatorInfo info;
  if (TestSymbolicator(symbolicator, &info)) {
    ShowInfo(&info);
  } else {
    fprintf(stderr, "%s: TestSymbolicator/SymbolicatorSymbolicate failed\n",
            argv[0]);
  }
  if (!SymbolicatorClose(&symbolicator)) {
    fprintf(stderr, "%s: SymbolicatorClose failed\n", argv[0]);
  }
}
