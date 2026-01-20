#ifndef AUX_FUNCS_H
#define AUX_FUNCS_H

#include <stddef.h>
#include <stdint.h>

/* Computes a base-256 representation of x mod 256^alpha */
void IntegerToBytes(uint8_t *y, uint32_t x, size_t alpha);

/* Generates an element in {0,1,..., q = 8380417} U {-1} */
int32_t CoeffFromThreeBytes(uint8_t b0, uint8_t b1, uint8_t b2);

int32_t CoeffFromHalfByte(uint8_t b);

void SimpleBitPack(uint8_t *z, int32_t w[256], int b);

int bit_len(uint32_t x);

void BitPack(uint8_t *z, int32_t w[256], int a, int b);
#endif