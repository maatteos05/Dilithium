#include "Sign_internal.h"
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
    for (size_t j = 0; j < size; j++) {
      y[i][j] = sk[offset + size * i + j];
    }
  }

  offset += size * l;
  for (int i = 0; i < k; i++) {
    for (size_t j = 0; j < size; j++) {
      z[i][j] = sk[offset + size * i + j];
    }
  }

  offset += size * k;
  for (int i = 0; i < k; i++) {
    for (int j = 0; j < 32 * d; j++) {
      w[i][j] = sk[offset + 32 * d * i + j];
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

  for (int i = 0; i < l; i++) {
    memcpy(rho_p, rho, 64);
    IntegerToBytes(z, x + i, 2);
    memcpy(rho_p + 64, z, 2);

    H(rho_p, 66, H_out, 32 * c);

    BitUnpack(y[i], delta1 - 1, delta1, H_out);
  }
  return;
}

void w1Encode(int32_t w1[k][256], uint8_t *w1tilde, size_t len) {
  /* Encodes a polynomial vector w1 into a byte strin
  Input: w1 a polynomial
  Output: a byte string representation w1tilde with length len given*/
  for (size_t i = 0; i < len; i++) {
    w1tilde[i] = 0;
  }
  size_t out_len = 32 * bit_len((q - 1) / (2 * delta2) - 1);
  uint8_t out[out_len];
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
  uint8_t h[64];
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

void SigEncode(uint8_t *sigma, size_t len, uint8_t c_tilde[len],
               int32_t z[l][256], bool h[k][256]) {
  /* Encodes a signature into a byte string
  Input: c_tilde a byte string, z a polynomial in R^l, h in R^k_2
  Output: Signature sigma a byte string */

  size_t temp_len = 32 * bit_len(2 * delta1 - 1);
  uint8_t temp[temp_len];
  uint8_t out[omega + k];

  memcpy(sigma, c_tilde, len);

  for (int i = 0; i < l; i++) {
    BitPack(temp, z[i], delta1 - 1, delta1);
    memcpy(sigma + len + i * temp_len, temp, temp_len);
  }

  HintBitPack(out, h);
  memcpy(sigma + len + l * temp_len, out, omega + k);
  return;
}

bool MakeHint(int32_t z, int32_t r) {
  /* Computes hint bit indicating whether adding z to r
     alters the high bits of r
     Input: z, r in Z_q
     Output: bool h*/
  int r1 = HighBits(fqred(r));
  int v1 = HighBits(fqred(r + z)); // Apply modular reduction!

  return (r1 != v1);
}

int Sign_internal(uint8_t *sigma, uint8_t *sk, uint8_t *Mp, size_t Mp_size,
                  uint8_t rnd[32]) {
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

  static int32_t A[k][l][256];
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

  int32_t z[l][256];
  int32_t r0[k][256];
  int32_t z_inf;
  int32_t r0_inf;
  bool h[k][256];
  int count_h;
  int32_t ct0[k][256];
  int32_t ct0_hat[k][256];
  int32_t ct0_inf;

  skDecode(sk, rho, K, tr, s1, s2, t_0);

  for (int i = 0; i < l; i++) {
    NTT(s1[i], s1_hat[i]);
  }

  for (int i = 0; i < k; i++) {
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
    counter += l;

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

    ScalarVectorNTT(k, cs2_bis, c_NTT, s2_hat);
    for (int i = 0; i < k; i++) {
      NTT_inv(cs2_bis[i], cs2[i]);
    }

    AddVectorNTT(l, z, y, cs1); // works although the inputs are in R_q
                                // and not of the form NTT

    for (int i = 0; i < k; i++) {
      for (int j = 0; j < 256; j++) {
        r0[i][j] = LowBits(w_inv[i][j] - cs2[i][j]);
      }
    }

    // Compute ||z||_inf where z in R^l_q
    z_inf = 0;
    int32_t temp;
    for (int i = 0; i < l; i++) {
      temp = infNorm(z[i]);
      if (temp >= z_inf) {
        z_inf = temp;
      }
    }
    // Compute ||r0||_inf where r0 in R^l_q
    r0_inf = 0;
    for (int i = 0; i < k; i++) {
      temp = infNorm(r0[i]);
      if (temp >= r0_inf) {
        r0_inf = temp;
      }
    }

    if (z_inf >= delta1 - beta || r0_inf >= delta2 - beta) {
      continue;
    } else {

      ScalarVectorNTT(k, ct0, c_NTT, t_0_hat);

      for (int i = 0; i < k; i++) {
        NTT_inv(ct0[i], ct0_hat[i]);
      }

      count_h = 0;
      for (int i = 0; i < k; i++) {
        for (int j = 0; j < 256; j++) {
          h[i][j] =
              MakeHint(-ct0_hat[i][j], w_inv[i][j] - cs2[i][j] + ct0_hat[i][j]);
          if (h[i][j] > 0) {
            count_h++;
          }
        }
      }

      // Computes ||ct0_hat||_inf
      ct0_inf = 0;
      for (int i = 0; i < k; i++) {
        temp = infNorm(ct0_hat[i]);
        if (temp >= ct0_inf) {
          ct0_inf = temp;
        }
      }

      if (ct0_inf >= delta2 || count_h > omega) {
        continue;
      }
    }

    checker = false;
  }

  // Center z values from [0, q-1] to signed representation [-(q-1)/2, (q-1)/2]
  // BitPack expects values in [-(delta1-1), delta1], not [0, q-1]
  for (int i = 0; i < l; i++) {
    for (int j = 0; j < 256; j++) {
      if (z[i][j] > q / 2) {
        z[i][j] = z[i][j] - q; // Center to negative half
      }
    }
  }
  SigEncode(sigma, lambda / 4, ctilde, z, h);

  return 0;
}