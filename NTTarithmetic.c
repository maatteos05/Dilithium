#include "NTTarithmetic.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "aux_funcs.h"
#include "params.h"

void AddNTT(int len, int32_t u[len], const int32_t v[len],
            const int32_t w[len]) {
  for (int i = 0; i < len; i++) {
    u[i] = fqred((int64_t)v[i] + (int64_t)w[i]);
  }
}

void AddVectorNTT(int len, int32_t u[len][256], int32_t w[len][256],
                  int32_t v[len][256]) {
  for (int i = 0; i < len; i++) {
    AddNTT(256, u[i], w[i], v[i]);
  }
}

void MultiplyNTT(int len, int32_t c[len], const int32_t a[len],
                 const int32_t b[len]) {
  for (int i = 0; i < len; i++) {
    c[i] = fqred((int64_t)a[i] * (int64_t)b[i]);
  }
}

void ScalarVectorNTT(size_t len, int32_t w[len][256], int32_t c[256],
                     int32_t v[len][256]) {
  for (size_t i = 0; i < len; i++) {
    MultiplyNTT(256, w[i], c, v[i]);
  }
}

void MatrixVectorNTT(uint8_t kk, uint8_t ll, int32_t w[kk][256],
                     const int32_t M[kk][ll][256], const int32_t v[ll][256]) {
  /* Matrix-vector multiplication in T_q:
   *   w[i] = sum_j M[i][j] \cdot v[j]  (coefficientwise mult, all mod q)
   */
  int32_t tmp[256];

  for (uint8_t i = 0; i < kk; i++) {
    for (int t = 0; t < 256; t++)
      w[i][t] = 0;

    for (uint8_t j = 0; j < ll; j++) {
      MultiplyNTT(256, tmp, M[i][j], v[j]);
      AddNTT(256, w[i], w[i], tmp);
    }
  }
}
