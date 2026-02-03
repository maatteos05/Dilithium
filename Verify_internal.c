#include "Verify_internal.h"
#include "KeyGen_internal.h"
#include "Sign_internal.h"
#include "aux_funcs.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

void pkDecode(uint8_t *pk, uint8_t rho[32], int32_t t1[k][256]) {
  /* Reverses the procedure pkEncode */
  memcpy(rho, pk, 32);

  int offset = 32 * (bit_len(q - 1) - d);
  uint8_t z[k][offset];

  for (int i = 0; i < k; i++) {
    memcpy(z[i], pk + 32 + offset * i, offset);
  }

  int pow = bit_len(q - 1) - d;
  for (int i = 0; i < k; i++) {
    SimpleBitUnpack(t1[i], (1 << pow) - 1, z[i]);
  }
  return;
}

void SigDecode(size_t sigma_len, uint8_t sigma[sigma_len],
               uint8_t c_tilde[lambda / 4], int32_t z[l][256], bool h[k][256]) {
  /* Reverses the procedure sigEncode
  Input: signature
  Output: c_tilde, z in R^l, h in R^k_2
  */

  memcpy(c_tilde, sigma, lambda / 4);
  int offset = 32 * (1 + bit_len(delta1 - 1));
  uint8_t x[l][offset];
  uint8_t y[omega + k];

  for (int i = 0; i < l; i++) {
    memcpy(x[i], sigma + lambda / 4 + offset * i, offset);
  }

  memcpy(y, sigma + lambda / 4 + offset * l, omega + k);

  for (int i = 0; i < l; i++) {
    BitUnpack(z[i], delta1 - 1, delta1, x[i]);
  }

  HintBitUnpack(h, y);

  return;
}

int UseHint(bool h, int32_t r) {
  /* Returns the high bits of r adjusted according to hint h
  Input: boolean h, r in Z_q
  Output: r1 in Z */

  int m = (q - 1) / (2 * delta2);
  int32_t r_decomp[2];
  Decompose(r, r_decomp);
  if (h == 1 && r_decomp[0] > 0) {
    return ((r_decomp[1] + 1) % m + m) % m; // Proper positive modulo
  } else if (h == 1 && r_decomp[0] <= 0) {
    return ((r_decomp[1] - 1) % m + m) % m; // Fixed: C modulo can be negative!
  }
  return r_decomp[1];
}
bool Verify_internal(size_t sigma_len, uint8_t sigma[sigma_len], uint8_t *pk,
                     uint8_t *Mp, size_t len_Mp) {
  /* Internal function to verify a signature sigma for a formatted message M' */

  uint8_t rho[32];
  uint8_t c_tilde[lambda / 4];
  uint8_t tr[64];
  uint8_t input[512 + len_Mp];
  uint8_t mu[64];
  size_t w1_len = 32 * k * bit_len((q - 1) / (2 * delta2) - 1);
  uint8_t w1_tilde[w1_len];
  uint8_t in[64 + w1_len];
  uint8_t cprime[lambda / 4];

  int32_t t1[k][256];
  int32_t t1p[k][256];
  int32_t t1p_NTT[k][256];
  int32_t z[l][256];
  int32_t z_NTT[l][256];
  int32_t A[k][l][256];
  int32_t Az[k][256];
  int32_t c[256];
  int32_t c_NTT[256];
  int32_t v[k][256];
  int32_t w[k][256];
  int32_t w_approx[k][256];
  int32_t w1[k][256];

  bool h[k][256];

  pkDecode(pk, rho, t1);

  SigDecode(sigma_len, sigma, c_tilde, z, h);

  // Count non-zero hints and verify count doesn't exceed omega
  int hint_count = 0;
  for (int i = 0; i < k; i++) {
    for (int j = 0; j < 256; j++) {
      if (h[i][j] != 0) {
        hint_count++;
      }
    }
  }

  if (hint_count > omega) {
    return false;
  }

  ExpandA(rho, A);
  H(pk, MLDSA44_PK_LEN, tr, 64);

  uint8_t temp[512];
  BytesToBits(temp, tr, 64);
  memcpy(input, temp, 512);
  memcpy(input + 512, Mp, len_Mp);
  H(input, 512 + len_Mp, mu, 64);

  SampleInBall(c_tilde, c);

  for (int i = 0; i < k; i++) {
    for (int j = 0; j < 256; j++) {
      t1p[i][j] = (1 << d) * t1[i][j];
    }
  }

  // Goal: compute NTT_inv( A * NTT(z) - NTT(c) * NTT(t1 * 2^d) )
  for (int i = 0; i < k; i++) {
    NTT(t1p[i], t1p_NTT[i]);
  }

  NTT(c, c_NTT);

  for (int i = 0; i < l; i++) {
    NTT(z[i], z_NTT[i]);
  }

  ScalarVectorNTT(k, v, c_NTT, t1p_NTT);
  MatrixVectorNTT(k, l, Az, A, z_NTT);

  for (int i = 0; i < k; i++) {
    for (int j = 0; j < 256; j++) {
      v[i][j] = fqred(-v[i][j]); // Proper modular negation
    }
  }

  AddVectorNTT(k, w, v, Az);
  for (int i = 0; i < k; i++) {
    NTT_inv(w[i], w_approx[i]);
    // Ensure values are in proper range for UseHint
    for (int j = 0; j < 256; j++) {
      w_approx[i][j] = fqred(w_approx[i][j]);
    }
  }

  for (int i = 0; i < k; i++) {
    for (int j = 0; j < 256; j++) {
      w1[i][j] = UseHint(h[i][j], w_approx[i][j]);
    }
  }

  memcpy(in, mu, 64);
  size_t w1encode_out_len = 32 * k * bit_len((q - 1) / (2 * delta2) - 1);
  w1Encode(w1, w1_tilde, w1encode_out_len);
  memcpy(in + 64, w1_tilde, w1encode_out_len);

  H(in, 64 + w1encode_out_len, cprime, lambda / 4);

  int max = 0;
  for (int i = 0; i < l; i++) {
    if (infNorm(z[i]) >= max) {
      max = infNorm(z[i]);
    }
  }

  bool alike = true;
  for (int i = 0; i < lambda / 4; i++) {
    if (c_tilde[i] != cprime[i]) {
      alike = false;
    }
  }

  return ((max < (delta1 - beta)) && alike);
}
