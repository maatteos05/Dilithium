#include <math.h>
#include <stdint.h>
#include <string.h>

#include "NTT-1_zetas.c"
#include "NTT_zetas.c"
#include "NTTarithmetic.c"
#include "aux_funcs.h"
#include "fips202.h"

#define q 8380417
#define d 13
#define ETA 2

uint8_t k = 4;
uint8_t l = 4;

void H(uint8_t *input, uint8_t *output, size_t out_len) {
  // declare the hash function
  shake256incctx state;
  shake256_inc_init(&state);
  shake256_inc_absorb(&state, input, 34);
  shake256_inc_finalize(&state);

  shake256_inc_squeeze(output, out_len, &state);

  // free memory held by context
  shake256_inc_ctx_release(&state);

  return;
}

// Forward declarations
void RejNTTPoly(uint8_t seed[34], int32_t a[256]);
void RejBoundedPoly(uint8_t seed[66], int32_t a[256]);

void ExpandA(uint8_t rho[32], int32_t A[k][l][256]) {
  /* Sample a kxl matrix A of elements of R_q */
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
  /* Samples an element a in R with coefficients in [-eta, ..., eta] computed
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
  Samples vectors s_1 in R^l and s_2 in R^k, each with polynomial
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

void NTT(int32_t w[256], int32_t w_hat[256]) {
  /* Computes the NTT (Number Theoretic Transform)
     Input: w in R_q
     Output: w_hat in T_q */
  for (int j = 0; j < 256; j++) {
    w_hat[j] = w[j];
  }

  int m = 0;
  int len = 128;

  while (len > 0) {
    int start = 0;
    while (start < 256) {
      m = m + 1;
      int32_t z = zetas[m]; // z <-- zeta^(BitRev_8(m)) mod q
      for (int j = start; j < start + len; j++) {
        int32_t t = (z * w_hat[j + len]) % q;
        w_hat[j + len] = (w_hat[j] - t) % q;
        w_hat[j] = (w_hat[j] + t) % q;
      }
      start = start + 2;
    }
    len = len / 2;
  }
  return;
}

void NTT_inv(int32_t w_hat[256], int32_t w[256]) {
  /* Computes the inverse of the NTT
     Input: w_hat in T_q
     Output: w in R_q */
  for (int j = 0; j < 256; j++) {
    w[j] = w_hat[j];
  }
  int m = 256;
  int len = 1;
  while (len < 256) {
    int start = 0;
    while (start < 256) {
      m = m - 1;
      int32_t z = zetas_inv[m];

      for (int j = start; j < start + len; j++) {
        int32_t t = w[j];
        w[j] = (t + w[j + len]) % q;
        w[j + len] = (t - w[j + len]) % q;
        w[j + len] = (z * w[j + len]) % q;
      }
      start += 2 * len;
    }
    len *= 2;
  }
  int f = 8347681;
  for (int j = 0; j < 256; j++) {
    w[j] = (f * w[j]) % q;
  }
  return;
}

void Power2Round(int32_t r, int32_t r_decomp[2]) {
  /* Decomposes r into r = r_1 2^d + r_0 mod q */
  int r_pos = r % q;

  int p = 1 << d;
  int r_0 = r_pos % p;

  while (r_0 > floor((double)r_pos / 2) && r_0 <= ceil((double)r_pos / 2)) {
    r_0++;
  }

  int r_1 = (r_pos - r_0) / p;

  r_decomp[0] = r_0;
  r_decomp[1] = r_1;
  return;
}

void pkEncode(uint8_t *pk, uint8_t rho[32], int32_t t1[k][256]) {
  int bound = (1 << (bit_len(q - 1) - d)) - 1;
  int pack_size = 32 * bit_len(bound); // Size of SimpleBitPack output
  int len = 32 + k * pack_size;

  uint8_t z[pack_size];

  for (int i = 0; i < len; i++) {
    pk[i] = 0;
  }

  memcpy(pk, rho, 32);

  for (int i = 0; i < k; i++) {
    SimpleBitPack(z, t1[i], bound);
    int offset = 32 + i * pack_size;
    memcpy(pk + offset, z, pack_size);
  }
  return;
}

#define sk_len 128 + 32 * (k + l) * (bit_len(2 * ETA))

void skEncode(uint8_t *sk, uint8_t rho[32], uint8_t K[32], uint8_t tr[64],
              int32_t s1[l][256], int32_t s2[k][256], int32_t t_0[k][256]) {
  /* Encodes a secret key for ML-DSA into a byte string */
  // int len = 128 + 32 * (k + l) * (bit_len(2 * ETA));

  // initialize the secret key bite string to zeros
  for (int i = 0; i < sk_len; i++) {
    sk[i] = 0;
  }

  memcpy(sk, rho, 32); // sk <-- rho||K||tr
  memcpy(sk + 32, K, 32);
  memcpy(sk + 64, tr, 64);

  uint8_t c[128];
  for (int i = 0; i < l; i++) {
    BitPack(c, s1[i], ETA, ETA);
    memcpy(sk + 128 + 128 * i, c, 128);
  }

  for (int i = 0; i < k; i++) {
    BitPack(c, s2[i], ETA, ETA);
    memcpy(sk + 128 + 128 * (l + i), c, 128);
  }

  for (int i = 0; i < k; i++) {
    BitPack(c, t_0[i], ETA, ETA);
    memcpy(sk + 128 + 128 * (l + k + i), c,
           32 * (((1 << (d - 1)) - 1) + (1 << (d - 1))));
  }
  return;
}

int KeyGen_internal(uint8_t seed[32], uint8_t *pk, uint8_t *sk) {
  /* Generates a public-private key pair from a seed*/
  uint8_t rho[32];
  uint8_t rho_p[64];
  uint8_t K[32];
  uint8_t *input;
  uint8_t *output;

  int32_t A[k][l][256];
  int32_t s1[l][256];
  int32_t s1_NTT[l][256];
  int32_t s2[k][256];
  int32_t t_0[k][256];
  int32_t t_inv[k][256];
  int32_t t[k][256];
  int32_t t_decomp[k][256][2];
  int32_t t_1[k][256];

  uint8_t tr[64];

  memcpy(input, seed, 32);
  input[32] = k;
  input[33] = l;

  H(input, output,
    128); // hash function generates rho, rho_p, K from random seed

  memcpy(rho, output, 32);
  memcpy(rho_p, output + 32, 64);
  memcpy(K, output + 96, 32);

  ExpandA(rho, A);        // Generate Matrix A in NTT form
  ExpandS(rho_p, s1, s2); // Samples vectors s_1 in R^l and s_2 in R^k

  for (int i = 0; i < l; i++) {
    NTT(s1[i], s1_NTT[i]);
  }

  MatrixVectorNTT(k, l, t_0, A, s1_NTT);

  for (int i = 0; i < k; i++) {
    NTT_inv(t_0[i], t_inv[i]);
  }

  for (int i = 0; i < k; i++) {
    AddNTT(256, t[i], t_inv[i], s2[i]);
  }

  for (int i = 0; i < k; i++) {
    for (int j = 0; j < 256; j++) {
      Power2Round(t[i][j], t_decomp[i][j]);
      t_1[i][j] = t_decomp[i][j][1];
    }
  }

  // get the public key
  pkEncode(&pk, rho, t_1);
  H(&pk, tr, 64); // tr in B^64 (64 bytes string)
  skEncode(&sk, rho, K, tr, s1, s2, t_0);

  return 0;
}