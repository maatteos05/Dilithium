#ifndef AUX_FUNCS_H
#define AUX_FUNCS_H

#include <stdint.h>
#include <stddef.h>

/* Computes a base-256 representation of x mod 256^alpha */
void IntegerToBytes(uint8_t *y, uint32_t x, size_t alpha);

/* Generates an element in {0,1,..., q = 8380417} U {-1} */
int32_t CoeffFromThreeBytes(uint8_t b0, uint8_t b1, uint8_t b2);

#endif