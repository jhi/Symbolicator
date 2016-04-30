OPT_CFLAGS = -O

# The -g is required to have the source code information.
# It CAN be combined with -O in modern C toolchains.
DEBUG_CFLAGS = -g

CLANG_WARN_FLAGS = -Weverything
GCC_WARN_FLAGS = -Wall -Wextra

CLANG_CFLAGS = $(CLANG_WARN_FLAGS) $(DEBUG_CFLAGS) $(OPT_CFLAGS)
GCC_CFLAGS = $(GCC_WARN_FLAGS) $(DEBUG_CFLAGS) $(OPT_CFLAGS)

CLANG_FORMAT = clang-format

CC = clang
CFLAGS = $(CLANG_CFLAGS)

all: SymbolicatorTest backtrace_symbols backtrace_json

SymbolicatorTest: Symbolicator.o SymbolicatorTest.o
	$(CC) $(CFLAGS) -o SymbolicatorTest Symbolicator.o SymbolicatorTest.o

backtrace_symbols: backtrace_symbols.o
	$(CC) $(CFLAGS) -o backtrace_symbols backtrace_symbols.c

backtrace_json: backtrace_json.o Symbolicator.o
	$(CC) $(CFLAGS) -o backtrace_json backtrace_json.o Symbolicator.o

clang_format:
	$(CLANG_FORMAT) -i *.[hc]

clean:
	$(RM) *.o
	$(RM) SymbolicatorTest backtrace_symbols backtrace_json
	$(RM) *~
	$(RM) -r *.dSYM
