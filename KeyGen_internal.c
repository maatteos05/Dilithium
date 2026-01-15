#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "aux_funcs.h"
#include "fips202.h"
#include "keccakf1600.h"

uint8_t k = 4;
uint8_t l = 4;

void H(uint8_t seed[32]) {
  /* Generates a public-private key pair from a seed. */
  uint8_t rho[32];
  uint8_t rho_p[64];
  uint8_t K[32];

  // declare the hash function
  shake256incctx state;
  shake256_inc_init(&state);

  shake256_inc_absorb(&state, seed, 32);
  shake256_inc_absorb(&state, &k, 1);
  shake256_inc_absorb(&state, &l, 1);

  shake256_inc_finalize(&state);

  uint8_t output[128];
  shake256_inc_squeeze(output, 128, &state);

  memcpy(rho, output, 32);
  memcpy(rho_p, output + 32, 64);
  memcpy(K, output + 96, 32);

  // free memory held by context
  shake256_inc_ctx_release(&state);

  return;
}

// Forward declarations
void RejNTTPoly(uint8_t seed[34], int32_t a[256]);
void RejBoundedPoly(uint8_t seed[66], int32_t a[256]);

void ExpandA(uint8_t seed[32], int32_t A[k][l][256]) {
  /* Sample a kxl matrix A of elements of R_q */
  uint8_t rho[32];
  memcpy(rho, seed, 32);

  for (uint8_t r = 0; r < k; r++) {
    for (uint8_t s = 0; s < l; s++) {
      uint8_t rho_p[34];
      memcpy(rho_p, rho, 32);
      rho_p[32] = s;
      rho_p[33] = r;

      RejNTTPoly(rho_p, A[r][s]);
    }
  }
  return;
}

void RejNTTPoly(uint8_t seed[34], int32_t a[256]) {
  /* Samples a polynomial in R_q */
  int j = 0;

  shake128incctx state;
  shake128_inc_init(&state);

  shake128_inc_absorb(&state, seed, 34);
  shake128_inc_finalize(&state);

  while (j < 256) {
    uint8_t out[3];
    shake128_inc_squeeze(out, 3, &state);
    int32_t coeff = CoeffFromThreeBytes(out[0], out[1], out[2]);
    if (coeff != -1) {
      a[j] = coeff;
      j++;
    }
  }
  shake128_inc_ctx_release(&state);
  return;
}

void RejBoundedPoly(uint8_t seed[66], int32_t a[256]) {
  /* Samples an element a in R with coefficients in [-eta, ..., eta] omputed
  via rejection sampling from rho (the seed). */
  int j = 0;

  shake256incctx state;
  shake256_inc_init(&state);
  shake256_inc_absorb(&state, seed, 66);
  shake256_inc_finalize(&state);

  while (j < 256) {
    uint8_t z; // H.Squeeze(ctx, 1)
    shake256_inc_squeeze(&z, 1, &state);

    uint8_t b0 = z % 16;
    uint8_t b1 = z / 16;

    int32_t z0 = CoeffFromHalfByte(b0); // z mod 16
    int32_t z1 = CoeffFromHalfByte(b1); // floor(z / 16)

    if (z0 != -1) {
      a[j] = z0;
      j++;
    }

    if (z1 != -1 && j < 256) {
      a[j] = z1;
      j++;
    }
  }
  shake256_inc_ctx_release(&state);
}

void ExpandS(uint8_t seed[64], int32_t s1[l][256], int32_t s2[k][256]) {
  /*
  Samples vectors s_1 in R^l and s_2 in R_k, each with polynomial
  coordinates whose coefficients whose are in the interval [-eta, eta].
  */

  for (int r = 0; r < l; r++) {
    uint8_t input[66];

    memcpy(input, seed, 64);
    IntegerToBytes(input + 64, r, 2);

    RejBoundedPoly(input, s1[r]);
  }

  for (int r = 0; r < k; r++) {
    uint8_t input[66];

    memcpy(input, seed, 64);
    IntegerToBytes(input + 64, r + l, 2);

    RejBoundedPoly(input, s2[r]);
  }
  return;
}