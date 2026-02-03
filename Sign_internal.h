#ifndef SIGN_INTERNAL_H
#define SIGN_INTERNAL_H

#include "params.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void skDecode(uint8_t *sk, uint8_t rho[32], uint8_t K[32], uint8_t tr[64],
              int32_t s1[l][256], int32_t s2[k][256], int32_t t_0[k][256]);

void ExpandMask(int32_t y[l][256], uint8_t rho[64], uint32_t x);

void w1Encode(int32_t w1[k][256], uint8_t *w1tilde, size_t len);

void SampleInBall(uint8_t rho[lambda / 4], int32_t c[256]);

void SigEncode(uint8_t *sigma, size_t len, uint8_t c_tilde[len],
               int32_t z[l][256], bool h[k][256]);

bool MakeHint(int32_t z, int32_t r);

int Sign_internal(uint8_t *sigma, uint8_t *sk, uint8_t *Mp, size_t Mp_size,
                  uint8_t rnd[32]);

#endif // SIGN_INTERNAL_H
