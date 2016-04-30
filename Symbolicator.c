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

#include <CoreFoundation/CoreFoundation.h>

#include <stdint.h>  /* uint64_t */
#include <dlfcn.h>   /* dlopen(), dlsym() */
#include <pthread.h> /* pthread_mutex_t */

#include <mach/mach.h> /* mach_vm_address_t, mach_task_self() */

typedef CSSymbolRef CSSymbolOwnerRef;
typedef CSSymbolRef CSSourceInfoRef;

typedef uint64_t CSTime;

typedef struct {
  uint64_t location;
  uint64_t length;
} CSRange;

#define kCSNull ((CSTypeRef){0, 0})
#define kCSNow UINT64_C(0x80000000)

static bool SymbolicatorRootInit(void) __attribute__((warn_unused_result));
static void SymbolicatorRootClose(void);

/* SymbolicatorRoot contains the handle to the CoreSymbolicator
 * dynamic library, and the function pointers retrieved via dlsym()
 * from the library. SymbolicatorRoot is a singleton that is protected
 * by the two mutexes. */
static struct {
  pthread_mutex_t init_mutex;
  pthread_mutex_t close_mutex;
  bool initialized; /* Are we there yet? */
  char __pad[7];    /* Placate clang -Weverything/-Wpadded. */
  size_t refcount;
  void *dlhandle; /* From dlopen(). */
#undef CS
#define CS(T, F, A) T(*F) A;
#include "Symbolicator.api" /* The function pointers. */
#undef CS
} SymbolicatorRoot;

static void SymbolicatorRootClose(void) {
  pthread_mutex_lock(&SymbolicatorRoot.close_mutex);

  if (SymbolicatorRoot.dlhandle != NULL) {
    dlclose(SymbolicatorRoot.dlhandle);
    SymbolicatorRoot.dlhandle = NULL;
  }

  SymbolicatorRoot.initialized = false;

  pthread_mutex_unlock(&SymbolicatorRoot.close_mutex);
}

static inline void SymbolicatorRootRefCountInc(void) {
  SymbolicatorRoot.refcount++;
}

static inline bool SymbolicatorRootRefCountDec(void) {
  if (SymbolicatorRoot.refcount == 0) {
    return false;
  }
  SymbolicatorRoot.refcount--;
  if (SymbolicatorRoot.refcount == 0) {
    SymbolicatorRootClose();
  }
  return true;
}

static bool SymbolicatorRootInit(void) {
  pthread_mutex_lock(&SymbolicatorRoot.init_mutex);

  if (!SymbolicatorRoot.initialized) {
    /* The first time this runs there might be a noticeable delay when
     * the CoreSymbolication library is loaded into memory. After that
     * it should stay loaded (of course, unless something unloads it)
     * and there should be little delay. */
    SymbolicatorRoot.dlhandle =
        dlopen("/System/Library/PrivateFrameworks/CoreSymbolication.framework/"
               "CoreSymbolication",
               RTLD_LAZY | RTLD_GLOBAL);
    if (SymbolicatorRoot.dlhandle) {
/* The icky LHS cast is needed because dlsym() returns void* */
#undef CS
#define CS(T, F, A)                                                            \
  *(void **) & (SymbolicatorRoot.F) = dlsym(SymbolicatorRoot.dlhandle, #F);    \
  if (SymbolicatorRoot.F == NULL) {                                            \
    fprintf(stderr, "SymbolicatorRootInit: dlsym() failure for %s\n", #F);     \
    goto done;                                                                 \
  }
#include "Symbolicator.api" /* Initialize the function pointers. */
#undef CS
      /* If still here, the initialization was successful. */
      SymbolicatorRoot.initialized = true;
    }
  }

done:

  if (!SymbolicatorRoot.initialized) {
    SymbolicatorRootClose(); /* Undo partial failed open. */
  }

  pthread_mutex_unlock(&SymbolicatorRoot.init_mutex);

  return SymbolicatorRoot.initialized;
}

bool SymbolicatorSymbolicate(Symbolicator symbolicator, void *address,
                             SymbolicatorInfo *symbolicator_info) {
  memset(symbolicator_info, '\0', sizeof(*symbolicator_info));

  if (!SymbolicatorRootInit()) {
    return false;
  }
  if (SymbolicatorRoot.CSIsNull(symbolicator)) {
    return false;
  }
  if (address == NULL) {
    return false;
  }

  CSSymbolRef symbol =
      SymbolicatorRoot.CSSymbolicatorGetSymbolWithAddressAtTime(
          symbolicator, (mach_vm_address_t)address, kCSNow);
  if (SymbolicatorRoot.CSIsNull(symbol)) {
    return false;
  }
  CSSymbolRef owner = SymbolicatorRoot.CSSymbolGetSymbolOwner(symbol);
  if (SymbolicatorRoot.CSIsNull(owner)) {
    return false;
  }
  symbolicator_info->owner_name = SymbolicatorRoot.CSSymbolOwnerGetName(owner);
  symbolicator_info->owner_base =
      SymbolicatorRoot.CSSymbolOwnerGetBaseAddress(owner);
  symbolicator_info->symbol_name = SymbolicatorRoot.CSSymbolGetName(symbol);
  symbolicator_info->symbol_base =
      SymbolicatorRoot.CSSymbolGetRange(symbol).location;
  CSSymbolRef source_info =
      SymbolicatorRoot.CSSymbolicatorGetSourceInfoWithAddressAtTime(
          symbolicator, (mach_vm_address_t)address, kCSNow);
  if (!SymbolicatorRoot.CSIsNull(source_info)) {
    symbolicator_info->filename =
        SymbolicatorRoot.CSSourceInfoGetFilename(source_info);
    symbolicator_info->linenumber =
        SymbolicatorRoot.CSSourceInfoGetLineNumber(source_info);
  }

  return true;
}

bool SymbolicatorInfoHasSourceInfo(SymbolicatorInfo *symbolicator_info) {
  return symbolicator_info->filename != NULL ||
         symbolicator_info->linenumber != 0;
}

bool SymbolicatorGetSelf(Symbolicator *symbolicator) {
  if (!SymbolicatorRootInit()) {
    return false;
  }
  *symbolicator =
      SymbolicatorRoot.CSSymbolicatorCreateWithTask(mach_task_self());
  if (SymbolicatorRoot.CSIsNull(*symbolicator)) {
    return false;
  }
  SymbolicatorRootRefCountInc();
  return true;
}

bool SymbolicatorClose(Symbolicator *symbolicator) {
  if (SymbolicatorRoot.CSIsNull(*symbolicator)) {
    return false;
  }
  if (!SymbolicatorRootRefCountDec()) {
    return false;
  }
  SymbolicatorRoot.CSRelease(*symbolicator);
  *symbolicator = kCSNull;
  return true;
}
