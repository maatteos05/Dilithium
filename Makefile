# Minimal Makefile for the student's ML-DSA (FIPS 204) toy code
# NOTE: several .c files are #included directly (e.g., KeyGen_internal.c,
# NTTarithmetic.c, zetas_array.c). To avoid duplicate symbols, we do NOT
# compile those included .c files as separate translation units.

CC      ?= gcc
CFLAGS  ?= -O2 -g -std=c11 -Wall -Wextra
CPPFLAGS?= -I.
LDLIBS  ?= -lm

# Support code that is *not* included via #include "... .c" in other files
LIB_SRCS = aux_funcs.c fips202.c keccakf1600.c randombytes.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

PROGS = keygen test_keygen test_aux_funcs

all: $(PROGS)

# KeyGen.c #includes KeyGen_internal.c, which itself pulls in NTTarithmetic.c and zetas_array.c.
# So: build KeyGen.o + the external helper objects only.
keygen: KeyGen.o $(LIB_OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

# test_keygen.c #includes KeyGen_internal.c (so it already contains internal code),
# and it does not use randombytes().
test_keygen: test_keygen.o aux_funcs.o fips202.o keccakf1600.o
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

test_aux_funcs: test_aux_funcs.o aux_funcs.o
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f *.o $(PROGS)

.PHONY: all clean

