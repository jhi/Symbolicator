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

#ifndef __APPLE__
#error "This API works only on Apple operating systems, sorry."
#endif

#ifndef SYMBOLICATOR_H
#define SYMBOLICATOR_H

/* The information returned for an address.
 *
 * NOTE: do not try to modify or free() the strings: they are pointers
 * to read-only static data. Copy them away with strdup() if you need
 * modified versions. */
typedef struct {
  const char *owner_name;         /* The module (executable, library) name */
  unsigned long long owner_base;  /* The start address the module */
  const char *symbol_name;        /* The name of the function */
  unsigned long long symbol_base; /* The start address of the function */
  const char *filename; /* Source code filename, or null if no debug info */
  int linenumber;       /* Source code linenumber, or zero if no debug info */
  char __pad[4];        /* Placate clang -Weverything/-Wpadded. */
} SymbolicatorInfo;

typedef struct {
  void *data;
  void *obj;
} CSTypeRef;

typedef CSTypeRef CSSymbolRef;
typedef CSSymbolRef CSSymbolicatorRef;
typedef CSSymbolicatorRef Symbolicator;

#include <stdbool.h> /* bool */
#include <unistd.h>  /* pid_t */

/* Set the symbolicator to the current process.
 *
 * Returns true if successful, false if not. */

bool SymbolicatorGetSelf(Symbolicator *symbolicator)
    __attribute__((warn_unused_result));

/* Call SymbolicatorClose() when done with the symbolicator.
 *
 * Returns true if successful, false if not.
 */

bool SymbolicatorClose(Symbolicator *symbolicator)
    __attribute__((warn_unused_result));

/* Call SymbolicatorSymbolicate() for addresses
 * that you want to symbolicate.  You can get such
 * addresses for example from the backtrace() API.
 *
 * Returns true if info can be found, or false if not.
 *
 * Returns false also if SymbolicatorInit() has not been called,
 * or if SymbolicatorClose() has been called.
 */

bool SymbolicatorSymbolicate(Symbolicator symbolicator, void *address,
                             SymbolicatorInfo *symbolicator_info)
    __attribute__((warn_unused_result));

/* SymbolicatorInfoHasSourceInfo() returns true if the symbolicator
 * info contains source info (filename and linenumber), or false if
 * not.  If there is no source info, the executable is probably
 * lacking debug information: not compiled with -g, or later stripped.
 */

bool SymbolicatorInfoHasSourceInfo(SymbolicatorInfo *symbolicator_info)
    __attribute__((warn_unused_result));

#endif /* #ifndef SYMBOLICATOR_H */
