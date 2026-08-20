# Minimal Makefile for the student's ML-DSA (FIPS 204) toy code
# All .c files are compiled as separate translation units

CC      ?= gcc
CFLAGS  ?= -O2 -g -std=c11 -Wall -Wextra
CPPFLAGS?= -I. -Isrc
LDLIBS  ?= -lm

# Support/library code
LIB_SRCS = src/aux_funcs.c src/fips202.c src/keccakf1600.c src/randombytes.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

# Internal implementation files (compiled as separate units now)
INTERNAL_SRCS = src/KeyGen_internal.c src/Sign_internal.c src/Verify_internal.c src/NTTarithmetic.c src/zetas_array.c
INTERNAL_OBJS = $(INTERNAL_SRCS:.c=.o)

PROGS = keygen sign verify test_keygen test_aux_funcs

all: $(PROGS)

# KeyGen executable
keygen: src/KeyGen.o $(INTERNAL_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

# Sign executable (with SIGN_MAIN to include main())
src/Sign.o: src/Sign.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -DSIGN_MAIN -c -o $@ $<

sign: src/Sign.o $(INTERNAL_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

# Sign_lib.o: Sign.c without main (for linking into verify)
src/Sign_lib.o: src/Sign.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

# Verify executable
verify: src/Verify.o src/Sign_lib.o $(INTERNAL_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

# test_keygen
test_keygen: tests/test_keygen.o $(INTERNAL_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

# test_aux_funcs
test_aux_funcs: tests/test_aux_funcs.o src/aux_funcs.o
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f src/*.o tests/*.o $(PROGS)

.PHONY: all clean
