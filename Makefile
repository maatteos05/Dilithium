# Minimal Makefile for the student's ML-DSA (FIPS 204) toy code
# All .c files are compiled as separate translation units

CC      ?= gcc
CFLAGS  ?= -O2 -g -std=c11 -Wall -Wextra
CPPFLAGS?= -I.
LDLIBS  ?= -lm

# Support/library code
LIB_SRCS = aux_funcs.c fips202.c keccakf1600.c randombytes.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

# Internal implementation files (compiled as separate units now)
INTERNAL_SRCS = KeyGen_internal.c Sign_internal.c Verify_internal.c NTTarithmetic.c zetas_array.c
INTERNAL_OBJS = $(INTERNAL_SRCS:.c=.o)

PROGS = keygen sign verify test_keygen test_aux_funcs

all: $(PROGS)

# KeyGen executable
keygen: KeyGen.o $(INTERNAL_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

# Sign executable (with SIGN_MAIN to include main())
Sign.o: Sign.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -DSIGN_MAIN -c -o $@ $<

sign: Sign.o $(INTERNAL_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

# Sign_lib.o: Sign.c without main (for linking into verify)
Sign_lib.o: Sign.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

# Verify executable
verify: Verify.o Sign_lib.o $(INTERNAL_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

# test_keygen
test_keygen: test_keygen.o $(INTERNAL_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

# test_aux_funcs
test_aux_funcs: test_aux_funcs.o aux_funcs.o
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f *.o $(PROGS)

.PHONY: all clean
