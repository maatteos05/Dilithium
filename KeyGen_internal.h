#ifndef KEY_INTERNAL_H
#define KEY_INTERNAL_H

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "NTTarithmetic.c"
#include "aux_funcs.h"
#include "fips202.h"
#include "params.h"
#include "zetas_array.c"

void H(uint8_t *input, size_t input_len, uint8_t *output, size_t out_len);

void NTT(int32_t w[256], int32_t w_hat[256]);

void NTT_inv(int32_t w[256], int32_t w_hat[256]);

void ExpandA(uint8_t rho[32], int32_t A[k][l][256]);

void Power2Round(int32_t r, int32_t r_decomp[2]);

void pkEncode(uint8_t *pk, uint8_t rho[32], int32_t t1[k][256]);

void skEncode(uint8_t *sk, uint8_t rho[32], uint8_t K[32], uint8_t tr[64],
              int32_t s1[l][256], int32_t s2[k][256], int32_t t_0[k][256]);

void ExpandS(uint8_t seed[64], int32_t s1[l][256], int32_t s2[k][256]);

void RejNTTPoly(uint8_t seed[34], int32_t a[256]);

void RejBoundedPoly(uint8_t seed[66], int32_t a[256]);

int KeyGen_internal(uint8_t seed[32], uint8_t *pk, uint8_t *sk);

#endif