#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// #include "NTT-1_zetas.c"
// #include "NTT_zetas.c"
#include "KeyGen_internal.h"

// bit_len(8380416) = 23, so (23 - 13) = 10
/* ML-DSA-44 public key length: 32 + k * (32*(bitlen(q-1)-d)) = 1312 */
static const size_t pk_size = 1312;

void H(uint8_t *input, size_t input_len, uint8_t *output, size_t out_len) {
  // declare the hash function
  shake256incctx state;
  shake256_inc_init(&state);
  shake256_inc_absorb(&state, input, input_len);
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
  /* Sample a kxl matrix A of elements of R_q
     Input: a see rho
     Output: matrix A
     */
  uint8_t rho_p[34];
  for (uint8_t r = 0; r < k; r++) {
    for (uint8_t s = 0; s < l; s++) {
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

  while (len >= 1) {
    int start = 0;
    while (start < 256) {
      m = m + 1;
      int64_t z = zetas[m]; // Use int64 to avoid overflow
      for (int j = start; j < start + len; j++) {
        int32_t t = (int32_t)(((int64_t)z * w_hat[j + len]) % q);
        if (t < 0)
          t += q;

        w_hat[j + len] = (w_hat[j] - t) % q;
        if (w_hat[j + len] < 0)
          w_hat[j + len] += q;

        w_hat[j] = (w_hat[j] + t) % q;
        if (w_hat[j] < 0)
          w_hat[j] += q;
      }
      start = start + 2 * len;
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
      int64_t z = -zetas[m]; // -zetas[m] mod q, always positive

      for (int j = start; j < start + len; j++) {
        int32_t t = w[j];

        w[j] = (t + w[j + len]) % q;
        if (w[j] < 0)
          w[j] += q;

        int32_t diff = (t - w[j + len]) % q;
        if (diff < 0)
          diff += q;

        w[j + len] = (int32_t)((z * diff) % q);
        if (w[j + len] < 0)
          w[j + len] += q;
      }
      start += 2 * len;
    }
    len *= 2;
  }
  int64_t f = 8347681;
  for (int j = 0; j < 256; j++) {
    w[j] = (int32_t)((f * w[j]) % q);
    if (w[j] < 0)
      w[j] += q;
  }
  return;
}

void Power2Round(int32_t r, int32_t r_decomp[2]) {
  /* FIPS 204 Algorithm 35 (Power2Round).
   *
   * Let r+ be the representative of r mod q in {0,...,q-1}.
   * Return (r1, r0) such that r+ = r1*2^d + r0 with
   *   r0 in [-(2^{d-1}-1), 2^{d-1}]  and  r1 in {0,...}
   */
  int32_t rp = r % q;
  if (rp < 0) rp += q;

  int32_t r1 = (rp + (1 << (d - 1)) - 1) >> d;
  int32_t r0 = rp - (r1 << d);

  r_decomp[0] = r0;
  r_decomp[1] = r1;
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

#define sk_len (128 + 32 * ((k + l) * (bit_len(2 * ETA)) + d * k))

void skEncode(uint8_t *sk, uint8_t rho[32], uint8_t K[32], uint8_t tr[64],
              int32_t s1[l][256], int32_t s2[k][256], int32_t t0[k][256]) {
 

  memset(sk, 0, sk_len);

  /* sk = rho || K || tr || ... */
  memcpy(sk, rho, 32);
  memcpy(sk + 32, K, 32);
  memcpy(sk + 64, tr, 64);

  const int s_pack_size = 32 * bit_len(2 * ETA); /* 96 for ETA=2 */
  const int t0_pack_size = 32 * d;              /* 416 for d=13 */

  uint8_t buf_s[128];
  uint8_t buf_t0[32 * d];

  /* s1 (l polynomials) */
  for (int i = 0; i < l; i++) {
    BitPack(buf_s, s1[i], ETA, ETA);
    memcpy(sk + 128 + i * s_pack_size, buf_s, s_pack_size);
  }

  /* s2 (k polynomials) */
  for (int i = 0; i < k; i++) {
    BitPack(buf_s, s2[i], ETA, ETA);
    memcpy(sk + 128 + (l + i) * s_pack_size, buf_s, s_pack_size);
  }

  /* t0 (k polynomials), coefficients in [-(2^{d-1}-1), 2^{d-1}] */
  const int a = (1 << (d - 1)) - 1;
  const int b = (1 << (d - 1));
  for (int i = 0; i < k; i++) {
    BitPack(buf_t0, t0[i], a, b);
    memcpy(sk + 128 + (l + k) * s_pack_size + i * t0_pack_size, buf_t0,
           t0_pack_size);
  }
}

int KeyGen_internal(uint8_t seed[32], uint8_t *pk, uint8_t *sk) {
  /* Generates a public-private key pair from a seed*/
  uint8_t rho[32];
  uint8_t rho_p[64];
  uint8_t K[32];
  uint8_t input[34];
  uint8_t output[128];

  int32_t A[k][l][256];
  int32_t s1[l][256];
  int32_t s1_NTT[l][256];
  int32_t s2[k][256];
  int32_t t_0[k][256];
  int32_t t_inv[k][256];
  int32_t t[k][256];
  int32_t t_decomp[k][256][2];
  int32_t t_0_round[k][256]; /* t0 from Power2Round */
  int32_t t_1[k][256];        /* t1 from Power2Round */

  uint8_t tr[64];
  size_t in_len_H;
  memcpy(input, seed, 32);
  input[32] = k % 256;
  input[33] = l % 256;

  in_len_H = 34;

  H(input, in_len_H, output, 128); // rho || rho' || K

  memcpy(rho, output, 32);
  memcpy(rho_p, output + 32, 64);
  memcpy(K, output + 96, 32);

  ExpandA(rho, A);
  ExpandS(rho_p, s1, s2);

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
      t_0_round[i][j] = t_decomp[i][j][0];
      t_1[i][j] = t_decomp[i][j][1];
    }
  }

  pkEncode(pk, rho, t_1);
  H(pk, pk_size, tr, 64);
  skEncode(sk, rho, K, tr, s1, s2, t_0_round);

  return 0;
}
