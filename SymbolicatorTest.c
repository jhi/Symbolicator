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
