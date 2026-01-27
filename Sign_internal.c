#include "KeyGen_internal.h"
#include "aux_funcs.h"
#include "params.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

void skDecode(uint8_t *sk, uint8_t rho[32], uint8_t K[32], uint8_t tr[64],
              int32_t s1[l][256], int32_t s2[k][256], int32_t t_0[k][256]) {
  /*
  Reverses the procedure skEncode
  Input: secret key sk
  Output: rho, K, tr, s1, s2, t_0
  */
  size_t size = 32 * bit_len(2 * ETA);

  uint8_t y[l][size];
  uint8_t z[k][size];
  uint8_t w[k][32 * d];

  memcpy(rho, sk, 32);
  memcpy(K, sk + 32, 32);
  memcpy(tr, sk + 64, 64);

  int offset = 128;

  for (int i = 0; i < l; i++) {
    for (int j = 0; j < size; j++) {
      y[i][j] = sk[128 + size * i + j];
      offset++;
    }
  }

  for (int i = 0; i < k; i++) {
    for (int j = 0; j < size; j++) {
      z[i][j] = sk[offset + size * i + j];
      offset++;
    }
  }

  for (int i = 0; i < k; i++) {
    for (int j = 0; j < 32 * d; j++) {
      w[i][j] = sk[offset + 32 * d * i + j];
      offset++;
    }
  }

  for (int i = 0; i < l; i++) {
    BitUnpack(s1[i], ETA, ETA, y[i]);
  }

  for (int i = 0; i < k; i++) {
    BitUnpack(s2[i], ETA, ETA, z[i]);
  }

  for (int i = 0; i < k; i++) {
    BitUnpack(t_0[i], (1 << (d - 1)) - 1, (1 << (d - 1)), w[i]);
  }

  return;
}

void ExpandMask(int32_t y[l][256], uint8_t rho[64], uint32_t x) {
  /*Samples a vector y such that each polynomial y[r] has coefficients between
   -delta_1 + 1 and delta_1
   Input: a seed rho and a nonnegatvie integer x
   Output: vector y */
  int c = 1 + bit_len(delta1 - 1);
  uint8_t rho_p[66];
  uint8_t z[2];
  uint8_t H_out[32 * c];
  int32_t poly[256];

  for (int i = 0; i < l; i++) {
    memcpy(rho_p, rho, 64);
    IntegerToBytes(z, x + i, 2);
    memcpy(rho_p, z, 2);

    H(rho_p, 66, H_out, 32 * c);

    BitUnpack(y[i], delta1 - 1, delta1, H_out);
  }
  return;
}

void w1Encode(int32_t w1[k][256], uint8_t *w1tilde, size_t len) {
  /* Encodes a polynomial vector w1 into a byte strin
  Input: w1 a polynomial
  Output: a byte string representation w1tilde with length len given*/
  for (int i = 0; i < len; i++) {
    w1tilde[i] = 0;
  }
  uint8_t *out;
  size_t out_len = 32 * bit_len((q - 1) / (2 * delta2) - 1);
  for (int i = 0; i < k; i++) {
    SimpleBitPack(out, w1[i], (q - 1) / (2 * delta2) - 1);
    memcpy(w1tilde + i * out_len, out, out_len); // error prone with memory
  }
}

void SampleInBall(uint8_t rho[lambda / 4], int32_t c[256]) {
  /* samples a polynomial c in R with coefficients from -1,0,1 and
  Hamming weight tau <= 64
  Input: a seed in bynary string of length lambda/4
  Output: a polynomial c in R */

  uint8_t s[8];
  uint8_t *h;
  uint8_t j;

  for (int i = 0; i < 256; i++) {
    c[i] = 0;
  }

  shake256incctx state;
  shake256_inc_init(&state);
  shake256_inc_absorb(&state, rho, 32);
  shake256_inc_finalize(&state);
  shake256_inc_squeeze(s, 8, &state);

  BytesToBits(h, s, 8);

  for (int i = 256 - tau; i < 256; i++) {
    shake256_inc_squeeze(&j, 1, &state);
    while (j > i) {
      shake256_inc_squeeze(&j, 1, &state);
    }
    c[i] = c[j];
    c[j] = (-1);
    if (h[i + tau - 256] % 2 == 0) {
      c[j] = 1;
    }
  }
}

void Sign_internal(uint8_t *sk, uint8_t *Mp, size_t Mp_size, uint8_t rnd[32]) {
  /*
    Deterministic algorithm to generate a signature for a
    formatted message Mprime
    Input: private key, formatted message, randomness.
    Output: signature
   */

  uint8_t rho[32];
  uint8_t K[32];
  uint8_t tr[64];
  uint8_t trBits[512];
  uint8_t H_in[512 + Mp_size];
  uint8_t mu[64];
  uint8_t in[128];
  uint8_t rho_pp[64];

  int32_t s1[l][256];
  int32_t s2[k][256];
  int32_t t_0[k][256];

  int32_t s1_hat[l][256];
  int32_t s2_hat[k][256];
  int32_t t_0_hat[k][256];

  int32_t A[k][l][256];
  int32_t y[l][256];
  int32_t y_hat[l][256];
  int32_t w[k][256];
  int32_t w_inv[k][256];
  int32_t w1[k][256];
  size_t w1tilde_len = 32 * k * bit_len((q - 1) / (2 * delta2) - 1);
  uint8_t w1tilde[w1tilde_len];
  uint8_t H_in2[64 + w1tilde_len];

  uint8_t ctilde[lambda / 4];
  int32_t c[256];
  int32_t c_NTT[256];

  int32_t cs1_bis[l][256];
  int32_t cs1[l][256];
  int32_t cs2_bis[k][256];
  int32_t cs2[k][256];

  skDecode(sk, rho, K, tr, s1, s2, t_0);

  for (int i = 0; i < l; i++) {
    NTT(s1[i], s1_hat[i]);
    NTT(s2[i], s2_hat[i]);
    NTT(t_0[i], t_0_hat[i]);
  }

  ExpandA(rho, A);
  BytesToBits(trBits, tr, 64);
  memcpy(H_in, trBits, 512);
  memcpy(H_in + 512, Mp, Mp_size);

  H(H_in, 512 + Mp_size, mu, 64);

  memcpy(in, K, 32);
  memcpy(in + 32, rnd, 32);
  memcpy(in + 64, mu, 64);

  H(in, 128, rho_pp, 64);

  uint32_t counter = 0;

  bool checker = true;

  while (checker) {
    ExpandMask(y, rho_pp, counter);

    for (int i = 0; i < l; i++) {
      NTT(y[i], y_hat[i]);
    }

    MatrixVectorNTT(k, l, w, A, y_hat);

    for (int i = 0; i < k; i++) {
      NTT_inv(w[i], w_inv[i]);
    }

    for (int i = 0; i < k; i++) {
      for (int j = 0; j < 256; j++) {
        w1[i][j] = HighBits(w_inv[i][j]);
      }
    }

    w1Encode(w1, w1tilde, w1tilde_len);
    memcpy(H_in2, mu, 64);
    memcpy(H_in2 + 64, w1tilde, w1tilde_len);

    H(H_in2, 64 + w1tilde_len, ctilde, lambda / 4);

    SampleInBall(ctilde, c);

    NTT(c, c_NTT);

    ScalarVectorNTT(l, cs1_bis, c_NTT, s1_hat);

    for (int i = 0; i < l; i++) {
      NTT_inv(cs1_bis[i], cs1[i]);
    }

    // keep implementing here
  }

  return;
}